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
#include "render.h"
#include "player.h"
#include "logic.h"

#define CMD(str) strncmp(data,str,3) == 0

/**
 * Protocol:
 */

static struct sockaddr_in broadcast_addr;
static int sockfd;
static fd_set readset;
static struct timeval tv;

static int requested_slot = -1;
static double request_sent_time;

static enum {
	STATE_INIT,
	STATE_REQUESTED,
	STATE_PLAYING
} state;

static Player * get_or_create_plajur(int id) {
	if(id != me->id && id < NUM_PLAYERS) {
		Player * p = players[id];
		if(p == NULL)
			p = create_player("PLAJUR", id);
		return p;
	} else {
		return NULL;
	}
}

void init_network() {

	state = STATE_INIT;

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

	request_slot(0);
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
		//printf(">> %s\n", buffer);
		if(strncmp(buffer,"omg ",4)==0) {
				char * data = buffer+4;
				switch(state) {
					case STATE_INIT:
						break;
					case STATE_REQUESTED:
						if(CMD("nak")) {
							int slot;
							sscanf(data, "nak %d", &slot);
							printf("Recived nak for slot %i\n", slot);
							++slot;
							if(slot < 4) 
								request_slot(slot);
							else {
								printf("No free slots, shutting down");
								exit(2);
							}
						}
						break;
					case STATE_PLAYING: 
						if(CMD("req")) {
							int slot;
							sscanf(data, "req %d", &slot);
							if(me != NULL && me->id == slot) {
								sprintf(buffer, "omg nak %i", slot);
								send_msg(buffer);
							} else { 
								sprintf(buffer, "omg hai %i %f %f %f", me->id, me->pos.x, me->pos.y, me->angle);
								send_msg(buffer);
							}
						} else if(CMD("mov")) {
							int id;
							sscanf(data, "mov %d", &id);
							Player * p = get_or_create_plajur(id);
							if(p != NULL) {
								sscanf(data, "mov %d %f %f %f %d %f %f %f",&id, &p->pos.x, &p->pos.y, &p->angle, &p->current_base_texture, &p->dx, &p->dy, &p->da);
							}
						} else if(CMD("hai")) {
							int id;
							sscanf(data, "hai %d", &id);
							Player * p = get_or_create_plajur(id);
							if(p != NULL) {
								sscanf(data, "hai %d %f %f %f %d %f %f %f",&id, &p->pos.x, &p->pos.y, &p->angle);
							}
						} else if(CMD("rot")) {
							int id;
							sscanf(data, "rot %d", &id);
							Player * p = get_or_create_plajur(id);
							if(p != NULL) {
								sscanf(data, "rot %d %f %f",&id, &p->angle, &p->da);
							}
						} else if(CMD("fir")) {
							int id;
							sscanf(data, "fir %d", &id);
							Player * p = get_or_create_plajur(id);
							if(id != me->id && id < NUM_PLAYERS) {
								p->fire = true;
							}
						} else if(CMD("nof")) {
							int id;
							sscanf(data, "nof %d", &id);
							Player * p = players[id];
							if(id != me->id && p!=NULL) {
								p->fire = false;
							}
						} else if(CMD("kil")) {
							int id;
							sscanf(data, "kil %d", &id);
							Player * p = players[id];
							if(p!=NULL) {
								p->dead = 1;
							}
						}
						break;
				}
		} else {
			buffer[size] = 0;
			fprintf(stderr,"Recieved invalid data: %s\n", buffer);
		}
	}

	//Check if the slot request has time out (aka succeded)
	if(request_sent_time+0.5 < curtime() && state == STATE_REQUESTED) {
		ready = true;
		me = create_player(myname, requested_slot);
		state = STATE_PLAYING;
	}
}

void request_slot(int i) {
	requested_slot = i;
	request_sent_time = curtime();
	char buffer[32];
	sprintf(buffer, "omg req %i", i);
	state = STATE_REQUESTED;
	send_msg(buffer);
}

void send_msg(const char * buffer) {
	if(sendto(sockfd, buffer, strlen(buffer)+1, 0, (sockaddr*) &broadcast_addr, sizeof(sockaddr_in))<0) {
		perror("Falid to send massage");
	}
}
