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

extern char cs_TestDir [];
extern char* cs_TestDirP;

int CStestA (bool verbose,char *test_file)
{
	int st;
	int line_nbr;
	int fail_cnt;

	long32_t frmt_SX;
	long32_t frmt_SY;
	long32_t frmt_DX;
	long32_t frmt_DY;

	char *cp;
	char *csvSrc;
	char *src_x;
	char *src_y;
	char *csvDest;
	char *dest_x;
	char *dest_y;
	char *tol_x;
	char *tol_y;
	FILE *fs_tst;

	double src_cc [3];
	double dest_cc [3];
	double rslt [3];
	double tol [2];
	double del [2];

    char src_nm [32];
    char dest_nm [32];
	char l_buf [256];
	char errBufr [256];
	char bufrX [32];
	char bufrY [32];

	printf ("Using %s to test single function interface.\n",test_file);

    csSetSystemFlavor (csMapFlvrAutodesk);

	/* Same as Test 4, except we use the single function interface.
	   Therefore, we leave out the comments, and get right to it. */

	CS_stncp (cs_TestDirP,test_file,MAXPATH); 
	fs_tst = fopen (cs_TestDir,_STRM_TXTRD);
	if (fs_tst == NULL)
	{
		(void)printf ("Couldn't open %s\n",test_file);
		return (1);
	}

	line_nbr = 0;
	fail_cnt = 0;
	while (fgets (l_buf,sizeof (l_buf),fs_tst) != NULL)
	{
		line_nbr += 1;
		CS_trim (l_buf);
		if (l_buf [0] == '#') continue;
		if (l_buf [0] == '\0') continue;

		/* Trim off any ending comments. */
		cp = strchr (l_buf,'#');
		if (cp != 0)
		{
			*cp = '\0';
		}

		cp = l_buf;
		csvSrc = cp;
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
		csvDest = cp;
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

		CS_stncp (src_nm,csvSrc,sizeof (src_nm));
		CS_stncp (dest_nm,csvDest,sizeof (dest_nm));		

		/* OK, now we can perform the conversion. */

		frmt_SX = CS_atof (&src_cc [0],src_x);
		frmt_SY = CS_atof (&src_cc [1],src_y);
		src_cc [2] = 0.0;
		frmt_DX = CS_atof (&dest_cc [0],dest_x);
		frmt_DY = CS_atof (&dest_cc [1],dest_y);
		dest_cc [2] = 0.0;
		CS_atof (&tol [0],tol_x);
		CS_atof (&tol [1],tol_y);

		rslt [0] = src_cc [0];
		rslt [1] = src_cc [1];
		rslt [2] = 0.0;
    	st = CS_cnvrt (src_nm,dest_nm,rslt);
		if (st < 0)
		{
			CS_errmsg (errBufr,sizeof (errBufr));
			printf ("At line %d, conversion of %s to %s failed.\n\tReason: %s\n",line_nbr,src_nm,dest_nm,errBufr);
			fail_cnt += 1;
			continue;
		}
		if (st > 0)
		{
			printf ("Warning, range/domain error on %s ==> %s.\n",src_nm,dest_nm);
		}

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
	CS_recvr ();

	return (fail_cnt);

frmterr:
	printf ("Test data file format error on line %d.\n",line_nbr);
	fclose (fs_tst);
	CS_recvr ();

	return (1);
}
