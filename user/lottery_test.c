#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static void
count_work(int tickets, char *fname)
{
  volatile long count = 0;
  int fd;
  uint start, now;

  if (settickets(tickets) < 0)
    exit(1);

  start = uptime();
  while (1) {
    count++;
    now = uptime();
    if (now - start >= 300)
      break;
  }

  long n = count;
  fd = open(fname, O_CREATE | O_RDWR);
  if (fd < 0)
    exit(1);
  write(fd, &n, sizeof(n));
  close(fd);
  exit(0);
}

int
main(int argc, char **argv)
{
  int pids[3];
  int tickets[] = {10, 50, 100};
  char *names[] = {"lot0", "lot1", "lot2"};
  long counts[3];
  long total;
  int fd;
  int i;

  (void)argc;
  (void)argv;

  printf("Lottery test: tickets 10, 50, 100\n");
  printf("Expected ratios: 6.25%%, 31.25%%, 62.50%%\n");

  for (i = 0; i < 3; i++) {
    if ((pids[i] = fork()) < 0) {
      printf("fork failed\n");
      exit(1);
    }
    if (pids[i] == 0)
      count_work(tickets[i], names[i]);
  }

  for (i = 0; i < 3; i++)
    wait(0);

  total = 0;
  for (i = 0; i < 3; i++) {
    fd = open(names[i], O_RDONLY);
    read(fd, &counts[i], sizeof(counts[i]));
    close(fd);
    unlink(names[i]);
    total += counts[i];
  }

  if (total == 0) {
    printf("no work recorded\n");
    exit(1);
  }

  for (i = 0; i < 3; i++) {
    printf("process %d: tickets=%d count=%ld share=%ld.%ld%%\n",
           i, tickets[i], counts[i],
           (counts[i] * 1000) / total / 10,
           (counts[i] * 1000) / total % 10);
  }

  printf("Build with: make qemu SCHEDULER=LOTTERY CPUS=1\n");
  exit(0);
}
