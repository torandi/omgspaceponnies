#ifndef SERVER_H_
#define SERVER_H_

#include "protocol.h"
#include "network_lib.h"
#include "socket.h"
#include "player.h"
#include <map>
#include <list>


class Server {
	int _network_port;
	int _sockfd;
	int _broadcast_sockfd;
	int _next_player_id;
	nw_var_t * _vars;

	std::list<int> _new_connections; //List of all sockets that doesn't have a player yet

	void incoming_network();
   void init_network();

	void send_error(int sockfd, const char * msg);

	// Sends a network cmd to all, set _vars before calling
	void send_frame_to_all(nw_cmd_t cmd, int ignore_player_id=-1);

	void remove_player(Player * p);

	public:
		std::map<Player*, int> players; //Int is socket addr

		Server(int port);
		~Server();

      void run(double dt);

		void network_kill(Player * killer, Player * killed);
		void network_score(Player * player);
			
		void check_new_connections();


			
};

extern Server * server;

#endif
