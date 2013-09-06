/*
 * PAFlib hardware capability handling.
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

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <link.h>
#include <sys/stat.h>

#include "paf-hwcap.h"

#ifndef AT_HWCAP2
# define AT_HWCAP2 26
#endif

/* Although glibc 2.16+ provides getauxval, only 2.18+ provides access
 * to AT_HWCAP2. To avoid rely on glibc version to correctly discover if
 * the kernel supports EBB, parse the hardware capabilities bits using the
 * either the /proc/self/auxv or the environment directly (__USE_ENVIRON).  */

/*#define __USE_ENVIRON*/

int
attribute_hidden
__paf_get_hwcap (uint32_t *hwcap1, uint32_t *hwcap2)
{
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
    return 1;
  auxv = (ElfW(auxv_t)*) malloc (page_size);
  if (auxv == NULL)
    return 1;
  bytes = read (auxv_f, (void*)auxv, page_size);
  close (auxv_f);
  if (bytes <= 0)
    {
      free (auxv);
      return 1;
    }
#endif

  for (i=0; auxv[i].a_type != AT_NULL; ++i)
    {
      if (auxv[i].a_type == AT_HWCAP)
	{
	  if (hwcap1)
	    *hwcap1 = auxv[i].a_un.a_val;
	}
      else if (auxv[i].a_type == AT_HWCAP2)
	{
	  if (hwcap2)
	    *hwcap2 = auxv[i].a_un.a_val;
	}
    }

#ifndef __USE_ENVIRON
  free (auxv);
#endif

  return 0;
}
