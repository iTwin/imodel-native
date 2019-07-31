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

#include "csTestCpp.hpp"

// The following table is an arbitrary list of ellipsoids that are used in
// the test.  There are two that should be special, indicated by "first"
// and "last".  These should be the first and last definitions in the
// dictionary: by name, alphabetically, collated by a simple strcmp
// compare function.  Encryption does not affect the collating
// order.
__ALIGNMENT__10				/* Required by some Sun compilers. */
static struct tst_lst_ el_tst [] =
{
	{     "APL4-5",NULL,NULL,NULL},	/* Try to keep set to the first one */
	{     "CLRK66",NULL,NULL,NULL},
	{   "CLRK-IGN",NULL,NULL,NULL},
	{      "INTNL",NULL,NULL,NULL},
	{     "CLRK80",NULL,NULL,NULL},
	{      "WGS84",NULL,NULL,NULL},
	{    "GRS1980",NULL,NULL,NULL},
	{     "Xian80",NULL,NULL,NULL},	/* Try to keep set to the last one */
	{           "",NULL,NULL}
};

extern "C"
{
extern int cs_Error;
extern int cs_Errno;
extern int csErrlng;
extern int csErrlat;
extern unsigned short cs_ErrSup;

#if _RUN_TIME <= _rt_UNIXPCC
extern ulong32_t cs_Doserr;
#endif

extern short cs_Protect;
extern char cs_Unique;
}

int CStest1 (bool verbose,bool crypt)
{
	short protectSave;
	char uniqueSave;

	int st;
	int cnt;
	int my_crypt;

	csFILE *elStrm;
	struct tst_lst_ *tp;

	char last_name [48];

	__ALIGNMENT__1			/* Required by some Sun compilers. */
	struct cs_Eldef_ el_def;

	protectSave = cs_Protect;
	uniqueSave = cs_Unique;
	printf ("Testing Ellipsoid Dictionary functions.\n");

	/* Test the Ellipsoid Dictionary manipulation functions
	   by fetching, updating, deleteing, and updating again
	   all the ellipsoids in the test list structure for
	   ellipsoids.  Most of these were all chosen as ellipsoids
	   used in the coordinate calculation test.  Thus, the
	   validity of the results can be tested by using test 4
	   after the completion of this test. */
	tp = el_tst;
	while (tp->name [0] != '\0')
	{
		cs_Protect = protectSave;
		cs_Unique = uniqueSave;

		/* First we fetch the ellipsoid definition. */
		if (verbose)
		{
			printf ("Fetching %s ellipsoid definition.",tp->name);
		}
		tp->el_ptr = CS_eldef (tp->name);
		if (tp->el_ptr == NULL)
		{
			(void)printf ("CS_eldef failed on %s. [%d:%d]\n",tp->name,cs_Error,cs_Errno);
			return (1);
		}
		if (verbose) printf (" OK.\n");

		/* There are two tests.  One with protection on, one with it  off.
		   If this is not a TEST ellipsoid,  then we can only do an update
		   with protection off.  If it is a TEST ellipsoid, we can test
		   with protection on. */
		if (CS_stricmp (tp->el_ptr->group,"teSt"))
		{
			/* Now we put it back in, with protection off. */
			if (verbose)
			{
				printf ("Updating %s ellipsoid definition, protection off.",tp->name);
			}
			cs_Protect = -1;
			st = CS_elupd (tp->el_ptr,crypt);
			if (st != 1)
			{
				(void)printf ("\nCS_elupd failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");

			/* Now we delete it. */
			if (verbose)
			{
				printf ("Deleting %s ellipsoid definition, protection off.",tp->name);
			}
			st = CS_eldel (tp->el_ptr);
			if (st != 0)
			{
				(void)printf ("\nCS_eldel failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");
		}
		else
		{
			/* Now we put it back in, with protection on. */
			if (verbose)
			{
				printf ("Updating %s ellipsoid definition, protection on.",tp->name);
			}
			cs_Protect = 10000;						/* Sets protect date 27 years into the future. */
			st = CS_elupd (tp->el_ptr,crypt);
			if (st != 1)
			{
				(void)printf ("\nCS_elupd failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");

			/* Now we delete it. */
			if (verbose)
			{
				printf ("Deleting %s ellipsoid definition, protection on.",tp->name);
			}
			st = CS_eldel (tp->el_ptr);
			if (st != 0)
			{
				(void)printf ("\nCS_eldel failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");
		}
		tp++;
	}

	/* Now we put all the deleted ellipsoid definitions back in.
	   This is a test of the add case since they shouldn't be
	   there any more. */
	tp = el_tst;
	while (tp->name [0] != '\0')
	{
		cs_Protect = protectSave;
		cs_Unique = uniqueSave;
		if (verbose)
		{
			printf ("Putting %s ellipsoid definition back in.",tp->name);
		}

		/* I can't really do a prtected test without changeing the name of the
		   definition.  So, we're stuck with testing with protection off. */
		cs_Unique = '\0';
		st = CS_elupd (tp->el_ptr,crypt);
		if (st != 0)
		{
			(void)printf ("\nCS_elupd failed on %s at line %d. [%d:%d]\n",tp->name,(__LINE__-3),cs_Error,cs_Errno);
			return (st);
		}
		if (verbose) printf (" OK.\n");
		CS_free (tp->el_ptr);
		tp++;
	}

	/* Finally, we cruise through the ellipsoid file and verify
	   that all are in sorted order and that there are no
	   duplicates.  */
	if (verbose)
	{
		printf ("Checking order of Ellipsoid Dictionary.\n");
	}
	cnt = 0;
	elStrm = CS_elopn (_STRM_BINRD);
	st = CS_elrd (elStrm,&el_def,&my_crypt);
	CS_stncp (last_name,el_def.key_nm,sizeof (last_name));
	while (CS_elrd (elStrm,&el_def,&my_crypt))
	{
		cnt += 1;
		if (verbose && (cnt % 3) == 0) putchar ('.');
		st = CS_stricmp (last_name,el_def.key_nm);
		if (st > 0)
		{
			printf ("\nEllipsoid Dictionary no longer sorted!!!\n");
			CS_fclose (elStrm);
			return (1);
		}
		else if (st == 0)
		{
			printf ("\nDuplicate entries in Ellipsoid Dictionary!!!\n");
			CS_fclose (elStrm);
			return (1);
		}
	}
	CS_elDictCls (elStrm);
	if (verbose) printf (" OK.\n");
	return (0);
}
