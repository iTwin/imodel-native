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
#include "csNameMapperSupport.h"

extern int cs_Error;
extern int cs_Errno;
extern int csErrlng;
extern int csErrlat;
extern unsigned short cs_ErrSup;
#if _RUN_TIME <= _rt_UNIXPCC
extern ulong32_t cs_Doserr;
#endif
extern char cs_TestDir [];
extern char* cs_TestDirP;


int CStest4 (int verbose,char *test_file)
{
	int stI;
	int stD;
	int stF;
	int line_nbr;
	int fail_cnt;

	long32_t frmt_SX;
	long32_t frmt_SY;
	long32_t frmt_DX;
	long32_t frmt_DY;

	char *cp;
	char *src_nm;
	char *src_x;
	char *src_y;
	char *dest_nm;
	char *dest_x;
	char *dest_y;
	char *tol_x;
	char *tol_y;
	FILE *fs_tst;
	struct cs_Csprm_ *src_cs;
	struct cs_Csprm_ *dest_cs;
	struct cs_Dtcprm_ *dtc_prm;

	double src_cc [3];
	double dest_cc [3];
	double rslt [3];
	double tol [3];
	double del [3];

	char last_src [32];
	char last_dest [32];
	char l_buf [256];
	char bufrX [32];
	char bufrY [32];
	char msgBufr [256];

	printf ("Checking all test cases in %s.\n",test_file);

	/* In this test, we read an ordinary text file and parse out
	   eight items.  The items are expected to be separated by
	   commas.  The eight items, in expected order, are:

	   1) Source coordinate system.
	   2) Source X coordinate.
	   3) Source Y coordinate.
	   4) Destination coordinate system.
	   5) Destination X coordinate.
	   6) Destination Y coordinate.
	   7) X Tolerance of the test.
	   8) Y Tolerance of the test.

	   All numbers are in simple decimal form.

	   Then, for each line, we convert the coordinate,
	   check the converted result against the supplied
	   value (using the tolerance figure). */

	CS_stncp (cs_TestDirP,test_file,MAXPATH); 
	fs_tst = fopen (cs_TestDir,_STRM_TXTRD);
	if (fs_tst == NULL)
	{
		(void)printf ("Couldn't open %s\n",test_file);
		return (1);
	}

	/* Initialize the loop so we don't call CS_csloc and
	   CS_dtcsu unless the coordinate systems have changed.
	   This just keeps this test from taking a long time. */

	last_src [0] = '\0';
	last_dest [0] = '\0';
	src_cs = NULL;
	dest_cs = NULL;
	dtc_prm = NULL;
	line_nbr = 0;

	/* OK, now we can loop and perform the coordinate calculation
	   tests. */

	fail_cnt = 0;
	while (fgets (l_buf,sizeof (l_buf),fs_tst) != NULL)
	{
		line_nbr += 1;

		/* Trim any leading and trailing white space. */

		CS_trim (l_buf);

		/* Here once for each line.  Skip a comment
		   line. */

		if (l_buf [0] == '#') continue;

		/* Skip any empty lines.  CS_trim considers the
		   trailing newline character to be white space. */

		if (l_buf [0] == '\0') continue;
		
		/* Trim off any ending comments. */
		cp = strchr (l_buf,'#');
		if (cp != 0)
		{
			*cp = '\0';
		}

		/* Parse the input.  This is a pretty hoeky
		   parser, but this is a test program.  We simply
		   locate all commas and replace same with a
		   null character, then save the pointer to the
		   next character. */

		cp = l_buf;
		src_nm = cp;
		cp = strchr (cp,',');
		if (cp == NULL) goto frmterr;
		*cp++ = '\0';
		src_x = cp;
		cp = strchr (cp,',');
		if (cp == NULL) goto frmterr;
		*cp++ = '\0';
		src_y = cp;
		cp = strchr (cp,',');
		if (cp == NULL) goto frmterr;
		*cp++ = '\0';
		dest_nm = cp;
        cp = strchr (cp,',');
		if (cp == NULL) goto frmterr;
		*cp++ = '\0';
		dest_x = cp;
		cp = strchr (cp,',');
		if (cp == NULL) goto frmterr;
		*cp++ = '\0';
		dest_y = cp;
		cp = strchr (cp,',');
		if (cp == NULL) goto frmterr;
		*cp++ = '\0';
		tol_x = cp;
		cp = strchr (cp,',');
		if (cp == NULL) goto frmterr;
		*cp++ = '\0';
		tol_y = cp;

		/* Get a coordinate system parameter structure for the
		   source system.  If the name is the same as the
		   last time, skip this to speed things up a little
		   bit. */

		if (CS_stricmp (src_nm,last_src))
		{
			last_src [0] = '\0';

			/* Since the source coordinate system has changed,
			   we discard any datum conversion which was set
			   up based on it. */
			if (dtc_prm != NULL)
			{
				CS_dtcls (dtc_prm);
				dtc_prm = NULL;
			}

			if (src_cs != NULL)
			{
				CS_free (src_cs);
				src_cs = NULL;
			}

			src_cs = CS_csloc (src_nm);
			if (src_cs == NULL)
			{
				CS_errmsg (msgBufr,sizeof (msgBufr));
				(void)printf ("At line %d, CS_csloc failed on %s.\n\tReason: %s\n",line_nbr,src_nm,msgBufr);
				fail_cnt += 1;
				continue;
			}
			(void)strcpy (last_src,src_nm);

			/* Kludge to handle Alaska stuff, longitude = -189. */
			if (!CS_stricmp (src_nm,"LL27") || !CS_stricmp (src_nm,"LL83"))
			{
				src_cs->proj_prms.unity.usr_min = -270.0;
				src_cs->proj_prms.unity.usr_max = 270.0;
				src_cs->proj_prms.unity.usr_rng = 540.0;
			}
		}

		/* Similarly with the destination coordinate system. */

		if (CS_stricmp (dest_nm,last_dest))
		{
			last_dest [0] = '\0';
			if (dtc_prm != NULL)
			{
				CS_dtcls (dtc_prm);
				dtc_prm = NULL;
			}
			if (dest_cs != NULL)
			{
				CS_free (dest_cs);
				dest_cs = NULL;
			}
			dest_cs = CS_csloc (dest_nm);
			if (dest_cs == NULL)
			{
				CS_errmsg (msgBufr,sizeof (msgBufr));
				(void)printf ("At line %d, CS_csloc failed on %s.\n\tReason: %s\n",line_nbr,dest_nm,msgBufr);
				fail_cnt += 1;
				if (src_cs != NULL)
				{
					CS_free (src_cs);
					src_cs = NULL;
					last_src [0] = '\0';
				}
				continue;
			}
			(void)strcpy (last_dest,dest_nm);

			/* Kludge to handle Alaska stuff, longitude = -189. */
			if (!CS_stricmp (dest_nm,"LL27") || !CS_stricmp (dest_nm,"LL83"))
			{
				dest_cs->proj_prms.unity.usr_min = -270.0;
				dest_cs->proj_prms.unity.usr_max = 270.0;
				dest_cs->proj_prms.unity.usr_rng = 540.0;
			}
		}

		/* Set up the datum conversion.  Again, we skip this
		   if the coordinate system names are the same.  Only
		   if both were the same (very unlikely) do we
		   reuse the datum setup. */

		if (dtc_prm == NULL)
		{
			dtc_prm = CS_dtcsu (src_cs,dest_cs,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_W);
			if (dtc_prm == NULL)
			{
				CS_errmsg (msgBufr,sizeof (msgBufr));
				(void)printf ("At line %d, CS_dtcsu failed on %s to %s.\n\tReason: %s\n",line_nbr,src_nm,dest_nm,msgBufr);
				if (src_cs != NULL)
				{
					CS_free (src_cs);
					src_cs = NULL;
				}
				if (dest_cs != NULL)
				{
					CS_free (dest_cs);
					dest_cs = NULL;
				}
				last_src [0] = '\0';
				last_dest [0] = '\0';
				continue;
			}
		}

		/* OK, now we can perform the conversion. */

		frmt_SX = CS_atof (&src_cc [0],src_x);
		frmt_SY = CS_atof (&src_cc [1],src_y);
		src_cc [2] = 0.0;
		frmt_DX = CS_atof (&dest_cc [0],dest_x);
		frmt_DY = CS_atof (&dest_cc [1],dest_y);
		dest_cc [2] = 0.0;
		CS_atof (&tol [0],tol_x);
		CS_atof (&tol [1],tol_y);

		stI = CS_cs2ll (src_cs,rslt,src_cc);
		stD = CS_dtcvt (dtc_prm,rslt,rslt);
		if (stD != 0)
		{
			CS_errmsg (msgBufr,sizeof (msgBufr));
			if (stD < 0) (void)printf ("CS_dtcvt failed on %s to %s.\n\tReason: %s\n",src_nm,dest_nm,msgBufr);
			fail_cnt += 1;
		}
		stF = CS_ll2cs (dest_cs,rslt,rslt);

		/* Evaluate the calculated result. */

		del [0] = fabs (rslt [0] - dest_cc [0]);
		del [1] = fabs (rslt [1] - dest_cc [1]);
		if (del [0] > tol [0] || del [1] > tol [1])
		{
			(void)printf ("Verification failure (line %d):\n",line_nbr);
			(void)printf ("\t%-23.23s %-16.16s %-16.16s\n",
							src_nm,src_x,src_y);
			(void)printf ("\t%-23.23s %-16.16s %-16.16s\n",
							dest_nm,dest_x,dest_y);
			CS_ftoa (bufrX,17,rslt [0],frmt_DX);
			CS_ftoa (bufrY,17,rslt [1],frmt_DY);
			(void)printf ("\t                   CALC %-16.16s %-16.16s\n",
								bufrX,bufrY);
			fail_cnt += 1;
		}
		else if (verbose)
		{
			(void)printf ("[%9.9s]%12.12s %12.12s ==> %12f %12f [%-9.9s]\n",
						src_nm,
						src_x,
						src_y,
						rslt [0],
						rslt [1],
						dest_nm);
		}
	}
	fclose (fs_tst);

	/* Clean up in preparation for exit. */

	if (src_cs != NULL) CS_free (src_cs);
	if (dest_cs != NULL) CS_free (dest_cs);
	if (dtc_prm != NULL) CS_dtcls (dtc_prm);

	return (fail_cnt);

frmterr:
	printf ("Test data file format error on line %d.\n",line_nbr);
	fclose (fs_tst);
	if (src_cs != NULL) CS_free (src_cs);
	if (dest_cs != NULL) CS_free (dest_cs);
	if (dtc_prm != NULL) CS_dtcls (dtc_prm);
	return (1);
}
