/* Event-Based Branch Facility API.  Common definitions.
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

#ifndef _PAF_EBB_COMMON_H
#define _PAF_EBB_COMMON_H

/* Constants used to get/set the EBB register through m[t|f]spr.  */
#define MMCR0       779  /* Monitor Mode Control Register 0.  */
#define MMCRA       770
#define MMCR1       782
#define MMCR2       769
#define MMCR0_FC    0x80000000
#define MMCR0_PMAO  0x00000080
#define MMCR0_PMAE  0x04000000

#define PMC1        771
#define PMC2        772
#define PMC3        773
#define PMC4        774
#define PMC5        775
#define PMC6        776

#define BESCRS      800  /* Branch Event Status and Control Set.  */
#define BESCRSU     801  /* Branch Event Status and Control Set.
                            (Upper 32 bits).  */
#define BESCRR      802  /* Branch Event Status and Control Reset.  */
#define BESCRRU     803  /* Branch Event Status and Control Reset
                            (Upper 32 bits).  */

#define BESCR_PMEO  0x1  /* PMU Event-based exception Occurred.   */
#define BESCR_PMEU  0x1  /* PMU Event-based exception Enable (Upper).  */

#define EBBHR       804  /* Event-Based Branch Handler Register.  */
#define EBBRR       805  /* Event-Based Branch Return Register.  */
#define BESCR       806  /* Branch Event Status and Control Register.  */

#endif
