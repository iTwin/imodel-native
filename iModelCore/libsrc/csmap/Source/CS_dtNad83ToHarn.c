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

	This object maintains a list of US datum shift objects
	enabling the conversion of geographic coordinates from NAD83
	to HARN (and vice versa).  Thus, NAD83/HARN can be considered
	a single object.
*/
/*lint -e722 */				/* suspicious use of semi-colon */

#include "cs_map.h"

/******************************************************************************
	Constructor
*/
struct csNad83ToHarn_* CSnewNad83ToHarn (Const char *catalog)
{
	int index;
	Const char *cp;
	struct csNad83ToHarn_ *__This;
	struct csDatumCatalog_ *catPtr;
	struct csDatumCatalogEntry_ *catEntryPtr;
	struct csNad83ToHarnEntry_* dtEntryPtr;
	struct csNad83ToHarnEntry_* findPtr;

	/* Prepare for an error. */
	__This = NULL;
	catPtr = NULL;
	catEntryPtr = NULL;
	dtEntryPtr = NULL;

	__This = (struct csNad83ToHarn_*) CS_malc (sizeof (struct csNad83ToHarn_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->fallback = NULL;
	__This->cachePtr = NULL;
	__This->listHead = NULL;

	/* Open the catalog file. */
	catPtr = CSnewDatumCatalog (catalog);
	if (catPtr == NULL) goto error;

	/* Activate the fallback if one was specified. */
	cp = CSgetFallbackName (catPtr);
	if (cp != NULL && *cp != '\0')
	{
		__This->fallback = CSnewFallback (cp,catalog);
		if (__This->fallback == NULL)
		{
			goto error;
		}
	}

	/* For each entry in the catalog, we build an appropriate datum
	   shift entry.  Right now, this is based on file names and
	   file extensions.  Not very good, but that's life. */

	index = 0;
	for (;;)
	{
		catEntryPtr = CSgetDatumCatalogEntry (catPtr,index++);
		if (catEntryPtr == NULL) break;
		dtEntryPtr = CSnewNad83ToHarnEntry (catEntryPtr);
		if (dtEntryPtr == NULL)
		{
			goto error;
		}
		/* Keep the list in the same order as they appear in the file. */
		if (__This->listHead == NULL)
		{
			__This->listHead = dtEntryPtr;
		}
		else
		{
			for (findPtr = __This->listHead;findPtr->next != NULL;findPtr = findPtr->next);
			findPtr->next = dtEntryPtr;
		}
	}
	CSdeleteDatumCatalog (catPtr);
	catPtr = NULL;

	/* Having done that successfully, allocate a grid cell cache.  If
	   this fails, we can either report it as an error, or just leave it
	   alone.  Lets report it as an error. */
	__This->cachePtr = CSnewLLGridCellCache (32);
	if (__This->cachePtr == NULL)
	{
		goto error;
	}

	/* It's ready to go. */
	return __This;

error:
	if (catPtr != NULL) CSdeleteDatumCatalog (catPtr);
	CSdeleteNad83ToHarn (__This);
	return NULL;
}

/******************************************************************************
	Destructor
*/
void CSdeleteNad83ToHarn (struct csNad83ToHarn_* __This)
{
	struct csNad83ToHarnEntry_ *dtEntryPtr;

	if (__This != NULL)
	{
		if (__This->fallback != NULL)
		{
			CSdeleteFallback (__This->fallback);
			__This->fallback = NULL;
		}
		if (__This->cachePtr != NULL)
		{
			CSdeleteLLGridCellCache (__This->cachePtr);
			__This->cachePtr = NULL;
		}
		while (__This->listHead != NULL)
		{
			dtEntryPtr = __This->listHead;
			__This->listHead = __This->listHead->next;
			CSdeleteNad83ToHarnEntry (dtEntryPtr);
		}
		CS_free (__This);
	}
	return;
}

/******************************************************************************
	Coverage Locator -- Some of this is overkill in the HARN case as all
	HARN files, currently, have the same density of 15 minutes per cell.
	Also, all HARN files overlap, but the results in all overlap areas
	are, by design, supposed to be the same.  Anyway, we maintain the
	design which enables selecting the specific HARN file with the
	lowest density.
*/
struct csNad83ToHarnEntry_* CSselectNad83ToHarn (struct csNad83ToHarn_* __This,Const double *ll83)
{
	double testValue;
	double bestSoFar;
	struct csNad83ToHarnEntry_* dtEntryPtr;
	struct csNad83ToHarnEntry_* rtnValue;

	rtnValue = NULL;
	bestSoFar = 3600000.00;
	dtEntryPtr = __This->listHead;
	while (dtEntryPtr != NULL)
	{
		testValue = CStestNad83ToHarnEntry (dtEntryPtr,ll83);
		if (testValue != 0.0 && testValue < bestSoFar)
		{
			bestSoFar = testValue;
			rtnValue = dtEntryPtr;
		}
		dtEntryPtr = dtEntryPtr->next;
	}
	return rtnValue;
}

/******************************************************************************
	Make First -- Used for performance.  Well, used to use it for performance.
	Replaced with the GridCellCache business.  We now want to keep these in
	the same order as they appeared in the data catalog file.
*/
void CSfirstNad83ToHarn (struct csNad83ToHarn_* __This,struct csNad83ToHarnEntry_* dtEntryPtr)
{
	struct csNad83ToHarnEntry_* curPtr;
	struct csNad83ToHarnEntry_* prvPtr;

	/* Take care of the already first situation very quickly. */
	if (dtEntryPtr == __This->listHead) return;

	/* Locate this guy on the list. */
	for (curPtr = __This->listHead,prvPtr = NULL;
		 curPtr != NULL;
		 prvPtr = curPtr,curPtr = curPtr->next)
	{
		if (curPtr == dtEntryPtr)
		{
			/* Make this guy the first one. */
			prvPtr->next = curPtr->next;						/*lint !e613 */
			curPtr->next = __This->listHead;
			__This->listHead = curPtr;
			break;
		}
	}
}

/******************************************************************************
	Calculate datum shift, the main man.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int CScalcNad83ToHarn (struct csNad83ToHarn_* __This,double* llHarn,Const double *ll83)
{
	int status;
	struct csNad83ToHarnEntry_* dtEntryPtr;

	status = 0;

	/* First see if using the cache works.  This works frequently. */
	if (__This->cachePtr != NULL)
	{
		status = CScalcLLGridCellCache (__This->cachePtr,llHarn,ll83);
		if (status == 0)
		{
			return status;
		}
	}

	/* Otherwise, we have to do it the hard way. */
	dtEntryPtr = CSselectNad83ToHarn (__This,ll83);
	if (dtEntryPtr != NULL)
	{
		status = CScalcNad83ToHarnEntry (dtEntryPtr,llHarn,ll83,__This->cachePtr);
	}
	else if (__This->fallback != NULL)
	{
		/* No coverage?  Use the fallback alternative is active. */
		status = CScalcFallbackForward (__This->fallback,llHarn,ll83);
	}
	else
	{
		/* We didn't find any coverage.  Use the fall back position.
		   Return a +1 to indicate an approximation. */
		status = 1;
	}
	return status;
}

/******************************************************************************
	Calculate inverse datum shift, the second main man.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int CSinverseNad83ToHarn (struct csNad83ToHarn_* __This,double* ll83,Const double *llHarn)
{
	static int itmax = 10;
	static double small = 1.0E-9;		/* this is degrees */

	short lng_ok;
	short lat_ok;
	int ii;
	int status;

	double guess [3];
	double epsilon [2];
	double newResult [3];

	guess [LNG] = llHarn [LNG];
	guess [LAT] = llHarn [LAT];
	guess [HGT] = llHarn [HGT];

	/* Start a loop which will iterate up to 10 times. The Canadians and
	   the Aussies max out at 4.  We would duplicate theirs, but since
	   this is an inverse, we'll do a little better than they do. */
	for (ii = 1;ii < itmax;ii++)
	{
		/* Assume we are done until we know different. */
		lng_ok = lat_ok = TRUE;

		/* Compute the NAD83 lat/long for our current guess. */
		status = CScalcNad83ToHarn (__This,newResult,guess);

		/* If there is no data for this lat/long, we use the fallback
		   in one is available. */
		if (status != 0)
		{
			if (status > 0 && __This->fallback != NULL)
			{
				status = CScalcFallbackInverse (__This->fallback,ll83,llHarn);
			}
			return (status);
		}

		/* See how far we are off. */
		epsilon [LNG] = llHarn [LNG] - newResult [LNG];
		epsilon [LAT] = llHarn [LAT] - newResult [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > small)
		{
			lng_ok = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		if (fabs (epsilon [LAT]) > small)
		{
			lat_ok = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newResult that is within
		   small of original, we are done. */
		if (lng_ok && lat_ok) break;
	}

	/* If we didn't resolve in four tries, we issue a warning
	   message.  Casual reading of the NADCON code would lead one
	   to believe that they do five iterations, but four is all
	   they really do. */
	if (ii >= itmax)
	{
		CS_erpt (cs_NADCON_ICNT);
		return (1);
	}

	/* Adjust the ll83 value to the computed value, now that we
	   know that it should be correct. */
	ll83 [LNG] = guess [LNG];
	ll83 [LAT] = guess [LAT];
	return 0;
}

/******************************************************************************
	Release -- Release resources, but maintain the directory status.
*/
void CSreleaseNad83ToHarn (struct csNad83ToHarn_* __This)
{
	struct csNad83ToHarnEntry_* dtEntryPtr;

	if (__This)
	{
		for (dtEntryPtr = __This->listHead;dtEntryPtr != NULL;dtEntryPtr = dtEntryPtr->next)
		{
			CSreleaseNad83ToHarnEntry (dtEntryPtr);
		}
	}
	return;
}
Const char *CSsourceNad83ToHarn (struct csNad83ToHarn_* __This,Const double ll_83 [2])
{
	const char *cp;
	struct csNad83ToHarnEntry_* dtEntryPtr;

	cp = NULL;
	if (__This->cachePtr != NULL)
	{
		cp = CSsourceLLGridCellCache (__This->cachePtr,ll_83);
	}
	if (cp == NULL)
	{
		dtEntryPtr = CSselectNad83ToHarn (__This,ll_83);
		if (dtEntryPtr != NULL)
		{
			cp = CSsourceNad83ToHarnEntry (dtEntryPtr,ll_83);
		}
	}
	if (cp == NULL && __This->fallback != NULL)
	{
		cp = CSsourceFallback (__This->fallback);
	}
	return cp;
}
/******************************************************************************
	Constructor for a Nad83ToHarnEntry object
*/
struct csNad83ToHarnEntry_* CSnewNad83ToHarnEntry (struct csDatumCatalogEntry_* catPtr)
{
	extern char cs_DirsepC;
	extern char cs_ExtsepC;
	extern char csErrnam [];

	char *cp;
	struct csNad83ToHarnEntry_* __This;

	/* Prepare for an error. */
	__This = NULL;

	/* Allocate some storage. */
	__This = (struct csNad83ToHarnEntry_*) CS_malc (sizeof (struct csNad83ToHarnEntry_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->next = NULL;
	__This->harnDatumPtr = NULL;

	/* Issue an error if this is not an HPGN guy. This will eliminate a lot
	   of frustration. */
	cp = strrchr (catPtr->pathName,cs_DirsepC);
	if (cp == NULL)
	{
		/* Is not supposed to happen, but we can't allow a bomb. */
		CS_stncp (csErrnam,catPtr->pathName,MAXPATH);
		CS_erpt (cs_DTC_PATH);
		goto error;
	}
	if (CS_stristr (cp,"HPGN") == NULL)
	{
		/* Report an NAD2787 in the HARN catalog. */
		CS_erpt (cs_HPGN_NAD);
		goto error;
	}

	/* Isolate the extension on the file. */
	cp = strrchr (catPtr->pathName,cs_ExtsepC);
	if (cp == NULL)
	{
		/* Is not supposed to happen. */
		CS_stncp (csErrnam,catPtr->pathName,MAXPATH);
		CS_erpt (cs_NAD_EXT);
		goto error;
	}
	cp += 1;

	/* Report special erros concerning the extensions.  Again, specific
	   error messages will reduce frustration. */
	if (!CS_stricmp (cp,"LAS"))
	{
		CS_erpt (cs_HPGN_EXTA);
		goto error;
	}
	if (!CS_stricmp (cp,"LOS"))
	{
		CS_erpt (cs_HPGN_EXTO);
		goto error;
	}
	if (CS_stricmp (cp,"L?S"))
	{
		CS_erpt (cs_HPGN_EXTX);
		goto error;
	}

	/* OK, the file name stuff appears OK.  We use the csDatumShiftUS object, as
	   all of the rest of this stuff is the same as NAD27 <--> NAD83. */
	__This->harnDatumPtr = CSnewDatumShiftUS (dtcTypeHarn,catPtr->pathName,catPtr->bufferSize,catPtr->flags,catPtr->density);
	if (__This->harnDatumPtr == NULL)
	{
		goto error;
	}
	return __This;
error:
	CSdeleteNad83ToHarnEntry (__This);
	return NULL;
}

/******************************************************************************
	Destructor for a Nad83ToHarnEntry object
*/
void CSdeleteNad83ToHarnEntry (struct csNad83ToHarnEntry_* __This)
{
	if (__This != NULL)
	{
		CSdeleteDatumShiftUS (__This->harnDatumPtr);
		CS_free (__This);
	}
	return;
}

/******************************************************************************
	Release resources for a Nad83ToHarnEntry object
*/
void CSreleaseNad83ToHarnEntry (struct csNad83ToHarnEntry_* __This)
{
	if (__This != NULL)
	{
		CSreleaseDatumShiftUS (__This->harnDatumPtr);
	}
	return;
}

/******************************************************************************
	Using grid density, locate the specific entry which is to be used for a
	specific point.
*/
double CStestNad83ToHarnEntry (struct csNad83ToHarnEntry_* __This,Const double* ll83)
{
	double rtnValue;

	rtnValue = CStestDatumShiftUS (__This->harnDatumPtr,ll83);
	return rtnValue;
}

/******************************************************************************
	Perform the actual calculation, once the specific "Entry" has been
	selected.  (Inverse is calculated at the main object level).
*/
int CScalcNad83ToHarnEntry (struct csNad83ToHarnEntry_* __This,double* llHarn,Const double *ll83,struct csLLGridCellCache_ *cachePtr)
{
	int status;

	status = CScalcDatumShiftUS (__This->harnDatumPtr,llHarn,ll83,cachePtr);
	return status;
}
/******************************************************************************
	Data source function.  Returns the data source file name.
*/
Const char *CSsourceNad83ToHarnEntry (struct csNad83ToHarnEntry_* __This,Const double *ll83)
{
	Const char *cp;

	cp = NULL;
	if (__This->harnDatumPtr != NULL)
	{
		cp = CSsourceDatumShiftUS (__This->harnDatumPtr,ll83);
	}
	return cp;
}
