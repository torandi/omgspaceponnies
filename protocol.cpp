#include "protocol.h"
#include "network_lib.h"
#include <stdio.h>

/***
 * These must be in the same order as nw_cmd_t 
 * since protocol[<nw_cmd_t>].cmd must be eq to <nw_cmd_t>  (these are located in protocol.h)
 ***/
frame_t protocol[] = {
	//{ CMD_NAME, NUM_VARS, { VAR1_TYPE, VAR2_TYPE.. } },
	{NW_CMD_INVALID, 0, {}},
	{NW_CMD_TEST, 4, { NW_VAR_UINT16, NW_VAR_FLOAT, NW_VAR_CHAR, NW_VAR_STR} },
	{NW_CMD_HELLO, 2, { NW_VAR_STR, NW_VAR_UINT16 } }, //JOIN: nick, team
	{NW_CMD_ACCEPT, 1, {  NW_VAR_UINT16 } }, //JOIN ok: id
	{NW_CMD_JOIN, 7, { NW_VAR_UINT16, NW_VAR_STR, NW_VAR_UINT16, NW_VAR_CHAR, NW_VAR_FLOAT, NW_VAR_FLOAT, NW_VAR_FLOAT } }, //JOIN: id, nick, team, dead, x, y, angle
	{NW_CMD_QUIT, 1, { NW_VAR_UINT16 } }, //QUIT: id
	// MOVE: player_id, x, y, angle, texture_id, velocity x, velocity y, angle velocity
	{NW_CMD_MOVE, 8, { NW_VAR_UINT16, NW_VAR_FLOAT, NW_VAR_FLOAT,NW_VAR_FLOAT,  NW_VAR_UINT16, NW_VAR_FLOAT, NW_VAR_FLOAT, NW_VAR_FLOAT} }, 
	{NW_CMD_ROTATE, 3, { NW_VAR_UINT16, NW_VAR_FLOAT, NW_VAR_FLOAT} }, 
	{NW_CMD_FIRE, 2, { NW_VAR_UINT16, NW_VAR_CHAR } }, //player id, 0/1 (1=start, 0=stop)
	{NW_CMD_KILL, 2, { NW_VAR_UINT16, NW_VAR_UINT16 } }, //Killer, killed
	{NW_CMD_SPAWN, 3, { NW_VAR_UINT16, NW_VAR_FLOAT, NW_VAR_FLOAT} }, 
	{NW_CMD_SCORE, 1, { NW_VAR_UINT16 }}, //Player score
	{NW_CMD_ERROR, 1, { NW_VAR_STR} }
};

void frame_t::print(nw_var_t * vars) {
	printf("Cmd #%i: {", cmd);
	for(int i=0; i < num_vars; ++i) {
		switch(var_types[i]) {
			case NW_VAR_UINT16:
				printf("[i:%i],", vars[i].i);
				break;
			case NW_VAR_FLOAT:
				printf("[f:%f],", vars[i].f);
				break;
			case NW_VAR_CHAR:
				printf("[c:%c],", vars[i].c);
				break;
			case NW_VAR_STR:
				printf("[str:%s]", vars[i].str);
				break;
		}
	}
	printf("}\n");
}
