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

struct csLLGridCellCache_* CSnewLLGridCellCache (int cellCount)
{
	struct csLLGridCellCache_* cachePtr;

	if (cellCount < 2)
	{
		CS_erpt (cs_ISER);
		return NULL;
	}
	cachePtr = (struct csLLGridCellCache_ *)CS_malc (sizeof (struct csLLGridCellCache_));
	if (cachePtr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		return cachePtr;
	}
	cachePtr->maxCount = cellCount;
	cachePtr->usedCount = 0;
	cachePtr->listHead = NULL;
	return cachePtr;
}
void CSdeleteLLGridCellCache (struct csLLGridCellCache_* __This)
{
	if (__This != NULL)
	{
		CSreleaseLLGridCellCache (__This);
		CS_free (__This);
	}
}
void CSreleaseLLGridCellCache (struct csLLGridCellCache_* __This)
{
	struct csLLGridCell_* cellPtr;
	struct csLLGridCell_* tmpPtr;

	if (__This != NULL)
	{
		cellPtr = __This->listHead;
		while (cellPtr != NULL)
		{
			tmpPtr = cellPtr;
			cellPtr = cellPtr->next;
			CS_free (tmpPtr);
		}
		__This->listHead = NULL;
		__This->usedCount = 0;
	}
	return;
}
int CSaddLLGridCellCache (struct csLLGridCellCache_* __This,enum csDtCvtType cellType,
															struct csGridCell_* lngCell,
															struct csGridCell_* latCell)
{
	extern char csErrnam [];

	struct csLLGridCell_ *cellPtr, *prvCellPtr;

	/* Adjust the cache so that the first entry is suitable for the
	   new addition. */
	if (__This->usedCount < __This->maxCount)
	{
		/* Here to add a complete new cell to the cache. */
		cellPtr = (struct csLLGridCell_*)CS_malc (sizeof (struct csLLGridCell_));
		if (cellPtr == NULL)
		{
			CS_erpt (cs_NO_MEM);
			return -1;
		}
		if (__This->listHead == NULL) cellPtr->next = NULL;
		else cellPtr->next = __This->listHead;
		__This->listHead = cellPtr;
		__This->usedCount += 1;
	}
	else
	{
		/* Here to reuse an existing grid cell. Move the last one in the
		   cache up to the front, then use it. */
		cellPtr = __This->listHead;
		prvCellPtr = NULL;
		while (cellPtr->next != NULL)
		{
			prvCellPtr = cellPtr;
			cellPtr = cellPtr->next;
		}
		if (prvCellPtr == NULL)
		{
			CS_stncp (csErrnam,"CSgridCellCache:1",MAXPATH);
			CS_erpt (cs_ISER);
			return -1;
		}
		prvCellPtr->next = NULL;
		cellPtr->next = __This->listHead;
		__This->listHead = cellPtr;
	}

	/* Cell pointer points to the first cell in the list; which
	   is also the target of the add. */
	cellPtr->type = cellType;
	cellPtr->southWest [LNG] = lngCell->coverage.southWest [LNG];
	cellPtr->southWest [LAT] = lngCell->coverage.southWest [LAT];
	cellPtr->northEast [LNG] = lngCell->coverage.northEast [LNG];
	cellPtr->northEast [LAT] = lngCell->coverage.northEast [LAT];

	cellPtr->deltaLng = lngCell->deltaLng;
	cellPtr->deltaLat = latCell->deltaLat;

	cellPtr->AA [0] = lngCell->currentAA;
	cellPtr->BB [0] = lngCell->currentBB;
	cellPtr->CC [0] = lngCell->currentCC;
	cellPtr->DD [0] = lngCell->currentDD;

	cellPtr->AA [1] = latCell->currentAA;
	cellPtr->BB [1] = latCell->currentBB;
	cellPtr->CC [1] = latCell->currentCC;
	cellPtr->DD [1] = latCell->currentDD;

	CS_stncp (cellPtr->sourceId,lngCell->sourceId,sizeof (cellPtr->sourceId));
	return 0;
}
void CSfirstLLGridCellCache (struct csLLGridCellCache_* __This,struct csLLGridCell_* cellPtr)
{
	struct csLLGridCell_ *srchPtr;

	/* This is tsurctured so that if cellPtr is the same as listHead,
	   nothing happens. */
	srchPtr = __This->listHead;
	while (srchPtr != NULL)
	{
		if (srchPtr->next == cellPtr)
		{
			/* We have found the cell. */
			srchPtr->next = srchPtr->next->next;
			cellPtr->next = __This->listHead;
			__This->listHead = cellPtr;
			break;
		}
		srchPtr = srchPtr->next;
	}
	return;
}

int CScalcLLGridCellCache (struct csLLGridCellCache_* __This,double trgLL [2],Const double srcLL [2])
{
	extern double cs_Sec2Deg;			/* 1.0 / 3600.0 */
	extern double cs_K360;

	int status;
	int alaskaFlag;
	struct csLLGridCell_* cellPtr;
	double gridDeltaLng, gridDeltaLat;
	double deltaLng, deltaLat;

	double lclLL [2];

	status = 1;
	alaskaFlag = FALSE;

	/* Search the cache. */
	cellPtr = __This->listHead;
	while (cellPtr != NULL)
	{
		/* For now, we can accomodate all four cases rather
		   easily. */
		switch (cellPtr->type) {
		case dtcTypeUS:
		case dtcTypeHarn:
			lclLL [LNG] = srcLL [LNG];
			lclLL [LAT] = srcLL [LAT];

			/* Need to play a game to properly convert Alaska longitude.
			   The grid cell may be set up with a longitude < -180. */
			if (lclLL [LNG] > 0.0)
			{
				lclLL [LNG] -= cs_K360;
				alaskaFlag = TRUE;
			}

			/* See if this cell covers the coordinate we're converting. */
			if (lclLL [LNG] >= cellPtr->southWest [LNG] &&
				lclLL [LAT] >= cellPtr->southWest [LAT] &&
				lclLL [LNG] <  cellPtr->northEast [LNG] &&
				lclLL [LAT] <  cellPtr->northEast [LAT])
			{
				/* Do the calculation. */
				gridDeltaLng = (lclLL [LNG] - cellPtr->southWest [LNG]) / cellPtr->deltaLng;
				gridDeltaLat = (lclLL [LAT] - cellPtr->southWest [LAT]) / cellPtr->deltaLat;
				deltaLng = cellPtr->AA [0] +
						   cellPtr->BB [0] * gridDeltaLng +
						   cellPtr->CC [0] * gridDeltaLat +
						   cellPtr->DD [0] * gridDeltaLng * gridDeltaLat;
				deltaLat = cellPtr->AA [1] +
						   cellPtr->BB [1] * gridDeltaLng +
						   cellPtr->CC [1] * gridDeltaLat +
						   cellPtr->DD [1] * gridDeltaLng * gridDeltaLat;
				trgLL [LNG] = lclLL [LNG] - deltaLng * cs_Sec2Deg;
				trgLL [LAT] = lclLL [LAT] + deltaLat * cs_Sec2Deg;

				/* Undo the Alaska thing. */
				if (alaskaFlag)
				{
					trgLL [LNG] += cs_K360;
				}

				/* Signal conversion successfully performed. */
				status = 0;
			}
			break;

		case dtcTypeAustralian:
		case dtcTypeCanadian1:
		case dtcTypeCanadian2:
			/* Similar, but different stuff for these guys.  In this case,
			   longitude is west positive, even for the Australians.  Thus,
			   the longitude code will seem strange.  Think of Canada being
			   in Russia, and Australia being in South America to keep your
			   mind straight. */
			lclLL [LNG] = -srcLL [LNG];
			lclLL [LAT] = srcLL [LAT];
			if (lclLL [LNG] >= cellPtr->southWest [LNG] &&
				lclLL [LAT] >= cellPtr->southWest [LAT] &&
				lclLL [LNG] <  cellPtr->northEast [LNG] &&
				lclLL [LAT] <  cellPtr->northEast [LAT])
			{
				/* Do the calculation. */
				gridDeltaLng = (lclLL [LNG] - cellPtr->southWest [LNG]) / cellPtr->deltaLng;
				gridDeltaLat = (lclLL [LAT] - cellPtr->southWest [LAT]) / cellPtr->deltaLat;
				deltaLng = cellPtr->AA [0] +
						   cellPtr->BB [0] * gridDeltaLng +
						   cellPtr->CC [0] * gridDeltaLat +
						   cellPtr->DD [0] * gridDeltaLng * gridDeltaLat;
				deltaLat = cellPtr->AA [1] +
						   cellPtr->BB [1] * gridDeltaLng +
						   cellPtr->CC [1] * gridDeltaLat +
						   cellPtr->DD [1] * gridDeltaLng * gridDeltaLat;
				trgLL [LNG] = srcLL [LNG] - deltaLng * cs_Sec2Deg;
				trgLL [LAT] = srcLL [LAT] + deltaLat * cs_Sec2Deg;

				/* Signal conversion successfully performed. */
				status = 0;
			}
			break;

		default:
			/* Just in case. */
			CS_erpt (cs_ISER);
			status = -1;
			break;
		}									/*lint !e788 */

		/* If we did the calculation, we're outa here. */
		if (status > 0)
		{
			cellPtr = cellPtr->next;
		}
		else
		{
			if (status == 0 && cellPtr != __This->listHead)
			{
				/* Make this the first cell. */
				CSfirstLLGridCellCache (__This,cellPtr);
			}
			break;
		}
	}
	return status;
}
Const char *CSsourceLLGridCellCache (struct csLLGridCellCache_* __This,Const double srcLL [2])
{
	extern double cs_K360;

	int status;
	int alaskaFlag;

	Const char *cp;
	struct csLLGridCell_* cellPtr;

	double lclLL [2];

	status = 1;
	cp = NULL;
	alaskaFlag = FALSE;

	/* Search the cache. */
	cellPtr = __This->listHead;
	while (cellPtr != NULL)
	{
		/* For now, we can accomodate all four cases rather
		   easily. */
		switch (cellPtr->type) {
		case dtcTypeUS:
		case dtcTypeHarn:
			lclLL [LNG] = srcLL [LNG];
			lclLL [LAT] = srcLL [LAT];

			/* Need to play a game to properly convert Alaska longitude.
			   The grid cell may be set up with a longitude < -180. */
			if (lclLL [LNG] > 0.0)
			{
				lclLL [LNG] -= cs_K360;
				alaskaFlag = TRUE;
			}

			/* See if this cell covers the coordinate we're converting. */
			if (lclLL [LNG] >= cellPtr->southWest [LNG] &&
				lclLL [LAT] >= cellPtr->southWest [LAT] &&
				lclLL [LNG] <  cellPtr->northEast [LNG] &&
				lclLL [LAT] <  cellPtr->northEast [LAT])
			{
				cp = cellPtr->sourceId;
				status = 0;
			}
			break;

		case dtcTypeAustralian:
		case dtcTypeCanadian1:
		case dtcTypeCanadian2:
			/* Similar, but different stuff for these guys.  In this case,
			   longitude is west positive, even for the Australians.  Thus,
			   the longitude code will seem strange.  Think of Canada being
			   in Russia, and Australia being in South America to keep your
			   mind straight. */
			lclLL [LNG] = -srcLL [LNG];
			lclLL [LAT] = srcLL [LAT];
			if (lclLL [LNG] >= cellPtr->southWest [LNG] &&
				lclLL [LAT] >= cellPtr->southWest [LAT] &&
				lclLL [LNG] <  cellPtr->northEast [LNG] &&
				lclLL [LAT] <  cellPtr->northEast [LAT])
			{
				cp = cellPtr->sourceId;
				status = 0;
			}
			break;

		default:
			/* Just in case. */
			CS_erpt (cs_ISER);
			status = -1;
			break;
		}									/*lint !e788 */

		/* If we did the calculation, we're outa here. */
		if (status > 0)
		{
			cellPtr = cellPtr->next;
		}
		else
		{
			break;
		}
	}
	return cp;
}																/*lint !e550 */

/******************************************************************************
	Single dimension grid cell cache.
*/
struct csZGridCellCache_* CSnewZGridCellCache (int cellCount)
{
	struct csZGridCellCache_* cachePtr;

	if (cellCount < 2)
	{
		CS_erpt (cs_ISER);
		return NULL;
	}
	cachePtr = (struct csZGridCellCache_ *)CS_malc (sizeof (struct csZGridCellCache_));
	if (cachePtr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		return cachePtr;
	}
	cachePtr->maxCount = cellCount;
	cachePtr->usedCount = 0;
	cachePtr->listHead = NULL;
	return cachePtr;
}
void CSdeleteZGridCellCache (struct csZGridCellCache_* __This)
{
	if (__This != NULL)
	{
		CSreleaseZGridCellCache (__This);
		CS_free (__This);
	}
}
void CSreleaseZGridCellCache (struct csZGridCellCache_* __This)
{
	struct csZGridCell_* cellPtr;
	struct csZGridCell_* tmpPtr;

	if (__This != NULL)
	{
		cellPtr = __This->listHead;
		while (cellPtr != NULL)
		{
			tmpPtr = cellPtr;
			cellPtr = cellPtr->next;
			CS_free (tmpPtr);
		}
		__This->usedCount = 0;
	}
	return;
}

int CSaddZGridCellCache (struct csZGridCellCache_* __This,enum csDtCvtType cellType,
														  struct csGridCell_* zzzCell)
{
	extern char csErrnam [];

	struct csZGridCell_ *cellPtr, *prvCellPtr;

	/* Adjust the cache so that the first entry is suitable for the
	   new addition. */
	if (__This->usedCount < __This->maxCount)
	{
		/* Here to add a complete new cell to the cache. */
		cellPtr = (struct csZGridCell_*)CS_malc (sizeof (struct csZGridCell_));
		if (cellPtr == NULL)
		{
			CS_erpt (cs_NO_MEM);
			return -1;
		}
		if (__This->listHead == NULL) cellPtr->next = NULL;
		else cellPtr->next = __This->listHead;
		__This->listHead = cellPtr;
		__This->usedCount += 1;
	}
	else
	{
		/* Here to reuse an existing grid cell. */
		cellPtr = __This->listHead;
		prvCellPtr = NULL;
		while (cellPtr->next != NULL)
		{
			prvCellPtr = cellPtr;
			cellPtr = cellPtr->next;
		}
		if (prvCellPtr == NULL)
		{
			CS_stncp (csErrnam,"CSgridCellCache:2",MAXPATH);
			CS_erpt (cs_ISER);
			return -1;
		}
		prvCellPtr->next = NULL;
		cellPtr->next = __This->listHead;
		__This->listHead = cellPtr;
	}

	/* Cell pointer points to the first cell in the list; which
	   is also the target of the add. */
	cellPtr->type = cellType;
	cellPtr->southWest [LNG] = zzzCell->coverage.southWest [LNG];
	cellPtr->southWest [LAT] = zzzCell->coverage.southWest [LAT];
	cellPtr->northEast [LNG] = zzzCell->coverage.northEast [LNG];
	cellPtr->northEast [LAT] = zzzCell->coverage.northEast [LAT];

	cellPtr->deltaLng = zzzCell->deltaLng;
	cellPtr->deltaLat = zzzCell->deltaLat;

	cellPtr->AA = zzzCell->currentAA;
	cellPtr->BB = zzzCell->currentBB;
	cellPtr->CC = zzzCell->currentCC;
	cellPtr->DD = zzzCell->currentDD;
	return 0;
}
void CSfirstZGridCellCache (struct csZGridCellCache_* __This,struct csZGridCell_* cellPtr)
{
	struct csZGridCell_ *srchPtr;

	/* This is structured so that if cellPtr is the same as listHead,
	   nothing happens. */
	srchPtr = __This->listHead;
	while (srchPtr != NULL)
	{
		if (srchPtr->next == cellPtr)
		{
			/* We have found the cell. */
			srchPtr->next = srchPtr->next->next;
			cellPtr->next = __This->listHead;
			__This->listHead = cellPtr;
			break;
		}
		srchPtr = srchPtr->next;
	}
	return;
}

int CScalcZGridCellCache (struct csZGridCellCache_* __This,double* target,Const double srcLL [2])
{
	extern double cs_K360;

	int status;
	struct csZGridCell_* cellPtr;
	double gridDeltaLng, gridDeltaLat;

	double lclLL [2];

	status = 1;

	/* Search the cache. */
	cellPtr = __This->listHead;
	while (cellPtr != NULL)
	{
		/* For now, we can accomodate all four cases rather
		   easily. */
		switch (cellPtr->type) {
		case dtcTypeGeoid96:
		case dtcTypeUSVertcon:
			lclLL [LNG] = srcLL [LNG];
			lclLL [LAT] = srcLL [LAT];

			/* Need to play a game to properly convert Alaska longitude.
			   The grid cell may be set up with a longitude < -180. */
			if (lclLL [LNG] > 0.0)
			{
				lclLL [LNG] -= cs_K360;
			}

			/* See if this cell covers the coordinate we're converting. */
			if (lclLL [LNG] >= cellPtr->southWest [LNG] &&
				lclLL [LAT] >= cellPtr->southWest [LAT] &&
				lclLL [LNG] <  cellPtr->northEast [LNG] &&
				lclLL [LAT] <  cellPtr->northEast [LAT])
			{
				/* Do the calculation. */
				gridDeltaLng = (lclLL [LNG] - cellPtr->southWest [LNG]) / cellPtr->deltaLng;
				gridDeltaLat = (lclLL [LAT] - cellPtr->southWest [LAT]) / cellPtr->deltaLat;
				*target = cellPtr->AA +
						  cellPtr->BB * gridDeltaLng +
						  cellPtr->CC * gridDeltaLat +
						  cellPtr->DD * gridDeltaLng * gridDeltaLat;

				/* Signal conversion successfully performed. */
				status = 0;
			}
			break;

		default:
			/* Just in case. */
			CS_erpt (cs_ISER);
			status = -1;
			break;
		}								/*lint !e788 */

		/* If we did the calculation, we're outa here. */
		if (status > 0)
		{
			cellPtr = cellPtr->next;
		}
		else
		{
			if (status == 0 && cellPtr != __This->listHead)
			{
				/* Make this the first cell. */
				CSfirstZGridCellCache (__This,cellPtr);
			}
			break;
		}
	}
	return status;
}
