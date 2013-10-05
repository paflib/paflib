/* Event-Based Branch Facility Tests.  Multi-counter test.
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

#include <paf/ebb.h>
#include "test_ebb_common.h"

#define PM_CMPLU_STALL_THRD        0x1001C
#define PM_CMPLU_STALL_FXU         0x2D016
#define PM_CMPLU_STALL_OTHER_CMPL  0x30006
#define PM_CMPLU_STALL             0x4000A
#define PM_RUN_CYC                 0x600f4
#define PM_RUN_INST_CMPL           0x500fa


/* Set it volatile to force memory read in loop below.  */
static volatile int ebb_handler_triggered = 0;

#define TEST_LOOP_COUNT 2

static void
attribute_noinline
ebb_handler_test (void *context)
{
  int *trigger = (int*) (context);
  printf ("%s: triggered\n", __FUNCTION__);
  *trigger += 1;
}

static int
ebb_test_pmu_multi_counter (void)
{
  int ebbfd_group;
  int ebbfd_child;
  ebbhandler_t handler;

#define CHECK_OPEN(result, event)                        \
  if (result == -1)                                      \
    {                                                    \
      printf("Error: paf_ebb_init_pmu (%s, -1) failed "   \
	     "(errno = %i)\n", event, errno);            \
      return -1;                                         \
    }

  ebbfd_group = paf_ebb_pmu_init (PM_CMPLU_STALL_THRD, -1);
  CHECK_OPEN(ebbfd_group, "PM_CMPLU_STALL_THRD");
  ebbfd_child = paf_ebb_pmu_init (PM_CMPLU_STALL_FXU, ebbfd_group);
  CHECK_OPEN(ebbfd_child, "PM_CMPLU_STALL_FXU");
  ebbfd_child = paf_ebb_pmu_init (PM_CMPLU_STALL_OTHER_CMPL, ebbfd_group);
  CHECK_OPEN(ebbfd_child, "PM_CMPLU_STALL_OTHER_CMPL");
  ebbfd_child = paf_ebb_pmu_init (PM_CMPLU_STALL, ebbfd_group);
  CHECK_OPEN(ebbfd_child, "PM_CMPLU_STALL");
  ebbfd_child = paf_ebb_pmu_init (PM_RUN_CYC, ebbfd_group);
  CHECK_OPEN(ebbfd_child, "PM_RUN_CYC");
  ebbfd_child = paf_ebb_pmu_init (PM_RUN_INST_CMPL, ebbfd_group);
  CHECK_OPEN(ebbfd_child, "PM_RUN_INST_CMPL");

  printf ("%s: testing multi events\n", __FUNCTION__);

  ebb_handler_triggered = 0;

  handler = paf_ebb_register_handler (ebb_handler_test,
				     (void*)&ebb_handler_triggered,
				     PAF_EBB_CALLBACK_GPR_SAVE,
				     PAF_EBB_FLAGS_RESET_PMU);
  if (handler != ebb_handler_test)
    {
      printf ("Error: paf_ebb_register_handler \
	      (ebb_handler_test) != handler\n");
      return -1;
    }

  paf_ebb_enable_branches ();

  while (ebb_handler_triggered != TEST_LOOP_COUNT)
    {
      if (ebb_check_mmcr0())
        return 1;
    }

  paf_ebb_disable_branches ();

  return 0;
}

#define EBB_TEST ebb_test_pmu_multi_counter

#include "test_ebb-skeleton.c"
