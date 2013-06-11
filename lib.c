/*
 *	lib.c
 *
 *
 *	Copyright (c) 2010, David Meyer <dmm@1-4-5.net>
 *	All rights reserved.
 *
 *	Various lig library routines
 *
 *	David Meyer
 *	dmm@1-4-5.net
 *	Thu Apr 23 15:37:01 2009
 *
 *	IPv6 support added by Lorand Jakab <lj@icanhas.net>
 *	Mon Aug 23 15:26:51 2010 +0200
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     o Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     o Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     o Neither the name of the University nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *	$Header: /mnt/disk1/dmm/src/lig/RCS/lib.c,v 1.1 2010/11/14 20:45:24 dmm Exp $
 *
 */


#include	"lig.h"
#include	"lig-external.h"


/*
 *	tvdiff(last,first) --
 *
 *	Return the difference of 2 timeval structs in ms. Assume
 *	last >= first or its 0.
 *
 */

long tvdiff(last,first)
struct timeval *last;
struct timeval *first;
{

    long	diff;

    diff = ((last->tv_sec - first->tv_sec) * 1000) +
            ((last->tv_usec - first->tv_usec)/1000);

    return((diff > 0) ? diff : 0);
}

/*
 *	wait_for_response(s)
 *
 *	Wait for a response on socket s
 *
 */

int wait_for_response(
        int s,
        int timeout)
{

    struct timeval tv;
    fd_set         readfds;

    tv.tv_sec  = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(s,&readfds);

    if (select(s+1,&readfds,NULL,NULL,&tv) == -1) {
        perror("select");
        exit(BAD);
    } 
    if (FD_ISSET(s,&readfds)) 
        return(1);		    /* got something */
    else 
        return(0);		    /* else we timed out */
}

/*
 *	Retrieve a map-reply from socket r
 */

int get_map_reply(r,packet,afi,from)
int			r;
uchar			*packet;
int			afi;
struct sockaddr		*from;
{

    socklen_t fromlen = SA_LEN(afi);

    memset((char *) from, 0, fromlen);

    if (recvfrom(r,
            packet,
            MAX_IP_PACKET,
            0,
            from,
            &fromlen) < 0) {
        perror("recvfrom");
        exit(BAD);
    }
    if (((struct map_reply_pkt *) packet)->lisp_type == LISP_MAP_REPLY){
        return(1);
    }

    if (debug){
        printf("Packet not a Map Reply (0x%x)\n", ((struct map_reply_pkt *) packet)->lisp_type);
    }
    return(0);
}


/*
 *	build_nonce 
 *
 *	Build and save 64 bit nonce per draft-ietf-lisp-04.txt and RFC 4086.  
 *
 *	Use a simple algorithm (see below).
 *
 */

void build_nonce(nonce,i,nonce0,nonce1)
unsigned int	*nonce;
int		i;
unsigned int	*nonce0;
unsigned int	*nonce1;
{
    nonce[2*i]     = *nonce0 = random()^random();
    nonce[(2*i)+1] = *nonce1 = random()^time(NULL);
}


/*
 *	find_nonce --
 *
 *	Find the matching nonce, if any. Note that the
 *	nonce is two unsigned ints, so the two ints are
 *	at (2*i) and [(2*i)+1]
 *	
 *
 *	09/10/2009:	nonce increased to 64 bits
 */

int find_nonce(map_reply, nonce, count)
struct   map_reply_pkt *map_reply;
unsigned int	         *nonce;
int		          count;

{
    int i;

    for (i = 0; i <= count; i++) {
        if ((ntohl(map_reply->lisp_nonce0) == nonce[(2*i)]) &&
                (ntohl(map_reply->lisp_nonce1) == nonce[(2*i)+1])) {
            return(1);			/* good nonce */
        }
    }	  
    return(0);				/* nope...*/
}

