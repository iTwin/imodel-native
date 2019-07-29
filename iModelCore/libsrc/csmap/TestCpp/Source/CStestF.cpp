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
}

#define XY_BASE (cs_ATOF_COMMA)
#define DMS_BASE (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | cs_ATOF_XEAST)
struct _fTable
{
	long32_t format;
	double tol;
	double min;
	double max;
};
double __dummy_21;			/* Required by some Sun compilers. */
struct _fTable fTable [] =
{
	{cs_ATOF_XXXDFLT,  0.01, -1E+08, 1E+08 },
	{cs_ATOF_YYYDFLT,  0.01, -1E+08, 1E+08 },
	{XY_BASE | 6,     1E-05, -1E+08, 1E+08 },
	{XY_BASE | 8,     1E-07, -1E+08, 1E+08 },
	{cs_ATOF_LNGDFLT, 3E-06, -180.0, 180.0 },
	{cs_ATOF_LATDFLT, 3E-06,  -90.0,  90.0 },
	{DMS_BASE | 6,    3E-09, -180.0, 180.0 }
};

int CStestF (bool verbose,long32_t duration)
{
	int err_cnt;
	int idx;
	int prec;
	
	size_t localeSize;
	long32_t il;
	
	char* localeIdPtr;
	char* localePtr;

	double dbl1, dbl2, tol;

	char bufr [32];

	localePtr = 0;
	duration *= 2000L;
	if (verbose)
	{
		printf ("Setting locale to \"C\".\n");
	}
	localeIdPtr = setlocale (LC_ALL,NULL);
	if (localeIdPtr != 0)
	{
		localeSize = strlen (localeIdPtr) + 1;
		localePtr = (char*)malloc (localeSize);
		if (localePtr != 0)
		{
			CS_stncp (localePtr,localeIdPtr,(int)localeSize);
		}
	}
	setlocale (LC_ALL,"C");

	printf ("Using sprintf to check CS_atof (no DMS).\n");
	err_cnt = 0;
	for (il = 0;il < duration && err_cnt == 0;il += 1)
	{
		dbl1 = CStestRN (-1E+12,1E+12);
		if (fabs (dbl1) < 1E-23) continue;
		sprintf	(bufr,"%28.14f",dbl1);
		CS_atof (&dbl2,bufr);
		prec = (int)log10 (fabs (dbl1));
		prec = (int)log10 (fabs (dbl1)) - 14;
		tol = pow (10.0,(double)prec); 
		if (fabs (dbl1 -dbl2) > tol)
		{
			printf ("CS_atof produced %f from %s.\n",dbl2,bufr);
			err_cnt += 1;
		}
	}
	printf ("Using CS_atof to check CS_ftoa.\n");
	for (il = 0;il < duration && err_cnt < 4;il += 1)
	{
		idx = il % 7;
		dbl1 = CStestRN (fTable [idx].min,fTable [idx].max);
		CS_ftoa (bufr,sizeof (bufr),dbl1,fTable [idx].format);
		CS_atof (&dbl2,bufr);
		if (fabs (dbl1 - dbl2) > fTable [idx].tol)
		{
	    	printf ("CS_atof/ftoa failed: %f -> %s ->%f\n",dbl1,bufr,dbl2);
			err_cnt += 1;
		}
	}
	if (localePtr != 0)
	{
		if (verbose)
		{
			printf ("Resetting locale to \"%s\".\n",localePtr);
		}
		setlocale (LC_ALL,localePtr);
		free (localePtr);
	}
	return (err_cnt);
}
