#include <stdio.h>
#include <errno.h>
#include <ppcdsc.h>

int main(void)
{
  long int dscr;

  if ((dscr = ppcdsc_get()) < 0)
    {
      perror("Cannot get dscr value.");
      return -1;
    }

  /* DSCR_LSD available only in ISA 2.06 */
  if (ppcdsc_set(dscr | DSCR_LSD) < 0)
    {
      perror("Cannot set DSCR_LSD flag (available only in ISA 2.06).");
      return -1;
    }
  if ((dscr = ppcdsc_get()) < 0)
    {
      perror("Cannot get dscr value.");
      return -1;
    }
  if ((dscr & DSCR_LSD) != DSCR_LSD)
    {
      fprintf(stderr, "DSCR_LSD not set (returned 0x%lx).\n", dscr);
      return -1;
    }

  /* DSCR_URG available only in ISA 2.06+ */
  if (ppcdsc_set(dscr | DSCR_URG(7)) < 0)
    {
      perror("Cannot set DSCR_URG flag (available only in ISA 2.06+).");
      return -1;
    }
  if ((dscr = ppcdsc_get()) < 0)
    {
      perror("Cannot get dscr value.");
      return -1;
    }
  if ((dscr & DSCR_URG(7)) != DSCR_URG(7))
    {
      fprintf(stderr, "DSCR_URG not set (returned 0x%lx).\n", dscr);
      return -1;
    }

  /* DSCR_HWUE available only in ISA 2.07 */
  if (ppcdsc_set(dscr | DSCR_HWUE) < 0)
    {
      perror("Cannot set DSCR_HWUE flag (available only in ISA 2.07).\n");
      return -1;
    }
  if ((dscr = ppcdsc_get()) < 0)
    {
      perror("Cannot get dscr value.");
      return -1;
    }
  if ((dscr & DSCR_HWUE) != DSCR_HWUE)
    {
      fprintf(stderr, "DSCR_HWUE not set (returned 0x%lx).\n", dscr);
      return -1;
    }

  return 0;
}
