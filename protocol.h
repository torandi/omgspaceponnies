#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "network_lib.h"

enum nw_cmd_t {
	NW_CMD_INVALID = 0,
	NW_CMD_MOVE,
	NW_CMD_FIRE
};

struct frame_t {
	nw_cmd_t cmd;
	int num_vars;
	nw_var_type_t var_types[PAYLOAD_SIZE-1]; //There can't be more than PAYLOAD_SIZE-1 num of variables in a frame
};

extern frame_t protocol[];

#endif
