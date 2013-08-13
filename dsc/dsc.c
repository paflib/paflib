#include <errno.h>
#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <pthread.h>

#include <paf/dsc.h>
#include "config.h"
#include "common.h"
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

#define __stringify(x)        #x

#define mfspr(SPRN)	\
	({ unsigned long int value;		\
	   asm volatile("mfspr %0," __stringify(SPRN)	\
			 : "=r" (value));	\
	   value;				\
	})

#define mfspr_dscr(rn)	(rn == SPRN_DSCR_USER ? \
		mfspr(SPRN_DSCR_USER) : mfspr(SPRN_DSCR))

#define mtspr(value, SPRN) \
	({ asm volatile("mtspr " __stringify(SPRN) ",%0"	\
			 : : "r" (value));	\
	})

#define mtspr_dscr(value, rn)	(rn == SPRN_DSCR_USER ? \
		mtspr(value, SPRN_DSCR_USER) : mtspr(value, SPRN_DSCR))

/* mask of DSCR features supported  (0 if not supported) */
static volatile long int dscr_support_mask = -1;

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
  dscr_support_mask = 0;
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
      dscr_support_mask = 1;
      /* try to issue a simple get dscr using non privileged SPR */
      dscr_sprn = SPRN_DSCR_USER;
      mfspr(SPRN_DSCR_USER);

      /* check if the previous mfspr was unsuccessful (i.e., SIGILLed) */
      if (dscr_support_mask == 0)
	{
	  /* if unsuccessful, try using the former (privileged) SPR */
	  dscr_support_mask = 1;
	  dscr_sprn = SPRN_DSCR;
	  mfspr(SPRN_DSCR);
	}

      /* re-sets the former sigaction */
      sigaction(SIGILL, &old_sa, NULL);

      /* if the insn is not illegal, check for hw supported features */
      if (dscr_support_mask != 0)
        dscr_support_mask = check_hwcap();
    }
  else
    {
      perror("Cannot set signal handler to check dscr support.\n");
      dscr_support_mask = 0;
      init_errno = ENOSYS;
      return -1;
    }

  return dscr_support_mask;
}

long int
attribute_hidden
check_dscr_support(void)
{
  if (dscr_support_mask == -1)
    {
      if (pthread_once (&check_dscr_once_control,
			(void (*)(void))check_dscr_insn))
	{ 
	  perror("Error pthread_once(check_dscr_insn()) failed");
	  return -1;
	}

      /* check if something went wrong when calling check_dscr_insn() and set
	 errno properly */
      if (init_errno != 0)
	{
	  errno = init_errno;
	  return -1;
	}
    }

  return dscr_support_mask;
}

long int paf_dsc_get(void)
{
  unsigned long int dscr;

  if (!dscr_support_mask)
    {
      errno = ENOSYS;
      return -1;
    }

  dscr = mfspr_dscr(dscr_sprn);

  DEBUG("get 0x%lx", dscr);
  return dscr;
}

int paf_dsc_set(unsigned long dscr)
{
  /* check whether DSCR support is available */
  if (!dscr_support_mask)
    {
      errno = ENOSYS;
      return -1;
    }

  /* check whether the value to be set in DSCR is supported in this machine */
  if (dscr_support_mask == 0 || (dscr | dscr_support_mask) != dscr_support_mask)
    {
      DEBUG("Cannot set dscr 0x%lx (supported mask 0x%lx)", dscr, dscr_support_mask);
      errno = EINVAL;
      return -1;
    }

  DEBUG("set 0x%lx", dscr);
  mtspr_dscr(dscr, dscr_sprn);

  return 0;
}
