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
struct csNad27ToNad83_* csNad27ToNad83 = NULL;
int csNad27ToNad83Cnt = 0;

/**********************************************************************
**	st = CSnadInit ();
**
**	int st;						returns 0 if the initialization proceeded to
**								completion successfully.  Otherwise,
**								returns -1.
**********************************************************************/
int EXP_LVL7 CSnadInit (void)
{
	extern char *cs_DirP;
	extern char cs_Dir [];
	extern char cs_NadName [];

	char catalog [MAXPATH];

	/* See if we already have a csNad27ToNad83 object.
	   Note, we can have such even if the count is zero. */
	if (csNad27ToNad83 == NULL)
	{
		/* Nope, we best create one. */
		CS_stcpy (cs_DirP,cs_NadName);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		csNad27ToNad83 = CSnewNad27ToNad83 (catalog);
		if (csNad27ToNad83 == NULL)
		{
			goto error;
		}
		csNad27ToNad83Cnt = 0;
	}
	csNad27ToNad83Cnt += 1;
	return 0;

error:
	if (csNad27ToNad83 != NULL)
	{
		CSdeleteNad27ToNad83 (csNad27ToNad83);
		csNad27ToNad83 = NULL;
	}
	csNad27ToNad83Cnt = 0;
	return -1;
}
/**********************************************************************
**	CSnadCls (void);
**
**	NOTE:  This function simply releases the resources, it doesn't
**	delete them.  This is for performance reasons.  This will cause a
**	memory leak message on some debuggers.  If this is undesirable,
**	replace CSreleaseNad27ToNad83 with CSdeleteNad27ToNad83 below, and
**	set the csNad27ToNad83 pointer to NULL.
**********************************************************************/
void EXP_LVL9 CSnadCls (void)
{
	/* If csNad27ToNad83 is null, we should never be called.  However,
	   rather than crash: */
	if (csNad27ToNad83 == NULL)
	{
		csNad27ToNad83Cnt = 0;
		return;
	}

	csNad27ToNad83Cnt -= 1;
	if (csNad27ToNad83Cnt <= 0)
	{
		// Alternate selected.
		CSdeleteNad27ToNad83 (csNad27ToNad83);
		csNad27ToNad83 = NULL;
		csNad27ToNad83Cnt = 0;

        /**************************************************************
          Performance code below
        CSreleaseNad27ToNad83 (csNad27ToNad83);
		csNad27ToNad83Cnt = 0;
		**************************************************************/

		/**************************************************************
		  Alternative code, commented out.
		CSdeleteNad27ToNad83 (csNad27ToNad83);
		csNad27ToNad83 = NULL;
		csNad27ToNad83Cnt = 0;
		**************************************************************/
	}
	return;
}
/**********************************************************************
**	status = CSnad27ToNad83 (ll_83,ll_27);
**
**	double ll_83 [3];			the converted longitude ([0]) and latitude ([1]) 
**								are returned here.
**	double ll_27 [3];			the longitude ([0]) and the latitude ([1]) to
**								be converted.
**	int status;					returns zero if conversion took place as expected,
**								+1 if conversion failed due to out of range
**								error, +2 if the fallback was used successfully,]
**								-1 for all other errors.
**
**	ll_83 and ll_27 may point at the same array.
**
**	In the event of a +1 return value, ll_27 will have been copied
**	to ll_83
**********************************************************************/
int EXP_LVL7 CSnad27ToNad83 (double ll_83 [3],Const double ll_27 [3])
{
	int status;
	double lcl_83 [3];

	/* Set the default return values.  We need this to allow both input
	   arrays to be the same physical memory. */
	lcl_83 [LNG] = ll_27 [LNG];
	lcl_83 [LAT] = ll_27 [LAT];

	/* Make sure we have been initialized. */
	if (csNad27ToNad83 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcNad27ToNad83 (csNad27ToNad83,lcl_83,ll_27);
	}

	/* Return the results. */
	if (status >= 0)
	{
		ll_83 [LNG] = lcl_83 [LNG];
		ll_83 [LAT] = lcl_83 [LAT];
	}
	return status;
}
/**********************************************************************
**	status = CSnad83ToNad27 (ll_27,ll_83,err_flg);
**
**	double ll_27 [3];			the converted longitude ([0]) and latitude ([1]) 
**								are returned here.
**	double ll_83 [3];			the longitude ([0]) and the latitude ([1]) to
**								be converted.
**	int status;					returns zero on a successful conversion, a
**								positive one if a range problem (i.e.
**								outside the range of coverage), +2 if the
**								fallback was used successfully, or -1 for
**								any other type of error.
**
**	ll_27 and ll_83 may point at the same array.
**
**	In the event of a +1 return value, ll_83 will have been copied to
**	ll_27.
**********************************************************************/
int EXP_LVL7 CSnad83ToNad27 (double ll_27 [3],Const double ll_83 [3])
{
	int status;
	double lcl_27 [3];

	/* Set the default return values.  We need this to allow both input
	   arrays to be the same physical memory. */
	lcl_27 [LNG] = ll_83 [LNG];
	lcl_27 [LAT] = ll_83 [LAT];

	/* Make sure we have been initialized. */
	if (csNad27ToNad83 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseNad27ToNad83 (csNad27ToNad83,lcl_27,ll_83);
	}

	/* Return the results. */
	if (status >= 0)
	{
		ll_27 [LNG] = lcl_27 [LNG];
		ll_27 [LAT] = lcl_27 [LAT];
	}
	return status;
}
Const char * EXP_LVL7 CSnad27ToNad83Log (Const double ll_27 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csNad27ToNad83 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceNad27ToNad83 (csNad27ToNad83,ll_27);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/
/******************************************************************************
	These are static global variables, represent a possible  multi-threading
	problem.
*/
struct csNad27ToAts77_* csNad27ToAts77 = NULL;
int csNad27ToAts77Cnt = 0;

/**********************************************************************
**	st = CSn27a77Init ();
**********************************************************************/
int EXP_LVL7 CSn27a77Init (void)
{
	extern char *cs_DirP;
	extern char cs_Dir [];
	extern char cs_N27A77Name [];

	char catalog [MAXPATH];

	/* See if we already have a csNad27ToNad83 object.
	   Note, we can have such even if the count is zero. */
	if (csNad27ToAts77 == NULL)
	{
		/* Nope, we best create one. */
		CS_stcpy (cs_DirP,cs_N27A77Name);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		csNad27ToAts77 = CSnewNad27ToAts77 (catalog);
		if (csNad27ToAts77 == NULL)
		{
			goto error;
		}
		csNad27ToAts77Cnt = 0;
	}
	csNad27ToAts77Cnt += 1;
	return 0;

error:
	if (csNad27ToAts77 != NULL)
	{
		CSdeleteNad27ToAts77 (csNad27ToAts77);
		csNad27ToAts77 = NULL;
	}
	csNad27ToAts77Cnt = 0;
	return -1;
}
/**********************************************************************
**	CSn27a77Cls (void);
**********************************************************************/
void EXP_LVL9 CSn27a77Cls (void)
{
	/* If csNad27ToAts77 is null, we should never be called.  However,
	   rather than crash: */
	if (csNad27ToAts77 == NULL)
	{
		csNad27ToAts77Cnt = 0;
		return;
	}

	csNad27ToNad83Cnt -= 1;
	if (csNad27ToAts77Cnt <= 0)
	{
		/* Alternate code selected. */
		CSdeleteNad27ToNad83 (csNad27ToNad83);
		csNad27ToNad83 = NULL;
		csNad27ToNad83Cnt = 0;

		/**************************************************************
          Performance code below
		CSreleaseNad27ToAts77 (csNad27ToAts77);
		csNad27ToAts77Cnt = 0;
		**************************************************************/

		/**************************************************************
		  Alternative code, commented out.
		CSdeleteNad27ToNad83 (csNad27ToNad83);
		csNad27ToNad83 = NULL;
		csNad27ToNad83Cnt = 0;
		**************************************************************/
	}
	return;
}
/**********************************************************************
**	status = CSnad27ToAts77 (ll_ats77,ll_nad27);
**********************************************************************/
int EXP_LVL7 CSnad27ToAts77 (double ll_ats77 [3],Const double ll_nad27 [3])
{
	int status;
	double lcl_77 [3];

	/* Set the default return values.  We need this to allow both input
	   arrays to be the same physical memory. */
	lcl_77 [LNG] = ll_nad27 [LNG];
	lcl_77 [LAT] = ll_nad27 [LAT];

	/* Make sure we have been initialized. */
	if (csNad27ToAts77 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcNad27ToAts77 (csNad27ToAts77,lcl_77,ll_nad27);
	}

	/* Return the results. */
	if (status >= 0)
	{
		ll_ats77 [LNG] = lcl_77 [LNG];
		ll_ats77 [LAT] = lcl_77 [LAT];
	}
	return status;
}
/**********************************************************************
**	status = CSats77ToNad27 (ll_nad27,ll_ats77,err_flg);
**********************************************************************/
int EXP_LVL7 CSats77ToNad27 (double ll_nad27 [3],Const double ll_ats77 [3])
{
	int status;
	double lcl_27 [3];

	/* Set the default return values.  We need this to allow both input
	   arrays to be the same physical memory. */
	lcl_27 [LNG] = ll_ats77 [LNG];
	lcl_27 [LAT] = ll_ats77 [LAT];

	/* Make sure we have been initialized. */
	if (csNad27ToAts77 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcAts77ToNad27 (csNad27ToAts77,lcl_27,ll_ats77);
	}

	/* Return the results. */
	if (status >= 0)
	{
		ll_nad27 [LNG] = lcl_27 [LNG];
		ll_nad27 [LAT] = lcl_27 [LAT];
	}
	return status;
}
Const char * EXP_LVL7 CSnad27ToAts77Log (Const double ll_27 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csNad27ToAts77 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceNad27ToAts77 (csNad27ToAts77,ll_27);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
