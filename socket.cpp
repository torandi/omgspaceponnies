#include "socket.h"
#include "network_lib.h"

#include <stdlib.h>
#include <stdio.h>

/**
 * Wrapper function to simplify socket change
 */
int read_raw(int sock,void * buffer,size_t len, int flags, addr_t * src_addr) {
	return recvfrom(sock,buffer,len, flags, &src_addr->addr, &src_addr->len);
}

void send_raw(int sockfd, void * data, sockaddr_in * to_addr) {
	if(sendto(sockfd, data, PAYLOAD_SIZE, 0, (sockaddr*) to_addr, sizeof(sockaddr_in))<0) {
		perror("Failed to send message");
	}
}
