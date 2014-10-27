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
/*lint -esym(715,ll27) */	/* parameter not referenced */

#include "cs_map.h"

/******************************************************************************
	Constructor
*/
struct csNad27ToAts77_* CSnewNad27ToAts77 (Const char *catalog)
{
	int index;
	Const char *cp;
	struct csNad27ToAts77_ *__This;
	struct csDatumCatalog_ *catPtr;
	struct csDatumCatalogEntry_ *catEntryPtr;
	struct csNad27ToAts77Entry_* dtEntryPtr;
	struct csNad27ToAts77Entry_* findPtr;

	/* Prepare for an error. */
	__This = NULL;
	catPtr = NULL;
	catEntryPtr = NULL;
	dtEntryPtr = NULL;

	__This = (struct csNad27ToAts77_*) CS_malc (sizeof (struct csNad27ToAts77_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->fallback = NULL;
	__This->listHead = NULL;

	/* Open the catalog file. */
	catPtr = CSnewDatumCatalog (catalog);
	if (catPtr == NULL) goto error;

	/* If a fallback was specified, activate it. */
	cp = CSgetFallbackName (catPtr);
	if (cp != NULL && *cp != '\0')
	{
		__This->fallback = CSnewFallback (cp,catalog);
		if (__This->fallback == NULL)
		{
			goto error;
		}
	}

	/* For each entry in the catalong, we build an appropriate datum
	   shift entry.  Catalog entries must carry path names to each individual
	   file.  We do not play any games with extensions here. */

	index = 0;
	for (;;)
	{
		catEntryPtr = CSgetDatumCatalogEntry (catPtr,index++);
		if (catEntryPtr == NULL) break;
		dtEntryPtr = CSnewNad27ToAts77Entry (catEntryPtr);
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

	/* OK, it's ready to go. */
	return __This;

error:
	if (catPtr != NULL) CSdeleteDatumCatalog (catPtr);
	CSdeleteNad27ToAts77 (__This);
	return NULL;
}
/******************************************************************************
	Destructor
*/
void CSdeleteNad27ToAts77 (struct csNad27ToAts77_* __This)
{
	struct csNad27ToAts77Entry_ *dtEntryPtr;

	if (__This != NULL)
	{
		if (__This->fallback != NULL)
		{
			CSdeleteFallback (__This->fallback);
			__This->fallback = NULL;
		}
		while (__This->listHead != NULL)
		{
			dtEntryPtr = __This->listHead;
			__This->listHead = __This->listHead->next;
			CSdeleteNad27ToAts77Entry (dtEntryPtr);
		}
		CS_free (__This);
	}
	return;
}
/******************************************************************************
	Coverage Locator: Selects the specific grid shift object to use for the
		provided point.  Selection is based on grid density value.
*/
struct csNad27ToAts77Entry_* CSselectNad27ToAts77 (struct csNad27ToAts77_* __This,enum cs_Ats77Dir_ direction,Const double *ll_test)
{
	double testValue;
	double bestSoFar;
	struct csNad27ToAts77Entry_* dtEntryPtr;
	struct csNad27ToAts77Entry_* rtnValue;

	rtnValue = NULL;
	bestSoFar = 3600000.00;
	dtEntryPtr = __This->listHead;
	while (dtEntryPtr != NULL)
	{
		testValue = CStestNad27ToAts77Entry (dtEntryPtr,direction,ll_test);
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
	Calculate datum shift, the main man.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int CScalcNad27ToAts77 (struct csNad27ToAts77_* __This,double* ll_ats77,Const double *ll_nad27)
{
	int status;
	struct csNad27ToAts77Entry_* dtEntryPtr;

	status = 0;

	dtEntryPtr = CSselectNad27ToAts77 (__This,ats77DirToAts77,ll_nad27);
	if (dtEntryPtr != NULL)
	{
		status = CScalcNad27ToAts77Entry (dtEntryPtr,ll_ats77,ll_nad27);
	}
	else if (__This->fallback != NULL)
	{
		/* Use the fallback definition if there is no coverage. */
		status = CScalcFallbackForward (__This->fallback,ll_ats77,ll_nad27);
	}
	else
	{
		/* We didn't find any coverage.  A positive value is used to indicate
		   an error, but not internally within this code. */
		status = 1;
	}
	return status;
}
int CScalcAts77ToNad27 (struct csNad27ToAts77_* __This,double* ll_nad27,Const double *ll_ats77)
{
	int status;
	struct csNad27ToAts77Entry_* dtEntryPtr;

	status = 0;

	dtEntryPtr = CSselectNad27ToAts77 (__This,ats77DirToNad27,ll_ats77);
	if (dtEntryPtr != NULL)
	{
		status = CScalcNad27ToAts77Entry (dtEntryPtr,ll_nad27,ll_ats77);
	}
	else if (__This->fallback != NULL)
	{
		/* Use the fallback definition if there is no coverage. */
		status = CScalcFallbackInverse (__This->fallback,ll_nad27,ll_ats77);
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
	Release -- Release resources, but maintain the directory status.
*/
void CSreleaseNad27ToAts77 (struct csNad27ToAts77_* __This)
{
	struct csNad27ToAts77Entry_* dtEntryPtr;

	if (__This != NULL)
	{
		for (dtEntryPtr = __This->listHead;dtEntryPtr != NULL;dtEntryPtr = dtEntryPtr->next)
		{
			CSreleaseNad27ToAts77Entry (dtEntryPtr);
		}
	}
	return;
}
Const char *CSsourceNad27ToAts77 (struct csNad27ToAts77_* __This,Const double ll_27 [2])
{
	const char *cp;
	struct csNad27ToAts77Entry_* dtEntryPtr;

	cp = NULL;
	dtEntryPtr = CSselectNad27ToAts77 (__This,ats77DirToAts77,ll_27);
	if (dtEntryPtr != NULL)
	{
		cp = CSsourceNad27ToAts77Entry (dtEntryPtr,ll_27);
	}
	if (cp == NULL && __This->fallback != NULL)
	{
		cp = CSsourceFallback (__This->fallback);
	}
	return cp;
}
/******************************************************************************
	Constructor: This is a constructor for the "Entry" object.  A linked list
	of these "Entry" objects is underlying structure of the main object.

	NOTE: the specific of handling different file types are handled here.
*/
struct csNad27ToAts77Entry_* CSnewNad27ToAts77Entry (struct csDatumCatalogEntry_* catPtr)
{
	struct csNad27ToAts77Entry_* __This;

	/* Prepare for an error. */
	__This = NULL;

	/* Allocate some storage. */
	__This = (struct csNad27ToAts77Entry_*) CS_malc (sizeof (struct csNad27ToAts77Entry_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->next = NULL;
	__This->type = dtNad27ToAts77NoneYet;
	__This->pointers.ats77XfrmPtr = NULL;

	/* Set up the file; construct a Canadian National Transformation,
	   Version 2 object.  It's smart enough to deal with the differences
	   between the Canadian format and the Australian variation. */
	__This->pointers.ats77XfrmPtr = CSnewAts77Xfrm (catPtr->pathName,catPtr->flags,catPtr->density);
	if (__This->pointers.ats77XfrmPtr == NULL)
	{
		goto error;
	}
	__This->type = dtAts77Xfrm;
	return __This;

error:
	CSdeleteNad27ToAts77Entry (__This);
	return NULL;
}

/******************************************************************************
	Destructor, for an "Entry" object.
*/
void CSdeleteNad27ToAts77Entry (struct csNad27ToAts77Entry_* __This)
{
	if (__This != NULL)
	{
		switch (__This->type){
		case dtAts77Xfrm:
			CSdeleteAts77Xfrm (__This->pointers.ats77XfrmPtr);
			break;
		case dtNad27ToAts77NoneYet:
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
void CSreleaseNad27ToAts77Entry (struct csNad27ToAts77Entry_* __This)
{
	if (__This != NULL)
	{
		switch (__This->type){
		case dtAts77Xfrm:
			CSreleaseAts77Xfrm (__This->pointers.ats77XfrmPtr);
			break;
		case dtNad27ToAts77NoneYet:
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
double CStestNad27ToAts77Entry (struct csNad27ToAts77Entry_* __This,enum cs_Ats77Dir_ direction,Const double* ll_test)
{
	extern double cs_Zero;

	double rtnValue = cs_Zero;

	if (__This != NULL)
	{
		switch (__This->type) {
		case dtAts77Xfrm:
			rtnValue = CStestAts77Xfrm (__This->pointers.ats77XfrmPtr,direction,ll_test);
			break;
		case dtNad27ToAts77NoneYet:
		default:
			break;
		}
	}
	return rtnValue;
}
/******************************************************************************
	Calculation function.  Calculates the conversion from Ats77 to NAD83.
*/
int CScalcNad27ToAts77Entry (struct csNad27ToAts77Entry_* __This,double* ll_out,Const double *ll_in)
{
	extern char csErrnam [];

	int status;

	switch (__This->type){
	case dtAts77Xfrm:
		status = CScalcAts77Xfrm (__This->pointers.ats77XfrmPtr,ll_out,ll_in);
		break;
	case dtNad27ToAts77NoneYet:
	default:
		CS_stncp (csErrnam,"CS_dtNad27ToAts77:1",MAXPATH);
		CS_erpt (cs_ISER);
		status = -1;
		break;
	}
	return status;
}
/******************************************************************************
	Data source function.  Returns the data source file name.
*/
Const char *CSsourceNad27ToAts77Entry (struct csNad27ToAts77Entry_* __This,Const double *ll27)
{
	Const char *cp;

	switch (__This->type) {
	case dtAts77Xfrm:
		cp = CSsourceAts77Xfrm (__This->pointers.ats77XfrmPtr);
		break;
	case dtNad27ToAts77NoneYet:
	default:
		cp = NULL;
		break;
	}
	return cp;
}
