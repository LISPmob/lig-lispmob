/*
 *
 *	print.c -- 
 *
 *	Various diagnostic print routines for lig
 *
 *	By David Meyer <dmm@1-4-5.net>
 *	Copyright 2009 David Meyer
 *
 *	08072009:
 *		make print_map_reply handle IPv6 RLOCs
 *
 *	09102009:
 *		update to the 04 spec including 64 bit nonces
 *
 *
 *	David Meyer
 *	dmm@1-4-5.net
 *	Thu Apr 23 15:34:18 2009
 *
 *	IPv6 support added by Lorand Jakab <lj@icanhas.net>
 *	Mon Aug 23 15:26:51 2010 +0200
 *	
 *	Machine parsable output added by Job Snijders <job@instituut.net>
 *	Wed Dec 15 11:38:42 CET 2010
 *
 *	Instance ID support added by Lorand Jakab <lj@icanhas.net>
 *	Thu Jul 26 00:50:51 PDT 2012
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
 *	$Header: /mnt/disk1/dmm/src/lig/RCS/print.c,v 1.1 2010/11/14 20:48:01 dmm Exp $
 *
 */

#include	"lig.h"
#include	"lig-external.h"

/*
 *	Print an IP header
 *
 *
 */

void print_ip_header(iph) 
    struct ip	           *iph;
{
    printf("\nIP Header\n");
    printf("=========\n");
    printf("iph->ip_hl\t= %d\n",  iph->ip_hl);
    printf("iph->ip_v\t= %d\n",   iph->ip_v);
    printf("iph->ip_tos\t= %d\n", iph->ip_tos);
    printf("iph->ip_len\t= %d\n", ntohs(iph->ip_len));
    printf("iph->ip_id\t= %d\n",  ntohs(iph->ip_id));
    printf("iph->ip_off\t= %d\n", iph->ip_off);
    printf("iph->ip_ttl\t= %d\n", iph->ip_ttl);
    printf("iph->ip_p\t= %d\n",   iph->ip_p);
    printf("iph->sum\t= 0x%x\n",  iph->ip_sum);
    printf("iph->ip_src\t= %s\n", inet_ntoa(iph->ip_src));
    printf("iph->ip_dst\t= %s\n", inet_ntoa(iph->ip_dst));
}

/*
 *	Print an UDP header
 *
 *
 */

void print_udp_header(udph) 
    struct udphdr          *udph;
{
    printf("\nUDP Header\n");
    printf("==========\n");
#ifdef BSD
    printf("udph->uh_sport\t= %d\n", ntohs(udph->uh_sport));
    printf("udph->uh_dport\t= %d\n",   ntohs(udph->uh_dport));
    printf("udph->uh_ulen\t= %d\n",    ntohs(udph->uh_ulen));
    printf("udph->uh_sum\t= 0x%x\n",  udph->uh_sum);
#else
    printf("udph->source\t= %d\n", ntohs(udph->source));
    printf("udph->dest\t= %d\n",   ntohs(udph->dest));
    printf("udph->len\t= %d\n",    ntohs(udph->len));
    printf("udph->check\t= 0x%x\n",  udph->check);
#endif
}

/*
 *	print_negative_cache_entry
 *
 *	Prettily print a negative cache entry
 *
 */
void print_negative_cache_entry(action)
     int	action;
{

 if (machinereadable) {
    printf("RESULT=\"Negative cache entry\"\n");
 }
 else {
    printf("  Negative cache entry, action: ");
 }

 if (machinereadable) printf("ACTION=");
 
 switch (action) {
    case LISP_ACTION_NO_ACTION:
	printf("no-action\n");
	break;
    case LISP_ACTION_FORWARD:
	printf("forward-native\n");
	break;
    case LISP_ACTION_DROP:
	printf("drop\n");
	break;
    case LISP_ACTION_SEND_MAP_REQUEST:
	printf("send-map-request\n");
	break;
    default:
        if (machinereadable) printf("\"unknown-action (%d)\"\n", action);
	else printf("unknown-action (%d)\n", action);
	break;
    }		
}


/*
 *	set_afi_and_addr_offset
 *
 *	Set up the afi for inet_ntop and set
 *	the addr_offset to calculate location of 
 *	the next locator.
 *
 */

int set_afi_and_addr_offset(loc_afi,afi,addr_offset)
     ushort		loc_afi;
     int		*afi;
     unsigned int	*addr_offset;
{
    switch (loc_afi) {
    case LISP_AFI_IP:
	*afi = AF_INET;
	*addr_offset = sizeof(struct in_addr);
	break;
    case LISP_AFI_IPV6:
	*afi = AF_INET6;
	*addr_offset = sizeof(struct in6_addr);
	break;
    case LISP_AFI_LCAF:
        return 1;
    default:
	fprintf(stderr, "Unknown AFI (0x%x)\n", loc_afi);
	break;
    }
    return 0;
}


/*
 *	Print a map_reply packet. The output format matches Dino's
 *	(to the extent possible).
 *
 *
 */

void print_map_reply(map_reply,requested_eid,mr_to,mr_from,elapsed_time)
    struct map_reply_pkt *map_reply;
    char *requested_eid;
    char *mr_to;
    char *mr_from;
    long elapsed_time;
{
    char			   pw[8];
    char			   buf[256];
    struct lcaf                    *lcaf_ptr       = NULL;
    struct lcaf_iid                *lcaf_iid_ptr   = NULL;
    struct lisp_map_reply_eidtype  *eidtype	   = NULL;
    struct lisp_map_reply_loctype  *loctype        = NULL; 
    const char			   *formatted_addr = NULL;
    unsigned int		   addr_offset     = 0;
    int				   record_count    = 0;
    int				   locator_count   = 0;
    int				   afi             = 0;
    int				   iid             = 0;
    int				   record          = 0;    
    int				   locator         = 0;

    if (machinereadable) 
           printf("RECEIVED_FROM=%s\nRTT=%f\n", mr_from, (double) elapsed_time/1000000);
    else 
           printf("Received map-reply from %s with rtt %f secs\n",
                   mr_from, (double) elapsed_time/1000000);

    if (!machinereadable)   
    printf("\nMapping entry for EID '%s':\n", requested_eid);

    record_count = map_reply->record_count;

    eidtype = (struct lisp_map_reply_eidtype *) &map_reply->data;

    /*
     *	loop through the records
     */

    for (record = 0; record < record_count; record++) {
        if (set_afi_and_addr_offset(ntohs(eidtype->eid_afi),
                &afi,&addr_offset)) {
            lcaf_ptr     = (struct lcaf *) &eidtype->eid_prefix;
            if (lcaf_ptr->type != 2) {
	        fprintf(stderr, "Unknown LCAF (0x%x)\n", lcaf_ptr->type);
                exit(BAD);
            }
            lcaf_iid_ptr = (struct lcaf_iid *) CO(lcaf_ptr, sizeof(struct lcaf));
            iid          = ntohl(lcaf_iid_ptr->iid);
            set_afi_and_addr_offset(ntohs(lcaf_iid_ptr->afi),
                    &afi,&addr_offset);
            addr_offset += sizeof(struct lcaf) + sizeof(struct lcaf_iid);
            if ((formatted_addr = inet_ntop(afi, &lcaf_iid_ptr->eid_prefix,
                            buf, sizeof(buf))) == NULL) {
                perror("inet_ntop");
                exit(BAD);
            }
        } else {
            if ((formatted_addr = inet_ntop(afi, &eidtype->eid_prefix,
                            buf, sizeof(buf))) == NULL) {
                perror("inet_ntop");
                exit(BAD);
            }
        }
        locator_count = eidtype->loc_count;
        if (machinereadable) {
            printf("LOCATOR_COUNT=%i\n", locator_count);
            printf("MAPPING_ENTRY=%s/%d\n", formatted_addr,eidtype->eid_mask_len);
            printf("TTL=%d\nAUTH=%s\nMOBILE=%s\n",
               ntohl(eidtype->record_ttl), 
               eidtype->auth_bit ? "1" : "0", 
               eidtype->mobility_bit ? "1" : "0");
        }
        else {                                  
	printf("%s/%d, instance ID: %d, ",formatted_addr,eidtype->eid_mask_len,iid);
	printf(" via map-reply, record ttl: %d, %s, %s\n", 
	       ntohl(eidtype->record_ttl), 
	       eidtype->auth_bit ? "auth" : "not auth", 
	       eidtype->mobility_bit ? "mobile" : "not mobile");
        }

	if (locator_count) {		/* have some locators */
	    loctype = (struct lisp_map_reply_loctype *)
		CO(eidtype->eid_prefix, addr_offset);

	    if (!machinereadable) printf("  %-40s%-10s%-10s\n","Locator","State","Priority/Weight");

	    /*
	     * loop through the locators (per record)
	     */

	    for (locator = 0; locator < locator_count; locator++) {
                set_afi_and_addr_offset(ntohs(loctype->loc_afi),
					&afi, &addr_offset);
		if ((formatted_addr = inet_ntop(afi, &loctype->locator,
						buf, sizeof(buf))) == NULL) {
		    perror("inet_ntop");
		    exit(BAD);
		}


		if (machinereadable) {
		printf("LOCATOR%i=%s\nLOCATOR%i_STATE=%s\nLOCATOR%i_PRIORITY=%d\nLOCATOR%i_WEIGHT=%d\n",
		       locator,
		       formatted_addr,
		       locator,
		       loctype->reach_bit ? "up" : "down",
		       locator,
		       loctype->priority,
		       locator,
		       loctype->weight);
		       
		}
		else {
		sprintf(pw, "%d/%d", loctype->priority, loctype->weight);
		printf("  %-40s%-10s%-10s\n",
		       formatted_addr,
		       loctype->reach_bit ? "up" : "down",
		       pw);

                }
		loctype = (struct lisp_map_reply_loctype *)
		    CO(loctype, (sizeof(struct lisp_map_reply_loctype) + addr_offset)); 
	    }
	} else {		/* zero locators means negative map reply */
	    print_negative_cache_entry(eidtype->action);
	}

        /* this should be the next record */
	eidtype = (struct lisp_map_reply_eidtype *) loctype;
    }
}


/*
 *	Print a map_reqest packet
 *
 *
 */
/*
void print_map_request(map_request)
    struct map_request_pkt *map_request;
{
    printf("\nMap-Request Packet\n");
    printf("==========================\n");

    printf("smr_bit\t\t\t= %d\n", map_request->smr_bit);
    printf("rloc_probet\t\t= %d\n", map_request->rloc_probe);
    printf("map_data_present\t= %d\n",map_request->map_data_present);
    printf("auth_bit\t\t= 0x%x\n", map_request->auth_bit);
    printf("lisp_type\t\t= %d\n",map_request->lisp_type);
    printf("lisp_nonce\t\t= 0x%08x-0x%08x\n",
	   ntohl(map_request->lisp_nonce0), 
	   ntohl(map_request->lisp_nonce1)); 
    printf("reserved\t\t\t= %d\n",map_request->reserved);
    printf("reserved1\t\t= %d\n",map_request->reserved1);
    printf("record_count\t\t= %d\n",map_request->record_count);
    printf("source_eid_afi\t\t= %d\n",
	   ntohs(map_request->source_eid_afi));
    printf("itr_afi\t\t\t= %d\n", 
	   ntohs(map_request->itr_afi));
    printf("originating_itr_rloc\t= %s\n",
           inet_ntoa(map_request->originating_itr_rloc));
    printf("reserved1\t\t= %d\n",map_request->reserved1);
    printf("eid_prefix\t\t= %s\n",
	   inet_ntoa(map_request->eid_prefix));
    printf("eid_prefix_afi\t\t= %d\n",
	   ntohs(map_request->eid_prefix_afi));
    printf("eid_mask_len\t\t= %d\n",map_request->eid_mask_len);


}
*/
