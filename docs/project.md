# Operating Systems Final Project

## Amirkabir University of Technology

### Spring 1405

## 1. General requirements

The project is based on the official MIT `xv6-riscv` operating system.

Repository:

```text
https://github.com/mit-pdos/xv6-riscv
```

Students must fork or clone the repository and preserve a clear Git commit history throughout development.

The submitted ZIP file must:

* Follow the required student-ID naming convention.
* Include only the modified xv6 files.
* Include the final project report as a PDF.

The report must contain:

* Description of implemented changes
* Architecture and solution design
* List of modified files
* Implementation explanation for each section
* Test programs and execution results
* Analysis of test outputs

The project is individual.

The deadline stated in the PDF is 31 Khordad.

A project defense session will be held. The student must be able to explain and interpret all submitted code.

The Lottery Scheduler section is optional bonus work.

## 2. Ubuntu development environment

Install the required packages:

```bash
sudo apt update
sudo apt install git build-essential gdb-multiarch \
  qemu-system-misc gcc-riscv64-linux-gnu \
  binutils-riscv64-linux-gnu
```

Clone and run xv6:

```bash
git clone https://github.com/mit-pdos/xv6-riscv.git
cd xv6-riscv
make clean
make qemu
```

Verify QEMU:

```bash
qemu-system-riscv64 --version
```

Verify the RISC-V compiler:

```bash
riscv64-linux-gnu-gcc --version
```

A successful xv6 boot should end with output similar to:

```text
xv6 kernel is booting
init: starting sh
```

## 3. Mandatory test programs

For every project section, separate user-space test programs must be written.

Examples:

```text
prio_test.c
lottery_test.c
```

The tests must:

* Create multiple child processes.
* Use different priorities or ticket counts.
* Print clear and readable output.
* Demonstrate correct syscall and scheduler behavior.
* Be usable during the project defense.

The submitted tests are an essential part of the evaluation.

## 4. Part One: `getpinfo` system call and `ps`

### Objective

Implement a new system call named:

```c
getpinfo
```

The syscall must collect information about all processes and copy it safely to user space.

The process information should include at least:

* PID
* Process state
* Priority
* Number of lottery tickets

### Required tasks

1. Create a shared header file such as:

```text
kernel/pinfo.h
```

2. Define a structure such as:

```c
struct pinfo
```

The structure must be accessible from both kernel space and user space.

3. Register the new syscall in the appropriate xv6 files.

Likely files include:

```text
kernel/syscall.h
kernel/syscall.c
kernel/sysproc.c
user/user.h
user/usys.pl
```

4. Implement the kernel-side syscall handler, for example:

```c
uint64
sys_getpinfo(void)
```

5. Safely read process information while holding the appropriate process locks.

In modern xv6-riscv, processes have individual locks. Do not blindly use solutions written for old xv6 versions that relied on a global `ptable.lock`.

6. Copy the collected data from kernel space to user space using the appropriate xv6 mechanism, usually:

```c
copyout(...)
```

7. Implement a user-space command:

```text
ps
```

The `ps` command must display process information in readable columns.

Example output format:

```text
PID   STATE       PRIORITY   TICKETS
1     SLEEPING    50         1
2     RUNNABLE    20         10
```

### Expected result

Running:

```text
ps
```

inside the xv6 shell must display the current process list.

## 5. Part Two: Priority Scheduler

### Objective

Replace the default Round Robin scheduler with a priority-based scheduler.

Each process must have a priority value between:

```text
0 and 100
```

Priority rules:

```text
0   = highest priority
100 = lowest priority
```

The default priority of a newly created process must be:

```text
50
```

### Required tasks

1. Add a priority field to:

```c
struct proc
```

in:

```text
kernel/proc.h
```

Example conceptual field:

```c
int priority;
```

2. Initialize new processes with priority `50`.

This will likely happen in:

```c
allocproc()
```

3. Decide and document how priority behaves during `fork()`.

A reasonable design is for a child process to inherit the parent process priority, but the PDF explicitly requires only a default priority of 50. Confirm this design decision with the teaching assistant if necessary.

4. Implement the syscall:

```c
int setpriority(int pid, int priority);
```

The syscall must:

* Validate the PID.
* Validate that priority is between 0 and 100.
* Find the specified process.
* Safely acquire the process lock.
* Update the process priority.
* Return success or failure.

Suggested behavior:

```text
0  on success
-1 on failure
```

5. Implement a user-space command:

```text
chpri
```

Usage:

```text
chpri PID PRIORITY
```

Example:

```text
chpri 5 10
```

6. Modify:

```c
scheduler()
```

in:

```text
kernel/proc.c
```

The scheduler must select the `RUNNABLE` process with the smallest numeric priority value.

7. Implement Round Robin tie-breaking.

When multiple runnable processes have the same highest priority, CPU time must be distributed fairly among them in Round Robin order.

This requirement is important. A scheduler that always scans the process table from index zero may repeatedly select the same process and fail the fairness requirement.

8. Dynamic aging is not required.

Starvation prevention is outside the required project scope.

### Expected result

Higher-priority processes must receive CPU time before lower-priority processes.

Processes with equal priority must share CPU time fairly.

A dedicated test program must demonstrate this behavior.

## 6. Priority scheduler test program

Create a user-space test such as:

```text
prio_test.c
```

The test should:

1. Create several child processes.
2. Assign different priorities to them.
3. Run CPU-intensive loops.
4. Print progress or completion information.
5. Demonstrate that higher-priority processes are scheduled more aggressively.

Recommended scenarios:

### Scenario A: Different priorities

```text
Process A: priority 10
Process B: priority 50
Process C: priority 90
```

Expected behavior:

```text
A should receive CPU time more aggressively than B and C.
```

### Scenario B: Equal priorities

```text
Process A: priority 20
Process B: priority 20
Process C: priority 20
```

Expected behavior:

```text
The three processes should progress fairly using Round Robin tie-breaking.
```

### Important testing caution

Do not assume that a process with higher priority must always finish first unless all processes execute equivalent workloads and begin under comparable conditions.

Use repeated counters, controlled loops, or scheduler-selection statistics to produce convincing evidence.

## 7. Part Three: Lottery Scheduler — Bonus

### Objective

Implement a Lottery Scheduler in which CPU allocation is probabilistic and based on ticket counts.

A process with more tickets must have a higher probability of being selected.

### Required tasks

1. Add a ticket field to:

```c
struct proc
```

Example:

```c
int tickets;
```

2. Every new process must initially have:

```text
1 ticket
```

3. During:

```c
fork()
```

the child must inherit the parent's number of tickets.

4. Implement the syscall:

```c
int settickets(int number);
```

This syscall changes the ticket count of the calling process.

The implementation must reject invalid values such as zero or negative ticket counts.

5. Implement a simple pseudo-random number generator inside the kernel.

The PDF suggests an LCG-style PRNG.

The implementation must not depend on external libraries.

The system tick count may be used as part of the initial seed.

6. Modify the scheduler for Lottery mode.

Each scheduling cycle must:

* Calculate the total number of tickets held by all `RUNNABLE` processes.
* Generate a random winning ticket between 1 and the total.
* Iterate over runnable processes.
* Select the process whose cumulative ticket range contains the winning ticket.

7. Carefully handle locking.

Because process states can change during scheduler scans, the implementation must avoid races, deadlocks, and inconsistent ticket totals.

### Expected result

A process with 100 tickets should be selected significantly more often than a process with 10 tickets over a sufficiently large number of scheduler decisions.

The result must be demonstrated statistically.

## 8. Lottery test program

Create a test program such as:

```text
lottery_test.c
```

Suggested scenario:

```text
Process A: 10 tickets
Process B: 50 tickets
Process C: 100 tickets
```

Each process should execute equivalent CPU-bound work.

The report should compare observed CPU shares with expected proportions.

Total tickets:

```text
10 + 50 + 100 = 160
```

Expected approximate shares:

```text
Process A: 10 / 160  = 6.25%
Process B: 50 / 160  = 31.25%
Process C: 100 / 160 = 62.50%
```

The observed results do not need to be exact, but should approach these proportions over many scheduling decisions.

## 9. Compile-time scheduler selection

The scheduler implementations must coexist without repeatedly rewriting `scheduler()`.

The scheduler must be selected at compile time using Makefile variables and C preprocessor directives.

Required commands:

```bash
make qemu SCHEDULER=PRIORITY
```

```bash
make qemu SCHEDULER=LOTTERY
```

It is also advisable to support the default scheduler:

```bash
make qemu
```

Conceptual structure:

```c
#if defined(SCHED_PRIORITY)
  // Priority scheduler
#elif defined(SCHED_LOTTERY)
  // Lottery scheduler
#else
  // Default Round Robin scheduler
#endif
```

The Makefile should convert the `SCHEDULER` value into compiler definitions.

Example concept:

```make
ifeq ($(SCHEDULER),PRIORITY)
CFLAGS += -DSCHED_PRIORITY
endif

ifeq ($(SCHEDULER),LOTTERY)
CFLAGS += -DSCHED_LOTTERY
endif
```

The exact implementation must be adapted to the existing xv6 Makefile.

## 10. Likely files to modify

The exact list depends on the final design, but likely files include:

```text
kernel/proc.h
kernel/proc.c
kernel/syscall.h
kernel/syscall.c
kernel/sysproc.c
kernel/defs.h
kernel/pinfo.h

user/user.h
user/usys.pl
user/ps.c
user/chpri.c
user/prio_test.c
user/lottery_test.c

Makefile
```

Additional files may be needed depending on the design.

## 11. Acceptance criteria and grading

### Part One: `getpinfo` and `ps` — 6 points

* Correct syscall registration and prototypes: 2 points
* Safe shared structure and kernel-to-user transfer: 2 points
* Readable `ps` user program: 2 points

### Part Two: Priority Scheduler — 10 points

* Process structure and initialization: 2 points
* `setpriority` syscall and `chpri`: 3 points
* Correct priority selection and Round Robin tie-breaking: 4 points
* System stability under load: 1 point

### Part Three: Lottery Scheduler — 4 bonus points

* Ticket inheritance and `settickets`: 1.5 points
* Kernel PRNG: 1 point
* Correct lottery scheduling and statistical evidence: 1.5 points

### General criteria — 4 points

* Compile-time scheduler selection and clean integration: 2 points
* Clean code and meaningful Git history: 2 points

The student must be able to explain the code during the defense. Lack of understanding may result in losing the corresponding section score.

## 12. Suggested implementation order

Follow this order:

1. Clone and build unmodified xv6.
2. Create a baseline Git commit.
3. Add process fields with safe defaults.
4. Implement `getpinfo`.
5. Implement `ps`.
6. Write and run a `getpinfo` test.
7. Implement `setpriority`.
8. Implement `chpri`.
9. Implement the Priority Scheduler.
10. Add Round Robin tie-breaking.
11. Write `prio_test`.
12. Add Makefile scheduler-selection flags.
13. Test the default scheduler.
14. Test the Priority Scheduler.
15. Implement ticket inheritance.
16. Implement `settickets`.
17. Implement the PRNG.
18. Implement the Lottery Scheduler.
19. Write `lottery_test`.
20. Collect test outputs for the report.
21. Review every modified file before submission.
