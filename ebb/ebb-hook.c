/* Event-Based Branch Facility API.  API implementation.
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
#include "ebb-priv.h"

void
attribute_hidden
__paf_ebb_ebb_hook (void)
{
  ebbhandler_t handler;
  void *context;
  spr_t val;
  int flags;
  uint32_t sample_period;

  val = mfspr (BESCR);
  if (!(val & BESCR_PMEO))
    goto out;

  handler = __paf_ebb_get_thread_handler ();
  context = __paf_ebb_get_thread_context ();

  handler (context);

out:
  flags = __paf_ebb_get_thread_flags ();
  if (flags & PAF_EBB_FLAGS_RESET_PMU)
    {
      /* 1) If requested, reset PMC to possible trigger another EBB  */
      uint32_t sample_period = __paf_ebb_get_thread_sample_period ();
      reset_pmcs (sample_period);
    }

  /* 2) clear MMCR0[PMAO] - docs say BESCR[PMEO] should do this */
  /* 3) set MMCR0[PMAE]   - docs say BESCR[PME] should do this */
  reset_mmcr0 ();

  /* 4) clear BESCR[PMEO] */
  mtspr (BESCRR,  BESCR_PMEO);

  /* 5) set BESCR[PME] */
  mtspr (BESCRSU, BESCR_PMEU);

  /* 6) rfebb 1 - done in ebb-callback*.S  */
}
