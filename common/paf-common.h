/*
 * PAFlib common definitions.
 *
 * Copyright IBM Corp. 2013
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * Contributors:
 *     IBM Corporation, Adhemerval Zanella - Initial implementation.
 */

#ifndef _PAF_COMMON_H
# define _PAF_COMMON_H

#define attribute_hidden       __attribute__ ((visibility ("hidden")))
#define attribute_constructor  __attribute__ ((constructor))
#define attribute_initial_exec __attribute__ ((tls_model ("initial-exec")))
#define attribute_noinline     __attribute__ ((noinline))


typedef unsigned long int spr_t;

#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)

#define mfspr(rn)					\
  ({ spr_t spr; 					\
     asm volatile("mfspr %0," __stringify(rn)		\
                  : "=r" (spr));			\
     spr;						\
  })
#define mtspr(rn, v)					\
  ({ spr_t spr = (spr_t)v;				\
     asm volatile("mtspr " __stringify(rn) ",%0"	\
	       : : "r" (spr));				\
  })


#ifdef ENABLE_DEBUG
# define DEBUG(fmt, args...) fprintf(stderr, "%s:%d: debug: " fmt "\n", \
  __FUNCTION__, __LINE__, ## args)
#else
# define DEBUG(fmt, args...) do { } while(0)
#endif

#endif
