/* libpaf-dsc test suite.
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
 *     IBM Corporation, Edjunior Barbosa Machado - Initial implementation.
 */

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <paf/dsc.h>

int main(void)
{
  uint64_t dscr, dscr_support;
  if ((dscr_support = paf_dsc_check_support()) == 0)
    {
      if (errno == ENOSYS)
	{
          fprintf(stderr, "DSCR not available.\n");
	  return 0;
	}
      else
	{
	  perror("Error checking DSCR availability.");
	  return -1;
	}
    }
      
  if ((paf_dsc_get(&dscr)) < 0)
    {
      perror("Cannot get dscr value.");
      return -1;
    }

  dscr = DSCR_DPFD_SET(dscr, DSCR_DPFD_MAX);

  /* DSCR_DPFD is available since ISA 2.05 */
  if ((dscr_support | dscr) != dscr_support)
    {
      fprintf(stderr, "DSCR_DPFD feature was supposed to be available (it is provided since DSCR initial support on ISA 2.05).\n");
      return -1;
    }
  else
    {
      uint64_t dscr_aux;
      if (paf_dsc_set(dscr) < 0)
	{
	  perror("Cannot set DSCR_DPFD field.");
	  return -1;
	}
      if (paf_dsc_get(&dscr_aux) < 0)
	{
	  perror("Cannot get dscr value.");
	  return -1;
	}
      if (dscr != dscr_aux)
	{
	  fprintf(stderr, "DSCR_DPFD_MAX not set properly (returned 0x%" PRIx64 ").\n", dscr_aux);
	  return -1;
	}
    }
  /* DSCR_LSD is available since ISA 2.06 */
  if ((dscr_support | DSCR_LSD) != dscr_support)
      fprintf(stderr, "DSCR_LSD flag is not supported (available only in ISA 2.06 and newer).\n");
  else
    {
      if (paf_dsc_set(dscr | DSCR_LSD) < 0)
	{
	  perror("Cannot set DSCR_LSD flag.");
	  return -1;
	}
      if (paf_dsc_get(&dscr) < 0)
	{
	  perror("Cannot get dscr value.");
	  return -1;
	}
      if ((dscr & DSCR_LSD) != DSCR_LSD)
	{
	  fprintf(stderr, "DSCR_LSD not set (returned 0x%" PRIx64 ").\n", dscr);
	  return -1;
	}
    }

  dscr = DSCR_URG_SET(dscr, 7);

  /* DSCR_URG available only in ISA 2.06+ */
  if ((dscr_support | dscr) != dscr_support)
      fprintf(stderr, "DSCR_URG feature is not supported (available in ISA 2.06+ and newer).\n");
  else
    {
      uint64_t dscr_aux;
      if (paf_dsc_set(dscr) < 0)
	{
	  perror("Cannot set DSCR_URG field.");
	  return -1;
        }
      if (paf_dsc_get(&dscr_aux) < 0)
	{
	  perror("Cannot get dscr value.");
	  return -1;
	}
      if (dscr_aux != dscr)
	{
	  fprintf(stderr, "DSCR_URG not set properly (returned 0x%" PRIx64 ").\n", dscr_aux);
	  return -1;
	}
    }

  /* DSCR_HWUE available only in ISA 2.07 */
  if ((dscr_support | DSCR_HWUE) != dscr_support)
      fprintf(stderr, "DSCR_HWUE flag is not supported (available only in ISA 2.07).\n");
  else
    {
      if (paf_dsc_set(dscr | DSCR_HWUE) < 0)
	{
	  perror("Cannot set DSCR_HWUE flag (available only in ISA 2.07).\n");
          return -1;
        }
      if (paf_dsc_get(&dscr) < 0)
	{
	  perror("Cannot get dscr value.");
	  return -1;
	}
      if ((dscr & DSCR_HWUE) != DSCR_HWUE)
	{
	  fprintf(stderr, "DSCR_HWUE not set (returned 0x%" PRIx64 ").\n", dscr);
	  return -1;
	}
    }

  /* disable DSCR */
  if (paf_dsc_set(0) < 0)
    {
      fprintf(stderr, "Cannot fully disable DSCR.\n");
      return -1;
    }
  if (paf_dsc_get(&dscr) < 0)
    {
      perror("Cannot get dscr value.");
      return -1;
    }
  if (dscr != 0)
    {
      fprintf(stderr, "DSCR is not fully disabled (returned 0x%" PRIx64 ").\n", dscr);
      return -1;
    }

  return 0;
}
