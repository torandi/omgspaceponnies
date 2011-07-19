/*********************************
 * This file contains functions to 
 * abstract the socket useage.
 *********************************/


#ifndef NETWORK_H
#define NETWORK_H

#include "network_lib.h"

#include <stdlib.h>

/**
 * Reads raw data from the socket
 */
int read_raw(int sock,void * buffer,size_t len, int flags, addr_t * src_addr);

#endif
