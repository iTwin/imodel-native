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

/*lint -e722 */   /* suspicious use of semi-colon */

#include "cs_map.h"

/******************************************************************************
	Constructor
*/
struct csRgf93ToNtf_* CSnewRgf93ToNtf (Const char *catalog)
{
	int index;
	Const char *cp;
	struct csRgf93ToNtf_ *__This;
	struct csDatumCatalog_ *catPtr;
	struct csDatumCatalogEntry_ *catEntryPtr;
	struct csRgf93ToNtfEntry_* dtEntryPtr;
	struct csRgf93ToNtfEntry_* findPtr;

	/* Prepare for an error. */
	__This = NULL;
	catPtr = NULL;
	catEntryPtr = NULL;
	dtEntryPtr = NULL;

	__This = (struct csRgf93ToNtf_*) CS_malc (sizeof (struct csRgf93ToNtf_));
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

	/* Get a pointer to the fallback definition.  This will be NULL if there
	   is none. */
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
		dtEntryPtr = CSnewRgf93ToNtfEntry (catEntryPtr);
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
	CSdeleteRgf93ToNtf (__This);
	return NULL;
}

/******************************************************************************
	Destructor
*/
void CSdeleteRgf93ToNtf (struct csRgf93ToNtf_* __This)
{
	struct csRgf93ToNtfEntry_ *dtEntryPtr;

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
			CSdeleteRgf93ToNtfEntry (dtEntryPtr);
		}
		CS_free (__This);
	}
	return;
}

/******************************************************************************
	Coverage Locator: Selects the specific grid shift object to use for the
	provided point.  Selection is based on grid density value.
*/
struct csRgf93ToNtfEntry_* CSselectRgf93ToNtf (struct csRgf93ToNtf_* __This,
											   enum csRgf93ToNtfType srchType,
											   Const double *ll)
{
	double testValue;
	double bestSoFar;
	struct csRgf93ToNtfEntry_* dtEntryPtr;
	struct csRgf93ToNtfEntry_* rtnValue;

	rtnValue = NULL;
	bestSoFar = 3600000.00;
	dtEntryPtr = __This->listHead;
	while (dtEntryPtr != NULL)
	{
		if (srchType == dtRgf93ToNtfNone || dtEntryPtr->type == srchType)
		{
			testValue = CStestRgf93ToNtfEntry (dtEntryPtr,ll);
			if (testValue != 0.0 && testValue < bestSoFar)
			{
				bestSoFar = testValue;
				rtnValue = dtEntryPtr;
			}
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
/******************************************************************************
   Major complication here.  The original French conversion technique using the
   gr3df97a.txt file implements a conversion from RGF93 to NTF (just the
   opposite of most conversion techniques).  The local grid files which use the
   Canadian NTv2 data format implement a conversion from NTF to RGF93 (just the
   opposite of the original technique).
   
   So, as usual, we have to do some major kludging to get all this to work
   correctly.  What this means, in this case, is that a lot of the standard
   .gdc type code cannot be used.  We'll need to recode a lot that.
*****************************************************************************/
int CScalcRgf93ToNtf (struct csRgf93ToNtf_* __This,double* llNtf,Const double *llRgf93)
{
	int status;
	struct csRgf93ToNtfEntry_* dtEntryPtr;
	struct csDatumShiftCa2_* caNtv2Ptr;
	struct csRgf93ToNtfTxt_* txtDatumPtr;

	double tmpHgt;
	double lclLlNtf [3];

	/* Until we know different */
	status = 1;
	llNtf [0] = llRgf93 [0];
	llNtf [1] = llRgf93 [1];
	llNtf [2] = llRgf93 [2];

	/* In order to accurately determine if a local grid file should be used,
	   we need NTF coordinates to examine the extents of the local grid files,
	   since the extents of those data files are provided in NTF coordinates
	   (at least they should be if they adhere to the Canadian NTv2 convention).

	   If it turns out that we are not to use a local grid file, the NTF
	   values we compute are the desired return value, so this is not all
	   wasted effort.  So, first step is to locate the national grid and
	   use it to convert the provided RGF93 coordinates to NTF.  The
	   select routine for this datum has been modified from the traditional
	   .gdc select function in that it takes an additional argument: the
	   type of file we wish to locate. */
	dtEntryPtr = CSselectRgf93ToNtf (__This,dtRgf93ToNtfTxt,llRgf93);
	if (dtEntryPtr != NULL)
	{
		txtDatumPtr = dtEntryPtr->pointers.txtDatumPtr;
		status = CScalcRgf93ToNtfTxt (txtDatumPtr,lclLlNtf,llRgf93);
		if (status == 0)
		{
			/* We have some NTF geographic coordinates which we can use
			   to see if there is a local grid which we really should
			   be using. */
			dtEntryPtr = CSselectRgf93ToNtf (__This,dtRgf93ToNtfC2,lclLlNtf);
			if (dtEntryPtr != NULL)
			{
				/* Yup, there is a local grid available whose extents include
				   this NTF point.  So, we use an inverse calculation on that
				   grid to compute the point.
				   
				   The above calculation of the NTF coordinates using the
				   national text file also gave us the converted height.
				   We need to save this height, as the CSinverseDatumShiftCa2
				   function is likely to clobber it. */
				tmpHgt = lclLlNtf [2];
				caNtv2Ptr = dtEntryPtr->pointers.c2DatumPtr;
				status = CSinverseDatumShiftCa2 (caNtv2Ptr,lclLlNtf,llRgf93);
				lclLlNtf [2] = tmpHgt;
			}
		}
	}
	else
	{
		/* No coverage at all.  Use the fallback technique if there is one.
		   Ugly!!!  Since most fallback definitions convert from the defined
		   datum to WGS84, we need to use the inverse of the specified fallback,
		   even though this is a forward function. */
		if (__This->fallback != NULL)
		{
			status = CScalcFallbackInverse (__This->fallback,lclLlNtf,llRgf93);
		}
	}

	/* Return the calculated results. */
	if (status == 0 || status == 2)
	{
		llNtf [0] = lclLlNtf [0];
		llNtf [1] = lclLlNtf [1];
		llNtf [2] = lclLlNtf [2];
	}
	return status;
}
/******************************************************************************
	Calculate inverse datum shift, the second main man.e levels.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 2for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int CSinverseRgf93ToNtf (struct csRgf93ToNtf_* __This,double* llRgf93,Const double *llNtf)
{
	int status;
	struct csRgf93ToNtfEntry_* dtEntryPtr;
	struct csRgf93ToNtfTxt_ *txtDatumPtr;
	struct csDatumShiftCa2_* caNtv2Ptr;

	double tmpLL [3];

	/* Until we know different.*/
	status = 1;

	/* The local grids use the NTv2 transformation format.  This is a 2D
	   transformation and, therefore, it does not give us a height shift.
	   Since the previous implementation used the national text file which
	   is a 3D transformation, the height shift was always present.  Therefore,
	   in order to get the height shift, we will always do the national text
	   calculation.  Then, if there is a local grid present whose extents
	   include thie provoded point, we will update the horizontal with the
	   new values from the local grid file. */
	dtEntryPtr = CSselectRgf93ToNtf (__This,dtRgf93ToNtfTxt,llRgf93);
	if (dtEntryPtr != NULL)
	{
		/* Note that the following expects CSinverseRgf93ToNtfTxt to do
		   a reasonably good job at computing the height. */
		txtDatumPtr = dtEntryPtr->pointers.txtDatumPtr;
		status = CSinverseRgf93ToNtfTxt (txtDatumPtr,llRgf93,llNtf);
		if (status == 0)
		{
			/* If there are no local grids for this point, we're done. */
			dtEntryPtr = CSselectRgf93ToNtf (__This,dtRgf93ToNtfC2,llRgf93);
			if (dtEntryPtr != NULL)
			{
				/* There is a local grid.  We use the local grid to compute a
				   new horizontal position, leaving the height calculated above
				   as is.  Note that the horizontal position change here is on
				   the order of 10 centimeters.  Thus, the change in the height
				   due to this sloght horizontal shift is negligble. */
				caNtv2Ptr = dtEntryPtr->pointers.c2DatumPtr;
				status = CScalcDatumShiftCa2 (caNtv2Ptr,tmpLL,llNtf,NULL);
				if (status == 0)
				{
					llRgf93 [0] = tmpLL [0];
					llRgf93 [1] = tmpLL [1];
				}
			}
		}
	}
	else if (__This->fallback != NULL)
	{
		status = CScalcFallbackForward (__This->fallback,llRgf93,llNtf);
	}
	
	if (status == 1 || status < 0)
	{
		llRgf93 [LNG] = llNtf [LNG];
		llRgf93 [LAT] = llNtf [LAT];
		llRgf93 [HGT] = llNtf [HGT];
	}
	return status;
}
/******************************************************************************
	Release -- Release resources, but maintain the catalog status.
*/
void CSreleaseRgf93ToNtf (struct csRgf93ToNtf_* __This)
{
	struct csRgf93ToNtfEntry_* dtEntryPtr;

	if (__This != NULL)
	{
		if (__This->cachePtr != NULL)
		{
			CSreleaseLLGridCellCache (__This->cachePtr);
		}
		for (dtEntryPtr = __This->listHead;dtEntryPtr != NULL;dtEntryPtr = dtEntryPtr->next)
		{
			/* This releases resources associated with the entry.  However, the
			   entry remains in place so we don't have to parse the catalog
			   file again should we need this transformation in the future. */
			CSreleaseRgf93ToNtfEntry (dtEntryPtr);
		}
	}
	return;
}
/* This function returns the name of the datum shift file which was/would be
   used to convert the supplied point. */
Const char *CSsourceRgf93ToNtf (struct csRgf93ToNtf_* __This,Const double* llRgf93)
{
	int status;
	const char *cp;
	struct csRgf93ToNtfEntry_* dtEntryPtr;
	struct csRgf93ToNtfTxt_ *txtDatumPtr;

	double lclLlNtf [3];

	cp = NULL;
	dtEntryPtr = CSselectRgf93ToNtf (__This,dtRgf93ToNtfTxt,llRgf93);
	if (dtEntryPtr != NULL)
	{
		cp = CSsourceRgf93ToNtfEntry (dtEntryPtr,llRgf93);
		txtDatumPtr = dtEntryPtr->pointers.txtDatumPtr;
		status = CScalcRgf93ToNtfTxt (txtDatumPtr,lclLlNtf,llRgf93);
		if (status == 0)
		{
			dtEntryPtr = CSselectRgf93ToNtf (__This,dtRgf93ToNtfC2,lclLlNtf);
			if (dtEntryPtr != NULL)
			{
				cp = dtEntryPtr->pointers.c2DatumPtr->gridPtr->sourceId;
			}
		}
	}
	else if (__This->fallback != NULL)
	{
		cp = CSsourceFallback (__This->fallback);
	}
	return cp;
}
/******************************************************************************
	Constructor: This is a constructor for the "Entry" object.  A linked list
	of these "Entry" objects is underlying structure of the main object.

	NOTE: the specific of handling different file types is handled here.
*/
struct csRgf93ToNtfEntry_* CSnewRgf93ToNtfEntry (struct csDatumCatalogEntry_* catPtr)
{
	extern char cs_DirsepC;
	extern char cs_ExtsepC;
	extern char csErrnam [];

	char *cp;
	struct csRgf93ToNtfEntry_* __This;

	/* Prepare for an error. */
	__This = NULL;

	/* Allocate some storage. */
	__This = (struct csRgf93ToNtfEntry_*) CS_malc (sizeof (struct csRgf93ToNtfEntry_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->next = NULL;
	__This->type = dtRgf93ToNtfNone;
	__This->pointers.txtDatumPtr = NULL;
	__This->pointers.c2DatumPtr = NULL;         /* Yes, redundant! */

	/* Issue an error if information in the catalog file is
	   inconsistent.  That is, if the extension is not one of the
	   two supported. */
	cp = strrchr (catPtr->pathName,cs_DirsepC);
	if (cp == NULL)
	{
		/* Can't find a path specification in the path name. */
		CS_stncp (csErrnam,catPtr->pathName,MAXPATH);
		CS_erpt (cs_DTC_PATH);
		goto error;
	}
	cp = strchr (cp,cs_ExtsepC);
	if (cp == NULL || strlen (cp) != 4)
	{
		/* Can't find an extension in the file name. */
		CS_stncp (csErrnam,catPtr->pathName,MAXPATH);
		CS_erpt (cs_DTC_PATH);
		goto error;
	}
	

	/* Process the entry according to the extension on the file name. */
	cp += 1;                                /* bump past the separator*/
	if (!CS_stricmp (cp,"TXT"))
	{
		/* Construct a text file object.  In the past 10 years, there has only
		   one.  But with htis new implementation (Nov 2008) we supported
		   multiple .txt files as well as multiple .gsb files. */
		__This->pointers.txtDatumPtr = CSnewRgf93ToNtfTxt (catPtr->pathName,catPtr->bufferSize,catPtr->flags,catPtr->density);
		__This->type = dtRgf93ToNtfTxt;
	}
	else if (!CS_stricmp (cp,"GSB"))
	{
		/* Construct a Canadian National Transformation Version 2 object. */
		__This->pointers.c2DatumPtr = CSnewDatumShiftCa2 (dtcTypeCanadian2,catPtr->pathName,catPtr->bufferSize,catPtr->flags,catPtr->density);
		if (__This->pointers.c2DatumPtr == NULL)
		{
			goto error;
		}
		__This->type = dtRgf93ToNtfC2;
	}
	else
	{
		CS_stncp (csErrnam,catPtr->pathName,MAXPATH);
		CS_erpt (cs_NAD_EXT);
		goto error;
	}

	/* Return a pointer to t2he 'constructed' entry object. */
	return __This;
error:
	CSdeleteRgf93ToNtfEntry (__This);
	return NULL;
}

/******************************************************************************
	Destructor, for an "Entry" object.
*/
void CSdeleteRgf93ToNtfEntry (struct csRgf93ToNtfEntry_* __This)
{
	if (__This != NULL)
	{
		switch (__This->type) {
		case dtRgf93ToNtfTxt:
			CSdeleteRgf93ToNtfTxt (__This->pointers.txtDatumPtr);
			break;
		case dtRgf93ToNtfC2:
			CSdeleteDatumShiftCa2 (__This->pointers.c2DatumPtr);
			break;
		case dtRgf93ToNtfNone:
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
void CSreleaseRgf93ToNtfEntry (struct csRgf93ToNtfEntry_* __This)
{
	if (__This != NULL)
	{
		switch (__This->type){
		case dtRgf93ToNtfTxt:
			CSreleaseRgf93ToNtfTxt (__This->pointers.txtDatumPtr);
			break;
		case dtRgf93ToNtfC2:
			CSreleaseDatumShiftCa2 (__This->pointers.c2DatumPtr);
			break;
		case dtRgf93ToNtfNone:
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
double CStestRgf93ToNtfEntry (struct csRgf93ToNtfEntry_* __This,Const double* llRgf93)
{
	double rtnValue;

	switch (__This->type){
	case dtRgf93ToNtfTxt:
		rtnValue = CStestRgf93ToNtfTxt (__This->pointers.txtDatumPtr,llRgf93);
		break;
	case dtRgf93ToNtfC2:
		rtnValue = CStestDatumShiftCa2 (__This->pointers.c2DatumPtr,llRgf93);
		break;
	case dtRgf93ToNtfNone:
	default:
		rtnValue = 0.0;
		break;
	}
	return rtnValue;
}

/******************************************************************************
	Calculation function.  Calculates the conversion from RGF93 to NTF.
*/
int CScalcRgf93ToNtfEntry (struct csRgf93ToNtfEntry_* __This,double* llNtf,Const double *llRgf93,struct csLLGridCellCache_ *cachePtr)
{
	extern char csErrnam [];

	int status;

	switch (__This->type){
	case dtRgf93ToNtfTxt:
		status = CScalcRgf93ToNtfTxt (__This->pointers.txtDatumPtr,llNtf,llRgf93);
		break;
	case dtRgf93ToNtfC2:
		status = CScalcDatumShiftCa2 (__This->pointers.c2DatumPtr,llNtf,llRgf93,cachePtr);
		break;
	case dtRgf93ToNtfNone:
	default:
		/* Minus one indicates a fatal error.  In this case, it is an internal
		   software error. */
		CS_stncp (csErrnam,"CS_dtRgf93ToNtf:1",MAXPATH);
		CS_erpt (cs_ISER);
		status = -1;
		break;
	}
	return status;
}
/******************************************************************************
	Data source function.  Returns the data source file name.
*/
Const char *CSsourceRgf93ToNtfEntry (struct csRgf93ToNtfEntry_* __This,Const double *llRgf93)
{
	Const char *cp;

	switch (__This->type) {
	case dtRgf93ToNtfTxt:
		cp = CSsourceRgf93ToNtfTxt (__This->pointers.txtDatumPtr);
		break;
	case dtRgf93ToNtfC2:
		cp = CSsourceDatumShiftCa2 (__This->pointers.c2DatumPtr,llRgf93);
		break;
	case dtRgf93ToNtfNone:
	default:
		cp = NULL;
		break;
	}
	return cp;
}
