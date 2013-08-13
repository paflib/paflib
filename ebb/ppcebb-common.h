/*
 * Event-Based Branch Facility API.  Common definitions.
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

#ifndef _PPCEBB_COMMON_H
#define _PPCEBB_COMMON_H

/* Constants used to get/set the EBB register through m[t|f]spr.  */
#define MMCR0	    779  /* Monitor Mode Control Register 0.  */
#define MMCR0_FC    0x80000000
#define MMCR0_PMAO  0x00000080
#define MMCR0_PMAE  0x04000000

#define PMC1        771
#define PMC2        772
#define PMC3        773
#define PMC4        774
#define PMC5        775
#define PMC6        776

#define BESCRS      800  /* Branch Event Status and Control Set.  */
#define BESCRSU     801  /* Branch Event Status and Control Set.
                            (Upper 32 bits).  */
#define BESCRR      802  /* Branch Event Status and Control Reset.  */
#define BESCRRU     803  /* Branch Event Status and Control Reset
                            (Upper 32 bits).  */

#define BESCR_PMEO  0x1  /* PMU Event-based exception Occurred.   */
#define BESCR_PMEU  0x1  /* PMU Event-based exception Enable (Upper).  */

#define EBBHR       804  /* Event-Based Branch Handler Register.  */
#define EBBRR       805  /* Event-Based Branch Return Register.  */
#define BESCR       806  /* Branch Event Status and Control Register.  */

#endif
