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

double CStestRN (double low, double high);
double CStstsclk (struct cs_Csprm_ *csprm,double ll [2]);
double CStstsclh (struct cs_Csprm_ *csprm,double ll [2]);
double CStstcnvrg (struct cs_Csprm_ *csprm,double ll [2]);

extern int cs_Error;
extern int cs_Errno;
extern int csErrlng;
extern int csErrlat;
extern unsigned short cs_ErrSup;
#if _RUN_TIME <= _rt_UNIXPCC
extern ulong32_t cs_Doserr;
#endif

int CStestB (int verbose,long32_t duration)
{
	extern struct cs_Grptbl_ cs_CsGrptbl [];
	extern double cs_Mone;   /* -1 */
	extern double cs_Km360;  /* -360 */
	extern double cs_SclInf; /* 9.9E+04 */

	int brk_flg;
	int err_cnt;
	int itr_cnt;

	long32_t il;

	struct cs_Grptbl_ *tp;
	struct cs_Csgrplst_ *gp;
	struct cs_Csgrplst_ *grp_list;
	struct cs_Csprm_ *csprm;

	double del_lat;
	double tolerance;
	double test_val, csmap_val;

	double low_lat, hi_lat;
	double low_lng, hi_lng;
	double ll [2];

	printf ("Checking scale and convergence for non-azimuthal projections.\n");

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
		CS_csgrp (tp->group,&grp_list);
		for (gp = grp_list;gp != NULL;gp = gp->next)
		{
			if (csprm != NULL)
			{
				CS_free (csprm);
				csprm = NULL;
			}

			/* Activate this coordinate system. */

			if (verbose)
			{
				printf ("Checking scale and convergence for %s.\n",
							gp->key_nm);
			}
			csprm = CS_csloc (gp->key_nm);
			if (csprm == NULL)
			{
				printf ("Couldn't setup coordinate system named %s.\n",gp->key_nm);
				err_cnt += 1;
				continue;
			}

			/* Skip over the LL projections.  They make no sense
			   here. */

			if (csprm->prj_code == cs_PRJCOD_UNITY)
			{
				continue;
			}

			/* We are testing the validity of the values here.
			   We test robustness elsewhere.

			   Generate a random coordinate now. */

			low_lng = csprm->cent_mer + csprm->min_ll [LNG] * 0.75;
			hi_lng =  csprm->cent_mer + csprm->max_ll [LNG] * 0.75;
			del_lat = csprm->max_ll [LAT] - csprm->min_ll [LAT];
			low_lat = csprm->min_ll [LAT] + 0.1 * del_lat;
			hi_lat  = csprm->max_ll [LAT] - 0.1 * del_lat;

			brk_flg = FALSE;
			for (il = 0;!brk_flg && il < duration;il += 1)
			{
				ll [LNG] = CStestRN (low_lng,hi_lng);
				ll [LNG] = CS_adj180 (ll [LNG]);
				itr_cnt = 0;
				do
				{
					if (itr_cnt++ > 5) break;
					ll [LAT] = CStestRN (low_lat,hi_lat);
				} while	(fabs (ll [LAT]) > 85.0);

				/* For Azimuthal projections, K and H
				   scale factors are often special, e.g.
				   normal to the radial from the origin
				   of the projection. In these cases,
				   the test functions in this module
				   are invalid.  Therefore, we have this
				   little kludge to skip the K and
				   H scale factor tests for certain
				   type projections.
				   
				   Similar considerations apply to the
				   Bipolar Oblique Conic. K and H are
				   relative to the oblique poles. Therefore,
				   our test functions will not generate
				   appropriate values. */

				if ((csprm->prj_flags & cs_PRJFLG_AZMTH) != 0 ||
					 csprm->prj_code == cs_PRJCOD_BPCNC)
				{
					goto skip_KH;
				}

				/* The special projection variations for WI
				   and MN won't work either as these systems
				   are elevated above the ellipsoid.

				   OSTN02, OSTN97, NBZLND and both Danish
				   systems include heavy duty non-analytical (i.e.
				   polynomials and/or grid file) techniques,
				   thus we have no way of generating a
				   reliable test value.
				   
				   NERTH has no concept of grid scale or convergence,
				   so we skip those as well.
				    */

				if (csprm->prj_code == cs_PRJCOD_WCCST ||
					csprm->prj_code == cs_PRJCOD_WCCSL ||
					csprm->prj_code == cs_PRJCOD_MNDOTL ||
					csprm->prj_code == cs_PRJCOD_MNDOTT ||
					csprm->prj_code == cs_PRJCOD_OSTN97 ||
					csprm->prj_code == cs_PRJCOD_OSTN02 ||
					csprm->prj_code == cs_PRJCOD_NZLND ||
					csprm->prj_code == cs_PRJCOD_SYS34 ||
					csprm->prj_code == cs_PRJCOD_SYS34_99 ||
					csprm->prj_code == cs_PRJCOD_NERTH ||
					csprm->prj_code == cs_PRJCOD_NRTHSRT
				   )
				{
					goto skip_KH;
				}

				/* The Krovak projection uses a strange radius as the base for
				   the calculations.  That is, rho0 (the radius of the standard
				   parallel) is calculated on the ellipsoid, but the actual
				   coordinate system is based on a sphere of a different
				   radius.  Thus, the test value generator, does not generate
				   appropriate values.  So, for now, we skip this test on
				   the Krovak. */

				if (csprm->prj_code == cs_PRJCOD_KROVAK || csprm->prj_code == cs_PRJCOD_KRVK95)
				{
					goto skip_KH;
				}

				/* Since we will be calculating our own
				   value, we can't use CS_csscl.
				   
				   Note, we could tighten up the tolerances, but
				   only if we limit the geographic extents of the
				   tests.  The values used are a compromise. */

				csmap_val = CS_cssck (csprm,ll);
				test_val  = CStstsclk (csprm,ll);
				if (test_val > cs_Mone && test_val < cs_SclInf)
				{
					tolerance = 0.00005 * fabs (test_val - 1.0);
					if (tolerance < 5.0E-06) tolerance = 5.0E-06;
					if (fabs (csmap_val - test_val) > tolerance)
					{
						printf ("K scale error, %s: lng = %11.6f, lat = %10.6f\n",
										csprm->csdef.key_nm,
										ll [LNG],
										ll [LAT]);
						err_cnt += 1;
						brk_flg = TRUE;

						/* Uncomment the following for
						   convenient debugging.
						*/

						csmap_val = CS_cssck (csprm,ll);
						test_val  = CStstsclk (csprm,ll);
					}
				}

				csmap_val = CS_cssch (csprm,ll);
				test_val  = CStstsclh (csprm,ll);
				if (test_val < cs_SclInf && test_val > cs_Mone)
				{
					tolerance = 0.00005 * fabs (test_val - 1.0);
					if (tolerance < 5.0E-06) tolerance = 5.0E-06;

					if (fabs (csmap_val - test_val) > tolerance)
					{
						printf ("H scale error, %s: lng = %11.6f, lat = %10.6f\n",
										csprm->csdef.key_nm,
										ll [LNG],
										ll [LAT]);
						err_cnt += 1;
						brk_flg = TRUE;

						/* Uncomment the following for
						   convenient debugging. */

						csmap_val = CS_cssch (csprm,ll);
						test_val  = CStstsclh (csprm,ll);
						/**/
					}
				}

skip_KH:
				/* We skip the coordinate systems which are
				   not rectified, i.e. Y is not necessarily
				   north. */

				if (csprm->prj_code == cs_PRJCOD_HOM1UV ||
					csprm->prj_code == cs_PRJCOD_HOM2UV)
				{
					continue;
				}

				csmap_val = CS_cscnv (csprm,ll);
				test_val  = CStstcnvrg (csprm,ll);
				if (test_val <= cs_Km360)
				{
					continue;
				}

				/* For the Transverse Mercator and similar projections, the
				   accuracy of the standard convergence angle calculation
				   deteriotates	rapidly as one moves away from the central
				   meridian.  So, we have this rather obnoxious kludge for
				   these projections.  Note, this value is in degrees,
				   therefore, a tolerance of 0.001 is pretty tight. */

				tolerance = 0.00001 * fabs (test_val);
				if (tolerance < 0.00001) tolerance = 0.00001;
				if (fabs (csmap_val - test_val) > tolerance)
				{
					printf ("Convergence error, %s: lng = %11.6f, lat = %10.6f\n",
									csprm->csdef.key_nm,
									ll [LNG],
									ll [LAT]);
					err_cnt += 1;
					brk_flg = TRUE;

					/* Uncomment the following for
					   convenient debugging.
					*/

					csmap_val = CS_cscnv (csprm,ll);
					test_val  = CStstcnvrg (csprm,ll);
				}
			}
			CS_free (csprm);
			csprm = NULL;
		}
		CS_csgrpf (grp_list);
	}
	return (err_cnt);
}
