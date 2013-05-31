/*
 * lispd_config.c
 *
 * This file is part of LISP Mobile Node Implementation.
 * Handle lispd command line and config file
 * Parse command line args using gengetopt.
 * Handle config file with libconfuse.
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
 *    Alberto Rodriguez Natal <arnatal@ac.upc.edu>
 *
 */


#include "lispd_external.h"
#include "lispd_iface_list.h"
#include "lispd_lib.h"
#include "lispd_mapping.h"
#include "lispd_config.h"



#ifdef OPENWRT
#include <uci.h>
#include <libgen.h>
#include <string.h>
#endif






int add_map_server(
        char         *map_server,
        int          key_type,
        char         *key,
        uint8_t      proxy_reply)

{
    lisp_addr_t             *addr;
    lispd_map_server_list_t *list_elt;
    struct hostent          *hptr;

    if ((addr = malloc(sizeof(lisp_addr_t))) == NULL) {
        lispd_log_msg(LISP_LOG_WARNING, "add_map_server: Unable to allocate memory for lisp_addr_t: %s", strerror(errno));
        return(BAD);
    }

    /*
     *  make sure this is clean
     */
    // XXX alopez: to be revised

    memset(addr,0,sizeof(lisp_addr_t));

    if (((hptr = gethostbyname2(map_server,AF_INET))  == NULL) &&
            ((hptr = gethostbyname2(map_server,AF_INET6)) == NULL)) {
        lispd_log_msg(LISP_LOG_WARNING, "can gethostbyname2 for map_server (%s)", map_server);
        free(addr);
        return(BAD);
    }


    memcpy((void *) &(addr->address),
            (void *) *(hptr->h_addr_list), sizeof(lisp_addr_t));
    addr->afi = hptr->h_addrtype;

    /*
     * Check that the afi of the map server matches with the default rloc afi (if it's defined).
     */
    if (default_rloc_afi != -1 && default_rloc_afi != addr->afi){
        lispd_log_msg(LISP_LOG_WARNING, "The map server %s will not be added due to the selected default rloc afi",map_server);
        free(addr);
        return(BAD);
    }

    if ((list_elt = malloc(sizeof(lispd_map_server_list_t))) == NULL) {
        lispd_log_msg(LISP_LOG_WARNING, "add_map_server: Unable to allocate memory for lispd_map_server_list_t: %s", strerror(errno));
        free(addr);
        return(BAD);
    }

    memset(list_elt,0,sizeof(lispd_map_server_list_t));

    list_elt->address     = addr;
    list_elt->key_type    = key_type;
    list_elt->key         = strdup(key);
    list_elt->proxy_reply = proxy_reply;

    /*
     * hook this one to the front of the list
     */

    if (map_servers) {
        list_elt->next = map_servers;
        map_servers = list_elt;
    } else {
        map_servers = list_elt;
    }

    return(GOOD);
}



/*
 * Editor modelines
 *
 * vi: set shiftwidth=4 tabstop=4 expandtab:
 * :indentSize=4:tabSize=4:noTabs=true:
 */

