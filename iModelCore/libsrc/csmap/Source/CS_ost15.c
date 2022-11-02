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

/* Quality check.  Verify that a grid file name is given and that the file
   exists, is readable, and appears to be in the OSTN15 format. */
int CSost15Q  (struct csGeodeticXfromParmsFile_* fileParms,
			   Const char* dictDir,
			   int err_list [],
			   int list_sz)
{
	extern char cs_DirsepC;

	int err_cnt;
#ifndef GEOCOORD_ENHANCEMENT
	size_t rdCnt;
#endif

	char *cp;
	csFILE* strm;

#ifndef GEOCOORD_ENHANCEMENT
	char chrBuffer [16];
#endif
	char pathBuffer [MAXPATH];

	cp = fileParms->fileName;
	if (*cp == '.' && *(cp + 1) == cs_DirsepC)
	{
		CS_stncp (pathBuffer,dictDir,sizeof (pathBuffer));
		CS_stncat (pathBuffer,cp,MAXPATH);
	}
	else
	{
		CS_stncp (pathBuffer,cp,MAXPATH);
	}

	/* We will return (err_cnt + 1) below. */
	err_cnt = -1;
	if (err_list == NULL) list_sz = 0;
#ifdef GEOCOORD_ENHANCEMENT
    /* We assume that the specified file is the binary one and that if it exists it is the proper format */
    strm = CS_fopen (pathBuffer,_STRM_BINRD);
	if (strm == NULL)
	{
        if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_FILE;
    }
    else
    {
    	CS_fclose (strm);
    }
#else
	/* Verify that the file exists and that the format appears to be correct. */
	strm = CS_fopen (pathBuffer,_STRM_BINRD);
	if (strm != NULL)
	{
		rdCnt = CS_fread (chrBuffer,1,sizeof (chrBuffer),strm);
		CS_fclose (strm);
		strm = NULL;

		if (rdCnt != sizeof (chrBuffer) || CS_strnicmp (chrBuffer,"Point_ID",8))
		{
			if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_FORMAT;
		}
	}
	else
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_FILE;
	}
#endif
	return (err_cnt + 1);
}

/* Setup function.  Initialize the grid interpolation object for use as an
   OSTN15 transformation object. */
int CSost15S (struct cs_GridFile_ *gridFile)
{
	int status;

	struct cs_Ostn15_* ostn15Ptr;

#ifdef GEOCOORD_ENHANCEMENT
	ostn15Ptr = CSnewOstn15 (gridFile->filePath, 1);
#else
	ostn15Ptr = CSnewOstn15 (gridFile->filePath);
#endif
	if (ostn15Ptr != NULL)
	{
		gridFile->fileObject.Ostn15 = ostn15Ptr;
		gridFile->test = (cs_TEST_CAST)CSost15T;
		gridFile->frwrd2D = (cs_FRWRD2D_CAST)CSost15F2;
		gridFile->frwrd3D = (cs_FRWRD3D_CAST)CSost15F3;
		gridFile->invrs2D = (cs_INVRS2D_CAST)CSost15I2;
		gridFile->invrs3D = (cs_INVRS3D_CAST)CSost15I3;
		gridFile->inRange = (cs_INRANGE_CAST)CSost15L;
		gridFile->release = (cs_RELEASE_CAST)CSost15R;
		gridFile->destroy = (cs_DESTROY_CAST)CSost15D;
		status = 0;
	}
	else
	{
		status = -1;
	}
	return status;
}
/* Test function, used to determine if this object covers the provided point.
   If we were to handle multiple grid files of this format (think Northern
   Ireland maybe), this function would be used to determine if the current file
   is to be used; i.e. does this file contain coverage for the given point in
   the given direction.
   
   Typically, a test function returns the density of the grid, i.e. the size
   of a grid cell square in terms of degrees.  In this case, out grid file
   is in meters, so the return value is kind of strange.  The important thing
   is to return zero if the provided point is outside the coverage of the
   grid data file.
*/
double CSost15T (struct cs_Ostn15_ *ostn15,double *ll_src,short direction)
{
	double density;

	density = CStestCoverage (&ostn15->coverage,ll_src);
	return density;
}
int CSost15F2 (struct cs_Ostn15_ *ostn15,double *ll_36,Const double *ll_89)
{
	extern char csErrnam [];

	int st;
	double xy89 [2];
	double xy36 [2];

	/* ll_89 and ll_36 may be the same array, so we don't set ll_36 until
	   we know the status of the conversion. */
	st = CSosgb2XY (TRUE,xy89,ll_89);
	if (st == 0)
	{
		st = CSforwardOstn15 (ostn15,xy36,xy89);
		if (st == 0)
		{
			st = CSosgb2LL (FALSE,ll_36,xy36);
		}
	}
	if (st != 0)
	{
		if (st > 0)
		{
			ll_36 [0] = ll_89 [0];
			ll_36 [1] = ll_89 [1];
		}
		else
		{
			CS_stncp (csErrnam,"CS_ost15::1",MAXPATH);
			CS_erpt (cs_ISER);
		}
	}
	return st;
}
int CSost15F3 (struct cs_Ostn15_ *ostn15,double *ll_36,Const double *ll_89)
{
	extern char csErrnam [];

	int st;
	double xy89 [2];
	double xy36 [2];

	/* ll_89 and ll_36 may be the same array, so we don't set ll_36 until
	   we know the status of the conversion. */
	st = CSosgb2XY (TRUE,xy89,ll_89);
	if (st == 0)
	{
		st = CSforwardOstn15 (ostn15,xy36,xy89);
		if (st == 0)
		{
			st = CSosgb2LL (FALSE,ll_36,xy36);
			ll_36 [2] = ll_89 [2];
		}
	}
	if (st != 0)
	{
		if (st > 0)
		{
			ll_36 [0] = ll_89 [0];
			ll_36 [1] = ll_89 [1];
			ll_36 [2] = ll_89 [2];
		}
		else
		{
			CS_stncp (csErrnam,"CS_ost15::1",MAXPATH);
			CS_erpt (cs_ISER);
		}
	}
	return st;
}
int CSost15I2 (struct cs_Ostn15_ *ostn15,double *ll_89,Const double* ll_36)
{
	extern char csErrnam [];

	int st;
	double xy89 [2];
	double xy36 [2];

	/* ll_36 and ll_89 may be the same array, so we don't set ll_89 until
	   we know the status of the conversion. */
	st = CSosgb2XY (FALSE,xy36,ll_36);
	if (st == 0)
	{
		st = CSinverseOstn15 (ostn15,xy89,xy36);
		if (st == 0)
		{
			st = CSosgb2LL (TRUE,ll_89,xy89);
		}
	}
	if (st != 0)
	{
		if (st > 0)
		{
			ll_89 [0] = ll_36 [0];
			ll_89 [1] = ll_36 [1];
		}
		else
		{
			CS_stncp (csErrnam,"CS_ost15::1",MAXPATH);
			CS_erpt (cs_ISER);
		}
	}
	return st;
}
int CSost15I3 (struct cs_Ostn15_ *ostn15,double *ll_89,Const double *ll_36)
{
	extern char csErrnam [];

	int st;
	double xy89 [2];
	double xy36 [2];

	/* ll_89 and ll_36 may be the same array, so we don't set ll_36 until
	   we know the status of the conversion. */
	st = CSosgb2XY (FALSE,xy36,ll_36);
	if (st == 0)
	{
		st = CSinverseOstn15 (ostn15,xy89,xy36);
		if (st == 0)
		{
			st = CSosgb2LL (TRUE,ll_89,xy89);
			ll_89 [2] = ll_36 [2];
		}
	}
	if (st != 0)
	{
		if (st > 0)
		{
			ll_89 [0] = ll_36 [0];
			ll_89 [1] = ll_36 [1];
			ll_89 [2] = ll_36 [2];
		}
		else
		{
			CS_stncp (csErrnam,"CS_ost15::1",MAXPATH);
			CS_erpt (cs_ISER);
		}
	}
	return st;
}
int CSost15L  (struct cs_Ostn15_ *ostn15,int cnt,Const double pnts [][3])
{
	int idx;
	int status;
	double density;

	status = cs_CNVRT_OK;
	for (idx = 0;idx < cnt;idx += 1)
	{
		density = CSost15T (ostn15,pnts [idx],cs_DTCDIR_FWD);
		if (fabs (density) < 1.0E-08)
		{
			status = cs_CNVRT_USFL;
			break;
	}
		}
	return status;
}
int CSost15R  (struct cs_Ostn15_ *ostn15)
{
	CSreleaseOstn15 (ostn15);
	return 0;
}
int CSost15D  (struct cs_Ostn15_ *ostn15)
{
	CSdeleteOstn15 (ostn15);
	return 0;
}

