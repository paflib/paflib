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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include "ppcebb.h"
#include "test_ppcebb_common.h"

#define PM_RUN_INST_CMPL  0x400FA

/* Set it volatile to force memory read in loop below.  */
static volatile int ebb_handler_triggered = 0;

#define TEST_LOOP_COUNT 2

static void
attribute_noinline
ebb_handler_test (void *context)
{
  int *trigger = (int*) (context);
  printf ("%s: ebb_handler_triggered address = %p\n", __FUNCTION__, trigger);
  *trigger += 1;
}


static int
ebb_test_pmu_reset_flag (ppcebb_callback_type_t type)
{
  ebbhandler_t handler;

  printf ("%s: testing %s save/restore:\n", __FUNCTION__,
    ebb_callback_type_string (type));

  ebb_handler_triggered = 0;

  handler = ppcebb_register_handler (ebb_handler_test,
				    (void*)&ebb_handler_triggered,
				     type,
				     PPCEBB_FLAGS_RESET_PMU);
  if (handler != ebb_handler_test)
    {
      printf ("Error: ppc_register_ebb_handler \
	      (ebb_handler_test) != handler\n");
      return -1;
    }

  ppcebb_enable_branches ();

  while (ebb_handler_triggered != TEST_LOOP_COUNT)
    {
      if (ebb_check_mmcr0())
        return 1;
    }

  ppcebb_disable_branches ();

  return 0;
}

static int
ebb_test_pmu_reset_all (void)
{
  int ebbfd;
  int ret;

  ebbfd = ppcebb_pmu_init (PM_RUN_INST_CMPL, -1);
  if (ebbfd == -1)
    {
      printf("Error: ppcebb_init_pmu (PM_RUN_CYC, -1) failed "
	     "(errno = %i)\n", errno);
      return -1;
    }

  ret  = ebb_test_pmu_reset_flag (PPCEBB_CALLBACK_GPR_SAVE);
  ret += ebb_test_pmu_reset_flag (PPCEBB_CALLBACK_FPR_SAVE);
  ret += ebb_test_pmu_reset_flag (PPCEBB_CALLBACK_VR_SAVE);
  ret += ebb_test_pmu_reset_flag (PPCEBB_CALLBACK_VSR_SAVE);

  close (ebbfd);

  return ret;
}

#define EBB_TEST ebb_test_pmu_reset_all

#include "test_ppcebb-skeleton.c"
