#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "network_lib.h"

enum nw_cmd_t {
	NW_CMD_MOVE = 0,
	NW_CMD_FIRE
};

struct frame_t {
	const nw_cmd_t cmd;
	const int num_vars;
	const nw_var_type_t var_types[PAYLOAD_SIZE-1]; //There can't be more than PAYLOAD_SIZE-1 num of variables in a frame
};

/***
 * These must be in the same order as nw_cmd_t 
 * since protocol[<nw_cmd_t>].cmd must be eq to <nw_cmd_t> 
 ***/
frame_t protocol[] = {
	{NW_CMD_MOVE, 2, {NW_VAR_FLOAT, NW_VAR_FLOAT}},
	{NW_CMD_FIRE, 2, {NW_VAR_UINT16, NW_VAR_FLOAT}}
};

#endif
