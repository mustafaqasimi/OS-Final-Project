# OS Final Project — xv6-riscv

Amirkabir University of Technology — Operating Systems Final Project

Based on [MIT xv6-riscv](https://github.com/mit-pdos/xv6-riscv).

## Prerequisites

```bash
sudo apt update
sudo apt install git build-essential gdb-multiarch \
  qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

## Build and run

Default Round Robin scheduler:

```bash
make clean && make qemu
```

Priority scheduler:

```bash
make clean && make qemu SCHEDULER=PRIORITY CPUS=1
```

Lottery scheduler (bonus):

```bash
make clean && make qemu SCHEDULER=LOTTERY CPUS=1
```

## New user commands

| Command | Description |
|---------|-------------|
| `ps` | List processes (PID, state, priority, tickets) |
| `chpri PID PRIORITY` | Set process priority (0=highest, 100=lowest) |
| `prio_test` | Priority scheduler validation |
| `lottery_test` | Lottery scheduler statistical test |

## New system calls

- `getpinfo(struct pinfo *buf, int max)` — returns process count or -1 on error
- `setpriority(int pid, int priority)` — returns 0 on success, -1 on failure
- `settickets(int number)` — sets calling process ticket count

## Modified files

**New:**

- `kernel/pinfo.h`
- `user/ps.c`
- `user/chpri.c`
- `user/prio_test.c`
- `user/lottery_test.c`

**Modified:**

- `kernel/proc.h`
- `kernel/proc.c`
- `kernel/syscall.h`
- `kernel/syscall.c`
- `kernel/sysproc.c`
- `user/user.h`
- `user/usys.pl`
- `Makefile`

## Project specification

See [docs/project.md](docs/project.md) for full requirements.

## Upstream xv6

See [README](README) for original xv6 documentation.
