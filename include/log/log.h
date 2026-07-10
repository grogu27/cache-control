#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#define LOG_BUFF 256

void open_log();
void logging(int log_level, char *msg);
void close_log();
#endif
