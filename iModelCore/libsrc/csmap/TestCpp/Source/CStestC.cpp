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

#include "csTestCpp.hpp"

extern "C"
{
	extern int cs_Error;
	extern int cs_Errno;
	extern int csErrlng;
	extern int csErrlat;
	extern unsigned short cs_ErrSup;

	#if _RUN_TIME <= _rt_UNIXPCC
	extern ulong32_t cs_Doserr;
	#endif

	extern struct cs_Grptbl_ cs_CsGrptbl [];
	extern double cs_Zero;
}

extern int cs_MeFlag;
extern char cs_MeKynm [];
extern char cs_MeFunc [];
extern double cs_Coords [3];

/*
	The following test exercises each coordinate system
	in the COORDSYS file, and exercises each attempting
	to produce a floating point exception.  We do this,
	using a random number generator, to throw all sorts
	of bogus numbers at each projection.
*/

int CStestC (bool verbose,long32_t duration)
{
	int err_cnt;

	unsigned sequencer;

	struct cs_Grptbl_ *tp;
	struct cs_Csgrplst_ *gp;
	struct cs_Csgrplst_ *grp_list;
	struct cs_Csprm_ *csprm;

	double ll [3];
	double xy [3];
	double falseOrg [2];
	double llAry [1][3];
	double xyAry [1][3];

	duration *= 10L;

	printf ("Trying very hard to produce a floating point exception.\n");

	/* Loop through the group table and fetch a linked list
	   for each group.  Count the number in the group and
	   add to the total in the group. */

	err_cnt = 0;
	csprm = NULL;
	for (tp = cs_CsGrptbl;tp->group [0] != 0;tp += 1)
	{
		if (csprm != NULL)
		{
			CS_free (csprm);
			csprm = NULL;
		}
		if (!CS_stricmp (tp->group,"LEGACY"))
		{
			continue;
		}
		CS_csgrp (tp->group,&grp_list);
		for (gp = grp_list;gp != NULL;gp = gp->next)
		{
			if (csprm != NULL)
			{
				CS_free (csprm);
				csprm = NULL;
			}
			if (verbose)
			{
				printf ("Trying to cause a floating exception using %s.\n",
							gp->key_nm);
			}
			else
			{
				printf ("%s               \r",gp->key_nm);
			}

			csprm = CS_csloc (gp->key_nm);
			if (csprm == NULL)
			{
				printf ("Couldn't setup coordinate system named %s.\n",gp->key_nm);
				err_cnt += 1;
				strcpy (cs_MeKynm,"???");
				continue;
			}
			cs_MeFlag = 0;
			strcpy (cs_MeKynm,csprm->csdef.key_nm);
			
			sequencer = 0;
			while (CStestLLH (ll,csprm->cent_mer,&sequencer) == 0)
			{
				cs_Coords [LNG] = ll [0];
				cs_Coords [LAT] = ll [1];
				cs_Coords [HGT] = ll [2];

				strcpy (cs_MeFunc,"CS_cssck");
				CS_cssck (csprm,ll);
				if (cs_MeFlag) break;
				strcpy (cs_MeFunc,"CS_cssch");
				CS_cssch (csprm,ll);
				if (cs_MeFlag) break;
				strcpy (cs_MeFunc,"CS_cscnv");
				CS_cscnv (csprm,ll);
				if (cs_MeFlag) break;
				strcpy (cs_MeFunc,"CS_ll2cs");
				CS_ll2cs (csprm,xy,ll);
				if (cs_MeFlag) break;

				llAry [0][0] = ll [0];		/* keep gcc compiler happy */
				llAry [0][1] = ll [1];
				llAry [0][2] = ll [2];
				strcpy (cs_MeFunc,"CS_llchk");
				CS_llchk (csprm,1,llAry);
				if (cs_MeFlag) break;
			}
			if (cs_MeFlag)
			{
				if (verbose)
				{
					printf ("\tLNG = %g, LAT = %g, HGT = %g     \n",ll [0],ll [1],ll [2]);
				}
				err_cnt += 1;
				continue;
			}

			sequencer = 0;
			falseOrg [0] = csprm->csdef.x_off;
			falseOrg [1] = csprm->csdef.y_off;
			while (CStestXYZ (xy,falseOrg,&sequencer) == 0)
			{
				cs_Coords [XX] = xy [XX];
				cs_Coords [YY] = xy [YY];
				cs_Coords [ZZ] = xy [ZZ];
				strcpy (cs_MeFunc,"CS_cs2ll");
				CS_cs2ll (csprm,ll,xy);
				if (cs_MeFlag) break;
				strcpy (cs_MeFunc,"CS_llchk");
				xyAry [0][0] = xy [0];		/* keep gcc compiler happy */
				xyAry [0][1] = xy [1];
				xyAry [0][2] = xy [2];
				CS_xychk (csprm,1,xyAry);
				if (cs_MeFlag) break;
			}
			if (cs_MeFlag)
			{
				if (verbose)
				{
					printf ("\tXX = %g, YY = %g, ZZ = %g          \n",xy [XX],xy [YY],xy [ZZ]);
				}
				err_cnt += 1;
				continue;
			}

			if (verbose)
			{
				printf ("                                                \r");
			}
			CS_free (csprm);
			csprm = NULL;
		}
		CS_csgrpf (grp_list);
	}
	printf ("                                 \r");
	return (err_cnt);
}
