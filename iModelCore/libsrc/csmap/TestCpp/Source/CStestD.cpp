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
	extern struct cs_Grptbl_ cs_CsGrptbl [];

	#if _RUN_TIME <= _rt_UNIXPCC
	extern ulong32_t cs_Doserr;
	#endif
}

extern int cs_MeFlag;
extern char cs_MeKynm [];
extern char cs_MeFunc [];
extern double cs_Coords [];

/*
	The following test exercises each coordinate system
	in the COORDSYS file, and exercises each assuring
	that coordinates within the useful range of each
	projection are reversible within rather liberal
	limits.
*/

int CStestD (bool verbose,long32_t duration)
{
	int status;
	int err_cnt;
	int isPseudo;

	long32_t il;

	struct cs_Grptbl_ *tp;
	struct cs_Csgrplst_ *gp;
	struct cs_Csgrplst_ *grp_list;
	struct cs_Csprm_ *csprm;

	double ll  [3];
	double xy  [3];
	double ll1 [3];

	double low_lng;
	double low_lat;
	double hi_lng;
	double hi_lat;
	double ll_tol;
	double del_lng;
	double del_lat;

	duration *= 100L;

	printf ("Checking reversibility of all projections, large region/tolerance.\n");

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
				printf ("Testing %s for reversibility.\n",
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
				continue;
			}
			strcpy (cs_MeKynm,csprm->csdef.key_nm);

			isPseudo = FALSE;
			if (csprm->prj_code == cs_PRJCOD_MRCATPV)
			{
				isPseudo = TRUE;
			}

			low_lng = csprm->cent_mer + csprm->min_ll [LNG];
			hi_lng =  csprm->cent_mer + csprm->max_ll [LNG];
			low_lat = csprm->min_ll [LAT];
			hi_lat  = csprm->max_ll [LAT];
			for (il = 0;il < duration;il += 1)
			{
				ll [LNG] = CStestRN (low_lng,hi_lng);
				ll [LNG] = CS_adj180 (ll [LNG]);
				ll [LAT] = CStestRN (low_lat,hi_lat);

				/* The pseudo Mercator cannot convert any point with
				   latitude higher than 84. */
				if (isPseudo && fabs (ll [LAT]) > 83.0)
				{
					continue;
				}

				/* We set the tolerance to what equals about one
				   a meter for this test.  A smaller tolerance is
				   possible, but only if we tighten up on the deviations
				   from the origin of the projection.  That is done, now,
				   int test E. */
				ll_tol = 1.0E-05;

				/* Convert to X and Y, then back again. */

				strcpy (cs_MeFunc,"CS_ll2cs");
				status = CS_ll2cs (csprm,xy,ll);
				if (status != cs_CNVRT_NRML) continue;
				strcpy (cs_MeFunc,"CS_cs2ll");
				status = CS_cs2ll (csprm,ll1,xy);
				if (status != cs_CNVRT_NRML) continue;

				/* Conversions were OK, see if we
				   ended up with what we started
				   with. */

				del_lng = CS_adj180 (ll [0] - ll1 [0]);
				del_lat = ll [1] - ll1 [1];
				if (fabs (del_lng) > ll_tol ||
					fabs (del_lat) > ll_tol)
				{
					printf ("%s:: Couldn't reverse %f:%f.\n",csprm->csdef.key_nm,ll[0],ll[1]);
					printf ("\t\tdel_lng = %g,del_lat = %g.\n",del_lng,del_lat);
					err_cnt += 1;

					/* Uncomment the following for
					   easier debugging.

					status = CS_ll2cs (csprm,xy,ll);
				    status = CS_cs2ll (csprm,ll1,xy);
					ll1 [0] = CS_adj180 (ll1 [0]);
					*/
					break;
				}
			}

			CS_free (csprm);
			csprm = NULL;
		}
		CS_csgrpf (grp_list);
	}
	return (err_cnt);
}
