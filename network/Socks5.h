#ifndef SOCKS5_H
#define SOCKS5_H 

#include "../common/platform.h"
#include "../common/config.h"

void Socks5onServer(mSOCKET fathersock,Config *common, ProxyConfig *pc);

void Socks5onClient(Config *common,ProxyConfig *proxy);

void SocksOnServerTask1(mSOCKET fathersock,mSOCKET childsock,Config *common, ProxyConfig *pc);

void Socks5onClientTask1(mSOCKET ClientSock,struct sockaddr_in clientAddr,ProxyConfig *proxy);

#endif