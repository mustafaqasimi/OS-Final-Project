#pragma once
#include "kernel/types.h"

struct pinfo {
  int pid;
  int state;
  int priority;
  int tickets;
};
