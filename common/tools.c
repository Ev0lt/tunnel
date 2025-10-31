#include "tools.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>

#ifdef _WIN32
    #include <direct.h>
    #include <sys/stat.h>
    #define STAT _stat
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define STAT stat
#endif

debug_level LOG_LEVEL = LOG_DEBUG;
char* LOG_FILE = NULL;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
int readfile(char *path,char *buf){
    FILE *fp = fopen(path,"rb");
    if(fp==NULL) return -1;
    char line[1024];
    int n = 0;
    char **tmp = &buf;
    while(fgets(line,1024,fp) != NULL){
        strncpy(*tmp+n,line,strlen(line));
        n+=strlen(line);
    }
    return n;
}

void set_logFile(char* path){
    LOG_FILE = path;
}

void set_logLevel(debug_level level){
    LOG_LEVEL = level;
}

int is_dir(char* path){
    struct STAT path_stat;

    if(STAT(path,&path_stat) != 0){
        return 0;
    }

    #ifdef _WIN32
        return (path_stat.st_mode & _S_IFDIR) != 0;
    #else
        return S_ISDIR(path_stat.st_mode);
    #endif
}

void logger(debug_level levl,char* name,char *fmt, ...){
    pthread_mutex_lock(&log_mutex);
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    // 打印时间戳和日志级别
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    if (levl >= LOG_LEVEL){
        switch(levl){
            case LOG_DEBUG:
                printf("%s [DEBUG] ", timestamp);
                break;
            case LOG_INFO:
                printf("%s [INFO] ", timestamp);
                break;
            case LOG_WARN:
                printf("%s [WARN] ", timestamp);
                break;
            case LOG_ERROR:
                printf("%s [ERROR] ", timestamp);
                break;
        }

        char buf[strlen(name)+2+strlen(fmt)+2];
        sprintf(buf,"[%s] %s",name,fmt);
        buf[strlen(buf)+1] = '\0';
        if(fmt[strlen(fmt)-1] != '\n'){
            buf[strlen(buf)] = '\n';
        }

        va_list args;
        va_start(args, fmt);
        vprintf(buf, args);
        va_end(args);
        if (LOG_FILE != NULL && LOG_FILE != ""){
            char name[1024] = "";
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d", tm_info);
            if(is_dir(LOG_FILE)){
                sprintf(name, "%s/tunnelServer-%s.log", LOG_FILE, timestamp);
            }else{
                #ifdef _WIN32
                    mkdir(LOG_FILE);
                #else
                    mkdir(LOG_FILE, 0777);
                #endif 
                sprintf(name, "%s/tunnelServer-%s.log", LOG_FILE,timestamp);
            }
            FILE *fp = fopen(name, "a");
            if (fp != NULL){
                fprintf(fp, "%s [%s] ",timestamp, levl == LOG_ERROR ? "ERROR" : levl == LOG_WARN ? "WARN" : levl == LOG_INFO ? "INFO" : "DEBUG");
                va_list args;
                va_start(args, fmt);
                vfprintf(fp, fmt, args);
                va_end(args);
            }
            fclose(fp);
        }
    }
    pthread_mutex_unlock(&log_mutex);
}


