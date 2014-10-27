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

struct cs_Test9_
{
	double geo;			/* degrees */
	double chi;			/* Delta, in seconds. */
	double beta;		/* Delta, in seconds. */
	double mu;			/* Delta, in seconds. Not used any more. */
	double centric;		/* Not used as yet. */
	double para;		/* Not used as yet. */
};

/* Values given are for the Clarke 1866 ellipsoid. Taken from
   Synder, Table 3 on page 18. */

__ALIGNMENT__13				/* Required by some Sun compilers. */
struct cs_Test9_ cs_Test9 [19] =
{/*        geo     chi    beta      mu centric    para   */
		{ 90.0,    0.0,    0.0,    0.0,    0.0,    0.0},
		{ 85.0, -121.9,  -81.2,  -91.4, -122.0,  -60.9},
		{ 80.0, -240.1, -160.0, -180.0, -240.3, -120.0},
		{ 75.0, -350.9, -233.9, -263.1, -351.3, -175.4},
		{ 70.0, -451.0, -300.6, -338.2, -451.4, -225.4},
		{ 65.0, -537.2, -358.2, -403.0, -537.7, -268.6},
		{ 60.0, -607.1, -404.8, -455.4, -607.6, -303.6},
		{ 55.0, -658.5, -439.1, -494.0, -658.9, -329.3},
		{ 50.0, -689.7, -460.1, -517.5, -690.2, -345.0},
		{ 45.0, -700.0, -467.0, -525.3, -700.5, -350.2},
		{ 40.0, -689.1, -459.8, -517.2, -689.4, -344.8},
		{ 35.0, -657.2, -438.6, -493.3, -657.4, -328.9},
		{ 30.0, -605.4, -404.1, -454.5, -605.6, -303.0},
		{ 25.0, -535.3, -357.3, -401.9, -535.4, -268.0},
		{ 20.0, -449.0, -299.7, -337.1, -449.1, -224.8},
		{ 15.0, -349.2, -233.1, -262.2, -349.2, -294.9},
		{ 10.0, -238.8, -159.4, -179.3, -238.8, -119.6},
		{  5.0, -121.2,  -80.9,  -91.0, -121.2,  -60.7},
		{  0.0,    0.0,    0.0,    0.0,    0.0,    0.0}
};

int CStest9 (int verbose)
{
	extern double cs_Zero;			/* 0.0 */
	extern double cs_Degree;		/* 1.0/57.29... */
	extern double cs_Sec2Rad;		/* Converts seconds to
									   radians. */
	int ii;
	int err_cnt;

	double ka;
	double e_sq;
	double lat;
	double lat_dd;
	double test;
	double test_val;
	double fwd;
	double inv;

	double mmi_sum;
	double chii_sum;
	double chif_sum;
	double bti_sum;
	double btf_sum;

	__ALIGNMENT__1			/* Required by some Sun compilers. */
	struct cs_MmcofF_  mmcofF;
	__ALIGNMENT__2			/* Required by some Sun compilers. */
	struct cs_MmcofI_  mmcofI;
	__ALIGNMENT__3			/* Required by some Sun compilers. */
	struct cs_ChicofF_ chicofF;
	__ALIGNMENT__4			/* Required by some Sun compilers. */
	struct cs_ChicofI_ chicofI;
	__ALIGNMENT__5			/* Required by some Sun compilers. */
	struct cs_BtcofF_  btcofF;
	__ALIGNMENT__6			/* Required by some Sun compilers. */
	struct cs_BtcofI_  btcofI;

	printf ("Checking ellipsoidal power series expansion functions.\n");

	err_cnt = 0;
	test_val = 2.424068406E-07;		/* 0.05 seconds of arc in
						   radians. */
	mmi_sum  = cs_Zero;
	chii_sum = cs_Zero;
	chif_sum = cs_Zero;
	bti_sum  = cs_Zero;
	btf_sum  = cs_Zero;

	/* For each value in the table, we compute the appropriate
	   value, compare with the value in the table, and then
	   compute the inverse, and compare with the original.
	   Again, we use the values for ka and e_sq provided by
	   Synder to be perfectly consistent on the test data.

	   Note, on the forward functions, the RMS value is what
	   some would call significant, however, it is well within
	   the significance of the prcision provided in the table.
	   Analysis of the RMS errors of the inverse are more
	   indicative of the accuracy of the series functions and
	   programming. */

	ka = 6378206.4;			/* Synder, Table 1, page 12. */
	e_sq = 0.006768658;		/* Synder, text, page 13. */

	CSmmFsu  (&mmcofF,ka,e_sq);
	CSmmIsu  (&mmcofI,ka,e_sq);
	CSchiFsu (&chicofF,e_sq);
	CSchiIsu (&chicofI,e_sq);
	CSbtFsu  (&btcofF,e_sq);
	CSbtIsu  (&btcofI,e_sq);

	/* We will use the forward function to generate a value which
	   is used to test the inverse function below.  However, we
	   have no specific table of data with which to comapre the
	   results of the forward function.  SYnder does provide, on
	   page 265 in Appendix A, a single test case which is used
	   here to perform a single test on the CSmmF??? functions.
	   Since this single test is at 40 degrees of latitude, the
	   check is special cased into the loop below. */


	for (ii = 0;ii < 19;ii++)
	{
		lat_dd = cs_Test9 [ii].geo;
		lat = lat_dd * cs_Degree;

		/* Test the meridonal distance functions. */

		fwd = CSmmFcal (&mmcofF,lat,sin (lat),cos (lat));
		if (lat_dd == 40.0)
		{
			/* See page 265 of Synder for this test. */
			test = fabs ((fwd / ka) - 0.6944458);
			if (test >= 0.00000005)
			{
				printf ("CSmmFcal failed on %4.1f degrees.\n",lat_dd);
				err_cnt += 1;
			}
		}

		inv = CSmmIcal (&mmcofI,fwd);
		test = fabs (inv - lat);
		if (test > test_val)
		{
			printf ("CSmmIcal failed on %4.1f degrees.\n",lat_dd);
			err_cnt += 1;
		}
		mmi_sum += test * test;

		/* Test the conformal latitude functions. */

		fwd = CSchiFcal (&chicofF,lat);
		test = fabs (fwd - (lat + (cs_Test9 [ii].chi * cs_Sec2Rad)));
		if (test > test_val)
		{
			printf ("CSchiFcal failed on %4.1f degrees.\n",lat_dd);
			err_cnt += 1;
		}
		chif_sum += test * test;

		inv = CSchiIcal (&chicofI,fwd);
		test = fabs (inv - lat);
		if (test > test_val)
		{
			printf ("CSchiIcal failed on %4.1f degrees.\n",lat_dd);
			err_cnt += 1;
		}
		chii_sum += test * test;

		/* Test the authalic latitude functions. */

		fwd = CSbtFcalPrec (&btcofF,lat);
		test = fabs (fwd - (lat + (cs_Test9 [ii].beta * cs_Sec2Rad)));
		if (test > test_val)
		{
			printf ("CSbtFcal failed on %4.1f degrees.\n",lat_dd);
			err_cnt += 1;
		}
		btf_sum += test * test;

		inv = CSbtIcalPrec (&btcofI,fwd);
		test = fabs (inv - lat);
		if (test > test_val)
		{
			printf ("CSbtIcal failed on %4.1f degrees.\n",lat_dd);
			err_cnt += 1;
		}
		bti_sum += test * test;
	}

	/* If verbose mode, print out the RMS errors for each item. */

	if (verbose)
	{
		printf (" Note that the forward results listed here are compared against\n");
		printf (" a published table which is limited in precision to 0.1 seconds\n");
		printf (" of arc. Thus, for the forward case, any RMS error less than 0.05\n");
		printf (" seconds of arc should be considered quite good. The inverse\n");
		printf (" error numebrs result from a comparison of the forward and inverse\n");
		printf (" calculations and are a true indication of the accuracy/precision\n");
		printf (" built into CS-MAP.\n\n");
		test = sqrt (mmi_sum / 18) / cs_Sec2Rad;
		printf (" Meridonal distance inverse error (RMS) = %12.8f seconds of arc.\n",
							test);
		test = sqrt (chif_sum / 18) / cs_Sec2Rad;
		printf (" Conformal latitude forward error (RMS) = %12.8f seconds of arc.\n",
							test);
		test = sqrt (chii_sum / 18) / cs_Sec2Rad;
		printf (" Conformal latitude inverse error (RMS) = %12.8f seconds of arc.\n",
							test);
		test = sqrt (btf_sum / 18) / cs_Sec2Rad;
		printf ("  Authalic latitude forward error (RMS) = %12.8f seconds of arc.\n",
							test);
		test = sqrt (bti_sum / 18) / cs_Sec2Rad;
		printf ("  Authalic latitude inverse error (RMS) = %12.8f seconds of arc.\n",
							test);
	}
	return (err_cnt);
}
