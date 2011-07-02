#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "network.h"

#define CMD(str) strncmp(buffer,str,3) == 0

/**
 * Protocol:
 */

static struct sockaddr_in broadcast_addr;
static int sockfd;
static fd_set readset;
static struct timeval tv;

void init_network() {

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		perror("network(): create socket");
		exit(1);
	}

	int one = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(int)) == -1) {
		perror("network(): setsockopt SO_BROADCAST");
		exit(1);
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
		perror("network(): setsockopt SO_REUSEADDR");
		exit(1);
	}

	int x;
	x=fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL,x | O_NONBLOCK);

	broadcast_addr.sin_family = AF_INET;
	broadcast_addr.sin_port = htons(port);
	broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);	

	if(bind(sockfd, (sockaddr *) &broadcast_addr, sizeof(sockaddr_in)) < 0) {
		perror("network(): bind");
		exit(1);
	}
}

/**
 * Handles network traffic.
 */
void network() {
	char buffer[1024];
	size_t size;
	struct sockaddr src_addr;
	socklen_t addrlen;

	FD_ZERO(&readset);
	FD_SET(sockfd,&readset);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if(select(sockfd+1,&readset,NULL,NULL,&tv) > 0) {
		size = recvfrom(sockfd, buffer, 1024, 0, &src_addr, &addrlen);
		if(size < 3) {
			
		} else {
			buffer[size] = 0;
			fprintf(stderr,"Recieved invalid data: %s\n", buffer);
		}
	}
}

void send_msg(const char * buffer) {
	if(sendto(sockfd, buffer, strlen(buffer)+1, 0, (sockaddr*) &broadcast_addr, sizeof(sockaddr_in))<0) {
		perror("Falid to send massage");
	}
}
