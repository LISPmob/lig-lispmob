/*
 *	lig.h --
 *
 *	By David Meyer <dmm@1-4-5.net>
 *	Copyright 2009 David Meyer
 *
 *	Definitions for lig
 *
 *	David Meyer
 *	dmm@1-4-5.net
 *	Thu Apr 16 14:50:33 2009
 *
 *	IPv6 support added by Lorand Jakab <lj@icanhas.net>
 *	Mon Aug 23 15:26:51 2010 +0200
 *
 *      Machine parsable output added by Job Snijders <job@instituut.net>
 *      Wed Dec 15 11:38:42 CET 2010
 * 
 *	Instance ID support added by Lorand Jakab <lj@icanhas.net>
 *	Thu Jul 26 00:50:51 PDT 2012
 *
 * 	Map-Register and Non-encapsulated Map-Request Functionality added by Alexandru Iuhas <iuhas@ac.upc.edu>
 * 	Mon May 27 17:00:20 2013,  Polytechnic University of Catalonia
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
 *	$Header: /mnt/disk1/dmm/src/lig/RCS/lig.h,v 1.1 2010/11/14 20:47:30 dmm Exp $
 *
 */


#include	<stdio.h>
#include	<unistd.h>
#include 	<getopt.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<ctype.h>
#include        <netdb.h>
#include	<ifaddrs.h>
#include	<strings.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in_systm.h>
#include	<netinet/in.h>
#include	<netinet/udp.h>
#include	<netinet/ip.h>
#include	<netinet/ip6.h>
#include	<arpa/inet.h>
#include	<net/if.h>
#include	<sys/ioctl.h>

typedef enum			{FALSE,TRUE} boolean;
#define	uchar			u_char

#define GOOD			0
#define BAD			-1
#define	MAX_IP_PACKET		4096
#define	COUNT			3
#define MIN_COUNT		1
#define	MAX_COUNT		5
#define MAP_REPLY_TIMEOUT	2	/* seconds */
#define	MIN_MR_TIMEOUT		1	/* seconds */
#define	MAX_MR_TIMEOUT		5	/* seconds */
#define	LISP_MAP_RESOLVER	"LISP_MAP_RESOLVER"
#define LISP_MAP_SERVER		"LISP_MAP_SERVER"			//NEW
#define	LOOPBACK		"127.0.0.1"
#define	LOOPBACK6		"::1"
#define	LINK_LOCAL		"fe80"
#define	LINK_LOCAL_LEN		4
#define	V4EID		        "153.16"
#define	V4EID_PREFIX_LEN        6	/* characters in "153.16" */
#define	V6EID		        "2610:00d0"
#define	V6EID_PREFIX_LEN        9	/* characters in "2610:00d0" */
#define	MIN_EPHEMERAL_PORT	32768
#define	MAX_EPHEMERAL_PORT	65535
#define MAX_IID                 16777215

#define	USAGE	"      Usage:\nFor Map Request: %s [-b] [-c <count>] [-d] [-e] -m <RLOC> [-p <port>] \
[-s <source address>] [-t <timeout>] [-u] [-v] [--probe] [--smr] [--smri] <EID>\n\nFor Encapsulated Map Request: ./lig [-b] \
[-c <count>] [-d] [-e] -m <Map Resolver> [-p <port>] [-s <source address>] [-t <timeout>] [-u] [-v] [--mrauth] [--smri] <EID>\n \
\nFor Map Register: ./lig -r -m <Map Server> --eidpref <EID> --eidmlen <EID mask length> --pass <password> \
[--noproxy] [--mnot] [--recordttl <TTL>] [--notauth] [--mapvers <Map Version>] --locator <RLOC> \
[--locpr <Priority>] [--locw <Weight>] [--locmpr <MPriority>] [--locmw <MWeight>] [--notreach] \
--addloc [-v]\n\nFor more detailed information type man ./lig.1\n\n"		//modified

/*
 *	VERSION 
 *
 *	XXYYZZ, where
 * 
 *	XX is the draft-ietf-lisp-XX.txt version
 *	YY is the draft-ietf-lisp-ms-YY.txt version
 *      ZZ is the lig version
 */

#define	VERSION "%s version 08.05.09\n"


/*
 *	CO --
 *
 *	Calculate Offset
 *
 *	Try not to make dumb mistakes with
 *	pointer arithmetic
 *
 */

#define	CO(addr,len) (((char *) addr + len))

/*
 *      SA_LEN --
 *
 *      sockaddr length
 *
 */

#define SA_LEN(a) ((a == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6))

/*
 *	names for where the udp checksum goes
 */

#ifdef BSD
#define udpsum(x) x->uh_sum
#else
#define udpsum(x) x->check
#endif

/*
 * LISP Types
 */

#define	LISP_MAP_REQUEST	1
#define	LISP_MAP_REPLY		2
#define	LISP_MAP_REGISTER	3
#define LISP_ENCAP_CONTROL_TYPE 8
#define	LISP_CONTROL_PORT	4342
#define	LISP_CONTROL_PORT_STR	"4342"

/*
 *	Map Reply action codes
 */

#define LISP_ACTION_NO_ACTION		0
#define LISP_ACTION_FORWARD		1
#define LISP_ACTION_DROP		2
#define LISP_ACTION_SEND_MAP_REQUEST	3

/*
 *	#define AF_INET         2
 *	#define AF_INET6        10
 *
 */

#define LISP_AFI_IP		1
#define LISP_AFI_IPV6		2
#define LISP_AFI_LCAF		16387
#define	LISP_IP_MASK_LEN	32
#define	LISP_IPV6_MASK_LEN	128



/*
 *	Encapsulated Control Message Format
 * 
 *         0                   1                   2                   3
 *         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      / |                       IPv4 or IPv6 Header                     |
 *    OH  |                      (uses RLOC addresses)                    |
 *      \ |                                                               |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      / |       Source Port = xxxx      |       Dest Port = 4342        |
 *    UDP +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      \ |           UDP Length          |        UDP Checksum           |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    LH  |Type=8 |                   Reserved                            |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      / |                       IPv4 or IPv6 Header                     |
 *    IH  |                  (uses RLOC or EID addresses)                 |
 *      \ |                                                               |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      / |       Source Port = xxxx      |       Dest Port = yyyy        |
 *    UDP +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      \ |           UDP Length          |        UDP Checksum           |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    LCM |                      LISP Control Message                     |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

struct lisp_control_pkt {
 #ifdef LITTLE_ENDIAN
    int rsvd:4;
    int type:4;
 #else
    int type:4;
    int rsvd:4;
 #endif
    uchar reserved[3];
} __attribute__ ((__packed__));

/* 
 *	Map-Request Message Format 
 *    
 *        0                   1                   2                   3
 *        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |Type=1 |A|M|P|S|       Reserved      |   IRC   | Record Count  |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |                         Nonce . . .                           |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |                         . . . Nonce                           |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |         Source-EID-AFI        |   Source EID Address  ...     |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |         ITR-RLOC-AFI 1        |    ITR-RLOC Address 1  ...    |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |                              ...                              |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |         ITR-RLOC-AFI n        |    ITR-RLOC Address n  ...    |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     / |   Reserved    | EID mask-len  |        EID-prefix-AFI         |
 *   Rec +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     \ |                       EID-prefix  ...                         |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |                   Map-Reply Record  ...                       |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *       |                     Mapping Protocol Data                     |
 *       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

struct map_request_pkt {
#ifdef LITTLE_ENDIAN
	uchar           smr_bit:1;
	uchar           rloc_probe:1;
	uchar           map_data_present:1;
	uchar           auth_bit:1;
	uchar           lisp_type:4;
#else
	uchar           lisp_type:4;
	uchar           auth_bit:1;
	uchar           map_data_present:1;
	uchar           rloc_probe:1;
	uchar           smr_bit:1;


#endif
#ifdef LITTLE_ENDIAN
	uchar           reserved1:6;
	uchar		smr_invoked:1;
	uchar		proxy_itr:1;
#else
	uchar		proxy_itr:1;
	uchar		smr_invoked:1;
	uchar           reserved1:6;
#endif	
#ifdef LITTLE_ENDIAN
	ushort          irc:5;
        uchar           reserved2:3;
#else
        uchar           reserved2:3;
	ushort          irc:5;
#endif
	uchar           record_count;
	unsigned int    lisp_nonce0;
	unsigned int    lisp_nonce1;
	ushort          source_eid_afi;
	ushort          itr_afi;
        uchar           originating_itr_rloc[0];
} __attribute__ ((__packed__));

struct map_request_eid {
	uchar 		reserved;
	uchar 		eid_mask_len;
	ushort		eid_prefix_afi;
        uchar           eid_prefix[0];
} __attribute__ ((__packed__));

struct lcaf {
        uint8_t         rsvd1;
        uint8_t         flags;
        uint8_t         type;
        uint8_t         rsvd2;
        uint16_t        len;
} __attribute__ ((__packed__));

struct lcaf_iid {
        uint32_t        iid;
        uint16_t        afi;
        uchar           eid_prefix[0];
} __attribute__ ((__packed__));


/* 
 *	Map-Reply Message Format 
 *
 *	Note: 64 bit nonce is concat(nonce0,nonce1)
 *
 *    
 *          0                   1                   2                   3 
 *          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
 *         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *         |Type=2 |P|E|            Reserved               | Record Count  | 
 *         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *         |                         Nonce0                                | 
 *         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *         |                         Nonce1                                | 
 *  +----> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *  |      |                          Record  TTL                          | 
 *  |      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *  R      | Locator Count | EID mask-len  | ACT |A|M|     Reserved        | 
 *  e      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *  c      |           Reserved            |            EID-AFI            | 
 *  o      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *  r      |                          EID-prefix                           | 
 *  d      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *  |     /|    Priority   |    Weight     |  M Priority   |   M Weight    | 
 *  |    / +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *  |  Loc |           Unused Flags      |R|           Loc-AFI             |  
 *  |    \ +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *  |     \|                             Locator                           | 
 *  +--->  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *         |                     Mapping Protocol Data                     | 
 *         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 */ 

struct map_reply_pkt {
#ifdef LITTLE_ENDIAN
     int            rsvd:2;
     int            echo_nonce_capable:1;
     int            rloc_probe:1;
     int            lisp_type:4;
#else
     int            lisp_type:4;
     int            rloc_probe:1;
     int            echo_nonce_capable:1;
     int            rsvd:2;
#endif
     ushort         reserved;
     uchar          record_count;
     unsigned int   lisp_nonce0;
     unsigned int   lisp_nonce1;
     uchar          data[0];
}  __attribute__ ((__packed__));



struct lisp_map_reply_eidtype {
    unsigned int	record_ttl;
    uchar 		loc_count;
    uchar 		eid_mask_len;
#ifdef LITTLE_ENDIAN
    int			reserved:3;
    int			mobility_bit:1;
    int			auth_bit:1;
    int			action:3;
#else
    int			action:3;
    int			auth_bit:1;
    int			mobility_bit:1;
    int			reserved:3;
#endif
    uchar 		reserved2;
    ushort		reserved3;
    ushort		eid_afi;
    uchar 		eid_prefix[0];         /* ITR-locator than EID-prefix */
} __attribute__ ((__packed__));

struct lisp_map_reply_loctype {
    uchar   priority;
    uchar   weight;
    uchar   mpriority;
    uchar   mweight;
    uchar   unused_flags1;
#ifdef LITTLE_ENDIAN
    uchar   reach_bit:1;
    uchar   unused_flags2:7;
#else
    uchar   unused_flags2:7;
    uchar   reach_bit:1;
#endif
    ushort loc_afi;
    uchar   locator[0];
} __attribute__ ((__packed__));

/*
 *	a RLOC can be v4 or v6 (so far)
 */

struct lisp_addrtype {
    union {
        struct in_addr	ip;
        struct in6_addr ipv6;
    } address;
};


/*
 *  emr_inner_src_port --
 * 
 *  source port in the EMR's inner UDP header. Listen on
 *  this port in the returned map-reply (the source port on
 *  the map-reply is 4342).
 *
 */

ushort emr_inner_src_port;


