#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define SPIN 100000000

static void
spin(void)
{
  volatile int i;
  for (i = 0; i < SPIN; i++)
    ;
}

static void
test_invalid_inputs(void)
{
  printf("Invalid input tests\n");

  if (setpriority(-1, 5) != -1)
    printf("FAIL: setpriority(-1, 5) should fail\n");
  else
    printf("OK: setpriority(-1, 5) failed as expected\n");

  if (setpriority(getpid(), 200) != -1)
    printf("FAIL: setpriority(pid, 200) should fail\n");
  else
    printf("OK: setpriority(pid, 200) failed as expected\n");

  if (setpriority(getpid(), -1) != -1)
    printf("FAIL: setpriority(pid, -1) should fail\n");
  else
    printf("OK: setpriority(pid, -1) failed as expected\n");
}

static void
scenario_different_priorities(void)
{
  int pfd[2];
  int pids[3];
  int priorities[] = {10, 50, 90};
  int i, who;

  printf("Scenario A: priorities 10, 50, 90\n");

  if (pipe(pfd) < 0) {
    printf("pipe failed\n");
    exit(1);
  }

  for (i = 0; i < 3; i++) {
    if ((pids[i] = fork()) < 0) {
      printf("fork failed\n");
      exit(1);
    }
    if (pids[i] == 0) {
      close(pfd[0]);
      if (setpriority(getpid(), priorities[i]) < 0)
        exit(1);
      spin();
      write(pfd[1], &i, sizeof(i));
      close(pfd[1]);
      exit(0);
    }
    if (setpriority(pids[i], priorities[i]) < 0) {
      printf("setpriority failed for pid %d\n", pids[i]);
      exit(1);
    }
  }

  close(pfd[1]);
  for (i = 0; i < 3; i++) {
    read(pfd[0], &who, sizeof(who));
    printf("finish order %d: process index %d (priority %d)\n",
           i + 1, who, priorities[who]);
  }
  close(pfd[0]);

  for (i = 0; i < 3; i++)
    wait(0);

  printf("Scenario A complete (expect lower priority values first)\n");
  printf("Build with: make qemu SCHEDULER=PRIORITY CPUS=1\n");
}

static void
scenario_equal_priorities(void)
{
  int pids[3];
  int i;
  int fd;
  long counts[3];
  char *names[] = {"eq0", "eq1", "eq2"};
  uint start, now;

  printf("Scenario B: equal priority 20\n");

  for (i = 0; i < 3; i++) {
    if ((pids[i] = fork()) < 0) {
      printf("fork failed\n");
      exit(1);
    }
    if (pids[i] == 0) {
      volatile long count = 0;
      if (setpriority(getpid(), 20) < 0)
        exit(1);
      start = uptime();
      while (1) {
        count++;
        now = uptime();
        if (now - start >= 100)
          break;
      }
      long n = count;
      fd = open(names[i], O_CREATE | O_RDWR);
      if (fd < 0)
        exit(1);
      write(fd, &n, sizeof(n));
      close(fd);
      exit(0);
    }
    if (setpriority(pids[i], 20) < 0) {
      printf("setpriority failed\n");
      exit(1);
    }
  }

  for (i = 0; i < 3; i++)
    wait(0);

  for (i = 0; i < 3; i++) {
    fd = open(names[i], O_RDONLY);
    read(fd, &counts[i], sizeof(counts[i]));
    close(fd);
    unlink(names[i]);
  }

  printf("counts: %ld %ld %ld\n", counts[0], counts[1], counts[2]);
  printf("Scenario B complete: counts should be roughly balanced\n");
}

int
main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  test_invalid_inputs();
  scenario_different_priorities();
  scenario_equal_priorities();
  exit(0);
}
