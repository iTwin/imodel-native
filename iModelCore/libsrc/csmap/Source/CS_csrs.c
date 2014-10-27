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
	The following is used to maintain the status of the NAD83<-->CSRS
	conversion system.  Namely, if its already opened, we don't have to
	open it again.  Upon close, we release resources, but don't actually
	destruct.  The counter keeps track of the number of opens.
*/
struct csNad83ToCsrs_* csNad83ToCsrs = NULL;
int csNad83ToCsrsCnt = 0;

/******************************************************************************
	Initialize the NAD83 <--> CSRS conversion system.  The catalog file is
	expected to	reside in the basic data directory.  The name of the file is
	taken from the cs_CsrsName global variable.
*/
int EXP_LVL7 CScsrsInit (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_CsrsName [];

	char catalog [MAXPATH];

	if (csNad83ToCsrs == NULL)
	{
		/* Set up the catalog file name. */
		CS_stcpy (cs_DirP,cs_CsrsName);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		/* Use it to build the Nad83ToCsrs object. */
		csNad83ToCsrs = CSnewNad83ToCsrs (catalog);
		if (csNad83ToCsrs == NULL) goto error;
	}
	csNad83ToCsrsCnt += 1;
	return 0;

error:
	if (csNad83ToCsrs != NULL)
	{
		CSdeleteNad83ToCsrs (csNad83ToCsrs);
		csNad83ToCsrs = NULL;
		csNad83ToCsrsCnt = 0;
	}
	return -1;
}

/******************************************************************************
	Close the NAD83 <--> CSRS conversion system.  Note, this only does a
	release, not a full delete.  This is for performance reasons.  During
	test, you may get a "memory leak message" because of this.  If this is
	undesirable, then change the 'release' function to the 'delete' function.
*/
void EXP_LVL7 CScsrsCls (void)
{
	csNad83ToCsrsCnt -= 1;
	if (csNad83ToCsrsCnt <= 0)
	{
		CSreleaseNad83ToCsrs (csNad83ToCsrs);
		csNad83ToCsrsCnt = 0;
	}
	return;
}

/******************************************************************************
	Convert a NAD83 coordinate to CSRS coordinate.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSnad83ToCsrs (double ll_csrs [3],Const double ll_nad83 [3])
{
	int status;
	double my_csrs [3];

	/* We always do the null conversion. */
	my_csrs [LNG] = ll_nad83 [LNG];
	my_csrs [LAT] = ll_nad83 [LAT];
	my_csrs [HGT] = ll_nad83 [HGT];

	/* Do the real conversion, if possible. */
	if (csNad83ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcNad83ToCsrs (csNad83ToCsrs,my_csrs,ll_nad83);
	}
	ll_csrs [LNG] = my_csrs [LNG];
	ll_csrs [LAT] = my_csrs [LAT];
	ll_csrs [HGT] = my_csrs [HGT];
	return status;
}

/******************************************************************************
	Convert an CSRS coordinate to NAD83 coordinate. Computationally, the
	inverse of the above.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CScsrsToNad83 (double ll_nad83 [3],Const double ll_csrs [3])
{
	int status;

	double my_nad83 [3];

	/* We always do the null conversion. */
	my_nad83 [LNG] = ll_csrs [LNG];
	my_nad83 [LAT] = ll_csrs [LAT];
	my_nad83 [HGT] = ll_csrs [HGT];

	/* Do the real conversion, if possible. */
	if (csNad83ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseNad83ToCsrs (csNad83ToCsrs,my_nad83,ll_csrs);
	}

	if (status >= 0)
	{
		ll_nad83 [LNG] = my_nad83 [LNG];
		ll_nad83 [LAT] = my_nad83 [LAT];
		ll_nad83 [HGT] = my_nad83 [HGT];
	}
	return status;
}
Const char * EXP_LVL7 CSnad83ToCsrsLog (Const double ll_83 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csNad83ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceNad83ToCsrs (csNad83ToCsrs,ll_83);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
/**********************************************************************
**	CS_csrsName (new_name);
**
**	char *new_name;				the name of the Nad83ToCsrs catalog
**								file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_csrsName (Const char *newName)
{
	extern char cs_CsrsName [];

	CS_stncp (cs_CsrsName,newName,cs_FNM_MAXLEN);
	return;
}

/******************************************************************************
*******************************************************************************
***                                                                         ***
***         NAD27 to CSRS Direct                                            ***
***                                                                         ***
*******************************************************************************
******************************************************************************/
/******************************************************************************
	The following is used to maintain the status of the NAD27<-->CSRS
	conversion system.  Namely, if its already opened, we don't have to
	open it again.  Upon close, we release resources, but don't actually
	destruct.  The counter keeps track of the number of opens.
*/
struct csNad27ToCsrs_* csNad27ToCsrs = NULL;
int csNad27ToCsrsCnt = 0;
/******************************************************************************
	Initialize the NAD27 <--> CSRS conversion system.  The catalog file is
	expected to	reside in the basic data directory.  The name of the file is
	taken from the cs_CsrsName global variable.
*/
int EXP_LVL7 CScsrs27Init (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_Csrs27Name [];

	char catalog [MAXPATH];

	if (csNad27ToCsrs == NULL)
	{
		/* Set up the catalog file name. */
		CS_stcpy (cs_DirP,cs_Csrs27Name);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		/* Use it to build the Nad83ToCsrs object. */
		csNad27ToCsrs = CSnewNad27ToCsrs (catalog);
		if (csNad27ToCsrs == NULL) goto error;
	}
	csNad27ToCsrsCnt += 1;
	return 0;

error:
	if (csNad27ToCsrs != NULL)
	{
		CSdeleteNad27ToCsrs (csNad27ToCsrs);
		csNad27ToCsrs = NULL;
		csNad27ToCsrsCnt = 0;
	}
	return -1;
}

/******************************************************************************
	Close the NAD27 <--> CSRS conversion system.  Note, this only does a
	release, not a full delete.  This is for performance reasons.  During
	test, you may get a "memory leak message" because of this.  If this is
	undesirable, then change the 'release' function to the 'delete' function.
*/
void EXP_LVL7 CScsrs27Cls (void)
{
	csNad27ToCsrsCnt -= 1;
	if (csNad27ToCsrsCnt <= 0)
	{
		CSreleaseNad27ToCsrs (csNad27ToCsrs);
		csNad27ToCsrsCnt = 0;
	}
	return;
}

/******************************************************************************
	Convert a NAD27 coordinate to CSRS coordinate.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSnad27ToCsrs (double ll_csrs [3],Const double ll_nad27 [3])
{
	int status;
	double my_csrs [3];

	/* We always do the null conversion. */
	my_csrs [LNG] = ll_nad27 [LNG];
	my_csrs [LAT] = ll_nad27 [LAT];
	my_csrs [HGT] = ll_nad27 [HGT];

	/* Do the real conversion, if possible. */
	if (csNad27ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcNad27ToCsrs (csNad27ToCsrs,my_csrs,ll_nad27);
	}
	ll_csrs [LNG] = my_csrs [LNG];
	ll_csrs [LAT] = my_csrs [LAT];
	ll_csrs [HGT] = my_csrs [HGT];
	return status;
}

/******************************************************************************
	Convert an CSRS coordinate to NAD27 coordinate. Computationally, the
	inverse of the above.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CScsrsToNad27 (double ll_nad27 [3],Const double ll_csrs [3])
{
	int status;

	double my_nad27 [3];

	/* We always do the null conversion. */
	my_nad27 [LNG] = ll_csrs [LNG];
	my_nad27 [LAT] = ll_csrs [LAT];
	my_nad27 [HGT] = ll_csrs [HGT];

	/* Do the real conversion, if possible. */
	if (csNad27ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseNad27ToCsrs (csNad27ToCsrs,my_nad27,ll_csrs);
	}

	if (status >= 0)
	{
		ll_nad27 [LNG] = my_nad27 [LNG];
		ll_nad27 [LAT] = my_nad27 [LAT];
		ll_nad27 [HGT] = my_nad27 [HGT];
	}
	return status;
}
Const char * EXP_LVL7 CSnad27ToCsrsLog (Const double ll_27 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csNad27ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceNad27ToCsrs (csNad27ToCsrs,ll_27);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
/**********************************************************************
**	CS_csrsName (new_name);
**
**	char *new_name;				the name of the Nad83ToCsrs catalog
**								file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_csrs27Name (Const char *newName)
{
	extern char cs_Csrs27Name [];

	CS_stncp (cs_Csrs27Name,newName,cs_FNM_MAXLEN);
	return;
}
