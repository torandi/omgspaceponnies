/*********************************
 * This file contains functions to 
 * abstract the socket useage.
 *********************************/


#ifndef SOCKET_H
#define SOCKET_H

#include "network_lib.h"

#include <stdlib.h>

/**
 * Reads raw data from the socket
 */
int read_raw(int sock,void * buffer,size_t len, int flags, addr_t * src_addr);
void send_raw(int sockfd, void * data, sockaddr_in * to_addr);

#endif
