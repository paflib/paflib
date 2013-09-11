/* Event-Based Branch Facility Tests.
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
#include <inttypes.h>
#include <paf/ebb.h>


#define BESCR   806

static inline unsigned long int
get_bescr (void)
{
  unsigned long int spr;
  asm volatile ("mfspr %0, %1\n\t"
		: "=r"(spr)
		: "i"(BESCR));
  return spr;
}

static int
ebb_context_test = 0;

static void
ebb_handler_test (void *context)
{
  return;
}

static int
ebb_interface_test (void)
{
  ebbhandler_t handler;
  unsigned long int bescr;
  unsigned long int mask;
  int ret;

  /* Register an invalid function to result in an error.  */
  handler = paf_ebb_register_handler (NULL, NULL, PAF_EBB_CALLBACK_GPR_SAVE, 0);
  if (handler != EBB_REG_ERR)
    {
      printf ("Error: paf_ebb_register_handler (NULL) != EBB_REG_ERR\n");
      return -1;
    }

  bescr = get_bescr ();
  printf ("Initial EBB state: bescr %lx\n", bescr);

  ret = paf_ebb_enable_branches ();
  if (ret != 0)
    {
      printf ("Error: paf_ebb_enable_branches () returned %i\n", ret);
      return -1;
    }

  /* To check agains both Global Enable (GE - bit 0) and Performance Monitor
     Event Exception Enable (PME - bit 31).  */
  mask = 1 | (1 << 31);

  /* Check if both GE (bit 0) and PME (bit 31) are set.  */
  bescr = get_bescr ();
  if (!(bescr & mask))
    {
      printf ("Error: get_descr () resulted in 0x%lx\n", bescr); 
      return -1;
    }
  printf ("Enable EBB       : bescr %lx\n", bescr);

  paf_ebb_disable_branches ();
  if (ret != 0)
    {
      printf ("Error: paf_ebb_disable_branches () returned %i\n", ret);
      return -1;
    }

  /* Check if both GE (bit 0) and PME (bit 31) are not set.  */
  bescr = get_bescr ();
  if (bescr & mask)
    {
      printf ("Error: get_bescr () resulted in 0x%lx\n", bescr);
      return -1;
    }
  printf ("Disable EBB      : bescr %lx\n", bescr);

  /* Register a valid callback and check its address.  */
  handler = paf_ebb_register_handler (ebb_handler_test, &ebb_context_test,
				     PAF_EBB_CALLBACK_GPR_SAVE, 0);
  if (handler != ebb_handler_test)
    {
      printf ("Error: paf_ebb_register_handler \
	      (ebb_handler_test) != handler\n");
      return -1;
    }

  return 0;
}

int
main (void)
{
  int ret = 0;

  ret += ebb_interface_test ();

  return ret;
}
