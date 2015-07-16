/* Event-Based Branch Facility Tests.
 *
 * Copyright IBM Corp. 2014
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

/* Set, save and restores GPR values to see if EBB handler clobbers any.
   Returns 0 is none has been altered, 1 otherwise.  */
static int
core_busy_loop (void)
{
  int rc = 1;

  asm volatile ("li  3,  0x3030\n"
		"std 3,  -96(1)\n"
		"li  4,  0x4040\n"
		"std 4,  -104(1)\n"
		"li  5,  0x5050\n"
		"std 5,  -112(1)\n"
		"li  6,  0x6060\n"
		"std 6,  -120(1)\n"
		"li  7,  0x7070\n"
		"std 7,  -128(1)\n"
		"li  8,  0x0808\n"
		"std 8,  -136(1)\n"
		"li  9,  0x0909\n"
		"std 9,  -144(1)\n"
		"li  10, 0x1010\n"
		"std 10, -152(1)\n"
		"li  11, 0x1111\n"
		"std 11, -160(1)\n"
		"li  14, 0x1414\n"
		"std 14, -168(1)\n"
		"li  15, 0x1515\n"
		"std 15, -176(1)\n"
		"li  16, 0x1616\n"
		"std 16, -184(1)\n"
		"li  17, 0x1717\n"
		"std 17, -192(1)\n"
		"li  18, 0x1818\n"
		"std 18, -200(1)\n"
		"li  19, 0x1919\n"
		"std 19, -208(1)\n"
		"li  20, 0x2020\n"
		"std 20, -216(1)\n"
		"li  21, 0x2121\n"
		"std 21, -224(1)\n"
		"li  22, 0x2222\n"
		"std 22, -232(1)\n"
		"li  23, 0x2323\n"
		"std 23, -240(1)\n"
		"li  24, 0x2424\n"
		"std 24, -248(1)\n"
		"li  25, 0x2525\n"
		"std 25, -256(1)\n"
		"li  26, 0x2626\n"
		"std 26, -264(1)\n"
		"li  27, 0x2727\n"
		"std 27, -272(1)\n"
		"li  28, 0x2828\n"
		"std 28, -280(1)\n"
		"li  29, 0x2929\n"
		"std 29, -288(1)\n"
		"li  30, 0x3030\n"
#ifndef _CALL_ELF
		"li  31, 0x3131\n"
#endif
		"li    3,  0\n"
		"0: "
		"addi  3, 3, 1\n"
		"cmpwi 3, 100\n"
		"blt   0b\n"
		/* Check none of our registers have been corrupted */
		"cmpwi  4,  0x4040\n"
		"bne	1f\n"
		"cmpwi  5,  0x5050\n"
		"bne	1f\n"
		"cmpwi  6,  0x6060\n"
		"bne	1f\n"
		"cmpwi  7,  0x7070\n"
		"bne	1f\n"
		"cmpwi  8,  0x0808\n"
		"bne	1f\n"
		"cmpwi  9,  0x0909\n"
		"bne	1f\n"
		"cmpwi  10, 0x1010\n"
		"bne	1f\n"
		"cmpwi  11, 0x1111\n"
		"bne	1f\n"
		"cmpwi  14, 0x1414\n"
		"bne	1f\n"
		"cmpwi  15, 0x1515\n"
		"bne	1f\n"
		"cmpwi  16, 0x1616\n"
		"bne	1f\n"
		"cmpwi  17, 0x1717\n"
		"bne	1f\n"
		"cmpwi  18, 0x1818\n"
		"bne	1f\n"
		"cmpwi  19, 0x1919\n"
		"bne	1f\n"
		"cmpwi  20, 0x2020\n"
		"bne	1f\n"
		"cmpwi  21, 0x2121\n"
		"bne	1f\n"
		"cmpwi  22, 0x2222\n"
		"bne	1f\n"
		"cmpwi  23, 0x2323\n"
		"bne	1f\n"
		"cmpwi  24, 0x2424\n"
		"bne	1f\n"
		"cmpwi  25, 0x2525\n"
		"bne	1f\n"
		"cmpwi  26, 0x2626\n"
		"bne	1f\n"
		"cmpwi  27, 0x2727\n"
		"bne	1f\n"
		"cmpwi  28, 0x2828\n"
		"bne	1f\n"
		"cmpwi  29, 0x2929\n"
		"bne	1f\n"
		"cmpwi  30, 0x3030\n"
		"bne	1f\n"
#ifndef _CALL_ELF
		"cmpwi  31, 0x3131\n"
#endif
		"bne	1f\n"
		/* Load junk into all our registers before we reload
		   them from the stack. */
		"li  3,  0xde\n"
		"li  4,  0xad\n"
		"li  5,  0xbe\n"
		"li  6,  0xef\n"
		"li  7,  0xde\n"
		"li  8,  0xad\n"
		"li  9,  0xbe\n"
		"li  10, 0xef\n"
		"li  11, 0xde\n"
		"li  14, 0xad\n"
		"li  15, 0xbe\n"
		"li  16, 0xef\n"
		"li  17, 0xde\n"
		"li  18, 0xad\n"
		"li  19, 0xbe\n"
		"li  20, 0xef\n"
		"li  21, 0xde\n"
		"li  22, 0xad\n"
		"li  23, 0xbe\n"
		"li  24, 0xef\n"
		"li  25, 0xde\n"
		"li  26, 0xad\n"
		"li  27, 0xbe\n"
		"li  28, 0xef\n"
		"li  29, 0xdd\n"
		"ld     3,  -96(1)\n"
		"cmpwi  3,  0x3030\n"
		"bne	1f\n"
		"ld     4,  -104(1)\n"
		"cmpwi  4,  0x4040\n"
		"bne	1f\n"
		"ld     5,  -112(1)\n"
		"cmpwi  5,  0x5050\n"
		"bne	1f\n"
		"ld     6,  -120(1)\n"
		"cmpwi  6,  0x6060\n"
		"bne	1f\n"
		"ld     7,  -128(1)\n"
		"cmpwi  7,  0x7070\n"
		"bne	1f\n"
		"ld     8,  -136(1)\n"
		"cmpwi  8,  0x0808\n"
		"bne	1f\n"
		"ld     9,  -144(1)\n"
		"cmpwi  9,  0x0909\n"
		"bne	1f\n"
		"ld     10, -152(1)\n"
		"cmpwi  10, 0x1010\n"
		"bne	1f\n"
		"ld     11, -160(1)\n"
		"cmpwi  11, 0x1111\n"
		"bne	1f\n"
		"ld     14, -168(1)\n"
		"cmpwi  14, 0x1414\n"
		"bne	1f\n"
		"ld     15, -176(1)\n"
		"cmpwi  15, 0x1515\n"
		"bne	1f\n"
		"ld     16, -184(1)\n"
		"cmpwi  16, 0x1616\n"
		"bne	1f\n"
		"ld     17, -192(1)\n"
		"cmpwi  17, 0x1717\n"
		"bne	1f\n"
		"ld     18, -200(1)\n"
		"cmpwi  18, 0x1818\n"
		"bne	1f\n"
		"ld     19, -208(1)\n"
		"cmpwi  19, 0x1919\n"
		"bne	1f\n"
		"ld     20, -216(1)\n"
		"cmpwi  20, 0x2020\n"
		"bne	1f\n"
		"ld     21, -224(1)\n"
		"cmpwi  21, 0x2121\n"
		"bne	1f\n"
		"ld     22, -232(1)\n"
		"cmpwi  22, 0x2222\n"
		"bne	1f\n"
		"ld     23, -240(1)\n"
		"cmpwi  23, 0x2323\n"
		"bne	1f\n"
		"ld     24, -248(1)\n"
		"cmpwi  24, 0x2424\n"
		"bne	1f\n"
		"ld     25, -256(1)\n"
		"cmpwi  25, 0x2525\n"
		"bne	1f\n"
		"ld     26, -264(1)\n"
		"cmpwi  26, 0x2626\n"
		"bne	1f\n"
		"ld     27, -272(1)\n"
		"cmpwi  27, 0x2727\n"
		"bne	1f\n"
		"ld     28, -280(1)\n"
		"cmpwi  28, 0x2828\n"
		"bne	1f\n"
		"ld     29, -288(1)\n"
		"cmpwi  29, 0x2929\n"
		"bne	1f\n"
		/* Load 0 (success) to return */
		"li	0, 0\n"
		"1: 	mr %0, 0\n"
		: "=r" (rc):	/* no inputs */
		:  "3",  "4",  "5",  "6",  "7",  "8",  "9", "10", "11", "14",
		  "15", "16", "17", "18", "19", "20", "21", "22", "23",
		  "24", "25", "26", "27", "28", "29", "30",
#ifndef _CALL_ELF
		   "31",
#endif
		   "memory");

  return rc;
}


static long int ebb_triggered = 0;

static void attribute_noinline
ebb_handler_test_gpr (void *context)
{
  long int *trigger = (long int *) (context);
  printf ("%s: ebb_triggered = %li (%p)\n", __FUNCTION__, *trigger, trigger);
  *trigger += 1;
}

static int
ebb_test_pmu_grp_clobber ()
{
  ebbhandler_t handler;
  int ret = 0;

  printf ("%s: testing GRP clobbering\n", __FUNCTION__);

  handler = paf_ebb_register_handler (ebb_handler_test_gpr,
				      (void *) &ebb_triggered,
				      PAF_EBB_CALLBACK_GPR_SAVE, 0);
  if (handler != ebb_handler_test_gpr)
    {
      printf ("Error: paf_ebb_register_handler \
	      (ebb_handler_test_gpr) != handler\n");
      return -1;
    }

  paf_ebb_enable_branches ();

  paf_ebb_pmu_reset ();

  while (ebb_triggered <= 10)
    {
      if (ebb_check_mmcr0 ())
	return 1;
      int rc = core_busy_loop ();
      if (rc != 0)
	printf ("%s: core_busy_loop returned %i\n", __FUNCTION__, rc);
      ret += rc;
    }

  paf_ebb_disable_branches ();

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

  ret = ebb_test_pmu_grp_clobber ();

  close (ebbfd);

  return ret;
}

#define EBB_TEST ebb_test_pmu_clobber

#include "test_ebb-skeleton.c"
