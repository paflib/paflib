/*
 * Event-Based Branch Facility Tests.
 *
 * Copyright (c) 2013 IBM Corporation.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 * 
 * Contributors:
 *     IBM Corporation, Adhemerval Zanella - Initial implementation.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>

#include "ppcebb.h"
#include "ppcebb-callback.h"
#include "test_ppcebb_common.h"

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
     test_ppcebb_backtrace : ebb_handler_backtrace
     libebb.so             : internal symbol (__ppcebb_ebb_hook)
     libebb.so             : internal symbol (__ppcebb_callback_handler_gpr
     test_ppcebb_backtrce  : ebb_test_backtrace
     libc.so ...
   */
  if (!match_sym (symbols[0], "ebb_handler_backtrace"))
    {
      fprintf (stderr, "%s: symbol[0] (%s) does not math %s\n",
	__FUNCTION__, symbols[0], "ebb_handler_backtrace");
      exit (EXIT_FAILURE);
    }
  if (!match_lib (symbols[1], "libppcebb.so") ||
      !match_lib (symbols[2], "libppcebb.so"))
    {
      fprintf (stderr, "%s: symbol[1] (%s) and symbol[2] (%s) "
        "does not math %s\n", __FUNCTION__, symbols[1], symbols[2],
        "libppcebb.so");
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

  ebbfd = ppcebb_pmu_init (PM_RUN_INST_CMPL, -1);
  if (ebbfd == -1)
    {
      printf("Error: ppcebb_init_pmu (PM_RUN_CYC, -1) failed "
	     "(errno = %i)\n", errno);
      return -1;
    }

  printf ("%s: testing backtrace information\n", __FUNCTION__);

  ebb_handler_triggered = 0;

  handler = ppcebb_register_handler (ebb_handler_backtrace,
				     (void*)&ebb_handler_triggered,
				     PPCEBB_CALLBACK_GPR_SAVE, 0);
  if (handler != ebb_handler_backtrace)
    {
      printf ("Error: ppc_register_ebb_handler \
	      (ebb_handler_test) != handler\n");
      return -1;
    }

  ppcebb_enable_branches ();

  while (ebb_handler_triggered == 0)
    {
      if (ebb_check_mmcr0())
        return 1;
    }

  ppcebb_disable_branches ();

  return 0;
}

#define EBB_TEST ebb_test_backtrace

#include "test_ppcebb-skeleton.c"
