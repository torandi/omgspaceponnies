#include "protocol.h"

/***
 * These must be in the same order as nw_cmd_t 
 * since protocol[<nw_cmd_t>].cmd must be eq to <nw_cmd_t>  (these are located in protocol.h)
 ***/
extern frame_t protocol[] = {
	//{ CMD_NAME, NUM_VARS, { VAR1_TYPE, VAR2_TYPE.. } },
	{NW_CMD_HELLO, 2, { NW_VAR_STR, NW_VAR_UINT16 } }, //JOIN: nick, team
	{NW_CMD_ACCEPT, 1, {  NW_VAR_UINT16 } }, //JOIN ok: id
	{NW_CMD_JOIN, 3, { NW_VAR_UINT16, NW_VAR_STR, NW_VAR_UINT16 } }, //JOIN: id, nick, team
	{NW_CMD_QUIT, 1, { NW_VAR_UINT16 } }, //QUIT: id
	// MOVE: player_id, x, y, angle, texture_id, velocity x, velocity y, angle velocity
	{NW_CMD_MOVE, 8, { NW_VAR_UINT16, NW_VAR_FLOAT, NW_VAR_FLOAT,NW_VAR_FLOAT,  NW_VAR_UINT16, NW_VAR_FLOAT, NW_VAR_FLOAT, NW_VAR_FLOAT} }, 
	{NW_CMD_FIRE, 1, { NW_VAR_UINT16, NW_VAR_CHAR } }, //player id, 0/1 (1=start, 0=stop)
	{NW_CMD_KILL, 2, { NW_VAR_UINT16, NW_VAR_UINT16 } }, //Killer, killed
	{NW_CMD_SCORE, 1, { NW_VAR_UINT16 }}, //Player score
	{NW_CMD_ERROR, 1, { NW_VAR_STR} }
};
