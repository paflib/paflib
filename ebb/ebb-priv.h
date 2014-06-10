/* Event-Based Branch Facility API. Private definitions.
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

#ifndef _PAF_EBB_PRIV_H
#define _PAF_EBB_PRIV_H

#include <paf/ebb.h>
#include "paf-common.h"
#include "ebb-common.h"
#include "ebb-init.h"

#ifdef __powerpc64__
# define LOAD_INST        "ld"
# define STORE_INST       "std"
# define THREAD_REGISTER  "13"
# define EBB_HANDLER      (-28728)  /* TCB field ebb_handler.  */
# define EBB_CTX_POINTER  (-28720)  /* TCB field ebb_context.  */
# define EBB_FLAGS        (-28712)  /* TCB field ebb_reserved1.  */
# define EBB_SAMPLE_PER   (-28704)  /* TCB field ebb_reserved2.  */
#else
# define LOAD_INST        "lwz"
# define STORE_INST       "stw"
# define THREAD_REGISTER  "2"
# define EBB_HANDLER      (-28700)
# define EBB_CTX_POINTER  (-28696)
# define EBB_FLAGS        (-28692)
# define EBB_SAMPLE_PER   (-28688)
#endif

/* EBB per-thread information to use where TCB fields are not available.  */
struct ebb_thread_info_t
{
  ebbhandler_t handler;
  void *context;
  int flags;
  uint32_t sample_period;
};

extern __thread
attribute_initial_exec
attribute_hidden
struct ebb_thread_info_t __paf_ebb_thread_info;

static inline
attribute_alwaysinline
ebbhandler_t __paf_ebb_get_thread_handler ()
{
  ebbhandler_t ret;
  if (__paf_ebb_use_tcb)
    asm (LOAD_INST " %0,%1(" THREAD_REGISTER ")"
	 : "=r" (ret)
	 : "i" (EBB_HANDLER));
  else
      ret = __paf_ebb_thread_info.handler;
  return ret;
}

static inline
attribute_alwaysinline
void __paf_ebb_set_thread_handler (ebbhandler_t handler)
{
  if (__paf_ebb_use_tcb)
    asm (STORE_INST " %0,%1(" THREAD_REGISTER ")"
	 : : "r" (handler), "i" (EBB_HANDLER));
  else
    __paf_ebb_thread_info.handler = handler;
}

static inline
attribute_alwaysinline
void* __paf_ebb_get_thread_context ()
{
  void *ret;
  if (__paf_ebb_use_tcb)
    asm (LOAD_INST " %0,%1(" THREAD_REGISTER ")"
	 : "=r" (ret)
	 : "i" (EBB_CTX_POINTER));
  else
      ret = __paf_ebb_thread_info.context;
  return ret;
}

static inline
attribute_alwaysinline
void __paf_ebb_set_thread_context (void *context)
{
  if (__paf_ebb_use_tcb)
    asm (STORE_INST " %0,%1(" THREAD_REGISTER ")"
	 : : "r" (context), "i" (EBB_CTX_POINTER));
  else
    __paf_ebb_thread_info.context = context;
}

static inline
attribute_alwaysinline
int __paf_ebb_get_thread_flags ()
{
  int ret;
  if (__paf_ebb_use_tcb)
    asm (LOAD_INST " %0,%1(" THREAD_REGISTER ")"
	 : "=r" (ret)
	 : "i" (EBB_FLAGS));
  else
    ret = __paf_ebb_thread_info.flags;
  return ret;
}

static inline
attribute_alwaysinline
void __paf_ebb_set_thread_flags (int flags)
{
  if (__paf_ebb_use_tcb)
    asm (STORE_INST " %0,%1(" THREAD_REGISTER ")"
	 : : "r" (flags), "i" (EBB_FLAGS));
  else
    __paf_ebb_thread_info.flags = flags;
}

static inline
attribute_alwaysinline
uint32_t
__paf_ebb_get_thread_sample_period (void)
{
  uint32_t ret;
  if (__paf_ebb_use_tcb)
    asm (LOAD_INST " %0,%1(" THREAD_REGISTER ")"
	 : "=r" (ret)
	 : "i" (EBB_SAMPLE_PER));
  else
    ret = __paf_ebb_thread_info.sample_period;
  return ret;
}

static inline
attribute_alwaysinline
void
__paf_ebb_set_thread_sample_period (uint32_t sample_period)
{
  if (__paf_ebb_use_tcb)
    asm (STORE_INST " %0,%1(" THREAD_REGISTER ")"
	 : : "r" (sample_period), "i" (EBB_SAMPLE_PER));
  else
    __paf_ebb_thread_info.sample_period = sample_period;
}

static inline
attribute_alwaysinline
void reset_mmcr0 (void)
{
  spr_t val = mfspr (MMCR0);
  mtspr (MMCR0, (val & ~(MMCR0_PMAO | MMCR0_FC)) | MMCR0_PMAE);
}

static inline
attribute_alwaysinline
void reset_pmcs (uint32_t sample_period)
{
  /* Performance monitor interrupts are generated when the value in an enabled
     PMC transitions to where the most significant bit becomes '1'.  If a useri
     wants to get an event based interrupt, say, every 1,000,000 cycles, then
     the PMC should be configured to count the cycles event and its initial
     value should be set to 0x80000000 - 1000000.  */
  sample_period = 0x80000000UL - sample_period;

  mtspr (PMC1, sample_period);
  mtspr (PMC2, sample_period);
  mtspr (PMC3, sample_period);
  mtspr (PMC4, sample_period);
  mtspr (PMC5, sample_period);
  mtspr (PMC6, sample_period);
}

#endif
