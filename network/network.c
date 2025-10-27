#include "network.h"
#include "../common/config.h"
#include "../common/tools.h"
#include "../common/protocol.h"
#include "../common/threading.h"

extern WorkConnection *WorkConnections;

WorkConnection *getLastWorkConnection(){
    bool last = WorkConnections->next == NULL ? false : true;
    WorkConnection* next = WorkConnections;
    while(last){
        next = next->next;
        last = next->next == NULL ? false : true;
    }
    return next;
}

void addWorkConnection(mSOCKET father,mSOCKET conn){
    WorkConnection *last = getLastWorkConnection();
    if(last->father > 0 || last->conn > 0){
        last->next = (WorkConnection*)malloc(sizeof(WorkConnection));
        last->next->father = father;
        last->next->conn = conn;
        last->next->use = false;
        last->next->next = NULL;
    }else{
        last->father = father;
        last->conn = conn;
        last->use = false;
        last->next = NULL;
    }
}

void delWorkConnFromfather(mSOCKET father){
    WorkConnection* tmp = WorkConnections;
    WorkConnection* next = WorkConnections;
    while(next != NULL){
        if(next->father == father){
            if(next == WorkConnections){
                if(next->next == NULL){
                    WorkConnections->father = 0;
                    closeSocket(WorkConnections->conn);
                    WorkConnections->conn = 0;
                    WorkConnections->use = false;
                    break;
                }
                next = next->next;
                free(WorkConnections);
                WorkConnections = next;
            }else{
                tmp->next = next->next;
                closeSocket(next->conn);
                free(next);
                next = tmp->next;
            }

        }else{
            tmp = next;
            next = next->next;
        }
    }
}

void delWorkConn(mSOCKET conn){
    WorkConnection* tmp = WorkConnections;
    WorkConnection* next = WorkConnections;
    while(next != NULL){
        if(next->conn == conn){
            if(next == WorkConnections){
                if(next->next == NULL){
                    closeSocket(WorkConnections->conn);
                    WorkConnections->father = 0;
                    WorkConnections->conn = 0;
                    WorkConnections->use = false;
                    break;
                }
                next = next->next;
                free(WorkConnections);
                WorkConnections = next;
            }else{
                tmp->next = next->next;
                closeSocket(next->conn);
                free(next);
                next = tmp->next;
            }

        }else{
            tmp = next;
            next = next->next;
        }
    }
}


WorkConnection *getWorkConnection(mSOCKET father) {
    WorkConnection *next = WorkConnections;
    while(father != next->father && !next->use){
        if(next->next == NULL){
            return NULL;
        }
        next = next->next;
    }
    next->use = true;
    return next;
}


//on client side

int create_WorkConnection(mSOCKET fathersock,int num,Config *common){
    for(int i = 0;i<num;i++){
        mSOCKET workconn = mSocket("TCP");
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(common->port);
        addr.sin_addr.s_addr = inet_addr(common->addr);
        if (connect(workconn, (struct sockaddr *)&addr, sizeof(addr)) == -1){
            logger(LOG_WARN,"WorkConnection","Work Connection connect error\n");
            return -1;
            // i--;
            // continue;
        }

        logger(LOG_DEBUG,"WorkConnection","Work Connection connect success");
        char buf[12];
        char tmp[12];
        memset(buf,0xff,12);
        send(workconn,ProtoWorkConn,strlen(ProtoWorkConn),0);
        logger(LOG_DEBUG,"WorkConnection","send ProtoWorkConn %s",ProtoWorkConn);
        recv(workconn,buf,12,0);
        if(strncmp(buf,ProtoWorkConnEnd,12) != 0){
            i--;
            logger(LOG_DEBUG,"WorkConnection","Error Recv ProtoWorkConnEnd %s",ProtoWorkConnEnd);
            closeSocket(workconn);
            continue;
        }
        memset(buf,0xff,12);
        itoa(fathersock,tmp,10);
        strncpy(buf,tmp,12);
        send(workconn,buf,10,0);
        logger(LOG_DEBUG,"WorkConnection","send father sockid %s",buf);
        addWorkConnection(fathersock,workconn);

        workThreadArg *arg = malloc(sizeof(workThreadArg));
        arg->common = common;
        arg->sock = workconn;
        create_thread(workThread,arg);
    }
    return 0;
}

void Relay(SOCKET fromSock, SOCKET toSock){
    char buf[4096];
    int len;
    fd_set readfds;
    while (1){
        FD_ZERO(&readfds);
        FD_SET(fromSock, &readfds);
        FD_SET(toSock, &readfds);
        int maxfd = (fromSock > toSock) ? fromSock : toSock;
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0){
            break;
        }
        if (FD_ISSET(fromSock, &readfds)){
            len = recv(fromSock, buf, sizeof(buf), 0);
            if (len <= 0){
                break;
            }
            send(toSock, buf, len, 0);
        }
        if (FD_ISSET(toSock, &readfds)){
            len = recv(toSock, buf, sizeof(buf), 0);
            if (len <= 0){
                break;
            }
            send(fromSock, buf, len, 0);
        }
    }
}

void work(mSOCKET sock,Config *Common){
    char buf[128];
    logger(LOG_DEBUG,"DEBUG","workid: %d",sock);
    int ret = recv(sock,buf,128,0);
    if(ret <= 0){
        delWorkConn(sock);
        return;
    }
    ProxyConfig *next = Common->next;
    while(next != NULL){
        logger(LOG_DEBUG,"DEBUG","config name: %s",next->name);
         if(strncmp(next->name,buf,strlen(next->name)) == 0){
            break;
        }
        logger(LOG_DEBUG,"DEBUG","%s %s.",next->name,buf);
        next = next->next;
    }
    
    mSOCKET toSock = mSocket("TCP");
    if(next->type != PROXY_SOCKS5 && next->type != PROXY_TCP && next->type != PROXY_STCP){
        mSOCKET toSock = mSocket("UDP");
    }

    struct sockaddr_in toAddr;
    toAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    toAddr.sin_family = AF_INET;
    toAddr.sin_port = htons(next->local_port);
    connect(toSock,(struct sockaddr*)&toAddr,sizeof(toAddr));
    Relay(sock,toSock);
    closeSocket(toSock);
    delWorkConn(sock);
}

void checkAlive(mSOCKET fsock,mSOCKET csock){
    WorkConnection* wc = getWorkConnection(fsock);
    while (wc == NULL){
        send(fsock,WorkConnectAdd,strlen(WorkConnectAdd),0);
        Sleep(1);
        wc = getWorkConnection(fsock);
    }
    recv(wc->conn,NULL,0,0);
    closeSocket(csock);
    delWorkConn(fsock);
}