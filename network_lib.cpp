#define __STDC_LIMIT_MACROS
#include <stdint.h>

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

#include <math.h>
#include <time.h>

#include "sha1.h"
#include "network_lib.h"

/*****
 * Contains network function shared by client and server
 *****/


static bool cmp_hash(char hash[HASH_SIZE],char * str,int len);
static void get_hash(char hash[HASH_SIZE],char * str,int len);

static void send(int sockfd, void * data, sockaddr_in * to_addr);
static void write_msg(int sock, void * data, sockaddr_in * to_addr);
static network_data_t read_frame(int sock);

static int ftnw(float f, void * nw);
static int nwtf(void * nw, float * f);
static int strtnw(char * str, void * nw);
static int nwtstr(void * nw, char * str);

void send_msg(int sock, sockaddr_in * target, nw_var_type_t * var_types, nw_var_t * vars, int num_vars) {
	char * nw = (char*)malloc(FRAME_SIZE);
	int pos = 0;	
	for(int i=0; i < num_vars; ++i) {
		switch(var_types[i]) {
			case NW_VAR_FLOAT:
				pos += ftnw(vars[i].f,nw+pos);
				break;
			case NW_VAR_UINT16:
				uint16_t nwi = htons(vars[i].i);
				memcpy(nw+pos,&nwi,sizeof(uint16_t));
				pos += sizeof(uint16_t);
				break;
			case NW_VAR_CHAR:
				nw[pos] = vars[i].c;
				pos += sizeof(char);
				break;
			case NW_VAR_STR:
				pos += strtnw(vars[i].str, nw);
				break;
		}
	}
	write_msg(sock, nw, target);
}

/**
 * Reads a frame from network. Returns an invalidated network_data_t (with srcaddr)
 */ 
network_data_t read_msg(int sock, nw_var_type_t * var_types, nw_var_t * vars, int num_vars) {
	network_data_t nw = read_frame(sock);
	if(nw.valid()) {	
		int pos = 0;
		for(int i=0;i<num_vars) {
			switch(var_types[i]) {
				case NW_VAR_FLOAT:
					pos += nwtf((char*)nw.data+pos, &vars[i].f);
					break;
				case NW_VAR_UINT16:
					uint16_t nwi;
					memcpy(&nwi, (char*)nw.data+pos,sizeof(uint16_t));
					vars[i].i = ntohs(nwi);
					pos += sizeof(uint16_t);
					break;
				case NW_VAR_CHAR:
					vars[i].c = (char*)nw.data[pos];
					pos += sizeof(char);
					break;
				case NW_VAR_STR:
					vars[i].str = malloc(PAYLOAD_SIZE); //Maximum possible size
					pos += nwtstr(nw, vars[i].str);
					break;
			}
		}
		nw.invalidate();
	}
	return nw;
}

/***************************
 * static functions
 **************************/

/**
 * Reads a frame
 */
static network_data_t read_frame(int sock) {
	network_data_t nd;
	char buffer[FRAME_SIZE];

	int r = recvfrom(sock,buffer,FRAME_SIZE, 0, &nd.src_addr, &nd.src_addr_len);
	if(r != FRAME_SIZE) {
		fprintf(stderr,"Invalid frame size recived");
		nd.invalidate();
		return nd;
	}
	if(cmp_hash(buffer,buffer+HASH_SIZE,PAYLOAD_SIZE)) {
		memcpy(nd.data,buffer+HASH_SIZE,PAYLOAD_SIZE);
	} else {
		fprintf(stderr,"Incorrect hash for frame.\n");
		nd.invalidate();
		return nd;
	}
	return nd;
}

static void write_msg(int sock, void * data, sockaddr_in * to_addr) {
		char * buffer =(char*) malloc(FRAME_SIZE);
		char hash[41];
		get_hash(hash,(char*)data,PAYLOAD_SIZE);
		memcpy(buffer,hash,HASH_SIZE);
		memcpy(buffer+HASH_SIZE, data, PAYLOAD_SIZE);
		send(sock, data, to_addr);
		free(buffer);	
}

static void get_hash(char * hexstring,char * str, int len) {
	unsigned char hash[20];
	sha1::calc(str,len,hash);
	sha1::toHexString(hash, hexstring);
}

static bool cmp_hash(char hash[HASH_SIZE],char * str,int len) {
	char hexstring[41];
	get_hash(hexstring,str,len);
	return (strncmp(hash,hexstring,HASH_SIZE) == 0);
}

static void send(int sockfd, void * data, sockaddr_in * to_addr) {
	if(sendto(sockfd, data, PAYLOAD_SIZE, 0, (sockaddr*) to_addr, sizeof(sockaddr_in))<0) {
		perror("Failed to send message");
	}
}

/** 
 * Converts the float to a network format and write it to nw, returns the number of bytes written (probably 3)
 */
static int ftnw(float f, void * nw) {
	if(fabs(f) >= UINT16_MAX) {
		fprintf(stderr,"Number to large. |%f| > %d\n", f,UINT16_MAX);
		return 0;
	}
	bool negative = (f<0);
	f = fabs(f);
	uint16_t integer_part;
	unsigned char decimal_part;
	integer_part = (uint16_t)floor(f);
	decimal_part = (unsigned char)floor((f - integer_part)*100.0);
	if(decimal_part > 99) {
		fprintf(stderr, "Internal error in ftnw(), decimal part (%d) > 99\n",decimal_part);
		return 0;
	}
	if(negative) {
		decimal_part+=100; //0-99: positive, 100-199: negative
	}

	uint16_t nw_order = htons(integer_part);
	memcpy(nw,&nw_order, sizeof(nw_order));
	((unsigned char*)nw)[sizeof(nw_order)] = decimal_part;

	return sizeof(nw_order)+sizeof(unsigned char);
}

static int nwtf(void * nw, float * f) {
	uint16_t integer_part;
	unsigned char decimal_part;
	bool negative = false;
	
	integer_part = ntohs(*((uint16_t*)nw));
	decimal_part = *((unsigned char*)nw+sizeof(uint16_t));
		
	if(decimal_part > 99) {
		negative = true;
		decimal_part -=100;
	}

	*f = integer_part + (float)decimal_part/100.0;
	if(negative)
		*f *= -1.0;

	return sizeof(uint16_t)+sizeof(unsigned char);
}

static int strtnw(char * str, void * nw) {
	int len = strlen(str);
	((char*)nw)[0] = len;
	memcpy((char*)nw+1,str,len);
	return len+1;
}

/**
 * Reads a string from the network buffer and writes to str.
 */ 
static int nwtstr(void * nw, char * str) {
	int len = ((char*)nw)[0];
	str[len] = 0x00;
	memcpy(str,(char*)nw+1,len);
	return len+1;
}

void test_network() {
	void * nw = malloc(10);
	printf("Testing converting positive floats back and forth:\n");
	srand(time(NULL));
	int errors = 0;
	for(int i = 0; i<1000;++i) {
		int n = rand() % UINT16_MAX;
		float f = (float)n/(float)100.0;
		printf("Converting float %f...",f);
		if(ftnw(f, nw)==3) {
			float b;
			nwtf(nw,&b);
			if(fabs(f-b) > 0.019) {
				printf("diff: %f (b: %f)\n", fabs(f-b),b);
				++errors;
			} else {
				printf("OK\n");
			}
		} else 
			++errors;
	}

	printf("Testing converting negative floats back and forth:\n");
	srand(time(NULL));
	for(int i = 0; i<1000;++i) {
		int n = rand() % UINT16_MAX;
		float f = (float)n/(float)100.0;
		f *=-1;
		printf("Converting float %f...",f);
		if(ftnw(f, nw)==3) {
			float b;
			nwtf(nw, &b);
			if(fabs(f-b) > 0.019) {
				printf("diff: %f (b: %f)\n", fabs(f-b),b);
				++errors;
			} else {
				printf("OK\n");
			}
		} else 
			++errors;
	}
	printf("Number of errors: %d/2000\n",errors);
	free(nw);
}

