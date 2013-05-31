/*
 * lispd_iface_list.h
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
 *    Albert López      <alopez@ac.upc.edu>
 *
 */

#ifndef LISPD_IFACE_LIST_H_
#define LISPD_IFACE_LIST_H_

#include "lispd.h"
#include "lispd_mapping.h"


/*
 * Interface structure
 * locator address (rloc) is linked to the interface address. If changes the address of the interface
 * , the locator address change automatically
 */
typedef struct lispd_iface_elt_ {
    char                        *iface_name;
    uint8_t                     status;
    lisp_addr_t                 *ipv4_address;
    lisp_addr_t                 *ipv6_address;
    lispd_mappings_list         *head_v4_mappings_list;
    lispd_mappings_list         *head_v6_mappings_list;
    int                         out_socket_v4;
    int                         out_socket_v6;
}lispd_iface_elt;

/*
 * List of interfaces
 */
typedef struct lispd_iface_list_elt_ {
    lispd_iface_elt                  *iface;
    struct lispd_iface_list_elt_     *next;
}lispd_iface_list_elt;


/*
 * Run through the available interfaces and add the usable ones to the interface list
 */

int load_interface_list();

/*
 * Return the interface if it already exists. If it doesn't exist,
 * create and add an interface element to the list of interfaces.
 */

lispd_iface_elt *add_interface(char *iface_name);



lispd_iface_elt *get_any_output_iface();



/*
 * Init the default interfaces to send control packets
 */
void set_default_ctrl_ifaces();


#endif /*LISPD_IFACE_LIST_H_*/
