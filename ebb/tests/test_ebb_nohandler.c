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
 */

#include <stdio.h>
#include <errno.h>
#include <paf/ebb.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include "test_ebb_common.h"

#define PM_CONFIG 0x1001e
/* Test that things work sanely if we have no handler */
int
ebb_no_handler_test (void)
{
  int i, ebbfd;
  unsigned long long val;

  ebbfd = paf_ebb_pmu_init (PM_CONFIG, -1);
  if (ebbfd == -1)
    {
      printf ("Error: paf_ebb_init_pmu () failed " "(errno = %i)\n", errno);
      return -1;
    }
  val = mfspr (EBBHR);
  if (val != 0)
    printf ("Error\n");

  /* Make sure it overflows quickly */
  mtspr (PMC1, 0x80000000ull - 100);

  /* Spin to make sure the event has time to overflow */
  for (i = 0; i < 100; i++)
    asm volatile ("sync":::"memory");

  /* We expect to see PMAO set */
  val = mfspr (MMCR0);
  if (val != (MMCR0_FC | MMCR0_PMAO))
    {
      printf ("Error: PMAO is not set\n");
      return -1;
    }
  paf_ebb_event_close (ebbfd);

  return 0;
}

#define EBB_TEST ebb_no_handler_test

#include "test_ebb-skeleton.c"
