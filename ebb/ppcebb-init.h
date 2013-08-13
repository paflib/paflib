/*
 * Event-Based Branch Facility API.  Per-thread data init.
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

#ifndef _PPCEBB_INIT_H
# define _PPCEBB_INIT_H

#include "ppcebb-priv.h"

/* Defined if GLIBC allocates the EBB fiels in TCB internal structure.  */
extern int __ppcebb_use_tcb attribute_hidden;

#endif
