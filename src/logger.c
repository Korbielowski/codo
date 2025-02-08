#include "logger.h"
#include <stdarg.h>
#include <stdio.h>

void log_to_file(char *format, ...) {
  va_list arg_ptr;
  FILE *d_file;

  va_start(arg_ptr, format);
  d_file = fopen("logs.log", "a");

  if (d_file == NULL) {
    perror("Cannot open d_file: logs.log");
    va_end(arg_ptr);
    return;
  }

  if (format == NULL) {
    fprintf(d_file, "No specified user arguments");
  } else {
    vfprintf(d_file, format, arg_ptr);
  }
  fprintf(d_file, "\n");
  fflush(d_file);

  fclose(d_file);
  va_end(arg_ptr);
}