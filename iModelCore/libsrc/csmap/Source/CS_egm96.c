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
/*lint -esym(715,flags) */	/* parameter not referenced */

#include "cs_map.h"

/*
	This file contains the implementation of the Egm96 object; i.e.
	struct cs_Egm96_.

	Egm96 is based on a text file with some 137,712 records, i.e.
	lines of text.  It provides geoid height data in meters for the
	entire world by means of a 15 minute by 15 minute grid structure.

	The first line of the file contains four values which define the
	nature of the grid:
		[1] => Latitude of the southern extent of the grid ( -90.000)
		[2] => Latitude of the northern extent of the grid ( +90.000)
		[3] => Longitude of the western extent of the grid (   0.000)
		[4] => Longitude of the eastern extent of the grid (+360.000)
		[5] => Size of a grid cell, in degrees, in the north/south direction.
		[6] => Size of a grid cell, in degrees, in the east/west direction,

	Data points follow the header delimited by variable amounts of white space.
	It's up to the reader to keep count to properly interpret the significance
	of the 1,036,800 data points.

	Given the four data points which define a grid cell, simple bi-linear
	interpolation is used to calculate the geoid height at a specific location.

	Converting the original text data file to binary, and therefore usable,
	form is non-trivial.  Thus, this module will compile a binary version of
	the data file and leave it in the "Dictionaries" folder for later reuse.
	Should the modifcation times on the source and binary files so indicate,
	the source file is re-compiled.

	Obvoiously, the compiled file contains binary data and is subject to
	platform variations.  A binary file compiled on another system should
	not be copied to the current system.  This may not work as expected.
	As a protection against this issue, the "constructor" (i.e. CSnewEgm96)
	will perform a basic test on the resulting binary form of the data
	obtained before returning a valid object pointer.
*/

struct cs_Egm96_ *CSnewEgm96 (const char *filePath,long32_t bufferSize,ulong32_t flags,double density)
{
	extern char cs_DirsepC;
	extern char cs_ExtsepC;
	extern double cs_Zero;
	extern char csErrnam [];

	int st;

	char *cp;
	struct cs_Egm96_ *__This = NULL;

	double testValue;

	/* Allocate and initialize the object. */
	__This = CS_malc (sizeof (struct cs_Egm96_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}

	/* Initialize the structure. */
	__This->southWest [LNG] = cs_Zero;
	__This->southWest [LAT] = cs_Zero;
	__This->northEast [LNG] = cs_Zero;
	__This->northEast [LAT] = cs_Zero;
	__This->density [LNG] = cs_Zero;
	__This->density [LAT] = cs_Zero;
	__This->searchDensity = density;
	__This->headerCount = 0L;
	__This->elementCount = 0L;
	__This->recordCount = 0L;
	__This->recordSize = 0L;
	__This->elementSize = sizeof (double);
	__This->strm = NULL;
	__This->bufferSize = 0L;
	__This->bufferBeginPosition = -1L;
	__This->bufferEndPosition = -2L;
	__This->dataBuffer = NULL;

	CS_stncp (__This->filePath,filePath,sizeof (__This->filePath));
	cp = strrchr (filePath,cs_DirsepC);							 /*lint !e605*/
	if (cp != NULL)
	{
		CS_stncp (__This->fileName,(cp + 1),sizeof (__This->fileName));
		cp = strrchr (__This->fileName,cs_ExtsepC);
		if (cp != NULL) *cp = '\0';
	}

	/* Create a binary file if one does not exist already. */
	st = CSmkBinaryEgm96 (__This);
	if (st != 0) goto error;

	/* Open the binary file. */
	st = CSopnBinaryEgm96 (__This,bufferSize);
	if (st != 0) goto error;

	/* Capture the proper search density. */
	__This->searchDensity = __This->density [LNG];
	if (__This->density [LAT] > __This->searchDensity)
	{
		__This->searchDensity = __This->density [LAT];
	}
	if (density > 0.0)
	{
		__This->searchDensity = density;
	}

	/* Defensive programming here to insure that we have a proper
	   binary representation, regardless of the source. */
	testValue = CSdebugEgm96 (__This);
	if (fabs (testValue) > 0.5)
	{
		CS_stncp (csErrnam,"cs_Egm96_",MAXPATH);
		CS_erpt (cs_SELF_TEST);
		goto error;
	}

	/* That's that. */
	return __This;
error:
	CSdeleteEgm96 (__This);
	return NULL;
}
void CSdeleteEgm96 (struct cs_Egm96_ *__This)
{
	if (__This != NULL)
	{
		CSreleaseEgm96 (__This);
		CS_free (__This);
	}
	return;
}
void CSreleaseEgm96 (struct cs_Egm96_ *__This)
{
	if (__This != NULL)
	{
		/* Close the file if opened. */
		if (__This->strm != NULL)
		{
			CS_fclose (__This->strm);
			__This->strm = NULL;
		}

		/* Delete the buffer is allocated. */
		if (__This->dataBuffer != NULL)
		{
			CS_free (__This->dataBuffer);
			__This->dataBuffer = NULL;
		}
		__This->bufferBeginPosition = -1L;
		__This->bufferEndPosition = -2L;
	}
	return;
}
double CStestEgm96 (struct cs_Egm96_ *__This,const double wgs84 [2])
{
	extern double cs_Zero;
	extern double cs_K360;

	double rtnValue;
	double lclLng;
	double lclLat;

	rtnValue = cs_Zero;
	lclLng = wgs84 [LNG];
	lclLat = wgs84 [LAT];
	if (lclLng < __This->southWest [LNG])
	{
		lclLng += cs_K360;
	}
	if (lclLng >= __This->southWest [LNG] &&
	    lclLng <= __This->northEast [LNG] &&
		lclLat >= __This->southWest [LAT] &&
	    lclLat <= __This->northEast [LAT])
	{
		rtnValue = __This->searchDensity;
	}
	return rtnValue;
}
/* Note, the following function is also called by the CSinverseOsgm91 function. */
int CScalcEgm96 (struct cs_Egm96_ *__This,double *geoidHgt,const double wgs84 [2])
{
	extern double cs_Mhuge;
	extern char csErrnam [];
	extern double cs_Zero;
	extern double cs_K360;

	enum {	none      =  0,
			north     =  1,
			south     =  2,
			east      =  4,
			west      =  8,
			northeast =  5,
			southeast =  6,
			northwest =  9,
			southwest = 10 } edge;

	long32_t readCount;
	long32_t checkCount;

	long32_t lngTmp;
	long32_t checkSeek;
	long32_t recNbr, eleNbr;
	long32_t recFirst, recLast;
	long32_t fposBegin, fposEnd;

	char *chrPtr;
	float *fltPtr;

	double lclLng, lclLat;
	double tt, uu;

	double cellNW [2];

	float southWest, southEast;
	float northWest, northEast;

	lclLng = wgs84 [LNG];
	lclLat = wgs84 [LAT];
	if (lclLng < __This->southWest [LNG])
	{
		lclLng += cs_K360;
	}

	/* Return now if out of range. */
	if (lclLng < __This->southWest [LNG] ||
		lclLng > __This->northEast [LNG] ||
		lclLat < __This->southWest [LAT] ||
		lclLat > __This->northEast [LAT])
	{
		*geoidHgt = cs_Zero;
		return 1;
	}

	/* Compute the indices of the grid cells involved.  That is, the record and
	   element number of the point which represents the NORTHwest corner of
	   the grid cell which covers the point we have been given.

	   Note, that the file is to proceed from North to South, then East to West.
	   that is the specification.  Note, first record = 0, and first element = 0. */

	/* Trac Ticket 100: Removing the small constant added to the real portion of
	   the calculation of the element and record numbers.  This constant is
	   appropriate in situations where the density is not a "nice" number.
	   Cell widths like 5 minutes yield rather awkward numbers when converted
	   to degrees.  In this case, 15 minutes converts to a nice binary number
	   in degrees.  It is considered very unlikely that we'll ever encounter a
	   geoid height file in this format where the density is something other
	   than 15 minutes; so we removed the addition of the 1.0E-012 constant */
	recNbr = (long32_t)((__This->northEast [LAT] - lclLat) / __This->density [LAT]);
	eleNbr = (long32_t)((lclLng - __This->southWest [LNG]) / __This->density [LNG]);

	/* Redundant; defensive purposes only. */
	if (recNbr < 0L || recNbr >= __This->recordCount ||
		eleNbr < 0L || eleNbr >= __This->elementCount)
	{
		*geoidHgt = cs_Zero;
		return 1;
	}

	/* Are we on an edge? A corner? */
	edge = none;
	if (recNbr == 0) edge |= north;
	if (recNbr >= (__This->recordCount - 1)) edge |= south;
	if (eleNbr == 0) edge |= west;
	if (eleNbr >= (__This->elementCount - 1)) edge |= east;

	/* Adjust the record and element numbers of south west corner of the
	   grid cell we will use if we are on an edge, or a corner. */
	switch (edge) {
	default:
	case none:
	case north:
	case west:
	case northwest:
		/* The algorithms will work fine for any of these situations. */
		break;
	case northeast:
		eleNbr -=1;
		break;
	case south:
		recNbr -= 1;
		break;
	case east:
		eleNbr -= 1;
		break;
	case southeast:
		recNbr -= 1;
		eleNbr -= 1;
		break;
	case southwest:
		recNbr -= 1;
		break;
	}

	/* Compute the lat/long of the southwest corner of the grid cell identified
	   by the recNbr and eleNbr variables. */
	cellNW [LAT] = __This->northEast [LAT] - __This->density [LAT] * (double)recNbr;
	cellNW [LNG] = __This->southWest [LNG] + __This->density [LNG] * (double)eleNbr;

	/* Do we have a buffer?  Could have been released.  Maybe this is the
	   first access. */
	if (__This->dataBuffer == NULL)
	{
		__This->dataBuffer = CS_malc ((size_t)__This->bufferSize);
		if (__This->dataBuffer == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}

		/* Make sure the rest of this stuff knows the buffer is empty.  These values
		   will fail to match any specific file position. */
		__This->bufferBeginPosition = -1L;
		__This->bufferEndPosition = -2L;
	}

	/* Determine exactly what specific data is required.  Note that we have
	   already adjusted the recNbr and eleNbr's to account for the edges, thus
	   we will always read in a complete cell even if the data point is on the
	   edge of a cell. */
	fposBegin = __This->headerCount + (recNbr * __This->recordSize) + (eleNbr * __This->elementSize);
	fposEnd   = fposBegin + __This->recordSize + __This->elementSize + __This->elementSize;

	/* Is the necessary data in the buffer already? */
	if (fposBegin < __This->bufferBeginPosition || fposBegin > __This->bufferEndPosition ||
		fposEnd   < __This->bufferBeginPosition || fposEnd   > __This->bufferEndPosition)
	{
		/* The data we need is not in the buffer.  If the file has been released,
		   open it again now. */
		if (__This->strm == NULL)
		{
			__This->strm = CS_fopen (__This->binaryPath,_STRM_BINRD);
			if (__This->strm == NULL)
			{
				CS_stncp (csErrnam,__This->binaryPath,MAXPATH);
				CS_erpt (cs_DTC_FILE);
				goto error;
			}
			/* We do our own buffering, turn stream buffering off. */
#if !defined(GEOCOORD_ENHANCEMENT)
			setvbuf (__This->strm,NULL,_IONBF,0);
#else			
			CS_setvbuf (__This->strm,NULL,_IONBF,0);
#endif
		}

		/* Fill the buffer with data, including the specific data that we need.
		   When we go to the "well", we always fetch a minimum of two "records"
		   of water. */
		recFirst = recNbr;
		recLast = recFirst + 1;
		lngTmp = __This->bufferSize / __This->recordSize;
		if (lngTmp > 3)
		{
			recFirst = recNbr - (lngTmp / 2);
			recLast = recFirst + (lngTmp - 1);
			while (recFirst < 0)
			{
				recFirst += 1;
				recLast += 1;
			}
		}
		readCount = (recLast - recFirst + 1) * __This->recordSize;
		if (readCount != __This->bufferSize)
		{
			CS_stncp (csErrnam,"CS_egm96:1",MAXPATH);
			CS_erpt (cs_ISER);
			goto error;
		}
#ifdef GEOCOORD_ENHANCEMENT
        /* CSMAP BUG. The size of the header of the file was not added to the begin position as it was used
		  in the computation of the fposbegin (location in file of north-west corner of cell). This resulted into
		  reading the wrong record for the north altitudes and sometimes reading out of the buffer for the south values
		  when located just west of the 0.0 longitude */
		__This->bufferBeginPosition = __This->headerCount + (recFirst * __This->recordSize);
#else
		__This->bufferBeginPosition = recFirst * __This->recordSize;
#endif
		__This->bufferEndPosition = __This->bufferBeginPosition + readCount;

		/* OK, read in the data. */
		checkSeek = CS_fseek (__This->strm,__This->bufferBeginPosition,SEEK_SET);
		if (checkSeek < 0L)
		{
			CS_stncp (csErrnam,__This->filePath,MAXPATH);
			CS_erpt (cs_IOERR);
			goto error;
		}
		checkCount = (long32_t)CS_fread (__This->dataBuffer,1,(size_t)readCount,__This->strm);
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
		/* We assume that the binary file is manufactured on the system which is
		   using it.  Therefore, byte swapping is not an issue. */
#ifdef GEOCOORD_ENHANCEMENT
		/* In the real world we close the file even if we have not read to the end */
		CS_fclose (__This->strm);
		__This->strm = NULL;
#endif
	}

	/* Extract the grid cell. */
	chrPtr = (char *)__This->dataBuffer + (fposBegin - __This->bufferBeginPosition);
	fltPtr = (float *)(chrPtr);									 /*lint !e826*/
	northWest = *fltPtr;
	northEast = *(fltPtr + 1);
	chrPtr += __This->recordSize;
	fltPtr = (float *)(chrPtr);									 /*lint !e826*/
	southWest = *fltPtr;
	southEast = *(fltPtr + 1);

	/* Perform the bi-linear calculation. */
	tt = (lclLng - cellNW [LNG]) / __This->density [LNG];
	uu = (cellNW [LAT] - lclLat) / __This->density [LAT];

	/* Again, some defensive stuff. */
#ifdef GEOCOORD_ENHANCEMENT
    // CSMAP is too strict as floating point error can introduce small variations in lat/long computations
    // Since the accuracy of a double is around 15 digits and lat / long values are up to 3 digits positive
    // it is reasonable to expect inacuracy to 1E-11 degrees. We will apply 1E-10 just be safe
    // Note that 1E-10 degrees is way smaller than measurable accuracy of any ground feature.
#define VERY_SMALL_INSIGNIFICANT_LAT_LONG (0.0000000001) /* Should represent far less than a millimeter ground tolerance */
	if (tt + (VERY_SMALL_INSIGNIFICANT_LAT_LONG / __This->density[LNG]) < 0.0 || tt - (VERY_SMALL_INSIGNIFICANT_LAT_LONG / __This->density[LNG]) > 1.0 ||
        uu + (VERY_SMALL_INSIGNIFICANT_LAT_LONG / __This->density[LAT]) < 0.0 || uu - (VERY_SMALL_INSIGNIFICANT_LAT_LONG / __This->density[LAT]) > 1.0)
#else
	if (tt < 0.0 || tt > 1.0 || uu < 0.0 || uu > 1.0)
#endif
	{
		CS_stncp (csErrnam,"CS_egm96:2",MAXPATH);
		CS_erpt (cs_ISER);
		goto error;
	}

	/* OK, do the calculation. */
	*geoidHgt = northWest +
				tt * (northEast - northWest) +
				uu * (southWest - northWest) +
				tt * uu * (northWest - northEast - southWest + southEast);

	/* We're done. */
	return 0;
error:
	CSreleaseEgm96 (__This);

	/* We do the following to make sure that no application uses an error
	   without some one noticing it. */
	*geoidHgt = cs_Mhuge;
	return -1;
}
/* This function creates a binary version of the Egm91 file if it doesn't
   exist, or if the date on the binary file is older than that of the
   text file.  Note, that this function uses the path name in the
   provided object, and modifies it to point to the binary file which
   is created.

   This function is required as records in the ASCII text file are not
   of fixed length, and there are a zillion of them.  The binary file
   enables random access to the data file for decent performance without
   eating up 3MB of RAM. */
/*lint -save -esym(644,minLng,minLat,maxLng,maxLat) */
/*lint -save -esym(645,deltaLat,deltaLng) */
int CSmkBinaryEgm96 (struct cs_Egm96_ *__This)
{
	extern char cs_ExtsepC;
	extern char csErrnam [];

	extern double cs_Zero;			/* 0.0 */

	int st;
	unsigned ii, jj;
	unsigned rows, elements;
#if !defined(GEOCOORD_ENHANCEMENT)
	cs_Time_ aTime, bTime;
#else
	cs_Time_ bTime;
#endif

	char *cp;
	csFILE *aStrm, *bStrm;

	double dblBufr;
	float floatBufr;

	double minLat = cs_Zero;
	double maxLat = cs_Zero;
	double minLng = cs_Zero;
	double maxLng = cs_Zero;
	double deltaLat = cs_Zero;
	double deltaLng = cs_Zero;

	st = CS_rwDictDir (__This->binaryPath,sizeof (__This->binaryPath),__This->filePath);
#if !defined(GEOCOORD_ENHANCEMENT)	/* Even if writable dir cannot be resolved, the binary file might already exist. */
	if (st != 0)
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_DTC_FILE);
		goto error;
	}
#endif
	cp = strrchr (__This->binaryPath,cs_ExtsepC);
	if (cp == NULL)
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_DTC_FILE);
		goto error;
	}
	CS_stcpy ((cp + 1),"_96");

#if !defined(GEOCOORD_ENHANCEMENT)
	aTime = CS_fileModTime (__This->filePath);
	if (aTime == 0)
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_DTC_FILE);
		goto error;
	}
#endif

	bTime = CS_fileModTime (__This->binaryPath);
#ifdef GEOCOORD_ENHANCEMENT
	if (bTime == 0)
#else
	if (bTime == 0 || bTime <= aTime)
#endif
	{
		/* Here to create a, possibly new, binary version of the (what is
		   usually named) WW15MGH.GRD file.  We write a file which contains
		   a header of six doubles, and a number of floats which is based
		   on the data contained in the first six doubles. */
		aStrm = CS_fopen (__This->filePath,_STRM_TXTRD);
		if (aStrm == NULL)
		{
			CS_stncp (csErrnam,__This->filePath,MAXPATH);
			CS_erpt (cs_DTC_FILE);
			goto error;
		}

		/* The mode of the following open will truncate any existing file, and
		   create a new file if necessary. */
		bStrm = CS_fopen (__This->binaryPath,_STRM_BINWR);
		if (bStrm == NULL)
		{
#ifdef GEOCOORD_ENHANCEMENT
            /* If we get an error opening the binary file then the input file must be closed */
            CS_fclose (aStrm);
            /* Sometimes the binary file exists but cannot be opened for write ... we will use
              the file as it is assuming that the time stamp difference is irrelevant */

            /* Note that on LINUX the bTime can be 0 even though the file exists so we open it as read to test existence */
            bStrm = CS_fopen (__This->binaryPath,_STRM_BINRD);
            if (bStrm != NULL)
                {
                /* File exists ... we will use it as is */
                CS_fclose (bStrm);
                return 0;
                }

            /* Binary file does not exist and we cannot create it ... error */
#endif
			CS_stncp (csErrnam,__This->binaryPath,MAXPATH);
			CS_erpt (cs_FL_OPEN);
			goto error;
		}

		/* If we're still here, we can copy the file, converting it to
		   binary form as we do.

		   First, we to successfully read and convert the six header values. */
		st = CSextractDbl (aStrm,&minLat);
		if (st == 0) st = CSextractDbl (aStrm,&maxLat);
		if (st == 0) st = CSextractDbl (aStrm,&minLng);
		if (st == 0) st = CSextractDbl (aStrm,&maxLng);
		if (st == 0) st = CSextractDbl (aStrm,&deltaLat);
		if (st == 0) st = CSextractDbl (aStrm,&deltaLng);
		if (st != 0 || (minLat >= maxLat) || (minLng >= maxLng) ||
					   (minLat <  -90.0)  || (maxLat >   90.0)  ||
					   (minLng < -180.0)  || (maxLng >  360.0))
		{
			CS_fclose (aStrm);
			CS_fclose (bStrm);
			CS_erpt (cs_INV_FILE);
			goto error;
		}
		CS_fwrite (&minLat,sizeof (minLat),1,bStrm);
		CS_fwrite (&maxLat,sizeof (maxLat),1,bStrm);
		CS_fwrite (&minLng,sizeof (minLng),1,bStrm);
		CS_fwrite (&maxLng,sizeof (maxLng),1,bStrm);
		CS_fwrite (&deltaLat,sizeof (deltaLat),1,bStrm);
		CS_fwrite (&deltaLng,sizeof (deltaLng),1,bStrm);

		rows = (unsigned)(((maxLat - minLat) / deltaLat) + 0.001) + 1;
		elements = (unsigned)(((maxLng - minLng) / deltaLng) + 0.001) + 1;
		for (ii = 0;ii < rows;ii++)
		{
			for (jj = 0;jj < elements;jj++)
			{
				st = CSextractDbl (aStrm,&dblBufr);
				if (st != 0)
				{
					CS_fclose (aStrm);
					CS_fclose (bStrm);
					CS_erpt (cs_INV_FILE);
					goto error;
				}
				floatBufr = (float)dblBufr;
				CS_fwrite (&floatBufr,sizeof (floatBufr),1,bStrm);
			}
		}
		CS_fclose (aStrm);
		st = CS_fclose (bStrm);
		if (st != 0)
		{
			CS_erpt (cs_INV_FILE);
			goto error;
		}
	}

	return 0;
error:
	return -1;
}
/*lint -restore */
/*lint -save -esym(644,deltaLat,deltaLng) */
int CSopnBinaryEgm96 (struct cs_Egm96_ *__This,long32_t bufrSize)
{
	extern char csErrnam [];

	extern double cs_Zero;			/* 0.0 */

	size_t st;

	double minLat = cs_Zero;
	double maxLat = cs_Zero;
	double minLng = cs_Zero;
	double maxLng = cs_Zero;
	double deltaLat = cs_Zero;
	double deltaLng = cs_Zero;

	__This->strm = CS_fopen (__This->binaryPath,_STRM_BINRD);
	if (__This->strm == NULL)
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_DTC_FILE);
		goto error;
	}

	st = CS_fread (&minLat,sizeof (minLat),1,__This->strm);
	if (st == 1) st = CS_fread (&maxLat,sizeof (maxLat),1,__This->strm);
	if (st == 1) st = CS_fread (&minLng,sizeof (minLng),1,__This->strm);
	if (st == 1) st = CS_fread (&maxLng,sizeof (maxLng),1,__This->strm);
	if (st == 1) st = CS_fread (&deltaLat,sizeof (deltaLat),1,__This->strm);
	if (st == 1) st = CS_fread (&deltaLng,sizeof (deltaLng),1,__This->strm);

	/* We close the stream now when we opened it above, it allocated its
	   own buffer.  When the first calculation is requested, the stream
	   will get opened again, but unbuffered.  You can't change the buffering
	   once an I/O operation has been performed. */
	CS_fclose (__This->strm);
	__This->strm = NULL;

	/* Did we get all that we needed. */
	if (st != 1)
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_INV_FILE);
		goto error;
	}

	__This->southWest [LNG] = minLng;
	__This->southWest [LAT] = minLat;
	__This->northEast [LNG] = maxLng;
	__This->northEast [LAT] = maxLat;
	__This->density [LNG] = deltaLng;
	__This->density [LAT] = deltaLat;

	__This->headerCount = sizeof (double) * 6;
	__This->elementCount = 	(long32_t)(((maxLng - minLng) / deltaLng) + 0.001) + 1L;
	__This->recordCount = (long32_t)(((maxLat - minLat	) / deltaLat) + 0.001) + 1L;
	__This->recordSize = __This->elementCount * sizeof (float);
	__This->elementSize = sizeof (float);
	__This->bufferSize = __This->recordSize * 2;
	if (bufrSize > __This->bufferSize)
	{
		__This->bufferSize = bufrSize;
	}
	__This->bufferBeginPosition = -1L;
	__This->bufferEndPosition = -2L;
	__This->dataBuffer = NULL;

	return 0;
error:
	return -1;
}
/*lint -restore */
/*lint -restore */
double CSdebugEgm96 (struct cs_Egm96_ *__This)
{
	int st;						/* For debugging convenience. */
	double calcHgt;
	double knownHgt;
	double wgs84 [2];

	wgs84 [0] = 62.3873799444444;
	wgs84 [1] = 12.3873799444444;
	st = CScalcEgm96 (__This,&calcHgt,wgs84);
	knownHgt = -63.87;
	return fabs (calcHgt - knownHgt);
}								/*lint !e550*/ /* st is set, but not used */
