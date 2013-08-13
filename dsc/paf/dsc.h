#ifndef _PPCDSC_H
#define _PPCDSC_H

/* ISA 2.05 features (POWER6) */
#define DSCR_DPFD_MAX	7	/* Default Prefetch Depth max value */
#define DSCR_DPFD(val) (val & DSCR_DPFD_MAX) /* Default Prefetch Depth mask */
#define DSCR_DPFD_UNMASK(val) (val & DSCR_DPFD_MAX)
#define DSCR_SSE   (1 << 3) /* Store Stream Enable */
#define DSCR_ISA_2_05_MASK	(DSCR_SSE | DSCR_DPFD(DSCR_DPFD_MAX))

/* ISA 2.06 features (POWER7) */
#define DSCR_SNSE  (1 << 4) /* Stride-N Stream Enable */
#define DSCR_LSD   (1 << 5) /* Load Stream Disable */
#define DSCR_ISA_2_06_MASK	(DSCR_ISA_2_05_MASK | DSCR_SNSE | DSCR_LSD)

/* ISA 2.06+ feature (POWER7+) */
#define DSCR_URG_MAX	7	/* Depth Attainment Urgency max value */
#define DSCR_URG(val) ((val & DSCR_URG_MAX) << 6) /* Depth Attainment Urgency mask */
#define DSCR_URG_UNMASK(val) ((val >> 6) & DSCR_URG_MAX)
#define DSCR_ISA_2_06p_MASK	(DSCR_ISA_2_06_MASK | DSCR_URG(DSCR_URG_MAX))

/* ISA 2.07 features (POWER8) */
#define DSCR_UNITCNT_MAX	1023 /* Number of units in data stream max value*/
#define DSCR_UNITCNT(val)	((val & DSCR_UNITCNT_MAX) << 9) /* Number of units in data stream */
#define DSCR_UNITCNT_UNMASK(val)	((val >> 9) && DSCR_UNITCNT_MAX)
#define DSCR_HWUE	(1 << 19) /* Hardware Unit count Enable */
#define DSCR_SWUE	(1 << 20) /* Software Unit count Enable */
#define DSCR_LTE	(1 << 21) /* Load Transient Enable */
#define DSCR_STE	(1 << 22) /* Store Transient Enable */
#define DSCR_HWTE	(1 << 23) /* Hardware Transient Enable */
#define DSCR_SWTE	(1 << 24) /* Software Transient Enable */
#define DSCR_ISA_2_07_MASK	(DSCR_ISA_2_06p_MASK | \
				 DSCR_UNITCNT(DSCR_UNITCNT_MAX) | DSCR_HWUE | \
				 DSCR_SWUE | DSCR_LTE | DSCR_STE | \
				 DSCR_HWTE | DSCR_SWTE)

#define DSCR_MASK  0x01FFFFFF

/* Returns the DSCR value. If unsupported, it returns -1 and errno is set to
   ENOSYS (DSCR support not available). */
long int ppcdsc_get(void);

/* Sets the DSCR value to 'dscr'. If unsupported, it returns -1 and errno is
   set to ENOSYS in case of DSCR support not available or EINVAL if parameter
   'dscr' is invalid. */
int ppcdsc_set(unsigned long dscr);

#endif /* _PPCDSC_H */
