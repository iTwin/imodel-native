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

/* Setup function.  Initialize the grid interpolation object for use as an
   OSTN02 transformation object. */
int CSost02S  (struct cs_GridFile_ *ost02)
{
	int status;

	struct cs_Ostn02_* ostn02Ptr;

#ifdef GEOCOORD_ENHANCEMENT
	ostn02Ptr = CSnewOstn02(ost02->filePath, 1);
#else
	ostn02Ptr = CSnewOstn02(ost02->filePath);
#endif
	if (ostn02Ptr != NULL)
	{
		ost02->fileObject.Ostn02 = ostn02Ptr;
		ost02->test = (cs_TEST_CAST)CSost02T;
		ost02->frwrd2D = (cs_FRWRD2D_CAST)CSost02F2;
		ost02->frwrd3D = (cs_FRWRD3D_CAST)CSost02F3;
		ost02->invrs2D = (cs_INVRS2D_CAST)CSost02I2;
		ost02->invrs3D = (cs_INVRS3D_CAST)CSost02I3;
		ost02->inRange = (cs_INRANGE_CAST)CSost02L;
		ost02->release = (cs_RELEASE_CAST)CSost02R;
		ost02->destroy = (cs_DESTROY_CAST)CSost02D;
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
double CSost02T (struct cs_Ostn02_ *ost02, double *ll_src, short direction)
{
	double density;

	density = CStestCoverage(&ost02->coverage, ll_src);
	return density;
}

int CSost02F2 (struct cs_Ostn02_ *ost02, double *ll_trg, Const double *ll_src)
{
	extern char csErrnam[];

	int st;
	double xy_src[2];
	double xy_trg[2];

	/* ll_src and ll_trg may be the same array, so we don't set ll_trg until
	   we know the status of the conversion. */
	st = CSosgb2XY(TRUE, xy_src, ll_src);
	if (st == 0)
	{
		st = CSforwardOstn02(ost02, xy_trg, xy_src);
		if (st == 0)
		{
			st = CSosgb2LL(FALSE, ll_trg, xy_trg);
		}
	}
	if (st != 0)
	{
		if (st > 0)
		{
			ll_trg[0] = ll_src[0];
			ll_trg[1] = ll_src[1];
		}
		else
		{
			CS_stncp(csErrnam, "CS_ost02::1", MAXPATH);
			CS_erpt(cs_ISER);
		}
	}
	return st;
}

int CSost02F3 (struct cs_Ostn02_ *ost02, double *ll_trg, Const double *ll_src)
{
	extern char csErrnam[];

	int st;
	double xy_src[2];
	double xy_trg[2];

	/* ll_src and ll_trg may be the same array, so we don't set ll_trg until
	   we know the status of the conversion. */
	st = CSosgb2XY(TRUE, xy_src, ll_src);
	if (st == 0)
	{
		st = CSforwardOstn02(ost02, xy_trg, xy_src);
		if (st == 0)
		{
			st = CSosgb2LL(FALSE, ll_trg, xy_trg);
			ll_trg[2] = ll_src[2];
		}
	}
	if (st != 0)
	{
		if (st > 0)
		{
			ll_trg[0] = ll_src[0];
			ll_trg[1] = ll_src[1];
			ll_trg[2] = ll_src[2];
		}
		else
		{
			CS_stncp(csErrnam, "CS_ost02::1", MAXPATH);
			CS_erpt(cs_ISER);
		}
	}
	return st;
}

int CSost02I2 (struct cs_Ostn02_ *ost02,double *ll_trg,Const double *ll_src)
{
	extern char csErrnam[];

	int st;
	double xy_src[2];
	double xy_trg[2];

	/* ll_src and ll_trg may be the same array, so we don't set ll_trg until
	   we know the status of the conversion. */
	st = CSosgb2XY(FALSE, xy_src, ll_src);
	if (st == 0)
	{
		st = CSinverseOstn02(ost02, xy_trg, xy_src);
		if (st == 0)
		{
			st = CSosgb2LL(TRUE, ll_trg, xy_trg);
		}
	}
	if (st != 0)
	{
		if (st > 0)
		{
			ll_trg[0] = ll_src[0];
			ll_trg[1] = ll_src[1];
		}
		else
		{
			CS_stncp(csErrnam, "CS_ost02::1", MAXPATH);
			CS_erpt(cs_ISER);
		}
	}
	return st;
}

int CSost02I3 (struct cs_Ostn02_ *ost02, double *ll_trg, Const double *ll_src)
{
	extern char csErrnam[];

	int st;
	double xy_src[2];
	double xy_trg[2];

	/* ll_src and ll_trg may be the same array, so we don't set ll_src until
	   we know the status of the conversion. */
	st = CSosgb2XY(FALSE, xy_src, ll_src);
	if (st == 0)
	{
		st = CSinverseOstn02(ost02, xy_trg, xy_src);
		if (st == 0)
		{
			st = CSosgb2LL(TRUE, ll_trg, xy_trg);
			ll_trg[2] = ll_src[2];
		}
	}
	if (st != 0)
	{
		if (st > 0)
		{
			ll_trg[0] = ll_src[0];
			ll_trg[1] = ll_src[1];
			ll_trg[2] = ll_src[2];
		}
		else
		{
			CS_stncp(csErrnam, "CS_ost02::1", MAXPATH);
			CS_erpt(cs_ISER);
		}
	}
	return st;
}

int CSost02L  (struct cs_Ostn02_ *ost02,int cnt,Const double pnts [][3])
{
	int idx;
	int status;
	double density;

	status = cs_CNVRT_OK;
	for (idx = 0; idx < cnt; idx += 1)
	{
		density = CSost02T(ost02, pnts[idx], cs_DTCDIR_FWD);
		if (fabs(density) < 1.0E-08)
		{
			status = cs_CNVRT_USFL;
			break;
		}
	}
	return status;
}

int CSost02Q  (struct csGeodeticXfromParmsFile_* fileParms,Const char* dictDir,int err_list [],int list_sz)
{
	extern char cs_DirsepC;

	int err_cnt;
	size_t rdCnt;

	char* cp;
	csFILE* strm;

	char chrBuffer[16];
	char pathBuffer[MAXPATH];

	cp = fileParms->fileName;
	if (*cp == '.' && *(cp + 1) == cs_DirsepC)
	{
		CS_stncp(pathBuffer, dictDir, sizeof(pathBuffer));
		CS_stncat(pathBuffer, cp, MAXPATH);
	}
	else
	{
		CS_stncp(pathBuffer, cp, MAXPATH);
	}

	/* We will return (err_cnt + 1) below. */
	err_cnt = -1;
	if (err_list == NULL) list_sz = 0;

	/* Verify that the file exists and that the format appears to be correct. */
	strm = CS_fopen(pathBuffer, _STRM_BINRD);
	if (strm != NULL)
	{
		rdCnt = CS_fread(chrBuffer, 1, sizeof(chrBuffer), strm);
		CS_fclose(strm);
		strm = NULL;

		if (rdCnt != sizeof(chrBuffer))
		{
			if (++err_cnt < list_sz) err_list[err_cnt] = cs_DTQ_FORMAT;
		}
	}
	else
	{
		if (++err_cnt < list_sz) err_list[err_cnt] = cs_DTQ_FILE;
	}
	return (err_cnt + 1);
}

int CSost02R  (struct cs_Ostn02_ *ost02)
{
	CSreleaseOstn02(ost02);
	return 0;
}

int CSost02D  (struct cs_Ostn02_ *ost02)
{
	CSdeleteOstn02(ost02);
	return 0;
}
