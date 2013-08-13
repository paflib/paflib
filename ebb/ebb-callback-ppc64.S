/*
 * Event-Based Branch Facility API.  PowerPC64 callback handler.
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
 *   --------------  __paf_ebb_callback_handler_vsr_[tcb|tls]  -----------------
 *   r1 + STACK_OFFSET+1088:1600 |  VSR VS1:VS32 (must be aligned to 16 bytes)
 *   --------------  __paf_ebb_callback_handler_vr_[tcb|tls]  ------------------
 *   r1 + STACK_OFFSET+1072      |  VSCR (32-bits)
 *   r1 + STACK_OFFSET+560:1072  |  VR  V1:V31 (must be aligned to 16 bytes)
 *   --------------  __paf_ebb_callback_handler_fpr_[tcb|tls]  -----------------
 *   r1 + STACK_OFFSET+296:552   |  FRP F1:F31
 *   r1 + STACK_OFFSET+288       |  FPSCR
 *   --------------  __paf_ebb_callback_handler_gpr_[tcb|tls]  -----------------
 *   r1 + STACK_OFFSET+24:264    |  GRP R0,R2:R31
 *   r1 + STACK_OFFSET+16        |  XER
 *   r1 + STACK_OFFSET+8         |  LR
 *   r1 + STACK_OFFSET           |  CCR
 */

#define ABI_GAP         288
#define CALLER_FRAME    112

#define CCR_SAVE        (CALLER_FRAME)
#define CCR_SIZE        (8)
#define LR_SAVE         (CCR_SAVE + CCR_SIZE)
#define LR_SIZE         (8)
#define XER_SAVE        (LR_SAVE + LR_SIZE)
#define XER_SIZE        (8)
#define GPR_SAVE(n)     (XER_SAVE + XER_SIZE + (8 * n))
#define GPR_SIZE        (30 * 8)
#define FPR_SAVE(n)     (GPR_SAVE(30) + (8 * n))
#define FPR_SIZE        (32 * 8)
#define FPSR_SAVE       (FPR_SAVE(32))
#define FPSR_SIZE       (8)
#define VR_SAVE         (FPSR_SAVE + FPSR_SIZE)
#define VR_SIZE         (32 * 16)
#define VSCR_SAVE       (VR_SAVE + VR_SIZE)
#define VSCR_SIZE       (4)
#define VSR_SAVE        (VSCR_SAVE + VSCR_SIZE)
#define VSR_SIZE        (32 * 16)

/* Save all General Purpose Register (GPR), but r1 (SP) and r13 (thr. id).  */
#define SAVE_GPR                                                   \
  std       r0, GPR_SAVE(0)(r1);                                   \
  std       r2, GPR_SAVE(1)(r1);                                   \
  std       r3, GPR_SAVE(2)(r1);                                   \
  std       r4, GPR_SAVE(3)(r1);                                   \
  std       r5, GPR_SAVE(4)(r1);                                   \
  std       r6, GPR_SAVE(5)(r1);                                   \
  std       r7, GPR_SAVE(6)(r1);                                   \
  std       r8, GPR_SAVE(7)(r1);                                   \
  std       r9, GPR_SAVE(8)(r1);                                   \
  std      r10, GPR_SAVE(9)(r1);                                   \
  std      r11, GPR_SAVE(10)(r1);                                  \
  std      r12, GPR_SAVE(11)(r1);                                  \
  std      r14, GPR_SAVE(12)(r1);                                  \
  std      r15, GPR_SAVE(13)(r1);                                  \
  std      r16, GPR_SAVE(14)(r1);                                  \
  std      r17, GPR_SAVE(15)(r1);                                  \
  std      r18, GPR_SAVE(16)(r1);                                  \
  std      r19, GPR_SAVE(17)(r1);                                  \
  std      r20, GPR_SAVE(18)(r1);                                  \
  std      r21, GPR_SAVE(19)(r1);                                  \
  std      r22, GPR_SAVE(20)(r1);                                  \
  std      r23, GPR_SAVE(21)(r1);                                  \
  std      r24, GPR_SAVE(22)(r1);                                  \
  std      r25, GPR_SAVE(23)(r1);                                  \
  std      r26, GPR_SAVE(24)(r1);                                  \
  std      r27, GPR_SAVE(25)(r1);                                  \
  std      r28, GPR_SAVE(26)(r1);                                  \
  std      r29, GPR_SAVE(27)(r1);                                  \
  std      r30, GPR_SAVE(28)(r1);                                  \
  std      r31, GPR_SAVE(29)(r1);

/* Restore GRP registers, but r1 (stack) and r13 (thread id).  */
#define RESTORE_GPR                                                \
  ld        r0, GPR_SAVE(0)(r1);                                   \
  ld        r2, GPR_SAVE(1)(r1);                                   \
  ld        r3, GPR_SAVE(2)(r1);                                   \
  ld        r4, GPR_SAVE(3)(r1);                                   \
  ld        r5, GPR_SAVE(4)(r1);                                   \
  ld        r6, GPR_SAVE(5)(r1);                                   \
  ld        r7, GPR_SAVE(6)(r1);                                   \
  ld        r8, GPR_SAVE(7)(r1);                                   \
  ld        r9, GPR_SAVE(8)(r1);                                   \
  ld       r10, GPR_SAVE(9)(r1);                                   \
  ld       r11, GPR_SAVE(10)(r1);                                  \
  ld       r12, GPR_SAVE(11)(r1);                                  \
  ld       r14, GPR_SAVE(12)(r1);                                  \
  ld       r15, GPR_SAVE(13)(r1);                                  \
  ld       r16, GPR_SAVE(14)(r1);                                  \
  ld       r17, GPR_SAVE(15)(r1);                                  \
  ld       r18, GPR_SAVE(16)(r1);                                  \
  ld       r19, GPR_SAVE(17)(r1);                                  \
  ld       r20, GPR_SAVE(18)(r1);                                  \
  ld       r21, GPR_SAVE(19)(r1);                                  \
  ld       r22, GPR_SAVE(20)(r1);                                  \
  ld       r23, GPR_SAVE(21)(r1);                                  \
  ld       r24, GPR_SAVE(22)(r1);                                  \
  ld       r25, GPR_SAVE(23)(r1);                                  \
  ld       r26, GPR_SAVE(24)(r1);                                  \
  ld       r27, GPR_SAVE(25)(r1);                                  \
  ld       r28, GPR_SAVE(26)(r1);                                  \
  ld       r29, GPR_SAVE(27)(r1);                                  \
  ld       r30, GPR_SAVE(28)(r1);                                  \
  ld       r31, GPR_SAVE(29)(r1);

/* Save status registers.  */
#define SAVE_SPEC_REG                                              \
  mflr     r0;                                                     \
  std      r0, LR_SAVE(r1);                                        \
  mfcr     r0;                                                     \
  std      r0, CCR_SAVE(r1);                                       \
  mfxer    r0;                                                     \
  std      r0, XER_SAVE(r1);

/* Restore state registers.  */
#define RESTORE_SPEC_REG                                           \
  ld       r0, LR_SAVE(r1);                                        \
  mtlr     r0;                                                     \
  ld       r0, CCR_SAVE(r1);                                       \
  mtcr     r0;                                                     \
  ld       r0, XER_SAVE(r1);                                       \
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
  addi     r9,r1,(16+VR_SAVE);                                     \
  rldicr   r3,r3,0,59;                                             \
  addi     r10,r9,16;                                              \
  stvx     v0,0,r10;                                               \
  stvx     v1,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v2,0,r10;                                               \
  stvx     v3,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v4,0,r10;                                               \
  stvx     v5,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v6,0,r10;                                               \
  stvx     v7,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v8,0,r10;                                               \
  stvx     v9,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v10,0,r10;                                              \
  stvx     v11,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v12,0,r10;                                              \
  stvx     v13,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v14,0,r10;                                              \
  stvx     v15,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v16,0,r10;                                              \
  stvx     v17,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v17,0,r10;                                              \
  stvx     v18,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v20,0,r10;                                              \
  stvx     v21,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v22,0,r10;                                              \
  stvx     v23,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v24,0,r10;                                              \
  stvx     v25,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v26,0,r10;                                              \
  stvx     v27,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v28,0,r10;                                              \
  stvx     v29,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stvx     v30,0,r10;                                              \
  stvx     v31,0,r9;                                               \
  addi     r10,r10,32;                                             \
  mfspr    r0,VRSAVE;                                              \
  stw      r0,0(r10);

/* Restore all VMX/Altivec registers, VR0-VR31.  */
#define RESTORE_VR                                                 \
  addi     r9,r1,(16+VR_SAVE);                                     \
  rldicr   r3,r3,0,59;                                             \
  addi     r10,r9,16;                                              \
  lvx      v0,0,r10;                                               \
  lvx      v1,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v2,0,r10;                                               \
  lvx      v3,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v4,0,r10;                                               \
  lvx      v5,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v6,0,r10;                                               \
  lvx      v7,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v8,0,r10;                                               \
  lvx      v9,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v10,0,r10;                                              \
  lvx      v11,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v12,0,r10;                                              \
  lvx      v13,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v14,0,r10;                                              \
  lvx      v15,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v16,0,r10;                                              \
  lvx      v17,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v17,0,r10;                                              \
  lvx      v18,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v20,0,r10;                                              \
  lvx      v21,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v22,0,r10;                                              \
  lvx      v23,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v24,0,r10;                                              \
  lvx      v25,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v26,0,r10;                                              \
  lvx      v27,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v28,0,r10;                                              \
  lvx      v29,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lvx      v30,0,r10;                                              \
  lvx      v31,0,r9;                                               \
  addi     r10,r10,32;                                             \
  lwz      r0,0(r10);                                              \
  mtspr    VRSAVE,r0;

/* Save all VSX registers, VSR0-VSR31. Setup r9 with the VR base stack
 * address align to 16-bytes and r10 as r9 plus 16.  */
#define SAVE_VSR                                                   \
  addi     r9,r1,(16+VSR_SAVE);                                    \
  rldicr   r3,r3,0,59;                                             \
  addi     r10,r9,16;                                              \
  stxvd2x  v0,0,r10;                                               \
  stxvd2x  v1,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v2,0,r10;                                               \
  stxvd2x  v3,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v4,0,r10;                                               \
  stxvd2x  v5,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v6,0,r10;                                               \
  stxvd2x  v7,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v8,0,r10;                                               \
  stxvd2x  v9,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v10,0,r10;                                              \
  stxvd2x  v11,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v12,0,r10;                                              \
  stxvd2x  v13,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v14,0,r10;                                              \
  stxvd2x  v15,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v16,0,r10;                                              \
  stxvd2x  v17,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v18,0,r10;                                              \
  stxvd2x  v19,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v20,0,r10;                                              \
  stxvd2x  v21,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v22,0,r10;                                              \
  stxvd2x  v23,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v24,0,r10;                                              \
  stxvd2x  v25,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v26,0,r10;                                              \
  stxvd2x  v27,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v28,0,r10;                                              \
  stxvd2x  v29,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  stxvd2x  v30,0,r10;                                              \
  stxvd2x  v31,0,r9;

/* Restore all VSR registers, VSR0-VSR31.  */
#define RESTORE_VSR                                                \
  addi     r9,r1,(16+VSR_SAVE);                                    \
  rldicr   r3,r3,0,59;                                             \
  addi     r10,r9,16;                                              \
  lxvd2x   v0,0,r10;                                               \
  lxvd2x   v1,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v2,0,r10;                                               \
  lxvd2x   v3,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v4,0,r10;                                               \
  lxvd2x   v5,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v6,0,r10;                                               \
  lxvd2x   v7,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v8,0,r10;                                               \
  lxvd2x   v9,0,r9;                                                \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v10,0,r10;                                              \
  lxvd2x   v11,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v12,0,r10;                                              \
  lxvd2x   v13,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v14,0,r10;                                              \
  lxvd2x   v15,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v16,0,r10;                                              \
  lxvd2x   v17,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v17,0,r10;                                              \
  lxvd2x   v18,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v20,0,r10;                                              \
  lxvd2x   v21,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v22,0,r10;                                              \
  lxvd2x   v23,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v24,0,r10;                                              \
  lxvd2x   v25,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v26,0,r10;                                              \
  lxvd2x   v27,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v28,0,r10;                                              \
  lxvd2x   v29,0,r9;                                               \
  addi     r10,r10,32;                                             \
  addi     r9,r9,32;                                               \
  lxvd2x   v30,0,r10;                                              \
  lxvd2x   v31,0,r9;

#define SETUP_TOC(__func)                                          \
  lis      r2,(__func)@highest;                                    \
  ori      r2,r2,(__func)@higher;                                  \
  rldicr   r2,r2,32,31;                                            \
  oris     r2,r2,(__func)@h;                                       \
  ori      r2,r2,(__func)@l;                                       \
  ld       r2,8(r2);

.hidden __paf_ebb_ebb_hook
#define CALL_EBB_HOOK                                              \
  bl       __paf_ebb_ebb_hook;                                      \
  nop


#define STACK_FRAME_GPR  (ABI_GAP + CALLER_FRAME + \
        CCR_SIZE + LR_SIZE + XER_SIZE + GPR_SIZE)

ENTRY(__paf_ebb_callback_handler_gpr)
  stdu  r1,-STACK_FRAME_GPR(r1)

  SAVE_GPR

  SAVE_SPEC_REG

  SETUP_TOC (__paf_ebb_callback_handler_gpr)
  CALL_EBB_HOOK

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_GPR

  RFEBB1
END(__paf_ebb_callback_handler_gpr)


#define STACK_FRAME_FPR  (STACK_FRAME_GPR + FPR_SIZE + FPSR_SIZE)

ENTRY(__paf_ebb_callback_handler_fpr)
  /* Reserve stack-frame size.  */
  stdu  r1,-STACK_FRAME_FPR(r1)

  SAVE_GPR

  SAVE_SPEC_REG

  SAVE_FPR

  SETUP_TOC (__paf_ebb_callback_handler_fpr)
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
  stdu  r1,-STACK_FRAME_VR(r1)

  SAVE_GPR

  SAVE_SPEC_REG

  SAVE_FPR

  SAVE_VR

  SETUP_TOC (__paf_ebb_callback_handler_vr)
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
  stdu  r1,-STACK_FRAME_VSR(r1)

  SAVE_GPR

  SAVE_SPEC_REG

  SAVE_FPR

  SAVE_VR

  SAVE_VSR

  SETUP_TOC (__paf_ebb_callback_handler_vsr)
  CALL_EBB_HOOK

  RESTORE_VSR

  RESTORE_VR

  RESTORE_FPR

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_VSR

  RFEBB1
END(__paf_ebb_callback_handler_vsr)
