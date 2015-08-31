/* Event-Based Branch Facility API.
 *
 * Copyright IBM Corp. 2013, 2014
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

#ifndef _PAF_EBB_H
#define _PAF_EBB_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*ebbhandler_t)(void *);
#define EBB_REG_ERR ((ebbhandler_t) -1)

typedef enum
{
  PAF_EBB_CALLBACK_GPR_SAVE,
#define PAF_EBB_CALLBACK_GPR_SAVE PAF_EBB_CALLBACK_GPR_SAVE
  PAF_EBB_CALLBACK_FPR_SAVE,
#define PAF_EBB_CALLBACK_FPR_SAVE PAF_EBB_CALLBACK_FPR_SAVE
  PAF_EBB_CALLBACK_VR_SAVE,
#define PAF_EBB_CALLBACK_VR_SAVE PAF_EBB_CALLBACK_VR_SAVE
  PAF_EBB_CALLBACK_VSR_SAVE,
#define PAF_EBB_CALLBACK_VSR_SAVE PAF_EBB_CALLBACK_VSR_SAVE
} paf_ebb_callback_type_t;

#define PAF_EBB_FLAGS_RESET_PMU	0x1

int paf_ebb_pmu_init (uint64_t raw_event, int group);
int paf_ebb_pmu_init_ex (uint64_t raw_event, int group, pid_t pid, int cpu,
                         unsigned long flags);
void paf_ebb_pmu_set_period (uint32_t sample_period);
void paf_ebb_pmu_reset (void);
int paf_ebb_event_close (int fd);

ebbhandler_t paf_ebb_handler (void);
ebbhandler_t paf_ebb_register_handler (ebbhandler_t handler, void *context,
  paf_ebb_callback_type_t type, int flags);
int paf_ebb_enable_branches (void);
int paf_ebb_disable_branches (void);

static inline void paf_enable_branches_fast (void)
{
  unsigned long int v = 1UL << 31;
  /* BESCRS (Branch Event Status and Control Set) is 800.  */
  asm volatile ("mtspr 800\n\t"
		: : "r" (v));
}

static inline void paf_disable_branches_fast (void)
{
  unsigned long int v = 1UL << 31;
  /* BESCRR (Branch Event Status and Control Reset) is 802.  */
  asm volatile ("mtspr 802\n\t"
		: : "r" (v));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
