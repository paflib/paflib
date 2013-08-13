/*
 * Event-Based Branch Facility Tests.
 *
 * Copyright (c) 2013 IBM Corporation.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
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
  handler = ppcebb_register_handler (NULL, NULL, PPCEBB_CALLBACK_GPR_SAVE, 0);
  if (handler != EBB_REG_ERR)
    {
      printf ("Error: ppc_register_ebb_handler (NULL) != EBB_REG_ERR\n");
      return -1;
    }

  bescr = get_bescr ();
  printf ("Initial EBB state: bescr %lx\n", bescr);

  ret = ppcebb_enable_branches ();
  if (ret != 0)
    {
      printf ("Error: ppc_enable_branches () returned %i\n", ret);
      return -1;
    }

  /* To check agains both Global Enable (GE - bit 0) and Performance Monitor
     Event Exception Enable (PME - bit 31).  */
  mask = 1 | (1 << 31);

  /* Check if both GE (bit 0) and PME (bit 31) are set.  */
  bescr = get_bescr ();
  if (!(bescr & mask))
    {
      printf ("Error: ppc_enable_branches () resulted in 0x%lx\n", bescr); 
      return -1;
    }
  printf ("Enable EBB       : bescr %lx\n", bescr);

  ppcebb_disable_branches ();
  if (ret != 0)
    {
      printf ("Error: ppc_disable_eb_branches () returned %i\n", ret);
      return -1;
    }

  /* Check if both GE (bit 0) and PME (bit 31) are not set.  */
  bescr = get_bescr ();
  if (bescr & mask)
    {
      printf ("Error: ppc_disable_eb_branches () resulted in 0x%lx\n", bescr);
      return -1;
    }
  printf ("Disable EBB      : bescr %lx\n", bescr);

  /* Register a valid callback and check its address.  */
  handler = ppcebb_register_handler (ebb_handler_test, &ebb_context_test,
				     PPCEBB_CALLBACK_GPR_SAVE, 0);
  if (handler != ebb_handler_test)
    {
      printf ("Error: ppc_register_ebb_handler \
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
