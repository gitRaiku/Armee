#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>

#ifndef SARMALE_LOG_NAME
#define SARMALE_LOG_NAME "SARMALE"
#endif

void set_logging_level(uint8_t level);
void logg(uint8_t severity, FILE *__restrict log_file, const char *format, ...);
void hexprint(FILE *__restrict log_file, char *buf, uint32_t l);

#endif

