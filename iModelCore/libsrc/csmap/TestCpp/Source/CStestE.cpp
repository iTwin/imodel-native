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
	The following test is a duplicate of the previous test.  However, in this
	case we restrict test coordinates to be more firmly rooted to the useful
	area of the projection, and therefore, can increase the tolerance level
	by an order of magnitude.
	
	Note that we have a list of coordinate systems which are real stretches
	of the useful range of the projection.  For these specifically coded
	coordinate systems, we reduce the range of testing even further.
*/

const char KcsTestEspecialList [][cs_KEYNM_DEF] =
{
//	"\0",						// Uncomment this line to temporarily turn this feature off.
	"SWEREF-99-TM",				// Used for all of Sweden which extends from 8 to 24 degrees
								// of east longitude, way beyond what one would normally use
								// the Transverse Mercator for.
	"\0"
};														


extern "C" double cs_Half;
extern "C" double cs_One;

int CStestE (bool verbose,long32_t duration)
{
	bool special;

	int index;
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

	double lat_del;
	double low_lng;
	double low_lat;
	double hi_lng;
	double hi_lat;
	double ll_tol;
	double del_lng;
	double del_lat;

	duration *= 100L;

	printf ("Checking reversibility of all projections, smaller region/tolerance.\n");

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

			/* Is this one on the "special" list? */
			special = false;
			for (index = 0;KcsTestEspecialList [index][0] != '\0';index += 1)
			{
				if (!CS_stricmp (csprm->csdef.key_nm,KcsTestEspecialList [index]))
				{
					special = true;
					break;
				}
			}

			isPseudo = FALSE;
			if (csprm->prj_code == cs_PRJCOD_MRCATPV)
			{
				isPseudo = TRUE;
			}

			/* Get the useful range of the coordinate system and reduce it
			   by about one half. */
			if (special)
			{
				/* here to reduce by two thirds. */
				low_lng = csprm->cent_mer + csprm->min_ll [LNG] + fabs (csprm->min_ll [LNG] * 0.6666);
				hi_lng =  csprm->cent_mer + csprm->max_ll [LNG] - fabs (csprm->min_ll [LNG] * 0.6666);
				lat_del = csprm->max_ll [LAT] - csprm->min_ll [LAT]; 
				low_lat = csprm->min_ll [LAT] + lat_del * 0.333333;
				hi_lat  = csprm->max_ll [LAT] - lat_del * 0.333333;
			}
			else
			{
				/* Code that has been used for testE for many years before the "special fix", unedited. */
				low_lng = csprm->cent_mer + csprm->min_ll [LNG] + fabs (csprm->min_ll [LNG] * 0.5);
				hi_lng =  csprm->cent_mer + csprm->max_ll [LNG] - fabs (csprm->min_ll [LNG] * 0.5);
				lat_del = csprm->max_ll [LAT] - csprm->min_ll [LAT]; 
				low_lat = csprm->min_ll [LAT] + lat_del * 0.5;
				hi_lat  = csprm->max_ll [LAT] - lat_del * 0.5;
			}
			
			/* insure that there is a range of some sort. */
			if ((hi_lng - low_lng) < cs_One)
			{
				low_lng = csprm->cent_mer - cs_Half;
				hi_lng = csprm->cent_mer + cs_Half;
			}
			if ((hi_lat - low_lat) < cs_One)
			{
				low_lat = ((csprm->max_ll [LAT] + csprm->min_ll [LAT]) * cs_Half) - cs_Half;
				hi_lat = low_lat + cs_One;
			}

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

				/* For the two projections which account for 95% of
				   the mapping in the world, we use a very small
				   tolerance which approximates 5 millimeters on the
				   earth.  For the less important projections, we
				   use a tolerance which approximates 50 millimeters.

				   That is, for several projections, specifically the
				   whole world type, the mathemagics simply doesn't
				   produce the same level of accuracy as the two
				   biggies do.

				   As the Van der Grinten approaches the central
				   meridian, however, we need to relax the tolerance
				   to account for the mathematical discontinuity
				   encountered there. */

				if (csprm->prj_code == cs_PRJCOD_TRMER ||
				    csprm->prj_code == cs_PRJCOD_LM2SP)
				{
					ll_tol = 4.5E-08;
				}
				else if (csprm->prj_code == cs_PRJCOD_CSINI)
				{
					/* Cassini doesn't reverse very well.  This is well known
					   in the coordinate conversion community.  Someone needs
					   to figure out the math involved and improve the
					   projection formulas. */
					ll_tol = 2.5E-06;
				}
				else
				{
					ll_tol = 4.5E-07;
				}
				if (csprm->prj_code == cs_PRJCOD_VDGRN)
				{
					if (fabs (ll [0] - csprm->cent_mer) < 0.1) ll_tol = 1.0E-05;
				}

				/* Due to the polynomials, the Danish stuff does not invert well
				   when you're not within the real domain of the polynomial; i.e.
				   actually in the region being converted.  Thus, randomly generated
				   numbers like this do not invert well.
				   
				   The polynomial code uses the inverse to determine if a coordinate
				   pair is within the useful range of the polynomial.  The tolerance
				   check is set to 4 centimeters.  Adjusting for the latitude of
				   Denmark, we use the following tolerance. */
				if (csprm->prj_code == cs_PRJCOD_SYS34    ||
				    csprm->prj_code == cs_PRJCOD_SYS34_99 ||
				    csprm->prj_code == cs_PRJCOD_SYS34_01)
				{
					ll_tol = 7.0E-06;
				}

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
					   easier debugging. */

					status = CS_ll2cs (csprm,xy,ll);
					status = CS_cs2ll (csprm,ll1,xy);
				}
			}

			CS_free (csprm);
			csprm = NULL;
		}
		CS_csgrpf (grp_list);
	}
	return (err_cnt);
}
