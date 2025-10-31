#ifndef NETWORK_H
#define NETWORK_H

#include "../common/platform.h"
#include "../common/config.h"
#include "../common/vector.h"
#include <stdlib.h>
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

void Relay(mSOCKET fromSock, mSOCKET toSock);

void work(mSOCKET sock,Config *Common);

void checkAlive(mSOCKET fsock);

typedef struct OnlineClient{
    mSOCKET fathersock;
    Vector *clients;
}OnlineClient;

#endif