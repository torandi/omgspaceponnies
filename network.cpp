#include "network.h"
#include "network_lib.h"
#include "socket.h"

int network_port;

static int sockfd;
static nw_var_t * vars = new nw_var_t[PAYLOAD_SIZE-1]; //Can't be more that this many vars


void init_network() {
	sockfd = create_socket(network_port, false);
}

void network() {
	if(data_available(sockfd)) {
		addr_t addr;
		frame_t f = read_frame(sockfd,vars, &addr);
		switch(f.cmd) {
			case NW_CMD_INVALID:
				fprintf(stderr,"Recived invalid package\n");
				break;
			case NW_CMD_MOVE:
				break;
			case NW_CMD_FIRE:
				break;
		}	
	}
}
