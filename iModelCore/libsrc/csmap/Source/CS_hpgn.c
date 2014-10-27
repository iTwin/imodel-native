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

/******************************************************************************
	These are static global variables, represent a possible  multi-threading
	problem.
*/
struct csNad83ToHarn_* csNad83ToHarn = NULL;
int csNad83ToHarnCnt = 0;

/**********************************************************************
**	st = CShpginit ();
**
**	int st;						returns 0 if the initialization proceeded to
**								completion successfully.  Otherwise returns
**								-1.
**********************************************************************/
int EXP_LVL7 CSharnInit (void)
{
	extern char *cs_DirP;
	extern char cs_Dir [];
	extern char cs_HarnName [];

	char catalog [MAXPATH];

	/* See if we already have a csNad83ToHarn object.
	   Note, we can have such even if the count is zero. */
	if (csNad83ToHarn == NULL)
	{
		/* Nope, we best create one. */
		CS_stcpy (cs_DirP,cs_HarnName);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		csNad83ToHarn = CSnewNad83ToHarn (catalog);
		if (csNad83ToHarn == NULL)
		{
			goto error;
		}
		csNad83ToHarnCnt = 0;
	}
	csNad83ToHarnCnt += 1;
	return 0;

error:
	if (csNad83ToHarn != NULL)
	{
		CSdeleteNad83ToHarn (csNad83ToHarn);
		csNad83ToHarn = NULL;
	}
	csNad83ToHarnCnt = 0;
	return -1;
}
/**********************************************************************
**	CSharnCls (void);
**
**	NOTE:  This function simply releases the resources, it doesn't
**	delete them.  This is for performance reasons.  This will cause a
**	memory leak message on some debuggers.  If this is undesirable,
**	replace CSreleaseNad83ToHarn with CSdeleteNad83ToHarn below, and
**	set the csNad83ToHarn pointer to NULL.
**********************************************************************/
void EXP_LVL9 CSharnCls (void)
{
	csNad83ToHarnCnt -= 1;
	if (csNad83ToHarnCnt <= 0)
	{
		CSdeleteNad83ToHarn (csNad83ToHarn);
		csNad83ToHarn = NULL;
		csNad83ToHarnCnt = 0;

        /**************************************************************
          Performance code below
		CSreleaseNad83ToHarn (csNad83ToHarn);
		csNad83ToHarnCnt = 0;
		**************************************************************/
	}
	return;
}
/**********************************************************************
**	status = CSnad83ToHarn (ll_harn,ll_83);
**
**	double ll_harn [3];			the converted longitude ([0]) and latitude ([1]) 
**								are returned here.
**	double ll_83 [3];			the longitude ([0]) and the latitude ([1]) to
**								be converted.
**	int status;					returns zero if conversion took place as expected,
**								+1 if conversion did not take place due to a
**								data block error, +2 if the fallback was used
**								successfully, -1 if conversion failed and
**								for any other reason.
**
**	ll_harn and ll_83 may point at the same array.
**
**	In the event of a +1 status return, ll_83 will have been copied
**	to ll_harn.
**
**	There is no way to determine a HGT change.  HGT element is
**	simply copied in all cases.
**********************************************************************/
int EXP_LVL7 CSnad83ToHarn (double ll_harn [3],Const double ll_83 [3])
{
	int status;
	double lcl_harn [3];

	/* Set the default return values.  We need this to allow both input
	   arrays to be the same physical memory. */
	lcl_harn [LNG] = ll_83 [LNG];
	lcl_harn [LAT] = ll_83 [LAT];
	lcl_harn [HGT] = ll_83 [HGT];

	/* Make sure we have been initialized. */
	if (csNad83ToHarn == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcNad83ToHarn (csNad83ToHarn,lcl_harn,ll_83);
	}

	/* Return the results. */
	if (status >= 0)
	{
		ll_harn [LNG] = lcl_harn [LNG];
		ll_harn [LAT] = lcl_harn [LAT];
		ll_harn [HGT] = lcl_harn [HGT];
	}
	return status;
}
/**********************************************************************
**	status = CSharnToNad83 (ll_83,ll_harn,err_flg);
**
**	double ll_83 [3];			the converted longitude ([0]) and latitude ([1]) 
**								are returned here.
**	double ll_harn [3];			the longitude ([0]) and the latitude ([1]) to
**								be converted.
**	int err_flg;				indicates what CSharn283 is to do in the event of an
**								error.  See remarks below.
**	int status;					returns zero on a successful conversion, +1
**								in the event of an out of range error, +2 if the
**								fallback was used successfully, -1 for a hard error.
**
**	ll_harn and ll_83 may point at the same array.
**
**	In the event of a +1 status return, ll_harn will have been copied
**	to ll_83.
**********************************************************************/
int EXP_LVL7 CSharnToNad83 (double ll_83 [3],Const double ll_harn [3])
{
	int status;
	double lcl_83 [3];

	/* Set the default return values.  We need this to allow both input
	   arrays to be the same physical memory. */
	lcl_83 [LNG] = ll_harn [LNG];
	lcl_83 [LAT] = ll_harn [LAT];
	lcl_83 [HGT] = ll_harn [HGT];

	/* Make sure we have been initialized. */
	if (csNad83ToHarn == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseNad83ToHarn (csNad83ToHarn,lcl_83,ll_harn);
	}

	/* Return the results. */
	if (status >= 0)
	{
		ll_83 [LNG] = lcl_83 [LNG];
		ll_83 [LAT] = lcl_83 [LAT];
		ll_83 [HGT] = lcl_83 [HGT];
	}
	return status;
}
Const char *EXP_LVL7 CSnad83ToHarnLog (Const double ll_83 [3])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csNad83ToHarn == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceNad83ToHarn (csNad83ToHarn,ll_83);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
