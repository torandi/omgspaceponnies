#ifndef NETWORK_LIB_INC
#define NETWORK_LIB_INC



#define FRAME_SIZE 32
#define HASH_SIZE 4
#define PAYLOAD_SIZE 28

/**
 * A variable from/to network traffic
 */
struct nw_var_t {
	float f;
	uint16_t i;
	char c;
	char * str;

	nw_var_t() : str(NULL) {};

	~nw_var_t() {
		if(str!=NULL)
			free(str);
	}
};

struct network_data_t {
	void * data;
	sockaddr src_addr;
	socklen_t src_addr_len;
		
private:
	bool _valid;

public:

	network_data_t() {
		_valid = true;
		data = malloc(PAYLOAD_SIZE);
	}

	network_data_t(network_data_t &nd) {
		data = nd.data;
		src_addr = nd.src_addr;
		src_addr_len = nd.src_addr_len;
		_valid = nd._valid;
		nd._valid = false;
		nd.data = NULL;
		_valid = true;
	}

	~network_data_t() {
		if(_valid)
			free(data);
	}

	char &operator[] (int index) {
		if(_valid && index < PAYLOAD_SIZE ) {
			return *((char*)data + index);
		} else if(!_valid) {
			throw "Reading from invalidated network data";
		} else  {
			throw "Index out of bounds";
		}
	}

	void invalidate() {
		if(_valid) {
			_valid = false;
			free(data);
			data = NULL;
		}
	}

	bool valid() {
		return _valid;
	}
};

enum nw_var_type_t {
	NW_VAR_FLOAT,
	NW_VAR_UINT16,
	NW_VAR_CHAR,
	NW_VAR_STR
};

void test_network();

void send_msg(int sock, sockaddr_in * target, nw_var_type_t * var_types, nw_var_t * vars, int num_vars);

#endif
