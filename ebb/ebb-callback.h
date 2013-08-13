/*
 * Event-Based Branch Facility API.  Callback definitions.
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

#ifndef _PAF_EBB_CALLBACK_H
# define _PAF_EBB_CALLBACK_H

void __paf_ebb_callback_handler_gpr (void);
void __paf_ebb_callback_handler_fpr (void);
void __paf_ebb_callback_handler_vr  (void);
void __paf_ebb_callback_handler_vsr (void);

#endif
