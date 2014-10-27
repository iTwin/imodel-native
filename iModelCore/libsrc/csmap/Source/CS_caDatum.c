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

/*		       * * * * R E M A R K S * * * *
	The US datum shift requires two grid files; thus a US datum
	shift object was required.  These objects are very simple
	and exist largely to be consistent with the US
	implementation.  Who knows, maybe some day there will be
	a need to do something special at this level.
*/

#include "cs_map.h"

/******************************************************************************
Constructor */
struct csDatumShiftCa2_* CSnewDatumShiftCa2 (enum csDtCvtType type,Const char *path,long32_t bufferSize,ulong32_t flags,double density)
{
	struct csDatumShiftCa2_ *__This = NULL;

	__This = (struct csDatumShiftCa2_ *)CS_malc (sizeof (struct csDatumShiftCa2_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->gridPtr = NULL;

	__This->gridPtr = CSnewGridFileCa2 (type,path,bufferSize,flags,density);
	if (__This->gridPtr == NULL) goto error;
	return __This;
error:
	if (__This != NULL)
	{
		if (__This->gridPtr != NULL) CSdeleteGridFileCa2 (__This->gridPtr);
		CS_free (__This);
	}
	return NULL;
}
/* Destructor */
void CSdeleteDatumShiftCa2 (struct csDatumShiftCa2_* __This)
{
	if (__This != NULL)
	{
		if (__This->gridPtr != NULL) CSdeleteGridFileCa2 (__This->gridPtr);
		CS_free (__This);
	}
}
void CSreleaseDatumShiftCa2 (struct csDatumShiftCa2_* __This)
{
	if (__This != NULL)
	{
		if (__This->gridPtr != NULL) CSreleaseGridFileCa2 (__This->gridPtr);
	}
}
double CStestDatumShiftCa2 (struct csDatumShiftCa2_* __This,Const double *coord)
{
	if (__This->gridPtr == NULL) return 0.0;
	return CStestGridFileCa2 (__This->gridPtr,coord);
}
int CScalcDatumShiftCa2 (struct csDatumShiftCa2_* __This,double* result,Const double* source,struct csLLGridCellCache_ *cachePtr)
{
	extern double cs_Sec2Deg;		/* 1/3600 */

	int status;
	double deltaLL [2];

	/* Here we simply shift the longitude using lngShift, and the latitude
	   using latShift. */
	status = CScalcGridFileCa2 (__This->gridPtr,deltaLL,source);
	if (status == 0)
	{
		result [LNG] = source [LNG] - deltaLL [LNG] * cs_Sec2Deg;
		result [LAT] = source [LAT] + deltaLL [LAT] * cs_Sec2Deg;
		if (cachePtr != NULL && __This->gridPtr->CellIsValid)
		{
			CSaddLLGridCellCache (cachePtr,dtcTypeCanadian2,&__This->gridPtr->longitudeCell,
															&__This->gridPtr->latitudeCell);
		}
	}
	else if (status > 0)
	{
		result [LNG] = source [LNG];
		result [LAT] = source [LAT];
	}
	return status;
}
Const char *CSsourceDatumShiftCa2 (struct csDatumShiftCa2_* __This,Const double llSource [2])
{
	Const char *cp;

	cp = CSsourceGridFileCa2 (__This->gridPtr,llSource);	
	return cp;
}

/******************************************************************************
Constructor */
struct csDatumShiftCa1_* CSnewDatumShiftCa1 (Const char *path,long32_t bufferSize,ulong32_t flags,double density)
{
	struct csDatumShiftCa1_ *__This = NULL;

	__This = (struct csDatumShiftCa1_ *)CS_malc (sizeof (struct csDatumShiftCa1_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->gridPtr = NULL;

	__This->gridPtr = CSnewGridFileCa1 (path,bufferSize,flags,density);
	if (__This->gridPtr == NULL) goto error;
	return __This;
error:
	if (__This != NULL)
	{
		if (__This->gridPtr != NULL) CSdeleteGridFileCa1 (__This->gridPtr);
		CS_free (__This);
	}
	return NULL;
}
/* Destructor */
void CSdeleteDatumShiftCa1 (struct csDatumShiftCa1_* __This)
{
	if (__This != NULL)
	{
		if (__This->gridPtr != NULL) CSdeleteGridFileCa1 (__This->gridPtr);
		CS_free (__This);
	}
}
void CSreleaseDatumShiftCa1 (struct csDatumShiftCa1_* __This)
{
	if (__This != NULL)
	{
		if (__This->gridPtr != NULL) CSreleaseGridFileCa1 (__This->gridPtr);
	}
}
double CStestDatumShiftCa1 (struct csDatumShiftCa1_* __This,Const double *coord)
{
	if (__This->gridPtr == NULL) return 0.0;
	return CStestGridFileCa1 (__This->gridPtr,coord);
}
int CScalcDatumShiftCa1 (struct csDatumShiftCa1_* __This,double* result,Const double* source,struct csLLGridCellCache_ *cachePtr)
{
	extern double cs_Sec2Deg;		/* 1/3600 */

	int status;
	double deltaLL [2];

	/* Here we simply shift the longitude using lngShift, and the latitude
	   using latShift. */
	status = CScalcGridFileCa1 (__This->gridPtr,deltaLL,source);
	if (status == 0)
	{
		result [LNG] = source [LNG] - deltaLL [LNG] * cs_Sec2Deg;
		result [LAT] = source [LAT] + deltaLL [LAT] * cs_Sec2Deg;
		if (cachePtr != NULL && __This->gridPtr->CellIsValid)
		{
			CSaddLLGridCellCache (cachePtr,dtcTypeCanadian1,&__This->gridPtr->longitudeCell,
															&__This->gridPtr->latitudeCell);
		}
	}
	else if (status > 0)
	{
		result [LNG] = source [LNG];
		result [LAT] = source [LAT];
	}
	return status;
}
Const char *CSsourceDatumShiftCa1 (struct csDatumShiftCa1_* __This,Const double* source)
{
	Const char *cp;

	cp = CSsourceGridFileCa1 (__This->gridPtr,source);
	return cp;
}

int CSinverseDatumShiftCa2 (struct csDatumShiftCa2_* __This,double* trgLl,Const double* srcLl)
{
	static int maxIteration = 8;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the source lat/longs to the local array.  This is the
	   default result which the user may want in the event of a fatal error.
	   Note, we assume such has been done below, even if there has not been
	   an error. */
	trgLl [LNG] = guess [LNG] = srcLl [LNG];
	trgLl [LAT] = guess [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the target lat/long for our current guess. */
		rtnVal = CScalcDatumShiftCa2 (__This,newLl,guess,NULL);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_NADCON_ICNT);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* If we are not returning fatal error status, we return the last
	   computed value even if we are returning a warning status.  It may be
	   of value, often is. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return rtnVal;
}
