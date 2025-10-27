#ifndef NETWORK_H
#define NETWORK_H

#include "../common/platform.h"
#include "../common/config.h"

#include <stdbool.h>


typedef struct WorkConnection{
    mSOCKET father;
    mSOCKET conn;
    bool use;
    struct WorkConnection *next;
}WorkConnection;

WorkConnection *getLastWorkConnection();

void delWorkConn(mSOCKET conn);

void delWorkConnFromfather(mSOCKET father);

void addWorkConnection(mSOCKET father,mSOCKET conn);

WorkConnection *getWorkConnection(mSOCKET father);

int create_WorkConnection(mSOCKET fathersock,int num,Config *common);

void Relay(SOCKET fromSock, SOCKET toSock);

void work(mSOCKET sock,Config *Common);

void checkAlive(mSOCKET fsock,mSOCKET csock);

#endif