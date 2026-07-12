#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pinfo.h"
#include "user/user.h"

static char *
statestr(int state)
{
  static char *states[] = {
    [0] "UNUSED",
    [1] "USED",
    [2] "SLEEPING",
    [3] "RUNNABLE",
    [4] "RUNNING",
    [5] "ZOMBIE",
  };
  if (state >= 0 && state <= 5)
    return states[state];
  return "???";
}

int
main(int argc, char **argv)
{
  struct pinfo buf[NPROC];
  int n;
  int i;

  (void)argc;
  (void)argv;

  n = getpinfo(buf, NPROC);
  if (n < 0) {
    fprintf(2, "ps: getpinfo failed\n");
    exit(1);
  }

  printf("PID   STATE       PRIORITY   TICKETS\n");
  for (i = 0; i < n; i++) {
    printf("%d     %-10s  %d         %d\n",
           buf[i].pid, statestr(buf[i].state),
           buf[i].priority, buf[i].tickets);
  }
  exit(0);
}
