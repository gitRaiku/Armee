#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>

void set_logging_level(uint8_t level);
void set_logging_string(char *s);
void logg(uint8_t severity, FILE *__restrict log_file, const char *format, ...);
void hexprint(FILE *__restrict log_file, char *buf, uint32_t l);

#endif

