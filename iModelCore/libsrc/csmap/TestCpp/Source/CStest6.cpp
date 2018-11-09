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
}

__ALIGNMENT__12				/* Required by some Sun compilers. */
static struct tst_lst_ sort_tst [] =
{			    
	{          "UTM27-13",NULL,NULL,NULL},
	{              "AK-1",NULL,NULL,NULL},
	{             "WY-EC",NULL,NULL,NULL},
	{           "UTM-13N",NULL,NULL,NULL},
	{              "US48",NULL,NULL,NULL},
	{           "UTM-13S",NULL,NULL,NULL},
	{              "CO-N",NULL,NULL,NULL},
	{             "WY-WC",NULL,NULL,NULL},
	{                "LL",NULL,NULL,NULL},
	{               "YAP",NULL,NULL,NULL},
	{"YellowMedicineMN-M",NULL,NULL,NULL},	/* Last one */
	{                  "",NULL,NULL,NULL}
};

/*
	The following function simply inverts the result of
	CS_cscmp, thereby yeilding a sort in reverse order.
*/
int CStest6cmp (struct cs_Csdef_ *pp,struct cs_Csdef_ *qq)
{
	return (-CS_cscmp (pp,qq));
}

int CStest6 (bool verbose,bool crypt)
{
	int st;
	int cnt;
	int flag;
	int my_crypt;

	csFILE *csStrm;
	struct tst_lst_ *tp;

	char last_name [48];

	__ALIGNMENT__1			/* Required by some Sun compilers. */
	struct cs_Csdef_ cs_def;

	printf ("Testing sort and binary search functions.\n");

	/* This test sorts the Coordinate System Dictionary, the
	   largest of the three dictionaries, into reverse order.
	   Then does some CS_bins calls.  Then resorts back into
	   normal order.  This is a resaonable good exercise of
	   the sort function which normally sorts files with a
	   single entry out of order. */

	if (verbose)
	{
		printf ("Sorting Coordinate System Dictionary into reverse order.\n");
	}
	csStrm = CS_csopn (_STRM_BINUP);
	if (csStrm == NULL)
	{
		printf ("Coordinate System Dictionary open failed.  Is it write protected?.\n");
		return 1;
	}

	/* NOTE: CS_csopn leaves the file positioned after
	   the magic number.  CS_ips always starts sorting
	   at the current file position. */

	CS_ips (csStrm,sizeof (struct cs_Csdef_),-1L,(CMPFUNC_CAST)CStest6cmp);

	/* The dictionary should now be in reverse order.  We
	   check the order now.  CS_ips is supposed to return
	   with the file position where it was when we started. */

	if (verbose)
	{
		printf ("Checking reverse order of Coordinate System Dictionary.\n");
	}
	cnt = 0;
	st = CS_csrd (csStrm,&cs_def,&my_crypt);
	if (st == 0)
	{
		printf ("CS_ips hosed the Coordinate System Dictionary.\n");
		CS_fclose (csStrm);
		return (1);
	}
	CS_stncp (last_name,cs_def.key_nm,sizeof (last_name));
	while (CS_csrd (csStrm,&cs_def,&my_crypt))
	{
		cnt += 1;
		if (verbose && (cnt % 10) == 0) putchar ('.');
		st = CS_stricmp (last_name,cs_def.key_nm);
		if (st < 0)
		{
			printf ("\nSort did not sort entire file correctly!!!\n");
			CS_fclose (csStrm);
			return (1);
		}
		else if (st == 0)
		{
			printf ("\nSort left duplicate entries when sorting entire file!!!\n");
			CS_fclose (csStrm);
			return (1);
		}
	}
	if (verbose) printf (" OK.\n");

	/* If we are still here, the sort worked correctly.
	   Use the sort_tst list as a list of names to binary
	   search on.  It includes the normal first and last;
	   which should now be the last and the first. */

	tp = sort_tst;
	while (tp->name [0] != '\0')
	{
		if (verbose)
		{
			printf ("Searching for %s in reversed CS Dictionary.",tp->name);
		}
		CS_stncp (cs_def.key_nm,tp->name,sizeof (cs_def.key_nm));
		flag = CS_bins (csStrm,(long32_t)sizeof (cs_magic_t),(long32_t)0,sizeof (cs_def),
				(char *)&cs_def,(CMPFUNC_CAST)CStest6cmp);
		if (flag <= 0)
		{
			printf ("CS_bins failed on file in reverse order!!!\n");
			CS_fclose (csStrm);
			return (1);
		}
		if (verbose) printf (" OK.\n");
		tp += 1;
	}

	/* OK, return the file back to normal sorted order. */

	if (verbose)
	{
		printf ("Sorting Coordinate System Dictionary back to normal order.\n");
	}
	CS_fseek (csStrm,(long)sizeof (cs_magic_t),SEEK_SET);
	CS_ips (csStrm,sizeof (struct cs_Csdef_),-1L,(CMPFUNC_CAST)CS_cscmp);

	/* One more time, check that the order is correct. */

	if (verbose)
	{
		printf ("Checking Coordinate System Dictionary normal sorted order.\n");
	}
	cnt = 0;
	st = CS_csrd (csStrm,&cs_def,&my_crypt);
	if (st == 0)
	{
		printf ("CS_ips hosed the Coordinate System Dictionary.\n");
		CS_fclose (csStrm);
		return (1);
	}
	CS_stncp (last_name,cs_def.key_nm,sizeof (last_name));
	while (CS_csrd (csStrm,&cs_def,&my_crypt))
	{
		cnt += 1;
		if (verbose && (cnt % 10) == 0) putchar ('.');
		st = CS_stricmp (last_name,cs_def.key_nm);
		if (st > 0)
		{
			printf ("Sort did not sort entire file correctly!!!\n");
			CS_fclose (csStrm);
			return (1);
		}
		else if (st == 0)
		{
			printf ("Sort left duplicate entries when sorting entire file!!!\n");
			CS_fclose (csStrm);
			return (1);
		}
	}
	if (verbose) printf (" OK.\n");

	/* OK, if we are still here, we passed the test. */

	CS_csDictCls (csStrm);
	return (0);
}
