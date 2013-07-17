#define LOG 0
#ifdef LOG
#define LOG_MESSAGE(msg) printf("%s\n", msg)
#else
#define LOG_MESSAGE(msg)
#endif
