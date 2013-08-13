/*
 * Event-Based Branch Facility API. Private definitions.
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

#ifndef _PAF_EBB_PRIV_H
#define _PAF_EBB_PRIV_H

#include <paf/ebb.h>
#include "ebb-common.h"

#define attribute_hidden       __attribute__ ((visibility ("hidden")))
#define attribute_constructor  __attribute__ ((constructor))
#define attribute_initial_exec __attribute__ ((tls_model ("initial-exec")))
#define attribute_noinline     __attribute__ ((noinline))

#ifdef __powerpc64__
# define LOAD_INST        "ld"
# define STORE_INST       "std"
# define THREAD_REGISTER  "13"
# define EBB_HANDLER      (-28728)
# define EBB_CTX_POINTER  (-28720)
# define EBB_FLAGS        (-28712)
#else
# define LOAD_INST        "lwz"
# define STORE_INST       "stw"
# define THREAD_REGISTER  "2"
# define EBB_HANDLER      (-28700)
# define EBB_CTX_POINTER  (-28696)
# define EBB_FLAGS        (-28692)
#endif

typedef unsigned long int spr_t;


/* EBB per-thread information to use where TCB fields are not available.  */
struct ebb_thread_info_t
{
  ebbhandler_t handler;
  void *context;
  unsigned long int flags;
};

extern __thread
attribute_initial_exec
attribute_hidden
struct ebb_thread_info_t __paf_ebb_thread_info;

/* Set to 1 if TCB fields to use TCB fields, 0 to use TLS ones.  */
extern int __paf_ebb_use_tcb attribute_hidden;


static inline
__attribute__((always_inline))
ebbhandler_t __paf_ebb_get_thread_handler ()
{
  ebbhandler_t ret;
  if (__paf_ebb_use_tcb)
    {
      asm (LOAD_INST " %0,%1(" THREAD_REGISTER ")"
	   : "=r" (ret)
	   : "i" (EBB_HANDLER));
    }
  else
    {
      ret = __paf_ebb_thread_info.handler;
    }
  return ret;
}

static inline
__attribute__((always_inline))
void __paf_ebb_set_thread_handler (ebbhandler_t handler)
{
  if (__paf_ebb_use_tcb)
    {
      asm (STORE_INST " %0,%1(" THREAD_REGISTER ")"
	   : : "r" (handler), "i" (EBB_HANDLER));
    }
  else
    {
      __paf_ebb_thread_info.handler = handler;
    }
}


static inline
__attribute__((always_inline))
void* __paf_ebb_get_thread_context ()
{
  void *ret;
  if (__paf_ebb_use_tcb)
    {
      asm (LOAD_INST " %0,%1(" THREAD_REGISTER ")"
	   : "=r" (ret)
	   : "i" (EBB_CTX_POINTER));
    }
  else
    {
      ret = __paf_ebb_thread_info.context;
    }
  return ret;
}

static inline
__attribute__((always_inline))
void __paf_ebb_set_thread_context (void *context)
{
  if (__paf_ebb_use_tcb)
    {
      asm (STORE_INST " %0,%1(" THREAD_REGISTER ")"
	   : : "r" (context), "i" (EBB_CTX_POINTER));
    }
  else
    {
      __paf_ebb_thread_info.context = context;
    }
}


static inline
__attribute__((always_inline))
int __paf_ebb_get_thread_flags ()
{
  int ret;
  if (__paf_ebb_use_tcb)
    {
      asm (LOAD_INST " %0,%1(" THREAD_REGISTER ")"
	   : "=r" (ret)
	   : "i" (EBB_FLAGS));
    }
  else
    {
      ret = __paf_ebb_thread_info.flags;
    }
  return ret;
}

static inline
__attribute__((always_inline))
void __paf_ebb_set_thread_flags (int flags)
{
  if (__paf_ebb_use_tcb)
    {
      asm (STORE_INST " %0,%1(" THREAD_REGISTER ")"
	   : : "r" (flags), "i" (EBB_FLAGS));
    }
  else
    {
      __paf_ebb_thread_info.flags = flags;
    }
}


#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)

#define mfspr(rn)				\
  ({spr_t spr; 					\
    asm volatile("mfspr %0," __stringify(rn)	\
                 : "=r" (spr));			\
    spr;					\
  })
#define mtspr(rn, v)				\
  asm volatile("mtspr " __stringify(rn) ",%0"	\
	       : : "r" (v))

static inline
__attribute__((always_inline))
void reset_mmcr0 (void)
{
  spr_t val = mfspr (MMCR0);
  mtspr (MMCR0, (val & ~(MMCR0_PMAO | MMCR0_FC)) | MMCR0_PMAE);
}

static inline
__attribute__((always_inline))
void reset_pmcs (void)
{
  mtspr (PMC1, 0);
  mtspr (PMC2, 0);
  mtspr (PMC3, 0);
  mtspr (PMC4, 0);
  mtspr (PMC5, 0);
  mtspr (PMC6, 0);
}

#endif
