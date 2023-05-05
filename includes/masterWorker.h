#ifndef MASTER_H
#define MASTER_H

#include <threadsPool.h>
#include "threadsPool.h"
#include <signal.h>

extern sigset_t mask;


void *signalHandler();

int signalMask();

void masterExitFun();

void init_coda_con();

void set_standard_coda_con();

#endif