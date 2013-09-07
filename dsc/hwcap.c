/* libpaf-dsc hwcap testing.
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

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <paf/dsc.h>
#include "paf-hwcap.h"
#include "common.h"
#include "config.h"

int
attribute_hidden
check_hwcap(void)
{
  struct hwcap_t hwcap;

  if (__paf_get_hwcap (&hwcap) != 0)
    {
      DEBUG ("Error reading hardware capabilities");
      return 0;
    }

  /* check for DSCR support on AT_HWCAP2, if available */
  if (hwcap.hwcap2 & PPC_FEATURE2_ARCH_2_07 &&
      hwcap.hwcap2 & PPC_FEATURE2_HAS_DSCR)
    {
      DEBUG("ISA 2.07 (POWER8) and PPC_FEATURE2_HAS_DSCR found");
      return DSCR_ISA_2_07_MASK;
    }

  /* check for ISA version based on AT_HWCAP */
  if (hwcap.hwcap1 & PPC_FEATURE_ARCH_2_06)
    {
      /* verify if 'power7+' checking AT_BASE_PLATFORM */
      if (!strcmp(hwcap.platform, "power7+"))
        {
          DEBUG("ISA 2.06+ (POWER7+) found");
          return DSCR_ISA_2_06P_MASK;
	}

      DEBUG("ISA 2.06 (POWER7) found");
      return DSCR_ISA_2_06_MASK;
    }
  else if (hwcap.hwcap1 & PPC_FEATURE_ARCH_2_05)
    {
      DEBUG("ISA 2.05 (POWER6) found");
      return DSCR_ISA_2_05_MASK;
    }

  /* no support available for machines previous then ISA 2.05 */
  DEBUG("POWER5 or older (no support available)");
  return 0;
}

