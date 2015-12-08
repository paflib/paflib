/*
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
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <paf/dsc.h>
#include "config.h"
#include "paf-common.h"

void
attribute_noreturn
__paflib_dsc_main (void)
{
  uint64_t dscr_support = paf_dsc_check_support ();
  const char *dscr_str = NULL;
  if (errno == ENOSYS)
    dscr_str = "not supported";
  else if (dscr_support == DSCR_ISA_2_05_MASK)
    dscr_str = "ISA 2.05 (POWER6)";
  else if (dscr_support == DSCR_ISA_2_06_MASK)
    dscr_str = "ISA 2.06 (POWER7)";
  else if (dscr_support == DSCR_ISA_2_06P_MASK)
    dscr_str = "ISA 2.06+ (POWER7+)";
  else if (dscr_support == DSCR_ISA_2_07_MASK)
    dscr_str = "ISA 2.07 (POWER8)";

  printf ("POWER Platform Library " PACKAGE_STRING".\n"
          "Copyright IBM Corp. 2013.\n"
          "Compiled by GNU version "__VERSION__".\n"
          "Runtime information:\n"
          "    System DSC capability: %s\n"
          "For bug reporting please contact:\n"
          PACKAGE_BUGREPORT".\n", dscr_str);
          
  return;
}
