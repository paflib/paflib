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

/* Set it volatile to force memory read in loop below.  */
static volatile int ebb_handler_triggered = 0;

#define EXPECTED_R3_VALUE      2U
#define EXPECTED_R4_VALUE      4U
#define EXPECTED_R5_VALUE      8U
#define EXPECTED_R6_VALUE     16U
#define EXPECTED_R7_VALUE     32U
#define EXPECTED_R8_VALUE     64U
#define EXPECTED_R15_VALUE   128U
#define EXPECTED_R16_VALUE   256U
#define EXPECTED_R18_VALUE   512U
#define EXPECTED_R21_VALUE  1024U
#define EXPECTED_R25_VALUE  4096U
#define EXPECTED_R30_VALUE  8192U

#ifdef __powerpc64__
#define LD_INST         "ld"
#define CALLER_FRAME    112+112
#define REG_SIZE        8
#else /* __powerpc__ */
#define LD_INST         "lwz"
#define CALLER_FRAME    16+16+64
#define REG_SIZE        4
#endif

#define CR_SAVE         (CALLER_FRAME)
#define CR_SIZE         (REG_SIZE)
#define LR_SAVE         (CR_SAVE + CR_SIZE)
#define LR_SIZE         (REG_SIZE)
#define CTR_SAVE        (LR_SAVE + LR_SIZE)
#define CTR_SIZE        (REG_SIZE)
#define XER_SAVE        (CTR_SAVE + CTR_SIZE)
#define XER_SIZE        (REG_SIZE)
#define GPR_SAVE(n)     (XER_SAVE + XER_SIZE + (REG_SIZE * n)) 

#ifdef __powerpc64__
#define R3_DISP         GPR_SAVE(2)
#define R4_DISP         GPR_SAVE(3)
#define R5_DISP         GPR_SAVE(4)
#define R6_DISP         GPR_SAVE(5)
#define R7_DISP         GPR_SAVE(6)
#define R8_DISP         GPR_SAVE(7)
#define R15_DISP        GPR_SAVE(13)
#define R16_DISP        GPR_SAVE(14)
#define R18_DISP        GPR_SAVE(16)
#define R21_DISP        GPR_SAVE(19)
#define R25_DISP        GPR_SAVE(23)
#define R30_DISP        GPR_SAVE(28)
#else
#define R3_DISP         GPR_SAVE(1)
#define R4_DISP         GPR_SAVE(2)
#define R5_DISP         GPR_SAVE(3)
#define R6_DISP         GPR_SAVE(4)
#define R7_DISP         GPR_SAVE(5)
#define R8_DISP         GPR_SAVE(6)
#define R15_DISP        GPR_SAVE(13)
#define R16_DISP        GPR_SAVE(14)
#define R18_DISP        GPR_SAVE(16)
#define R21_DISP        GPR_SAVE(19)
#define R25_DISP        GPR_SAVE(23)
#define R30_DISP        GPR_SAVE(28)
#endif

static void
attribute_noinline
ebb_handler_test (void *context)
{
  int *trigger = (int*) (context);
  unsigned long int r3;
  unsigned long int r4;
  unsigned long int r5;
  unsigned long int r6;
  unsigned long int r7;
  unsigned long int r8;
  unsigned long int r15;
  unsigned long int r16;
  unsigned long int r18;
  unsigned long int r21;
  unsigned long int r25;
  unsigned long int r30;

  asm (LD_INST " %0,%12(1)\n"
       LD_INST " %1,%13(1)\n"
       LD_INST " %2,%14(1)\n"
       LD_INST " %3,%15(1)\n"
       LD_INST " %4,%16(1)\n"
       LD_INST " %5,%17(1)\n"
       LD_INST " %6,%18(1)\n"
       LD_INST " %7,%19(1)\n"
       LD_INST " %8,%20(1)\n"
       LD_INST " %9,%21(1)\n"
       LD_INST " %10,%22(1)\n"
       LD_INST " %11,%23(1)\n"
       : "=r"(r3), "=r"(r4), "=r"(r5), "=r"(r6), "=r"(r7), "=r"(r8),
         "=r"(r15), "=r"(r16), "=r"(r18), "=r"(r21), "=r"(r25), "=r"(r30)
       : "i"(R3_DISP),  "i"(R4_DISP),  "i"(R5_DISP),  "i"(R6_DISP),
         "i"(R7_DISP),  "i"(R8_DISP),  "i"(R15_DISP), "i"(R16_DISP),
         "i"(R18_DISP), "i"(R21_DISP), "i"(R25_DISP), "i"(R30_DISP));

  *trigger = 1;

  *trigger += (r3 == EXPECTED_R3_VALUE);
  *trigger += (r4 == EXPECTED_R4_VALUE);
  *trigger += (r5 == EXPECTED_R5_VALUE);
  *trigger += (r6 == EXPECTED_R6_VALUE);
  *trigger += (r7 == EXPECTED_R7_VALUE);
  *trigger += (r8 == EXPECTED_R8_VALUE);
  *trigger += (r15 == EXPECTED_R15_VALUE);
  *trigger += (r16 == EXPECTED_R16_VALUE);
  *trigger += (r18 == EXPECTED_R18_VALUE);
  *trigger += (r21 == EXPECTED_R21_VALUE);
  *trigger += (r25 == EXPECTED_R25_VALUE);
  *trigger += (r30 == EXPECTED_R30_VALUE);

  printf ("%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n%ld\n",
          r3, r4, r5, r6, r7, r8, r15, r16, r18, r21, r25, r30);
}

static int
ebb_test_pmu_grp_clobber ()
{
  ebbhandler_t handler;
  register unsigned long int r3  __asm__ ("r3");
  register unsigned long int r4  __asm__ ("r4");
  register unsigned long int r5  __asm__ ("r5");
  register unsigned long int r6  __asm__ ("r6");
  register unsigned long int r7  __asm__ ("r7");
  register unsigned long int r8  __asm__ ("r8");
  register unsigned long int r15 __asm__ ("r15");
  register unsigned long int r16 __asm__ ("r16");
  register unsigned long int r18 __asm__ ("r18");
  register unsigned long int r21 __asm__ ("r21");
  register unsigned long int r25 __asm__ ("r25");
  register unsigned long int r30 __asm__ ("r30");

  printf ("%s: testing GRP clobbering\n", __FUNCTION__);

  ebb_handler_triggered = 0;

  handler = paf_ebb_register_handler (ebb_handler_test,
				      (void*)&ebb_handler_triggered,
				      PAF_EBB_CALLBACK_GPR_SAVE, 0);
  if (handler != ebb_handler_test)
    {
      printf ("Error: paf_ebb_register_handler \
	      (ebb_handler_test) != handler\n");
      return -1;
    }

  ebb_handler_triggered = 0;

  paf_ebb_enable_branches ();

  asm ("li %0,%1\n"
       "li %2,%3\n"
       "li %4,%5\n"
       "li %6,%7\n"
       "li %8,%9\n"
       "li %10,%11\n"
       "li %12,%13\n"
       "li %14,%15\n"
       "li %16,%17\n"
       "li %18,%19\n"
       "li %20,%21\n"
       "li %22,%23\n"
       : : "r"(r3), "i"(EXPECTED_R3_VALUE),
           "r"(r4), "i"(EXPECTED_R4_VALUE),
           "r"(r5), "i"(EXPECTED_R5_VALUE),
           "r"(r6), "i"(EXPECTED_R6_VALUE),
           "r"(r7), "i"(EXPECTED_R7_VALUE),
           "r"(r8), "i"(EXPECTED_R8_VALUE),
           "r"(r15), "i"(EXPECTED_R15_VALUE),
           "r"(r16), "i"(EXPECTED_R16_VALUE),
           "r"(r18), "i"(EXPECTED_R18_VALUE),
           "r"(r21), "i"(EXPECTED_R21_VALUE),
           "r"(r25), "i"(EXPECTED_R25_VALUE),
           "r"(r30), "i"(EXPECTED_R30_VALUE));

  while (ebb_handler_triggered == 0);

  paf_ebb_disable_branches ();

  return (ebb_handler_triggered == 1);
}

static int
ebb_test_pmu_clobber (void)
{
  int ebbfd;
  int ret;

  ebbfd = paf_ebb_pmu_init (PM_RUN_INST_CMPL, -1);
  if (ebbfd == -1)
    {
      printf("Error: paf_ebb_init_pmu (PM_RUN_CYC, -1) failed "
	     "(errno = %i)\n", errno);
      return -1;
    }

  ret = ebb_test_pmu_grp_clobber ();

  close (ebbfd);

  return ret;
}

#define EBB_TEST ebb_test_pmu_clobber

#include "test_ebb-skeleton.c"
