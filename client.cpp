#include <cstdio>

#include "client.h"

#include "protocol.h"
#include "network_lib.h"
#include "socket.h"
#include "player.h"
#include "render.h"
#include "common.h"

#include <map>

Client * client = NULL;

Client::Client(const char * hostname, int port) {
	_vars = new nw_var_t[PAYLOAD_SIZE-1]; //Can't be more that this many vars
	players = std::map<int, Player*>();
	_sockfd = create_tcp_client(hostname, port);
	me = NULL;
}

Client::~Client() {
	delete[](_vars);
	std::map<int, Player*>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		delete it->second;
	}
	close_socket(_sockfd);
}

void Client::run(double dt) {
	std::map<int, Player*>::iterator it;
	for(it=players.begin(); it!=players.end(); ++it) {
		//Run player logic
		if(it->first != me->id)
			it->second->logic(dt);
	}

}

void Client::incoming_network() {
	if(data_available(_sockfd)) {
		addr_t addr;
		frame_t f = read_frame(_sockfd,_vars, &addr);
		if(ready) {
			Player * p;
			switch(f.cmd) {
				case NW_CMD_INVALID:
					fprintf(stderr, "Recived invalid package\n");
					break;
				case NW_CMD_QUIT:
					printf("Delete user %i:%s\n", _vars[0].i, players[_vars[0].i]->nick.c_str());
					players.erase(_vars[0].i);
					break;
				case NW_CMD_MOVE:
					p = players[_vars[0].i];
					//Save data to player:
					p->pos.x = _vars[1].f;
					p->pos.y = _vars[2].f;
					p->angle = _vars[3].f;
					p->velocity.x = _vars[5].f;
					p->velocity.y = _vars[6].f;
					p->da = _vars[7].f;
					break;
				case NW_CMD_ROTATE:
					p = players[_vars[0].i];
					p->angle = _vars[1].f;
					p->da = _vars[2].f;
					break;
				case NW_CMD_FIRE:
					p = players[_vars[0].i];
					p->fire = (bool)_vars[1].c;
					break;
				case NW_CMD_JOIN:
					p = new Player(_vars[1].str, _vars[2].i);
					p->id = _vars[0].i;
					players[p->id] = p;
					printf("Added player %i:%s\n", p->id, p->nick.c_str());
					break;
				case NW_CMD_KILL:
					{
						Player * killer = players[_vars[0].i];
						Player * killed = players[_vars[1].i];

						killed->dead = 1;
						log_message(killer->nick+" killed "+killed->nick);
						char buffer[64];
						if(killed->id == me->id) {
							queue_announcement(std::string("KILLED BY ")+killer->nick);
						} else if(killer->id == me->id && killed->team != me->team) {
							sprintf(buffer, "KILLED %s! %ip", killed->nick.c_str(), KILL_SCORE);
							queue_announcement(std::string(buffer));
						} else if(killer->id == me->id)  {
							sprintf(buffer, "TEAMKILL! %ip", TEAM_KILL_SCORE);
							queue_announcement(std::string(buffer));
						}
					}
					break;
				case NW_CMD_SCORE:
					{
						me->score = _vars[0].i;
						char buffer[128];
						sprintf(buffer,"Your score: %i", me->score);
						log_message(std::string(buffer));
					}
				case NW_CMD_ERROR:
					fprintf(stderr, "Server reported error: %s\n", _vars[0].str);
					break;
			}	
		} else {
			switch(f.cmd) {
				case NW_CMD_INVALID:
					fprintf(stderr, "Recived invalid package\n");
					break;
				case NW_CMD_ACCEPT:
					me->id=_vars[0].i;
					printf("Accepted, player id: %i\n", me->id);
					players[me->id] = me;
					me->spawn();
					ready = true;
					break;
				default:
					printf("Ignore message (cmd: %i)\n", f.cmd);
					break;
		}
	}
}
}


void Client::create_me(const char * nick, int team) {
	me = new Player(nick, team);
	_vars[0].set_str(nick);
	_vars[1].i = team;
	send_frame(_sockfd, no_addr, NW_CMD_HELLO, _vars);
}

void Client::send_move(const vector_t &delta, float da) {
	_vars[0].i = me->id;
	_vars[1].f = me->pos.x;
	_vars[2].f = me->pos.y;
	_vars[3].f = me->angle;
	_vars[4].i = me->current_base_texture;
	_vars[5].f = delta.x;
	_vars[6].f = delta.y;
	_vars[7].f = da;
	send_frame(_sockfd, no_addr, NW_CMD_MOVE, _vars);
}

void Client::send_fire() {
	_vars[0].i = me->id;
	_vars[1].c = me->fire;
	send_frame(_sockfd, no_addr, NW_CMD_FIRE, _vars);
}


void Client::send_rotate(float da) {
	_vars[0].i = me->id;
	_vars[1].f = me->angle;
	_vars[2].f = da;
	send_frame(_sockfd, no_addr, NW_CMD_ROTATE,_vars);
}	
