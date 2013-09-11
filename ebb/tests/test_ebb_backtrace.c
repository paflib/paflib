/* Event-Based Branch Facility Tests.
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
 *     IBM Corporation, Adhemerval Zanella - Initial implementation.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>

#include <paf/ebb.h>
#include "ebb-callback.h"
#include "test_ebb_common.h"

#define PM_RUN_INST_CMPL  0x400FA

static inline int
match_sym (const char *sym, const char *name)
{
  char *p = strchr (sym, '(');
  return p != NULL && strstr (p, name) != NULL;
}

static inline int
match_lib (const char *sym, const char *name)
{
  return strstr (sym, name) != NULL;
}

static volatile int ebb_handler_triggered = 0;

void
attribute_noinline
ebb_handler_backtrace (void *context)
{
  int j, n;
#define NUM_FUNCTIONS 6
  void *buffer[NUM_FUNCTIONS];
  char **symbols;

  n = backtrace (buffer, NUM_FUNCTIONS);
  if (n < NUM_FUNCTIONS)
    {
      fprintf (stderr, "%s: backtrace() returned %i, expecting at least %i\n",
	__FUNCTION__, n, NUM_FUNCTIONS);
      exit (EXIT_FAILURE);
    }
  printf ("%s: backtrace() returned %d addresses\n", __FUNCTION__, n);

  symbols = backtrace_symbols (buffer, NUM_FUNCTIONS);
  if (symbols == NULL)
    {
      fprintf (stderr, "%s: backtrace_symbols() failed (ernro %i)\n",
	__FUNCTION__, errno);
      exit (EXIT_FAILURE);
    }

  for (j = 0; j < n; j++)
    printf("%s: %s\n", __FUNCTION__, symbols[j]);

  /* Check if the function names make sense, the backtrace expected is:
     test_paf_ebb_backtrace : ebb_handler_backtrace
     libpaf-ebb.so          : internal symbol (__paf_ebb_ebb_hook)
     libpaf-ebb.so          : internal symbol (__paf_ebb_callback_handler_gpr
     test_paf_ebb_backtrce  : ebb_test_backtrace
     libc.so ...
   */
  if (!match_sym (symbols[0], "ebb_handler_backtrace"))
    {
      fprintf (stderr, "%s: symbol[0] (%s) does not math %s\n",
	__FUNCTION__, symbols[0], "ebb_handler_backtrace");
      exit (EXIT_FAILURE);
    }
  if (!match_lib (symbols[1], "libpaf-ebb.so") ||
      !match_lib (symbols[2], "libpaf-ebb.so"))
    {
      fprintf (stderr, "%s: symbol[1] (%s) and symbol[2] (%s) "
        "does not math %s\n", __FUNCTION__, symbols[1], symbols[2],
        "libpaf_ebb.so");
      exit (EXIT_FAILURE);
    }
  if (!match_sym (symbols[3], "ebb_test_backtrace"))
    {
      fprintf (stderr, "%s: symbol[0] (%s) does not math %s\n",
	__FUNCTION__, symbols[0], "ebb_test_backtrace");
      exit (EXIT_FAILURE);
    }

  free (symbols);

  ebb_handler_triggered = 1;
}

int
ebb_test_backtrace (void)
{
  ebbhandler_t handler;
  int ebbfd;

  ebbfd = paf_ebb_pmu_init (PM_RUN_INST_CMPL, -1);
  if (ebbfd == -1)
    {
      printf("Error: paf_ebb_init_pmu (PM_RUN_CYC, -1) failed "
	     "(errno = %i)\n", errno);
      return -1;
    }

  printf ("%s: testing backtrace information\n", __FUNCTION__);

  ebb_handler_triggered = 0;

  handler = paf_ebb_register_handler (ebb_handler_backtrace,
				     (void*)&ebb_handler_triggered,
				     PAF_EBB_CALLBACK_GPR_SAVE, 0);
  if (handler != ebb_handler_backtrace)
    {
      printf ("Error: paf_ebb_register_handler \
	      (ebb_handler_test) != handler\n");
      return -1;
    }

  paf_ebb_enable_branches ();

  while (ebb_handler_triggered == 0)
    {
      if (ebb_check_mmcr0())
        return 1;
    }

  paf_ebb_disable_branches ();

  return 0;
}

#define EBB_TEST ebb_test_backtrace

#include "test_ebb-skeleton.c"
