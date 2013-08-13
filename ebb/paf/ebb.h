/*
 * Event-Based Branch Facility API.
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

#ifndef _PPCEBB_H
#define _PPCEBB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*ebbhandler_t)(void *);
#define EBB_REG_ERR ((ebbhandler_t) -1)

typedef enum
{
  PPCEBB_CALLBACK_GPR_SAVE,
#define PPCEBB_CALLBACK_GPR_SAVE PPCEBB_CALLBACK_GPR_SAVE
  PPCEBB_CALLBACK_FPR_SAVE,
#define PPCEBB_CALLBACK_FPR_SAVE PPCEBB_CALLBACK_FPR_SAVE
  PPCEBB_CALLBACK_VR_SAVE,
#define PPCEBB_CALLBACK_VR_SAVE PPCEBB_CALLBACK_VR_SAVE
  PPCEBB_CALLBACK_VSR_SAVE,
#define PPCEBB_CALLBACK_VSR_SAVE PPCEBB_CALLBACK_VSR_SAVE
} ppcebb_callback_type_t;

#define PPCEBB_FLAGS_RESET_PMU	0x1

int ppcebb_pmu_init (uint64_t raw_event, int group);
void ppcebb_pmu_reset (void);

ebbhandler_t ppcebb_handler (void);
ebbhandler_t ppcebb_register_handler (ebbhandler_t handler, void *context,
  ppcebb_callback_type_t type, int flags);
int ppcebb_enable_branches (void);
int ppcebb_disable_branches (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
