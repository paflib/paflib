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

#ifndef _TEST_PPCEBB_COMMON_H
# define _TEST_PPCEBB_COMMON_H

#include "ppcebb-priv.h"

int ebb_check_mmcr0 (void);
const char *ebb_callback_type_string (ppcebb_callback_type_t type);

#endif
