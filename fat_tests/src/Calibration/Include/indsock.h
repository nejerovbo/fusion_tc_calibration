/**************************************************************************
(c) Copyright 2022 Digital Dynamics Inc. Scotts Valley CA USA.
Unpublished copyright. All rights reserved. Contains proprietary and
confidential trade secrets belonging to DDI. Disclosure or release without
prior written authorization of DDI is prohibited.
**************************************************************************/

int connect_to_host(const char* IPAddress, int port);
int socket_status(int sock, int timeout_msec);
void close_connection(int sock);

#ifdef __linux__ 

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#else

#include <winsock2.h>

#endif

