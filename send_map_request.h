/*
 * send_map_request.h
 *
 *  Created on: Feb 29, 2016
 *      Author: alopez
 */

#ifndef SEND_MAP_REQUEST_H_
#define SEND_MAP_REQUEST_H_

int send_map_request(
        int             send_socket,
        unsigned int    nonce0,
        unsigned int    nonce1,
        struct timeval  *before,
        struct sockaddr *eid_addr,
        struct sockaddr *map_resolver_addr,
        struct sockaddr *my_addr,
        struct sockaddr *src_eid,
        int             encapsulate);

#endif /* SEND_MAP_REQUEST_H_ */
