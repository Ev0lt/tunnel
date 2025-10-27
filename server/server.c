#include "../common/platform.h"
#include "../common/tools.h"
#include "../common/config.h"
#include "../common/threading.h"
#include "../network/network.h"

WorkConnection *WorkConnections;

int main(){
    WorkConnections = (WorkConnection*)malloc(sizeof(WorkConnection));
    WorkConnections->next = NULL;
    WorkConnections->father = 0;
    WorkConnections->conn = 0;

    Config cnf;
    char *cnfName = "tunnels.ini";
    parse_config(cnfName,&cnf,true);
    set_logLevel(LOG_INFO);
    socketInit();

    mSOCKET sock = mSocket("TCP");
    
    struct sockaddr_in ClientAddr;
    int ClientAddrLen = sizeof(ClientAddr);
    struct sockaddr_in ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(cnf.port);
    ServerAddr.sin_addr.s_addr = inet_addr(cnf.addr);

    if(bind(sock, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1){
        getLastError();
        logger(LOG_WARN,"ServerConsole","%s:%d binding ERROR",cnf.addr, cnf.port);
        return -1;
    }else{
        logger(LOG_INFO,"ServerConsole","Listening on:%s:%d",cnf.addr, cnf.port);
    }

    if(listen(sock, 100000) == -1){
        getLastError();
        logger(LOG_WARN,"ServerConsole","Listening error!!!!");
        return -1;
    }

    while(1){
        mSOCKET client = accept(sock, (struct sockaddr*)&ClientAddr, &ClientAddrLen);

        if(client == -1){
            getLastError();
            continue;
        }
        
        ServerProcessThreadArg arg;
        arg.common = &cnf;
        arg.sock = client;
        arg.Addr = ClientAddr;
        create_thread(ServerProcessThread,&arg);
        // ServerProcess(client,ClientAddr,&cnf);

    }

    socketCleanup();
    return 0;
}