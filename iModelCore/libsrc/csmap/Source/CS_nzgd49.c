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

	Provides conversion of NZGD49 geodetic coordinates to NZGD2K
	coordinates.  Handles multiple files in the Canadian National
	Transformation, Version 2, Australian variation, format.
*/

#include "cs_map.h"

/******************************************************************************
	The following is used to maintain the status of the NZGD49<-->NZGD2K
	conversion system.  Namely, if its already opened, we don't have to
	open it again.  Upon close, we release resources, but don't actually
	destruct.  The counter keeps track of the number of opens.
*/
struct csNzgd49ToNzgd2K_* csNzgd49ToNzgd2K = NULL;
int csNzgd49ToNzgd2KCnt = 0;

/******************************************************************************
	Initialize the NZGD49 <--> NZGD2K conversion system.  The catalog file is
	expected to	reside in the basic data directory.  The name of the file is
	taken from the cs_Nzgd49Name global variable.
*/
int EXP_LVL7 CSnzgd49Init (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_Nzgd49Name [];

	char catalog [MAXPATH];

	if (csNzgd49ToNzgd2K == NULL)
	{
		/* Set up the catalog file name. */
		CS_stcpy (cs_DirP,cs_Nzgd49Name);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		/* Use it to build the Nzgd49ToNzgd2K object. */
		csNzgd49ToNzgd2K = CSnewNzgd49ToNzgd2K (catalog);
		if (csNzgd49ToNzgd2K == NULL) goto error;
	}
	csNzgd49ToNzgd2KCnt += 1;
	return 0;

error:
	if (csNzgd49ToNzgd2K != NULL)
	{
		CSdeleteNzgd49ToNzgd2K (csNzgd49ToNzgd2K);
		csNzgd49ToNzgd2K = NULL;
		csNzgd49ToNzgd2KCnt = 0;
	}
	return -1;
}

/******************************************************************************
	Close the NZGD49 <--> NZGD2K conversion system.  Note, this only does a
	release, not a full delete.  This is for performance reasons.  During
	test, you may get a "memory leak message" because of this.  If this is
	undesirable, then change the 'release' function to the 'delete' function.
*/
void EXP_LVL7 CSnzgd49Cls (void)
{
	csNzgd49ToNzgd2KCnt -= 1;
	if (csNzgd49ToNzgd2KCnt <= 0)
	{
		if (csNzgd49ToNzgd2K != NULL)
		{
			CSreleaseNzgd49ToNzgd2K (csNzgd49ToNzgd2K);
		}
		csNzgd49ToNzgd2KCnt = 0;
	}
	return;
}

/******************************************************************************
	Convert an NZGD49 coordinate to NZGD2K coordinate.
*/
int EXP_LVL7 CSnzgd49ToNzgd2K (double ll_nzgd2K [3],Const double ll_nzgd49 [3])
{
	int status;
	double my_ll2K [3];

	/* We always do the null conversion. */
	my_ll2K [LNG] = ll_nzgd49 [LNG];
	my_ll2K [LAT] = ll_nzgd49 [LAT];
	my_ll2K [HGT] = ll_nzgd49 [HGT];

	/* Do the real conversion, if possible. */
	if (csNzgd49ToNzgd2K == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcNzgd49ToNzgd2K (csNzgd49ToNzgd2K,my_ll2K,ll_nzgd49);
	}
	ll_nzgd2K [LNG] = my_ll2K [LNG];
	ll_nzgd2K [LAT] = my_ll2K [LAT];
	ll_nzgd2K [HGT] = my_ll2K [HGT];
	return status;
}

/******************************************************************************
	Convert an NZGD2K coordinate to NZGD49 coordinate. Computationally, the
	inverse of the above.
*/
int EXP_LVL7 CSnzgd2KToNzgd49 (double ll_nzgd49 [3],Const double ll_nzgd2K [3])
{
	int status;

	double my_ll49 [3];

	/* We always do the null conversion. */
	my_ll49 [LNG] = ll_nzgd2K [LNG];
	my_ll49 [LAT] = ll_nzgd2K [LAT];
	my_ll49 [HGT] = ll_nzgd2K [HGT];

	/* Do the real conversion, if possible. */
	if (csNzgd49ToNzgd2K == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseNzgd49ToNzgd2K (csNzgd49ToNzgd2K,my_ll49,ll_nzgd2K);
	}

	if (status >= 0)
	{
		ll_nzgd49 [LNG] = my_ll49 [LNG];
		ll_nzgd49 [LAT] = my_ll49 [LAT];
		ll_nzgd49 [HGT] = my_ll49 [HGT];
	}
	return status;
}
/**********************************************************************
**	CS_nzgd49Name (new_name);
**
**	char *new_name;				the name of the Nzgd49ToNzgd2K catalog
**								file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_nzgd49Name (Const char *newName)
{
	extern char cs_Nzgd49Name [];

	CS_stncp (cs_Nzgd49Name,newName,cs_FNM_MAXLEN);
	return;
}
