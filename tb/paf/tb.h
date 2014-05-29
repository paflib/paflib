/* Time Base Facility API.  API implementation.
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
#ifndef _PAF_TB_H
#define _PAF_TB_H

#include <stdint.h>

/* We use 64bit values for the times.  */
#ifdef __powerpc64__
typedef uint64_t timing;
#else
typedef uint32_t timing;
#endif

static inline timing
paf_timing_now ()
{
  /* Read the Time Base Register.   */
#ifdef __powerpc64__
  timing val;
  __asm__ __volatile__ ("mfspr %0, 268":"=r" (val));
  return val;
#else
    timing hi, lo, tmp;
    __asm__ __volatile__ ("1:   mfspr   %0,269;"
                          "     mfspr   %1,268;"
                          "     mfspr   %2,269;"
                          "     cmpw    %0,%2;"
                          "     bne     1b;"
                          : "=&r" (hi), "=&r" (lo), "=&r" (tmp));
    return (((uint64_t) hi << 32) | lo);
#endif
}

static inline void 
paf_timing_accum (timing * sum, timing diff)
{
  *sum += diff;
}

static inline timing 
paf_timing_diff (timing start, timing end)
{
  timing diff;
  diff = end - start;
  return diff;
}

#endif
