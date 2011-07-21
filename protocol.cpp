#include "protocol.h"

/***
 * These must be in the same order as nw_cmd_t 
 * since protocol[<nw_cmd_t>].cmd must be eq to <nw_cmd_t> 
 ***/
extern frame_t protocol[] = {
	{NW_CMD_MOVE, 2, {NW_VAR_FLOAT, NW_VAR_FLOAT}},
	{NW_CMD_FIRE, 2, {NW_VAR_UINT16, NW_VAR_FLOAT}}
};
