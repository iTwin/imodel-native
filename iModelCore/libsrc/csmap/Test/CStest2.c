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
#include "cs_Test.h"

__ALIGNMENT__11				/* Required by some Sun compilers. */
static struct tst_lst_ dt_tst [] =
{
	{   "ZANDERIJ",NULL,NULL},	/* last one */
	{      "NAD27",NULL,NULL},
	{      "NAD83",NULL,NULL},
	{   "WGS72-BW",NULL,NULL},
	{     "PSAD56",NULL,NULL},
	{      "OLDHI",NULL,NULL},
	{      "WGS72",NULL,NULL},
	{   "TIMBALAI",NULL,NULL},
	{    "ADINDAN",NULL,NULL},	/* first one */
	{           "",NULL,NULL}
};

extern int cs_Error;
extern int cs_Errno;
extern int csErrlng;
extern int csErrlat;
extern unsigned short cs_ErrSup;
#if _RUN_TIME <= _rt_UNIXPCC
extern ulong32_t cs_Doserr;
#endif

int CStest2 (int verbose,int crypt)
{
	extern short cs_Protect;
	extern char cs_Unique;

	short protectSave;
	char uniqueSave;

	int st;
	int cnt;
	int my_crypt;

	csFILE *dtStrm;
	struct tst_lst_ *tp;

	char last_name [48];

	__ALIGNMENT__1			/* Required by some Sun compilers. */
	struct cs_Dtdef_ dt_def;

	protectSave = cs_Protect;
	uniqueSave = cs_Unique;
	printf ("Testing Datum Dictionary functions.\n");

	/* We do the same thing now with datum definitions. */

	tp = dt_tst;
	while (tp->name [0] != '\0')
	{
		cs_Protect = protectSave;
		cs_Unique = uniqueSave;
		if (verbose)
		{
			printf ("Fetching %s datum definition.",tp->name);
		}
		tp->dt_ptr = CS_dtdef (tp->name);
		if (tp->dt_ptr == NULL)
		{
			(void)printf ("\nCS_dtdef failed on %s. [%d:%d]\n",
							tp->name,
							cs_Error,
							cs_Errno);
			return (1);
		}
		if (verbose) printf (" OK.\n");

		if (CS_stricmp (tp->dt_ptr->group,"tESt"))
		{
			if (verbose)
			{
				printf ("Updating %s datum definition, protection off.",tp->name);
			}
			cs_Protect = -1;
			st = CS_dtupd (tp->dt_ptr,crypt);
			if (st != 1)
			{
				(void)printf ("\nCS_dtupd failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");
			if (verbose)
			{
				printf ("Deleting %s datum definition.",tp->name);
			}
			st = CS_dtdel (tp->dt_ptr);
			if (st != 0)
			{
				(void)printf ("CS_dtdel failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");
		}
		else
		{
			if (verbose)
			{
				printf ("Updating %s datum definition, protection on.",tp->name);
			}
			cs_Protect = 10000;
			st = CS_dtupd (tp->dt_ptr,crypt);
			if (st != 1)
			{
				(void)printf ("\nCS_dtupd failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");
			if (verbose)
			{
				printf ("Deleting %s datum definition.",tp->name);
			}
			st = CS_dtdel (tp->dt_ptr);
			if (st != 0)
			{
				(void)printf ("CS_dtdel failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");
		}
		tp++;
	}
	tp = dt_tst;
	while (tp->name [0] != '\0')
	{
		cs_Protect = protectSave;
		cs_Unique = uniqueSave;
		if (verbose)
		{
			printf ("Putting %s datum definition back in.",tp->name);
		}
		cs_Unique = '\0';
		st = CS_dtupd (tp->dt_ptr,crypt);
		if (st != 0)
		{
			(void)printf ("\nCS_dtupd (2) failed on %s. [%d:%d]\n",
							tp->name,
							cs_Error,
							cs_Errno);
			return (1);
		}
		if (verbose) printf (" OK.\n");
		CS_free (tp->dt_ptr);
		tp++;
	}
	if (verbose)
	{
		printf ("Checking the order of the Datum Dictionary.\n");
	}
	cnt = 0;
	dtStrm = CS_dtopn (_STRM_BINRD);
	st = CS_dtrd (dtStrm,&dt_def,&my_crypt);
	CS_stncp (last_name,dt_def.key_nm,sizeof (last_name));
	while (CS_dtrd (dtStrm,&dt_def,&my_crypt))
	{
		cnt += 1;
		if (verbose && (cnt % 4) == 0) putchar ('.');
		st = CS_stricmp (last_name,dt_def.key_nm);
		if (st > 0)
		{
			printf ("\nDatum Dictionary no longer sorted!!!\n");
			CS_fclose (dtStrm);
			return (1);
		}
		else if (st == 0)
		{
			printf ("\nDuplicate entries in Datum Dictionary!!!\n");
			CS_fclose (dtStrm);
			return (1);
		}
	}
	if (verbose) printf (" OK.\n");
	CS_dtDictCls (dtStrm);
	return (0);
}
