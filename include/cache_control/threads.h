#pragma once

#ifndef THREAD_H
#define THREAD_H

#include "context.h"
#include <pthread.h>

int start_threads(struct cache_control *cc);
int stop_threads();

#endif
