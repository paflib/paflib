/* Program Priority Register Facility API.  API implementation.
 *
 * Copyright IBM Corp. 2014
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
 *     IBM Corporation, Rajalakshmi S - Initial implementation.
 */
#ifndef _PAF_PPR_H
#define _PAF_PPR_H

/* The following functions provide hints about the usage of shared processor
   resources, as defined in ISA 2.06 and newer. */

/* Provides a hint that performance will probably be improved if shared
   resources dedicated to the executing processor are released for use by other
   processors.  */
static inline void
paf_yield (void)
{
  __asm__ volatile ("or 27,27,27");
}

/* Provides a hint that performance will probably be improved if shared
   resources dedicated to the executing processor are released until
   all outstanding storage accesses to caching-inhibited storage have been
   completed.  */
static inline void
paf_mdoio (void)
{
  __asm__ volatile ("or 29,29,29");
}

/* Provides a hint that performance will probably be improved if shared
   resources dedicated to the executing processor are released until all
   outstanding storage accesses to cacheable storage for which the data is not
   in the cache have been completed.  */
static inline void
paf_mdoom (void)
{
  __asm__ volatile ("or 30,30,30");
}

/* ISA 2.05 and beyond support the Program Priority Register (PPR) to adjust
   thread priorities based on lock acquisition, wait and release. The ISA
   defines the use of form 'or Rx,Rx,Rx' as the way to modify the PRI field.
   The unprivileged priorities are:
     Rx = 1 (low)
     Rx = 2 (medium)
     Rx = 6 (medium-low/normal)
   The 'or' instruction form is a nop in previous hardware, so it is safe to
   use unguarded. The default value is 'medium'.
   From ISA 2.07:
   Problem-state programs may only set values 1,2 and 6.
   Privileged programs may set values 1,2,6,5 and 3.
   If a program attempts to set a value that is not available to it,
   the PRI field remains unchanged.

     Rx = 5 (medium high)
     Rx = 3 (high)
 */

static inline void
paf_ppr_med (void)
{
  __asm__ volatile ("or 2,2,2");
}

static inline void
paf_ppr_med_low (void)
{
  __asm__ volatile ("or 6,6,6");
}

static inline void
paf_ppr_low (void)
{
  __asm__ volatile ("or 1,1,1");
}

static inline void
paf_ppr_med_high (void)
{
  __asm__ volatile ("or 5,5,5");
}

static inline void
paf_ppr_high (void)
{
  __asm__ volatile ("or 3,3,3");
}

#endif
