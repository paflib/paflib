/* Event-Based Branch Facility API register clobber ASM hooks.
 *
 * Copyright IBM Corp. 2015
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

#include "ebb-sysdep.h"

/* This in no way represents the optimal stack size, it just happens
 * to keep everything properly aligned in both 32 and 64b interfaces
 * without the ifdef madness.
 *
 * Note, we cannot perfectly test this as one CR must be used by to
 * keep us in a busy loop. Unfortunately, we have to duplicate most
 * of the work in the handler as we too need to preserve all the
 * registers.
 */
#define STACK_OFFSET (512)
#define SPRS(x) (STACK_OFFSET + (8 * x))
#define GPRS(x) (SPRS(8)  + (8 * x))
#define FPRS(x) (GPRS(32) + (8 * x))
#define VRS(x)  (FPRS(32) + (16 * x))
#define VSRS(x) (VRS(32)  + (16 * x))

#define LXER 0
#define LCTR 1
#define LLR  2
#define LCR  3
#define LVSCR 4
#define LVRSAVE 5
#define LFPSCR 6

#define STACK_SIZE (VSRS(64) + 16)

/* QW aligned stack location to move between regs.  */
#define SCRATCH (VSRS(64))

/* These definitions are specific to the machine. */
#ifdef __powerpc64__
#  define STORE std
#  define LOAD ld
#  define CMPI  cmpdi
#  define ALLOC_STACK(x) stdu r1,-(x)(r1);
#else /* ppc32 */
#  define STORE stw
#  define LOAD  lwz
#  define CMPI  cmpwi
#  define ALLOC_STACK(x) stdu r1,-(x)(r1);
#endif

/* Endian specific macros.  Some are unavoidable.  */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define EOLD 0
#  define EOHD 4
#  define VSPLTW_LD(x,y) vspltw x,y,3;
#  define VSPLTW_ST(x,y) vspltw x,y,3;
#else
#  define EOLD 4
#  define EOHD 0
#  define VSPLTW_LD(x,y) vspltw x,y,0;
#  define VSPLTW_ST(x,y) vspltw x,y,3;
#endif

#define FREE_STACK(x) addi r1,r1,(x);

/* The macros below generate the test procedures.
 *
 */

#define RUN_HANDLER bl ebb_handler_test; nop;

/* Duplicate the operation x on values 0-31 in order.  */
#define OP_32(x)                                         \
	x(0);x(1);x(2);x(3);x(4);x(5);x(6);x(7);         \
	x(8);x(9);x(10);x(11);x(12);x(13);x(14);x(15);   \
	x(16);x(17);x(18);x(19);x(20);x(21);x(22);x(23); \
	x(24);x(25);x(26);x(27);x(28);x(29);x(30);x(31);

/* Similar to above.  Used to avoid touching r1-r3,r13,r31. */
#define GPR_OP_30(x)                                     \
	x(0);                 x(4);x(5);x(6);x(7);       \
	x(8);x(9);x(10);x(11);x(12);      x(14);x(15);   \
	x(16);x(17);x(18);x(19);x(20);x(21);x(22);x(23); \
	x(24);x(25);x(26);x(27);x(28);x(29);x(30);

/* Duplicate for 0-63 x's in order.  */
#define OP_64(x)                                         \
	OP_32(x)                                         \
	x(32);x(33);x(34);x(35);x(36);x(37);x(38);x(39); \
	x(40);x(41);x(42);x(43);x(44);x(45);x(46);x(47); \
	x(48);x(49);x(50);x(51);x(52);x(53);x(54);x(55); \
	x(56);x(57);x(58);x(59);x(60);x(61);x(62);x(63);

/* Duplicate the operation x on values 0-31 in order with y input.  */
#define OP_32_1(x,y)                                                     \
	x(0,y);x(1,y);x(2,y);x(3,y);x(4,y);x(5,y);x(6,y);x(7,y);         \
	x(8,y);x(9,y);x(10,y);x(11,y);x(12,y);x(13,y);x(14,y);x(15,y);   \
	x(16,y);x(17,y);x(18,y);x(19,y);x(20,y);x(21,y);x(22,y);x(23,y); \
	x(24,y);x(25,y);x(26,y);x(27,y);x(28,y);x(29,y);x(30,y);x(31,y);

#define OP_64_1(x,y)                                                     \
        OP_32_1(x,y)                                                     \
	x(32,y);x(33,y);x(34,y);x(35,y);x(36,y);x(37,y);x(38,y);x(39,y); \
	x(40,y);x(41,y);x(42,y);x(43,y);x(44,y);x(45,y);x(46,y);x(47,y); \
	x(48,y);x(49,y);x(50,y);x(51,y);x(52,y);x(53,y);x(54,y);x(55,y); \
	x(56,y);x(57,y);x(58,y);x(59,y);x(60,y);x(61,y);x(62,y);x(63,y);

/* Similar to GPR_OP_30, with an extra param.  */
#define GPR_OP_30_1(x,y)                                                 \
	x(0,y);                     x(4,y);x(5,y);x(6,y);x(7,y);         \
	x(8,y);x(9,y);x(10,y);x(11,y);x(12,y);        x(14,y);x(15,y);   \
	x(16,y);x(17,y);x(18,y);x(19,y);x(20,y);x(21,y);x(22,y);x(23,y); \
	x(24,y);x(25,y);x(26,y);x(27,y);x(28,y);x(29,y);x(30,y);

#define _SAVE_GPR(a) STORE a,(GPRS(a))(r1);
#define SAVE_GPRS GPR_OP_30(_SAVE_GPR)

#define _REST_GPRS(a) LOAD a,(GPRS(a))(r1);
#define REST_GPRS GPR_OP_30(_REST_GPRS)

#define _SAVE_FPRS(a) stfd a,(FPRS(a))(r1);
#define SAVE_FPRS OP_32(_SAVE_FPRS)

#define _REST_FPRS(a) lfd a,(FPRS(a))(r1);
#define REST_FPRS OP_32(_REST_FPRS)

#define _SAVE_VRS(a) addi r3,r1,VRS(a); stvx a,0,r3;
#define SAVE_VRS OP_32(_SAVE_VRS)

#define _REST_VRS(a) addi r0,r1,VRS(a); lvx a,0,r0;
#define REST_VRS OP_32(_REST_VRS)

#define _SAVE_VSRS(a) addi r3,r1,VSRS(a); stxvd2x a,0,r3;
#define SAVE_VSRS OP_64(_SAVE_VSRS)

#define _REST_VSRS(a) addi r0,r1,VSRS(a); lxvd2x a,0,r0;
#define REST_VSRS OP_64(_REST_VSRS)

#define SAVE_FPSCR               \
	mffs f0;                 \
	stfd f0,SPRS(LFPSCR)(r1);

#define REST_FPSCR               \
	lfd f0,SPRS(LFPSCR)(r1); \
	mtfsf 0xff,f0;

#define SAVE_GP_SPRS             \
	mfcr r3;                 \
	STORE r3,SPRS(LCR)(r1);  \
	mflr r3;                 \
	STORE r3,SPRS(LLR)(r1);  \
	mfctr r3;                \
	STORE r3,SPRS(LCTR)(r1); \
	mfxer r3;                \
	STORE r3,SPRS(LXER)(r1);

#define REST_GP_SPRS           \
	LOAD r0,SPRS(LCR)(r1); \
	mtcr r0;               \
	LOAD r0,SPRS(LLR)(r1); \
	mtlr r0;               \
	LOAD r0,SPRS(LCTR)(r1);\
	mtctr r0;              \
	LOAD r0,SPRS(LXER)(r1);\
	mtxer r0;

#define SAVE_VRSAVE                 \
	mfspr r3,VRSAVE;            \
	STORE r3,SPRS(LVRSAVE)(r1);

#define REST_VRSAVE                \
	LOAD r0,SPRS(LVRSAVE)(r1); \
	mtspr VRSAVE,r0;

#define SAVE_VSCR               \
	mfvscr v0;              \
	VSPLTW_ST(v0,v0);       \
	addi r3,r1,SPRS(LVSCR); \
	stvewx v0,0,r3;

#define REST_VSCR               \
	addi r0,r1,SPRS(LVSCR); \
	lvewx v0,0,r0;          \
	VSPLTW_LD(v0,v0);       \
	mtvscr v0;

#define _PREP_GPRS(a) li a,(a+1000);
#define PREP_GPRS GPR_OP_30(_PREP_GPRS)

#define _PREP_FPRS(a)             \
	li r3,(a+2000);           \
	stw 3,(SCRATCH+EOLD)(r1); \
	li r3,0;                  \
	stw 3,(SCRATCH+EOHD)(r1); \
	lfd a,(SCRATCH)(r1);
#define PREP_FPRS OP_32(_PREP_FPRS)

#define PREP_FPSCR                 \
	li r3,(3);                 \
	stw r3,(SCRATCH+EOLD)(r1); \
	li r3,0;                   \
	stw r3,(SCRATCH+EOHD)(r1); \
	lfd f0,(SCRATCH)(r1);      \
	mtfsf 0xff,f0;

#define CHECK_FPSCR(x)            \
	mffs f0;                  \
	stfd f0,(SCRATCH)(r1);    \
	lwz r0,(SCRATCH+EOLD)(r1);\
	cmpwi cr7,r0,(3);         \
	li r3,SPRS(LFPSCR);       \
	bne cr7,.Ldone_##x;

#define _CHECK_FPRS(a,l)           \
	stfd a,(SCRATCH)(r1);      \
	lwz r0,(SCRATCH+EOLD)(r1); \
	cmpwi cr7,r0,(a+2000);     \
	li r3,(FPRS(a)+4);         \
	bne cr7,.Ldone_##l;        \
	lwz r0,(SCRATCH+EOHD)(r1); \
	cmpwi cr7,r0,(0);          \
	li r3,FPRS(a);             \
	bne cr7,.Ldone_##l;
#define CHECK_FPRS(x) OP_32_1(_CHECK_FPRS,x)

/* Load some values into the GP SPRs.
 * Note that 0xAA translate to all
 * "equal" comparisons to simplify
 * testing.
 *
 * Note, this isn't perfect.
 */
#define PREP_GP_SPRS \
	li r3,0x22;           \
	mtcr r3;              \
	li r3,0x55;           \
	mtxer r3;             \
	li r3,0x11;           \
	mtctr r3;             \
	li r3,0x33;           \
	mtlr r3;

#define CHECK_GP_SPRS(x)     \
	mfcr r3;             \
	CMPI cr7,r3,0x22;    \
	li r3,SPRS(LCR);     \
	bne cr7,.Ldone_##x;  \
	mfxer r3;            \
	CMPI cr7,r3,0x55;    \
	li r3,SPRS(LXER);    \
	bne cr7,.Ldone_##x;  \
	mfctr r3;            \
	CMPI cr7,r3,0x11;    \
	li r3,SPRS(LCTR);    \
	bne cr7,.Ldone_##x;  \
	mflr r3;             \
	CMPI cr7,r3,0x33;    \
	li r3,SPRS(LCTR);    \
	bne cr7,.Ldone_##x;

#define _CHECK_GPRS(a,l) \
	CMPI cr7,a,(1000+a); \
	li r3,GPRS(a);       \
	bne cr7,.Ldone_##l;
#define CHECK_GPRS(l) GPR_OP_30_1(_CHECK_GPRS,l)

#define _PREP_VEC_LIKE_REG(x,LVECINSN) \
	li r3,x+(x*256);         \
	addis r3,r3,x+(x*256);   \
	stw r3,(SCRATCH+0)(r1);  \
	stw r3,(SCRATCH+4)(r1);  \
	stw r3,(SCRATCH+8)(r1);  \
	stw r3,(SCRATCH+12)(r1); \
	addi r3,r1,(SCRATCH);    \
	LVECINSN x,0,r3;

#define _PREP_VRS(x) _PREP_VEC_LIKE_REG(x,lvx)
#define PREP_VRS OP_32(_PREP_VRS)

#define _PREP_VSRS(x) _PREP_VEC_LIKE_REG(x,lxvd2x)
#define PREP_VSRS OP_64(_PREP_VSRS)

#define _CHECK_VEC_LIKE_REG(x,l,SVECINSN) \
	addi r0,r1,(SCRATCH);    \
	SVECINSN x,0,r0;         \
	li r3,x+(x*256);         \
	addis r3,r3,x+(x*256);   \
	lwz r0,(SCRATCH+0)(r1);  \
	cmpw r0,r3;              \
	bne .Ldone_##l;          \
	lwz r0,(SCRATCH+4)(r1);  \
	cmpw r0,r3;              \
	bne .Ldone_##l;          \
	lwz r0,(SCRATCH+8)(r1);  \
	cmpw r0,r3;              \
	bne .Ldone_##l;          \
	lwz r0,(SCRATCH+12)(r1); \
	cmpw r0,r3;              \
	bne .Ldone_##l;

#define _CHECK_VRS(x,l) _CHECK_VEC_LIKE_REG(x,l,stvx)
#define CHECK_VRS(l) OP_32_1(_CHECK_VRS,l)

#define _CHECK_VSRS(x,l) _CHECK_VEC_LIKE_REG(x,l,stxvd2x)
#define CHECK_VSRS(l) OP_64_1(_CHECK_VSRS,l)

#define PREP_VR_SPRS          \
	li r3,0x44;           \
	mtspr VRSAVE, r3;     \
	li r3,0x01;           \
	stw r3,(SCRATCH)(r1); \
	addi r3,r1,(SCRATCH); \
	lvx v0,0,r3;          \
	VSPLTW_LD(v0,v0);     \
	mtvscr v0;


#define CHECK_VR_SPRS(l)      \
	mfspr r3,VRSAVE;      \
	cmpwi r3,0x44;        \
	li r3, SPRS(LVRSAVE); \
	bne .Ldone_##l;       \
	mfvscr v0;            \
	VSPLTW_ST(v0,v0);     \
	addi r3,r1,(SCRATCH); \
	stvewx v0,0,r3;       \
	lwz r3,(SCRATCH)(r1); \
	cmpwi r3,1;           \
	li r3,SPRS(LVSCR);    \
	bne .Ldone_##l;

/* Count to 100 and hope for an interrupt. r3 has
 * definitely been clobbered if it isn't [0,100].
 */
#define BUSY_LOOP(x)       \
	li r3,0;           \
.Lloop_##x:                \
	addi r3,r3,1;      \
	cmpwi cr7,r3,100;  \
	blt cr7,.Lloop_##x;\
	li r3,1000;        \
	bgt cr7,.Ldone_##x;

#define EPILOGUE(x)            \
	FREE_STACK(STACK_SIZE) \
	blr
	

ENTRY(ebb_test_gpr)
	ALLOC_STACK(STACK_SIZE)
	SAVE_GPRS SAVE_GP_SPRS

	PREP_GPRS PREP_GP_SPRS

	BUSY_LOOP(gpr)

	CHECK_GPRS(gpr) CHECK_GP_SPRS(gpr)

	li r3,0
.Ldone_gpr:
	REST_GP_SPRS REST_GPRS
	EPILOGUE(gpr)
END(ebb_test_gpr)


ENTRY(ebb_test_fpr)
	ALLOC_STACK(STACK_SIZE)
	SAVE_GPRS SAVE_GP_SPRS
	SAVE_FPRS SAVE_FPSCR

	PREP_FPSCR PREP_FPRS
	PREP_GPRS PREP_GP_SPRS

	BUSY_LOOP(fpr)

	CHECK_GPRS(fpr)
	CHECK_GP_SPRS(fpr)
	CHECK_FPRS(fpr)
	CHECK_FPSCR(fpr)

	li r3,0
.Ldone_fpr:
	REST_GP_SPRS REST_FPSCR REST_FPRS REST_GPRS
	EPILOGUE(fpr)
END(ebb_test_fpr)


ENTRY(ebb_test_vr)
	ALLOC_STACK(STACK_SIZE)
	SAVE_GPRS SAVE_FPRS SAVE_VRS
	SAVE_GP_SPRS SAVE_FPSCR SAVE_VRSAVE SAVE_VSCR

	PREP_VR_SPRS
	PREP_VRS
	PREP_FPSCR PREP_FPRS
	PREP_GPRS PREP_GP_SPRS

	BUSY_LOOP(vr)

	CHECK_GPRS(vr)
	CHECK_GP_SPRS(vr)
	CHECK_VRS(vr);
	CHECK_VR_SPRS(vr);
	CHECK_FPRS(vr)
	CHECK_FPSCR(vr)

	li r3,0
.Ldone_vr:
	REST_GP_SPRS REST_FPSCR REST_VRSAVE REST_VSCR
	REST_VRS REST_FPRS REST_GPRS
	EPILOGUE(vr)
END(ebb_test_vr)

ENTRY(ebb_test_vsr)
	ALLOC_STACK(STACK_SIZE)
	SAVE_GPRS SAVE_VSRS
	SAVE_GP_SPRS SAVE_FPSCR SAVE_VRSAVE SAVE_VSCR

	PREP_FPSCR
	PREP_VR_SPRS
	PREP_VSRS
	PREP_GP_SPRS
	PREP_GPRS

	BUSY_LOOP(vsr)

	CHECK_GPRS(vsr)
	CHECK_GP_SPRS(vsr)
	CHECK_VSRS(vsr)
	CHECK_VR_SPRS(vsr)
	CHECK_FPSCR(vsr)

	li r3,0
.Ldone_vsr:
	REST_GP_SPRS REST_FPSCR REST_VRSAVE REST_VSCR
	REST_VSRS REST_GPRS
	EPILOGUE(vsr)
END(ebb_test_vsr)

#ifdef __powerpc64__
#  define LR_OFFSET 16
#else
#  define LR_OFFSET 8
#endif

/* Note, there is actually an intermediate
   C function called between this and the
   actual handler. So we can't safely clobber
   nonvolatile registers. */

#define CLOBBER_GPRS \
	li 0, 0; \
	li 3, 0; li 4, 0; li 5, 0; li 6, 0; \
	li 7, 0; li 8, 0; li 9, 0; li 10,0; \
	li 11,0; li 12,0; \
	mtctr 0; \
	mtxer 0; \
	mtcrf 0, 0; mtcrf 1, 0; \
	mtcrf 5, 0; mtcrf 6, 0; mtcrf 7, 0; \

#define CLOBBER_FPRS \
	fsub 0, 0, 0; fsub 1, 1, 1; fsub 2, 2, 2; \
	fsub 3, 3, 3; fsub 4, 4, 4; fsub 5, 5, 5; \
	fsub 6, 6, 6; fsub 7, 7, 7; fsub 8, 8, 8; \
	fsub 9, 9, 9; fsub 10, 10, 10; fsub 11, 11, 11; \
	fsub 12, 12, 12; fsub 13, 13, 13; \
	mtfsf 0xff, 0;

/* Note, overlap with FPRS, don't clobber nonvolatiles. */
#define CLOBBER_VSRS \
	xxlxor 0, 0, 0; xxlxor 1, 1, 1; xxlxor 2, 2, 2; \
	xxlxor 3, 3, 3; xxlxor 4, 4, 4; xxlxor 5, 5, 5; \
	xxlxor 6, 6, 6; xxlxor 7, 7, 7; xxlxor 8, 8, 8; \
	xxlxor 9, 9, 9; xxlxor 10, 10, 10; xxlxor 11, 11, 11; \
	xxlxor 12, 12, 12; xxlxor 13, 13, 13; \
	mtfsf 0xff, 0;

#define CLOBBER_VRS \
	vxor 0,0,0; vxor 1,1,1; vxor 2,2,2; \
	vxor 3,3,3; vxor 4,4,4; vxor 5,5,5; \
	vxor 6,6,6; vxor 7,7,7; vxor 8,8,8; \
	vxor 9,9,9; vxor 10,10,10; vxor 11,11,11;    \
	vxor 12,12,12; vxor 13,13,13; vxor 14,14,14; \
	vxor 15,15,15; vxor 16,16,16; vxor 17,17,17; \
	vxor 18,18,18; vxor 19,19,19;

#if _CALL_ELF != 2
  #define ENTRY_CLOB(_x)           \
	  ENTRY(_x)                \
	  mflr r0;                 \
	  STORE r0,LR_OFFSET(r1);  \
	  ALLOC_STACK(32)
#else
  #define ENTRY_CLOB(_x)           \
          ENTRY(_x)                \
0:        addis 2,12,.TOC.-0b@ha;  \
	  addi  2,2,.TOC.-0b@l;    \
	  .localentry _x,.-##_x;   \
	  mflr r0;                 \
	  STORE r0,LR_OFFSET(r1);  \
	  ALLOC_STACK(32)
#endif

#define END_CLOB(_x)          \
	FREE_STACK(32)        \
	LOAD r0,LR_OFFSET(r1);\
	mtlr r0;              \
	blr;                  \
	END(_x)

ENTRY_CLOB(ebb_clobber_gpr)
	RUN_HANDLER
	CLOBBER_GPRS
END_CLOB(ebb_clobber_gpr)


ENTRY_CLOB(ebb_clobber_fpr)
	RUN_HANDLER
	CLOBBER_GPRS CLOBBER_FPRS
END_CLOB(ebb_clobber_fpr)


ENTRY_CLOB(ebb_clobber_vr)
	RUN_HANDLER
	CLOBBER_GPRS CLOBBER_FPRS CLOBBER_VRS
END_CLOB(ebb_clobber_vr)

ENTRY_CLOB(ebb_clobber_vsr)
	RUN_HANDLER
	CLOBBER_GPRS CLOBBER_VSRS CLOBBER_VRS
END_CLOB(ebb_clobber_vsr)
