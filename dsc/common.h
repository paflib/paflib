#ifndef _COMMON_H
#define _COMMON_H

#include "config.h"

#define attribute_hidden __attribute__ ((visibility ("hidden")))

#ifdef ENABLE_DEBUG
#define DEBUG(fmt, args...) fprintf(stderr, "%s:%d: debug: " fmt "\n", __FUNCTION__, __LINE__, ## args)
#else
#define DEBUG(fmt, args...) do { } while(0)
#endif

long int check_dscr_support(void);

#endif /* _COMMON_H */
