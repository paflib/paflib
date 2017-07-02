/* Event-Based Branch Facility API.  PowerPC{32,64,64le} callback handler.
 *
 * Copyright IBM Corp. 2013-2015
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
 *   SP Offsets GPR
 *   ppc32    | ppc64   | ppc64le |  Contents
 *   ---------+---------+---------+----------
 *   152:160  | 384:672 | 304:816 | Alignment padding / Reserved space
 *   32:152   | 144:384 | 64:304  | GPRS
 *   28       | 136     | 56      | XER
 *   24       | 128     | 48      | CTR
 *   20       | 120     | 40      | LR
 *   16       | 112     | 32      | CR
 *
 *   SP Offsets FPR
 *   ppc32    | ppc64    | ppc64le  |  Contents
 *   ---------+----------+----------+----------
 *   408:416  | 648:944 | 568:1088 | Alignment Padding / Reserved space
 *   408      | 640     | 560      | FPSCR (64b store)
 *   152:408  | 384:640 | 304:560  | FPR f0:f31
 *   32:152   | 144:384 | 64:304   | GPR
 *   28       | 136     | 56       | XER
 *   24       | 128     | 48       | CTR
 *   20       | 120     | 40       | LR
 *   16       | 112     | 32       | CR
 *
 *   SP Offsets VR
 *   ppc32    | ppc64     | ppc64le   |  Contents
 *   ---------+-----------+-----------+----------
 *   936:944  | 1176:1472 | 1096:1616 | Alignment Padding / Reserved space
 *   932      | 1172      | 1092      | VRSAVE
 *   928      | 1168      | 1088      | VSCR
 *   416:928  | 656:1168  | 576:1088  | VR v0:v31
 *   408      | 640       | 560       | FPSCR (64b store) (+8B padding 64/64le)
 *   152:408  | 384:640   | 304:560   | FPR f0:f31
 *   32:152   | 144:384   | 64:304    | GPR
 *   28       | 136       | 56        | XER
 *   24       | 128       | 48        | CTR
 *   20       | 120       | 40        | LR
 *   16       | 112       | 32        | CR
 *
 *   SP Offsets VSR
 *   ppc32    | ppc64     | ppc64le   |  Contents
 *   ---------+-----------+-----------+----------
 *   1200     | 1432:1728 | 1352:1872 | Alignment Padding / Reserved space
 *   1188     | 1428      | 1348      | VRSAVE
 *   1184     | 1424      | 1344      | VSCR
 *   160:1184 | 400:1424  | 320:1344  | VSR vr0:vr63
 *   152      | 384       | 304       | FPSCR (64b store) (+8B padding 64/64le)
 *   32:152   | 144:384   | 64:304    | GPR
 *   28       | 136       | 56        | XER
 *   24       | 128       | 48        | CTR
 *   20       | 120       | 40        | LR
 *   16       | 112       | 32        | CR
 */

#ifdef __powerpc64__
#  if _CALL_ELF == 2 /* LE specific */
#    define CALLER_FRAME 32
#    define ABI_GAP      512
#    define VRSPLT(x,y)  vspltw x,y,3;
#  else	/* BE  */
#    define CALLER_FRAME 112
#    define ABI_GAP      288
#    define VRSPLT(x,y)  vspltw x,y,0;
#  endif
#  define GPRLD  ld
#  define GPRST  std
#  define STU    stdu
#  define GPRSZ  8
#  define FPSCR_PAD 8
#  define GPR_STACK_PAD 0
#  define VEC_STACK_PAD 8
#  define TREG r2
#else
#  define ABI_GAP         0
#  define CALLER_FRAME    16
#  define GPRLD  lwz
#  define GPRST  stw
#  define STU    stwu
#  define GPRSZ  4
#  define FPSCR_PAD 0
#  define VRSPLT(x,y)  vspltw x,y,0;
#  define GPR_STACK_PAD 8
#  define VEC_STACK_PAD 8
#  define TREG r13
#endif

#define CR_SAVE         (CALLER_FRAME)
#define CR_SIZE         (GPRSZ)
#define LR_SAVE         (CR_SAVE + CR_SIZE)
#define LR_SIZE         (GPRSZ)
#define CTR_SAVE        (LR_SAVE + LR_SIZE)
#define CTR_SIZE        (GPRSZ)
#define XER_SAVE        (CTR_SAVE + CTR_SIZE)
#define XER_SIZE        (GPRSZ)
#define GPR_SAVE(n)     (XER_SAVE + XER_SIZE + (GPRSZ * n))
#define GPR_SIZE        (30 * GPRSZ)
#define STACK_FRAME_GPR (ABI_GAP + GPR_SAVE(30) + GPR_STACK_PAD)

#define FPR_SAVE(n)     (GPR_SAVE(30) + (8 * n))
#define FPR_SIZE        (32 * 8)
#define FPSR_SAVE       (FPR_SAVE(32))

/* PPC64 requires padding to get back onto 16B alignment for VR's.  */
#define FPSR_SIZE       (8 + FPSCR_PAD)
#define STACK_FRAME_FPR (ABI_GAP + FPSR_SAVE + FPSR_SIZE)

#define VR_SAVE(n)      (FPSR_SAVE + FPSR_SIZE + (16 * n))
#define VSCR_SIZE       (4)
#define VSCR_SAVE       (VR_SAVE(32))
#define VRSAVE_SIZE     (4)
#define VRSAVE_SAVE     (VSCR_SAVE + VSCR_SIZE)
#define STACK_FRAME_VR  (ABI_GAP + VRSAVE_SAVE + VRSAVE_SIZE + VEC_STACK_PAD)

/* VSR registers overlap both FPR and VR registers. We take advantage of that
   to reduce our burgeoning stack footprint.  */
#define VSR_SIZE        (64 * 16)
#define VSR_FPSR        (GPR_SAVE(30))
#define VSR_SAVE(n)     (VSR_FPSR + FPSR_SIZE + (16 * n))
#define VSR_VSCR        (VSR_SAVE(64))
#define VSR_VRSAVE      (VSR_VSCR + VSCR_SIZE)
#define STACK_FRAME_VSR (ABI_GAP + VSR_VRSAVE + VRSAVE_SIZE + VEC_STACK_PAD)

/* Save all General Purpose Register (GPR), but r1 (SP) and r2 (thr. id).  */
#define SAVE_GPR                                                   \
  GPRST       r0, GPR_SAVE(0)(r1);                                   \
  GPRST       r3, GPR_SAVE(1)(r1);                                   \
  GPRST       r4, GPR_SAVE(2)(r1);                                   \
  GPRST       r5, GPR_SAVE(3)(r1);                                   \
  GPRST       r6, GPR_SAVE(4)(r1);                                   \
  GPRST       r7, GPR_SAVE(5)(r1);                                   \
  GPRST       r8, GPR_SAVE(6)(r1);                                   \
  GPRST       r9, GPR_SAVE(7)(r1);                                   \
  GPRST      r10, GPR_SAVE(8)(r1);                                   \
  GPRST      r11, GPR_SAVE(9)(r1);                                   \
  GPRST      r12, GPR_SAVE(10)(r1);                                  \
  GPRST      TREG, GPR_SAVE(11)(r1);                                 \
  GPRST      r14, GPR_SAVE(12)(r1);                                  \
  GPRST      r15, GPR_SAVE(13)(r1);                                  \
  GPRST      r16, GPR_SAVE(14)(r1);                                  \
  GPRST      r17, GPR_SAVE(15)(r1);                                  \
  GPRST      r18, GPR_SAVE(16)(r1);                                  \
  GPRST      r19, GPR_SAVE(17)(r1);                                  \
  GPRST      r20, GPR_SAVE(18)(r1);                                  \
  GPRST      r21, GPR_SAVE(19)(r1);                                  \
  GPRST      r22, GPR_SAVE(20)(r1);                                  \
  GPRST      r23, GPR_SAVE(21)(r1);                                  \
  GPRST      r24, GPR_SAVE(22)(r1);                                  \
  GPRST      r25, GPR_SAVE(23)(r1);                                  \
  GPRST      r26, GPR_SAVE(24)(r1);                                  \
  GPRST      r27, GPR_SAVE(25)(r1);                                  \
  GPRST      r28, GPR_SAVE(26)(r1);                                  \
  GPRST      r29, GPR_SAVE(27)(r1);                                  \
  GPRST      r30, GPR_SAVE(28)(r1);                                  \
  GPRST      r31, GPR_SAVE(29)(r1);

/* Restore GRP registers, but r1 (stack) and r2 (thread id).  */
#define RESTORE_GPR                                                \
  GPRLD       r0, GPR_SAVE(0)(r1);                                   \
  GPRLD       r3, GPR_SAVE(1)(r1);                                   \
  GPRLD       r4, GPR_SAVE(2)(r1);                                   \
  GPRLD       r5, GPR_SAVE(3)(r1);                                   \
  GPRLD       r6, GPR_SAVE(4)(r1);                                   \
  GPRLD       r7, GPR_SAVE(5)(r1);                                   \
  GPRLD       r8, GPR_SAVE(6)(r1);                                   \
  GPRLD       r9, GPR_SAVE(7)(r1);                                   \
  GPRLD      r10, GPR_SAVE(8)(r1);                                   \
  GPRLD      r11, GPR_SAVE(9)(r1);                                   \
  GPRLD      r12, GPR_SAVE(10)(r1);                                  \
  GPRLD      TREG, GPR_SAVE(11)(r1);                                 \
  GPRLD      r14, GPR_SAVE(12)(r1);                                  \
  GPRLD      r15, GPR_SAVE(13)(r1);                                  \
  GPRLD      r16, GPR_SAVE(14)(r1);                                  \
  GPRLD      r17, GPR_SAVE(15)(r1);                                  \
  GPRLD      r18, GPR_SAVE(16)(r1);                                  \
  GPRLD      r19, GPR_SAVE(17)(r1);                                  \
  GPRLD      r20, GPR_SAVE(18)(r1);                                  \
  GPRLD      r21, GPR_SAVE(19)(r1);                                  \
  GPRLD      r22, GPR_SAVE(20)(r1);                                  \
  GPRLD      r23, GPR_SAVE(21)(r1);                                  \
  GPRLD      r24, GPR_SAVE(22)(r1);                                  \
  GPRLD      r25, GPR_SAVE(23)(r1);                                  \
  GPRLD      r26, GPR_SAVE(24)(r1);                                  \
  GPRLD      r27, GPR_SAVE(25)(r1);                                  \
  GPRLD      r28, GPR_SAVE(26)(r1);                                  \
  GPRLD      r29, GPR_SAVE(27)(r1);                                  \
  GPRLD      r30, GPR_SAVE(28)(r1);                                  \
  GPRLD      r31, GPR_SAVE(29)(r1);

/* Save status registers.  */
#define SAVE_SPEC_REG                                              \
  mfcr     r0;                                                     \
  GPRST    r0, CR_SAVE(r1);                                        \
  mflr     r0;                                                     \
  GPRST    r0, LR_SAVE(r1);                                        \
  mfctr    r0;                                                     \
  GPRST    r0, CTR_SAVE(r1);                                       \
  mfxer    r0;                                                     \
  GPRST    r0, XER_SAVE(r1);

/* Restore state registers.  */
#define RESTORE_SPEC_REG                                           \
  GPRLD    r0, CR_SAVE(r1);                                        \
  mtcr     r0;                                                     \
  GPRLD    r0, LR_SAVE(r1);                                        \
  mtlr     r0;                                                     \
  GPRLD    r0, CTR_SAVE(r1);                                       \
  mtctr    r0;                                                     \
  GPRLD    r0, XER_SAVE(r1);                                       \
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
  VRSPLT(v0,v0)                \
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
  VRSPLT(v0,v0)                 \
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

/* Restore our TOC pointer using our entry point */
#if _CALL_ELF != 2 && defined( __powerpc64__)
# define SETUP_TOC(__func)                                         \
   bcl 20,31,1f;                                                   \
1: mflr r12;                                                       \
   addis r12,r12,(__func-1b)@ha;                                   \
   addi r12,r12,(__func-1b)@l;                                     \
   ld r2,8(r12);
# define CALL_EBB_HOOK bl       __paf_ebb_ebb_hook; nop;

#elif defined(__powerpc64__)

# define SETUP_TOC(__func)                                         \
   bcl 20,31,1f;                                                   \
1: mflr r2;                                                        \
   addis r2,r2,(.TOC.-1b)@ha;                                      \
   addi r2,r2,(.TOC.-1b)@l;

#define CALL_EBB_HOOK bl __paf_ebb_ebb_hook; nop;

#else
# define SETUP_TOC(__func)
# define CALL_EBB_HOOK bl __paf_ebb_ebb_hook;
#endif


ENTRY(__paf_ebb_callback_handler_gpr)
  STU  r1,-STACK_FRAME_GPR(r1)
  .cfi_adjust_cfa_offset STACK_FRAME_GPR

  SAVE_GPR

  SAVE_SPEC_REG

  SETUP_TOC (__paf_ebb_callback_handler_gpr)
  CALL_EBB_HOOK

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_GPR

  RFEBB1
END(__paf_ebb_callback_handler_gpr)


ENTRY(__paf_ebb_callback_handler_fpr)
  STU  r1,-STACK_FRAME_FPR(r1)
  .cfi_adjust_cfa_offset STACK_FRAME_FPR

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


  .machine "altivec"
ENTRY(__paf_ebb_callback_handler_vr)
  /* Reserve stack-frame size.  */
  STU  r1,-STACK_FRAME_VR(r1)
  .cfi_adjust_cfa_offset STACK_FRAME_VR

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


  .machine "power7"
ENTRY(__paf_ebb_callback_handler_vsr)
  /* Reserve stack-frame size.  */
  STU  r1,-STACK_FRAME_VSR(r1)
  .cfi_adjust_cfa_offset STACK_FRAME_VSR

  SAVE_GPR

  SAVE_SPEC_REG

  SAVE_VSR

  SETUP_TOC (__paf_ebb_callback_handler_vsr)
  CALL_EBB_HOOK

  RESTORE_VSR

  RESTORE_SPEC_REG

  RESTORE_GPR

  addi  r1,r1,STACK_FRAME_VSR

  RFEBB1
END(__paf_ebb_callback_handler_vsr)
