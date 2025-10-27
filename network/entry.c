#include "entry.h"
#include "Socks5.h"
#include "network.h"
#include "../common/protocol.h"
#include "../common/tools.h"
#include "../common/threading.h"
#include <math.h>

extern WorkConnection *WorkConnections;

void ServerProcess(mSOCKET sock, struct sockaddr_in Addr, Config *common){
    char *buf = (char*)malloc(pow(2,22));

    Config *clientConfig = (Config*)malloc(sizeof(Config));

    if (recv(sock,buf,8,0) <= 0){
        return;
    }

    if(strncmp(buf,ProtoWorkConn,8) == 0){
        logger(LOG_INFO,"ServerConsole","New Workconnection from %s:%d",inet_ntoa(Addr.sin_addr), ntohs(Addr.sin_port));
        send(sock,ProtoWorkConnEnd,strlen(ProtoWorkConnEnd),0);
        recv(sock,buf,10,0);
        logger(LOG_DEBUG,"WorkConnection","Recv ProtoWorkConnEnd: %s",buf);
        mSOCKET sockid = atoi(trim(buf,10));
        addWorkConnection(sockid,sock);
        logger(LOG_DEBUG,"WorkConnection","Send ProtoWorkConnEnd: %s",ProtoWorkConnEnd);
        logger(LOG_INFO,"ServerConsole","WorkConnection Add %d:%d",sockid,sock);
        return;
    }

    if(strncmp(buf,ProtoStart,8) != 0){
        logger(LOG_WARN,"ServerConsole","Error Protocol from %s:%d client kick out",inet_ntoa(Addr.sin_addr), ntohs(Addr.sin_port));
        return;
    }

    logger(LOG_INFO,"ServerConsole","New connection from %s:%d",inet_ntoa(Addr.sin_addr), ntohs(Addr.sin_port));

    if(recv(sock,buf,4096,0) <= 0){
        return;
    }

    parse_config(buf,clientConfig,false);
    if(common->auth_token != "" && strncmp(common->auth_token,clientConfig->auth_token,strlen(common->auth_token)) != 0){
        logger(LOG_WARN,"ServerConsole","Auth Failed from %s:%d client kick out",inet_ntoa(Addr.sin_addr), ntohs(Addr.sin_port));
        send(sock,AuthFaild,strlen(AuthFaild),0);
        return;
    }else{
        logger(LOG_INFO,"ServerConsole","Auth Success from %s:%d",inet_ntoa(Addr.sin_addr), ntohs(Addr.sin_port));
        send(sock,AuthSuccess,strlen(AuthSuccess),0);
        memset(buf,'\xff',strlen(buf));
        itoa(sock,buf,10);
        send(sock,buf,10,0);
    }

    ProxyConfig *next = clientConfig->next;
    StartConfigArg *args = (StartConfigArg*)malloc(sizeof(StartConfigArg));
    args->common = clientConfig;
    args->fathersock = sock;
    while(next!=NULL){
        args->proxy = next;
        create_thread(StartConfigOnServerThread, args);
        // StartConfigOnServer(sock,&clientConfig,next);
        if(next->last){
            break;
        }else{
            next = next->next;
        }
    }
}
void StartConfigOnServer(mSOCKET fathersock,Config *common, ProxyConfig *pc){
    logger(LOG_DEBUG,"DEBUG","%d",pc->type);
    if (pc->type == PROXY_SOCKS5){
        Socks5onServer(fathersock,common,pc);
    }

}

void StartConfigOnClient(mSOCKET fathersock,Config *common, ProxyConfig *pc){
    if (pc->type == PROXY_SOCKS5){
        Socks5onClient(common,pc);
    }
}