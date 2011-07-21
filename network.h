#ifndef NETWORK_H
#define NETWORK_H
	extern int network_port;

	//Default port
	#define PORT 7412

	void init_network();
	void network();
	void send_player(const Player &p);
	void send_fire(const Player &p);

#endif
