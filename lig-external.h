
/*
 *	lig-external.h
 *
 *	By David Meyer <dmm@1-4-5.net>
 *	Copyright 2009 David Meyer
 *
 *	Any externals that everyone needs go here.
 *
 *	David Meyer
 *	dmm@1-4-5.net
 *	Thu Apr  9 12:01:29 2009
 * 
 *  	Map-Register and Non-encapsulated Map-Request Functionality added by Alexandru Iuhas <iuhas@ac.upc.edu>
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
 *	$Header: /mnt/disk1/dmm/src/lig/RCS/lig-external.h,v 1.1 2010/11/14 20:47:00 dmm Exp $
 *
 */


extern  unsigned int	debug;
extern	unsigned int	machinereadable;
extern  unsigned int	disallow_eid;
extern  unsigned int	udp_checksum_disabled;
extern	ushort		emr_inner_src_port;

	u_char		mrauth;	
	u_char		smri;
	u_char		probe;
	u_char		smr;

#ifdef BSD
#include <string.h>
#else
extern void		*memcpy();
extern void		*memset();
extern char		*strdup();
#endif

extern ushort		csum ();
extern long		tvdiff();
