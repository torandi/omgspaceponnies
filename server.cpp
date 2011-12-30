#include <cstdio>

#include "server.h"

#include "protocol.h"
#include "network_lib.h"
#include "socket.h"
#include "player.h"
#include "common.h"

#include <map>
#include <list>

Server * server = NULL;

Server::Server(int port) {
_vars = new nw_var_t[PAYLOAD_SIZE-1]; //Can't be more that this many vars
_network_port = port;
players = std::map<Player*, int>();
_new_connections = std::list<int>();
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
	check_new_connections();

	incoming_network();

	std::map<Player*, int>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		//Run player logic
		it->first->logic(dt);
	}

}

void Server::remove_player(Player * p) {
	close_socket(players[p]);
	_vars[0].i = p->id;
	printf("Disconnected player %s\n", p->nick.c_str());
	players.erase(p);
	send_frame_to_all(NW_CMD_QUIT);
}

void Server::init_network() {
	_sockfd = create_tcp_server(_network_port);
	_broadcast_sockfd = create_udp_socket(BROADCAST_PORT, true);
}

void Server::send_error(int sockfd, const char * msg) {
	fprintf(stderr,"Send error msg: %s\n",msg);
	_vars[0].set_str(msg);
	send_frame(sockfd, no_addr, NW_CMD_ERROR, _vars);
}

void Server::incoming_network() {
	std::map<Player*, int>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		int sockfd = it->second;
		Player * p = it->first;
		if(data_available(sockfd,0,0)) {
			addr_t addr;
			frame_t f = read_frame(sockfd,_vars, &addr);
			switch(f.cmd) {
				case NW_CMD_INVALID:
					printf("Recieved invalid message, closing connection\n");
					remove_player(p);
					break;
				case NW_CMD_QUIT:
					if(p->id == _vars[0].i) {
						remove_player(p);
					} else {
						send_error(sockfd,"QUIT: Invalid player id");
					}
					break;
				case NW_CMD_SPAWN:
					if(p->id == _vars[0].i) {
						p->spawn_remote(vector_t(_vars[1].f, _vars[2].f));
						send_frame_to_all(NW_CMD_SPAWN, p->id);
					} else {
						send_error(sockfd,"SPAWN: Invalid player id");
					}
					break;
				case NW_CMD_SHIELD:
					if(p->id == _vars[0].i) {
						p->shield_angle = _vars[1].f;
						p->full_shield = _vars[2].c;
						send_frame_to_all(NW_CMD_SHIELD, p->id);
					} else {
						send_error(sockfd,"SHIELD: Invalid player id");
					}
					break;
				case NW_CMD_MOVE:
					if(p->id == _vars[0].i) {
						//Save data to player:
						p->pos.x = _vars[1].f;
						p->pos.y = _vars[2].f;
						p->angle = _vars[3].f;
						p->velocity.x = _vars[5].f;
						p->velocity.y = _vars[6].f;
						//Forward to all other:
						send_frame_to_all(NW_CMD_MOVE, p->id);
					} else {
						send_error(sockfd,"MOVE: Invalid player id");
					}
					break;
				case NW_CMD_ROTATE:
					if(p->id == _vars[0].i) {
						//Save data to player:
						p->angle = _vars[1].f;
						//Forward to all other:
						send_frame_to_all(NW_CMD_ROTATE, p->id);
					} else {
						send_error(sockfd,"MOVE: Invalid player id");
					}
					break;
				case NW_CMD_FIRE:
					if(p->id == _vars[0].i) {
						//mark fire
						p->fire = (bool)_vars[1].c;
						//Forward to all other:
						send_frame_to_all(NW_CMD_FIRE, p->id);
					} else {
						send_error(sockfd,"FIRE: Invalid player id");
					}
					break;
			}	
		}
	}
	std::list<int>::iterator new_it;
	for(new_it=_new_connections.begin();new_it!=_new_connections.end(); ++new_it) {
		int sockfd = *new_it;
		if(data_available(sockfd,0,0)) {
			addr_t addr;
			frame_t f = read_frame(sockfd,_vars, &addr);
			switch(f.cmd) {	
				case NW_CMD_INVALID:
					send_error(sockfd, "Invalid message");
					break;
				case NW_CMD_HELLO:
					{
						Player * p = new Player(_vars[0].str, _vars[1].i);
						p->id = _next_player_id++;
						printf("Player %s joined. Assigned id: %i\n", p->nick.c_str(), p->id);
						players[p] = sockfd;
						_vars[0].i = p->id;
						new_it = _new_connections.erase(new_it);
						--new_it;
						send_frame(sockfd, no_addr, NW_CMD_ACCEPT, _vars);
						_vars[1].set_str(p->nick.c_str());
						_vars[2].i = p->team;
						_vars[3].c = 1; //dead
						_vars[4].f = 0;
						_vars[5].f = 0;
						_vars[6].f = 0;
						send_frame_to_all(NW_CMD_JOIN,p->id);
						//Send all players to the new player:
						for(it=players.begin(); it!=players.end(); ++it) {
							if(it->second != sockfd) {
								_vars[0].i = it->first->id;
								_vars[1].set_str(it->first->nick.c_str());
								_vars[2].i = it->first->team;
								_vars[3].c = it->first->dead;
								_vars[4].f = it->first->pos.x;
								_vars[5].f = it->first->pos.y;
								_vars[6].f = it->first->angle;
								send_frame(sockfd, no_addr, NW_CMD_JOIN, _vars);
							}
						}
						break;
					}
				default:
					send_error(sockfd, "Command not accepted when not ACCEPTED");
					break;
			}
		}
	}

	//Check broadcast port:
	if(data_available(_broadcast_sockfd, 0, 0)) {
		addr_t addr;
		frame_t f = read_frame(_broadcast_sockfd,_vars, &addr);
		if(f.cmd == NW_CMD_FIND_SERVER) {
			_vars[0].i = _network_port;
			_vars[1].i = players.size();
			send_frame(_broadcast_sockfd, addr, NW_CMD_EXISTS_SERVER, _vars);
		}
	}
}

void Server::check_new_connections() {
	int sockfd = accept_client(_sockfd);
	if(sockfd != -1) {
		printf("New client connected: %s\n", getpeer(sockfd).c_str());
		_new_connections.push_back(sockfd);
	}
}

void Server::send_frame_to_all(nw_cmd_t cmd, int ignore_player_id) {
	std::map<Player*, int>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		if(ignore_player_id != it->first->id)
			if(!send_frame(it->second,no_addr, cmd, _vars)) {
				remove_player(it->first);
			}
	}
}

void Server::network_kill(Player * killer, Player * killed) {
	_vars[0].i = killer->id;
	_vars[1].i = killed->id;
	send_frame_to_all(NW_CMD_KILL);
}

void Server::network_score(Player * player) {
	int client_sock = players[player];
	_vars[0].i = player->score;
	send_frame(client_sock,no_addr, NW_CMD_SCORE, _vars);
}
