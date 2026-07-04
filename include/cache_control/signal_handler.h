#pragma once

#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "context.h"

void sigint_handler(int sig);
int register_handler(int signum, void (*handler)(int));
void init_signal_handler();
void set_context(struct cache_control *cc);

#endif
