#ifndef ENTRY_H
#define ENTRY_H

#include "../common/platform.h"
#include "../common/config.h"

void ServerProcess(mSOCKET sock, struct sockaddr_in Addr, Config *common);

void StartConfigOnServer(mSOCKET fathersock,Config *common, ProxyConfig *pc);

void StartConfigOnClient(mSOCKET fathersock,Config *common, ProxyConfig *pc);

#endif // ENTRY_H