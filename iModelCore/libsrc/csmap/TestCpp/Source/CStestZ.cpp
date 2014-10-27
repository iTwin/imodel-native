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

int CStestZ (bool verbose,char *test_file)
{
	bool ok;

	int stInv;
	int stDtc;
	int stFwd;
	int errCount;
	
	unsigned idx;
	unsigned testCount;

	EcsTestMethod testMethod;
	EcsCrsAuthority srcKeyAuthority;
	EcsCrsAuthority trgKeyAuthority;

	struct cs_Csprm_*  srcCsPrm;
	struct cs_Csprm_*  trgCsPrm;
	struct cs_Dtcprm_* dtcParms;

	double delta;

	double llhWrk [3];
	double sourceCoords [3];
	double resultCoords [3];

	char srcKeyNm [32];
	char trgKeyNm [32];
	
	std::wstring fieldData;

	errCount = 0;

	printf ("Checking all test cases in %s.\n",test_file);

	/* This test.  We'll use our CsvFileSupport mechanizsm. */
	CS_stncp (cs_TestDirP,test_file,MAXPATH); 
	TcsOsGeoTestFile osGeoTestFile (cs_TestDir);
	ok = osGeoTestFile.IsOk ();
	testCount = osGeoTestFile.RecordCount ();
	ok = testCount > 0U;
	for (idx = 0;ok && idx < testCount;idx += 1)
	{
		if (osGeoTestFile.IsComment (idx))
		{
			continue;
		}
		testMethod = osGeoTestFile.GetTestMethod (idx);
		if (testMethod != testMthCrs2D && testMethod != testMthCrs3D)
		{
			continue;
		}
		srcKeyAuthority = osGeoTestFile.GetSrcCrsKey (fieldData,idx);
		wcstombs (srcKeyNm,fieldData.c_str (),sizeof (srcKeyNm));
		trgKeyAuthority = osGeoTestFile.GetTrgCrsKey (fieldData,idx);
		wcstombs (trgKeyNm,fieldData.c_str (),sizeof (trgKeyNm));
		if (srcKeyAuthority != crsAuthCsMap || trgKeyAuthority != crsAuthCsMap)
		{
			continue;
		}
		ok = osGeoTestFile.GetSourceCoordinates (sourceCoords,idx);

		dtcParms = 0;
		srcCsPrm = CS_csloc (srcKeyNm);
		trgCsPrm = CS_csloc (trgKeyNm);
		if (srcCsPrm != 0 && trgCsPrm != 0)
		{
			dtcParms = CS_dtcsu (srcCsPrm,trgCsPrm,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_W);
		}
		if (dtcParms == 0)
		{
			if (srcCsPrm != 0)
			{
				CS_free (srcCsPrm);
				srcCsPrm = 0;
			}
			if (trgCsPrm != 0)
			{
				CS_free (trgCsPrm);
				trgCsPrm = 0;
			}
			if (dtcParms != 0)
			{
				CS_dtcls (dtcParms);
				dtcParms = 0;
				
			}
			osGeoTestFile.GetTestName (fieldData,idx);
			printf ("%S: Set up failed.  [%s <--> %s]\n",fieldData.c_str (),srcKeyNm,trgKeyNm);
			errCount += 1;
			continue;
		}
		if (testMethod == testMthCrs2D)
		{
			stInv = CS_cs2ll (srcCsPrm,llhWrk,sourceCoords);
			stDtc = CS_dtcvt (dtcParms,llhWrk,llhWrk);
			stFwd = CS_ll2cs (trgCsPrm,resultCoords,llhWrk);
		}
		else if (testMethod == testMthCrs3D)
		{
			stInv = CS_cs3ll (srcCsPrm,llhWrk,sourceCoords);
			stDtc = CS_dtcvt3D (dtcParms,llhWrk,llhWrk);
			stFwd = CS_ll3cs (trgCsPrm,resultCoords,llhWrk);
		}
		bool testOk = osGeoTestFile.CompareResults (resultCoords,idx,delta);
		if (!testOk)
		{
			osGeoTestFile.GetTestName (fieldData,idx);
			printf ("%S: Calculation failed.  [%s -> %s; %.2fmm; %d]\n",fieldData.c_str (),srcKeyNm,trgKeyNm,delta,stDtc);
			errCount += 1;
		}
		if (srcCsPrm != 0)
		{
			CS_free (srcCsPrm);
			srcCsPrm = 0;
		}
		if (trgCsPrm != 0)
		{
			CS_free (trgCsPrm);
			trgCsPrm = 0;
		}
		if (dtcParms != 0)
		{
			CS_dtcls (dtcParms);
			dtcParms = 0;
		}
	}
	printf ("Test Z: %d tests of %d failed.\n",errCount,testCount);
	return (ok) ? errCount : -1;
}
