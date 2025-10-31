#include "../common/platform.h"
#include "../common/tools.h"
#include "../common/config.h"
#include "../common/threading.h"
#include "../common/protocol.h"
#include "../network/network.h"
#include "../common/vector.h"
#include <math.h>

WorkConnection *WorkConnections;
Vector *Online;
int main(){
    WorkConnections = (WorkConnection*)malloc(sizeof(WorkConnection));
    WorkConnections->conn = 0;
    WorkConnections->father = 0;
    WorkConnections->next = NULL;
    Config cnf;
    char *cnfName = "tunnelc.ini";
    parse_config(cnfName,&cnf,true);
    // set_logLevel(cnf.logLevel);
    socketInit();
   
    mSOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        logger(LOG_WARN,"ServerConsole","socket error\n");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cnf.port);
    addr.sin_addr.s_addr = inet_addr(cnf.addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        logger(LOG_WARN,"ServerConsole","connect error\n");
        return -1;
    }

    send(sock,ProtoStart,strlen(ProtoStart),0);

    //发送客户端的配置文件给服务端
    char *buf = (char*)malloc(pow(2,22));
    readfile(cnfName,buf);
    
    send(sock,buf,strlen(buf),0);

    if (recv(sock,buf,4096,0) <= 0){
        logger(LOG_WARN,"ServerConsole","Server no Connect\n");
        return -1;
    }

    if(strncmp(buf,AuthFaild,strlen(AuthFaild)) == 0){
        logger(LOG_WARN,"ServerConsole","AuthCode Error!!,Please check your config file");
        return -1;
    }else if(strncmp(buf,AuthSuccess,strlen(AuthSuccess)) == 0){
        logger(LOG_INFO,"ServerConsole","Auth Success!!\n");
    }

    int fatherid;
    recv(sock,buf,10,0);
    fatherid = atoi(trim(buf,10));
    
    ProxyConfig *next = cnf.next;
    StartConfigArg *args = (StartConfigArg*)malloc(sizeof(StartConfigArg));
    args->common = &cnf;    
    args->fathersock = sock;
    while(true){
        args->proxy = next;
        create_thread(StartConfigOnClientThread,args);
        if (!next->last){
            next = next->next;
        }else{
            break;
        }
    }

    int ret = create_WorkConnection(fatherid,10,&cnf);
    if(ret == -1){
        return -1;
    }
    memset(buf,0,sizeof(buf));
    //处理WorkConnection的消息
    while(1){
        if (recv(sock,buf,strlen(WorkConnectAdd),0) <= 0){
            delWorkConnFromfather(fatherid);
            return -1;
        }
        buf[12] = '\0';
        logger(LOG_DEBUG,"WorkConnection","Recv WorkConnectAdd %s",buf);
        if(strncmp(buf,WorkConnectAdd,strlen(WorkConnectAdd)) == 0){
            ret = create_WorkConnection(fatherid,10,&cnf);
            if(ret == -1){
                return -1;
            }
        }
    }
}