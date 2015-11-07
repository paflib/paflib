/* Event-Based Branch Facility API.  PowerPC32 callback handler.
 *
 * Copyright IBM Corp. 2013,2015
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

#include "ebb-sysdep.h"

/* The callback handler fetches the EBB function handler pointer and the
 * context from the TCB field inside GLIBC or from the per-thread variable
 * __paf_ebb_thread_info and calls the function pointer with provided context
 * as argument. Since it can be triggered in any time of program execution,
 * its currently state needs to be save.
 *
 * The stack frame layout is:
 *
 * Stack pointer (r1) offset     |  Contents
 *   --------------  __paf_ebb_callback_handler_gpr_[tcb|tls]  -----------------
 *   r1 + 152:160   |  Alignment padding
 *   r1 + 32:152    |  GRP r0,r3-r31
 *   r1 + 28        |  XER
 *   r1 + 24        |  CTR
 *   r1 + 20        |  LR
 *   r1 + 16        |  CR
 *
 *   --------------  __paf_ebb_callback_handler_fpr_[tcb|tls]  -----------------
 *   r1 + 408       |  FPSCR (64b store)
 *   r1 + 152:408   |  FRP f0:f31
 *   r1 + 32:152    |  GRP r0,r3-r31
 *   r1 + 28        |  XER
 *   r1 + 24        |  CTR
 *   r1 + 20        |  LR
 *   r1 + 16        |  CR
 *
 *   --------------  __paf_ebb_callback_handler_vr_[tcb|tls]  ------------------
 *   r1 + 936:944   |  Alignment padding
 *   r1 + 932       |  VRSAVE
 *   r1 + 928       |  VSCR
 *   r1 + 416:928   |  VR  V0:V31
 *   r1 + 408       |  FPSCR (64b store)
 *   r1 + 152:408   |  FRP f0:f31
 *   r1 + 32:152    |  GRP r0,r3-r31
 *   r1 + 28        |  XER
 *   r1 + 24        |  CTR
 *   r1 + 20        |  LR
 *   r1 + 16        |  CR
 *
 *   --------------  __paf_ebb_callback_handler_vsr_[tcb|tls]  -----------------
 *   r1 + 1192:1200 |  Alignment padding
 *   r1 + 1188      |  VRSAVE
 *   r1 + 1184      |  VSCR (Immediately after VSR63 for qw align)
 *   r1 + 160:1184  |  VSR0-63
 *   r1 + 152       |  FPSCR
 *   r1 + 32:152    |  GRP r0,r3-r31
 *   r1 + 28        |  XER
 *   r1 + 24        |  CTR
 *   r1 + 20        |  LR
 *   r1 + 16        |  CR
 */

#define CALLER_FRAME    16

#define CR_SAVE         (CALLER_FRAME)
#define CR_SIZE         (4)
#define LR_SAVE         (CR_SAVE + CR_SIZE)
#define LR_SIZE         (4)
#define CTR_SAVE        (LR_SAVE + LR_SIZE)
#define CTR_SIZE        (4)
#define XER_SAVE        (CTR_SAVE + CTR_SIZE)
#define XER_SIZE        (4)
#define GPR_SAVE(n)     (XER_SAVE + XER_SIZE + (4 * n))
#define GPR_SIZE        (30 * 4)
#define STACK_FRAME_GPR (GPR_SAVE(30) + 8)

#define FPR_SAVE(n)     (GPR_SAVE(30) + (8 * n))
#define FPR_SIZE        (32 * 8)
#define FPSR_SAVE       (FPR_SAVE(32))
#define FPSR_SIZE       (8)
#define STACK_FRAME_FPR  (FPSR_SAVE + FPSR_SIZE)

#define VR_SAVE(n)      (FPSR_SAVE + FPSR_SIZE + (16 * n))
#define VSCR_SIZE       (4)
#define VSCR_SAVE       (VR_SAVE(32))
#define VRSAVE_SIZE     (4)
#define VRSAVE_SAVE     (VSCR_SAVE + VSCR_SIZE)
#define STACK_FRAME_VR  (VRSAVE_SAVE + VRSAVE_SIZE + 8)

/* VSR registers overlap both FPR and VR registers. We take advantage of that
   to reduce our burgeoning stack footprint.  */
#define VSR_SIZE        (64 * 16)
#define VSR_FPSR        (GPR_SAVE(30))
#define VSR_SAVE(n)     (VSR_FPSR + FPSR_SIZE + (16 * n))
#define VSR_VSCR        (VSR_SAVE(64))
#define VSR_VRSAVE      (VSR_VSCR + VSCR_SIZE)
#define STACK_FRAME_VSR (VSR_VRSAVE + VRSAVE_SIZE + 8)

/* Save all General Purpose Register (GPR), but r1 (SP) and r2 (thr. id).  */
#define SAVE_GPR                                                   \
  stw       r0, GPR_SAVE(0)(r1);                                   \
  stw       r3, GPR_SAVE(1)(r1);                                   \
  stw       r4, GPR_SAVE(2)(r1);                                   \
  stw       r5, GPR_SAVE(3)(r1);                                   \
  stw       r6, GPR_SAVE(4)(r1);                                   \
  stw       r7, GPR_SAVE(5)(r1);                                   \
  stw       r8, GPR_SAVE(6)(r1);                                   \
  stw       r9, GPR_SAVE(7)(r1);                                   \
  stw      r10, GPR_SAVE(8)(r1);                                   \
  stw      r11, GPR_SAVE(9)(r1);                                   \
  stw      r12, GPR_SAVE(10)(r1);                                  \
  stw      r13, GPR_SAVE(11)(r1);                                  \
  stw      r14, GPR_SAVE(12)(r1);                                  \
  stw      r15, GPR_SAVE(13)(r1);                                  \
  stw      r16, GPR_SAVE(14)(r1);                                  \
  stw      r17, GPR_SAVE(15)(r1);                                  \
  stw      r18, GPR_SAVE(16)(r1);                                  \
  stw      r19, GPR_SAVE(17)(r1);                                  \
  stw      r20, GPR_SAVE(18)(r1);                                  \
  stw      r21, GPR_SAVE(19)(r1);                                  \
  stw      r22, GPR_SAVE(20)(r1);                                  \
  stw      r23, GPR_SAVE(21)(r1);                                  \
  stw      r24, GPR_SAVE(22)(r1);                                  \
  stw      r25, GPR_SAVE(23)(r1);                                  \
  stw      r26, GPR_SAVE(24)(r1);                                  \
  stw      r27, GPR_SAVE(25)(r1);                                  \
  stw      r28, GPR_SAVE(26)(r1);                                  \
  stw      r29, GPR_SAVE(27)(r1);                                  \
  stw      r30, GPR_SAVE(28)(r1);                                  \
  stw      r31, GPR_SAVE(29)(r1);

/* Restore GRP registers, but r1 (stack) and r2 (thread id).  */
#define RESTORE_GPR                                                \
  lwz       r0, GPR_SAVE(0)(r1);                                   \
  lwz       r3, GPR_SAVE(1)(r1);                                   \
  lwz       r4, GPR_SAVE(2)(r1);                                   \
  lwz       r5, GPR_SAVE(3)(r1);                                   \
  lwz       r6, GPR_SAVE(4)(r1);                                   \
  lwz       r7, GPR_SAVE(5)(r1);                                   \
  lwz       r8, GPR_SAVE(6)(r1);                                   \
  lwz       r9, GPR_SAVE(7)(r1);                                   \
  lwz      r10, GPR_SAVE(8)(r1);                                   \
  lwz      r11, GPR_SAVE(9)(r1);                                   \
  lwz      r12, GPR_SAVE(10)(r1);                                  \
  lwz      r13, GPR_SAVE(11)(r1);                                  \
  lwz      r14, GPR_SAVE(12)(r1);                                  \
  lwz      r15, GPR_SAVE(13)(r1);                                  \
  lwz      r16, GPR_SAVE(14)(r1);                                  \
  lwz      r17, GPR_SAVE(15)(r1);                                  \
  lwz      r18, GPR_SAVE(16)(r1);                                  \
  lwz      r19, GPR_SAVE(17)(r1);                                  \
  lwz      r20, GPR_SAVE(18)(r1);                                  \
  lwz      r21, GPR_SAVE(19)(r1);                                  \
  lwz      r22, GPR_SAVE(20)(r1);                                  \
  lwz      r23, GPR_SAVE(21)(r1);                                  \
  lwz      r24, GPR_SAVE(22)(r1);                                  \
  lwz      r25, GPR_SAVE(23)(r1);                                  \
  lwz      r26, GPR_SAVE(24)(r1);                                  \
  lwz      r27, GPR_SAVE(25)(r1);                                  \
  lwz      r28, GPR_SAVE(26)(r1);                                  \
  lwz      r29, GPR_SAVE(27)(r1);                                  \
  lwz      r30, GPR_SAVE(28)(r1);                                  \
  lwz      r31, GPR_SAVE(29)(r1);

/* Save status registers.  */
#define SAVE_SPEC_REG                                              \
  mfcr     r0;                                                     \
  stw      r0, CR_SAVE(r1);                                        \
  mflr     r0;                                                     \
  stw      r0, LR_SAVE(r1);                                        \
  mfctr    r0;                                                     \
  stw      r0, CTR_SAVE(r1);                                       \
  mfxer    r0;                                                     \
  stw      r0, XER_SAVE(r1);

/* Restore state registers.  */
#define RESTORE_SPEC_REG                                           \
  lwz      r0, CR_SAVE(r1);                                        \
  mtcr     r0;                                                     \
  lwz      r0, LR_SAVE(r1);                                        \
  mtlr     r0;                                                     \
  lwz      r0, CTR_SAVE(r1);                                       \
  mtctr    r0;                                                     \
  lwz      r0, XER_SAVE(r1);                                       \
  mtxer    r0;

/* Save all Floating-Point Registers (FPR).  */
#define SAVE_FPR      \
  stfd      f0, FPR_SAVE(0)(r1);                                   \
  stfd      f1, FPR_SAVE(1)(r1);                                   \
  stfd      f2, FPR_SAVE(2)(r1);                                   \
  stfd      f3, FPR_SAVE(3)(r1);                                   \
  stfd      f4, FPR_SAVE(4)(r1);                                   \
  stfd      f5, FPR_SAVE(5)(r1);                                   \
  stfd      f6, FPR_SAVE(6)(r1);                                   \
  stfd      f7, FPR_SAVE(7)(r1);                                   \
  stfd      f8, FPR_SAVE(8)(r1);                                   \
  stfd      f9, FPR_SAVE(9)(r1);                                   \
  stfd     f10, FPR_SAVE(10)(r1);                                  \
  stfd     f11, FPR_SAVE(11)(r1);                                  \
  stfd     f12, FPR_SAVE(12)(r1);                                  \
  stfd     f13, FPR_SAVE(13)(r1);                                  \
  stfd     f14, FPR_SAVE(14)(r1);                                  \
  stfd     f15, FPR_SAVE(15)(r1);                                  \
  stfd     f16, FPR_SAVE(16)(r1);                                  \
  stfd     f17, FPR_SAVE(17)(r1);                                  \
  stfd     f18, FPR_SAVE(18)(r1);                                  \
  stfd     f19, FPR_SAVE(19)(r1);                                  \
  stfd     f20, FPR_SAVE(20)(r1);                                  \
  stfd     f21, FPR_SAVE(21)(r1);                                  \
  stfd     f22, FPR_SAVE(22)(r1);                                  \
  stfd     f23, FPR_SAVE(23)(r1);                                  \
  stfd     f24, FPR_SAVE(24)(r1);                                  \
  stfd     f25, FPR_SAVE(25)(r1);                                  \
  stfd     f26, FPR_SAVE(26)(r1);                                  \
  stfd     f27, FPR_SAVE(27)(r1);                                  \
  stfd     f28, FPR_SAVE(28)(r1);                                  \
  stfd     f29, FPR_SAVE(29)(r1);                                  \
  stfd     f30, FPR_SAVE(30)(r1);                                  \
  stfd     f31, FPR_SAVE(31)(r1);                                  \
  mffs      f0;                                                    \
  stfd      f0, FPSR_SAVE(r1)

/* Restore all Floating-Point Registers (FPR).  */
#define RESTORE_FPR                                                \
  lfd       f0, FPSR_SAVE(r1);                                     \
  mtfsf     0xff,f0;                                               \
  lfd       f0, FPR_SAVE(0)(r1);                                   \
  lfd       f1, FPR_SAVE(1)(r1);                                   \
  lfd       f2, FPR_SAVE(2)(r1);                                   \
  lfd       f3, FPR_SAVE(3)(r1);                                   \
  lfd       f4, FPR_SAVE(4)(r1);                                   \
  lfd       f5, FPR_SAVE(5)(r1);                                   \
  lfd       f6, FPR_SAVE(6)(r1);                                   \
  lfd       f7, FPR_SAVE(7)(r1);                                   \
  lfd       f8, FPR_SAVE(8)(r1);                                   \
  lfd       f9, FPR_SAVE(9)(r1);                                   \
  lfd      f10, FPR_SAVE(10)(r1);                                  \
  lfd      f11, FPR_SAVE(11)(r1);                                  \
  lfd      f12, FPR_SAVE(12)(r1);                                  \
  lfd      f13, FPR_SAVE(13)(r1);                                  \
  lfd      f14, FPR_SAVE(14)(r1);                                  \
  lfd      f15, FPR_SAVE(15)(r1);                                  \
  lfd      f16, FPR_SAVE(16)(r1);                                  \
  lfd      f17, FPR_SAVE(17)(r1);                                  \
  lfd      f18, FPR_SAVE(18)(r1);                                  \
  lfd      f19, FPR_SAVE(19)(r1);                                  \
  lfd      f20, FPR_SAVE(20)(r1);                                  \
  lfd      f21, FPR_SAVE(21)(r1);                                  \
  lfd      f22, FPR_SAVE(22)(r1);                                  \
  lfd      f23, FPR_SAVE(23)(r1);                                  \
  lfd      f24, FPR_SAVE(24)(r1);                                  \
  lfd      f25, FPR_SAVE(25)(r1);                                  \
  lfd      f26, FPR_SAVE(26)(r1);                                  \
  lfd      f27, FPR_SAVE(27)(r1);                                  \
  lfd      f28, FPR_SAVE(28)(r1);                                  \
  lfd      f29, FPR_SAVE(29)(r1);                                  \
  lfd      f30, FPR_SAVE(30)(r1);                                  \
  lfd      f31, FPR_SAVE(31)(r1);

#define SAVE_VR_REGS(x,y) \
  addi r9,r1,VR_SAVE(x);  \
  addi r10,r1,VR_SAVE(y); \
  stvx x,0,r9;            \
  stvx y,0,r10;

/* Save all VMX/Altive registers, VR0-VR31.  */
#define SAVE_VR        \
  SAVE_VR_REGS(0,1);   \
  SAVE_VR_REGS(2,3);   \
  SAVE_VR_REGS(4,5);   \
  SAVE_VR_REGS(6,7);   \
  SAVE_VR_REGS(8,9);   \
  SAVE_VR_REGS(10,11); \
  SAVE_VR_REGS(12,13); \
  SAVE_VR_REGS(14,15); \
  SAVE_VR_REGS(16,17); \
  SAVE_VR_REGS(18,19); \
  SAVE_VR_REGS(20,21); \
  SAVE_VR_REGS(22,23); \
  SAVE_VR_REGS(24,25); \
  SAVE_VR_REGS(26,27); \
  SAVE_VR_REGS(28,29); \
  SAVE_VR_REGS(30,31); \
  addi     r9,r1,VSCR_SAVE;    \
  mfspr    r0,VRSAVE;          \
  stw      r0,VRSAVE_SAVE(r1); \
  mfvscr   v0;                 \
  vspltw   v0,v0,3;            \
  stvewx   v0,0,r9;

#define RESTORE_VR_REGS(x,y) \
  addi r9,r1,VR_SAVE(x);     \
  addi r10,r1,VR_SAVE(y);    \
  lvx x,0,r9;                \
  lvx y,0,r10;

/* Restore all VMX/Altivec registers, VR0-VR31 VSCR VRSAVE. r8 is
   dirtied to restore VSCR prior to restoring v0.  */
#define RESTORE_VR             \
  addi     r9,r1,VSCR_SAVE;   \
  lvewx    v0,0,r9;            \
  vspltw   v0,v0,0;            \
  mtvscr   v0;                 \
  lwz      r0,VRSAVE_SAVE(r1); \
  mtspr    VRSAVE,r0;          \
  RESTORE_VR_REGS(0,1);   \
  RESTORE_VR_REGS(2,3);   \
  RESTORE_VR_REGS(4,5);   \
  RESTORE_VR_REGS(6,7);   \
  RESTORE_VR_REGS(8,9);   \
  RESTORE_VR_REGS(10,11); \
  RESTORE_VR_REGS(12,13); \
  RESTORE_VR_REGS(14,15); \
  RESTORE_VR_REGS(16,17); \
  RESTORE_VR_REGS(18,19); \
  RESTORE_VR_REGS(20,21); \
  RESTORE_VR_REGS(22,23); \
  RESTORE_VR_REGS(24,25); \
  RESTORE_VR_REGS(26,27); \
  RESTORE_VR_REGS(28,29); \
  RESTORE_VR_REGS(30,31);

/* Save a pair of VSR registers to the stack.  */
#define SAVE_VSR_REGS(x,y)  \
  addi r9,r1,VSR_SAVE(x);  \
  addi r10,r1,VSR_SAVE(y); \
  stxvd2x x,0,r9;          \
  stxvd2x y,0,r10;

/* Save all VSX registers, VSR0-VSR63. FSCR, VRSAVE, and VSCR.  */
#define SAVE_VSR        \
  SAVE_VSR_REGS(0,1);   \
  SAVE_VSR_REGS(2,3);   \
  SAVE_VSR_REGS(4,5);   \
  SAVE_VSR_REGS(6,7);   \
  SAVE_VSR_REGS(8,9);   \
  SAVE_VSR_REGS(10,11); \
  SAVE_VSR_REGS(12,13); \
  SAVE_VSR_REGS(14,15); \
  SAVE_VSR_REGS(16,17); \
  SAVE_VSR_REGS(18,19); \
  SAVE_VSR_REGS(20,21); \
  SAVE_VSR_REGS(22,23); \
  SAVE_VSR_REGS(24,25); \
  SAVE_VSR_REGS(26,27); \
  SAVE_VSR_REGS(28,29); \
  SAVE_VSR_REGS(30,31); \
  SAVE_VSR_REGS(32,33); \
  SAVE_VSR_REGS(34,35); \
  SAVE_VSR_REGS(36,37); \
  SAVE_VSR_REGS(38,39); \
  SAVE_VSR_REGS(40,41); \
  SAVE_VSR_REGS(42,43); \
  SAVE_VSR_REGS(44,45); \
  SAVE_VSR_REGS(46,47); \
  SAVE_VSR_REGS(48,49); \
  SAVE_VSR_REGS(50,51); \
  SAVE_VSR_REGS(52,53); \
  SAVE_VSR_REGS(54,55); \
  SAVE_VSR_REGS(56,57); \
  SAVE_VSR_REGS(58,59); \
  SAVE_VSR_REGS(60,61); \
  SAVE_VSR_REGS(62,63); \
  addi     r9,r1,VSR_VSCR;     \
  mfvscr   v0;                 \
  vspltw   v0,v0,3;            \
  stvewx   v0,0,r9;            \
  mfspr    r0,VRSAVE;          \
  stw      r0,VSR_VRSAVE(r1);  \
  mffs     f0;                 \
  stfd     f0,VSR_FPSR(r1);


/* Reload an adjacent pair of VSR registers.  */
#define RESTORE_VSR_REGS(x,y) \
  addi r9,r1,VSR_SAVE(x);     \
  addi r10,r1,VSR_SAVE(y);    \
  lxvd2x x,0,r9;              \
  lxvd2x y,0,r10;

/* Restore all VSR registers, VSR0-VSR63, VRSAVE, VSCR, FPSR.  */
#define RESTORE_VSR             \
  lwz      r0,(VSR_VRSAVE)(r1); \
  mtspr    VRSAVE,r0;           \
  lfd      f0,(VSR_FPSR)(r1);   \
  mtfsf    0xff,f0;             \
  addi     r9,r1,VSR_VSCR;      \
  lvewx    v0,0,r9;             \
  vspltw   v0,v0,0;             \
  mtvscr   v0;                  \
  RESTORE_VSR_REGS(0,1);   \
  RESTORE_VSR_REGS(2,3);   \
  RESTORE_VSR_REGS(4,5);   \
  RESTORE_VSR_REGS(6,7);   \
  RESTORE_VSR_REGS(8,9);   \
  RESTORE_VSR_REGS(10,11); \
  RESTORE_VSR_REGS(12,13); \
  RESTORE_VSR_REGS(14,15); \
  RESTORE_VSR_REGS(16,17); \
  RESTORE_VSR_REGS(18,19); \
  RESTORE_VSR_REGS(20,21); \
  RESTORE_VSR_REGS(22,23); \
  RESTORE_VSR_REGS(24,25); \
  RESTORE_VSR_REGS(26,27); \
  RESTORE_VSR_REGS(28,29); \
  RESTORE_VSR_REGS(30,31); \
  RESTORE_VSR_REGS(32,33); \
  RESTORE_VSR_REGS(34,35); \
  RESTORE_VSR_REGS(36,37); \
  RESTORE_VSR_REGS(38,39); \
  RESTORE_VSR_REGS(40,41); \
  RESTORE_VSR_REGS(42,43); \
  RESTORE_VSR_REGS(44,45); \
  RESTORE_VSR_REGS(46,47); \
  RESTORE_VSR_REGS(48,49); \
  RESTORE_VSR_REGS(50,51); \
  RESTORE_VSR_REGS(52,53); \
  RESTORE_VSR_REGS(54,55); \
  RESTORE_VSR_REGS(56,57); \
  RESTORE_VSR_REGS(58,59); \
  RESTORE_VSR_REGS(60,61); \
  RESTORE_VSR_REGS(62,63);


.hidden __paf_ebb_ebb_hook
#define CALL_EBB_HOOK                                              \
  bl       __paf_ebb_ebb_hook;


ENTRY(__paf_ebb_callback_handler_gpr)
  stwu  r1,-STACK_FRAME_GPR(r1)
  .cfi_adjust_cfa_offset STACK_FRAME_GPR

  SAVE_GPR

  SAVE_SPEC_REG

  CALL_EBB_HOOK

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_GPR

  RFEBB1
END(__paf_ebb_callback_handler_gpr)


ENTRY(__paf_ebb_callback_handler_fpr)
  stwu  r1,-STACK_FRAME_FPR(r1)
  .cfi_adjust_cfa_offset STACK_FRAME_FPR

  SAVE_GPR

  SAVE_SPEC_REG

  SAVE_FPR

  CALL_EBB_HOOK

  RESTORE_FPR

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_FPR

  RFEBB1
END(__paf_ebb_callback_handler_fpr)


  .machine "altivec"
ENTRY(__paf_ebb_callback_handler_vr)
  /* Reserve stack-frame size.  */
  stwu  r1,-STACK_FRAME_VR(r1)
  .cfi_adjust_cfa_offset STACK_FRAME_VR

  SAVE_GPR

  SAVE_SPEC_REG

  SAVE_FPR

  SAVE_VR

  CALL_EBB_HOOK

  RESTORE_VR

  RESTORE_FPR

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_VR

  RFEBB1
END(__paf_ebb_callback_handler_vr)


  .machine "power7"
ENTRY(__paf_ebb_callback_handler_vsr)
  /* Reserve stack-frame size.  */
  stwu  r1,-STACK_FRAME_VSR(r1)
  .cfi_adjust_cfa_offset STACK_FRAME_VSR

  SAVE_GPR

  SAVE_SPEC_REG

  SAVE_VSR

  CALL_EBB_HOOK

  RESTORE_VSR

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_VSR

  RFEBB1
END(__paf_ebb_callback_handler_vsr)
