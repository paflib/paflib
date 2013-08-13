/*
 * Event-Based Branch Facility API.  Hardware capabilities probe functions.
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

#include "config.h"
#include "ppcebb-hwcap.h"

/* Although glibc 2.16+ provides getauxval, only 2.18+ provides access
 * to AT_HWCAP2. To avoid rely on glibc version to correctly discover if
 * the kernel supports EBB, parse the hardware capabilities bits using the
 * either the /proc/self/auxv or the environment directly (__USE_ENVIRON).  */

//#define __USE_ENVIRON

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <link.h>
#include <sys/stat.h>

#ifndef AT_HWCAP2
# define AT_HWCAP2 26
#endif

#ifndef PPC_FEATURE_HAS_ALTIVEC
# define PPC_FEATURE_HAS_ALTIVEC 0x10000000
#endif
#ifndef PPC_FEATURE2_HAS_EBB
# define PPC_FEATURE2_HAS_EBB 0x10000000
#endif

/* Contains the Hardware Capability from runtime.  */
int __ppcebb_hwcap = 0;

/* Init the Hardware Capability mask.  */
void
attribute_hidden
attribute_constructor
__ppcebb_init_hwcap (void)
{
  int hwcap1 = 0;
  int hwcap2 = 0;
  ElfW(auxv_t) *auxv;
  int i;

#ifdef __USE_ENVIRON
  char **environ;

  for (environ = __environ; *environ != NULL; ++environ)
    {
    }
  auxv = (ElfW (auxv_t) *) ++environ;
#else
  int auxv_f;
  const size_t page_size = getpagesize();
  ssize_t bytes;

  auxv_f = open ("/proc/self/auxv", O_RDONLY);
  if (auxv_f == -1)
    {
      // TODO warn error
      return;
    }
  auxv = (ElfW(auxv_t)*) malloc (page_size);
  if (auxv == NULL)
    {
      // TODO warn error
      return;
    }
  bytes = read (auxv_f, (void*)auxv, page_size);
  close (auxv_f);
  if (bytes <= 0)
    {
     free (auxv);
      // TODO warn error
      return;
    }
#endif

  for (i=0; auxv[i].a_type != AT_NULL; ++i)
    {
      if (auxv[i].a_type == AT_HWCAP)
	hwcap1 = auxv[i].a_un.a_val;
      else if (auxv[i].a_type == AT_HWCAP2)
	hwcap2 = auxv[i].a_un.a_val;
    }

#ifndef __USE_ENVIRON
  free (auxv);
#endif

  __ppcebb_hwcap |= (hwcap1 & PPC_FEATURE_HAS_ALTIVEC) ?
                   PPCEBB_FEATURE_HAS_ALTIVEC : 0;
  __ppcebb_hwcap |= (hwcap2 & PPC_FEATURE2_HAS_EBB) ?
                   PPCEBB_FEATURE_HAS_EBB : 0;
}
