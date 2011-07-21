#ifndef NETWORK_LIB_H
#define NETWORK_LIB_H

#include <stdint.h>
#include "socket.h"

#define FRAME_SIZE 32
#define HASH_SIZE 4
#define PAYLOAD_SIZE 28

/**
 * A variable from/to network traffic
 */
struct nw_var_t {
	float f;
	uint16_t i;
	char c;
	char * str;

	nw_var_t();

	~nw_var_t();
};

struct network_data_t {
	void * data;
	addr_t addr;	

private:
	bool _valid;

public:

	network_data_t();
	network_data_t(network_data_t &nd);
	~network_data_t();
	char &operator[] (int index);
	void invalidate();
	bool valid();
};

enum nw_var_type_t {
	NW_VAR_FLOAT,
	NW_VAR_UINT16,
	NW_VAR_CHAR,
	NW_VAR_STR
};


void test_network();

/**
 * Sends a frame on the network. 
 * vars[0] will be overwritten with the protocol cmd, put nothing or irrelevant data there
 */
void send_frame(int sock, sockaddr_in * target, nw_var_type_t * var_types, nw_var_t * vars, int num_vars);

addr_t read_frame(int sock, nw_var_type_t * var_types, nw_var_t * vars, int num_vars);

#endif
