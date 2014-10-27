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

	This object encapsulates the functionality of a Version 2
	Canadian National Transformation grid data file.  The
	Australians have adopted this basic format, but there are
	some variations.  These variations are supported by this
	code, the code senses the difference automatically.

	struct csGridFileCa2_* __This = TcsCaNTv2Construct (char *fileName);
	int TcsCaNTv2Forward (__This,double* result,double* original);
	double TcsCaNTv2CheckCoverage (__This,double* point);
	int TcsCaVTv2Inverse (__This,double* result,double* original);
	void TcsCaNTv2Destruct (__This);
*/
/*lint -esym(715,flags) */			/* parameter not referenced */
/*lint -e514 */						/* Lint doesn't like using |= when the r-value is boolean */

#include "cs_map.h"

/* The following is the minimum buffer size.  Can be any value, even zero.
   The value provided will provide reasoable performance. */

int csGridFileCa2BufrSz = 32768;

/* The following table defines two regions where cells cannot be cached
   due to a bust in the file format.  That is, the specification says
   that each sub-grid shall completely cover all grid cells in the
   parent which it covers.  I.e. no partial coverage.  For the two
   cells indicated, this is not the case.  Thus, we can't allow any of
   the parent cells in the cache. */

static double csKludgeTable [3][4] =
{	/*  SW Long         SW Lat       NE Long        NE Lat     */
	{ -135.33333334, 60.50000000, -134.75000000, 61.00000000},	/* Whitehorse */
	{ -132.83333333, 61.75000000, -132.16666667, 62.33333334},	/* Rossri */
	{           0.0,         0.0,           0.0,         0.0}	/* Marks end of table */
};

/******************************************************************************
	Constructor; can't have one of these things without a data file.
	Returns NULL in the event of a failure.  Currently, there is no use
	of the flags value.
*/
struct csGridFileCa2_* CSnewGridFileCa2 (enum csDtCvtType type,Const char *filePath,long32_t bufferSize,ulong32_t flags,double density)
{
	extern double cs_Sec2Deg;
	extern char cs_DirsepC;
	extern char csErrnam [];

	short idx;
	short parIdx;

	int overlap;
	int seekStat;

	size_t readCnt;
	size_t readCntRq;
	size_t malcCnt;
	long32_t skipAmount;

	char *cp;
	struct csGridFileCa2SubGrid_* subPtr;
	struct csGridFileCa2SubGrid_* kidPtr;
	struct csGridFileCa2SubGrid_* parPtr;
	struct csGridFileCa2_* __This = NULL;

	union csGridFileCa2Hdr fileHdr;
	struct csGridFileCa2SubHdr fileSubHdr;

	char ctemp [MAXPATH];

	/* In the event of an error; this eliminates dupliating this
	   many many times. */
	CS_stncp (csErrnam,filePath,MAXPATH);

	/* Allocate and initialize the basic structure. */
	__This = (struct csGridFileCa2_ *)CS_malc (sizeof (struct csGridFileCa2_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->SubGridDir = NULL;
	__This->Stream = NULL;
	__This->HdrRecCnt = 0;
	__This->SubCount = 0;
	__This->RecSize = 16;
	__This->CellIsValid = FALSE;
	__This->SubOverlap = (short)((flags & 0x01) != 0);
	__This->ExtType = type;
	__This->IntType = csCaNTv2TypeNone;
	__This->BufferSize = bufferSize;
	__This->sourceId [0] = '\0';
	if (__This->BufferSize <= 0) __This->BufferSize = csGridFileCa2BufrSz;
	if (__This->BufferSize <= 4096) __This->BufferSize = 4096;
	CSinitGridCell (&__This->longitudeCell);
	CSinitGridCell (&__This->latitudeCell);
	
	/* Deal with the file path. */
	CS_stncp (__This->FilePath,filePath,sizeof (__This->FilePath));

	/* Extract and save last 15 characters of the data file name. */
	cp = strrchr (__This->FilePath,cs_DirsepC);
	if (cp == NULL) cp = __This->FilePath;
	CS_stncp (ctemp,cp,sizeof (ctemp));
	cp = strrchr (ctemp,'.');
	if (cp != NULL) *cp = '\0';
	cp = ctemp;
	if (strlen (ctemp) > 15)
	{
		cp = ctemp + strlen (ctemp) - 15;
	}
	CS_stncp (__This->FileName,cp,sizeof (__This->FileName));

	/* Open the file. */
	__This->Stream = CS_fopen (__This->FilePath,_STRM_BINRD);
	if (__This->Stream == NULL)
	{
		CS_erpt (cs_DTC_FILE);
		goto error;
	}
	setvbuf (__This->Stream,NULL,_IOFBF,(size_t)__This->BufferSize);

	/* We've got a file.  Read the header. */
	readCnt = CS_fread (&fileHdr,1,sizeof (fileHdr),__This->Stream);
	if (CS_ferror (__This->Stream))
	{
		CS_erpt (cs_IOERR);
		goto error;
	}
	if (readCnt != sizeof (fileHdr))
	{
		CS_erpt (cs_INV_FILE);
		goto error;
	}

	/* Verify that this is the kind of file we know how to deal with. */
	if (strncmp (fileHdr.Canadian.titl01,"NUM_OREC",8))
	{
		/* Opps!!! Not a CaNTv2 file. */
		CS_erpt (cs_INV_FILE);
		goto error;
	}

	/* Determine the type/source of the file. */
	if (fileHdr.Canadian.titl02 [0] == 'N' &&
	    fileHdr.Canadian.titl02 [1] == 'U')
	{
		/* It appears that the file is Canadian. */
		__This->IntType = csCaNTv2TypeCanada;
		skipAmount = sizeof (struct csGridFileCa2HdrCA);
		CS_bswap (&fileHdr.Canadian,cs_BSWP_GridFileCa2HdrCA);
	}
	else if (fileHdr.Australian.titl02 [0] == 'N' &&
			 fileHdr.Australian.titl02 [1] == 'U')
	{
		/* It appears to be an Australian file. */
		__This->IntType = csCaNTv2TypeAustralia;
		skipAmount = sizeof (struct csGridFileCa2HdrAU);
		CS_bswap (&fileHdr.Australian,cs_BSWP_GridFileCa2HdrAU);
	}
	else
	{
		/* Opps!!! Don't know what kind of file it is. */
		CS_erpt (cs_INV_FILE);
		goto error;
	}

	/* Reposition the input file as is appropriate due to the
	   type of file.  A little hoeky, but it should be portable. */
	seekStat = CS_fseek (__This->Stream,skipAmount,SEEK_SET);
	if (seekStat != 0)
	{
		CS_erpt (cs_INV_FILE);
		goto error;
	}

	/* Extract the valuable stuff. */
	if (__This->IntType == csCaNTv2TypeCanada)
	{
		__This->HdrRecCnt = fileHdr.Canadian.num_orec;
		__This->SubCount = fileHdr.Canadian.num_file;
		__This->SubHdrRecCnt = fileHdr.Canadian.num_srec;
	}
	else
	{
		__This->HdrRecCnt = fileHdr.Australian.num_orec;
		__This->SubCount = fileHdr.Australian.num_file;
		__This->SubHdrRecCnt = fileHdr.Australian.num_srec;
	}

	/* Now, we deal with the sub-directories. */
	malcCnt = sizeof (struct csGridFileCa2SubHdr) * (ulong32_t)__This->SubCount;
	__This->SubGridDir = (struct csGridFileCa2SubGrid_ *)CS_malc (malcCnt);
	if (__This->SubGridDir == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}

	/* Initialize (i.e. construct) each of the sub-grid items we just
	   allocated. */
	for (idx = 0;idx < __This->SubCount;idx += 1)
	{
		subPtr = &__This->SubGridDir [idx];

		/* Initialize to a boundary which will not match anything. */
		subPtr->SouthWest [LNG] = 180.0;
		subPtr->SouthWest [LAT] = 90.0;
		subPtr->NorthEast [LNG] = -180.0;
		subPtr->NorthEast [LAT] = -90.0;

		/* Remeber, these values are WEST positive. */
		subPtr->SeReference [LNG] =  180.0;
		subPtr->SeReference [LAT] =   90.0;
		subPtr->NwReference [LNG] = -180.0;
		subPtr->NwReference [LAT] =  -90.0;

		subPtr->DeltaLng  = 0.0;
		subPtr->DeltaLat  = 0.0;
		subPtr->Density   = 0.0;
		subPtr->FirstRecord = -1;
		subPtr->GridRecCnt = 0;
		subPtr->ParentIndex = -1;
		subPtr->ChildIndex = -1;
		subPtr->RowCount = 0;
		subPtr->ElementCount = 0;
		subPtr->RowSize = 0;
		subPtr->Cacheable = FALSE;
		subPtr->Name [0] = '\0';
		subPtr->Parent [0] = '\0';
	}

	/* Once for each sub-grid in the file; read in the header.  At this point,
	   we just read them in.  Later on, we peruse the array and figure out
	   who the mamas and the papas are. */
	for (idx = 0;idx < __This->SubCount;idx += 1)
	{
		/* Kludge to handle the variation in format.  Doing this right
		   would require duplication of a whole bunch of code. So . . . */
		readCntRq = sizeof (fileSubHdr);
		if (__This->IntType == csCaNTv2TypeAustralia) readCntRq -= 4;

		readCnt = CS_fread (&fileSubHdr,1,readCntRq,__This->Stream);
		if (CS_ferror (__This->Stream))
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
		if (readCnt != readCntRq)
		{
			CS_erpt (cs_INV_FILE);
			goto error;
		}
		if (strncmp (fileSubHdr.titl01,"SUB_NAME",8))
		{
			CS_erpt (cs_INV_FILE);
			goto error;
		}
		if (__This->IntType == csCaNTv2TypeCanada)
		{
			CS_bswap (&fileSubHdr,cs_BSWP_GridFileCa2SubHdrCA);
		}
		else
		{
			CS_bswap (&fileSubHdr,cs_BSWP_GridFileCa2SubHdrAU);
		}
		
		/* Collect the useful stuff. */		
		subPtr = &__This->SubGridDir [idx];

		/* Data for each sub-grid immediately follows the sub-grid header. */
		subPtr->FirstRecord = CS_ftell (__This->Stream);

		/* These boundaries are rational east positive boundaries. */
		subPtr->SouthWest [LNG] = -fileSubHdr.w_long * cs_Sec2Deg;
		subPtr->SouthWest [LAT] =  fileSubHdr.s_lat * cs_Sec2Deg;
		subPtr->NorthEast [LNG] = -fileSubHdr.e_long * cs_Sec2Deg;
		subPtr->NorthEast [LAT] =  fileSubHdr.n_lat * cs_Sec2Deg;

		/* These boundaries are the screwy west positive ones the
		   Canadians use. */
		subPtr->SeReference [LNG] = fileSubHdr.e_long * cs_Sec2Deg;
		subPtr->SeReference [LAT] = fileSubHdr.s_lat * cs_Sec2Deg;
		subPtr->NwReference [LNG] = fileSubHdr.w_long * cs_Sec2Deg;
		subPtr->NwReference [LAT] = fileSubHdr.n_lat * cs_Sec2Deg;

		/* The remainder of this is pretty rational. */
		subPtr->DeltaLng  = fileSubHdr.long_inc * cs_Sec2Deg;
		subPtr->DeltaLat  = fileSubHdr.lat_inc * cs_Sec2Deg;
		subPtr->Density   = subPtr->DeltaLng;
		if (subPtr->DeltaLat < subPtr->DeltaLng) subPtr->Density = subPtr->DeltaLat;

		/* If the user has specified a default density value, we use it. */
		if (density != 0.0)
		{
			subPtr->Density = density;
		}

		/* Save the name for reporting purposes. */
		CS_stncp (subPtr->Name,fileSubHdr.sub_name,9);
		CS_stncp (subPtr->Parent,fileSubHdr.parent,9);

		subPtr->GridRecCnt = fileSubHdr.gs_count;
		/* WEST Positive, dummy.  The extra .1 is to eliminate possible fuzz in the
		   double potion of the calculations. */
		subPtr->RowCount = (unsigned short)(((subPtr->NwReference [LAT] - subPtr->SeReference [LAT]) / subPtr->DeltaLat) + 1.01);
		subPtr->ElementCount = (unsigned short)(((subPtr->NwReference [LNG] - subPtr->SeReference [LNG]) / subPtr->DeltaLng) + 1.01);
		subPtr->RowSize = (unsigned short)(subPtr->ElementCount * __This->RecSize);

		/* Certain sub grids are not cacheable.  In the Canadian file, the region
		   which is not cacheable is rather small.  We use the csKludgeTable
		   to handle it.  The one Austrailian sub-grid we've seen is screwed up,
		   so we disable cacheing (at least for now), for all Australian files.
		   Australian, in this context, means file in the old Australian format,
		   not necessarily data files covering Australian geography.
		   
		   In the case of the Spanish variation, grids overlap, and therefore none
		   of the sub-grids are cacheable. */
		subPtr->Cacheable = (short)((__This->IntType == csCaNTv2TypeCanada) && (__This->SubOverlap == 0));

		/* Skip over the data records in the file. */
		skipAmount = subPtr->GridRecCnt * __This->RecSize;
		seekStat = CS_fseek (__This->Stream,skipAmount,SEEK_CUR);
		if (seekStat != 0)
		{
			CS_erpt (cs_INV_FILE);
			goto error;
		}
	}

	/* Now we figure out who the mammas and the pappas are.  Note, all we have
	   to work with are parent names.  Therefore, we have to work bassackwards.

	   End result of all of this, is that each child needs to have the index
	   of its parent; and each sub-grid that has a child needs to be so marked. */
	for (idx =  0;idx < __This->SubCount;idx += 1)
	{
		kidPtr = &__This->SubGridDir [idx];
		if (CS_stricmp (kidPtr->Parent,"NONE    "))
		{
			/* Its a child, find the parent. */
			for (parIdx = 0;parIdx < __This->SubCount;parIdx += 1)
			{
				parPtr = &__This->SubGridDir [parIdx];
				if (!CS_stricmp (kidPtr->Parent,parPtr->Name))
				{
					/* Save the index of the parent. */
					kidPtr->ParentIndex = parIdx;

					/* Mark the parent as having a child, if not already so marked. */
					if (parPtr->ChildIndex == -1 || parPtr->ChildIndex > idx)
					{
						parPtr->ChildIndex = idx;
					}
				}
			}
		} 
	}

	/* To accomdate the Spanish (and perhaps others in the future, we check the
	   parent grids in the list of sub-grids for overlap.  If overlap exists, we
	   turn on the SubOverlap flag.  Of course, if this flag is already on, we
	   have nothing to do.  If we did indeed turn on the SubOverlap flag,
	   we need to cruise through all the sub-grids and set the Cacheable flag
	   to false to assure that no data from this file makes it to the grid cell
	   cache. */
	if (__This->SubOverlap == 0)
	{
		for (parIdx = 0;parIdx < __This->SubCount && __This->SubOverlap == 0;parIdx += 1)
		{
			parPtr = &__This->SubGridDir [parIdx];
			/* Top level grids only, we know the children overlap. */
			if (parPtr->ParentIndex >= 0) continue;

			overlap = FALSE;			
			for (idx = 0;idx < __This->SubCount;idx += 1)
			{
				if (idx == parIdx) continue;
				subPtr = &__This->SubGridDir [idx];
				if (subPtr->ParentIndex >= 0) continue;
				
				/* See if subPtr overlaps with parPtr. */
				overlap  = subPtr->SeReference [LNG] > parPtr->SeReference [LNG] &&
						   subPtr->SeReference [LAT] > parPtr->SeReference [LAT] &&
						   subPtr->SeReference [LNG] < parPtr->NwReference [LNG] &&
						   subPtr->SeReference [LAT] < parPtr->NwReference [LAT];
				overlap |= subPtr->NwReference [LNG] > parPtr->SeReference [LNG] &&
						   subPtr->NwReference [LAT] > parPtr->SeReference [LAT] &&
						   subPtr->NwReference [LNG] < parPtr->NwReference [LNG] &&
						   subPtr->NwReference [LAT] < parPtr->NwReference [LAT];
				if (overlap)
				{
					__This->SubOverlap = TRUE;		/* for testing ease */
				}
			}
		}
	
		if (__This->SubOverlap != 0)
		{
			for (idx = 0;idx < __This->SubCount;idx += 1)
			{
				subPtr = &__This->SubGridDir [idx];
				subPtr->Cacheable = FALSE;
			}
		}
	}

	/* OK, we should be ready to rock and roll.  We close the Stream until
	   we actually need it.  Often, we get constructed just so there is a
	   record of the coverage afforded by the file. */
	if (__This->Stream != NULL)
	{
		CS_fclose (__This->Stream);
		__This->Stream = NULL;
	}
	csErrnam [0] = '\0';
	return __This;
error:
	if (__This != NULL)
	{
		if (__This->Stream != NULL) CS_fclose (__This->Stream);
		if (__This->SubGridDir != NULL) CS_free (__This->SubGridDir);
		CS_free (__This);
	}
	return NULL;
}

/* Destructor */
void CSdeleteGridFileCa2 (struct csGridFileCa2_* __This)
{
	if (__This != NULL)
	{
		if (__This->Stream != NULL) CS_fclose (__This->Stream);
		if (__This->SubGridDir != NULL) CS_free (__This->SubGridDir);
		CS_free (__This);
	}
}

/* Release allocated resources without losing existence information.
	Pbject can still be used. */
void CSreleaseGridFileCa2 (struct csGridFileCa2_* __This)
{
	if (__This != NULL)
	{
		if (__This->Stream != NULL)
		{
			CS_fclose (__This->Stream);
			__This->Stream = NULL;
		}
	}
}

/* Returns the name of the specific grid file which would be used if the
	provided point was to be converted. */
Const char* CSpathGridFileCa2 (struct csGridFileCa2_* __This)
{
	return (Const char *)__This->FilePath;
}

/* Interpolation Calculator
	Due to a bust in the file format, we do not buffer up grid cells and stuff.
	There are a couple of sub-grids which overlap other grids in such a way that
	buffering can cause errors.  So, at least until (if ever) the data file is
	corrected, we do no buffering of the grid cells.

	Also, this file format is being adopted by others, like the Aussie's.  We
	don't know what they are going to do.  So to be safe, NO BUFFERING OF GRID
	CELLS.

	Also, due to the sub-grid nature of the data file, we do not buffer the
	data file in any special way; we simply use normal stream buffering.
*/
int CScalcGridFileCa2 (struct csGridFileCa2_* __This,double deltaLL [2],Const double source [2])
{
	extern double cs_Zero;				/* 0.0 */
	extern double cs_LlNoise;			/* 1.0E-12 */
	extern char csErrnam [];

	short idx;
	short parIdx;
	short onLimit;
	unsigned short eleNbr, rowNbr;

	int seekStat;
	int rtnValue;
	size_t readCnt;
	long32_t filePosition;
	struct csGridFileCa2SubGrid_ *subPtr;
	struct csGridFileCa2SubGrid_ *cvtPtr;

	double bestCellSize;
	double wpLL [2];
	double swCell [2];

	struct TcsCaNTv2Data southEast;
	struct TcsCaNTv2Data southWest;
	struct TcsCaNTv2Data northEast;
	struct TcsCaNTv2Data northWest;

	/* Until we know differently. */
	__This->CellIsValid = FALSE;

	/* In case of an error.  This saves duplication of this many many times. */
	CS_stncp (csErrnam,__This->FilePath,MAXPATH);

	/* Remember, source is East Positive.  The CaNTv2 file is West Positive. */
	wpLL [LNG] = -source [LNG];
	wpLL [LAT] =  source [LAT];

	/* Locate the appropriate sub-grid.  If there is none, than there is no
	   coverage.  There are two algorithms:  the original one and one invented
	   to cater to the Spaniards, maybe someothers in the future.
	   
	   In the original alghorithm, we search through the top level of parent
	   grids looing for coverage.  The top level parents are those which have no
	   parent.  If none is found, there is no coverage.  If we locate a parent
	   which provides coverage, we examine all children of that parent looking
	   for a sub-grid; and so on.
	   
	   In the Spanish algorithm, we search all grids, and choose the grid which
	   produces the smallest cell size.  This is necessary as the grids are
	   allowed to overlap in the Spanish variation. */

	cvtPtr = NULL;			/* NULL says no coverage (yet). */
	if (__This->SubOverlap == 0)
	{
		parIdx = -1;
		/* The Canadian algorithm. We loop, only considering those
			sub-grids whose parent index match parIdx. */
		for (idx = 0;idx < __This->SubCount;idx += 1)
		{
			subPtr = &__This->SubGridDir [idx];

			/* The following verifies that the current sub is a child of
			   the located parent.  Also, causes children to be skipped
			   until such time as we have found a parent. */
			if (subPtr->ParentIndex != parIdx) continue;

			/* Does this sub grid cover the point we are to convert?
			   Remember, we're dealing with WEST POSITIVE longitude.
			   the SeReference & NwReference values are west positive.
			   Think of this being a transformation that applies to
			   Russia, instead of Canada. */
			if (wpLL [LNG] >= subPtr->SeReference [LNG] &&
				wpLL [LAT] >= subPtr->SeReference [LAT] &&
				wpLL [LNG] <= subPtr->NwReference [LNG] &&
				wpLL [LAT] <= subPtr->NwReference [LAT])
			{
				/* If this is a sub grid and on the northern or western
				   boundary, we do not consider it a match. */
				if (subPtr->ParentIndex >= 0 &&
					(wpLL [LNG] >= subPtr->NwReference [LNG] ||
					 wpLL [LAT] >= subPtr->NwReference [LAT])
				   )
				{
					continue;
				}
				
				/* We have a match. */
				cvtPtr = subPtr;

				/* See if this grid has one or more children. */
				if (cvtPtr->ChildIndex < 0)
				{
					/* This one has no children; use cvtPtr. */
					break;
				}

				/* This guy has children. We need to see if any of these
				   children cover the point we are converting.  Need a minus
				   one here as the loop code is going to bump idx. */
				parIdx = idx;
				idx = cvtPtr->ChildIndex - 1;
			}
		}
	}
	else
	{
		/* The Spanish variation.  We search all subgrids looking for
		   coverages.  As the sub-grids are allowed to overlap, we must
		   search them all, and we select the one which produces the
		   smallest cell size as the "appropriate" one. */
		bestCellSize = 1.0E+100;
		for (idx = 0;idx < __This->SubCount;idx += 1)
		{
			subPtr = &__This->SubGridDir [idx];

			/* Does this sub grid cover the point we are to convert?
			Remember, we're dealing with WEST POSITIVE longitude.
			the SeReference & NwReference values are west positive.
			Think of this being a transformation that applies to Russia,
			instead of Canada. */
			if (wpLL [LNG] >= subPtr->SeReference [LNG] &&
				wpLL [LAT] >= subPtr->SeReference [LAT] &&
				wpLL [LNG] <= subPtr->NwReference [LNG] &&
				wpLL [LAT] <= subPtr->NwReference [LAT])
			{
				/* Yes it does.  Getthe cell size and see if it is batter
				   than what we have found so far. */
				if (subPtr->Density < bestCellSize)
				{
					cvtPtr = subPtr;
					bestCellSize = subPtr->Density;
					onLimit = 0;
					if (fabs (wpLL [LAT] - cvtPtr->NwReference [LAT]) <= cs_LlNoise) onLimit |= 1;
					if (fabs (wpLL [LNG] - cvtPtr->NwReference [LNG]) <= cs_LlNoise) onLimit |= 2;
				}
			}
		}
		/* cvtPtr should still be null if no coverage was found, we rely on this. */
	}

	/* OK, if cvtPtr is not NULL, its a pointer to the approriate
	   sub grid for this conversion. */
	if (cvtPtr != NULL)
	{
		/* Make sure the file is opened.  It can get closed by a release. */
		if (__This->Stream == NULL)
		{
			__This->Stream = CS_fopen (__This->FilePath,_STRM_BINRD);
			if (__This->Stream == NULL)
			{
				CS_stncp (csErrnam,__This->FilePath,MAXPATH);
				CS_erpt (cs_DTC_FILE);
				goto error;
			}
			setvbuf (__This->Stream,NULL,_IOFBF,(size_t)__This->BufferSize);
		}

		/* Compute onLimit for this point and the selected sub-grid regardless
		   of how we got here.  This should now only occur at the extreme edges
		   of the entire file coverage. */
		onLimit = 0;
		if (fabs (wpLL [LAT] - cvtPtr->NwReference [LAT]) <= cs_LlNoise) onLimit |= 1;
		if (fabs (wpLL [LNG] - cvtPtr->NwReference [LNG]) <= cs_LlNoise) onLimit |= 2;

		/* Compute the elements required for the file access.  This is common to
		   all cases of "onLimit". */
		eleNbr = (unsigned short)(((wpLL [LNG] - cvtPtr->SeReference [LNG]) / cvtPtr->DeltaLng) + cs_LlNoise);
		rowNbr = (unsigned short)(((wpLL [LAT] - cvtPtr->SeReference [LAT]) / cvtPtr->DeltaLat) + cs_LlNoise);

		/* Compute the boundaries of the specific cell we dealing with, assuming
		   onLimit is zero (which is the case 99.999% of the time).  The
		   nonmenclature goes funny here as we use a grid cell structure which
		   is common to US and Canandian style files.  The craziness comes from
		   the fact that the US uses east positive longitude and the Canadians
		   use west positive latitude. */
		swCell [LNG] = cvtPtr->SeReference [LNG] + cvtPtr->DeltaLng * (double)eleNbr;
		swCell [LAT] = cvtPtr->SeReference [LAT] + cvtPtr->DeltaLat * (double)rowNbr;

		/* Build the extent portions of the grid cells.  Note that due to the
		   west positive nature of this dataset, the elements are used differently
		   than what the element names indicate.
		   
		   That is, since:
		   
		   1> the Canadians use positive west longitude, and
		   2> the US grid uses positive east lonigtudes, and
		   3> since we use the same grid cell structure for both the
		      US data and Canadian data,

		   we need to swap east and west in the nonmenclature used when
		   building a grid cell structure.  So, in thew code below, we are
		   actually putting the southeast corner in the southwest point
		   of the grid cell.  Similarly with the northwest and the
		   northeast. While we use the same grid cell structure (as we
		   must if as we cache these together in the same cache), we
		   have separate calculation routines; so this does work.
		 */
		__This->longitudeCell.coverage.southWest [LNG] = swCell [LNG];
		__This->longitudeCell.coverage.southWest [LAT] = swCell [LAT];
		__This->longitudeCell.coverage.northEast [LNG] = swCell [LNG] + cvtPtr->DeltaLng;
		__This->longitudeCell.coverage.northEast [LAT] = swCell [LAT] + cvtPtr->DeltaLat;
		__This->longitudeCell.coverage.density = cvtPtr->Density;
		__This->longitudeCell.deltaLng = cvtPtr->DeltaLng;
		__This->longitudeCell.deltaLat = cvtPtr->DeltaLat;

		__This->latitudeCell.coverage.southWest [LNG] = swCell [LNG];
		__This->latitudeCell.coverage.southWest [LAT] = swCell [LAT];
		__This->latitudeCell.coverage.northEast [LNG] = swCell [LNG] + cvtPtr->DeltaLng;
		__This->latitudeCell.coverage.northEast [LAT] = swCell [LAT] + cvtPtr->DeltaLat;
		__This->latitudeCell.coverage.density = cvtPtr->Density;
		__This->latitudeCell.deltaLng = cvtPtr->DeltaLng;
		__This->latitudeCell.deltaLat = cvtPtr->DeltaLat;

		/* We code reduce the code level here by getting smart with the onLimit
		   thing.  However, this gets very tricky.  My excuse here is that this
		   emulates the way the Canadians did it in FORTRAN. */
		if (onLimit == 0)
		{
			/* The normal case, probably about 99.99999 percent of the time.
			   Read the data into my record buffer. */
			filePosition = cvtPtr->FirstRecord + rowNbr * cvtPtr->RowSize + eleNbr * __This->RecSize;
			seekStat = CS_fseek (__This->Stream,filePosition,SEEK_SET);
			if (seekStat != 0)
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}

			/* Read southeast shifts. */
			readCnt = CS_fread (&southEast,1,sizeof (southEast),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (southEast))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}

			/* Read southwest shifts. */
			readCnt = CS_fread (&southWest,1,sizeof (southWest),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (southWest))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}

			/* Read northeast shifts. */
			filePosition += cvtPtr->RowSize;
			seekStat = CS_fseek (__This->Stream,filePosition,SEEK_SET);
			if (seekStat != 0)
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}
			readCnt = CS_fread (&northEast,1,sizeof (northEast),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (northEast))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}

			/* Read northwest shifts. */
			readCnt = CS_fread (&northWest,1,sizeof (northWest),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (northWest))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}

			/* Swap as necessary. */
			CS_bswap (&southEast,cs_BSWP_GridFileCa2Data);
			CS_bswap (&southWest,cs_BSWP_GridFileCa2Data);
			CS_bswap (&northEast,cs_BSWP_GridFileCa2Data);
			CS_bswap (&northWest,cs_BSWP_GridFileCa2Data);

			/* Build the grid cell AA, BB, CC, and DD values. */
			__This->longitudeCell.currentAA = southEast.del_lng;
			__This->longitudeCell.currentBB = southWest.del_lng - southEast.del_lng;
			__This->longitudeCell.currentCC = northEast.del_lng - southEast.del_lng;
			__This->longitudeCell.currentDD = northWest.del_lng - southWest.del_lng - northEast.del_lng + southEast.del_lng;

			__This->latitudeCell.currentAA = southEast.del_lat;
			__This->latitudeCell.currentBB = southWest.del_lat - southEast.del_lat;
			__This->latitudeCell.currentCC = northEast.del_lat - southEast.del_lat;
			__This->latitudeCell.currentDD = northWest.del_lat - southWest.del_lat - northEast.del_lat + southEast.del_lat;
		}
		else if (onLimit == 1)
		{
			/* Point is on the extreme northern edge of the sub-grid.  This occurs
			   ocassionally.  In this case, the "northern" boundary of the grid cell
			   doesn't exist, and we must manufacture such.  This is called a
			   virtual cell in the Canadian documentation.  */
			filePosition = cvtPtr->FirstRecord + rowNbr * cvtPtr->RowSize + eleNbr * __This->RecSize;
			seekStat = CS_fseek (__This->Stream,filePosition,SEEK_SET);
			if (seekStat != 0)
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}

			readCnt = CS_fread (&southEast,1,sizeof (southEast),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (southWest))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}

			readCnt = CS_fread (&southWest,1,sizeof (southWest),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (southEast))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}
			/* Swap as necessary. */
			CS_bswap (&southEast,cs_BSWP_GridFileCa2Data);
			CS_bswap (&southWest,cs_BSWP_GridFileCa2Data);

			/* Do not attempt to read the northern boundary, it ain't there.
			   Compute the AA, BB, CC, DD values. */
			__This->longitudeCell.currentAA = southEast.del_lng;
			__This->longitudeCell.currentBB = southWest.del_lng - southEast.del_lng;
			__This->longitudeCell.currentCC = cs_Zero;
			__This->longitudeCell.currentDD = cs_Zero;

			__This->latitudeCell.currentAA = southEast.del_lat;
			__This->latitudeCell.currentBB = southWest.del_lat - southEast.del_lat;
			__This->latitudeCell.currentCC = cs_Zero;
			__This->latitudeCell.currentDD = cs_Zero;
			
			/* Adjust the grid cell boundaries to indicate that the northern
			   limits are the same as the southern limits.  I.e. a grid cell
			   that has zero height. */
			__This->longitudeCell.coverage.northEast [LAT] = __This->longitudeCell.coverage.southWest [LAT] + cs_LlNoise;
			__This->latitudeCell.coverage.northEast [LAT]  = __This->latitudeCell.coverage.southWest [LAT] + cs_LlNoise;
		}
		else if (onLimit == 2)
		{
			/* Point is on the extreme western edge of the sub-grid. */
			filePosition = cvtPtr->FirstRecord + rowNbr * cvtPtr->RowSize + eleNbr * __This->RecSize;
			seekStat = CS_fseek (__This->Stream,filePosition,SEEK_SET);
			if (seekStat != 0)
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}
			readCnt = CS_fread (&southEast,1,sizeof (southEast),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (southWest))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}
			/* Don't read the south west, it ain't there. */

			filePosition += cvtPtr->RowSize;
			seekStat = CS_fseek (__This->Stream,filePosition,SEEK_SET);
			if (seekStat != 0)
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}
			readCnt = CS_fread (&northEast,1,sizeof (northEast),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (northWest))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}
			/* Don't read the northwest, it ain't there. */

			CS_bswap (&southEast,cs_BSWP_GridFileCa2Data);
			CS_bswap (&northEast,cs_BSWP_GridFileCa2Data);

			__This->longitudeCell.currentAA = southEast.del_lng;
			__This->longitudeCell.currentBB = cs_Zero;
			__This->longitudeCell.currentCC = northEast.del_lng - southEast.del_lng;
			__This->longitudeCell.currentDD = cs_Zero;

			__This->latitudeCell.currentAA = southEast.del_lat;
			__This->latitudeCell.currentBB = cs_Zero;
			__This->latitudeCell.currentCC = northEast.del_lat - southEast.del_lat;
			__This->latitudeCell.currentDD = cs_Zero;

			/* Adjust the grid cell boundaries to indicate that the eastern
			   limits are the same as the western limits.  I.e. a grid cell
			   that has zero width. */
			__This->longitudeCell.coverage.northEast [LNG] = __This->longitudeCell.coverage.southWest [LNG] + cs_LlNoise;
			__This->latitudeCell.coverage.northEast [LNG]  = __This->latitudeCell.coverage.southWest [LNG] + cs_LlNoise;
		}
		else  /* onLimit == 3 */
		{
			/* Point is actually the northwestern corner of the sub-grid. */
			filePosition = cvtPtr->FirstRecord + rowNbr * cvtPtr->RowSize + eleNbr * __This->RecSize;
			seekStat = CS_fseek (__This->Stream,filePosition,SEEK_SET);
			if (seekStat != 0)
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}
			readCnt = CS_fread (&southEast,1,sizeof (southWest),__This->Stream);
			if (CS_ferror (__This->Stream))
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (readCnt != sizeof (southWest))
			{
				CS_erpt (cs_INV_FILE);
				goto error;
			}

			/* Don't read anything else.  There's nothing there. */
			CS_bswap (&southEast,cs_BSWP_GridFileCa2Data);

			/* Compute the AA, BB, CC, DD values. */
			__This->longitudeCell.currentAA = southEast.del_lng;
			__This->longitudeCell.currentBB = cs_Zero;
			__This->longitudeCell.currentCC = cs_Zero;
			__This->longitudeCell.currentDD = cs_Zero;

			__This->latitudeCell.currentAA = southEast.del_lat;
			__This->latitudeCell.currentBB = cs_Zero;
			__This->latitudeCell.currentCC = cs_Zero;
			__This->latitudeCell.currentDD = cs_Zero;

			/* Adjust the grid cell boundaries to indicate that the northeastern
			   limits are the same as the southwestern limits.  I.e. a grid cell
			   that has zero width and zero height. */
			__This->longitudeCell.coverage.northEast [LAT] = __This->longitudeCell.coverage.southWest [LAT] + cs_LlNoise;
			__This->latitudeCell.coverage.northEast [LAT]  = __This->latitudeCell.coverage.southWest [LAT] + cs_LlNoise;
			__This->longitudeCell.coverage.northEast [LNG] = __This->longitudeCell.coverage.southWest [LNG] + cs_LlNoise;
			__This->latitudeCell.coverage.northEast [LNG]  = __This->latitudeCell.coverage.southWest [LNG] + cs_LlNoise;
		}

		/* The cells are now valid, maybe.  We now work around a bust
		   in the Canadian grid file format. */
		__This->CellIsValid = cvtPtr->Cacheable;
		for (idx = 0;csKludgeTable [idx][0] != 0.0;idx += 1)
		{
			if (source [LNG] >= csKludgeTable [idx][0] &&
			    source [LAT] >= csKludgeTable [idx][1] &&
			    source [LNG] <= csKludgeTable [idx][2] &&
			    source [LAT] <= csKludgeTable [idx][3])
			{
				__This->CellIsValid = FALSE;
				break;
			}
		}

		/* Perform the interpolation calculation. */
		deltaLL [LNG] = CScalcGridCellCA (&__This->longitudeCell,source);
		deltaLL [LAT] = CScalcGridCellCA (&__This->latitudeCell,source);
		rtnValue = 0;
	}
	else
	{
		/* We didn't find a sub-grid.  The return value is +1 to indicate no
		   coverage.  We should, in this case, use the fall back guy. */
		deltaLL [LNG] = cs_Zero;
		deltaLL [LAT] = cs_Zero;
		rtnValue = 1;
	}
	csErrnam [0] = '\0';
	return rtnValue;
error:
	return -1;
}

/* Test function, used to determine if this object covers the provided point.
	If so, the "grid density" of the conversion is returned as a means of
	selecting one grid object over another. */
double CStestGridFileCa2 (struct csGridFileCa2_* __This,const double* location)
{
	short idx;
	short parIdx;

	struct csGridFileCa2SubGrid_ *subPtr;
	struct csGridFileCa2SubGrid_ *cvtPtr;

	double bestCellSize;
	double wpLL [2];

	/* Remember, location is East Positive.  The CaNTv2 file is
	   West Positive. */
	wpLL [LNG] = -location [LNG];
	wpLL [LAT] =  location [LAT];

	/* See if we have some coverage here.  We process through the sub
	   grid directory looking for a parent which covers the provided point.
	   If we don't find one, there is no coverage for this point. */
	cvtPtr = NULL;
	if (__This->SubOverlap == 0)
	{
		parIdx = -1;
		for (idx = 0;idx < __This->SubCount;idx += 1)
		{
			subPtr = &__This->SubGridDir [idx];

			/* The following verifies that the current sub is a child of the
			the located parent.  Also, causes children to be skipped until
			such time as we have found a parent. */
			if (subPtr->ParentIndex != parIdx) continue;

			/* Does this sub grid cover the location point we have been given.
			Remember, we're dealing with WEST POSITIVE longitude. */
			if (wpLL [LNG] >= subPtr->SeReference [LNG] &&
				wpLL [LAT] >= subPtr->SeReference [LAT] &&
				wpLL [LNG] <= subPtr->NwReference [LNG] &&
				wpLL [LAT] <= subPtr->NwReference [LAT])
			{
				/* Yes it does.  See if this grid has one or more children.
				Note here, how important it is for ChildIndex to be the
				lowest value of all child indices. */
				cvtPtr = subPtr;
				if (cvtPtr->ChildIndex < 0)
				{
					/* This one has no children; use cvtPtr. */
					break;
				}
				/* This guy has children. We need to see if any of these children
				cover the point we are converting.  Need a minus one here as
				the loop code is going to bump idx. */
				parIdx = idx;
				idx = cvtPtr->ChildIndex - 1;
			}
		}
	}
	else
	{
		/* The Spanish variation.  We search all subgrids looking for
		   coverages.  As the sub-grids are allowed to overlap, we must
		   search them all, and we select the one which produces the
		   smallest cell size as the "appropriate" one. */
		bestCellSize = 1.0E+100;
		for (idx = 0;idx < __This->SubCount;idx += 1)
		{
			subPtr = &__This->SubGridDir [idx];

			/* Does this sub grid cover the point we are to convert?
			Remember, we're dealing with WEST POSITIVE longitude.
			the SeReference & NwReference values are west positive.
			Think of this being a transformation that applies to Russia,
			instead of Canada. */
			if (wpLL [LNG] >= subPtr->SeReference [LNG] &&
				wpLL [LAT] >= subPtr->SeReference [LAT] &&
				wpLL [LNG] <= subPtr->NwReference [LNG] &&
				wpLL [LAT] <= subPtr->NwReference [LAT])
			{
				/* Yes it does.  Getthe cell size and see if it is batter
				   than what we have found so far. */
				if (subPtr->Density < bestCellSize)
				{
					cvtPtr = subPtr;
					bestCellSize = subPtr->Density;
				}
			}
		}
		/* cvtPtr should still be null if no coverage was found, we rely on this. */
	}

	/* Return the grid density of the sub-grid covering this point. If no
	   coverage, return zero. */
	return (cvtPtr == NULL) ? 0.0 : cvtPtr->Density;
}
/*
	The following returns a pointer to the file which would be used to
	do the calculation.  At this point, the file is already selected.
	What we do here is to add the name of the sub-grid to the name
	of the file.
*/
Const char *CSsourceGridFileCa2 (struct csGridFileCa2_* __This,Const double llSource [2])
{
	short idx;
	short parIdx;

	char *cp1;
	Const char *cp;
	struct csGridFileCa2SubGrid_ *subPtr;
	struct csGridFileCa2SubGrid_ *cvtPtr;

	double bestCellSize;
	double wpLL [2];

	cp = NULL;

	/* Remember, location is East Positive.  The CaNTv2 file is
	   West Positive. */
	wpLL [LNG] = -llSource [LNG];
	wpLL [LAT] =  llSource [LAT];

	/* See if we have some coverage here.  We process through the sub
	   grid directory looking for a parent which covers the provided point.
	   If we don't find one, there is no coverage for this point. */
	cvtPtr = NULL;
	if (__This->SubOverlap == 0)
	{
		parIdx = -1;
		for (idx = 0;idx < __This->SubCount;idx += 1)
		{
			subPtr = &__This->SubGridDir [idx];

			/* The following verifies that the current sub is a child of the
			the located parent.  Also, causes children to be skipped until
			such time as we have found a parent. */
			if (subPtr->ParentIndex != parIdx) continue;

			/* Does this sub grid cover the location point we have been given.
			Remember, we're dealing with WEST POSITIVE longitude. */
			if (wpLL [LNG] >= subPtr->SeReference [LNG] &&
				wpLL [LAT] >= subPtr->SeReference [LAT] &&
				wpLL [LNG] <= subPtr->NwReference [LNG] &&
				wpLL [LAT] <= subPtr->NwReference [LAT])
			{
				/* Yes it does.  See if this grid has one or more children.
				Note here, how important it is for ChildIndex to be the
				lowest value of all child indices. */
				cvtPtr = subPtr;
				if (cvtPtr->ChildIndex < 0)
				{
					/* This one has no children; use cvtPtr. */
					break;
				}
				/* This guy has children. We need to see if any of these children
				cover the point we are converting.  Need a minus one here as
				the loop code is going to bump idx. */
				parIdx = idx;
				idx = cvtPtr->ChildIndex - 1;
			}
		}
	}
	else
	{
		/* The Spanish variation.  We search all subgrids looking for
		   coverages.  As the sub-grids are allowed to overlap, we must
		   search them all, and we select the one which produces the
		   smallest cell size as the "appropriate" one. */
		bestCellSize = 1.0E+100;
		for (idx = 0;idx < __This->SubCount;idx += 1)
		{
			subPtr = &__This->SubGridDir [idx];

			/* Does this sub grid cover the point we are to convert?
			Remember, we're dealing with WEST POSITIVE longitude.
			the SeReference & NwReference values are west positive.
			Think of this being a transformation that applies to Russia,
			instead of Canada. */
			if (wpLL [LNG] >= subPtr->SeReference [LNG] &&
				wpLL [LAT] >= subPtr->SeReference [LAT] &&
				wpLL [LNG] <= subPtr->NwReference [LNG] &&
				wpLL [LAT] <= subPtr->NwReference [LAT])
			{
				/* Yes it does.  Getthe cell size and see if it is batter
				   than what we have found so far. */
				if (subPtr->Density < bestCellSize)
				{
					cvtPtr = subPtr;
					bestCellSize = subPtr->Density;
				}
			}
		}
		/* cvtPtr should still be null if no coverage was found, we rely on this. */
	}

	/* Return the grid density of the sub-grid covering this point. If no
	   coverage, return zero. */
	if (cvtPtr != NULL)
	{
		/* This file does cover the point, and we have located the
		   sub-grid. */
		cp1 = CS_stncp (__This->sourceId,__This->FileName,16);
		*cp1++ = ':';
		*cp1++ = ':';
		CS_stncp (cp1,cvtPtr->Name,16);
		cp = __This->sourceId;
	}
	return cp;
}
