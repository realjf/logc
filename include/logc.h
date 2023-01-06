#ifndef _LOGC_H_
#define _LOGC_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define LOGC_VERSION_MAJOR 0
#define LOGC_VERSION_MINOR 1
#define LOGC_VERSION_PATCH 0

#define LOGC_VERSION logc_version(LOGC_VERSION_MAJOR, LOGC_VERSION_MINOR, LOGC_VERSION_PATCH)

typedef enum {
    LEVEL_TRACE,
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_FATAL
} log_level;

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

char *logc_version(int major, int minor, int patch);

// ############# Below: Implementation ###############

#ifdef LOGC_IMPLEMENTATION

#define MAX_CALLBACKS 32

typedef struct {
    log_LogFn fn;
    void *udata;
    log_level level;
} Callback;

static struct {
    void *udata;
    log_LockFn lock;
    log_level level;
    bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} L;

static const char *level_strings[] = {"TRACE", "DEBUG", "INFO",
                                      "WARN", "ERROR", "FATAL"};

#ifdef LOGC_USE_COLOR
static const char *level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m",
                                     "\x1b[33m", "\x1b[31m", "\x1b[35m"};
#endif

static void stdout_callback(log_event *ev) {
    char buf[22];
    buf[strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", ev->time)] = '\0';
#ifdef LOGC_USE_COLOR
    fprintf(ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", buf,
            level_colors[ev->level], level_strings[ev->level], ev->file,
            ev->line);
#else
    fprintf(ev->udata, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file,
            ev->line);
#endif
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void file_callback(log_event *ev) {
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    fprintf(ev->udata, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file,
            ev->line);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void lock(void) {
    if (L.lock) {
        L.lock(true, L.udata);
    }
}

static void unlock(void) {
    if (L.lock) {
        L.lock(false, L.udata);
    }
}

const char *log_level_string(log_level level) { return level_strings[level]; }

void log_set_lock(log_LockFn fn, void *udata) {
    L.lock = fn;
    L.udata = udata;
}

void log_set_level(log_level level) { L.level = level; }

void log_set_quiet(bool enable) { L.quiet = enable; }

int log_add_callback(log_LogFn fn, void *udata, log_level level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
            L.callbacks[i] = (Callback){fn, udata, level};
            return 0;
        }
    }
    return -1;
}

int log_add_fp(FILE *fp, log_level level) {
    return log_add_callback(file_callback, fp, level);
}

static void init_event(log_event *ev, void *udata) {
    if (!ev->time) {
        time_t t = time(NULL);
        ev->time = localtime(&t);
    }
    ev->udata = udata;
}

void log_log(log_level level, const char *file, int line, const char *fmt,
             ...) {
    log_event ev = {
        .fmt = fmt,
        .file = file,
        .line = line,
        .level = level,
    };

    lock();

    if (!L.quiet && level >= L.level) {
        init_event(&ev, stderr);
        va_start(ev.ap, fmt);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    unlock();
}

char *logc_version(int major, int minor, int patch) {
    char version[20];
    sprintf(version, "%d.%d.%d", major, minor, patch);
    char *ver_str = malloc(sizeof(version));
    memcpy(ver_str, version, sizeof(version));
    return ver_str;
}

#endif  // LOGC_IMPLEMENTATION

#endif /* _LOGC_H_ */
