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

	Currently, there is only a single file. For consistency, we use the
	current code which is very very similar to that used in other
	implementations.  Eventually, this will all be replaced with
	a C++ abstract base class, and all this duplicate coade will go
	away.
*/
/*lint -e722 */				/* suspicious use of semi-colon */

#include "cs_map.h"

/******************************************************************************
	Constructor
*/
struct csNzgd49ToNzgd2K_* CSnewNzgd49ToNzgd2K (Const char *catalog)
{
	int index;
	Const char *cp;
	struct csNzgd49ToNzgd2K_ *__This;
	struct csDatumCatalog_ *catPtr;
	struct csDatumCatalogEntry_ *catEntryPtr;
	struct csNzgd49ToNzgd2KEntry_* dtEntryPtr;
	struct csNzgd49ToNzgd2KEntry_* findPtr;

	/* Prepare for an error. */
	__This = NULL;
	catPtr = NULL;
	catEntryPtr = NULL;
	dtEntryPtr = NULL;

	__This = (struct csNzgd49ToNzgd2K_*) CS_malc (sizeof (struct csNzgd49ToNzgd2K_));
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

	/* Activate the fallback algorithm if one was specified. */
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
	   shift entry.  Catalog entries must carry path names to each individual
	   file.  We do not play any games with extensions here. */

	index = 0;
	for (;;)
	{
		catEntryPtr = CSgetDatumCatalogEntry (catPtr,index++);
		if (catEntryPtr == NULL) break;
		dtEntryPtr = CSnewNzgd49ToNzgd2KEntry (catEntryPtr);
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

	/* OK, it's ready to go. */
	return __This;

error:
	if (catPtr != NULL) CSdeleteDatumCatalog (catPtr);
	CSdeleteNzgd49ToNzgd2K (__This);
	return NULL;
}

/******************************************************************************
	Destructor
*/
void CSdeleteNzgd49ToNzgd2K (struct csNzgd49ToNzgd2K_* __This)
{
	struct csNzgd49ToNzgd2KEntry_ *dtEntryPtr;

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
			CSdeleteNzgd49ToNzgd2KEntry (dtEntryPtr);
		}
		CS_free (__This);
	}
	return;
}

/******************************************************************************
	Coverage Locator: Selects the specific grid shift object to use for the
		provided point.  Selection is based on grid denity value.
*/
struct csNzgd49ToNzgd2KEntry_* CSselectNzgd49ToNzgd2K (struct csNzgd49ToNzgd2K_* __This,Const double *ll49)
{
	double testValue;
	double bestSoFar;
	struct csNzgd49ToNzgd2KEntry_* dtEntryPtr;
	struct csNzgd49ToNzgd2KEntry_* rtnValue;

	rtnValue = NULL;
	bestSoFar = 3600000.00;
	dtEntryPtr = __This->listHead;
	while (dtEntryPtr != NULL)
	{
		testValue = CStestNzgd49ToNzgd2KEntry (dtEntryPtr,ll49);
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
	Make First: Makes provided entry first on the linked list. Used for
		performance.  Well used to use it for performance.  This has been
		replaced with the GridCellCache business.  We now want to keep the
		order of these things the same as they appear in the catalog file.
*/
void CSfirstNzgd49ToNzgd2K (struct csNzgd49ToNzgd2K_* __This,struct csNzgd49ToNzgd2KEntry_* dtEntryPtr)
{
	struct csNzgd49ToNzgd2KEntry_* curPtr;
	struct csNzgd49ToNzgd2KEntry_* prvPtr;

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
int CScalcNzgd49ToNzgd2K (struct csNzgd49ToNzgd2K_* __This,double* ll_2K,Const double *ll_49)
{
	int status;
	struct csNzgd49ToNzgd2KEntry_* dtEntryPtr;

	status = 0;

	/* First see if using the cache works.  This works more often than not. */
	if (__This->cachePtr != NULL)
	{
		status = CScalcLLGridCellCache (__This->cachePtr,ll_2K,ll_49);
		if (status == 0)
		{
			return status;
		}
	}

	/* I guess we'll have to do it the hard way. */
	dtEntryPtr = CSselectNzgd49ToNzgd2K (__This,ll_49);
	if (dtEntryPtr != NULL)
	{
		status = CScalcNzgd49ToNzgd2KEntry (dtEntryPtr,ll_2K,ll_49,__This->cachePtr);
	}
	else if (__This->fallback != NULL)
	{
		/* Use the fallback if specified and no coverage. */
		status = CScalcFallbackForward (__This->fallback,ll_2K,ll_49);
	}
	else
	{
		/* We didn't find any coverage.  A positive value is used to indicate
		   an error, but not internally within this code. */
		status = 1;
	}
	return status;
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
int CSinverseNzgd49ToNzgd2K (struct csNzgd49ToNzgd2K_* __This,double* ll_49,Const double *ll_2K)
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

	guess [LNG] = ll_2K [LNG];
	guess [LAT] = ll_2K [LAT];
	guess [HGT] = ll_2K [HGT];

	/* Start a loop which will iterate up to 10 times. The Canadians and
	   the Aussies max out at 4.  We would duplicate theirs, but since
	   this is an inverse, we'll do a little better than they do. */
	for (ii = 1;ii < itmax;ii++)
	{
		/* Assume we are done until we know different. */
		lng_ok = lat_ok = TRUE;

		/* Compute the NZGD2K lat/long for our current guess. */
		status = CScalcNzgd49ToNzgd2K (__This,newResult,guess);

		/* If there is no data for this lat/long, we use the fallback
		   if one is available. */
		if (status != 0)
		{
			if (status > 0 && __This->fallback != NULL)
			{
				status = CScalcFallbackInverse (__This->fallback,ll_49,ll_2K);
			}
			return (status);
		}

		/* See how far we are off. */
		epsilon [LNG] = ll_2K [LNG] - newResult [LNG];
		epsilon [LAT] = ll_2K [LAT] - newResult [LAT];

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

	/* Adjust the ll66 value to the computed value, now that we
	   know that it should be correct. */
	ll_49 [LNG] = guess [LNG];
	ll_49 [LAT] = guess [LAT];
	return 0;
}

/******************************************************************************
	Release -- Release resources, but maintain the directory status.
*/
void CSreleaseNzgd49ToNzgd2K (struct csNzgd49ToNzgd2K_* __This)
{
	struct csNzgd49ToNzgd2KEntry_* dtEntryPtr;

	if (__This != NULL)
	{
		if (__This->cachePtr != NULL)
		{
			CSreleaseLLGridCellCache (__This->cachePtr);
		}
		for (dtEntryPtr = __This->listHead;dtEntryPtr != NULL;dtEntryPtr = dtEntryPtr->next)
		{
			CSreleaseNzgd49ToNzgd2KEntry (dtEntryPtr);
		}
	}
	return;
}

/******************************************************************************
	Constructor: This is a constructor for the "Entry" object.  A linked list
	of these "Entry" objects is underlying structure of the main object.

	NOTE: the specific of handling different file types are handled here.
*/
struct csNzgd49ToNzgd2KEntry_* CSnewNzgd49ToNzgd2KEntry (struct csDatumCatalogEntry_* catPtr)
{
	struct csNzgd49ToNzgd2KEntry_* __This;

	/* Prepare for an error. */
	__This = NULL;

	/* Allocate some storage. */
	__This = (struct csNzgd49ToNzgd2KEntry_*) CS_malc (sizeof (struct csNzgd49ToNzgd2KEntry_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->next = NULL;
	__This->type = dtNzgd49ToNzgd2KNoneYet;
	__This->pointers.c2DatumPtr = NULL;

	/* Set up the file; construct a Canadian National Transformation,
	   Version 2 object.  It's smart enough to deal with the differences
	   between the Canadian format and the Australian variation. */
	__This->pointers.c2DatumPtr = CSnewDatumShiftCa2 (dtcTypeAustralian,catPtr->pathName,catPtr->bufferSize,catPtr->flags,catPtr->density);
	if (__This->pointers.c2DatumPtr == NULL)
	{
		goto error;
	}
	__This->type = dtNzgd49ToNzgd2KC2;
	return __This;

error:
	CSdeleteNzgd49ToNzgd2KEntry (__This);
	return NULL;
}

/******************************************************************************
	Destructor, for an "Entry" object.
*/
void CSdeleteNzgd49ToNzgd2KEntry (struct csNzgd49ToNzgd2KEntry_* __This)
{
	if (__This != NULL)
	{
		switch (__This->type){
		case dtNzgd49ToNzgd2KC2:
			CSdeleteDatumShiftCa2 (__This->pointers.c2DatumPtr);
			break;
		case dtNzgd49ToNzgd2KNoneYet:
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
void CSreleaseNzgd49ToNzgd2KEntry (struct csNzgd49ToNzgd2KEntry_* __This)
{
	if (__This != NULL)
	{
		switch (__This->type){
		case dtNzgd49ToNzgd2KC2:
			CSreleaseDatumShiftCa2 (__This->pointers.c2DatumPtr);
			break;
		case dtNzgd49ToNzgd2KNoneYet:
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
double CStestNzgd49ToNzgd2KEntry (struct csNzgd49ToNzgd2KEntry_* __This,Const double* ll66)
{
	double rtnValue;

	switch (__This->type){
	case dtNzgd49ToNzgd2KC2:
		rtnValue = CStestDatumShiftCa2 (__This->pointers.c2DatumPtr,ll66);
		break;
	case dtNzgd49ToNzgd2KNoneYet:
	default:
		rtnValue = 0.0;
		break;
	}
	return rtnValue;
}

/******************************************************************************
	Calculation function.  Calculates the conversion from Nzgd49 to NZGD2K.
*/
int CScalcNzgd49ToNzgd2KEntry (struct csNzgd49ToNzgd2KEntry_* __This,double* ll_2K,Const double *ll_49,struct csLLGridCellCache_ *cachePtr)
{
	extern char csErrnam [];

	int status;

	switch (__This->type){
	case dtNzgd49ToNzgd2KC2:
		status = CScalcDatumShiftCa2 (__This->pointers.c2DatumPtr,ll_2K,ll_49,cachePtr);
		break;
	case dtNzgd49ToNzgd2KNoneYet:
	default:
		/* Minus one indicates a fatal error.  In this case, it is an internal
		   software error. */
		CS_stncp (csErrnam,"CS_dtNzgd49ToNzgd2K:1",MAXPATH);
		CS_erpt (cs_ISER);
		status = -1;
		break;
	}
	return status;
}
