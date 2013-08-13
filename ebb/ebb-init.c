/*
 * Event-Based Branch Facility API.  Perf-thread data function init.
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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gnu/libc-version.h>

#include "config.h"
#include "ebb-priv.h"
#include "ebb-init.h"

/* Sets if GLIBC supports the EBB fields (handler and context) in the
 * Thread Control Block.  */
int __ppcebb_use_tcb = 0;

#ifndef USE_EBB_TCB
static inline const char *
__ppcebb_init_readnumber (const char *str, int *ret)
{
#define MAX_NUMBER_LEN 16
  char number[MAX_NUMBER_LEN];
  char *endptr;
  int i;

  memset (number, 0, MAX_NUMBER_LEN);
  for (i=0; (i<MAX_NUMBER_LEN) && (str[i] != '\0'); ++i)
    {
      if (str[i] == '.')
	{
	  i++;
	  break;
 	}
      number[i] = str[i];
    }
  *ret = strtol (number, &endptr, 10);
  if ((errno == ERANGE) || (*endptr != '\0'))
    return NULL;
  return &str[i];
}
#endif /* USE_EBB_TCB  */

/* Check GLIBC version to see if interneal TCB header supports or
 * not the EBB fields.  */
void
attribute_hidden
attribute_constructor
__ppcebb_init_tcb_usage (void)
{
#if defined(USE_EBB_TCB)
  __ppcebb_use_tcb = 1;
#elif defined(USE_EBB_TLS)
  __ppcebb_use_tcb = 0;
#else
  const char *glibc_release;
  int major;
  int minor;

  glibc_release = gnu_get_libc_version ();

  glibc_release = __ppcebb_init_readnumber (glibc_release, &major);
  if (glibc_release == NULL)
    {
      //WARN ("failed to parse GLIBC version");
      return;
    }

  if (major >= 3)
    __ppcebb_use_tcb = 1;
  else
    {
      glibc_release = __ppcebb_init_readnumber (glibc_release, &minor);
      if (minor >= 18)
        __ppcebb_use_tcb = 1;
    }
#endif
}
