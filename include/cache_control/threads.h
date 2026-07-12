#pragma once

#ifndef THREAD_H
#define THREAD_H

#include "context.h"
#include <pthread.h>
#include "command_listener.h"
#include "file_monitoring.h"

int start_threads(struct cache_control *cc);
int stop_threads();

#endif
