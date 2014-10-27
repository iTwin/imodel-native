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

	This object maintains a list of various datum shift objects
	enabling the conversion of geographic coordinates from NAD27
	to NAD83 (and vice versa).  Thus, NAD27/NAD83 can be
	considered a single object.  Includes US stuff, and both
	versions of the Canadian National Transformation.
*/
/*lint -e722 */				/* suspicious use of semi-colon */

#include "cs_map.h"

/******************************************************************************
	Constructor
*/
struct csNad27ToNad83_* CSnewNad27ToNad83 (Const char *catalog)
{
	int index;
	Const char *cp;
	struct csNad27ToNad83_ *__This;
	struct csDatumCatalog_ *catPtr;
	struct csDatumCatalogEntry_ *catEntryPtr;
	struct csNad27ToNad83Entry_* dtEntryPtr;
	struct csNad27ToNad83Entry_* findPtr;

	/* Prepare for an error. */
	__This = NULL;
	catPtr = NULL;
	catEntryPtr = NULL;
	dtEntryPtr = NULL;

	__This = (struct csNad27ToNad83_*) CS_malc (sizeof (struct csNad27ToNad83_));
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

	/* Activate the fallback is such was specified. */
	cp = CSgetFallbackName (catPtr);
	if (cp != NULL && *cp != '\0')
	{
		/* Set name of catalog file in case of error. */
		__This->fallback = CSnewFallback (cp,catalog);
		if (__This->fallback == NULL)
		{
			goto error;
		}
	}

	/* For each entry in the catalong, we build an appropriate datum
	   shift entry.  Right now, this is based on file names and
	   file extensions.  Not very good, but that's life. */

	index = 0;
	for (;;)
	{
		catEntryPtr = CSgetDatumCatalogEntry (catPtr,index++);
		if (catEntryPtr == NULL) break;
		dtEntryPtr = CSnewNad27ToNad83Entry (catEntryPtr);
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
	CSdeleteNad27ToNad83 (__This);
	return NULL;
}

/******************************************************************************
	Destructor
*/
void CSdeleteNad27ToNad83 (struct csNad27ToNad83_* __This)
{
	struct csNad27ToNad83Entry_ *dtEntryPtr;

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
			CSdeleteNad27ToNad83Entry (dtEntryPtr);
		}
		CS_free (__This);
	}
	return;
}

/******************************************************************************
	Coverage Locator: Selects the specific grid shift object to use for the
		provided point.  Selection is based on grid denity value.
*/
struct csNad27ToNad83Entry_* CSselectNad27ToNad83 (struct csNad27ToNad83_* __This,Const double *ll27)
{
	double testValue;
	double bestSoFar;
	struct csNad27ToNad83Entry_* dtEntryPtr;
	struct csNad27ToNad83Entry_* rtnValue;

	rtnValue = NULL;
	bestSoFar = 3600000.00;
	dtEntryPtr = __This->listHead;
	while (dtEntryPtr != NULL)
	{
		testValue = CStestNad27ToNad83Entry (dtEntryPtr,ll27);
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
	Make First: Makes provided entry first on the linked list.  Was intended
	to boost performance, but replaced with the GridCellCache business.  We
	now keep the entries ordered in the order they appear in the catalog file.
*/
void CSfirstNad27ToNad83 (struct csNad27ToNad83_* __This,struct csNad27ToNad83Entry_* dtEntryPtr)
{
	struct csNad27ToNad83Entry_* curPtr;
	struct csNad27ToNad83Entry_* prvPtr;

	/* Take care of the already first situation very quickly. */
	if (dtEntryPtr == __This->listHead) return;

	/* Locate this guy on the list. */
	for (curPtr = __This->listHead,prvPtr = NULL;
		 curPtr != NULL;
		 prvPtr = curPtr,curPtr = curPtr->next)
	{
		if (curPtr == dtEntryPtr)
		{
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
int CScalcNad27ToNad83 (struct csNad27ToNad83_* __This,double* ll83,Const double *ll27)
{
	extern double cs_Km180;
	extern double cs_K360;

	int status;
	int flag180;
	struct csNad27ToNad83Entry_* dtEntryPtr;

	double lcl_ll27 [3];

	/* First see if using the cache works.  This works frequently. */
	if (__This->cachePtr != NULL)
	{
		status = CScalcLLGridCellCache (__This->cachePtr,ll83,ll27);
		if (status == 0)
		{
			return status;
		}
	}

	/* OK, we have to do it the hard way. */
	status = 0;
	flag180 = FALSE;

	lcl_ll27 [LNG] = ll27 [LNG];
	lcl_ll27 [LAT] = ll27 [LAT];

	/* Need to play a litle game here for Alaska.  The NadCon data
	   files are built on a concept of longitude < (west of) -180.
	   Therefore, we may need to play a game here. */
	if (lcl_ll27 [LNG] > 0.0)
	{
		flag180 = TRUE;
		lcl_ll27 [LNG] -= cs_K360;
	}

	/* Locate the appropriate database file. */
	dtEntryPtr = CSselectNad27ToNad83 (__This,lcl_ll27);
	if (dtEntryPtr != NULL)
	{
		/* Do the calculation. */
		status = CScalcNad27ToNad83Entry (dtEntryPtr,ll83,lcl_ll27,__This->cachePtr);

		/* Reverse the Alaska kludge.  Careful, it might have switched back to
		   being greater than (i.e. east of) -180. */
		if (status >= 0 && flag180 && (ll83 [LNG] < cs_Km180))
		{
			ll83 [LNG] += cs_K360;
		}
	}
	else if (__This->fallback != NULL)
	{
		/* Use the fallback if there is no coverage for this point. */
		status = CScalcFallbackForward (__This->fallback,ll83,ll27);
	}
	else
	{
		/* We didn't find any coverage, and no fallback was specified,
		   so return +1; i.e. a soft error. */
		ll83 [LNG] = ll27 [LNG];
		ll83 [LAT] = ll27 [LAT];
		status = 1;
	}
	return status;
}
Const char *CSsourceNad27ToNad83 (struct csNad27ToNad83_* __This,Const double ll_in [2])
{
	const char *cp;
	struct csNad27ToNad83Entry_* dtEntryPtr;

	cp = NULL;
	if (__This->cachePtr != NULL)
	{
		cp = CSsourceLLGridCellCache (__This->cachePtr,ll_in);
	}
	if (cp == NULL)
	{
		dtEntryPtr = CSselectNad27ToNad83 (__This,ll_in);
		if (dtEntryPtr != NULL)
		{
			cp = CSsourceNad27ToNad83Entry (dtEntryPtr,ll_in);
		}
	}
	if (cp == NULL && __This->fallback != NULL)
	{
		cp = CSsourceFallback (__This->fallback);
	}
	return cp;
}
/******************************************************************************
	Calculate inverse datum shift, the second main man.  Note that this is a
		deviation from using the exact code used by the governmental products.
		The different government supplied programs use the same algorothm, but
		with slightly different implementation details.  Here we use the same
		details for all objects.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int CSinverseNad27ToNad83 (struct csNad27ToNad83_* __This,double* ll27,Const double *ll83)
{
	static int itmax = 10;
	static double small = 1.0E-11;		/* this is degrees */

	short lng_ok;
	short lat_ok;
	int ii;
	int status;

	double guess [3];
	double epsilon [2];
	double newResult [3];

	guess [LNG] = ll83 [LNG];
	guess [LAT] = ll83 [LAT];
	guess [HGT] = ll83 [HGT];

	/* Start a loop which will iterate up to 10 times. The Canadians and
	   the Aussies max out at 4.  We would duplicate theirs, but since
	   this is an inverse, we'll do a little better than they do. */
	for (ii = 1;ii < itmax;ii++)
	{
		/* Assume we are done until we know different. */
		lng_ok = lat_ok = TRUE;

		/* Compute the NAD83 lat/long for our current guess. */
		status = CScalcNad27ToNad83 (__This,newResult,guess);

		/* If there is no data for this lat/long, we use the fallback
		   in one is available. */
		if (status != 0)
		{
			if (status > 0 && __This->fallback != NULL)
			{
				status = CScalcFallbackInverse (__This->fallback,ll27,ll83);
			}
			return (status);
		}

		/* See how far we are off. */
		epsilon [LNG] = ll83 [LNG] - newResult [LNG];
		epsilon [LAT] = ll83 [LAT] - newResult [LAT];

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
		return (-1);
	}

	/* Adjust the ll_27 value to the computed value, now that we
	   know that it should be correct. */
	ll27 [LNG] = guess [LNG];
	ll27 [LAT] = guess [LAT];
	return 0;
}
/******************************************************************************
	Release -- Release resources, but maintain the directory status.
*/
void CSreleaseNad27ToNad83 (struct csNad27ToNad83_* __This)
{
	struct csNad27ToNad83Entry_* dtEntryPtr;

	if (__This != NULL)
	{
		if (__This->cachePtr != NULL)
		{
			CSreleaseLLGridCellCache (__This->cachePtr);
		}
		for (dtEntryPtr = __This->listHead;dtEntryPtr != NULL;dtEntryPtr = dtEntryPtr->next)
		{
			CSreleaseNad27ToNad83Entry (dtEntryPtr);
		}
	}
	return;
}

/******************************************************************************
	Constructor: This is a constructor for the "Entry" object.  A linked list
	of these "Entry" objects is underlying structure of the main object.

	NOTE: the specific of handling different file types are handled here.
*/
struct csNad27ToNad83Entry_* CSnewNad27ToNad83Entry (struct csDatumCatalogEntry_* catPtr)
{
	extern char cs_DirsepC;
	extern char cs_ExtsepC;
	extern char csErrnam [];

	char *cp;
	struct csNad27ToNad83Entry_* __This;

	/* Prepare for an error. */
	__This = NULL;

	/* ALlocate some storage. */
	__This = (struct csNad27ToNad83Entry_*) CS_malc (sizeof (struct csNad27ToNad83Entry_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->next = NULL;
	__This->type = dtNad27To83NoneYet;
	__This->pointers.usDatumPtr = NULL;

	/* Issue an error if this is an HPGN guy. This will eliminate a lot
	   of frustration. */
	cp = strrchr (catPtr->pathName,cs_DirsepC);
	if (cp == NULL)
	{
		/* Is not supposed to happen, but we can't allow a bomb. */
		CS_stncp (csErrnam,catPtr->pathName,MAXPATH);
		CS_erpt (cs_DTC_PATH);
		goto error;
	}
	if (CS_stristr (cp,"HPGN") != NULL)
	{
		/* Report an HPGN file in the NAD2783 catalog. */
		CS_erpt (cs_NAD_HPGN);
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

	/* Report special errors concerning the extensions.  Again, specific
	   error messages will reduce frustration. */
	if (!CS_stricmp (cp,"LAS"))
	{
		CS_erpt (cs_NAD_LAS);
		goto error;
	}
	if (!CS_stricmp (cp,"LOS"))
	{
		CS_erpt (cs_NAD_LOS);
		goto error;
	}

	/* Do what's appropriate for this extension. */
	if (!CS_stricmp (cp,"L?S"))
	{
		/* Must not set the type until allocated for correct error handling. */
		__This->pointers.usDatumPtr = CSnewDatumShiftUS (dtcTypeUS,catPtr->pathName,catPtr->bufferSize,catPtr->flags,catPtr->density);
		if (__This->pointers.usDatumPtr == NULL)
		{
			goto error;
		}
		__This->type = dtNad27To83US;
	}
	else if (!CS_stricmp (cp,"DAC"))
	{
		/* Construct a Canadian Version 1 object. */
		__This->pointers.c1DatumPtr = CSnewDatumShiftCa1 (catPtr->pathName,catPtr->bufferSize,catPtr->flags,catPtr->density);
		if (__This->pointers.c1DatumPtr == NULL)
		{
			goto error;
		}
		__This->type = dtNad27To83C1;
	}
	else if (!CS_stricmp (cp,"GSB"))
	{
		/* Construct a Canadian Version 2 object. */
		__This->pointers.c2DatumPtr = CSnewDatumShiftCa2 (dtcTypeCanadian2,catPtr->pathName,catPtr->bufferSize,catPtr->flags,catPtr->density);
		if (__This->pointers.c2DatumPtr == NULL)
		{
			goto error;
		}
		__This->type = dtNad27To83C2;
	}
	else
	{
		CS_stncp (csErrnam,catPtr->pathName,MAXPATH);
		CS_erpt (cs_NAD_EXT);
		goto error;
	}
	return __This;
error:
	CSdeleteNad27ToNad83Entry (__This);
	return NULL;
}

/******************************************************************************
	Destructor, for an "Entry" object.
*/
void CSdeleteNad27ToNad83Entry (struct csNad27ToNad83Entry_* __This)
{
	if (__This != NULL)
	{
		switch (__This->type){
		case dtNad27To83US:
			CSdeleteDatumShiftUS (__This->pointers.usDatumPtr);
			break;
		case dtNad27To83C1:
			CSdeleteDatumShiftCa1 (__This->pointers.c1DatumPtr);
			break;
		case dtNad27To83C2:
			CSdeleteDatumShiftCa2 (__This->pointers.c2DatumPtr);
			break;
		case dtNad27To83NoneYet:
		default:
			break;
		}
		CS_free (__This);
	}
	return;
}

/******************************************************************************
	Release resources, for an "Entry" object.
*/
void CSreleaseNad27ToNad83Entry (struct csNad27ToNad83Entry_* __This)
{
	if (__This != NULL)
	{
		switch (__This->type){
		case dtNad27To83US:
			CSreleaseDatumShiftUS (__This->pointers.usDatumPtr);
			break;
		case dtNad27To83C1:
			CSreleaseDatumShiftCa1 (__This->pointers.c1DatumPtr);
			break;
		case dtNad27To83C2:
			CSreleaseDatumShiftCa2 (__This->pointers.c2DatumPtr);
			break;
		case dtNad27To83NoneYet:
		default:
			break;
		}
	}
	return;
}

/******************************************************************************
	Test function.  Used to locate the specific "Entry" item in the main object
	linked list which is the best entry to use for converting the supplied
	point.
*/
double CStestNad27ToNad83Entry (struct csNad27ToNad83Entry_* __This,Const double* ll27)
{
	double rtnValue;

	switch (__This->type){
	case dtNad27To83US:
		rtnValue = CStestDatumShiftUS (__This->pointers.usDatumPtr,ll27);
		break;
	case dtNad27To83C1:
		rtnValue = CStestDatumShiftCa1 (__This->pointers.c1DatumPtr,ll27);
		break;
	case dtNad27To83C2:
		rtnValue = CStestDatumShiftCa2 (__This->pointers.c2DatumPtr,ll27);
		break;
	case dtNad27To83NoneYet:
	default:
		rtnValue = 0.0;
		break;
	}
	return rtnValue;
}

/******************************************************************************
	Calculation function.  Calculates the conversion from NAD27 to NAD83.
*/
int CScalcNad27ToNad83Entry (struct csNad27ToNad83Entry_* __This,double* ll83,Const double *ll27,struct csLLGridCellCache_ *cachePtr)
{
	int status;

	switch (__This->type){
	case dtNad27To83US:
		status = CScalcDatumShiftUS (__This->pointers.usDatumPtr,ll83,ll27,cachePtr);
		break;
	case dtNad27To83C1:
		status = CScalcDatumShiftCa1 (__This->pointers.c1DatumPtr,ll83,ll27,cachePtr);
		break;
	case dtNad27To83C2:
		status = CScalcDatumShiftCa2 (__This->pointers.c2DatumPtr,ll83,ll27,cachePtr);
		break;
	case dtNad27To83NoneYet:
	default:
		status = -1;
		break;
	}
	return status;
}
/******************************************************************************
	Data source function.  Returns the data source file name.
*/
Const char *CSsourceNad27ToNad83Entry (struct csNad27ToNad83Entry_* __This,Const double *ll27)
{
	Const char *cp;

	switch (__This->type) {
	case dtNad27To83US:
		cp = CSsourceDatumShiftUS (__This->pointers.usDatumPtr,ll27);
		break;
	case dtNad27To83C1:
		cp = CSsourceDatumShiftCa1 (__This->pointers.c1DatumPtr,ll27);
		break;
	case dtNad27To83C2:
		cp = CSsourceDatumShiftCa2 (__This->pointers.c2DatumPtr,ll27);
		break;
	case dtNad27To83NoneYet:
	default:
		cp = NULL;
		break;
	}
	return cp;
}
