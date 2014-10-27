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

#define cs_TEST_COORD_COUNT 10
static double csTestcoords [10][3] =
{
	{ 456661.0, 4316548.0, 0.0 },
	{ 462938.0, 4314975.0, 0.0 },
	{ 512384.0, 4419456.0, 0.0 },
	{ 417845.0, 4391564.0, 0.0 },
	{ 489263.0, 4346712.0, 0.0 },
	{ 612374.0, 4394561.0, 0.0 },
	{ 472384.0, 4314904.0, 0.0 },
	{ 498374.0, 4334978.0, 0.0 },
	{ 452894.0, 4401245.0, 0.0 },
	{ 413649.0, 4294561.0, 0.0 }
};

int CStest5 (int verbose,long32_t duration)
{
	int ii;

	long32_t cvt_cnt;
	long32_t rate;

	clock_t done;
	clock_t start;

	struct cs_Csprm_ *utm13;
	struct cs_Csprm_ *co83c;
	struct cs_Dtcprm_ *dtcptr;

	double elapsed;				/* In seconds. */
	double dbls [3];

	/* Here to perform a performance test.  For this
	   we will convert a series of coordinates from
	   UTM-13 to CO83-C */

	utm13 = CS_csloc ("UTM27-13");
	if (utm13 == NULL) return (1);
	co83c = CS_csloc ("CO83-C");
	if (co83c == NULL) return (1);
	dtcptr = CS_dtcsu (utm13,co83c,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_F);
	if (dtcptr == NULL) return (1);

	duration = duration * 10000;

	printf ("Measuring performance: %ld coordinates; UTM27-13 ==> CO83-C\n",duration);

	cvt_cnt = 0L;
	start = clock ();
	while (cvt_cnt < duration)
	{
		for (ii = 0;ii < 10;ii++)
		{
			CS_cs2ll (utm13,dbls,csTestcoords [ii]);
			CS_dtcvt (dtcptr,dbls,dbls);
			CS_ll2cs (co83c,dbls,dbls);
			cvt_cnt += 1L;
		}
	}
	done = clock ();
	CS_free (utm13);
	CS_free (co83c);
	CS_dtcls (dtcptr);

	/* Report on the performance. */
	elapsed = (double)(done - start) / (double)CLOCKS_PER_SEC;
	if (elapsed > 0.00001)
	{
		rate = (cvt_cnt > 0) ? (long32_t)((double)cvt_cnt / elapsed) : 0;
	}
	else
	{
		rate = 0;
	}
	printf ("Converted %ld coordinates from UTM27-13 to CO83C in %6.2lf seconds.\n",
					cvt_cnt,elapsed);
	printf ("Effective conversion rate = %ld conversions per second.\n",rate);

	return (0);
}
