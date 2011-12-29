#include <cstdio>

#include "server.h"

#include "protocol.h"
#include "network_lib.h"
#include "socket.h"
#include "player.h"
#include "network.h"

#include <map>

Server::Server(int port) {
	_vars = new nw_var_t[PAYLOAD_SIZE-1]; //Can't be more that this many vars
	_network_port = port;
	players = std::map<Player*, int>();
	_next_player_id = 0;
   init_network();
}

Server::~Server() {
	delete[](_vars);
	std::map<Player*, int>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		delete it->first;
		close_socket(it->second);
	}
}

void Server::run(double dt) {
	incoming_network();

	std::map<Player*, int>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		//Run player logic
		it->first->logic(dt);
	}

	outgoing_network();
}

void Server::init_network() {
	_sockfd = create_tcp_server(_network_port);
}

void Server::send_error(int sockfd, const char * msg) {
	addr_t addr;
	fprintf(stderr,"Send error msg: %s\n",msg);
	_vars[0].set_str(msg);
	send_frame(sockfd, addr, NW_CMD_ERROR, _vars);
}

void Server::incoming_network() {
	std::map<Player*, int>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		int sockfd = it->second;
		Player * p = it->first;
		if(data_available(sockfd)) {
			addr_t addr;
			frame_t f = read_frame(sockfd,_vars, &addr);
			switch(f.cmd) {
				case NW_CMD_INVALID:
					send_error(sockfd, "Invalid message");
					break;
				case NW_CMD_QUIT:
					if(p->id == _vars[0].i) {
						players.erase(p);
						//Forward to all other:
						send_frame_to_all(NW_CMD_QUIT);
					} else {
						send_error(sockfd,"QUIT: Invalid player id");
					}
					break;
				case NW_CMD_HELLO:
					p = new Player(_vars[0].str, _vars[1].i);
					p->id = _next_player_id++;
					players[p] = sockfd;
					_vars[0].i = p->id;
					send_frame(sockfd, addr, NW_CMD_ACCEPT, _vars);
					_vars[1].set_str(p->nick.c_str());
					_vars[2].i = p->team;
					send_frame_to_all(NW_CMD_JOIN);
					break;
				case NW_CMD_MOVE:
					if(p->id == _vars[0].i) {
						//Save data to player:
						p->pos.x = _vars[1].f;
						p->pos.y = _vars[2].f;
						p->angle = _vars[3].f;
						p->velocity.x = _vars[5].f;
						p->velocity.y = _vars[6].f;
						p->da = _vars[7].f;
						//Forward to all other:
						send_frame_to_all(NW_CMD_MOVE);
					} else {
						send_error(sockfd,"MOVE: Invalid player id");
					}
					break;
				case NW_CMD_FIRE:
					if(p->id == _vars[0].i) {
						//mark fire
						p->fire = (bool)_vars[1].c;
						//Forward to all other:
						send_frame_to_all(NW_CMD_FIRE);
					} else {
						send_error(sockfd,"FIRE: Invalid player id");
					}
					break;
			}	
		}
	}
}

void Server::outgoing_network() {

}

void Server::send_frame_to_all(nw_cmd_t cmd, int ignore_player_id) {
	addr_t addr;
	std::map<Player*, int>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		if(ignore_player_id != it->first->id)
			send_frame(it->second,addr, cmd, _vars);
	}
}

void Server::network_kill(Player * killer, Player * killed) {
	_vars[0].i = killer->id;
	_vars[1].i = killed->id;
	send_frame_to_all(NW_CMD_KILL);
}

void Server::network_score(Player * player) {
	addr_t addr;
	int client_sock = players[player];
	_vars[0].i = player->score;
	send_frame(client_sock,addr, NW_CMD_SCORE, _vars);
}
