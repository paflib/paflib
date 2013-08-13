/* libpaf-dsc constructor function. */

#include "config.h"
#include "common.h"
#include "hwcap.h"

void
__attribute__ ((constructor))
attribute_hidden
__paf_dsc_init (void)
{
  check_dscr_support();
}
