/* Event-Based Branch Facility Tests.
 *
 * Copyright IBM Corp. 2014-2015
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

#include <paf/ebb.h>
#include "test_ebb_common.h"

#define PM_RUN_INST_CMPL  0x400FA

#define TEST_ITERATIONS (4)

#define TIMEOUT 100

extern int ebb_test_gpr(void);
extern int ebb_test_fpr(void);
extern int ebb_test_vr(void);
extern int ebb_test_vsr(void);

extern void ebb_clobber_gpr(void*);
extern void ebb_clobber_fpr(void*);
extern void ebb_clobber_vr(void*);
extern void ebb_clobber_vsr(void*);

struct test {
    int (*lfunc)(void);
    void (*cfunc)(void*);
    char const *name;
    int trigger;
    paf_ebb_callback_type_t type;
};

struct test clobber_tests[] = {
  { &ebb_test_fpr, &ebb_clobber_fpr, "FPR", 0, PAF_EBB_CALLBACK_FPR_SAVE },
  { &ebb_test_vsr, &ebb_clobber_vsr, "VSR", 0, PAF_EBB_CALLBACK_VSR_SAVE },
  { &ebb_test_gpr, &ebb_clobber_gpr, "GPR", 0, PAF_EBB_CALLBACK_GPR_SAVE },
  { &ebb_test_vr,  &ebb_clobber_vr,  "VR",  0, PAF_EBB_CALLBACK_VR_SAVE  },
  { NULL, NULL, 0, 0 }
};

static long test_idx = 0;

/* Callbed by the ASM hooks before clobbering.  */
void attribute_noinline
ebb_handler_test (void *arg)
{
  struct test *test = &clobber_tests[test_idx];
  printf ("%s: %s: ebb_triggered = %d\n", __FUNCTION__, test->name, test->trigger);
  test->trigger += 1;
}

static int
run_ebb_test_pmu_clobber ()
{
  ebbhandler_t handler;
  int ret = 0;
  struct test *test;

  for (test = &clobber_tests[0]; test->lfunc; test++, test_idx++)
    {
      printf ("%s: testing %s clobbering\n", __FUNCTION__, test->name);
      handler = paf_ebb_register_handler (test->cfunc, //ebb_handler_test,
					  (void *) test,
					  test->type, 0);
      //if (handler != ebb_handler_test)
      if (handler != test->cfunc)
	{
	  printf ("Error: paf_ebb_register_handler \
		  (ebb_handler_test) != handler\n");
	  return -1;
	}
      paf_ebb_enable_branches ();
      paf_ebb_pmu_reset ();
      while (test->trigger <= TEST_ITERATIONS)
	{
	  if (ebb_check_mmcr0 ())
	    return 1;
	  long rc = test->lfunc();
	  if (rc != 0)
	    {
	      printf ("%s: check returned %lu (0x%lx)\n", __FUNCTION__, rc, rc);
	      break;
	    }
	  ret += rc;
	}
      paf_ebb_disable_branches ();
    }

  return ret;
}

static int
ebb_test_pmu_clobber (void)
{
  int ebbfd;
  int ret = 0;

  ebbfd = paf_ebb_pmu_init (PM_RUN_INST_CMPL, -1);
  if (ebbfd == -1)
    {
      printf ("Error: paf_ebb_init_pmu (PM_RUN_CYC, -1) failed "
	      "(errno = %i)\n", errno);
      return -1;
    }

  ret = run_ebb_test_pmu_clobber ();

  close (ebbfd);

  return ret;
}

#define EBB_TEST ebb_test_pmu_clobber

#include "test_ebb-skeleton.c"
