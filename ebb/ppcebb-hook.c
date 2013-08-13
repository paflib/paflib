/*
 * Event-Based Branch Facility API.  API implementation.
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
#include "ppcebb-priv.h"

void
attribute_hidden
__ppcebb_ebb_hook (void)
{
  ebbhandler_t handler;
  void *context;
  spr_t val;
  int flags;

  val = mfspr (BESCR);
  if (!(val & BESCR_PMEO))
    goto out;

  handler = __ppcebb_get_thread_handler ();
  context = __ppcebb_get_thread_context ();

  handler (context);

out:
  flags = __ppcebb_get_thread_flags ();
  if (flags & PPCEBB_FLAGS_RESET_PMU)
    {
      reset_mmcr0 ();
      reset_pmcs ();
    }

  mtspr (BESCRR,  BESCR_PMEO);
  mtspr (BESCRSU, BESCR_PMEU);
}
