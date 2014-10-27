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

/*lint -esym(715,flags) */			/* parameter not referenced */
/*lint -e826 */						/* we use char pointers to do calculations,
									   then cast the result to a double pointer
									   to access the data. */

/*??*/ /*Probably should rewrite the calculations to do the calculations using
		 a double pointer.  HOwever, this code has worked for a long time, so
		 its probably best not to mess with it until there is another reason
		 to do so. */

#include "cs_map.h"

/* Default buffer size which can be modified at run time.  Obvisously,
   must be modified before object is constructed. */

int csGridFileCa1BufrSz = 16384;

/*****************************************************************************/
/*	Implementation
		Note, we use some standard stuff originally developed for the US grid
		file system.  Note, that since the Canadians use west positive
		longitude, some modifications have to be made. */

/* "Private" support function. */
void CSinitGridFileCa1 (struct csGridFileCa1_ *__This)
{
	CSinitCoverage (&__This->coverage);
	__This->headerCount = 0;
	__This->elementCount = 0;
	__This->recordCount = 0;
	__This->recordSize = 0;
	__This->deltaLng = 0.0;
	__This->deltaLat = 0.0;
	__This->fileSize = 0L;
	__This->strm = NULL;
	__This->bufferSize = 0L;
	__This->bufferBeginPosition = -1L;
	__This->bufferEndPosition = -2L;
	__This->dataBuffer = NULL;
	__This->CellIsValid = FALSE;
	CSinitGridCell (&__This->longitudeCell);
	CSinitGridCell (&__This->latitudeCell);
	__This->filePath [0] = '\0';
	__This->FileName [0] = '\0';
}

/* Constructor */
struct csGridFileCa1_* CSnewGridFileCa1 (Const char *path,long32_t bufferSize,ulong32_t flags,double density)
{
	extern double cs_Sec2Deg;				/* 1.0 / 3600.0 */
	extern double cs_AnglTest;				/* 0.001 seconds of arc in radians. */
	extern char cs_DirsepC;
	extern char csErrnam [];

	size_t readCount;
	long32_t lclFileSize;
	double dblTmp;
	char *cp;
	FILE *fstr;
	struct csGridFileCa1_* __This = NULL;
	struct csGridFileCa1Hdr_ ca1GridHdr;
	char ctemp [MAXPATH];

	fstr = NULL;

	/* Malloc and initialize */
	__This = (struct csGridFileCa1_ *)CS_malc (sizeof (struct csGridFileCa1_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}

	/* Set default values for all members. */
	CSinitGridFileCa1 (__This);

	/* Save file path/name. */
	CS_stncp (__This->filePath,path,sizeof (__This->filePath));

	/* Extract and save last 15 characters of the data file name. */
	cp = strrchr (__This->filePath,cs_DirsepC);
	if (cp == NULL) cp = __This->filePath;
	else            cp += 1;
	CS_stncp (ctemp,cp,sizeof (ctemp));
	cp = strrchr (ctemp,'.');
	if (cp != NULL) *cp = '\0';
	cp = ctemp;
	if (strlen (ctemp) > 15)
	{
		cp = ctemp + strlen (ctemp) - 15;
	}
	CS_stncp (__This->FileName,cp,sizeof (__This->FileName));

	/* Get file information. */
	fstr = CS_fopen (__This->filePath,_STRM_BINRD);
	if (fstr == NULL)
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_DTC_FILE);
		goto error;
	}
	readCount = CS_fread (&ca1GridHdr,1,sizeof (ca1GridHdr),fstr);
	if (readCount != sizeof (ca1GridHdr))
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_INV_FILE);
		goto error;
	}
	if (CS_ferror (fstr))
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_IOERR);
		goto error;
	}
	if (CS_fseek (fstr,0L,SEEK_END))
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_IOERR);
		goto error;
	}
	__This->fileSize = CS_ftell (fstr);
	CS_fclose (fstr);
	fstr = NULL;
	if (__This->fileSize < 0L)
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_IOERR);
		goto error;
	}

	/* Swap the bytes if necessary. */
	CS_bswap (&ca1GridHdr,cs_BSWP_GridFileCa1Hdr);

	/* Initialize the file specific stuff.  Note, the Canadians use west
	   positive longitude.  So, think of Canada as if it was where Russia is. */
	__This->coverage.southWest [LNG] = ca1GridHdr.min_lng;
	__This->coverage.southWest [LAT] = ca1GridHdr.min_lat;
	__This->coverage.northEast [LNG] = ca1GridHdr.max_lng;
	__This->coverage.northEast [LAT] = ca1GridHdr.max_lat;
	__This->headerCount = ca1GridHdr.hdr_cnt;

	/* The following guys are doubles in this format. */
	__This->deltaLng = ca1GridHdr.del_lng;
	__This->deltaLat = ca1GridHdr.del_lat;

	/* Use the smallest of the denity values as the grid density. */
	__This->coverage.density = (ca1GridHdr.del_lng > ca1GridHdr.del_lat) ? ca1GridHdr.del_lat : ca1GridHdr.del_lng;

	/* We add a skosh to the density here to make sure the Version 2 file
	   takes precedence over the version 1 file. */
	__This->coverage.density += cs_Sec2Deg;

	/* Override the automatic density if the user has provided one. */
	if (density != 0.0)
	{
		__This->coverage.density = density;
	}

	/* Need to compute the record count, element count, and then the record size.
	   We use cs_AnglTest, a small number, to force 0.9999999999 to a 1.0. */
	dblTmp = (ca1GridHdr.max_lat - ca1GridHdr.min_lat) / ca1GridHdr.del_lat;
	__This->recordCount = (long32_t) (dblTmp + cs_AnglTest) + 1;
	dblTmp = (ca1GridHdr.max_lng - ca1GridHdr.min_lng) / ca1GridHdr.del_lng;
	__This->elementCount = (long32_t) (dblTmp + cs_AnglTest) + 1;
	__This->recordSize = __This->elementCount * (int)sizeof (double) * 2;

	/* Set the buffer size. */
	__This->bufferSize = bufferSize;

	/* Never more than the file size. */
	lclFileSize = __This->fileSize - (__This->headerCount * 16);
	if (__This->bufferSize >= lclFileSize) __This->bufferSize = __This->fileSize;

	/* Reduce to a multiple of record size */
	__This->bufferSize = (__This->bufferSize / __This->recordSize) * __This->recordSize;

	/* Make sure buffer is always large enough for two records. */
	if (bufferSize < (2 * __This->recordSize)) __This->bufferSize = 2 * __This->recordSize;

	return (__This);
error:
	if (fstr != NULL) fclose (fstr);
	CSdeleteGridFileCa1 (__This);
	return NULL;
}

/* Destructor */
void CSdeleteGridFileCa1 (struct csGridFileCa1_* __This)
{
	if (__This != NULL)
	{
		if (__This->strm != NULL) CS_fclose (__This->strm);
		if (__This->dataBuffer != NULL)	CS_free (__This->dataBuffer);
		CS_free (__This);
	}
}

/* Release: release allocated resources, but leaves object constructed.
	Object can still be used; resources will be restored as necessary. */
void CSreleaseGridFileCa1 (struct csGridFileCa1_* __This)
{
	if (__This != NULL)
	{
		/* Be sure to leave the buffer size, file size, and file coverage unmolested.
		   Be sure to cancel any information in the individual grid cell coverage. */
		if (__This->dataBuffer != NULL)
		{
			CS_free (__This->dataBuffer);
		}
		__This->dataBuffer = NULL;
		__This->bufferBeginPosition = -1L;
		__This->bufferEndPosition = -2L;
		CSinitGridCell (&__This->longitudeCell);
		CSinitGridCell (&__This->latitudeCell);
		if (__This->strm != NULL)
		{
			CS_fclose (__This->strm);
		}
		__This->strm = NULL;
	}
	return;
}

/* Coverage Test, determines if this object carries data necessary to
	convert the provided point. */
double CStestGridFileCa1 (struct csGridFileCa1_* __This,Const double sourceLL [2])
{
	/* Returns zero if not covered, file density if covered. */
	return CStestCoverageCA (&(__This->coverage),sourceLL);
}

/* This function would be a "private" function in C++.
	This function sets the currentCell member to the cell which covers the
	provided geographic coordinate.  Returns 0 on success or a neagtive value
	for a system error of some sort.  By design, this function is not
	supposed to be called unless there is a coverage match. Serious problems
	result is this is not the case. */
int CSextractGridCellCa1 (struct csGridFileCa1_* __This,Const double sourceLL [2])
{
	extern char csErrnam [];

	int eleNbr;
	int recNbr;
	long32_t lngTmp;
	long32_t checkSeek;
	long32_t fpos;
	long32_t fposBegin;
	long32_t fposEnd;
	long32_t dataBegin;
	int readCount;
	int checkCount;
	char* chrPtr;
	double* dblPtr;

	double wpLL [2];
	double southEast [2];
	double southWest [2];
	double northEast [2];
	double northWest [2];

	double swCell [2];
	double neCell [2];

	/* Mark the cell structures are invalid until we know different. */
	__This->CellIsValid = FALSE;

	/* Source lat/longs are west longitude negative.  The whole Canadian thing
	   is based on west longitude positive. So, we use wpLL in this function. */
	wpLL [LNG] = -sourceLL [LNG];
	wpLL [LAT] =  sourceLL [LAT];

	/* Compute the basic indices to the cell in the data file.  Notice that
	   in this file, west longitude is positive, thus the longitude calculations
	   may appear a bit strange.  Think of Canada being where Russia is now. */
	eleNbr = (long32_t)((wpLL [LNG] - __This->coverage.southWest [LNG]) / __This->deltaLng);
	recNbr = (long32_t)((wpLL [LAT] - __This->coverage.southWest [LAT]) / __This->deltaLat);
	if (eleNbr > __This->elementCount || recNbr > __This->recordCount)
	{
		/* Not supposed to call this function unless there is a known coverage
		   match.  Therefore, we consider this an internal software error. */
		CS_stncp (csErrnam,"CS_caV1GridFile:1",MAXPATH);
		CS_erpt  (cs_ISER);
		goto error;
	}

	/* Compute the latitude and longitude of the southwest corner of the required grid cell. */
	swCell [LNG] = __This->coverage.southWest [LNG] + __This->deltaLng * (double)eleNbr;
	swCell [LAT] = __This->coverage.southWest [LAT] + __This->deltaLat * (double)recNbr;

	/* The northeast corner of the grid cell. */
	neCell [LNG] = swCell [LNG] + __This->deltaLng;
	neCell [LAT] = swCell [LAT] + __This->deltaLat;

	/* Set these values into the grid cells. */
	CSsetCoverage (&__This->longitudeCell.coverage,swCell,neCell);
	__This->longitudeCell.coverage.density = __This->coverage.density;
	CSsetCoverage (&__This->latitudeCell.coverage,swCell,neCell);
	__This->latitudeCell.coverage.density = __This->coverage.density;

	/* Transfer the grid cell size. */
	__This->longitudeCell.deltaLng = __This->deltaLng;
	__This->longitudeCell.deltaLat = __This->deltaLat;
	__This->latitudeCell.deltaLng = __This->deltaLng;
	__This->latitudeCell.deltaLat = __This->deltaLat;

	/* Compute the position in the file of the data of interest.  Note, a header occupies
	   the first headerCount 16 byte records. */
	fpos = recNbr * __This->recordSize + eleNbr * (int)sizeof (double) * 2;
	fposBegin = (fpos / __This->recordSize) * __This->recordSize;
	fposEnd = fposBegin + __This->recordSize + __This->recordSize;
	fpos += __This->headerCount * 16;

	/* If the data we need is not already in the buffer, we get it there. First, do
	   we have a buffer yet? */
	if (__This->dataBuffer == NULL)
	{
		/* Constructor must set bufferSize. */
		__This->dataBuffer = CS_malc ((size_t)__This->bufferSize);
		if (__This->dataBuffer == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		/* Make sure the rest of this stuff knows the buffer is empty.  These values
		   will fail to match any specific position. */
		__This->bufferBeginPosition = -1L;
		__This->bufferEndPosition = -2L;
	}

	/* Is the data we need already in the buffer? */
	if (fposBegin < __This->bufferBeginPosition || fposBegin > __This->bufferEndPosition ||
		fposEnd   < __This->bufferBeginPosition || fposEnd   > __This->bufferEndPosition)
	{
		/* Need to read it in.  Is the file open? */
		if (__This->strm == NULL)
		{
			__This->strm = CS_fopen (__This->filePath,_STRM_BINRD);
			if (__This->strm == NULL)
			{
				CS_stncp (csErrnam,__This->filePath,MAXPATH);
				CS_erpt (cs_DTC_FILE);
				goto error;
			}
			/* We do our own buffering, turn stream buffering off. */
			setvbuf (__This->strm,NULL,_IONBF,0);
		}

		/* Compute the starting position of the actual read. */
		if (__This->bufferSize >= __This->fileSize)
		{
			__This->bufferBeginPosition = 0L;
			__This->bufferEndPosition = __This->fileSize;
			readCount = __This->fileSize;
		}
		else
		{
			dataBegin = __This->headerCount * 16;	/* position of first data record */
			__This->bufferBeginPosition = fposBegin;
			__This->bufferEndPosition = fposEnd;
			readCount = __This->bufferEndPosition - __This->bufferBeginPosition;

			/* In this section, lngTmp is the number of additional records
			   which can fit in the buffer. */
			lngTmp = (__This->bufferSize - readCount) / __This->recordSize;
			if (lngTmp > 2L)
			{
				/* Move the beginning of the read up by one half of the
				   amount of extra space in the buffer; but never past the
				   beginning the actual data. */
				lngTmp = lngTmp / 2;
				__This->bufferBeginPosition -= __This->recordSize * lngTmp;
				if (__This->bufferBeginPosition < dataBegin)
				{
					__This->bufferBeginPosition = dataBegin;
				}
				readCount = __This->bufferEndPosition - __This->bufferBeginPosition;
			}

			lngTmp = (__This->bufferSize - readCount) / __This->recordSize;
			if (lngTmp > 2L)
			{
				/* Move the end of the read back by the amount of extra
				   space in the buffer, but never past the end of the file. */
				__This->bufferEndPosition += __This->recordSize * lngTmp;
				if (__This->bufferEndPosition > __This->fileSize)
				{
					__This->bufferEndPosition = __This->fileSize;
				}
				readCount = __This->bufferEndPosition - __This->bufferBeginPosition;
			}

			lngTmp = (__This->bufferSize - readCount) / __This->recordSize;
			if (lngTmp > 0L)
			{
				/* In case the expanded end of read exceeded the end of the
				   file, we can move the beginning of the read up some more,
				   However, never more than the beginning of the first
				   data record. */
				__This->bufferBeginPosition -= __This->recordSize * lngTmp;
				if (__This->bufferBeginPosition < dataBegin)
				{
					__This->bufferBeginPosition = dataBegin;
				}
				readCount = __This->bufferEndPosition - __This->bufferBeginPosition;
			}

			/* Defensive programming. */
			if (readCount != __This->bufferSize)
			{
				CS_stncp (csErrnam,"CS_caV1GridFile:2",MAXPATH);
				CS_erpt (cs_ISER);
				goto error;
			}
		}

		/* OK, read in the data. */
		checkSeek = CS_fseek (__This->strm,__This->bufferBeginPosition,SEEK_SET);
		if (checkSeek < 0L)
		{
			CS_stncp (csErrnam,__This->filePath,MAXPATH);
			CS_erpt (cs_IOERR);
			goto error;
		}
		checkCount = (int)CS_fread (__This->dataBuffer,1,(size_t)readCount,__This->strm);
		if (checkCount != readCount)
		{
			CS_stncp (csErrnam,__This->filePath,MAXPATH);
			CS_erpt (cs_INV_FILE);
			goto error;
		}
		if (CS_ferror (__This->strm))
		{
			CS_stncp (csErrnam,__This->filePath,MAXPATH);
			CS_erpt (cs_IOERR);
			goto error;
		}
		/* Note, we swap the individual cells as we need them.  Perhaps we should
		   swap the whole file now, but we don't. */

		/* We have the desired data in the buffer.  Extract what we need from it.
		   If we read in the whole file, we close the stream now.  No need to
		   have the file descriptor opem. */
		if (__This->bufferSize == __This->fileSize)
		{
			CS_fclose (__This->strm);
			__This->strm = NULL;
		}
	}

	/* Extract from the buffer the four floats which we need. */
	chrPtr = (char *)(__This->dataBuffer) + (fpos - __This->bufferBeginPosition);
	dblPtr = (double *)(chrPtr);
	southEast [LAT] = *dblPtr;
	southEast [LNG] = *(dblPtr + 1);
	southWest [LAT] = *(dblPtr + 2);
	southWest [LNG] = *(dblPtr + 3);

	chrPtr += __This->recordSize;
	dblPtr = (double *)(chrPtr);
	northEast [LAT] = *dblPtr;
	northEast [LNG] = *(dblPtr + 1);
	northWest [LAT] = *(dblPtr + 2);
	northWest [LNG] = *(dblPtr + 3);

	/* Swap bytes as appropriate. */
	CS_bswap (southEast,"dd");
	CS_bswap (southWest,"dd");
	CS_bswap (northEast,"dd");
	CS_bswap (northWest,"dd");

	/* Do the calculations.  We do these here once and save the results in the
	   current cell.  Note, an ANSI 'C' compiler should do the calculation as
	   a double. */
	__This->longitudeCell.currentAA = (double)(southEast [LNG]);
	__This->longitudeCell.currentBB = (double)(southWest [LNG] - southEast [LNG]);
	__This->longitudeCell.currentCC = (double)(northEast [LNG] - southEast [LNG]);
	__This->longitudeCell.currentDD = (double)(northWest [LNG] - southWest [LNG] - northEast [LNG] + southEast [LNG]);
	strcpy (__This->latitudeCell.sourceId,__This->FileName);

	__This->latitudeCell.currentAA = (double)(southEast [LAT]);
	__This->latitudeCell.currentBB = (double)(southWest [LAT] - southEast [LAT]);
	__This->latitudeCell.currentCC = (double)(northEast [LAT] - southEast [LAT]);
	__This->latitudeCell.currentDD = (double)(northWest [LAT] - southWest [LAT] - northEast [LAT] + southEast [LAT]);
	strcpy (__This->latitudeCell.sourceId,__This->FileName);
	__This->CellIsValid = TRUE;

	return 0;
error:
	/* Disable the current grid cell. */
	CSinitCoverage (&__This->longitudeCell.coverage);
	CSinitCoverage (&__This->latitudeCell.coverage);

	/* Release all resources, forcing a reinitializing on next call.  This
	   guy is not supposed to fail. */
	CSreleaseGridFileCa1 (__This);
	return -1;	
}

/* Calculate: returns the shift (from NAD27 to NAD83) carried in the file
	for the provided point. Note, the shift is as recorded in the data file.
	i.e. seconds and west shift is positive. */
int CScalcGridFileCa1 (struct csGridFileCa1_* __This,double* deltaLL,Const double* ll27)
{
	extern double cs_Zero;			/* 0.0 */

	int status;

	if (CStestCoverageCA (&__This->longitudeCell.coverage,ll27) != 0.0)
	{
		status = 0;
	}
	else
	{
		status = CSextractGridCellCa1 (__This,ll27);
	}
	if (status == 0)
	{
		deltaLL [0] = CScalcGridCellCA (&__This->longitudeCell,ll27);
		deltaLL [1] = CScalcGridCellCA (&__This->latitudeCell,ll27);
	}
	else if (status > 0)
	{
		deltaLL [LNG] = cs_Zero;
		deltaLL [LAT] = cs_Zero;
	}
	return status;
}
Const char *CSsourceGridFileCa1 (struct csGridFileCa1_* __This,Const double *source)
{
	Const char *cp;

	cp = NULL;
	if (CStestGridFileCa1 (__This,source) != 0.0)
	{
		cp = __This->FileName;
	}
	return cp;
}
