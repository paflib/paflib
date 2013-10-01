/* Event-Based Branch Facility API.  Hardware capabilities probe functions.
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

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <link.h>
#include <sys/stat.h>

#include "config.h"
#include "paf-hwcap.h"
#include "ebb-hwcap.h"


/* Contains the Hardware Capability from runtime.  */
int __paf_ebb_hwcap = 0;

/* Init the Hardware Capability mask.  */
void
attribute_hidden
attribute_constructor
__paf_ebb_init_hwcap (void)
{
  struct hwcap_t hwcap;

  if (__paf_get_hwcap (&hwcap) != 0)
    {
      /* TODO: print error */
      return;
    }

  __paf_ebb_hwcap |= (hwcap.hwcap2 & PPC_FEATURE2_HAS_EBB) ?
                     PAF_EBB_FEATURE_HAS_EBB : 0;
}
