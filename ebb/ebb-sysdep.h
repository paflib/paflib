/* Event-Based Branch Facility API.  PowerPC callback definitions.
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

#ifndef _PAF_EBB_SYSDEP_H
# define _PAF_EBB_SYSDEP_H

#include "ebb-common.h"

/* Instruction 'rfebb 1' - Return from EBB handler (ISA 2.07).  */
#define RFEBB1      .long 0x4c000924

/* Symbolic names for the registers.  */

#define cr0	0

/* Integer registers.  */
#define r0	0
#define r1	1
#define r2	2
#define r3	3
#define r4	4
#define r5	5
#define r6	6
#define r7	7
#define r8	8
#define r9	9
#define r10	10
#define r11	11
#define r12	12
#define r13	13
#define r14	14
#define r15	15
#define r16	16
#define r17	17
#define r18	18
#define r19	19
#define r20	20
#define r21	21
#define r22	22
#define r23	23
#define r24	24
#define r25	25
#define r26	26
#define r27	27
#define r28	28
#define r29	29
#define r30	30
#define r31	31

/* Floating-point registers.  */
#define f0	0
#define f1	1
#define f2	2
#define f3	3
#define f4	4
#define f5	5
#define f6	6
#define f7	7
#define f8	8
#define f9	9
#define f10	10
#define f11	11
#define f12	12
#define f13	13
#define f14	14
#define f15	15
#define f16	16
#define f17	17
#define f18	18
#define f19	19
#define f20	20
#define f21	21
#define f22	22
#define f23	23
#define f24	24
#define f25	25
#define f26	26
#define f27	27
#define f28	28
#define f29	29
#define f30	30
#define f31	31

/* Vector registers. */
#define v0	0
#define v1	1
#define v2	2
#define v3	3
#define v4	4
#define v5	5
#define v6	6
#define v7	7
#define v8	8
#define v9	9
#define v10	10
#define v11	11
#define v12	12
#define v13	13
#define v14	14
#define v15	15
#define v16	16
#define v17	17
#define v18	18
#define v19	19
#define v20	20
#define v21	21
#define v22	22
#define v23	23
#define v24	24
#define v25	25
#define v26	26
#define v27	27
#define v28	28
#define v29	29
#define v30	30
#define v31	31

#define VRSAVE	256

#define ALIGNARG(num)	num


#ifdef __powerpc64__

# if _CALL_ELF != 2
/* PPC64 BE ELFv1 ABI  */
#  define BODY_LABEL(X)	.##X
#  define DOT_LABEL(X)	.##X
#  define OPD_ENT(name)	.quad BODY_LABEL (name), .TOC.@tocbase

#  define ENTRY_1(name)   				\
        .type BODY_LABEL(name),@function;		\
        .globl name;					\
        .section ".opd","aw";				\
        .align 3;					\
name##: OPD_ENT (name);					\
        .previous;

#  define ENTRY_ABI(name)				\
        .globl BODY_LABEL(name);			\
        ENTRY_1(name)                           	\
        .size name, 24;
#  define LOCALENTRY(name)

#  define END_ABI(name)					\
        .size BODY_LABEL(name),.-BODY_LABEL(name);
# else
/* PPC64 LE ELFv2 ABI  */
#define DOT_LABEL(X)	X
#define BODY_LABEL(X)	X
#define ENTRY_ABI(name)					\
	.globl name;					\
	.type name,@function;

#define END_ABI(name)					\
	.size name,.-name;

/* Although ELFv2 defines the localentry entrypoint, for EBB handlers we
   need to save current status before materialize handler's TOC.  */
#define LOCALENTRY(name)
# endif

/* Both PPC64 LE and BE defines  */
# define ENTRY(name)					\
        .section ".text";				\
	ENTRY_ABI(name)					\
	.align	ALIGNARG(2);				\
BODY_LABEL(name):					\
	.cfi_startproc;					\
	LOCALENTRY(name)

/* Traceback defines  */
#define TB_ASM                  0x000c000000000000
#define TB_HAS_TBOFF            0x0000200000000000
#define TB_NAME_PRESENT         0x0000004000000000
#define TB_DEFAULT		TB_ASM | TB_HAS_TBOFF | TB_NAME_PRESENT

#define TRACEBACK(name)					\
LT_LABEL(name): ;					\
	.long   0;					\
        .quad   TB_DEFAULT;				\
        .long   LT_LABEL(name)-BODY_LABEL(name) ; 	\
        .short  LT_LABELSUFFIX(name,_name_end)-		\
		LT_LABELSUFFIX(name,_name_start) ;	\
LT_LABELSUFFIX(name,_name_start):;			\
        .ascii  stringify(name); 			\
LT_LABELSUFFIX(name,_name_end):;			\
        .align  2;

# define END(name)					\
	.cfi_endproc;					\
	END_ABI(name)

#else

/* PPC32 BE ELF ABI  */
# define C_LABEL(name)  name##:

# define ENTRY(name)					\
	.globl name;					\
	.hidden name;					\
	.type name,@function;				\
	.align ALIGNARG(2);				\
	C_LABEL(name)					\
	.cfi_startproc;

# define END(name)					\
	.cfi_endproc;					\
	.size name,.-name

#endif

#endif /* _PAF_EBB_SYSDEP_H  */
