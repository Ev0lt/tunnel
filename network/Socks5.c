#include "../common/platform.h"
#include "../common/config.h"
#include "../common/tools.h"
#include "../common/protocol.h"
#include "../common/threading.h"
#include "network.h"

extern WorkConnection WorkConnections;

void Socks5onServer(mSOCKET fathersock,Config *common, ProxyConfig *pc){
    mSOCKET Socks5ServerSock = mSocket("tcp");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(pc->remote_port);
    addr.sin_addr.s_addr = inet_addr(common->addr);

    if(bind(Socks5ServerSock, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        logger(LOG_ERROR,pc->name,"Port is Aready in Use!!!");
        return;
    }

    // create_thread(checkAliveThread,&(checkAliveThreadArg){fathersock,Socks5ServerSock});

    listen(Socks5ServerSock,1000);
    logger(LOG_INFO,pc->name,"Socks5 Server Started On %d\n",pc->remote_port);
    
    int optval = 1;
    #ifdef SO_EXCLUSIVEADDRUSE
        // Windows特有：如果需要独占地址使用权（避免其他进程绑定相同地址）
    if (setsockopt(Socks5ServerSock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
        logger(LOG_ERROR,pc->name,"setsockopt SO_EXCLUSIVEADDRUSE failed");
    }
    #endif


    struct sockaddr_in outsider;
    int outsiderSize = sizeof(outsider);
    mSOCKET outsiderSock;

    SocksOnServerTask1ThreadArg *args = malloc(sizeof(SocksOnServerTask1ThreadArg));
    args->fathersock = fathersock;
    args->common = common;
    args->pc = pc;

    while(1){
        outsiderSock = accept(Socks5ServerSock,(struct sockaddr*)&outsider,&outsiderSize);
        if (outsiderSock == INVALID_SOCKET) {
            continue;
        }

        args->childsock = outsiderSock;
        //SocksOnServerTask1 fathersock,outsiderSock,common,pc);
        create_thread(SocksOnServerTask1Thread, args);
    }
}

void Socks5onClient(Config *common,ProxyConfig *proxy){
    logger(LOG_DEBUG,proxy->name,"Socks On Client\n");

    mSOCKET Socks5onClientSock = mSocket("tcp");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(proxy->local_port ? proxy->local_port : 0);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    bind(Socks5onClientSock, (struct sockaddr*)&addr, sizeof(addr));
    listen(Socks5onClientSock,100);
    getsockname(Socks5onClientSock, (struct sockaddr*)&addr, &(int){sizeof(addr)});
    int port = ntohs(addr.sin_port);
    logger(LOG_INFO,proxy->name,"SocksOnClient Started On Port %d\n",port);
    proxy->local_port = port;
    mSOCKET clientsock;

    Socks5onClientTask1ThreadArg *args = malloc(sizeof(Socks5onClientTask1ThreadArg));
    args->proxy = proxy;

    while(1){
        clientsock = accept(Socks5onClientSock,(struct sockaddr*)&addr,&(int){sizeof(addr)});
        if (clientsock == INVALID_SOCKET) {
            continue;
        }

        args->clientAddr = addr;
        args->ClientSock = clientsock;

        create_thread(SocksOnClientTask1Thread,args);
    }
}

void SocksOnServerTask1(mSOCKET fathersock,mSOCKET childsock,Config *common, ProxyConfig *pc){
    WorkConnection *workC = getWorkConnection(fathersock);
    int cout = 0;
    while (workC == NULL){
        send(fathersock,WorkConnectAdd,strlen(WorkConnectAdd),0);
        Sleep(3);
        workC = getWorkConnection(fathersock);
        cout++;
        if (cout > 5){
            closeSocket(childsock);
            logger(LOG_WARN,pc->name,"Busy, waiting for work connection...");
            return;
        }
    }
    logger(LOG_INFO,pc->name,"Get work connection...");
    send(workC->conn,pc->name,strlen(pc->name),0);
    Relay(childsock,workC->conn);
    delWorkConn(workC->conn);
}

void Socks5onClientTask1(mSOCKET ClientSock,struct sockaddr_in clientAddr,ProxyConfig *proxy){
    logger(LOG_DEBUG,proxy->name,"%s:%d Connect!!",inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    char buf[4096];
    memset(buf,0,4096);
    recv(ClientSock, buf, sizeof(buf)-1, 0);
    /*
    socks5客户端发起请求，会发送类似下面的字节流：
    |version|nmethods|methods|
    | 0x01  |  0x01  | 0x1-0xff |
    version中代表坂本 socks5代理，nmethods中代表支持的方法数，methods中代表支持的方法。
    0x00: NO AUTHENTICATION REQUIRED
    0x1: GSSAPI
    0x2: USERNAME/PASSWORD
    0x80-0xFE: IANA ASSIGNED
    0xFF: NO ACCEPTABLE METHODS
    例如：\x05\x01\x00 代表支持1种方法，即不需要认证
    */

    //判断坂本是否为socks5
    if (buf[0] != 0x05){
        logger(LOG_WARN,proxy->name,"Client %s:%d is not a SOCKS5 client\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        closeSocket(ClientSock);
        return;
    }

    /*
    查看她发送的socks5认证方法
    如果USERNAME不为空则需要验证账号密码就回到第二个if发送\x05\x02告诉客户端需要验证
    否则就发送\x05\x00告诉客户端不需要验证
    如果都不是，则要么没有写对应的逻辑，要么就是没有这种方法，则返回\xff代表没有可接受的方法，这样客户端就会断开连接
    */
    if (proxy->username != NULL && buf[2] == 0x00){
        send(ClientSock, "\x05\x00", 2, 0);
    }else if (buf[2] == 0x02){
        send(ClientSock, "\x05\x02", 2, 0);
    }else{
        send(ClientSock, "\x05\xff", 2, 0);
        logger(LOG_WARN, proxy->name, "Client %s:%d No Acceptable Methods\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        closeSocket(ClientSock);
        return;
    }

    /*
    如果需要验证账号密码，客户端则会发送账号密码到服务端
    其格式如下：
    |version|ulen|username|plen|password|
    | 0x01  | 0x01| 1-255  |0x01| 1-255  |
    version中代表版本号，ulen中代表用户名长度，username中代表用户名，plen中代表密码长度，password中代表密码
    例如：\x01\x04test\x04test 表示账号密码的长度都是\x04也就是四个字节
    验证成功则发送\x05\x00 否则发送\x00以外的字节表示失败
    */
    if (proxy->username != NULL){
        recv(ClientSock, buf, sizeof(buf)/sizeof(char), 0);
        if(memcmp(&buf[2],&(proxy->username),strlen(proxy->username)) == 0 || memcmp(&buf[2+strlen(proxy->username)],&(proxy->password),strlen(proxy->password)) == 0){
            logger(LOG_INFO, proxy->name, "Client %s:%d Authentication Successful\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            send(ClientSock, "\x05\x00",2, 0);
        }else{
            send(ClientSock, "\x05\x01",2, 0);
            logger(LOG_WARN, proxy->name,"[%s] Client %s:%d Authentication Failed\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            closeSocket(ClientSock);
            return;
        }
    }

    // recv(ClientSock, buf, sizeof(buf)-1, 0);
    // for(int i;i<sizeof(buf)/sizeof(char);i++){
    //     printf("%02x ",buf[i]);
    // }

    recv(ClientSock, buf, sizeof(buf)-1, 0);
    // for(int i;i<sizeof(buf)/sizeof(char);i++){
    //     printf("%02x ",buf[i]);
    // }
    if(buf[1] == 0x01){
        if (buf[3] == 0x1){

            // printf("IPv4:%s",inet_ntoa(*(struct in_addr*)&buf[4]));
            // printf("Port:%d\n",ntohs(*(unsigned short*)&buf[8]));
            SOCKET toSock = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in toAddr;
            toAddr.sin_family = AF_INET;
            toAddr.sin_port = *(unsigned short*)&buf[8];
            toAddr.sin_addr.s_addr = *(unsigned int*)&buf[4];
            if (connect(toSock,(struct sockaddr*)&toAddr,sizeof(toAddr)) == SOCKET_ERROR){
                char response[10];
                response[0] = 0x05;
                response[1] = 0X05;
                response[2] = 0x00;
                response[3] = buf[1];
                *(u_long*)&response[4] = 0;
                *(u_short*)&response[8] = 0;
                send(ClientSock, response, 10, 0);
                closeSocket(toSock);
                return;
            }
            char response[10];
            response[0] = 0x05;
            response[1] = 0X00;
            response[2] = 0x00;
            response[3] = buf[1];
            *(u_long*)&response[4] = 0;
            *(u_short*)&response[8] = 0;
            send(ClientSock, response, 10, 0);
            Relay(ClientSock,toSock);
        }else if (buf[3] == 0x3){
            int DomainLen = buf[4];
            char Domain[DomainLen];
            memcpy(Domain,&buf[5],DomainLen);
            struct addrinfo *hints = (struct addrinfo*)malloc(sizeof(struct addrinfo));
            getaddrinfo(Domain,NULL,NULL,&hints);
            struct sockaddr_in *si = (struct sockaddr_in*)(hints->ai_addr);
            SOCKET toSock = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in toAddr;
            toAddr.sin_family = AF_INET;
            toAddr.sin_port = *(unsigned short*)&buf[4+DomainLen];
            toAddr.sin_addr.s_addr = si->sin_addr.s_addr;
            if (connect(toSock,(struct sockaddr*)&toAddr,sizeof(toAddr)) == SOCKET_ERROR){
                char response[10];
                response[0] = 0x05;
                response[1] = 0X04;
                response[2] = 0x00;
                response[3] = buf[1];
                *(u_long*)&response[4] = 0;
                *(u_short*)&response[8] = 0;
                send(ClientSock, response, 10, 0);
                closeSocket(toSock);
                free(hints);
                free(si);
                return;
            }
            char response[10];
            response[0] = 0x05;
            response[1] = 0X00;
            response[2] = 0x00;
            response[3] = buf[1];
            *(u_long*)&response[4] = 0;
            *(u_short*)&response[8] = 0;
            send(ClientSock, response, 10, 0);
            Relay(ClientSock,toSock);
            free(hints);
            free(si);
        }else if (buf[3] == 0x4){
            //暂时不支持IPV6
            char response[10];
            response[0] = 0x05;
            response[1] = 0X08;
            response[2] = 0x00;
            response[3] = buf[1];
            *(u_long*)&response[4] = 0;
            *(u_short*)&response[8] = 0;
            send(ClientSock, response, 10, 0);
        }else{
            char response[10];
            response[0] = 0x05;
            response[1] = 0X08;
            response[2] = 0x00;
            response[3] = buf[1];
            *(u_long*)&response[4] = 0;
            *(u_short*)&response[8] = 0;
            send(ClientSock, response, 10, 0);
        }
    }else if (buf[1] == 0x2){
        // SOCKET bindSock = socket(AF_INET, SOCK_STREAM, 0);
        // struct sockaddr_in bindAddr;
        // bindAddr.sin_family = AF_INET;
        // bindAddr.sin_port = htons(0);
        // bindAddr.sin_addr.s_addr = INADDR_ANY;
        // if (bind(bindSock, (struct sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR) {
        //     char response[10];
        //     response[0] = 0x05;
        //     response[1] = 0X01;
        //     response[2] = 0x00;
        //     response[3] = buf[1];
        //     *(u_long*)&response[4] = 0;
        //     *(u_short*)&response[8] = 0;
        //     send(ClientSock, response, 10, 0);
        //     closesocket(bindSock);
        //     closesocket(ClientSock);
        //     return NULL;
        // }
        // listen(bindSock, 3);
        // getsockname(bindSock, (struct sockaddr*)&bindAddr, &(int){sizeof(bindAddr)});
        // printf("Start Bind Command on Port %d\n", ntohs(bindAddr.sin_port));
        // char response[10];
        // response[0] = 0x05;
        // response[1] = 0X00;
        // response[2] = 0x00;
        // response[3] = buf[1];
        // *(u_long*)&response[4] = htonl(bindAddr.sin_addr.s_addr);
        // *(u_short*)&response[8] = htons(bindAddr.sin_port);
        // send(ClientSock, response, 10, 0);


    }else if (buf[1] == 0x3){

    }else{
        char response[10];
        response[0] = 0x05;
        response[1] = 0X07;
        response[2] = 0x00;
        response[3] = buf[1];
        *(u_long*)&response[4] = 0;
        *(u_short*)&response[8] = 0;
        send(ClientSock, response, 10, 0);
        closeSocket(ClientSock);
    }
    
}