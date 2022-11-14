/*
 * somewhat indpendent socket interface.
 *  Really, this is just for windows or linux.
 *
 *  If compiling for windows, you must include the wsock32.lib file
 *   (add "-lwsock32" to the makefile line)
 */

#include "indsock.h"

#include <stdio.h>
#include <string.h>


#ifdef __linux__ 

//linux code goes here
int connect_to_host(const char* IPAddress, int port)
{
    struct sockaddr_in addr; /* connector's address information */
    int fd;

    // now open a network socket to that machine
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -2;
    }

    //connect to it.
    memset(&addr, 0, sizeof(addr)); /* zero the struct */
    addr.sin_family = AF_INET;      /* host byte order */
    addr.sin_port = htons(port);    /* short, network byte order */
    addr.sin_addr.s_addr = inet_addr (IPAddress); //Target IP

    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) != 0) {
        return -1;
    }

    return fd;
}

//
int socket_status(int sock, int timeout_msec) {
    fd_set rfds;
    struct timeval tv;
    int r;

    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

    // Wait for character to arrive.
    tv.tv_sec = timeout_msec / 1000;
    tv.tv_usec = (timeout_msec%1000) * 1000;

    r = select(sock+1, &rfds, NULL, NULL, &tv);
    return r;
}

void close_connection(int sock)
{
    close(sock);
}

#elif _WIN32
// windows code goes here

//CONNECTTOHOST Connects to a remote host
int connect_to_host(char* IPAddress, int port)
{
    //Start up Winsock

    WSADATA wsadata;
    SOCKET sock; //Socket handle

    int error = WSAStartup(0x0202, &wsadata);
    //Did something happen?

    if (error)
        return -4;

    //Did we get the right Winsock version?
    if (wsadata.wVersion != 0x0202)
    {
        WSACleanup(); //Clean up Winsock
        return -3;
    }

    //Fill out the information needed to initialize a socket

    SOCKADDR_IN target; //Socket address information
    target.sin_family = AF_INET; 
    target.sin_port = htons (port); //Port to connect on
    target.sin_addr.s_addr = inet_addr (IPAddress); //Target IP

    sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket

    if (sock == INVALID_SOCKET)
        return -2; //Couldn't create the socket

    //Try connecting...

    if (connect(sock, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
        return -1; //Couldn't connect
    else
        return sock; //Success
}

//
int socket_status(int sock, int timeout_msec) {
    fd_set rfds;
    TIMEVAL tv;
    int r;

    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

    // Wait for character to arrive.
    tv.tv_sec = timeout_msec / 1000;
    tv.tv_usec = (timeout_msec%1000) * 1000;

    r = select(sock+1, &rfds, NULL, NULL, &tv);
    return r;
}


//shut down the socket and clean up Winsock

void close_connection(int sock)
{
    closesocket(sock);
    WSACleanup(); //Clean up Winsock
}

#else
   #error "UNSUPPORTED operating system type in indsock"
#endif


