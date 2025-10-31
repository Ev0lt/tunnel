#ifndef THREADING_H
#define THREADING_H

#include "../common/platform.h"
#include "../common/config.h"
#include "../network/entry.h"

typedef struct {
    mSOCKET sock;
    struct sockaddr_in Addr;
    Config *common;
}ServerProcessThreadArg;

void *ServerProcessThread(void *arg);

typedef struct {
    mSOCKET fathersock;
    Config *common;
    ProxyConfig *proxy;
}StartConfigArg;

void *StartConfigOnServerThread(void *arg);
void *StartConfigOnClientThread(void *arg);

typedef struct{
    mSOCKET fathersock;
    mSOCKET childsock;
    Config *common;
    ProxyConfig *pc;
}SocksOnServerTask1ThreadArg;

void *SocksOnServerTask1Thread(void *arg);

typedef struct{
    mSOCKET ClientSock;
    struct sockaddr_in clientAddr;
    ProxyConfig *proxy;
}Socks5onClientTask1ThreadArg;

void *SocksOnClientTask1Thread(void *arg);

typedef struct{
    mSOCKET sock;
    Config *common;
}workThreadArg;

void *workThread(void *arg);

void *checkAliveThread(void *arg);

#endif //THREADING_H