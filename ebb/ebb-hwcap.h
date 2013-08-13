/*
 * Event-Based Branch Facility API.  Hardware capabilites probe functions.
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

#ifndef _PAF_EBB_HWCAP_H
# define _PAF_EBB_HWCAP_H

#include "ebb-priv.h"

#define PAF_EBB_FEATURE_HAS_ALTIVEC 0x1
#define PAF_EBB_FEATURE_HAS_EBB     0x2

extern int __paf_ebb_hwcap attribute_hidden;

#endif
