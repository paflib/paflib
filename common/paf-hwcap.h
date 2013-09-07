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

#ifndef _PAF_HWCAP_H
# define _PAF_HWCAP_H

#include <stdint.h>
#include <sys/auxv.h>
#include "paf-common.h"

#ifndef PPC_FEATURE2_ARCH_2_07
# define PPC_FEATURE2_ARCH_2_07  0x80000000 /* ISA 2.07 */ 
#endif
#ifndef PPC_FEATURE2_HAS_EBB
# define PPC_FEATURE2_HAS_EBB    0x10000000 /* Event Base Branching */
#endif
#ifndef PPC_FEATURE2_HAS_DSCR
# define PPC_FEATURE2_HAS_DSCR   0x20000000 /* Data Stream Control Register */ 
#endif

struct hwcap_t
{
  uint32_t hwcap1;
  uint32_t hwcap2;
#define PAFPLATLEN 64
  char     platform[PAFPLATLEN];
};

int __paf_get_hwcap (struct hwcap_t *hwcap) attribute_hidden;

#endif
