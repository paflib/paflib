/* Event-Based Branch Facility Tests.  Skeleton framwork.
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

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef TIMEOUT
# define TIMEOUT 10
#endif

static const char *program = NULL;
static pid_t pid;

static void
__attribute__ ((noreturn))
signal_handler (int sig __attribute__ ((unused)))
{
  int killed;
  int status;

  /* Send signal.  */
  kill (pid, SIGKILL);

  /* Wait for it to terminate.  */
  int i;
  for (i = 0; i < 5; ++i)
    {   
      killed = waitpid (pid, &status, WNOHANG|WUNTRACED);
      if (killed != 0)
        break;

      struct timespec ts; 
      ts.tv_sec = 0;
      ts.tv_nsec = 100000000;
      nanosleep (&ts, NULL);
    }   
  if (killed != 0 && killed != pid)
    {   
      fprintf (stderr, "%s: error: failed to kill test process: %s",
               program, strerror (errno));
      exit (EXIT_FAILURE);
    }   

  if (killed == 0 || (WIFSIGNALED (status) && WTERMSIG (status) == SIGKILL))
    fprintf (stderr, "%s: error: timed out, killed the child process\n",
             program);
  else if (WIFSTOPPED (status))
    fprintf (stderr, "%s: error: timed out: the child process was %s\n",
             program, strsignal (WSTOPSIG (status)));
  else if (WIFSIGNALED (status))
    fprintf (stderr, "%s: timed out: the child process got signal %s\n",
             program, strsignal (WTERMSIG (status)));
  else
    fprintf (stderr, "%s: timed out: killed the child process but it exited %d\n",
             program, WEXITSTATUS (status));

  exit (EXIT_FAILURE);
}


static struct option options[] =
{
#define OPT_DIRECT 1000
  { "direct", no_argument, NULL, OPT_DIRECT },
  { NULL, 0, NULL, 0 } 
};

int
main (int argc, char *argv[])
{
  int direct = 0;
  pid_t termpid;
  int opt;
  int status;

  program = argv[0];

  /* Make sure we see all message, even those on stdout.  */
  setvbuf (stdout, NULL, _IONBF, 0);

  while ((opt = getopt_long (argc, argv, "+", options, NULL)) != -1)
    switch (opt)
      {
      case '?':
        exit (1);
      case OPT_DIRECT:
        direct = 1;
        break;
      }

  if (direct)
    return EBB_TEST ();

  /* Set up the test environment:
   * 1. set up the timer
   * 2. fork and execute the function.  */

  pid = fork (); 
  if (pid == 0)
    {   
      /* We put the test process in its own group so that if it bogusly
         generates any job control signals, they won't hit the whole build.  */
      setpgid (0, 0);

      /* Execute the test function and exit with the return value.   */
      exit (EBB_TEST ());
    }
  else if (pid < 0)
    {
      printf ("%s: error: fork failed: %s\n", program, strerror (errno));
      exit (EXIT_FAILURE);
    }

  signal (SIGALRM, signal_handler);
  alarm (TIMEOUT);

  /* Make sure we clean up if the wrapper gets interrupted.  */
  signal (SIGINT, signal_handler);

  do {
    termpid = waitpid (pid, &status, 0);
  } while (termpid == -1 && errno == EINTR);

  if (termpid == -1)
    {
      fprintf (stderr, "%s: error: waiting for test program failed: %m\n",
               program);
      exit (EXIT_FAILURE);
    }
  else if (termpid != pid)
    {
      fprintf (stderr, "%s: error: wrong test program terminated: expected "
	       "%ld, got %ld\n", program,
              (long int) pid, (long int) termpid);
      exit (EXIT_FAILURE);
    }

  return WEXITSTATUS (status);
}
