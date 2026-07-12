#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int pid, priority;

  if (argc != 3) {
    fprintf(2, "usage: chpri PID PRIORITY\n");
    exit(1);
  }

  pid = atoi(argv[1]);
  priority = atoi(argv[2]);
  if (setpriority(pid, priority) < 0) {
    fprintf(2, "chpri: failed\n");
    exit(1);
  }
  exit(0);
}
