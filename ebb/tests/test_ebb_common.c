/*
 * Event-Based Branch Facility Tests.  Test common functions.
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
ebb_callback_type_string (ppcebb_callback_type_t type)
{
  static const char *callback_types[] =
  {
    "PPCEBB_CALLBACK_GPR_SAVE",
    "PPCEBB_CALLBACK_FPR_SAVE",
    "PPCEBB_CALLBACK_VR_SAVE",
    "PPCEBB_CALLBACK_VSR_SAVE"
  };
  switch (type)
    {
      case PPCEBB_CALLBACK_GPR_SAVE: return callback_types[0];
      case PPCEBB_CALLBACK_FPR_SAVE: return callback_types[1];
      case PPCEBB_CALLBACK_VR_SAVE:  return callback_types[2];
      case PPCEBB_CALLBACK_VSR_SAVE: return callback_types[3];
    }
  return NULL;
}
