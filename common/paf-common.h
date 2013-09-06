/*
 * PAFlib common definitions.
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

#ifndef _PAF_COMMON_H
# define _PAF_COMMON_H

#define attribute_hidden       __attribute__ ((visibility ("hidden")))
#define attribute_constructor  __attribute__ ((constructor))
#define attribute_initial_exec __attribute__ ((tls_model ("initial-exec")))
#define attribute_noinline     __attribute__ ((noinline))


typedef unsigned long int spr_t;

#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)

#define mfspr(rn)					\
  ({ spr_t spr; 					\
     asm volatile("mfspr %0," __stringify(rn)		\
                  : "=r" (spr));			\
     spr;						\
  })
#define mtspr(rn, v)					\
  ({ spr_t spr = (spr_t)v;				\
     asm volatile("mtspr " __stringify(rn) ",%0"	\
	       : : "r" (spr));				\
  })

#endif
