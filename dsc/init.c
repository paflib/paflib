/* libpaf-dsc constructor function. */
#include <paf/dsc.h>

#include "config.h"
#include "common.h"
#include "hwcap.h"

void
__attribute__ ((constructor))
attribute_hidden
__paf_dsc_init (void)
{
  paf_dsc_check_support();
}
