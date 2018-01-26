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
	extern int csCscachI;
	extern int csDtcachI;
}

extern char cs_TestDir [];
extern char* cs_TestDirP;

int CStestR (bool verbose,char *test_file)
{
	bool ok;
	bool testOk;
	bool threeD;
	bool trgIsGeo;
	int status;

	unsigned idx;
	unsigned testCount (0);
	unsigned  errCount;

	const char* cPtr;
	double dd;				// distance between test & calculated results in meters
	double expected [3];
	double calcResult [3];

	errCount = 0U;

	printf ("[R] Checking for regressions using %s.\n",test_file);

	// Bump the High Level Interface cache count up to improve
	// performance as we will be using the same geodetic transforms
	// over and over again, and some of these are resource consuming
	// in the construction phase.  Setting the cache size means nothing
	// if they have already been allocated.  So we CS_recvr() first
	// just in case.
	CS_recvr ();
	csCscachI = 64;
	csDtcachI = 64;

	// We'll use our CsvFileSupport mechanism.
	CS_stncp (cs_TestDirP,test_file,MAXPATH); 
	TcsOsGeoTestFile osGeoTestFile (cs_TestDir);
	ok = osGeoTestFile.IsOk ();
	if (!ok)
	{
		printf ("Open of regression test (%s) failed.\n",test_file);
		errCount += 1;
	}

	if (ok)
	{
		testCount = osGeoTestFile.RecordCount ();
		ok = testCount > 0U;
	}
	for (idx = 0;ok && idx < testCount;idx += 1)
	{
		// A throw here is possible,  but unlikely.
		const TcsCsvRecord& curRecord = osGeoTestFile.GetRec (idx);
		TcsTestCase curTest (curRecord);
		cPtr = curTest.GetTrgCrs ();
		threeD = curTest.IsThreeD ();
		trgIsGeo = CS_isgeo (cPtr);
		testOk = curTest.ExecuteTest (dd,status,calcResult);
		if (!testOk)
		{
			curTest.GetResultPnt (expected);
			cPtr = curTest.GetPointID ();
			printf ("Test \"%s\" failed: status = %d, Delta (mm) = %.3f.\n",cPtr,status,dd);
			if (verbose)
			{
				if (threeD)
				{
					if (trgIsGeo)
					{
						printf ("\tExpected:   %.9f : %.9f : %.4f\n",expected [0],
																	 expected [1],
																	 expected [2]);
						printf ("\tCalculated: %.9f : %.9f : %.4f\n",calcResult [0],
																	 calcResult [1],
																	 calcResult [2]);
					}
					else
					{
						printf ("\tExpected:   %.5f : %.5f : %.5f\n",expected [0],
																	 expected [1],
																	 expected [2]);
						printf ("\tCalculated: %.5f : %.5f : %.5f\n",calcResult [0],
																	 calcResult [1],
																	 calcResult [2]);
					}
				}
				else
				{
					if (trgIsGeo)
					{
						printf ("\tExpected:   %.9f : %.9f\n",expected [0],
															  expected [1]);
						printf ("\tCalculated: %.9f : %.9f\n",calcResult [0],
															  calcResult [1]);
					}
					else
					{
						printf ("\tExpected:   %.5f : %.5f\n",expected [0],
															  expected [1]);
						printf ("\tCalculated: %.5f : %.5f\n",calcResult [0],
															  calcResult [1]);
					}
				}
			}
			errCount += 1;
		}

		// Provide a progress indication.
		if ((idx % 1000) == 0)
		{
			printf ("Testing: %s            \r",curTest.GetPointID());
			fflush (stdout);
		}
	}
	if (ok)
	{
		printf ("Test R: %u tests processed, %u failed.\n",testCount,errCount);
	}
	return (ok) ? errCount : -1;
}
