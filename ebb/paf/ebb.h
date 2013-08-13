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

#ifndef _PAF_EBB_H
#define _PAF_EBB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*ebbhandler_t)(void *);
#define EBB_REG_ERR ((ebbhandler_t) -1)

typedef enum
{
  PAF_EBB_CALLBACK_GPR_SAVE,
#define PAF_EBB_CALLBACK_GPR_SAVE PAF_EBB_CALLBACK_GPR_SAVE
  PAF_EBB_CALLBACK_FPR_SAVE,
#define PAF_EBB_CALLBACK_FPR_SAVE PAF_EBB_CALLBACK_FPR_SAVE
  PAF_EBB_CALLBACK_VR_SAVE,
#define PAF_EBB_CALLBACK_VR_SAVE PAF_EBB_CALLBACK_VR_SAVE
  PAF_EBB_CALLBACK_VSR_SAVE,
#define PAF_EBB_CALLBACK_VSR_SAVE PAF_EBB_CALLBACK_VSR_SAVE
} paf_ebb_callback_type_t;

#define PAF_EBB_FLAGS_RESET_PMU	0x1

int paf_ebb_pmu_init (uint64_t raw_event, int group);
void paf_ebb_pmu_reset (void);

ebbhandler_t paf_ebb_handler (void);
ebbhandler_t paf_ebb_register_handler (ebbhandler_t handler, void *context,
  paf_ebb_callback_type_t type, int flags);
int paf_ebb_enable_branches (void);
int paf_ebb_disable_branches (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
