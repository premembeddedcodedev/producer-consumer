#define DEBUG 1

#ifdef DEBUG
#define DEBUGLOG_INIT(filename) debuglog_init(filename)
#else
#define debuglog_init(...)
#endif

#ifdef DEBUG
#define DEBUGLOG_CLOSE debuglog_close
#else
#define debuglog_close(...)
#endif

#define DEBUGLOG_LOG(level, fmt, ...) DEBUGLOG_LOG ## level (fmt, ##__VA_ARGS__)

#if DEBUG == 0
#define DEBUGLOG_LOG0(...)
#endif

#if DEBUG >= 1
#define DEBUGLOG_LOG1(fmt, ...) debuglog_log (fmt, ##__VA_ARGS__)
#else
#define DEBUGLOG_LOG1(...)
#endif

#if DEBUG >= 2
#define DEBUGLOG_LOG2(fmt, ...) debuglog_log (fmt, ##__VA_ARGS__)
#else
#define DEBUGLOG_LOG2(...)
#endif

#if DEBUG == 3
#define DEBUGLOG_LOG3(fmt, ...) debuglog_log (fmt, ##__VA_ARGS__)
#else
#define DEBUGLOG_LOG3(...)
#endif

void debuglog_init(char *filename);
void debuglog_close(void);
void debuglog_log(char* format, ...);
