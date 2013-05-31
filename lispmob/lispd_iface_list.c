/* 
 * lispd_iface_list.c
 *
 * This file is part of LISP Mobile Node Implementation.
 * Various routines to manage the list of interfaces.
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
 *    Preethi Natarajan <prenatar@cisco.com>
 *    Lorand Jakab      <ljakab@ac.upc.edu>
 *
 */

#include "lispd.h"
#include "lispd_external.h"
#include "lispd_lib.h"
#include "lispd_sockets.h"
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>



lispd_iface_list_elt *head_interface_list = NULL;

lispd_iface_elt *default_out_iface_v4 = NULL;
lispd_iface_elt *default_out_iface_v6 = NULL;

lispd_iface_elt *default_ctrl_iface_v4  = NULL;
lispd_iface_elt *default_ctrl_iface_v6  = NULL;


int load_interface_list()
{

    struct	ifaddrs		*ifaddr;
    struct	ifaddrs		*ifa;

    if (getifaddrs(&ifaddr) == -1) {
	perror("getifaddrs");
	exit(BAD);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
	if (ifa->ifa_addr == NULL)
	    continue;
	if (ifa->ifa_addr != NULL && strcmp("lo", ifa->ifa_name) != 0) {
	    add_interface(ifa->ifa_name);
	}
    }

    freeifaddrs(ifaddr);
    return 1;
}


lispd_iface_elt *add_interface(char *iface_name)
{
    lispd_iface_list_elt *iface_list, *aux_iface_list;
    lispd_iface_elt *iface;

    /* Creating the new interface*/
    if ((iface_list = malloc(sizeof(lispd_iface_list_elt)))==NULL){
        lispd_log_msg(LISP_LOG_WARNING,"add_interface: Unable to allocate memory for iface_list_elt: %s", strerror(errno));
        return(NULL);
    }
    if ((iface = malloc(sizeof(lispd_iface_elt)))==NULL){
        lispd_log_msg(LISP_LOG_WARNING,"add_interface: Unable to allocate memory for iface_elt: %s", strerror(errno));
        free(iface_list);
        return(NULL);
    }
    if ((iface->ipv4_address = malloc(sizeof(lisp_addr_t)))==NULL){
    	lispd_log_msg(LISP_LOG_WARNING,"add_interface: Unable to allocate memory for lisp_addr_t: %s", strerror(errno));
    	free(iface_list);
    	free(iface);
    	return(NULL);
    }
    if ((iface->ipv6_address = malloc(sizeof(lisp_addr_t)))==NULL){
    	lispd_log_msg(LISP_LOG_WARNING,"add_interface: Unable to allocate memory for lisp_addr_t: %s", strerror(errno));
    	free(iface_list);
    	free(iface->ipv4_address);
    	free(iface);
    	return(NULL);
    }
    iface->iface_name = malloc(strlen(iface_name) + 1);   // XXX Must free elsewhere
    strcpy(iface->iface_name, iface_name);
    iface->status = UP;
    iface->ipv4_address = lispd_get_iface_address(iface_name, iface->ipv4_address, AF_INET);
    if (iface->ipv4_address != NULL){
        iface->out_socket_v4 = open_device_binded_raw_socket(iface->iface_name,AF_INET);
    }else {
        iface->out_socket_v4 = -1;
    }
    iface->ipv6_address = lispd_get_iface_address(iface_name, iface->ipv6_address, AF_INET6);
    if (iface->ipv6_address != NULL){
        iface->out_socket_v6 = open_device_binded_raw_socket(iface->iface_name,AF_INET6);
    }else {
        iface->out_socket_v6 = -1;
    }
    iface->head_v4_mappings_list = NULL;
    iface->head_v6_mappings_list = NULL;
    iface_list->iface = iface;
    iface_list->next = NULL;

    /* Add iface to the list */
    if (!head_interface_list){
        head_interface_list = iface_list;
    }else {
        aux_iface_list = head_interface_list;
        while (aux_iface_list->next)
           aux_iface_list = aux_iface_list->next;
        aux_iface_list->next = iface_list;
    }
    lispd_log_msg(LISP_LOG_DEBUG_2,"add_interface: Interface %s added to interfaces lists",iface_name);
    return (iface);
}

/* Search the iface list for the first UP iface that has an 'afi' address*/

lispd_iface_elt *get_any_output_iface(int afi)
{

    lispd_iface_elt *iface;
    lispd_iface_list_elt *iface_list_elt;

    iface_list_elt = head_interface_list;

    iface = NULL;
    
    switch (afi){
        case AF_INET:
            while (iface_list_elt!=NULL){
                if ((iface_list_elt->iface->ipv4_address!=NULL)
                        && (iface_list_elt->iface->status == UP)) {
                    iface = iface_list_elt->iface;
                    break;
                }
                iface_list_elt = iface_list_elt->next;
            }
            break;
        case AF_INET6:
            while (iface_list_elt!=NULL){
                if ((iface_list_elt->iface->ipv6_address!=NULL)
                        && (iface_list_elt->iface->status == UP)) {
                    iface = iface_list_elt->iface;
                    break;
                }
                iface_list_elt = iface_list_elt->next;
            }
            break;
        default:
            lispd_log_msg(LISP_LOG_DEBUG_2, "get_output_iface: unknown afi %d",afi);
            break;
    }

    return (iface);
}




void set_default_ctrl_ifaces()
{
    default_ctrl_iface_v4 = get_any_output_iface(AF_INET);

    if (default_ctrl_iface_v4 != NULL) {
       lispd_log_msg(LISP_LOG_DEBUG_2,"Default IPv4 control iface %s\n",default_ctrl_iface_v4->iface_name);
    }

    default_ctrl_iface_v6 = get_any_output_iface(AF_INET6);
    if (default_ctrl_iface_v6 != NULL) {
        lispd_log_msg(LISP_LOG_DEBUG_2,"Default IPv6 control iface %s\n",default_ctrl_iface_v6->iface_name);
    }

    // XXX alopez If no output interface found exit --> To be modified when iface management implemented
    if (!default_ctrl_iface_v4 && !default_ctrl_iface_v6){
        lispd_log_msg(LISP_LOG_CRIT,"No default control interface. Exiting ...");
        exit(EXIT_FAILURE);
    }
}





/*
 * Editor modelines
 *
 * vi: set shiftwidth=4 tabstop=4 expandtab:
 * :indentSize=4:tabSize=4:noTabs=true:
 */
