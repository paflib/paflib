/* Event-Based Branch Facility API.
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
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "paf-common.h"

#include "ebb-hwcap.h"
#include "ebb-init.h"

void
attribute_noreturn
__paflib_ebb_main (void)
{
  __paf_ebb_init_hwcap ();
  __paf_ebb_init_usage ();

  printf ("POWER Platform Library " PACKAGE_STRING".\n"
          "Copyright IBM Corp. 2013.\n"
          "Compiled by GNU version "__VERSION__".\n"
          "Runtime information:\n"
          "    System EBB capable: %s\n"
          "    Context data area: %s\n"
          "For bug reporting please contact:\n"
          PACKAGE_BUGREPORT".\n",
          (__paf_ebb_hwcap & PAF_EBB_FEATURE_HAS_EBB) ? "yes" : "no",
          (__paf_ebb_use_tcb) ? "TCB" : "TLS");
  exit (0);
}
