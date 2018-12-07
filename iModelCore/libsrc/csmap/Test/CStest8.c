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

int CStest8 (int verbose,int crypt)
{
    extern char cs_MeKynm [128];
    extern char cs_MeFunc [128];
    extern double cs_Coords [3];

	extern struct cs_Grptbl_ cs_CsGrptbl [];
	extern int cs_Error;

	int err_cnt;

	struct cs_Grptbl_ *tp;
	struct cs_Csgrplst_ *gp;
	struct cs_Csgrplst_ *grp_list;
	struct cs_Csprm_ *csprm;

	double xy [3];
	double ll [3];

	printf ("Exercising each coordinate system definition at least once.\n");

	/* Loop through the group table and fetch a linked list
	   for each group.  Count the number in the group and
	   add to the total in the group. */

	err_cnt = 0;
	for (tp = cs_CsGrptbl;tp->group [0] != 0;tp += 1)
	{
		if (!CS_stricmp (tp->group,"LEGACY"))
		{
			continue;
		}
		CS_csgrp (tp->group,&grp_list);
		for (gp = grp_list;gp != NULL;gp = gp->next)
		{
			/* Activate this coordinate system.  This
			   ensures that every coordinate system
			   is "set up" at least once in all of this. */

			if (verbose)
			{
				printf ("Exercising coordinate system named %s.\n",
							gp->key_nm);
			}
			csprm = CS_csloc (gp->key_nm);
			if (csprm == NULL)
			{
				printf ("Couldn't setup coordinate system named %s.\n",gp->key_nm);
				err_cnt += 1;
			}
			else
			{
			    /* Set information in case there is a floating point exception. */
			    CS_stncp (cs_MeKynm,gp->key_nm,sizeof (cs_MeKynm));
			    cs_Coords [0] = csprm->csdef.x_off;
			    cs_Coords [1] = csprm->csdef.y_off;
			    cs_Coords [2] = 0.0;
   
				/* Convert a coordinate, the false origin should
				   be a safe one. */

				xy [0] = csprm->csdef.x_off;
				xy [1] = csprm->csdef.y_off;
				xy [2] = 0.0;
				cs_Error = 0;
				CS_stncp (cs_MeFunc,"CS_cs2ll",sizeof (cs_MeFunc));
				CS_cs2ll (csprm,ll,xy);
				CS_stncp (cs_MeFunc,"CS_ll2cs",sizeof (cs_MeFunc));
				CS_ll2cs (csprm,xy,ll);
				CS_stncp (cs_MeFunc,"CS_csscl",sizeof (cs_MeFunc));
				CS_csscl (csprm,ll);
				CS_stncp (cs_MeFunc,"CS_cscnv",sizeof (cs_MeFunc));
				CS_cscnv (csprm,ll);
				if (cs_Error != 0)
				{
					printf ("Exercise of coordinate system %s failed.\n",
								csprm->csdef.key_nm);
					err_cnt += 1;
				}
				CS_free (csprm);
			}
		}
		CS_csgrpf (grp_list);
	}
    cs_MeKynm [0] = '\0';
    cs_MeFunc [0] = '\0';
    cs_Coords [0] = 0.0;
    cs_Coords [1] = 0.0;
    cs_Coords [2] = 0.0;
	return (err_cnt);
}
