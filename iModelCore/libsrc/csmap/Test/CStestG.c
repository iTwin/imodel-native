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

/*
	Test G is the creep test. We start with a cartesian coordinate.
	We then repeatedly convert from cartesian to geographic and
	back to cartesian; about 1,000 times. We then measure the
	difference between what we started with and what we ended
	up with. Our tolerance here is 1 centimeter per 1000 conversions.
*/

struct _gTable
{
	char cs_name [32];
	char prjLabel [64];
	double point [2];
	double tol;
};
__ALIGNMENT__14				/* Required by some Sun compilers. */
struct _gTable gTable [] =
{
	{             "UTM27-13",      "Transverse Mercator", { 400000.0, 5000000.0},   5.000},
	{                 "CO-C",  "Lambert Conformal Conic", { 1500000.0, 250000.0},   5.000},
	{                 "AK-1",  "Hotine Oblique Mercator", { 2615716.0,1156768.0},   5.000},
	{                 "US48",       "Alber's Equal Area", { 1000000.0,2000000.0},   5.000},
	{                 "TRUK",     "Lambert Equi-Distant", {   75000.0,  60000.0},   5.000},
	{           "ArPoly-27F",       "American Polyconic", {  100000.0, 100000.0},   5.000},
	{           "CaspianSea",                 "Mercator", {  100500.0,  95000.0},   5.000},
	{"NTF.Lambert-1-ClrkIGN", "Lambert Tangential Conic", { 1029705.0, 272723.0},   5.000},
	{                     "",                         "", {       0.0,      0.0},   0.000}
};

int CStestG (int verbose,long32_t duration)
{
	int err_cnt;
	int idx;
	int ii;

	struct cs_Csprm_ *csPtr;

	double del_xx;
	double del_yy;
	double creep;

	double start [2];
	double xy [3];
	double ll [3];
	
	printf ("Performing creep test.\n");
	err_cnt = 0;
	for (idx = 0;gTable [idx].cs_name [0] != '\0' && err_cnt < 4;idx += 1)
	{
		start [0] = gTable [idx].point [0];
		start [1] = gTable [idx].point [1];
		xy [0] = start [0];
		xy [1] = start [1];
		csPtr = CS_csloc (gTable [idx].cs_name);
		if (csPtr == NULL)
		{
			return (1);
		}
		for (ii = 0;ii < 1000;ii += 1)
		{
			CS_cs2ll (csPtr,ll,xy);
			CS_ll2cs (csPtr,xy,ll);
		}
		del_xx = xy [0] - start [0];
		del_yy = xy [1] - start [1];
		creep = sqrt (del_xx * del_xx + del_yy * del_yy) * csPtr->csdef.unit_scl * 1000.0;
		if (creep > gTable [idx].tol)
		{
			printf ("Creep test on %s failed, (%f mm/1,000 conversions).\n",gTable [idx].cs_name,creep);
			err_cnt += 1;
		}
		else if (verbose)
		{
			printf ("%s creep is %f mm per %d conversions\n",gTable [idx].prjLabel,creep,ii);
		}
		CS_free (csPtr);
	}
	return (err_cnt);
}
