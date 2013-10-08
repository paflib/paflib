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
static volatile int ebb_matched_regs = 0;

#define EXPECTED_R3_VALUE      2
#define EXPECTED_R4_VALUE      4
#define EXPECTED_R5_VALUE      8
#define EXPECTED_R6_VALUE     16
#define EXPECTED_R7_VALUE     32
#define EXPECTED_R8_VALUE     64
#define EXPECTED_R15_VALUE   128
#define EXPECTED_R16_VALUE   256
#define EXPECTED_R18_VALUE   512
#define EXPECTED_R21_VALUE  1024
#define EXPECTED_R25_VALUE  4096
#define EXPECTED_R30_VALUE  8192

#ifdef __powerpc64__
#define LD_INST         "ld"
#define CALLER_FRAME    112+112  /* EBB internal callback handler (ebb_hook)
                                    plus the callee stack frame.  */
#define REG_SIZE        8
#else /* __powerpc__ */
#define LD_INST         "lwz"
#define CALLER_FRAME    16+16
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


/* To avoid stack allocation by the function, it minimizes its local
   variables. If any stack allocation is done, the CALLER_FRAME should
   be adjusted.  */

static unsigned long int cr;
static unsigned long int lr;
static unsigned long int ctr;
static unsigned long int xer;

static void
attribute_noinline
ebb_handler_test_special_reg (void *context)
{
  int *trigger = (int*) (context);
  unsigned long int reg;

  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(CR_SAVE));
  *trigger += (reg == cr);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(LR_SAVE));
  *trigger += (reg == lr);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(CTR_SAVE));
  *trigger += (reg == ctr);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(XER_SAVE));
  *trigger += (reg == xer);
}

static int
ebb_test_pmu_special_reg_clobber ()
{
  ebbhandler_t handler;
  unsigned long int reg;

  printf ("%s: testing SPECIAL REG clobbering\n", __FUNCTION__);

  ebb_matched_regs = 0;

  handler = paf_ebb_register_handler (ebb_handler_test_special_reg,
				      (void*)&ebb_matched_regs,
				      PAF_EBB_CALLBACK_GPR_SAVE, 0);
  if (handler != ebb_handler_test_special_reg)
    {
      printf ("Error: paf_ebb_register_handler \
	      (ebb_handler_test_gpr) != handler\n");
      return -1;
    }

  ebb_matched_regs = 0;

  paf_ebb_enable_branches ();

  /* First save current values.  */
  asm volatile ("mfcr  %0\n" : "=r"(reg));
  cr = reg;
  asm volatile ("mflr  %0\n" : "=r"(reg));
  lr = reg;
  asm volatile ("mfctr %0\n" : "=r"(reg));
  ctr = reg;
  asm volatile ("mfxer %0\n" : "=r"(reg));
  xer = reg;

  while (ebb_matched_regs == 0);

  paf_ebb_disable_branches ();

  printf ("%s: ebb_matched_regs == %d\n", __FUNCTION__,
    ebb_matched_regs);
  return (ebb_matched_regs != 4);
}

static void
attribute_noinline
ebb_handler_test_gpr (void *context)
{
  int *trigger = (int*) (context);
  unsigned long int reg;

  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R3_DISP));
  *trigger += (reg == EXPECTED_R3_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R4_DISP));
  *trigger += (reg == EXPECTED_R4_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R5_DISP));
  *trigger += (reg == EXPECTED_R5_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R6_DISP));
  *trigger += (reg == EXPECTED_R6_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R7_DISP));
  *trigger += (reg == EXPECTED_R7_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R8_DISP));
  *trigger += (reg == EXPECTED_R8_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R15_DISP));
  *trigger += (reg == EXPECTED_R15_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R16_DISP));
  *trigger += (reg == EXPECTED_R16_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R18_DISP));
  *trigger += (reg == EXPECTED_R18_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R21_DISP));
  *trigger += (reg == EXPECTED_R21_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R25_DISP));
  *trigger += (reg == EXPECTED_R25_VALUE);
  asm (LD_INST " %0,%1(1)\n" : "=r"(reg) : "i"(R30_DISP));
  *trigger += (reg == EXPECTED_R30_VALUE);
}

static int
ebb_test_pmu_grp_clobber ()
{
  ebbhandler_t handler;

  printf ("%s: testing GRP clobbering\n", __FUNCTION__);
  ebb_matched_regs = 0;

  handler = paf_ebb_register_handler (ebb_handler_test_gpr,
				      (void*)&ebb_matched_regs,
				      PAF_EBB_CALLBACK_GPR_SAVE, 0);
  if (handler != ebb_handler_test_gpr)
    {
      printf ("Error: paf_ebb_register_handler \
	      (ebb_handler_test_gpr) != handler\n");
      return -1;
    }

  ebb_matched_regs = 0;

  paf_ebb_enable_branches ();

  asm ("li 3,%0\n"
       "li 4,%1\n"
       "li 5,%2\n"
       "li 6,%3\n"
       "li 7,%4\n"
       "li 8,%5\n"
       "li 15,%6\n"
       "li 16,%7\n"
       "li 18,%8\n"
       "li 21,%9\n"
       "li 25,%10\n"
       "li 30,%11\n"
       : : "i"(EXPECTED_R3_VALUE),
           "i"(EXPECTED_R4_VALUE),
           "i"(EXPECTED_R5_VALUE),
           "i"(EXPECTED_R6_VALUE),
           "i"(EXPECTED_R7_VALUE),
           "i"(EXPECTED_R8_VALUE),
           "i"(EXPECTED_R15_VALUE),
           "i"(EXPECTED_R16_VALUE),
           "i"(EXPECTED_R18_VALUE),
           "i"(EXPECTED_R21_VALUE),
           "i"(EXPECTED_R25_VALUE),
           "i"(EXPECTED_R30_VALUE)
       : "r3", "r4", "r5", "r6", "r7", "r8",
         "r15", "r16", "r18", "r21", "r25", "r30");

  while (ebb_matched_regs == 0);

  paf_ebb_disable_branches ();

  printf ("%s: ebb_matched_regs == %d\n", __FUNCTION__,
    ebb_matched_regs);

  return (ebb_matched_regs != 12);
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

  ret  = ebb_test_pmu_special_reg_clobber ();
  paf_ebb_pmu_reset ();
  ret += ebb_test_pmu_grp_clobber ();

  close (ebbfd);

  return ret;
}

#define EBB_TEST ebb_test_pmu_clobber

#include "test_ebb-skeleton.c"
