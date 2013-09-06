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
#include "paf-common.h"

#ifndef PPC_FEATURE_HAS_ALTIVEC
# define PPC_FEATURE_HAS_ALTIVEC 0x10000000
#endif
#ifndef PPC_FEATURE2_HAS_EBB
# define PPC_FEATURE2_HAS_EBB    0x10000000
#endif

int __paf_get_hwcap (uint32_t *hwcap1, uint32_t *hwcap2) attribute_hidden;

#endif
