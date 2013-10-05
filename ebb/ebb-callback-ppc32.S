/* Event-Based Branch Facility API.  PowerPC32 callback handler.
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

#include "ebb-sysdep.h"

/* The callback handler fetches the EBB function handler pointer and the
 * context from the TCB field inside GLIBC or from the per-thread variable
 * __paf_ebb_thread_info and calls the function pointer with provided context
 * as argument. Since it can be triggered in any time of program execution,
 * its currently state needs to be save.
 *
 * The stack frame layout is:
 *
 * STACK_OFFSET is 288 (default stack size) to not clobber the interrupted
 * function stack.
 *
 * Stack pointer (r1) offset     |  Contents
 *   --------------  __paf_ebb_callback_handler_vsr_[tcb|tls]  -----------------
 *   r1 + STACK_OFFSET+936:1448  |  VSR VS1:VS32 (must be aligned to 16 bytes)
 *   --------------  __paf_ebb_callback_handler_vr_[tcb|tls]  ------------------
 *   r1 + STACK_OFFSET+932       |  VSCR (32-bits)
 *   r1 + STACK_OFFSET+416:928   |  VR  V1:V31 (must be aligned to 16 bytes)
 *   --------------  __paf_ebb_callback_handler_fpr_[tcb|tls]  -----------------
 *   r1 + STACK_OFFSET+152:408   |  FRP F1:F31
 *   r1 + STACK_OFFSET+148       |  FPSCR
 *   --------------  __paf_ebb_callback_handler_gpr_[tcb|tls]  -----------------
 *   r1 + STACK_OFFSET+16:144    |  GRP r0,r3-r31
 *   r1 + STACK_OFFSET+12        |  XER
 *   r1 + STACK_OFFSET+8         |  CTR
 *   r1 + STACK_OFFSET+4         |  LR
 *   r1 + STACK_OFFSET           |  CCR
 */

#define CALLER_FRAME    16

#define CCR_SAVE        (CALLER_FRAME)
#define CCR_SIZE        (4)
#define LR_SAVE         (CCR_SAVE + CCR_SIZE)
#define LR_SIZE         (4)
#define XER_SAVE        (LR_SAVE + LR_SIZE)
#define XER_SIZE        (4)
#define GPR_SAVE(n)     (XER_SAVE + XER_SIZE + (4 * n))
#define GPR_SIZE        (30 * 4)
#define FPR_SAVE(n)     (GPR_SAVE(30) + GPR_SIZE + (4 * n))
#define FPR_SIZE        (32 * 8)
#define FPSR_SAVE       (FPR_SAVE(32))
#define FPSR_SIZE       (4)
#define VR_SAVE         (FPSR_SAVE + FPSR_SIZE)
#define VR_SIZE         (32 * 16)
#define VSCR_SAVE       (VR_SAVE + VR_SIZE)
#define VSCR_SIZE       (4)
#define VSR_SAVE        (VSCR_SAVE + VSCR_SIZE)
#define VSR_SIZE        (32 * 16)

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
  mflr     r0;                                                     \
  stw      r0, LR_SAVE(r1);                                        \
  mfcr     r0;                                                     \
  stw      r0, CCR_SAVE(r1);                                       \
  mfxer    r0;                                                     \
  stw      r0, XER_SAVE(r1);

/* Restore state registers.  */
#define RESTORE_SPEC_REG                                           \
  lwz      r0, LR_SAVE(r1);                                        \
  mtlr     r0;                                                     \
  lwz      r0, CCR_SAVE(r1);                                       \
  mtcr     r0;                                                     \
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

/* Save all VMX/Altive registers, VR0-VR31. Setup r9 with the VR base stack
 * address align to 16-bytes and r10 as r9 plus 16.  */
#define SAVE_VR                                                    \
  addi     r9,r1,(15+VR_SAVE);                                     \
  rlwinm   r9,r9,0,0,27;                                           \
  addi     r10,r9,16;                                              \
  stvx     v0,0,r9;                                                \
  stvx     v1,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v2,0,r9;                                                \
  stvx     v3,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v4,0,r9;                                                \
  stvx     v5,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v6,0,r9;                                                \
  stvx     v7,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v8,0,r9;                                                \
  stvx     v9,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v10,0,r9;                                               \
  stvx     v11,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v12,0,r9;                                               \
  stvx     v13,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v14,0,r9;                                               \
  stvx     v15,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v16,0,r9;                                               \
  stvx     v17,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v17,0,r9;                                               \
  stvx     v18,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v20,0,r9;                                               \
  stvx     v21,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v22,0,r9;                                               \
  stvx     v23,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v24,0,r9;                                               \
  stvx     v25,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v26,0,r9;                                               \
  stvx     v27,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v28,0,r9;                                               \
  stvx     v29,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stvx     v30,0,r9;                                               \
  stvx     v31,0,r10;                                              \
  addi     r10,r10,32;                                             \
  mfspr    r0,VRSAVE;                                              \
  stw      r0,0(r10);

/* Restore all VMX/Altivec registers, VR0-VR31.  */
#define RESTORE_VR                                                 \
  addi     r9,r1,(15+VR_SAVE);                                     \
  rlwinm   r9,r9,0,0,27;                                           \
  addi     r10,r9,16;                                              \
  lvx      v0,0,r9;                                                \
  lvx      v1,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v2,0,r9;                                                \
  lvx      v3,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v4,0,r9;                                                \
  lvx      v5,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v6,0,r9;                                                \
  lvx      v7,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v8,0,r9;                                                \
  lvx      v9,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v10,0,r9;                                               \
  lvx      v11,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v12,0,r9;                                               \
  lvx      v13,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v14,0,r9;                                               \
  lvx      v15,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v16,0,r9;                                               \
  lvx      v17,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v17,0,r9;                                               \
  lvx      v18,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v20,0,r9;                                               \
  lvx      v21,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v22,0,r9;                                               \
  lvx      v23,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v24,0,r9;                                               \
  lvx      v25,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v26,0,r9;                                               \
  lvx      v27,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v28,0,r9;                                               \
  lvx      v29,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lvx      v30,0,r9;                                               \
  lvx      v31,0,r10;                                              \
  addi     r10,r10,32;                                             \
  lwz      r0,0(r10);                                              \
  mtspr    VRSAVE,r0;

/* Save all VSX registers, VSR0-VSR31. Setup r9 with the VR base stack
 * address align to 16-bytes and r10 as r9 plus 16.  */
#define SAVE_VSR                                                   \
  addi     r9,r1,(15+VSR_SAVE);                                    \
  rlwinm   r9,r9,0,0,27;                                           \
  addi     r10,r9,16;                                              \
  stxvd2x  v0,0,r9;                                                \
  stxvd2x  v1,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v2,0,r9;                                                \
  stxvd2x  v3,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v4,0,r9;                                                \
  stxvd2x  v5,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v6,0,r9;                                                \
  stxvd2x  v7,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v8,0,r9;                                                \
  stxvd2x  v9,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v10,0,r9;                                               \
  stxvd2x  v11,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v12,0,r9;                                               \
  stxvd2x  v13,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v14,0,r9;                                               \
  stxvd2x  v15,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v16,0,r9;                                               \
  stxvd2x  v17,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v18,0,r9;                                               \
  stxvd2x  v19,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v20,0,r9;                                               \
  stxvd2x  v21,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v22,0,r9;                                               \
  stxvd2x  v23,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v24,0,r9;                                               \
  stxvd2x  v25,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v26,0,r9;                                               \
  stxvd2x  v27,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v28,0,r9;                                               \
  stxvd2x  v29,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  stxvd2x  v30,0,r9;                                               \
  stxvd2x  v31,0,r11;

/* Restore all VSR registers, VSR0-VSR31.  */
#define RESTORE_VSR                                                \
  addi     r9,r1,(15+VSR_SAVE);                                    \
  rlwinm   r9,r9,0,0,27;                                           \
  addi     r10,r9,16;                                              \
  lxvd2x   v0,0,r9;                                                \
  lxvd2x   v1,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v2,0,r9;                                                \
  lxvd2x   v3,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v4,0,r9;                                                \
  lxvd2x   v5,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v6,0,r9;                                                \
  lxvd2x   v7,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v8,0,r9;                                                \
  lxvd2x   v9,0,r10;                                               \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v10,0,r9;                                               \
  lxvd2x   v11,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v12,0,r9;                                               \
  lxvd2x   v13,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v14,0,r9;                                               \
  lxvd2x   v15,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v16,0,r9;                                               \
  lxvd2x   v17,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v18,0,r9;                                               \
  lxvd2x   v19,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v20,0,r9;                                               \
  lxvd2x   v21,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v22,0,r9;                                               \
  lxvd2x   v23,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v24,0,r9;                                               \
  lxvd2x   v25,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v26,0,r9;                                               \
  lxvd2x   v27,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v28,0,r9;                                               \
  lxvd2x   v29,0,r10;                                              \
  addi     r9,r9,32;                                               \
  addi     r10,r10,32;                                             \
  lxvd2x   v30,0,r9;                                               \
  lxvd2x   v31,0,r11;

.hidden __paf_ebb_ebb_hook
#define CALL_EBB_HOOK                                              \
  bl       __paf_ebb_ebb_hook;


#define STACK_FRAME_GPR  (CALLER_FRAME + CCR_SIZE + LR_SIZE + XER_SIZE + GPR_SIZE)

ENTRY(__paf_ebb_callback_handler_gpr)
  stwu  r1,-STACK_FRAME_GPR(r1)

  SAVE_GPR

  SAVE_SPEC_REG

  CALL_EBB_HOOK

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_GPR

  RFEBB1
END(__paf_ebb_callback_handler_gpr)


#define STACK_FRAME_FPR  (STACK_FRAME_GPR + FPR_SIZE + FPSR_SIZE)

ENTRY(__paf_ebb_callback_handler_fpr)
  stwu  r1,-STACK_FRAME_FPR(r1)

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


/* Add extra 16 in case the stack + VR_SAVE is not 16-bytes aligned.  */
#define STACK_FRAME_VR  (STACK_FRAME_FPR + VR_SIZE + 16 + VSCR_SIZE)

  .machine "altivec"
ENTRY(__paf_ebb_callback_handler_vr)
  /* Reserve stack-frame size.  */
  stwu  r1,-STACK_FRAME_VR(r1)

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


/* Add extra 16 in case the stack + VSR_SAVE is not 16-bytes aligned.  */
#define STACK_FRAME_VSR  (STACK_FRAME_VR + VSR_SIZE + 16)

  .machine "power7"
ENTRY(__paf_ebb_callback_handler_vsr)
  /* Reserve stack-frame size.  */
  stwu  r1,-STACK_FRAME_VSR(r1)

  SAVE_GPR

  SAVE_SPEC_REG

  SAVE_FPR

  SAVE_VR

  SAVE_VSR

  CALL_EBB_HOOK

  RESTORE_VSR

  RESTORE_VR

  RESTORE_FPR

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_VSR

  RFEBB1
END(__paf_ebb_callback_handler_vsr)
