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

	US datum shift calculations, i.e. NAD27 <--> NAD83 and
	NAD83 <--> HPGN, require a system of two US grid files.
	The object defined and implemented here combines two
	csGridFileUS_ structures into a single object, and
	implements a datum shift conversion.  Such objects are
	used to perform these datum shifts.
*/

#include "cs_map.h"

/******************************************************************************
	Constructor
*/
struct csDatumShiftUS_* CSnewDatumShiftUS (enum csDtCvtType type,Const char *path,long32_t bufferSize,ulong32_t flags,double density)
{
	extern char cs_DirsepC;
	extern char cs_ExtsepC;

	/* Do the .LAS, then the .LOS.  Note, this is a datum shift object,
	   so there will always be two of these. */

	char *cpFront;
	char *cpBack;
	struct csDatumShiftUS_ *__This = NULL;
	char ctemp [MAXPATH];

	__This = (struct csDatumShiftUS_ *)CS_malc (sizeof (struct csDatumShiftUS_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->lngShift = NULL;
	__This->latShift = NULL;
	__This->type = type;

	CS_stncp (ctemp,path,sizeof (ctemp));
	cpFront = strrchr (ctemp,cs_DirsepC);
	if (cpFront == NULL) cpFront = ctemp;
	else cpFront += 1;

	/* Do the .LOS file */
	cpBack = strrchr (cpFront,cs_ExtsepC);
	if (cpBack == NULL) cpBack = cpFront + strlen (cpFront);
	*cpBack++ = cs_ExtsepC;
	*cpBack = '\0';													/*lint !e661 */
	strcat (ctemp,cs_NADCON_LOS);
	__This->lngShift = CSnewGridFileUS (ctemp,bufferSize,flags,density);
	if (__This->lngShift == NULL) goto error;

	/* Do the .LAS file */
	cpBack = strrchr (cpFront,cs_ExtsepC);
	if (cpBack == NULL) cpBack = cpFront + strlen (cpFront);
	*cpBack++ = cs_ExtsepC;
	*cpBack = '\0';													/*lint !e661 */
	strcat (ctemp,cs_NADCON_LAS);
	__This->latShift = CSnewGridFileUS (ctemp,bufferSize,flags,density);
	if (__This->latShift == NULL) goto error;

	/* Verify that the two files are consistent. */
	if (__This->lngShift->coverage.southWest [0] != __This->latShift->coverage.southWest [0] ||		/*lint !e777 */
		__This->lngShift->coverage.southWest [1] != __This->latShift->coverage.southWest [1] ||		/*lint !e777 */
		__This->lngShift->coverage.northEast [0] != __This->latShift->coverage.northEast [0] ||		/*lint !e777 */
		__This->lngShift->coverage.northEast [1] != __This->latShift->coverage.northEast [1] ||		/*lint !e777 */
		__This->lngShift->elementCount           != __This->latShift->elementCount ||
		__This->lngShift->recordCount            != __This->latShift->recordCount ||
		__This->lngShift->deltaLng               != __This->latShift->deltaLng ||					/*lint !e777 */
		__This->lngShift->deltaLat               != __This->latShift->deltaLat)						/*lint !e777 */
	{
		CS_erpt (cs_NADCON_CONS);
		goto error;
	}
		
	return __This;
error:
	if (__This != NULL)
	{
		if (__This->lngShift != NULL) CSdeleteGridFileUS (__This->lngShift);
		if (__This->latShift != NULL) CSdeleteGridFileUS (__This->latShift);
		CS_free (__This);
	}
	return NULL;
}

/******************************************************************************
	Destructor
*/
void CSdeleteDatumShiftUS (struct csDatumShiftUS_* __This)
{
	if (__This != NULL)
	{
		if (__This->lngShift != NULL) CSdeleteGridFileUS (__This->lngShift);
		if (__This->latShift != NULL) CSdeleteGridFileUS (__This->latShift);
		CS_free (__This);
	}
}

/******************************************************************************
	Release resource
*/
void CSreleaseDatumShiftUS (struct csDatumShiftUS_* __This)
{
	if (__This != NULL)
	{
		if (__This->lngShift != NULL) CSreleaseGridFileUS (__This->lngShift);
		if (__This->latShift != NULL) CSreleaseGridFileUS (__This->latShift);
	}
}

/******************************************************************************
	Select a specific "Entry" from the linked list of "Entry" objects.
*/
double CStestDatumShiftUS (struct csDatumShiftUS_* __This,Const double *coord)
{
	if (__This->lngShift == NULL) return 0.0;
	return CStestGridFileUS (__This->lngShift,coord);
}

/******************************************************************************
	Calculate the shift.
*/
int CScalcDatumShiftUS (struct csDatumShiftUS_* __This,double* result,Const double* source,struct csLLGridCellCache_ *cachePtr)
{
	extern double cs_Sec2Deg;			/* 1.0 / 3600.0 */

	int status;
	double deltaLng;
	double deltaLat;

	/* Here we simply shift the longitude using lngShift, and the latitude
	   using latShift. */
	status = CScalcGridFileUS (__This->lngShift,&deltaLng,source);
	if (status == 0)
	{
		status = CScalcGridFileUS (__This->latShift,&deltaLat,source);
		if (status == 0)
		{
			result [LNG] = source [LNG] - deltaLng * cs_Sec2Deg;
			result [LAT] = source [LAT] + deltaLat * cs_Sec2Deg;

			/* If cachePtr is not NULL, we add the current cell to the
			   cache.  If we find situations where cacheing a cell is
			   inappropriate, add that code here. */
			if (cachePtr != NULL)
			{
				CSaddLLGridCellCache (cachePtr,__This->type,&__This->lngShift->currentCell,&__This->latShift->currentCell);
			}
		}
	}
	if (status > 0)
	{
		result [LNG] = source [LNG];
		result [LAT] = source [LAT];
	}
	return status;
}
Const char *CSsourceDatumShiftUS (struct csDatumShiftUS_* __This,Const double llSource [2])
{
	Const char *cp;

	cp = CSsourceGridFileUS (__This->lngShift,llSource);
	return cp;
}
