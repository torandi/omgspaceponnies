#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <math.h>
#include <time.h>

#include "sha1.h"
#include "network_lib.h"
#include "socket.h"
#include "protocol.h"

/*****
 * Contains network function shared by client and server
 *****/


static bool cmp_hash(char hash[HASH_SIZE],char * str,int len);
static void get_hash(char hash[HASH_SIZE],char * str,int len);

static bool get_frame(int sock, network_data_t * nd);

static int ftnw(float f, void * nw);
static int nwtf(void * nw, float * f);
static int strtnw(char * str, void * nw);
static int nwtstr(void * nw, char * str);

/**
 * Sends a frame on the network. 
 * vars[0] will be overwritten with the protocol cmd, put nothing or irrelevant data there
 */
void send_frame(int sock, sockaddr_in * target, nw_cmd_t cmd, nw_var_t * vars) {
	char * nw = (char*)malloc(FRAME_SIZE);
	int pos = HASH_SIZE;	
	uint16_t nwi;

	/**
	 * Select protocol cmd and write cmd to first pos
	 */
	if(protocol[cmd].cmd != cmd)
		fprintf(stderr, "WARNING: Protocol cmd #%d does not match value in frame_t struct\n",cmd);

	const int num_vars = protocol[cmd].num_vars;
	const nw_var_type_t * var_types = protocol[cmd].var_types;
	nw[pos] = cmd;
	pos+=sizeof(char);

	for(int i=0; i < num_vars; ++i) {
		switch(var_types[i]) {
			case NW_VAR_FLOAT:
				pos += ftnw(vars[i].f,nw+pos);
				break;
			case NW_VAR_UINT16:
				nwi = htons(vars[i].i);
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

	char hash[41];
	get_hash(hash,(char*)nw+HASH_SIZE,PAYLOAD_SIZE);
	memcpy(nw,hash,HASH_SIZE);
	send_raw(sock, nw, target);
	free(nw);
}

/**
 * Reads a frame from network and writes the result to vars.
 * Returns the frame (with data about protocol cmd and var types)
 * Fills addr with relevant address data about src
 */ 
frame_t read_frame(int sock, nw_var_t * vars, addr_t * addr) {
	network_data_t nw;
	if(get_frame(sock,&nw)) {
		int pos = 0;
		uint16_t nwi;
		int cmd;

		//Read protocol cmd:
		cmd = ((char*)nw.data)[0];
		pos += sizeof(char);

		const nw_var_type_t * var_types = protocol[cmd].var_types;
		const int num_vars = protocol[cmd].num_vars;

		for(int i=0;i<num_vars;++i) {
			switch(var_types[i]) {
				case NW_VAR_FLOAT:
					pos += nwtf((char*)nw.data+pos, &vars[i].f);
					break;
				case NW_VAR_UINT16:
					memcpy(&nwi, (char*)nw.data+pos,sizeof(uint16_t));
					vars[i].i = ntohs(nwi);
					pos += sizeof(uint16_t);
					break;
				case NW_VAR_CHAR:
					vars[i].c = ((char*)nw.data)[pos];
					pos += sizeof(char);
					break;
				case NW_VAR_STR:
					vars[i].str = (char*)malloc(PAYLOAD_SIZE); //Maximum possible size
					pos += nwtstr(nw.data, vars[i].str);
					break;
			}
		}
		nw.invalidate();
	}
	*addr = nw.addr;
	return {cmd, num_vars, nw_var_type_t};
}

/***************************
 * static functions
 **************************/


/**
 * Recvs next frame from socket
 */
static bool get_frame(int sock, network_data_t * nd) {

	char buffer[FRAME_SIZE];

	int r = read_raw(sock,buffer,FRAME_SIZE, 0, &nd->addr.addr, &nd->addr.len);
	if(r != FRAME_SIZE) {
		fprintf(stderr,"Invalid frame size recived");
		nd->invalidate();
		return false;
	}
	if(cmp_hash(buffer,buffer+HASH_SIZE,PAYLOAD_SIZE)) {
		memcpy(nd->data,buffer+HASH_SIZE,PAYLOAD_SIZE);
		return true;
	} else {
		fprintf(stderr,"Incorrect hash for frame.\n");
		nd->invalidate();
		return false; 
	}
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


/***************
 * nw_var_t
 **************/

nw_var_t::nw_var_t() : str(NULL) {};

nw_var_t::~nw_var_t() {
	if(str!=NULL)
		free(str);
}

/****************
 * network_data_t
 ***************/


network_data_t::network_data_t() {
	_valid = true;
	data = malloc(PAYLOAD_SIZE);
}

network_data_t::network_data_t(network_data_t &nd) {
	data = nd.data;
	addr = nd.addr;
	_valid = nd._valid;
	nd._valid = false;
	nd.data = NULL;
	_valid = true;
}

network_data_t::~network_data_t() {
	if(_valid)
		free(data);
}

char &network_data_t::operator[] (int index) {
	if(_valid && index < PAYLOAD_SIZE ) {
		return *((char*)data + index);
	} else if(!_valid) {
		throw "Reading from invalidated network data";
	} else  {
		throw "Index out of bounds";
	}
}

void network_data_t::invalidate() {
	if(_valid) {
		_valid = false;
		free(data);
		data = NULL;
	}
}

bool network_data_t::valid() {
	return _valid;
}
