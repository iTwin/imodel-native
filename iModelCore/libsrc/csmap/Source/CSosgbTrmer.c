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

/* This module is coded to provide a rather simple function: return a poointer
   to an 'struct cs_Trmer_' object which has been initialized to convert
   coordinates from OSGB to LatLongs, and vice versa.  This is necessary in
   several instances as Ordnance Survey geodetic and geoid grids are mostly
   based on cartesian coordinates of the National Transformation Grid (rather
   than lat/longs as most others are).

   Requires one argument: pointer to an ellsipoid definition.  This can be
   either AIry1830 or GRS1980.

*/

#include "cs_map.h"

static struct cs_Trmer_ cs_Osgb1980;
static struct cs_Trmer_ cs_OsgbAiry1830;

void CSsetOsgbTmer (struct cs_Trmer_* osgbTrmer,short grs1980)
{
	extern double cs_Half;
	extern double cs_One;
	extern double cs_Two;
	extern double cs_Degree;

	struct cs_Trmer_* This = osgbTrmer;
	memset (This,'\0',sizeof (struct cs_Trmer_));

	double e_rad = grs1980 ? 6378137.0 : 6377563.396;
	double flatI  = grs1980 ? 298.257222101 : 299.3249646;
	double flat   = cs_One / flatI;
	double p_rad  = e_rad - flat * e_rad;
	double e_sq   = (e_rad * e_rad - p_rad * p_rad) / (e_rad * e_rad);
	double e_cent = sqrt (e_sq);

	This->kruger   = 0;						/* Use traditional formulation */
	This->cent_lng = -cs_Two * cs_Degree;
	This->org_lat  = 49.0 * cs_Degree;
	This->k        = 0.9996012717;
	This->k0       = 0.9996012717;
	This->x_off    = 400000.0;
	This->y_off    = -100000.0;
	This->ecent    = e_cent;
	This->e_sq     = e_sq;
	This->e_rad    = e_rad;
	This->Rk       = e_rad * This->k;
	This->Rk_ovr_2 = This->Rk * cs_Half;
	This->var_K    = cs_One;
	This->eprim_sq = e_sq / (cs_One - e_sq);
	This->xx_max   = 1500000.0;
	CSmmFsu (&This->mmcofF,e_rad,e_sq);
	CSmmIsu (&This->mmcofI,e_rad,e_sq);
	This->M0       = CSmmFcal (&This->mmcofF,This->org_lat,
											 sin (This->org_lat),
											 cos (This->org_lat));
	This->quad = 0;
}

int CSosgb2LL (short grs1980,double ll [2],Const double xy [2])
{
	extern double cs_One;

	int st;

	struct cs_Trmer_ *This = grs1980 ? &cs_Osgb1980 : &cs_OsgbAiry1830;
	if (This->var_K != cs_One)
		CSsetOsgbTmer (This,grs1980);

	st = CStrmerI (This,ll,xy);
	return st;
}
int CSosgb2XY (short grs1980,double xy [2],Const double ll [2])
{
	extern double cs_One;

	int st;

	struct cs_Trmer_ *This = grs1980 ? &cs_Osgb1980 : &cs_OsgbAiry1830;
	if (This->var_K != cs_One)
		CSsetOsgbTmer (This,grs1980);

	st = CStrmerF (This,xy,ll);
	return st;
}

