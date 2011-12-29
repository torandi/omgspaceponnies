#ifndef CLIENT_H_
#define CLIENT_H_

#include "protocol.h"
#include "network_lib.h"
#include "socket.h"
#include "player.h"
#include <map>

class Client {
	int _network_port;
	int _sockfd;
	nw_var_t * _vars;


	public:
		void incoming_network();
		void outgoing_network();

		std::vector<int, Player*> players; 

		Client(const char* host, int port);
		~Client();

      void run(double dt);

		void create_me(const char * nick, int team);

		Player * me;
};

extern Client * client;

#endif
