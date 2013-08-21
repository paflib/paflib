#ifndef _PAF_DSC_H
#define _PAF_DSC_H

#include <inttypes.h>

/* Default Prefetch Depth max value */
#define DSCR_DPFD_MAX	7
/* Default Prefetch Depth mask */
#define DSCR_DPFD(val) (val & DSCR_DPFD_MAX)
#define DSCR_DPFD_UNMASK(val) (val & DSCR_DPFD_MAX)
/* Store Stream Enable */
#define DSCR_SSE   (1 << 3)
/* Bitmask of features supported by Power ISA 2.05 (Power6) */
#define DSCR_ISA_2_05_MASK	(DSCR_SSE | DSCR_DPFD(DSCR_DPFD_MAX))

/* Stride-N Stream Enable */
#define DSCR_SNSE  (1 << 4)
/* Load Stream Disable */
#define DSCR_LSD   (1 << 5)
/* Bitmask of features supported by Power ISA 2.06 (Power7) */
#define DSCR_ISA_2_06_MASK	(DSCR_ISA_2_05_MASK | DSCR_SNSE | DSCR_LSD)

/* Depth Attainment Urgency max value */
#define DSCR_URG_MAX	7
/* Depth Attainment Urgency mask */
#define DSCR_URG(val) ((val & DSCR_URG_MAX) << 6)
#define DSCR_URG_UNMASK(val) ((val >> 6) & DSCR_URG_MAX)
/* Bitmask of features supported by Power ISA 2.06+ (Power7+) */
#define DSCR_ISA_2_06P_MASK	(DSCR_ISA_2_06_MASK | DSCR_URG(DSCR_URG_MAX))

/* Number of units in data stream max value*/
#define DSCR_UNITCNT_MAX	1023
/* Number of units in data stream */
#define DSCR_UNITCNT(val)	((val & DSCR_UNITCNT_MAX) << 9)
#define DSCR_UNITCNT_UNMASK(val)	((val >> 9) && DSCR_UNITCNT_MAX)
/* Hardware Unit count Enable */
#define DSCR_HWUE	(1 << 19)
/* Software Unit count Enable */
#define DSCR_SWUE	(1 << 20)
/* Load Transient Enable */
#define DSCR_LTE	(1 << 21)
/* Store Transient Enable */
#define DSCR_STE	(1 << 22)
/* Hardware Transient Enable */
#define DSCR_HWTE	(1 << 23)
/* Software Transient Enable */
#define DSCR_SWTE	(1 << 24)
/* Bitmask of features supported by Power ISA 2.07 (Power8) */
#define DSCR_ISA_2_07_MASK	(DSCR_ISA_2_06P_MASK | \
				 DSCR_UNITCNT(DSCR_UNITCNT_MAX) | DSCR_HWUE | \
				 DSCR_SWUE | DSCR_LTE | DSCR_STE | \
				 DSCR_HWTE | DSCR_SWTE)

/* Returns the bitmask of the current DSCR supported features. If unsupported,
   it returns 0 and errno is set to ENOSYS (DSCR support not available). */
uint64_t paf_dsc_check_support(void);

/* Retrieves the current DSCR value on '*dscr'. If unsupported, it returns -1
 * and errno is set to ENOSYS (DSCR support not available). */
int paf_dsc_get(uint64_t *dscr);

/* Sets the DSCR value to 'dscr'. If unsupported, it returns -1 and errno is
   set to ENOSYS in case of DSCR support not available or EINVAL if parameter
   'dscr' is invalid. */
int paf_dsc_set(uint64_t dscr);

#endif /* _PAF_DSC_H */
