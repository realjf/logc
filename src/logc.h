#ifndef _LOGC_H_
#define _LOGC_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define LOGC_VERSION_MAJOR 0
#define LOGC_VERSION_MINOR 1
#define LOGC_VERSION_PATCH 0

#define LOGC_VERSION LOGC_VERSION_MAJOR.LOGC_VERSION_MINOR.LOGC_VERSION_PATCH

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  log_level level;
} log_event;

typedef void (*log_LogFn)(log_event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

typedef enum {
  LEVEL_TRACE,
  LEVEL_DEBUG,
  LEVEL_INFO,
  LEVEL_WARN,
  LEVEL_ERROR,
  LEVEL_FATAL
} log_level;

#define log_trace(...) log_log(LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

const char *log_level_string(log_level level);
void log_set_lock(log_LockFn fn, void *udata);
void log_set_level(log_level level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void *udata, log_level level);
int log_add_fp(FILE *fp, log_level level);

void log_log(log_level level, const char *file, int line, const char *fmt, ...);

#endif /* _LOGC_H_ */
