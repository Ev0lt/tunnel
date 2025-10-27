// config.h - configuration parsing for client and server
#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_KEY_LEN 128
#define MAX_VAL_LEN 256

typedef struct Config{
    char name[128];                        //[xxxx]
    char addr[128];                          //bind_ip
    int port;                               //bind_port
    bool use_encryption;                    //use_encryption
    char encryption_key[MAX_KEY_LEN];       //encryption_key
    char auth_token[MAX_VAL_LEN];           //auth_token
    struct ProxyConfig *next;
    char logfile_path[128];                 //logfile_path 仅限目录
    int logLevel;
    bool last; // true is last one , false isn't last one
} Config;

typedef enum { PROXY_TCP, PROXY_STCP, PROXY_UDP, PROXY_SUDP, PROXY_SOCKS5} ProxyType;

typedef struct ProxyConfig {
    char name[128];                        //[xxxx]
    ProxyType type;                         //type
    char username[128];                    //user
    char password[128];                    //pass
    int local_port;                         //local_port
    int remote_port;                        //remote_port
    char servertype[32];                    //servertype
    char pass[128];                        //pass
    struct ProxyConfig *next;
    bool last; // true is last one , false isn't last one
} ProxyConfig;



typedef struct {
    char key[128];
    char value[128];
} kv;

typedef struct LastConfig{
    struct ProxyConfig *PC;
    struct Config *C;
    bool last; // true is PC is last one , false is C is last one;
} LastConfig;

void parse_config(char *path, Config *common, bool method);

void initConfig(Config *C);

char* trim(char *line,int size);

LastConfig *getLast(Config *common);

kv *pkv(char *line);

void printConfig(Config *p);

// free proxy list
void freeConfig(Config *p);

#endif // CONFIG_H
