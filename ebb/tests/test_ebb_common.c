/* Event-Based Branch Facility Tests.  Test common functions.
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

#include <stdio.h>

#include "test_ebb_common.h"

/* Called outside of the EBB handler to check MMCR0 is sane */
int
ebb_check_mmcr0 (void)
{
  unsigned long int val = mfspr (MMCR0);
  if ((val & (MMCR0_FC | MMCR0_PMAO)) == MMCR0_FC) {
    /* It's OK if we see FC & PMAO, but not FC by itself */
    printf("Outside of loop, only FC set 0x%lx\n", val);
    return 1;
  }
  return 0;
}

const char *
ebb_callback_type_string (paf_ebb_callback_type_t type)
{
  static const char *callback_types[] =
  {
    "PAF_EBB_CALLBACK_GPR_SAVE",
    "PAF_EBB_CALLBACK_FPR_SAVE",
    "PAF_EBB_CALLBACK_VR_SAVE",
    "PAF_EBB_CALLBACK_VSR_SAVE"
  };
  switch (type)
    {
      case PAF_EBB_CALLBACK_GPR_SAVE: return callback_types[0];
      case PAF_EBB_CALLBACK_FPR_SAVE: return callback_types[1];
      case PAF_EBB_CALLBACK_VR_SAVE:  return callback_types[2];
      case PAF_EBB_CALLBACK_VSR_SAVE: return callback_types[3];
    }
  return NULL;
}
