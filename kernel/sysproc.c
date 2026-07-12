#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "pinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_getpinfo(void)
{
  uint64 user_addr;
  int max;
  struct proc *p;
  struct proc *cur = myproc();
  struct pinfo kbuf[NPROC];
  int n = 0;
  uint64 size;

  argaddr(0, &user_addr);
  argint(1, &max);
  if (max <= 0 || max > NPROC)
    return -1;

  size = (uint64)max * sizeof(struct pinfo);
  if (user_addr >= cur->sz || user_addr + size < user_addr ||
      user_addr + size > cur->sz)
    return -1;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->state != UNUSED) {
      if (n < max) {
        kbuf[n].pid = p->pid;
        kbuf[n].state = p->state;
        kbuf[n].priority = p->priority;
        kbuf[n].tickets = p->tickets;
        n++;
      }
    }
    release(&p->lock);
  }

  if (copyout(cur->pagetable, user_addr, (char *)kbuf,
              n * sizeof(struct pinfo)) < 0)
    return -1;
  return n;
}

uint64
sys_setpriority(void)
{
  int pid, priority;
  struct proc *p;

  argint(0, &pid);
  argint(1, &priority);
  if (priority < 0 || priority > 100)
    return -1;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->pid == pid && p->state != UNUSED) {
      p->priority = priority;
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

uint64
sys_settickets(void)
{
  int number;
  struct proc *p = myproc();

  argint(0, &number);
  if (number <= 0)
    return -1;

  acquire(&p->lock);
  p->tickets = number;
  release(&p->lock);
  return 0;
}
