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

/*lint -esym(715,flags)                          parameter not referenced */

#include "cs_map.h"
#include <ctype.h>

int CScalcNtfToRfg93Deltas (struct csRgf93ToNtfTxt_ *__This,Const double ll_rgf93 [3],double *deltaX,double *deltaY,double *deltaZ);

struct csRgf93ToNtf_* csRgf93ToNtf = NULL;
int csRgf93ToNtfCnt = 0;

int EXP_LVL7 CSrgf93Init (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_Rgf93Name [];

	char catalog [MAXPATH];

	if (csRgf93ToNtf == NULL)
	{
		CS_stcpy (cs_DirP,cs_Rgf93Name);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));
		csRgf93ToNtf = CSnewRgf93ToNtf (catalog);
		if (csRgf93ToNtf == NULL) goto error;
	}
	csRgf93ToNtfCnt += 1;
	return 0;

error:
	if (csRgf93ToNtf != NULL)
	{
		CSdeleteRgf93ToNtf (csRgf93ToNtf);
		csRgf93ToNtf = NULL;
		csRgf93ToNtfCnt = 0;
	}
	return -1;
}
void EXP_LVL7 CSrgf93Cls (void)
{
	csRgf93ToNtfCnt -= 1;
	if (csRgf93ToNtfCnt <= 0)
	{
		if (csRgf93ToNtf != NULL)
		{
			CSreleaseRgf93ToNtf (csRgf93ToNtf);
		}
		csRgf93ToNtfCnt = 0;
	}
	return;
}
int EXP_LVL7 CSrgf93Chk (void)
{
	int status;
	struct csRgf93ToNtfEntry_* dtEntryPtr;

	status = 0;
	if (csRgf93ToNtf != NULL)
	{
		dtEntryPtr = csRgf93ToNtf->listHead;
		while (dtEntryPtr != NULL && status == 0)
		{
			if (dtEntryPtr->type == dtRgf93ToNtfTxt)
			{
				status = CScheckRgf93ToNtfTxt (dtEntryPtr->pointers.txtDatumPtr);
			}
			dtEntryPtr = dtEntryPtr->next;
		}
	}
	return status;
}
int EXP_LVL7 CSrgf93ToNtf (double ll_ntf [3],Const double ll_rgf93 [3])
{
	int status;
	double my_ntf [3];

	/* We always do the null conversion. */
	my_ntf [LNG] = ll_rgf93 [LNG];
	my_ntf [LAT] = ll_rgf93 [LAT];
	my_ntf [HGT] = ll_rgf93 [HGT];

	/* Do the real conversion, if possible. */
	if (csRgf93ToNtf == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcRgf93ToNtf (csRgf93ToNtf,my_ntf,ll_rgf93);
	}
	ll_ntf [LNG] = my_ntf [LNG];
	ll_ntf [LAT] = my_ntf [LAT];
	ll_ntf [HGT] = my_ntf [HGT];
	return status;
}
int EXP_LVL7 CSntfToRgf93 (double ll_rgf93 [3],Const double ll_ntf [3])
{
	int status;
	double my_rgf93 [3];

	/* We always do the null conversion. */
	my_rgf93 [LNG] = ll_ntf [LNG];
	my_rgf93 [LAT] = ll_ntf [LAT];
	my_rgf93 [HGT] = ll_ntf [HGT];

	/* Do the real conversion, if possible. */
	if (csRgf93ToNtf == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseRgf93ToNtf (csRgf93ToNtf,my_rgf93,ll_ntf);
	}
	ll_rgf93 [LNG] = my_rgf93 [LNG];
	ll_rgf93 [LAT] = my_rgf93 [LAT];
	ll_rgf93 [HGT] = my_rgf93 [HGT];
	return status;
}
/*
	The following object is used to convert geographic coordinates from the
	New Triangulation (actually, quite old now) to the newer RGF (don't
	know what RGF stands for) 1993 datum.

	Essentially, the grid shift file is read into memory when this thing
	is constructed.  The file is then closed.  The size of the grids in
	RAM is about 200KB, small potatoes by today's standards.

	Note that this is a bit of a deviation from the way this problem has
	been approached by others.  This grid defines the transformation from
	RGF93 to NTF.  Most algorithms go the other way.  Therefore, we are
	doing the inverse where most algorithms are doing the forward.

	To conserve RAM, we keep the shift values as millimeters in 32 bit
	longs.  This reduces, by half, the amount of memory required.  This
	does not introduce any inaccuracies, as the source .txt file provides
	all deltas in meters to three decimal places only.
*/
void CSinitializeRgf93ToNtfTxt (struct csRgf93ToNtfTxt_ *__This)
{
	extern double cs_Zero;

	CSinitCoverage (&__This->coverage);
	__This->lngCount = 0L;
	__This->latCount = 0L;
	__This->deltaLng = cs_Zero;
	__This->deltaLat = cs_Zero;
	__This->rgf93ERad = cs_Zero;
	__This->rgf93ESq = cs_Zero;
	__This->ntfERad = cs_Zero;
	__This->ntfESq = cs_Zero;
	__This->filePath [0] = '\0';
	__This->fileName [0] = '\0';
	__This->deltaX = NULL;
	__This->deltaX = NULL;
	__This->deltaX = NULL;
	__This->crcX = 0U;
	__This->crcY = 0U;
	__This->crcZ = 0U;
	return;
}
struct csRgf93ToNtfTxt_ *CSnewRgf93ToNtfTxt (Const char *filePath,long32_t bufferSize,ulong32_t flags,double density)
{
	extern char cs_DirsepC;
	extern char cs_ExtsepC;
	extern char csErrnam [];
	extern double cs_Zero;

	int hdrFlag = 0;

	long32_t dblFrmt;
	long32_t lngIdx, latIdx;
	unsigned arrayIdx;
	unsigned tknCount;
	size_t malcSize;

	char *cpV;
	csFILE *fstrm;
	struct cs_Eldef_ *elPtr;
	struct csRgf93ToNtfTxt_* __This;
	double lng, lat;
	double deltaX, deltaY, deltaZ;

	double sw [3], ne [2];
	char *ptrs [20];

	char lineBuffer [256];

	/* Prepare for an error. */
	fstrm = NULL;
	__This = NULL;
	elPtr = NULL;
	
	/* Keep lint happy! */
	sw [0] = sw [1] = ne [0] = ne [1] = cs_Zero;

	/* Allocate the initial chunk of memory for this thing. */
	__This = CS_malc (sizeof (struct csRgf93ToNtfTxt_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	CSinitializeRgf93ToNtfTxt (__This);

	/* Capture the full path to the file and the file name. */
	CS_stncp (__This->filePath,filePath,sizeof (__This->filePath));
	cpV = strrchr (__This->filePath,cs_DirsepC);
	if (cpV != NULL)
	{
		cpV += 1;
	}
	else
	{
		cpV = __This->filePath;
	}
	CS_stncp (__This->fileName,cpV,sizeof (__This->fileName));
	cpV = strrchr (__This->fileName,cs_ExtsepC);
	if (cpV != NULL) *cpV = '\0';

	/* OK, open the file, and extra the parameters.  There are
	   two types of files.  We handle either kind (since we can do it
	   all in this module).  Get the first line and determine which
	   type of file we have. */
	fstrm = CS_fopen (filePath,_STRM_TXTRD);
	if (fstrm == NULL)
	{
		CS_stncp (csErrnam,__This->filePath,MAXPATH);
		CS_erpt (cs_DTC_FILE);
		goto error;
	}
	
	/* If a buffersize has been given, we use it. */
	if (bufferSize > 0L)
	{
		setvbuf (fstrm,NULL,_IOFBF,(size_t)bufferSize);
	}

	/* Parse the file content. */
	if (CS_fgets (lineBuffer,sizeof (lineBuffer),fstrm) == NULL)
	{
		CS_erpt (cs_IOERR);
		goto error;
	}
	tknCount = CS_spaceParse (lineBuffer,ptrs,20);

	/* Decide which file type we have. */
	if (!CS_stricmp (ptrs [0],"GR3D"))
	{
		/* Here for the verbose format. */
		while (CS_fgets (lineBuffer,sizeof (lineBuffer),fstrm) != NULL)
		{
			tknCount = CS_spaceParse (lineBuffer,ptrs,20);
			if (!CS_stricmp (ptrs [0],"GR3D3")) break;
			if (!CS_stricmp (ptrs [0],"GR3D1") && tknCount == 7)
			{
				hdrFlag = 1;
				/* Convert the parsed tokens in a locale independent manner.
				   That is, we need to avoid use of functions like strtod
				   as they are locale dependent. */
				dblFrmt  = CSatof (&sw[0],ptrs [1],'.',',',':');
				dblFrmt |= CSatof (&ne[0],ptrs [2],'.',',',':');
				dblFrmt |= CSatof (&sw[1],ptrs [3],'.',',',':');
				dblFrmt |= CSatof (&ne[1],ptrs [4],'.',',',':');
				dblFrmt |= CSatof (&__This->deltaLng,ptrs [5],'.',',',':');
				dblFrmt |= CSatof (&__This->deltaLat,ptrs [6],'.',',',':');
				if (dblFrmt < 0)
				{
					/* This should never happen with a good data file. */
					CS_erpt (cs_INV_FILE);
					goto error;
				}
			}
		}
		if (hdrFlag == 0)
		{
			/* If we didn't see the expected header record, we have
			   a big problem. */
			CS_stncp (csErrnam,__This->filePath,MAXPATH);
			CS_erpt (cs_INV_FILE);
			goto error;
		}

		/* Capture the coverage area. */
		__This->coverage.southWest [LNG] = sw [0];
		__This->coverage.southWest [LAT] = sw [1];
		__This->coverage.northEast [LNG] = ne [0];
		__This->coverage.northEast [LAT] = ne [1];
		/* Set the density.  Normally it would be the smaller of deltaLng
		   or deltaLat.  These are usually the smae for this file type.
		   Alternatively, a density may have been specified by the user
		   in the Geodetic Data Catalog (.gdc file).  In that case,
		   we use that value. */
		if (density > 1.0E-12)			/* like to avoid == on doubles */
		{
			__This->coverage.density = density;
		}
		else if (__This->deltaLat >= __This->deltaLng)
		{
			__This->coverage.density = __This->deltaLat;
		}
		else
		{
			__This->coverage.density = __This->deltaLng;
		}

		/* Compute the size of the grid.  We need to add the extra 0.1
		   to account for round off errors on certain compilers/platforms. */
		__This->lngCount = (long32_t)(((ne [0] - sw [0]) / __This->deltaLng) + 0.1) + 1;
		__This->latCount = (long32_t)(((ne [1] - sw [1]) / __This->deltaLat) + 0.1) + 1;

		/* Now we can allocate the arrays. */
		malcSize = (size_t)(__This->lngCount * __This->latCount) * sizeof (long32_t);
		__This->deltaX = CS_malc (malcSize);
		if (__This->deltaX == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		__This->deltaY = CS_malc (malcSize);
		if (__This->deltaY == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		__This->deltaZ = CS_malc (malcSize);
		if (__This->deltaZ == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}

		/* Initialize the delta arrays to zero.  Perhaps we should use memset
		   here for performance reasons.  Not much to be gained, however, since
		   we have to parse through 2MB of text anyway. */
		for (lngIdx = 0;lngIdx < __This->lngCount;lngIdx += 1)
		{
			for (latIdx = 0;latIdx < __This->latCount;latIdx += 1)
			{
				arrayIdx = (unsigned)((latIdx * __This->lngCount) + lngIdx);
				*(__This->deltaX + arrayIdx) = 0L;
				*(__This->deltaY + arrayIdx) = 0L;
				*(__This->deltaZ + arrayIdx) = 0L;
			}
		}

		/* Fill in the ellipsoid numbers.  Pretty hokey, but it works.
		   Unfortuneately, the specific ellipsoids involved are impled by
		   the data file, but the numbers are not included in the format.
		   Thus, for now, this grid file format is usful only for the
		   specific French geography for which it was developed. */
		elPtr = CS_eldef ("GRS1980");
		if (elPtr == NULL) goto error;
		__This->rgf93ERad = elPtr->e_rad;
		__This->rgf93ESq = elPtr->ecent * elPtr->ecent;
		CS_free (elPtr);
		elPtr = NULL;

		elPtr = CS_eldef ("CLRK-IGN");
		if (elPtr == NULL) goto error;
		__This->ntfERad = elPtr->e_rad;
		__This->ntfESq = elPtr->ecent * elPtr->ecent;
		CS_free (elPtr);
		elPtr = NULL;

		/* Process the rest of the file. */
		while (CS_fgets (lineBuffer,sizeof (lineBuffer),fstrm) != NULL)
		{
			tknCount = CS_spaceParse (lineBuffer,ptrs,20);
			if (tknCount == 8)
			{
				/* Convert the data to numeric form in a locale independent
				   manner. */
				dblFrmt  = CSatof (&lng,ptrs [1],'.',',',':');
				dblFrmt |= CSatof (&lat,ptrs [2],'.',',',':');
				dblFrmt |= CSatof (&deltaX,ptrs [3],'.',',',':');
				dblFrmt |= CSatof (&deltaY,ptrs [4],'.',',',':');
				dblFrmt |= CSatof (&deltaZ,ptrs [5],'.',',',':');
				if (dblFrmt < 0)
				{
					/* This should never happen with a good data file. */
					CS_erpt (cs_INV_FILE);
					goto error;
				}

				/* Determine the location of the node in the grid. */
				lngIdx = (long32_t)(((lng - sw [0]) / __This->deltaLng) + 1.0E-10);
				latIdx = (long32_t)(((lat - sw [1]) / __This->deltaLat) + 1.0E-10);
				if (lngIdx < 0 || lngIdx >= __This->lngCount ||
					latIdx < 0 || latIdx >= __This->latCount)
				{
					/* This should never happen with a good data file. */
					CS_erpt (cs_INV_FILE);
					goto error;
				}
				
				/* Adjust for possible round down; required for Linux (or is it
				   the gcc runtime library), perhaps others as well. */
				deltaX += (deltaX < 0.0) ? -0.0002 : 0.0002;
				deltaY += (deltaY < 0.0) ? -0.0002 : 0.0002;
				deltaZ += (deltaZ < 0.0) ? -0.0002 : 0.0002;

				/* Stuff extracted values in the local arrays. */
				arrayIdx = (unsigned)((latIdx * __This->lngCount) + lngIdx);
				*(__This->deltaX + arrayIdx) = (long32_t)(deltaX * 1000.0);
				*(__This->deltaY + arrayIdx) = (long32_t)(deltaY * 1000.0);
				*(__This->deltaZ + arrayIdx) = (long32_t)(deltaZ * 1000.0);
			}
		}
	}
	else
	{
		/* We don't support the abbreviated file.  The abbreviated file is
		   simply a binary version of the text file.  Thus, we avoid all byte
		   swapping issues.  There is little to be gained by using the
		   abbreviated file. */
		CS_erpt (cs_INV_FILE);
		goto error;
	}

	/* For testing purposes, we generate the check-sum of the three memory
	   arrays allocated above.  The CScheckRgf93ToNtf function verifies that the
	   memory in these arrays has not changed since they were allocated.
	   
	   This was implemented in response to a bug report which suggested that the
	   in memory arrays may have been corrupted by some code elsehwere in the
	   library.  This check has never produced a failure and this code is a
	   good candidate for reemoval. */
	__This->crcX = CS_crc16 (0X0101,(unsigned char *)__This->deltaX,(int)malcSize);
	__This->crcY = CS_crc16 (0X0202,(unsigned char *)__This->deltaY,(int)malcSize);
	__This->crcZ = CS_crc16 (0X0404,(unsigned char *)__This->deltaZ,(int)malcSize);

	/* OK, we're outa here. */
	CS_fclose (fstrm);
	fstrm = NULL;
	return __This;

error:
	if (elPtr != NULL)
	{
		CS_free (elPtr);
		elPtr = NULL;
	}
	if (fstrm != NULL)
	{
		CS_fclose (fstrm);
		fstrm = NULL;
	}
	CSdeleteRgf93ToNtfTxt (__This);
	return NULL;
}
void CSreleaseRgf93ToNtfTxt (struct csRgf93ToNtfTxt_* __This)
{
	/* Contrary to other datum shift objects, this object does
	   not access the shift data file directly.  The content
	   of the file is maintained in memory and there is no
	   file descriptor kept open.  Thus, when we release this
	   object, we actually do nothing. */
	return;
}
void CSdeleteRgf93ToNtfTxt (struct csRgf93ToNtfTxt_ *__This)
{
	if (__This != NULL)
	{
		if (__This->deltaX != NULL)
		{
			CS_free (__This->deltaX);
			__This->deltaX = NULL;
		}
		if (__This->deltaY != NULL)
		{
			CS_free (__This->deltaY);
			__This->deltaY = NULL;
		}
		if (__This->deltaZ != NULL)
		{
			CS_free (__This->deltaZ);
			__This->deltaZ = NULL;
		}
		CS_free (__This);
	}
}
double CStestRgf93ToNtfTxt (struct csRgf93ToNtfTxt_* __This,Const double llRgf93 [3])
{
	double density;
	density = CStestCoverageUS (&__This->coverage,llRgf93);
	return density;
}
/* The forward function will convert an RFG93 geographic coordinate to
   NTF geographic coordinate.  This is the opposite of the normal case.  The
   forward conversion on most datum shift algorithms convert from the old to
   the new.  Not so in this case. */
int CScalcRgf93ToNtfTxt (struct csRgf93ToNtfTxt_ *__This,double ll_ntf [3],Const double ll_rgf93 [3])
{
	int rtnVal, xyzSt;

	double deltaX, deltaY, deltaZ;

	double xyz [3];

	rtnVal = 0;

	/* Prepare for an out of range condition. */
	ll_ntf [LNG] = ll_rgf93 [LNG];
	ll_ntf [LAT] = ll_rgf93 [LAT];
	ll_ntf [HGT] = ll_rgf93 [HGT];

	/* Determine the delta shift values. */
	rtnVal = CScalcNtfToRfg93Deltas (__This,ll_rgf93,&deltaX,&deltaY,&deltaZ);

	/* TO DO: The CScalcNtfToRfg93Deltas function will return the average
	   geocentric translation values for France whenever presented a geographic
	   coordinate outside the coverage of the grid data file.  These values are
	   hardcoded.  These values should, perhaps, be obtained from the fallback
	   or some means other than hardcoded.  Since the ellipsoids in use are also
	   harcoded (as there are no ellipsoid numbers in the data file format) this
	   is probably not a big deal. */

	/* A status value of +1 says that the given point is not covered by the grid file.
	   However, in this case, CScalcNtfToRfg93Deltas returns delta X, Y, & Z set
	   to average values.  Thus, we do the following regardless of the value
	   of rtnVal. */
	if (rtnVal >= 0)
	{
		/* OK, we have the appropriate delta values.  We need to do a three parameter
		   transformation using these values.  Before we can do this, we need to convert
		   the source geographic coordinates to geocentric form. */
		CS_llhToXyz (xyz,ll_rgf93,__This->rgf93ERad,__This->rgf93ESq);
		xyz [0] -= deltaX;
		xyz [1] -= deltaY;
		xyz [2] -= deltaZ;
		xyzSt = CS_xyzToLlh (ll_ntf,xyz,__This->ntfERad,__This->ntfESq);
		if (xyzSt != 0)
		{
			CS_erpt ( cs_XYZ_ITR);
			rtnVal = -1;
		}
	}
	return rtnVal;
}
/* The following version of the inverse calculation is commented out.  This
   version will produce the exact inverse of the forward within the
   tolerances programmed (i.e. 1.0E-09 degrees, about .1 millimeters.

   The reference document uses the alternative inverse which appears below.
   It will not produce the exact inverse, but is very very close, and is a
   lot faster.  Since this is what the French use, we use it as well.
*/
#if !defined (__SKIP__)
/* Since the following code has been tested, we leave it in here.  Easier to
   delete than it is to create.
   
   It is important to note that while this algorithm will produce the exact
   (i.e. within .1 millimeters) inverse, it does not take the height of the
   NTF source point into consideration.  Thus, while the horizontal inverse
   is rather precise, the effect of the height on this calculation has not
   been evaluated as yet. */
int CSinverseRgf93ToNtfTxt (struct csRgf93ToNtfTxt_ *__This,double ll_rgf93 [3],Const double ll_ntf [3])
{
	static int maxIteration = 20;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the source lat/longs to the local array.
	   This is the default result which the user may want in
	   the event of an error.  Note, we assume such has been
	   done below, even if there has not been an error. */
	ll_rgf93 [LNG] = guess [LNG] = ll_ntf [LNG];
	ll_rgf93 [LAT] = guess [LAT] = ll_ntf [LAT];
	ll_rgf93 [HGT] = guess [HGT] = ll_ntf [HGT];

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		rtnVal = CScalcRgf93ToNtfTxt (__This,newLl,guess);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = ll_ntf [LNG] - newLl [LNG];
		epsilon [LAT] = ll_ntf [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_RGF93_ICNT);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* Return the resuls if everything converged. */
	if (rtnVal >= 0)
	{
		ll_rgf93 [LNG] = guess [LNG];
		ll_rgf93 [LAT] = guess [LAT];
	}
	return rtnVal;
}
#else
/* The following calculation is the calculation as described in the
   source document from the IGN.  It produces values which are within
   1.5 millimeters of the original  coordinate.  So while the above is
   more accurate (accurate to 0.1 millimeters), this is the algorithm
   used as it is believed to be the officially sanctioned technique. */
int CSinverseRgf93ToNtfTxt (struct csRgf93ToNtfTxt_ *__This,double ll_rgf93 [3],Const double ll_ntf [3])
{
	int rtnVal = 0;

	double deltaX, deltaY, deltaZ;

	double xyz [3];

	/* We start with the basic, default, average, values. */
	deltaX = -168.0;
	deltaY =  -60.0;
	deltaZ =  320.0;

	/* The grid file is indexed by RGF93 lat/longs, even though the delta values
	   in the grid file are the values which convert from NTF to RGF93.  We only
	   have ntf lat/longs.  So, first thing we do is to do an approximate
	   calculation, converting the provided NTF lat/longs to RGF93 lat/longs using
	   standard, average, delta X, Y, & Z values.  Note that in the event
	   of an error below, these approximate results are allowed to remain in
	   the target array. */
	CS_llhToXyz (xyz,ll_ntf,__This->ntfERad,__This->ntfESq);
	xyz [0] += deltaX;
	xyz [1] += deltaY;
	xyz [2] += deltaZ;
	rtnVal = CS_xyzToLlh (ll_rgf93,xyz,__This->rgf93ERad,__This->rgf93ESq);
	if (rtnVal != 0)
	{
		CS_erpt ( cs_XYZ_ITR);
	}
	else
	{
		/* Now we use this approximate RGF93 lat/long to access the grid file to get
		   the Delta X, Y, & Z.  Obviously, not perfectly exact, but very close. */
		rtnVal = CScalcNtfToRfg93Deltas (__This,ll_rgf93,&deltaX,&deltaY,&deltaZ);

		/* A status value of +1 says that the given point is not covered by the grid file.
		   In this case, since we have already calculated an approximation, we are done. */
		if (rtnVal == 0)
		{
			CS_llhToXyz (xyz,ll_ntf,__This->ntfERad,__This->ntfESq);
			xyz [0] += deltaX;
			xyz [1] += deltaY;
			xyz [2] += deltaZ;
			rtnVal = CS_xyzToLlh (ll_rgf93,xyz,__This->rgf93ERad,__This->rgf93ESq);
			if (rtnVal != 0)
			{
				CS_erpt ( cs_XYZ_ITR);
			}
		}
	}
	return rtnVal;
}
#endif
Const char *CSsourceRgf93ToNtfTxt (struct csRgf93ToNtfTxt_* __This)
{
	Const char *cp;

	cp = __This->fileName;
	return cp;
}
/* Given a RGF93 lat/long in degreess relative to Greenwich, this function
   extracts the appropaite delta X, Y, and Z from the grid file.  Returns
   +1 if the provided lat/long are not covered by the grid file, in which
   case, the average geocentric translation values are returned.  These
   values are hard coded. */
int CScalcNtfToRfg93Deltas (struct csRgf93ToNtfTxt_ *__This,Const double ll_rgf93 [3],double *deltaX,double *deltaY,double *deltaZ)
{
	short westEdg, northEdg, eastEdg, southEdg;

	int rtnVal;

	unsigned swIdx, nwIdx, neIdx, seIdx;

	long32_t lngIdx, latIdx;

	double density;
	double cellLng, cellLat;
	double x1, x2, x3, x4;
	double y1, y2, y3, y4;										/*lint !e578 */
	double z1, z2, z3, z4;

	double sw [3];

	rtnVal = 0;

	/* In the case of a +1 return, we return the standard values. */
	*deltaX = -168.0;
	*deltaY =  -60.0;
	*deltaZ =  320.0;

	/* Check and make sure we have proper coverage.  The US qualification on
	   CStestCoverage function distinguishes this coverage cell from the Canadian
	   variety where west longitude is positive. */
	density = CStestCoverageUS (&__This->coverage,ll_rgf93);
	if (density == 0.0) return 1;

	/* Here if we have coverage.  Calculate the indices to the appropriate cell. */
	lngIdx = (long32_t)((ll_rgf93 [0] - __This->coverage.southWest [0]) / __This->deltaLng);
	latIdx = (long32_t)((ll_rgf93 [1] - __This->coverage.southWest [1]) / __This->deltaLat);

	/* The coverage test should have caught any coordinate which would cause
	   us a problem.  But to be extra safe: */
	westEdg  = (short)(lngIdx < 0);
	northEdg = (short)(latIdx >= __This->latCount);
	eastEdg  = (short)(lngIdx >= __This->lngCount);
	southEdg = (short)(latIdx < 0);
	if (westEdg || northEdg || eastEdg || southEdg) return 1;

	/* We calculate the geographic coordinates of the southwest corner of the
	   appropriate grid cell. */
	sw [0] = __This->coverage.southWest [0] + (lngIdx * __This->deltaLng);
	sw [1] = __This->coverage.southWest [1] + (latIdx * __This->deltaLat);

	/* Now, deltaLng and deltaLat are the normalized distances into the
	   grid cell of the provided point. */
	cellLng = (ll_rgf93 [0] - sw [0]) / __This->deltaLng;
	cellLat = (ll_rgf93 [1] - sw [1]) / __This->deltaLat;

	/* Compute the indices into the delta arrays.  Due to the above checking,
	   these indices should be safe to use for memory access. */
	swIdx = (unsigned)(( latIdx      * __This->lngCount) + lngIdx);
	nwIdx = (unsigned)(((latIdx + 1) * __This->lngCount) + lngIdx);
	neIdx = (unsigned)(((latIdx + 1) * __This->lngCount) + lngIdx + 1);
	seIdx = (unsigned)(( latIdx      * __This->lngCount) + lngIdx + 1);

	/* Now we can do the bilinear calculation.  The nomenclature here jives
	   with the reference document. */
	x1 = (double)(*(__This->deltaX + swIdx)) * 0.001;
	x2 = (double)(*(__This->deltaX + nwIdx)) * 0.001;
	x3 = (double)(*(__This->deltaX + seIdx)) * 0.001;
	x4 = (double)(*(__This->deltaX + neIdx)) * 0.001;

	y1 = (double)(*(__This->deltaY + swIdx)) * 0.001;
	y2 = (double)(*(__This->deltaY + nwIdx)) * 0.001;
	y3 = (double)(*(__This->deltaY + seIdx)) * 0.001;
	y4 = (double)(*(__This->deltaY + neIdx)) * 0.001;

	z1 = (double)(*(__This->deltaZ + swIdx)) * 0.001;
	z2 = (double)(*(__This->deltaZ + nwIdx)) * 0.001;
	z3 = (double)(*(__This->deltaZ + seIdx)) * 0.001;
	z4 = (double)(*(__This->deltaZ + neIdx)) * 0.001;

	*deltaX = x1 + (x3 - x1) * cellLng + (x2 - x1) * cellLat +
				   (x1 - x2 - x3 + x4) * cellLng * cellLat;
	*deltaY = y1 + (y3 - y1) * cellLng + (y2 - y1) * cellLat +
				   (y1 - y2 - y3 + y4) * cellLng * cellLat;
	*deltaZ = z1 + (z3 - z1) * cellLng + (z2 - z1) * cellLat +
				   (z1 - z2 - z3 + z4) * cellLng * cellLat;

	/* If we are still here, the return value is zero. */
	return rtnVal;
}
