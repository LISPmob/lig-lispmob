#
#	Make the LISP Internet Groper (lig)
#
#	By David Meyer <dmm@1-4-5.net>
#	Copyright 2009 David Meyer
#
#	David Meyer
#	dmm@1-4-5.net
#	Wed Apr  8 13:36:24 2009
#
# 	Map-Register and Non-encapsulated Map-Request Functionality added by Alexandru Iuhas <iuhas@ac.upc.edu>
# 	Mon May 27 17:00:20 2013,  Polytechnic University of Catalonia
#
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     o Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     o Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     o Neither the name of the University nor the names of its contributors
#       may be used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
#	$Header: /mnt/disk1/dmm/src/lig/RCS/Makefile,v 1.1 2010/11/14 20:49:29 dmm Exp $
#


SRC      = lig.c send_map_request.c lib.c cksum.c print.c get_my_ip_addr.c \
	   lispmob/lispd_config.c lispmob/lispd_lib.c lispmob/lispd_locator.c lispmob/lispd_log.c \
	   lispmob/lispd_mapping.c lispmob/lispd_map_register.c lispmob/lispd_pkt_lib.c \
	   lispmob/lispd_sockets.c lispmob/lispd_iface_list.c
INC	 = lig.h lig-external.h lispmob/lispd_external.h lispmob/lispd.h lispmob/lispd_lib.h lispmob/lispd_log.h  \
	   lispmob/lispd_locator.h lispmob/lispd_map_register.h  lispmob/lispd_mapping.h  \
	   lispmob/lispd_pkt_lib.h lispmob/lispd_sockets.h lispmob/lispd_iface_list.h
OBJ	 = $(SRC:%.c=%.o)
EXE      = lig
#
#	man pages
#
#	man is just a target so you can say 'make man'
MAN	 = man
MANSRC	 = lig.1
MANOUT	 = lig.man
#
#	misc junk
#
RCS      = RCS
MISC     = Makefile README 
#
#	compile/load options
#
CC	 = gcc
CFLAGS   = -Wall -Wno-implicit-function-declaration
LDLIBS   = -lcrypto -lssl -lrt -lm
LDFLAGS  = 
#
#
${EXE}: ${OBJ} ${INC} Makefile
	$(CC) -o $@ ${OBJ} $(LDLIBS) $(LDFLAGS)

${MAN}: ${MANSRC}
	groff -t -e -mandoc -Tascii ${MANSRC} | col -bx > ${MANOUT}

clean:
	/bin/rm -f *.o lispmob/*.o ${EXE} ${MANOUT} core a.out Make.log Make.err *~ lispmob/*~

