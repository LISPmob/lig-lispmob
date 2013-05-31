/* 
 * lispd_map_register.c
 *
 * This file is part of LISP Mobile Node Implementation.
 * Send registration messages for each database mapping to
 * configured map-servers.
 * 
 * Copyright (C) 2011 Cisco Systems, Inc, 2011. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please send any bug reports or fixes you make to the email address(es):
 *    LISP-MN developers <devel@lispmob.org>
 *
 * Written or modified by:
 *    David Meyer       <dmm@cisco.com>
 *    Preethi Natarajan <prenatar@cisco.com>
 *    Lorand Jakab      <ljakab@ac.upc.edu>
 *    Alexandru Iuhas   <iuhas@ac.upc.edu>
 */

#include <sys/timerfd.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "lispd_external.h"
#include "lispd_lib.h"
#include "lispd_map_register.h"
#include "lispd_pkt_lib.h"
#include "lispd_sockets.h"


/*
 *  build_map_register_pkt
 *
 *  Build the map-register
 *
 */

lispd_pkt_map_register_t *build_map_register_pkt(
        lispd_mapping_elt       *mapping,
        int                     *mrp_len);

/*
 * Send the Map-Register message
 */

int send_map_register(
        lisp_addr_t                 *ms_address,
        lispd_pkt_map_register_t    *mrp,
        int                         mrp_len);



int map_register(
	lispd_mapping_elt	*mapping_elt)
{
  lispd_map_server_list_t   *ms;
  lispd_pkt_map_register_t  *map_register_pkt; 
  int                       mrp_len = 0;
  int                       sent_map_registers = 0;
  uint32_t                  md_len;


  if (!map_servers) {
        lispd_log_msg(LISP_LOG_CRIT, "map_register: No Map Servers conifgured!");
        exit(EXIT_FAILURE);
  }


  if (mapping_elt) {
       if ((map_register_pkt = build_map_register_pkt(mapping_elt, &mrp_len)) == NULL) {
           lispd_log_msg(LISP_LOG_DEBUG_1, "map_register: Couldn't build map register packet");
           return(BAD);
       }

                 //  for each map server, send a register

    ms = map_servers;

    while (ms) {

    /*
     * Fill in proxy_reply and compute the HMAC with SHA-1.
     */

       map_register_pkt->proxy_reply = ms->proxy_reply;
       memset(map_register_pkt->auth_data,0,LISP_SHA1_AUTH_DATA_LEN);   /* make sure */

       if (!HMAC((const EVP_MD *) EVP_sha1(),
                 (const void *) ms->key,
                 strlen(ms->key),
                 (uchar *) map_register_pkt,
                 mrp_len,
                 (uchar *) map_register_pkt->auth_data,
                 &md_len)) {
                    lispd_log_msg(LISP_LOG_DEBUG_1, "HMAC failed for map-register");
                    return(BAD);
                 }

                    /* Send the map register */

       if ((err = send_map_register(ms->address,map_register_pkt,mrp_len)) == GOOD) {
          lispd_log_msg(LISP_LOG_DEBUG_1, "Sent map register for %s/%d to maps server %s",
                                         get_char_from_lisp_addr_t(mapping_elt->eid_prefix),
                                         mapping_elt->eid_prefix_length,
                                         get_char_from_lisp_addr_t(*(ms->address)));             
          sent_map_registers++;
       }else {
          lispd_log_msg(LISP_LOG_WARNING, "Couldn't send map-register for %s",get_char_from_lisp_addr_t(mapping_elt->eid_prefix));
       }
       ms = ms->next;
    }
    free(map_register_pkt);

    if (sent_map_registers == 0){
       lispd_log_msg(LISP_LOG_CRIT, "Couldn't register %s. \n Exiting ...",get_char_from_lisp_addr_t(mapping_elt->eid_prefix));
       exit(EXIT_FAILURE);
    }
    sent_map_registers = 0;
  }

  return(GOOD);
}


/*
 *  build_map_register_pkt
 *
 *  Build the map-register
 *
 */

lispd_pkt_map_register_t *build_map_register_pkt(
        lispd_mapping_elt       *mapping,
        int                     *mrp_len)
{
    lispd_pkt_map_register_t *mrp;
    lispd_pkt_mapping_record_t *mr;

    *mrp_len = sizeof(lispd_pkt_map_register_t) +
              pkt_get_mapping_record_length(mapping);

    if ((mrp = malloc(*mrp_len)) == NULL) {
        lispd_log_msg(LISP_LOG_WARNING, "build_map_register_pkt: Unable to allocate memory for Map Register packet: %s", strerror(errno));
        return(NULL);
    }
    memset(mrp, 0, *mrp_len);

    /*
     *  build the packet
     *
     *  Fill in mrp->proxy_reply and compute the HMAC in 
     *  send_map_register()
     *
     */

    mrp->lisp_type        = LISP_MAP_REGISTER;
    mrp->map_notify       = mnot;              /* TODO conf item */
    mrp->nonce            = 0;
    mrp->record_count     = 1;				/* XXX Just supported one record per map register */
    mrp->key_id           = htons(1);       /* XXX not sure */
    mrp->auth_data_len    = htons(LISP_SHA1_AUTH_DATA_LEN);


    /* skip over the fixed part,  assume one record (mr) */

    mr = (lispd_pkt_mapping_record_t *) CO(mrp, sizeof(lispd_pkt_map_register_t));

    if (pkt_fill_mapping_record(mr, mapping, NULL)) {
        return(mrp);
    } else {
        free(mrp);
        return(NULL);
    }
}


/*
 *  send_map_register
 */

int send_map_register(
        lisp_addr_t                 *ms_address,
        lispd_pkt_map_register_t    *mrp,
        int                         mrp_len)
{
    int result;
    if (ms_address->afi == AF_INET){
        if (default_ctrl_iface_v4 != NULL){
            result = send_udp_ipv4_packet(default_ctrl_iface_v4->ipv4_address,ms_address,0,LISP_CONTROL_PORT,(void *)mrp,mrp_len);
        }else{
            lispd_log_msg(LISP_LOG_DEBUG_1,"send_map_register: No local RLOC compatible with the afi of the Map Server %s",
                    get_char_from_lisp_addr_t(*ms_address));
            result = BAD;
        }
    }else{
        if (default_ctrl_iface_v6 != NULL){
            result = send_udp_ipv6_packet(default_ctrl_iface_v6->ipv6_address,ms_address,0,LISP_CONTROL_PORT,(void *)mrp,mrp_len);
        }else{
            lispd_log_msg(LISP_LOG_DEBUG_1,"send_map_register: No local RLOC compatible with the afi of the Map Server %s",
                    get_char_from_lisp_addr_t(*ms_address));
            result = BAD;
        }
    }
    return result;
}


/*
 * Editor modelines
 *
 * vi: set shiftwidth=4 tabstop=4 expandtab:
 * :indentSize=4:tabSize=4:noTabs=true:
 */
