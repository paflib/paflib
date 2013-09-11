/* libpaf-dsc published header.
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
 *     IBM Corporation, Edjunior Barbosa Machado - Initial implementation.
 */

#ifndef _PAF_DSC_H
#define _PAF_DSC_H

#include <inttypes.h>

/* Power ISA 2.05 (Power6) Features */

/* Default Prefetch Depth max value */
#define DSCR_DPFD_MAX			7
/* Default Prefetch Depth mask */
#define DSCR_DPFD_MASK			DSCR_DPFD_MAX
/* Macros to get/set Default Prefetch Depth field */
#define DSCR_DPFD_GET(dscr)		(dscr & DSCR_DPFD_MAX)
#define DSCR_DPFD_SET(dscr, value)	((dscr & ~DSCR_DPFD_MASK) | \
					 (value & DSCR_DPFD_MAX))

/* Store Stream Enable bit */
#define DSCR_SSE			(1L << 3)

/* Bitmask of features supported by Power ISA */
#define DSCR_ISA_2_05_MASK		(DSCR_SSE | DSCR_DPFD_MASK)

/* Power ISA 2.06 (Power7) Features */

/* Stride-N Stream Enable bit */
#define DSCR_SNSE			(1L << 4)

/* Load Stream Disable bit */
#define DSCR_LSD			(1L << 5)

/* Bitmask of features supported by Power ISA 2.06 */
#define DSCR_ISA_2_06_MASK		(DSCR_ISA_2_05_MASK | DSCR_SNSE | \
					 DSCR_LSD)

/* Power ISA 2.06+ (Power7) Features */

/* Depth Attainment Urgency max value */
#define DSCR_URG_MAX			7
/* Depth Attainment Urgency mask */
#define DSCR_URG_MASK			(DSCR_URG_MAX << 6)
/* Macros to get/set Depth Attainment Urgency field */
#define DSCR_URG_GET(dscr)		((dscr & DSCR_URG_MASK) >> 6)
#define DSCR_URG_SET(dscr, value)	((dscr & ~DSCR_URG_MASK) | \
					 ((value & DSCR_URG_MAX) << 6))

/* Bitmask of features supported by Power ISA 2.06+ */
#define DSCR_ISA_2_06P_MASK		(DSCR_ISA_2_06_MASK | \
					 DSCR_URG_MASK)

/* Power ISA 2.07 (Power8) Features */

/* Number of units in data stream max value*/
#define DSCR_UNITCNT_MAX		1023
/* Number of units in data stream mask */
#define DSCR_UNITCNT_MASK		(DSCR_UNITCNT_MAX << 9)
/* Macros to get/set number of units in data stream field */
#define DSCR_UNITCNT_GET(dscr)		((dscr & DSCR_UNITCNT_MASK) >> 9)
#define DSCR_UNITCNT_SET(dscr, value)	((dscr & ~DSCR_UNITCNT_MASK) | \
					 ((value & DSCR_UNITCNT_MAX) << 9))

/* Hardware Unit count Enable */
#define DSCR_HWUE	(1L << 19)

/* Software Unit count Enable */
#define DSCR_SWUE	(1L << 20)

/* Load Transient Enable */
#define DSCR_LTE	(1L << 21)

/* Store Transient Enable */
#define DSCR_STE	(1L << 22)

/* Hardware Transient Enable */
#define DSCR_HWTE	(1L << 23)

/* Software Transient Enable */
#define DSCR_SWTE	(1L << 24)

/* Bitmask of features supported by Power ISA 2.07 */
#define DSCR_ISA_2_07_MASK		(DSCR_ISA_2_06P_MASK | \
					 DSCR_UNITCNT_MASK | DSCR_HWUE | \
					 DSCR_SWUE | DSCR_LTE | DSCR_STE | \
					 DSCR_HWTE | DSCR_SWTE)

/* Returns the bitmask of the current DSCR supported features. If unsupported,
   it returns 0 and errno is set to ENOSYS (DSCR support not available). */
uint64_t paf_dsc_check_support(void);

/* Retrieves the current DSCR value on '*dscr'. If unsupported, it returns -1
 * and errno is set to ENOSYS (DSCR support not available). */
int paf_dsc_get(uint64_t *dscr);

/* Sets the DSCR value to 'dscr'. If unsupported, it returns -1 and errno is
   set to ENOSYS in case of DSCR support not available or EINVAL if parameter
   'dscr' is invalid. */
int paf_dsc_set(uint64_t dscr);

#endif /* _PAF_DSC_H */
