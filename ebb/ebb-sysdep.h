/*
 * Event-Based Branch Facility API.  PowerPC callback definitions.
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

#ifndef _PPCEBB_SYSDEP_H
# define _PPCEBB_SYSDEP_H

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
#define BODY_LABEL(X)	.##X
#define OPD_ENT(name)	.quad BODY_LABEL (name), .TOC.@tocbase

#define ENTRY(name)					\
	.type name,@function;				\
        .section        ".text";			\
        .type BODY_LABEL(name),@function;		\
        .globl name;					\
	.hidden name;					\
        .section ".opd","aw";				\
        .align 3;					\
name##: OPD_ENT (name);					\
        .previous;					\
        .align ALIGNARG(2);				\
BODY_LABEL(name):					\

#define END(name)					\
        .size name,.-BODY_LABEL(name);          	\
        .size BODY_LABEL(name),.-BODY_LABEL(name);

#else
# define C_LABEL(name)  name##:

# define ENTRY(name)					\
	.globl name;					\
	.hidden name;					\
	.type name,@function;				\
	.align ALIGNARG(2);				\
	C_LABEL(name)

# define END(name)					\
	.size name,.-name

#endif

#endif /* _PPCEBB_SYSDEP_H  */
