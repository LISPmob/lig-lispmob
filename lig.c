/* 
 *	lig.c --
 * 
 *	lig -- LISP Internet Groper
 *
 *	Copyright (c) 2010, David Meyer <dmm@1-4-5.net>
 *	All rights reserved.
 *
 *	David Meyer
 *	dmm@1-4-5.net
 *	Thu Apr  9 09:44:57 2009
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
 *	$Header: /mnt/disk1/dmm/src/lig/RCS/lig.c,v 1.1 2010/11/14 20:46:22 dmm Exp $
 *
 */



#include	"lig.h"
#include	"lig-external.h"

#include 	"lispmob/lispd_iface_list.h"
#include 	"lispmob/lispd_external.h"
#include 	"lispmob/lispd_lib.h"
#include 	"lispmob/lispd_locator.h"
#include 	"lispmob/lispd_map_register.h"
#include 	"lispmob/lispd_pkt_lib.h"
#include 	"lispmob/lispd_sockets.h"
#include 	"lispmob/lispd_log.h"
#include 	"lispmob/lispd_mapping.h"
#include 	"lispmob/lispd_config.h"
/*
 *	globals
 */

int			s;			/* send socket */
int			r;			/* receive socket */


unsigned int		*nonce;
uchar			packet[MAX_IP_PACKET];

/*
 *	use these to construct and parse packets
 */

struct ip		*iph;
struct udphdr		*udph;
struct map_reply_pkt	*map_reply;

/*
 *	global options 
 */

unsigned int udp_checksum_disabled	= 0;
unsigned int disallow_eid		= 0;
unsigned int debug			= 0;
unsigned int machinereadable		= 0;
unsigned int mreg			= 0;			// used to specify it is a map-register message
int32_t      iid			= -1;
int 	     encapsulate		= 1;
int	     check			= 0;

/*
 * Set the locator paramaters priority, weight, mpriority, mweight 
 * and the Reachability bit to their default values
 */

void set_locator_defaults(
	uint8_t 		*priority,
	uint8_t 		*weight,
	uint8_t 		*mpriority,
	uint8_t 		*mweight,
	uint8_t 		*reach);



int main(int argc, char *argv[])
{

    struct addrinfo	    hints;
    struct addrinfo	    *res;
    struct timeval	    before;
    struct timeval	    after;
    struct protoent	    *proto;
    struct sockaddr_storage from;
    struct sockaddr_storage my_addr; 

    /*
     * Remember the requested eid and map resolver properties here 
     */

    char *eid		= NULL;
    char *src_ip_addr	= NULL;
    char *eid_name	= NULL;
    char *mr_name	= NULL;
    char *progname	= NULL;
    char *map_resolver	= getenv(LISP_MAP_RESOLVER); /* check for env var */
    int  eid_addrtype	= 0;
    int  eid_length	= 0;
    int  mr_addrtype	= 0;
    int  mr_length	= 0;

    struct sockaddr_storage eid_addr;
    struct sockaddr_storage map_resolver_addr;
    
    /*
     * The following are used in the map-register function
     */
    
    int				src_addrtype		= 0;
    lispd_mapping_elt		*mapping		= NULL;
    char 			*eidpref		= NULL;
    lisp_addr_t			*eid_prefix		= 0;
    uint8_t			eid_prefix_length	= 0;
    char			*loc			= NULL;
    lispd_locator_elt		*locator		= NULL;
    lisp_addr_t			*locator_addr		= 0;
    uint8_t			priority 		= 1;
    uint8_t			weight 			= 100;
    uint8_t			mpriority 		= 1;
    uint8_t			mweight 		= 100;
    char 			*map_server		= getenv(LISP_MAP_RESOLVER); 
    uint8_t			proxy			= 1;
    uint8_t			reach			= UP;
    char			*pass			= NULL;

    
    
	
    int i		= 0;		/* generic counter */
    unsigned int iseed  = 0;		/* initial random number generator */
    unsigned int nonce0 = 0;
    unsigned int nonce1 = 0;   

    /*
     *	Get defaults
     */

    int  count		= COUNT;
    int	 timeout	= MAP_REPLY_TIMEOUT;
    unsigned int port	= 0;		/* if -p <port> specified, put it in here to find overflow */
    emr_inner_src_port	= 0;		
    char  emr_inner_src_port_str[NI_MAXSERV];
    
    
    
    
    /*
     * Variables used for filling in the map-register message
     */
    
    mnot 		= 0;						// Map-Notify bit
    auth		= 1;						// Authoritative bit
    recordttl 		= htonl(DEFAULT_MAP_REGISTER_TIMEOUT);		// Record TTL
    mapvers		= 0;						// Map Version
    
    
    /*
     * Variables used for filling in the map request message
     */
    
    mrauth  		= 0;						// Authoritative bit
    probe		= 0;						// RLOC Probe bit
    smr			= 0;						// SMR bit
    smri		= 0;						// SMR-invoked bit

    /*
     * Temporary data
     */

    int e		= 0;		/* generic errno holder */
    char buf[NI_MAXHOST];		/* buffer for getnameinfo() results */

    /*
     *	parse args				//modified for long opts
     */  

    int  opt		= 0;
    char *optstring	= "bc:dei:m:p:t:s:uvr";
    int longindex	= 0;
    static struct option long_options[] = {
	{"noproxy", 	no_argument,		0, '0'},
	{"mnot",	no_argument,		0, '1'},
	{"recordttl",	required_argument,	0, '2'},
	{"eidmlen",	required_argument,	0, '3'},
	{"notauth",	no_argument,		0, '4'},
	{"mapvers",	required_argument,	0, '5'},
	{"eidpref",	required_argument,	0, '6'},
	{"locator",	required_argument,	0, '7'},
	{"locpr",	required_argument,	0, '8'},
	{"locw",	required_argument,	0, '9'},
	{"locmpr",	required_argument,	0, 'z'},
	{"locmw",	required_argument,	0, 'y'},
	{"notreach",	no_argument,		0, 'x'},
	{"pass",	required_argument, 	0, 'k'},
	{"addloc",	no_argument,		0, 'a'},
	{"mrauth", 	no_argument,		0, 'h'},
	{"probe",	no_argument,		0, 'o'},
	{"smr",		no_argument,		0, 'q'},
	{"smri",	no_argument,		0, 'n'},
	{"noencap",	no_argument,		0, 'l'}
    };

    while ((opt = getopt_long (argc, argv, optstring, long_options, &longindex)) != -1) {
	switch (opt) {
	  case '0':
	    proxy = 0;			// Proxy Reply bit
	    break;
	  case '1':
	    mnot = 1;			// want Map Notify
	    break;
	  case '2':
	    recordttl = atoi(optarg);	
	    break;
	  case '3':									//create the new mapping element with the 
	    eid_prefix_length = atoi(optarg);						//eid prefix address and length specified
	    mapping = new_local_mapping(*eid_prefix, eid_prefix_length, iid);
	    if(mapping == NULL) {
	      lispd_log_msg(LISP_LOG_ERR, "mapping");
	      free(eid_prefix);
	      exit(BAD);
	    }
	    break;
	  case '4':
	    auth = 0;						// Authoritative bit
	    break;
	  case '5':
	    mapvers = atoi(optarg);				// Map Version
	    break;
	  case '6':
	    eid_prefix = (lisp_addr_t *)malloc(sizeof(lisp_addr_t));
	    eidpref = strdup(optarg);
	    get_lisp_addr_from_char(eidpref, eid_prefix);			// get the eid prefix address
	    break;
	  case '7':
	    locator_addr = (lisp_addr_t *)malloc(sizeof(lisp_addr_t));
	    if ((loc = strdup(optarg)) == NULL) {
		perror ("strdup(loc)");
		exit(BAD);
	    }
	    get_lisp_addr_from_char(loc, locator_addr);				// get the locator address
	    check = 1;
	    break;
	  case '8':
	    priority = atoi(optarg);
	    break;
	  case '9':
	    weight = atoi(optarg);
	    break;
	  case 'z':
	    mpriority = atoi(optarg);
	    break;
	  case 'y':
	    mweight = atoi(optarg);
	    break;
	  case 'x':
	    reach = DOWN;
	    break;  
	  case 'k':								// the Map Server password used to encode with HMAC
	    pass = strdup(optarg);
	    break;
	  case 'a':
	   if (check == 1){
	    locator = new_local_locator(locator_addr, reach, priority, weight, mpriority, mweight, 0);			//generate the locator element
	    if(locator == NULL) {
	      lispd_log_msg(LISP_LOG_ERR, "locator");
	      free(locator_addr);
	      exit(BAD);
	    }
	    if ( add_locator_to_mapping(mapping, locator) == 0 ) {							//add the locator element to the mapping
	      lispd_log_msg(LISP_LOG_ERR, "add_locator_to_mapping");
	      exit(BAD);
	    }
	    set_locator_defaults(&priority, &weight, &mpriority, &mweight, &reach);					//reset the locator parameters for the next entry
	    check = 0;
	   }
	   else{
	    printf("WARNING: The --locator option must be specified before each new --addloc option\n"); 
	   }
	    break;
	case 'b':
	    machinereadable += 1;
	    break;
	case 'c':
	    count = atoi(optarg);
	    if ((count < MIN_COUNT) || (count > MAX_COUNT)) {
		fprintf(stderr,
			"%s: Invalid number, specify count in the range (%u:%u)\n",
			argv[0], MIN_COUNT,MAX_COUNT);
		exit(BAD);
	    }
	    break;
	case 'd':
	    debug += 1;
	    break;
	case 'e':
	    disallow_eid = 1;
	    break;
	case 'i':
	    iid = atoi(optarg);
	    if ((iid < 0) || (iid > MAX_IID)) {
		fprintf(stderr,
			"%s: Invalid Instance ID, specify count in the range (%u:%u)\n",
			argv[0], 0, MAX_IID);
		exit(BAD);
	    }
	    printf("Using Instance ID %d\n", iid);
	    break;
	case 'p':
	    if ((port = atoi(optarg)) > MAX_EPHEMERAL_PORT) {
		fprintf(stderr, "%s: Invalid port (%d)\n", argv[0], port);
		exit(BAD);
	    }
	    emr_inner_src_port = (ushort) port;
	    break;
	case 'm':
	    if ((map_resolver = strdup(optarg)) == NULL) {
		perror ("strdup(map_resolver)");
		exit(BAD);
	    }
	    if ((map_server = strdup(optarg)) == NULL) {
		perror ("strdup(map_server)");
		exit(BAD);
	    }
	    break;
	case 'r':							// Specify it is a Map register message
	    mreg = 1;
	    
	    /*
	     * Run through the interfaces and select a usable address as the source address
	     */
        
	    if(load_interface_list() != 1) {
		lispd_log_msg(LISP_LOG_ERR, "load_interface_list");
		exit(BAD);
	    }	
	    set_default_ctrl_ifaces();	
	    break;
	case 's':
	    if ((src_ip_addr = strdup(optarg)) == NULL) {
		perror ("strdup(src_ip_addr)");
		exit(BAD);
	    }
	    break;
	case 't':
	    timeout = atoi(optarg);
	    if ((timeout < MIN_MR_TIMEOUT) || (timeout > MAX_MR_TIMEOUT)) {
		fprintf(stderr,
			"%s: Invalid number, specify timeout in the range (%u:%u) seconds\n",
			argv[0], MIN_MR_TIMEOUT,MAX_MR_TIMEOUT);
		exit(BAD);
	    }
	    break;
	case 'u':
	    udp_checksum_disabled = 1;
	    break;
	case 'h':
	    mrauth = 1;					// Map Request Authoritative bit
	    break;
	case 'o':
	    probe = 1;					// RLOC Probe bit
	    break;
	case 'q':
	    smr = 1;					// SMR bit
	    break;
	case 'n':
	    smri = 1;					// SMR-invoked bit
	    break;
	case 'l':
	    encapsulate = 0;
	    break;
	case 'v':
	    fprintf(stderr, VERSION, argv[0]);
	    exit (GOOD);
	default:
	    fprintf(stderr, USAGE, argv[0]);
	    exit (BAD);
	}
    }

    /*
     * 	Map register
     */
    
    if(mreg) {
      if (src_ip_addr) {									//use the source address specified 
	  src_addrtype = get_afi(src_ip_addr);							//in the commandline
	  if (src_addrtype == AF_INET) {
	    if (get_lisp_addr_from_char(src_ip_addr, default_ctrl_iface_v4->ipv4_address) == BAD) {
	      lispd_log_msg(LISP_LOG_ERR, "source_ipv4");
	    }
	  }
	  else {				
	    if (get_lisp_addr_from_char(src_ip_addr, default_ctrl_iface_v6->ipv6_address) == BAD) {
	      lispd_log_msg(LISP_LOG_ERR, "source_ipv6");
	    }
	  }
      }
      if(add_map_server(map_server, 1, pass, proxy) == 0) {					// add the map server entry to the list
	lispd_log_msg(LISP_LOG_ERR, "add_map_server");
	exit(BAD);
      }
      if(map_register(mapping) == 0) {
	lispd_log_msg(LISP_LOG_ERR, "map_register");
	exit(BAD);
      }
      printf("\nSent Map-Register to %s for %s/%d\n\n", map_server, eidpref, eid_prefix_length);
      exit(GOOD);
    }
    
    
    
    /* 
     *	save the program name somewhere
     */
    

    if ((progname  = strdup(argv[0])) == NULL) {
	perror ("strdup");
	exit(BAD);
    }

    argc -= optind;
    argv += optind;

    if (argc != 1) {
	fprintf(stderr, USAGE, progname);
	exit (BAD);
    }

    /* 
     *	The requested eid should be in argv[0]
     */
    
    
    /*
     * start skip if map register
     *
     */
    
    if (mreg == 0) {
    if ((eid = strdup(argv[0])) == NULL) {		
	perror ("strdup(argv[0])");
	exit(BAD);
    }

    if ((eid_name = strdup(eid)) == NULL) {	
	perror ("strdup(eid)");
	exit(BAD);
    }

    if (map_resolver == NULL) {
	fprintf(stderr,
		  "%s not set and -m not specified\n",
		  LISP_MAP_RESOLVER); 
	fprintf(stderr, USAGE, progname);
	exit(BAD);
    }

    if ((mr_name = strdup(map_resolver)) == NULL) {
	perror ("strdup(map_resolver)");
	exit(BAD);
    }

    /*
     * We'll use getaddrinfo() for name lookups.
     * Explicitly set options by setting up a non-NULL hints
     */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family    = AF_UNSPEC;	/* Allow IPv4 or IPv6 */
    hints.ai_socktype  = SOCK_DGRAM;	/* Datagram socket */
    hints.ai_flags     = AI_ADDRCONFIG;	/* Only return families configured on host */
    hints.ai_protocol  = 0;		/* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;

    
    

    if ((e = getaddrinfo(eid_name, NULL, &hints, &res)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
	exit(BAD);
    }

    /*
     *  Save the eid, eid_addr, eid_addrtype
     *
     */

    if (res != NULL) {
        eid_addrtype = res->ai_family;
	eid_length = res->ai_addrlen;
	if ((e = getnameinfo(res->ai_addr,res->ai_addrlen,
			buf,NI_MAXHOST,NULL,0,NI_NUMERICHOST)) != 0) {
	    fprintf(stderr,"getnameinfo: %s\n",gai_strerror(e));
	    exit(BAD);
	}
	eid = strdup(buf);
	memcpy(&eid_addr, res->ai_addr, res->ai_addrlen);
    }

    freeaddrinfo(res);
    


    /*
     *  likewise for the map resolver
     */

    
    

    if ((e = getaddrinfo(map_resolver, LISP_CONTROL_PORT_STR, &hints, &res)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
	exit(BAD);
    }

    if (res != NULL) {
	mr_addrtype = res->ai_family;
	mr_length = res->ai_addrlen;
	if ((e = getnameinfo(res->ai_addr,res->ai_addrlen,
			buf,NI_MAXHOST,NULL,0,NI_NUMERICHOST)) != 0) {
	    fprintf(stderr,"getnameinfo: %s\n",gai_strerror(e));
	    exit(BAD);
	}
	map_resolver = strdup(buf);
	memcpy(&map_resolver_addr, res->ai_addr, res->ai_addrlen);
    }

    freeaddrinfo(res);
    
    /*
     *	get an array of nonces of size 2*count
     *  (need 2*count as nonces are 64 bit as of 
     *	draft-ietf-lisp-04.txt)
     */

    if ((nonce = (unsigned int *) malloc(2*count*sizeof(unsigned int))) < 0) {
	perror ("malloc (nonce)");
	exit(BAD);
    }

    if ((proto = getprotobyname("UDP")) == NULL) {
	perror ("getprotobyname");
	exit(BAD);
    }


    if ((s = socket(mr_addrtype,SOCK_DGRAM,proto->p_proto)) < 0) {
	perror("SOCK_DGRAM (s)");
	exit(1);
    }

    if ((r = socket(mr_addrtype,SOCK_DGRAM,proto->p_proto)) < 0) {
	perror("SOCK_DGRAM (r)");
	exit(1);
    }

    if (src_ip_addr) {
	if ((e = getaddrinfo(src_ip_addr, NULL, &hints, &res)) != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
	    exit(BAD);
	}
	memcpy(&my_addr, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);	
    } else 
	if (get_my_ip_addr(mr_addrtype,&my_addr)) {
	    fprintf(stderr, "No usable %s source address\n",
		    (mr_addrtype == AF_INET) ? "IPv4" : "IPv6");
	    exit(BAD);
	}
      

    if (debug) {
	if ((e = getnameinfo((struct sockaddr *)&my_addr,SA_LEN(my_addr.ss_family),
			buf,NI_MAXHOST,NULL,0,NI_NUMERICHOST)) != 0) {
	    fprintf(stderr,"getnameinfo: %s\n",gai_strerror(e));
	    exit(BAD);
	}
        printf("Using source address (ITR-RLOC) %s\n", buf);
    }

    /* 
     *	Initialize the random number generator for the nonces
     */
     
    iseed = (unsigned int) time (NULL);
    srandom(iseed);

    /*
     * http://tools.ietf.org/html/draft-larsen-tsvwg-port-randomization-02.txt
     */

    if (!emr_inner_src_port)
	emr_inner_src_port = MIN_EPHEMERAL_PORT +
	    random() % (MAX_EPHEMERAL_PORT - MIN_EPHEMERAL_PORT);

    memset(packet,       0, MAX_IP_PACKET);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family    = mr_addrtype;	/* Bind on AF based on AF of Map-Resolver */
    hints.ai_socktype  = SOCK_DGRAM;	/* Datagram socket */
    hints.ai_flags     = AI_PASSIVE;	/* For wildcard IP address */
    hints.ai_protocol  = proto->p_proto;
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;

    sprintf(emr_inner_src_port_str, "%d", emr_inner_src_port);
    if ((e = getaddrinfo(NULL, emr_inner_src_port_str, &hints, &res)) != 0) {
	fprintf(stderr, "getting local socket: getaddrinfo: %s\n", gai_strerror(e));
	exit(BAD);
    }

    if (bind(r, res->ai_addr, res->ai_addrlen) == -1) {
	perror("bind");
	exit(BAD);
    }

    freeaddrinfo(res);

    for (i = 0; i < count; i++) {

        build_nonce(nonce,i,&nonce0,&nonce1);

	if (debug) {
	    if ((e = getnameinfo((struct sockaddr *)(&map_resolver_addr), mr_length,
			    buf,NI_MAXHOST,NULL,0,NI_NUMERICHOST)) != 0) {
		fprintf(stderr,"getnameinfo: %s\n",gai_strerror(e));
		exit(BAD);
	    }
	    if (encapsulate == 1){
	    	printf("Send encapsulated map-request to %s (%s) for %s (%s) ...\n",
		   	mr_name,
		   	buf,
		   	eid_name,
		   	eid);
	    }
	    else {
	      printf("Send map-request to %s (%s) for %s (%s) ...\n",
		   	mr_name,
		   	buf,
		   	eid_name,
		   	eid);
	    }
	      
	    } else if (machinereadable && i == 0) {
		printf("MAPRESOLVER=%s\nEID=%s\n", mr_name, eid_name);
		
            }                  	
        else if (!machinereadable){
	  if (encapsulate == 1) {
		printf("Send encapsulated map-request to %s for %s ...\n", mr_name, eid_name);
	  }
	  else {
		printf("Send map-request to %s for %s ...\n", mr_name, eid_name);
	  }
	}

	if (send_map_request(s,
			     nonce0,
			     nonce1,
			     &before,
			     (struct sockaddr *)&eid_addr,
			     iid,
			     (struct sockaddr *)&map_resolver_addr,
			     (struct sockaddr *)&my_addr,
			     encapsulate)) {
	    fprintf(stderr, "send_map_request: can't send map-request\n");
	    exit(BAD);
	}

        if (wait_for_response(r,timeout)) {	
	    if (gettimeofday(&after,NULL) == -1) {
		perror("gettimeofday");
		return(BAD);
	    }

	    if (!get_map_reply(r, packet, mr_addrtype, &from))
		continue;			/* try again if count left */

	    map_reply = (struct map_reply_pkt *) packet;

	    if ((e = getnameinfo((struct sockaddr *)&from,SA_LEN(from.ss_family),
			    buf,NI_MAXHOST,NULL,0,NI_NUMERICHOST)) != 0) {
		fprintf(stderr,"getnameinfo: %s\n",gai_strerror(e));
		exit(BAD);
	    }

	    if (find_nonce(map_reply,nonce, (i+1))) {
		print_map_reply(map_reply,
				eid,
				map_resolver,
				strdup(buf),
				tvdiff(&after,&before),
				machinereadable);
		exit(GOOD);
	    } else {	                    /* Otherwise assume its spoofed */
		printf("Spoofed map-reply: 0x%08x/0x%08x-0x%08x/0x%08x\n",
                       nonce0,
		       ntohl(map_reply->lisp_nonce0),
		       nonce1,
		       ntohl(map_reply->lisp_nonce1));
		continue;			/* try again if count left */
	    }

	}					/* timed out */
    }
    printf("*** No map-reply received ***\n");
    exit(GOOD);
    
}

    /*
     * end skip if map register
     *
     */
    
    exit(GOOD);
}


void set_locator_defaults(
	uint8_t 		*priority,
	uint8_t 		*weight,
	uint8_t 		*mpriority,
	uint8_t 		*mweight,
	uint8_t 		*reach)
{
      *priority 		= 1;
      *weight 			= 100;
      *mpriority 		= 1;
      *mweight 			= 100;
      *reach			= UP;
}

