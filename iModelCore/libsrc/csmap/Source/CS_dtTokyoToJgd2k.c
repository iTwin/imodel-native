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

/*lint -e722 */				/* suspicious use of semi-colon */

#include "cs_map.h"

/******************************************************************************
	Constructor
*/
struct csTokyoToJgd2k_ *CSnewTokyoToJgd2k (Const char *catalog)
{
	int index;
	Const char *cp;
	struct csTokyoToJgd2k_ *__This;
	struct csDatumCatalog_ *catPtr;
	struct csDatumCatalogEntry_ *catEntryPtr;
	struct csTokyoToJgd2kEntry_* dtEntryPtr;
	struct csTokyoToJgd2kEntry_* findPtr;

	/* Prepare for an error. */
	__This = NULL;
	catPtr = NULL;
	catEntryPtr = NULL;
	dtEntryPtr = NULL;

	__This = (struct csTokyoToJgd2k_*) CS_malc (sizeof (struct csTokyoToJgd2k_));
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

	/* For each entry in the catalog, we build an appropriate datum
	   shift entry.  Right now, this is based on file names and
	   file extensions.  Not very good, but that's life. */
	index = 0;
	for (;;)
	{
		catEntryPtr = CSgetDatumCatalogEntry (catPtr,index++);
		if (catEntryPtr == NULL) break;
		dtEntryPtr = CSnewTokyoToJgd2kEntry (catEntryPtr);
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

	/* Having done that successfully, allocate a grid cell cache.  If this
	   fails, we can either report it as an error, or just leave it alone.
	   Lets report it as an error. */
	__This->cachePtr = CSnewLLGridCellCache (32);
	if (__This->cachePtr == NULL)
	{
		goto error;
	}

	/* It's ready to go. */
	return __This;
error:
	if (catPtr != NULL) CSdeleteDatumCatalog (catPtr);
	CSdeleteTokyoToJgd2k (__This);
	return NULL;
}
/******************************************************************************
	Destructor
*/
void CSdeleteTokyoToJgd2k (struct csTokyoToJgd2k_* __This)
{
	struct csTokyoToJgd2kEntry_ *dtEntryPtr;

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
			CSdeleteTokyoToJgd2kEntry (dtEntryPtr);
		}
		CS_free (__This);
	}
	return;
}
/******************************************************************************
	Release -- Release resources, but maintain the 'catalog' status.  Leave
	the fallback alone, as the only way to get that back is to open up the
	catalog file again.
*/
void CSreleaseTokyoToJgd2k (struct csTokyoToJgd2k_* __This)
{
	struct csTokyoToJgd2kEntry_* dtEntryPtr;

	if (__This != NULL)
	{
		if (__This->cachePtr != NULL)
		{
			CSreleaseLLGridCellCache (__This->cachePtr);
		}
		for (dtEntryPtr = __This->listHead;dtEntryPtr != NULL;dtEntryPtr = dtEntryPtr->next)
		{
			CSreleaseTokyoToJgd2kEntry (dtEntryPtr);
		}
	}
	return;
}
/******************************************************************************
	Calculate datum shift, the main man.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int CScalcTokyoToJgd2k (struct csTokyoToJgd2k_* __This,double* llJgd2k,Const double *llTokyo)
{
	int status, st;
	struct csTokyoToJgd2kEntry_* dtEntryPtr;

	/* First see if using the cache works.  This works frequently. */
	if (__This->cachePtr != NULL)
	{
		status = CScalcLLGridCellCache (__This->cachePtr,llJgd2k,llTokyo);
		if (status == 0)
		{
			return status;
		}
	}

	/* Do the default in case none of the stuff below works. */
	llJgd2k [LNG] = llTokyo [LNG];
	llJgd2k [LAT] = llTokyo [LAT];
	status = 1;

	/* OK, we have to do it the hard way. */
	dtEntryPtr = __This->listHead;
	while (dtEntryPtr != NULL)
	{	
		st = CScalcTokyoToJgd2kEntry (dtEntryPtr,llJgd2k,llTokyo,__This->cachePtr);
		if (st < 0) status = st;
		else if (st == 0)
		{
			/* We found an entry which can effectively convert this point.
			   Save the new status value and terminate the loop.
			   CScalcTokyoToJgd2kEntry will have added this cell to the cache. */
			status = st;
			break;
		}

		/* Try the next entry listed in the catalog. */
		dtEntryPtr = dtEntryPtr->next;
	}
	if (status == 1)
	{
		/* We didn't any coverage.  Do the fallback if one has been specified. */
		if (__This->fallback != NULL)
		{
			status = CScalcFallbackForward (__This->fallback,llJgd2k,llTokyo);
		}
	}
	return status;
}
/******************************************************************************
	Calculate inverse datum shift, the second main man.  Note that this is a
		deviation from using the exact code used by the governmental products.
		The different government supplied programs use the same algorithm, but
		with slightly different implementation details.  Here we use the same
		details for all objects.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int CSinverseTokyoToJgd2k (struct csTokyoToJgd2k_* __This,double* llTokyo,Const double *llJgd2k)
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

	guess [LNG] = llJgd2k [LNG];
	guess [LAT] = llJgd2k [LAT];
	guess [HGT] = llJgd2k [HGT];

	/* Start a loop which will iterate up to 10 times. The Canadians and
	   the Aussies max out at 4.  We would duplicate theirs, but since
	   this is an inverse, we'll do a little better than they do. */
	for (ii = 1;ii < itmax;ii++)
	{
		/* Assume we are done until we know different. */
		lng_ok = lat_ok = TRUE;

		/* Compute the NAD83 lat/long for our current guess. */
		status = CScalcTokyoToJgd2k (__This,newResult,guess);

		/* If there is no data for this lat/long, we use the fallback
		   if one is available. */
		if (status != 0)
		{
			if (status > 0 && __This->fallback != NULL)
			{
				status = CScalcFallbackInverse (__This->fallback,llTokyo,llJgd2k);
			}
			return status;
		}

		/* See how far we are off. */
		epsilon [LNG] = llJgd2k [LNG] - newResult [LNG];
		epsilon [LAT] = llJgd2k [LAT] - newResult [LAT];

		/* If our guess at the longitude is off by more than 'small', we adjust
		 our guess by the amount we are off. */
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

	if (ii >= itmax)
	{
		CS_erpt (cs_TOKYO_ICNT);
		return (-1);
	}

	/* Adjust the ll_27 value to the computed value, now that we
	   know that it should be correct. */
	llTokyo [LNG] = guess [LNG];
	llTokyo [LAT] = guess [LAT];
	return 0;
}
/******************************************************************************
	Constructor: This is a constructor for the "Entry" object.  A linked list
	of these "Entry" objects is underlying structure of the main object.

	NOTE: the specific of handling different file types are handled here.
*/
struct csTokyoToJgd2kEntry_* CSnewTokyoToJgd2kEntry (struct csDatumCatalogEntry_* catPtr)
{
	extern char cs_ExtsepC;
	extern char csErrnam [];

	char *cp;
	struct csTokyoToJgd2kEntry_* __This;

	/* Prepare for an error. */
	__This = NULL;

	/* ALlocate some storage. */
	__This = (struct csTokyoToJgd2kEntry_*) CS_malc (sizeof (struct csTokyoToJgd2kEntry_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->next = NULL;
	__This->jgd2kPtr = NULL;

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

	/* Do what's appropriate for this extension. */
	if (!CS_stricmp (cp,"par"))
	{
		__This->jgd2kPtr = CSnewJgd2kGridFile (catPtr->pathName,catPtr->bufferSize,catPtr->flags,catPtr->density);
		if (__This->jgd2kPtr == NULL)
		{
			goto error;
		}
	}
	else
	{
		CS_erpt (cs_TOKYO_EXT);
		goto error;
	}
	return __This;
error:
	CSdeleteTokyoToJgd2kEntry (__This);
	return NULL;
}
/******************************************************************************
	Destructor, for an "Entry" object.
*/
void CSdeleteTokyoToJgd2kEntry (struct csTokyoToJgd2kEntry_* __This)
{
	if (__This != NULL)
	{
		CSdeleteJgd2kGridFile (__This->jgd2kPtr);
		CS_free (__This);
	}
	return;
}
/******************************************************************************
	Release resources, for an "Entry" object.
*/
void CSreleaseTokyoToJgd2kEntry (struct csTokyoToJgd2kEntry_* __This)
{
	if (__This != NULL)
	{
		CSreleaseJgd2kGridFile (__This->jgd2kPtr);
	}
	return;
}
/******************************************************************************
	Calculation function.  Calculates the conversion from NAD27 to NAD83.
*/
int CScalcTokyoToJgd2kEntry (struct csTokyoToJgd2kEntry_* __This,double* llJgd2k,Const double *llTokyo,struct csLLGridCellCache_ *cachePtr)
{
	int status;

	status = CScalcJgd2kGridFile (__This->jgd2kPtr,llJgd2k,llTokyo);
	if (status == 0 && cachePtr != NULL)
	{
		CSaddLLGridCellCache (cachePtr,dtcTypeJapanese,&__This->jgd2kPtr->lngCell,&__This->jgd2kPtr->latCell);
	}
	else if (status > 0)
	{
		llJgd2k [LNG] = llTokyo [LNG];
		llJgd2k [LAT] = llTokyo [LAT];
	}
	return status;
}
