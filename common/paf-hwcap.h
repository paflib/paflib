/* PAFlib hardware capability handling.
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

#ifndef _PAF_HWCAP_H
# define _PAF_HWCAP_H

#include <stdint.h>
#include "paf-common.h"

#ifndef PPC_FEATURE_ARCH_2_05
# define PPC_FEATURE_ARCH_2_05   0x00001000 /* ISA 2.05 */
#endif
#ifndef PPC_FEATURE_ARCH_2_06
# define PPC_FEATURE_ARCH_2_06   0x00000100 /* ISA 2.06 */
#endif
#ifndef PPC_FEATURE2_ARCH_2_07
# define PPC_FEATURE2_ARCH_2_07  0x80000000 /* ISA 2.07 */ 
#endif
#ifndef PPC_FEATURE2_HAS_EBB
# define PPC_FEATURE2_HAS_EBB    0x10000000 /* Event Base Branching */
#endif
#ifndef PPC_FEATURE2_HAS_DSCR
# define PPC_FEATURE2_HAS_DSCR   0x20000000 /* Data Stream Control Register */ 
#endif

struct hwcap_t
{
  uint32_t hwcap1;
  uint32_t hwcap2;
#define PAFPLATLEN 64
  char     platform[PAFPLATLEN];
};

int __paf_get_hwcap (struct hwcap_t *hwcap) attribute_hidden;

#endif
