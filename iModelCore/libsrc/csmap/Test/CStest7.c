/*
 * Copyright (c) 2008, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autodesk, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "cs_map.h"
#include "cs_Test.h"
#include "csNameMapperSupport.h"

extern int cs_Error;
extern int cs_Errno;
extern int csErrlng;
extern int csErrlat;
extern unsigned short cs_ErrSup;
#if _RUN_TIME <= _rt_UNIXPCC
extern ulong32_t cs_Doserr;
#endif

int CStest7 (int verbose,int crypt)
{
	extern struct cs_Grptbl_ cs_CsGrptbl [];

	int cnt;
	int f_cnt;
	int cs_cnt;
	int grp_cnt;
	int err_cnt;
	int my_crypt;

	csFILE *csStrm;
	struct cs_Grptbl_ *tp;
	struct cs_Csgrplst_ *gp;
	struct cs_Csgrplst_ *grp_list;

	__ALIGNMENT__1			/* Required by some Sun compilers. */
	struct cs_Csdef_ cs_def;

	err_cnt = 0;
	printf ("Testing Coordinate System Group functions.\n");

	/* Count the number of entries in the Coordinate System
	   Dictionary. */

	cs_cnt = 0;
	csStrm = CS_csopn (_STRM_BINRD);
	while (CS_csrd (csStrm,&cs_def,&my_crypt))
	{
		cs_cnt += 1;

    	/* Verify that the group name is one that we know about. */
    	for (tp = cs_CsGrptbl;tp->group [0] != '\0';tp += 1)
    	{
    		if (!CS_stricmp (tp->group,cs_def.group)) break;
    	}
    	if (tp->group [0] == '\0')
    	{
    	    printf ("Coordinate system %s has unknown group name [%s]\n",cs_def.key_nm,cs_def.group);   
			err_cnt += 1;
    	}
	}	
	CS_csDictCls (csStrm);

	/* Loop through the group table and fetch a linked list
	   for each group.  Count the number in the group and
	   add to the total in the group. */

	grp_cnt = 0;
	for (tp = cs_CsGrptbl;tp->group [0] != '\0';tp += 1)
	{
		f_cnt = CS_csgrp (tp->group,&grp_list);
		cnt = 0;
		for (gp = grp_list;gp != NULL;gp = gp->next)
		{
			cnt += 1;
		}
		CS_csgrpf (grp_list);
		if (cnt != f_cnt)
		{
			printf ("CS_csgrp returned a bogus count/list for group %s\n",tp->group);
			err_cnt += 1;
		}
		if (verbose)
		{
			printf ("%d Coordinate Systems in %s group.\n",cnt,tp->group);
		}
		grp_cnt += cnt;
	}

	/* Grp_cnt should now equal cs_cnt. */

	if (cs_cnt != grp_cnt)
	{
		printf ("Group function failed to return all appropriate coordinate systems.\n");
		err_cnt += 1;
	}
	return (err_cnt);
}
