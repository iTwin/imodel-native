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
	The following is used to maintain the status of the CH1903<-->CH1903+
	conversion system.  Namely, if its already opened, we don't have to
	open it again.  Upon close, we release resources, but don't actually
	destruct.  The counter keeps track of the number of opens.
*/
struct csCh1903ToPlus_* csCh1903ToPlus = NULL;
int csCh1903ToPlusCnt = 0;

/******************************************************************************
	Initialize the CH1903 <--> CH1903+ conversion system.  The catalog file is
	expected to	reside in the basic data directory.
*/
int EXP_LVL7 CSch1903Init (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_Ch1903Name [];

	char catalog [MAXPATH];

	if (csCh1903ToPlus == NULL)
	{
		/* Set up the catalog file name.  Need a copy of it as the
		   new function will access CS_dtloc. */
		CS_stcpy (cs_DirP,cs_Ch1903Name);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		/* Use it to build the Ch1903ToPlus object. */
		csCh1903ToPlus = CSnewCh1903ToPlus (catalog);
		if (csCh1903ToPlus == NULL) goto error;
	}
	csCh1903ToPlusCnt += 1;
	return 0;

error:
	if (csCh1903ToPlus != NULL)
	{
		CSdeleteCh1903ToPlus (csCh1903ToPlus);
		csCh1903ToPlus = NULL;
		csCh1903ToPlusCnt = 0;
	}
	return -1;
}

/******************************************************************************
	Close the CH1903 <--> CH1903+ conversion system.  Note, this only does a
	release, not a full delete.  This is for performance reasons.  During
	test, you may get a "memory leak message" because of this.  If this is
	undesirable, then change the 'release' function to the 'delete' function.
*/
void EXP_LVL7 CSch1903Cls (void)
{
	csCh1903ToPlusCnt -= 1;
	if (csCh1903ToPlusCnt <= 0)
	{
		if (csCh1903ToPlus != NULL)
		{
			CSreleaseCh1903ToPlus (csCh1903ToPlus);
		}
		csCh1903ToPlusCnt = 0;
	}
	return;
}

/******************************************************************************
	Convert a CH1903 coordinate to CH1903+ coordinate.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSch1903ToPlus (double llPlus [3],Const double llCh1903 [3])
{
	int status;
	double myPlus [3];

	/* We always do the null conversion. */
	myPlus [LNG] = llCh1903 [LNG];
	myPlus [LAT] = llCh1903 [LAT];
	myPlus [HGT] = llCh1903 [HGT];

	/* Do the real conversion, if possible. */
	if (csCh1903ToPlus == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcCh1903ToPlus (csCh1903ToPlus,myPlus,llCh1903);
	}
	llPlus [LNG] = myPlus [LNG];
	llPlus [LAT] = myPlus [LAT];
	llPlus [HGT] = myPlus [HGT];
	return status;
}

/******************************************************************************
	Convert an CH1903+ coordinate to a Ch1903 coordinate. Computationally, the
	inverse of the above.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSplusToCh1903 (double llCh1903 [3],Const double llPlus [3])
{
	int status;

	double myCh1903 [3];

	/* We always do the null conversion. */
	myCh1903 [LNG] = llPlus [LNG];
	myCh1903 [LAT] = llPlus [LAT];
	myCh1903 [HGT] = llPlus [HGT];

	/* Do the real conversion, if possible. */
	if (csCh1903ToPlus == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseCh1903ToPlus (csCh1903ToPlus,myCh1903,llPlus);
	}

	if (status >= 0)
	{
		llCh1903 [LNG] = myCh1903 [LNG];
		llCh1903 [LAT] = myCh1903 [LAT];
		llCh1903 [HGT] = myCh1903 [HGT];
	}
	return status;
}
Const char * EXP_LVL7 CSch1903ToPlusLog (Const double llCh1903 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csCh1903ToPlus == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceCh1903ToPlus (csCh1903ToPlus,llCh1903);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
/**********************************************************************
**	CS_ch1903Name (new_name);
**
**	char *new_name;				the name of the Ch1903ToPlus catalog
**								file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_ch1903Name (Const char *newName)
{
	extern char cs_Ch1903Name [];

	CS_stncp (cs_Ch1903Name,newName,cs_FNM_MAXLEN);
	return;
}
