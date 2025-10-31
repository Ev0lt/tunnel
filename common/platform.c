#include "platform.h"
#include <stdio.h>
#include "tools.h"
void create_thread(void* (*func)(void*), void* arg){
    pthread_t thread;
    pthread_create(&thread, NULL, func, arg);
    pthread_detach(thread);
}

void socketInit(){
    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2,2), &wsaData);
    #endif
}

void socketCleanup(){
    #ifdef _WIN32
        WSACleanup();
    #endif
}

mSOCKET mSocket(char* type){
    if (strcasecmp(type, "TCP") == 0){
        return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }else if (strcasecmp(type, "UDP") == 0){
        return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    return -1;
}

void closeSocket(mSOCKET sock){ 
    #ifdef _WIN32
        closesocket(sock);
    #else
        close(sock);
    #endif
}

void getLastError(){
    #ifdef _WIN32
        logger(LOG_ERROR,"ServerConsole","Error: %d", WSAGetLastError());
    #else
        logger(LOG_ERROR,"ServerConsole","Error: %s", hstrerror(errno));
    #endif
}

int mPoll(struct pollfd *fds, int nfds, int timeout){
    #ifdef _WIN32
        return WSAPoll(fds, nfds, timeout);
    #else
        return poll(fds, nfds, timeout);
    #endif
}

void mSleep(int milliseconds){
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}