#include "socket.h"
#include "network_lib.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/*
 * Wrapper functions to simplify socket change
 */

ssize_t read_raw(int sock,void * buffer,size_t len, int flags, addr_t * src_addr) {
	ssize_t size = recvfrom(sock,buffer,len, flags, (sockaddr*)&src_addr->addr, &src_addr->len);
	if(size < 0) {
		perror("read_raw()");
	}
	return size;
}

void send_raw(int sockfd, void * data, const addr_t &to_addr) {
	if(sendto(sockfd, data, PAYLOAD_SIZE, 0, (sockaddr*) &to_addr.addr, sizeof(sockaddr_in))<0) {
		perror("send_raw()");
	}
}

int create_socket(int port, bool broadcast) {

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		perror("create socket()");
		exit(1);
	}

	int one = 1;
	if(broadcast) {
		if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(int)) == -1) {
			perror("create_socket(): setsockopt SO_BROADCAST");
			exit(1);
		}
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
		perror("create_socket(): setsockopt SO_REUSEADDR");
		exit(1);
	}

	int x;
	x=fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL,x | O_NONBLOCK);
	sockaddr_in addr;

	if(broadcast) {
		addr = broadcast_addr(port).addr;
	} else {
		addr = create_addr(INADDR_ANY,port).addr;
	}

	if(bind(sockfd, (sockaddr *) &broadcast_addr, sizeof(sockaddr_in)) < 0) {
		perror("network(): bind");
		exit(1);
	}

	return sockfd;
}

void close_socket(int sock) {
	close(sock);
}

addr_t create_addr(uint32_t address, int port) {
	addr_t addr;
	addr.addr.sin_family = AF_INET;
	addr.addr.sin_port = htons(port);
	addr.addr.sin_addr.s_addr = htonl(address);	
	return addr;
}

addr_t broadcast_addr(int port) {
	return create_addr(INADDR_BROADCAST,port);
}

addr_t create_addr_from_hn(const char * hostname, int port) {
	hostent * host;
	host = gethostbyname(hostname);
	if(host == NULL) {
		fprintf(stderr,"create_addr_from_hn(): No such host %s\n",hostname);
		addr_t a;
		a.len = 0;
		return a;
	}
	addr_t addr = create_addr(0,port);
	bcopy((char *)host->h_addr,
		(char *)&addr.addr.sin_addr.s_addr,
		host->h_length);
	
	return addr;
}

bool data_available(int sock) {
	fd_set readset;
	struct timeval tv;

	FD_ZERO(&readset);
	FD_SET(sock,&readset);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	return (select(sock+1,&readset,NULL,NULL,&tv) > 0);
}
