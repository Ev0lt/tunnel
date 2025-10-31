#include "threading.h"
#include "../network/entry.h"
#include "../network/Socks5.h"
#include "../network/network.h"
void *ServerProcessThread(void *arg){
    ServerProcessThreadArg *args = (ServerProcessThreadArg *)arg;
    ServerProcess(args->sock,args->Addr, args->common);
}

void *StartConfigOnServerThread(void *arg){ 
    StartConfigArg *args = (StartConfigArg *)arg;
    StartConfigOnServer(args->fathersock,args->common,args->proxy);
}

void *StartConfigOnClientThread(void *arg){
    StartConfigArg *args = (StartConfigArg *)arg;
    StartConfigOnClient(args->fathersock,args->common,args->proxy);
}

void *SocksOnServerTask1Thread(void *arg){
    SocksOnServerTask1ThreadArg *args = (SocksOnServerTask1ThreadArg *)arg;
    SocksOnServerTask1(args->fathersock,args->childsock,args->common,args->pc);
}

void *SocksOnClientTask1Thread(void *arg){
    Socks5onClientTask1ThreadArg *args = (Socks5onClientTask1ThreadArg *)arg;
    Socks5onClientTask1(args->ClientSock,args->clientAddr,args->proxy);
}

void *workThread(void *arg){
    workThreadArg *args = (workThreadArg *)arg;
    work(args->sock,args->common);
}

void *checkAliveThread(void *arg){
    checkAlive(*(mSOCKET*)arg);
}