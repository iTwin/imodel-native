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
	The following is used to maintain the status of the DHDN<-->ETRF89
	conversion system.  Namely, if its already opened, we don't have to
	open it again.  Upon close, we release resources, but don't actually
	destruct.  The counter keeps track of the number of opens.
*/
struct csDhdnToEtrf89_* csDhdnToEtrf89 = NULL;
int csDhdnToEtrf89Cnt = 0;

/******************************************************************************
	Initialize the DHDN <--> ETRF conversion system.  The catalog file is
	expected to	reside in the basic data directory.
*/
int EXP_LVL7 CSdhdnInit (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_DhdnName [];

	char catalog [MAXPATH];

	if (csDhdnToEtrf89 == NULL)
	{
		/* Set up the catalog file name.  Need a copy of it as the
		   new function will access CS_dtloc. */
		CS_stcpy (cs_DirP,cs_DhdnName);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		/* Use it to build the DhdnToEtrf89 object. */
		csDhdnToEtrf89 = CSnewDhdnToEtrf89 (catalog);
		if (csDhdnToEtrf89 == NULL) goto error;
	}
	csDhdnToEtrf89Cnt += 1;
	return 0;

error:
	if (csDhdnToEtrf89 != NULL)
	{
		CSdeleteDhdnToEtrf89 (csDhdnToEtrf89);
		csDhdnToEtrf89 = NULL;
		csDhdnToEtrf89Cnt = 0;
	}
	return -1;
}

/******************************************************************************
	Close the AGD66 <--> GDA94 conversion system.  Note, this only does a
	release, not a full delete.  This is for performance reasons.  During
	test, you may get a "memory leak message" because of this.  If this is
	undeirable, then change the 'release' function to the 'delete' function.
*/
void EXP_LVL7 CSdhdnCls (void)
{
	csDhdnToEtrf89Cnt -= 1;
	if (csDhdnToEtrf89Cnt <= 0)
	{
		if (csDhdnToEtrf89 != NULL)
		{
			CSreleaseDhdnToEtrf89 (csDhdnToEtrf89);
		}
		csDhdnToEtrf89Cnt = 0;
	}
	return;
}

/******************************************************************************
	Convert an AGD66 coordinate to GDA94 coordinate.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSdhdnToEtrf89 (double ll_etrf89 [3],Const double ll_dhdn [3])
{
	int status;
	double my_ll89 [3];

	/* We always do the null conversion. */
	my_ll89 [LNG] = ll_dhdn [LNG];
	my_ll89 [LAT] = ll_dhdn [LAT];
	my_ll89 [HGT] = ll_dhdn [HGT];

	/* Do the real conversion, if possible. */
	if (csDhdnToEtrf89 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcDhdnToEtrf89 (csDhdnToEtrf89,my_ll89,ll_dhdn);
	}
	ll_etrf89 [LNG] = my_ll89 [LNG];
	ll_etrf89 [LAT] = my_ll89 [LAT];
	ll_etrf89 [HGT] = my_ll89 [HGT];
	return status;
}

/******************************************************************************
	Convert an GDA94 coordinate to AGD66 coordinate. Computationally, the
	inverse of the above.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSetrf89ToDhdn (double ll_dhdn [3],Const double ll_etrf89 [3])
{
	int status;

	double my_ll50 [3];

	/* We always do the null conversion. */
	my_ll50 [LNG] = ll_etrf89 [LNG];
	my_ll50 [LAT] = ll_etrf89 [LAT];
	my_ll50 [HGT] = ll_etrf89 [HGT];

	/* Do the real conversion, if possible. */
	if (csDhdnToEtrf89 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseDhdnToEtrf89 (csDhdnToEtrf89,my_ll50,ll_etrf89);
	}

	if (status >= 0)
	{
		ll_dhdn [LNG] = my_ll50 [LNG];
		ll_dhdn [LAT] = my_ll50 [LAT];
		ll_dhdn [HGT] = my_ll50 [HGT];
	}
	return status;
}
Const char * EXP_LVL7 CSdhdnToEtrf89Log (Const double ll_50 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csDhdnToEtrf89 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceDhdnToEtrf89 (csDhdnToEtrf89,ll_50);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
/**********************************************************************
**	CS_dhdnName (new_name);
**
**	char *new_name;				the name of the DhdnToEtrf89 catalog
**								file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_dhdnName (Const char *newName)
{
	extern char cs_DhdnName [];

	CS_stncp (cs_DhdnName,newName,cs_FNM_MAXLEN);
	return;
}
