#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <paf/dsc.h>
#include "common.h"
#include "config.h"

#ifdef HAVE_GETAUXVAL
#include <sys/auxv.h>
#define get_auxv(type) getauxval(type)
#else /* HAVE_GETAUXVAL */
#ifdef HAVE_LIBAUXV
#include <auxv/auxv.h>
#include <auxv/hwcap.h>
#define get_auxv(type) query_auxv(type)
#else /* HAVE_LIBAUXV */
#error "no support to glibc getauxval() and libauxv found"
#endif /* HAVE_LIBAUXV */
#endif /* HAVE_GETAUXVAL */

/* FIXME: if AT_HWCAP2 definitions are not available */
#ifndef AT_HWCAP2
#define AT_HWCAP2	26 
#endif
#ifndef PPC_FEATURE2_ARCH_2_07
#define PPC_FEATURE2_ARCH_2_07	0x80000000 /* ISA 2.07 */ 
#endif
#ifndef PPC_FEATURE2_HAS_DSCR
#define PPC_FEATURE2_HAS_DSCR	0x20000000 /* Data Stream Control Register */ 
#endif

int
attribute_hidden
check_hwcap(void)
{
  unsigned long int hwcap2, hwcap;
#ifdef HAVE_LIBAUXV
  if (prefetch_auxv ())
    {
      perror ("Cannot prefetch_auxv()");
      return 0;
    }
#endif

  /* check for DSCR support on AT_HWCAP2, if available */
  hwcap2 = (unsigned long int) get_auxv (AT_HWCAP2);
  if (hwcap2 & PPC_FEATURE2_ARCH_2_07 && hwcap2 & PPC_FEATURE2_HAS_DSCR)
    {
      DEBUG("ISA 2.07 (POWER8) and PPC_FEATURE2_HAS_DSCR found");
      return DSCR_ISA_2_07_MASK;
    }

  /* check for ISA version based on AT_HWCAP */
  hwcap = (unsigned long int) get_auxv (AT_HWCAP);
  if (hwcap & PPC_FEATURE_ARCH_2_06)
    {
      /* verify if 'power7+' checking AT_BASE_PLATFORM */
      char *base_platform = NULL;
      base_platform = (char*) get_auxv (AT_BASE_PLATFORM);

      if (!strcmp(base_platform, "power7+"))
        {
          DEBUG("ISA 2.06+ (POWER7+) found");
          return DSCR_ISA_2_06P_MASK;
	}

      DEBUG("ISA 2.06 (POWER7) found");
      return DSCR_ISA_2_06_MASK;
    }
  else if (hwcap & PPC_FEATURE_ARCH_2_05)
    {
      DEBUG("ISA 2.05 (POWER6) found");
      return DSCR_ISA_2_05_MASK;
    }

  /* no support available for machines previous then ISA 2.05 */
  DEBUG("POWER5 or older (no support available)");
  return 0;
}

