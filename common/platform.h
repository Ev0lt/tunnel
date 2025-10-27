#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
#else
    #include<sys/socket.h>
    #include<arpa/inet.h>
    #include<netinet/in.h>
    #include<netdb.h>
    #include<unistd.h>
    #include<errno.h>
    #include<poll.h>
#endif

#include<pthread.h>

#ifdef _WIN32
    typedef SOCKET mSOCKET;
#else
    typedef int mSOCKET;
#endif

void create_thread(void* (*func)(void*), void* arg);
void socketInit();
void socketCleanup();
mSOCKET mSocket(char* type);
void closeSocket(mSOCKET sock);
void getLastError();
int mPoll(struct pollfd *fds, int nfds, int timeout);
#endif //PLATFORM_H