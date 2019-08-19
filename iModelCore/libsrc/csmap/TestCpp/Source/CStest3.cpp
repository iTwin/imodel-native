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

__ALIGNMENT__12				/* Required by some Sun compilers. */
static struct tst_lst_ cs_tst [] =
{			    
	{          "UTM27-13",NULL,NULL},
	{              "AK-1",NULL,NULL},
	{             "WY-EC",NULL,NULL},
	{           "UTM-13N",NULL,NULL},
	{              "US48",NULL,NULL},
	{           "UTM-13S",NULL,NULL},
	{              "CO-N",NULL,NULL},
	{             "WY-WC",NULL,NULL},
	{                "LL",NULL,NULL},
	{               "YAP",NULL,NULL},
	{"YellowMedicineMN-M",NULL,NULL},	/* Last one */
	{                  "",NULL,NULL}
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

int CStest3 (bool verbose,bool crypt)
{
	short protectSave;
	char uniqueSave;

	int st;
	int cnt;
	int my_crypt;

	csFILE *csStrm;
	struct tst_lst_ *tp;

	char last_name [48];

	__ALIGNMENT__1			/* Required by some Sun compilers. */
	struct cs_Csdef_ cs_def;

	/* Preserve the original values of the dictionary protection system.
	   We will need to turn protection off and on to do this test
	   properly.  Upon entry, the protection system should be on. */
	protectSave = cs_Protect;
	uniqueSave = cs_Unique;

	printf ("Testing Coordinate System Dictionary functions.\n");

	tp = cs_tst;
	while (tp->name [0] != '\0')
	{
		/* Always restore protection to the distribution values. */
		cs_Protect = protectSave;
		cs_Unique = uniqueSave;
		if (verbose)
		{
			printf ("Fetching %s from the Coordinate System Dictionary.",tp->name);
		}

		tp->cs_ptr = CS_csdef (tp->name);
		if (tp->cs_ptr == NULL)
		{
			(void)printf ("\nCS_csdef failed on %s. [%d:%d]\n",
							tp->name,
							cs_Error,
							cs_Errno);
			return (1);
		}
		if (verbose) printf (" OK.\n");

		/* If this is not a TEST (i.e. group) coordinate system, we can't really test
		   it with protection on as the compiler will have marked it as a
		   distribution coordinate system.  Therefore, we have two tests,
		   one for the TEST group and one for other groups.  We can do the
		   TEST group with protection on. */
		if (CS_stricmp (tp->cs_ptr->group,"tEst"))
		{
			/* Its not a test definition, we have to test with protection off. */
			if (verbose)
			{
				printf ("Updating %s in the Coordinate System Dictionary with protection turned off.\n",tp->name);
			}

			cs_Protect = -1;
			cs_Unique = '\0';
			st = CS_csupd (tp->cs_ptr,crypt);
			/* Return value should be one since this should be an update. */
			if (st != 1)
			{
				(void)printf ("\nUnprotected CS_csupd failed failed on %s at line %d. [%d:%d]\n",
								tp->name,
								(__LINE__-6),
								cs_Error,
								cs_Errno);
				return (1);
			}
			st = CS_csdel (tp->cs_ptr);
			if (st != 0)
			{
				(void)printf ("\nUnprotected CS_csdel failed on %s at line %d. [%d:%d]\n",
								tp->name,
								(__LINE__-6),
								cs_Error,
								cs_Errno);
				return (1);
			}
		}
		else
		{
			if (verbose)
			{
				printf ("Updating %s in the Coordinate System Dictionary with protection on.",tp->name);
			}

			/* The TEST definitions are manufactured with a creation date of (aprrox) Jan 1, 2002.
			   We set cs_Protect to a large number to essentially enable this definition to
			   be updated in the normal fashion.  That's what we want to test here.  We
			   actually test protection system operation elsewhere. */
			cs_Protect = 10000;
			st = CS_csupd (tp->cs_ptr,crypt);	/* This should succeed. */
			if (st != 1)
			{
				(void)printf ("\nProtected CS_csupd failed on %s at line %d. [%d:%d]\n",
								tp->name,
								(__LINE__-6),
								cs_Error,
								cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");
			if (verbose)
			{
				printf ("Deleting %s from the Coordinate System Dictionary (protected).",tp->name);
			}

			/* Test delete, and remove the entry with the bogus protect value in
			   same operation.  This delete should succeed. */
			st = CS_csdel (tp->cs_ptr);
			if (st != 0)
			{
				(void)printf ("\nProtected CS_csdel failed on %s at line %d. [%d:%d]\n",
								tp->name,
								(__LINE__-6),
								cs_Error,
								cs_Errno);
				return (1);
			}
			if (verbose) printf (" OK.\n");
		}
		tp++;
	}

	/* We go through the list again, adding each definition back into the deictionary.
	   Since each was deleted, they should all succeed.  However, we have to turn unique
	   off since these defintiions will have their distribution names. */
	tp = cs_tst;
	while (tp->name [0] != '\0')
	{
		cs_Protect = protectSave;
		cs_Unique = uniqueSave;
		if (verbose)
		{
			printf ("Putting %s back in the Coordinate System Dictionary.",tp->name);
		}
		cs_Unique = '\0';					/* This turns name protection off. */
		st = CS_csupd (tp->cs_ptr,crypt);	/* This should succeed. */
		if (st != 0)
		{
			(void)printf ("\nCS_csupd failed on %s at line %d. [%d:%d]\n",
								tp->name,
								(__LINE__-6),
								cs_Error,
								cs_Errno);
			return (1);
		}
		if (verbose) printf (" OK.\n");
		CS_free (tp->cs_ptr);
		tp++;
	}
	if (verbose)
	{
		printf ("Checking the sorted order of the Coordinate System Dictionary.\n");
	}

	/* Verify that we haven't screwed up the dictionary. */
	cnt = 0;
	csStrm = CS_csopn (_STRM_BINRD);
	st = CS_csrd (csStrm,&cs_def,&my_crypt);
	CS_stncp (last_name,cs_def.key_nm,sizeof (last_name));
	while (CS_csrd (csStrm,&cs_def,&my_crypt))
	{
		cnt += 1;
		if (verbose && (cnt % 14) == 0) putchar ('.');
		st = CS_stricmp (last_name,cs_def.key_nm);
		if (st > 0)
		{
			printf ("\nCoordinate System Dictionary no longer sorted!!!\n");
			CS_fclose (csStrm);
			return (1);
		}
		else if (st == 0)
		{
			printf ("\nDuplicate entries in Coordinate System Dictionary!!!\n");
			CS_fclose (csStrm);
			return (1);
		}
	}
	if (verbose) printf (" OK.\n");
	CS_csDictCls (csStrm);
	return (0);
}
