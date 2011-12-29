#ifndef CLIENT_H_
#define CLIENT_H_

#include "protocol.h"
#include "network_lib.h"
#include "socket.h"
#include "player.h"
#include <map>

class Client {
	int _sockfd;
	nw_var_t * _vars;

	void send_cmd(nw_cmd_t cmd);

	public:
		void incoming_network();

		std::map<int, Player*> players; 

		Client(const char* host, int port);
		~Client();

      void run(double dt);

		void create_me(const char * nick, int team);

		Player * me;

		void send_move(const vector_t &delta, float da);
		void send_rotate(float da);
		void send_fire();
		void send_spawn();
};

extern Client * client;

#endif
