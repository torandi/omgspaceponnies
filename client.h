#ifndef CLIENT_H
	#define CLIENT_H

	void network();
	void init_network();
	void send_msg(const char * buffer);
	void request_slot(int i);

#endif
