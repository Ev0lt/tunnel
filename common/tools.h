#ifndef TOOLS_H
#define TOOLS_H

typedef enum{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
}debug_level;

extern debug_level LOG_LEVEL;
extern char* LOG_FILE;

int readfile(char *path,char *buf);

void logger(debug_level levl,char* name,char *fmt, ...);

void set_logLevel(debug_level level);

#endif