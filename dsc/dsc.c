/* libpaf-dsc primary implementation details.
 *
 * Copyright IBM Corp. 2013
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 *     IBM Corporation, Edjunior Barbosa Machado - Initial implementation.
 */

#include <errno.h>
#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdlib.h>

#include "config.h"
#include <paf/dsc.h>
#include "paf-common.h"
#include "hwcap.h"

/* PowerPC instruction length */
#define PPC_INSN_LEN	4
/* SPR number used for DSCR */
typedef enum {
  SPRN_DSCR_USER = 0x03,
#define SPRN_DSCR_USER 0x03
  SPRN_DSCR = 0x11
#define SPRN_DSCR 0x11
} dscr_sprn_t;

#define mfspr_dscr(rn) \
  (rn == SPRN_DSCR_USER ? mfspr(SPRN_DSCR_USER) : mfspr(SPRN_DSCR))

#define mtspr_dscr(rn, value) \
  (rn == SPRN_DSCR_USER ? mtspr(SPRN_DSCR_USER, value) : mtspr(SPRN_DSCR, value))

/* mask of DSCR features supported  (0 if not supported) */
static volatile uint64_t *dscr_support_mask = NULL;

static dscr_sprn_t dscr_sprn = SPRN_DSCR_USER;

/* error control inside the thread check_dscr_insn() */
static int init_errno = 0;

static pthread_once_t check_dscr_once_control = PTHREAD_ONCE_INIT;

void static
sigill_callback(int sig, siginfo_t *si, void *ctx)
{
  ucontext_t *uctx = (ucontext_t*)(ctx);
  DEBUG("Caught signal %d. No DSCR support available in kernel using SPR %d", sig, dscr_sprn);
  /* set dscr_support_mask as support unavailable */
  *dscr_support_mask = 0;
  /* manually increment PC register in order to step over the illegal insn */
  uctx->uc_mcontext.regs->nip += PPC_INSN_LEN;
}

/* verify whether dscr-related insn is treated as illegal or not */
static int
check_dscr_insn(void)
{
  /* set up a signal handler to check whether the DSCR instruction
     (m[ft]spr) is treated as illegal or not */
  struct sigaction sa, old_sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_ONSTACK | SA_SIGINFO;
  sa.sa_sigaction = sigill_callback;
  if (sigaction(SIGILL, &sa, &old_sa) == 0)
    {
      /* dscr_support_mask will be used to check if mfspr insn does not sigill.
         Marking as support available initially.  */
      *dscr_support_mask = 1;
      /* try to issue a simple get dscr using non privileged SPR */
      dscr_sprn = SPRN_DSCR_USER;
      mfspr(SPRN_DSCR_USER);

      /* check if the previous mfspr was unsuccessful (i.e., SIGILLed) */
      if (*dscr_support_mask == 0)
	{
	  /* if unsuccessful, try using the former (privileged) SPR */
	  *dscr_support_mask = 1;
	  dscr_sprn = SPRN_DSCR;
	  mfspr(SPRN_DSCR);
	}

      /* re-sets the former sigaction */
      sigaction(SIGILL, &old_sa, NULL);

      /* if the insn is not illegal, check for hw supported features */
      if (*dscr_support_mask != 0)
        *dscr_support_mask = check_hwcap();
    }
  else
    {
      perror("Cannot set signal handler to check dscr support.\n");
      *dscr_support_mask = 0;
      init_errno = ENOSYS;
      return -1;
    }

  return 0;
}

uint64_t
paf_dsc_check_support(void)
{
  if (dscr_support_mask == NULL)
    {
      dscr_support_mask = (uint64_t *)malloc(sizeof(uint64_t));
      if (pthread_once (&check_dscr_once_control,
			(void (*)(void))check_dscr_insn))
	{ 
	  perror("Error pthread_once(check_dscr_insn()) failed");
	  *dscr_support_mask = 0;
	  errno = ENOSYS;
	  return 0;
	}
      /* check if something went wrong when calling check_dscr_insn() and set
         errno properly */
      if (init_errno != 0)
	{
	  *dscr_support_mask = 0;
	  errno = init_errno;
	  return 0;
	}
    }

  if (*dscr_support_mask == 0)
      errno = ENOSYS;
      
  return *dscr_support_mask;
}

int
paf_dsc_get(uint64_t *dscr)
{
  if (*dscr_support_mask == 0)
    {
      errno = ENOSYS;
      return -1;
    }

  *dscr = mfspr_dscr(dscr_sprn);

  DEBUG("get 0x%" PRIx64, *dscr);
  return 0;
}

int
paf_dsc_set(uint64_t dscr)
{
  /* check whether DSCR support is available */
  if (*dscr_support_mask == 0)
    {
      errno = ENOSYS;
      return -1;
    }

  /* check whether the value to be set in DSCR is supported in this machine */
  if ((dscr | *dscr_support_mask) != *dscr_support_mask)
    {
      DEBUG("Cannot set dscr 0x%" PRIx64 " (supported mask 0x%" PRIx64 ")", dscr, *dscr_support_mask);
      errno = EINVAL;
      return -1;
    }

  DEBUG("set 0x%" PRIx64, dscr);
  mtspr_dscr(dscr_sprn, dscr);

  return 0;
}
