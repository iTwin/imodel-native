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

//lint -esym(715,verbose,duration)
//lint -esym(752,cs_Doserr,cs_Error,cs_Errno,cs_ErrSup,csErrlng,csErrlat)

#include "csTestCpp.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

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

/* This is the Temporary test module.  That is, simply a module which ordinarily
   succeeds at doing nothing (it's also very fast :>).  The purpose here is
   to provide a place where it is easy to place some temporary code into the test
   module environment.  That is, write some quick code and get it compiled and
   run it without all the hassles of establishing a solution, a project, set
   all the parameters, etc. etc.

   Case in point, I needed to generate a list of test points for a specific
   conversion before a major change so that I could verify that the change
   did not produce any regressions.  Thus, I simply add the code here and
   run the console test module with the /tT option.  Whalla!!!  I got it done
   in 30 minutes instead of two hours.
*/

extern "C" char cs_Dir [];
extern "C" char *cs_DirP;
extern "C" char csErrmsg [256];
extern "C" double cs_Zero;
extern "C" double cs_LlNoise;
extern "C" const char csDictDir [];

int CStestT (bool verbose,long32_t duration)
{
	int err_cnt = 0;
	clock_t nmStartClock;
	clock_t nmDoneClock;
	double nmExecTime;

	// Exercising GDA2020 systems.
	int status;
	struct cs_Csprm_ *srcCS;
	struct cs_Csprm_ *trgCS;
	struct cs_Dtcprm_ *dtcprm;

	double llAgd66 [3] = {112.00, -20.00, 0.0};
	double llGda2020 [3] = {0.0, 0.0, 0.0};
	
	
	// Mostly to keep lint/compiler happy.
	nmStartClock = clock ();

	srcCS = CS_csloc ("LL-AGD84-Grid");			// via grid
	trgCS = CS_csloc ("GDA2020-7P.MGA-46");

	if (srcCS != NULL && trgCS != NULL)
	{
		dtcprm = CS_dtcsu (srcCS,trgCS,cs_DTCFLG_DAT_W1,cs_DTCFLG_BLK_W);
		if (dtcprm != NULL)
		{
			status = CS_dtcvt (dtcprm,llAgd66,llGda2020);
		}
	}
	CS_dtcls (dtcprm);

	if (srcCS != NULL && trgCS != NULL)
	{
		dtcprm = CS_dtcsu (trgCS,srcCS,cs_DTCFLG_DAT_W1,cs_DTCFLG_BLK_W);
		if (dtcprm != NULL)
		{
			status = CS_dtcvt (dtcprm,llGda2020,llAgd66);
		}
	}
	CS_dtcls (dtcprm);
	nmDoneClock = clock ();

#ifdef __SKIP__
	// Working ticket #206
	int status;
	struct cs_Csprm_ *srcCS;
	struct cs_Csprm_ *trgCS;
	struct cs_Dtcprm_ *dtcprm;

	// Arbitrary geographic point selected to be in the coverage hole of the
	// AGD84 to GDA84 NTv2 grid file.  Suspect that theinverse in the hole
	// is not using the inverse of the fallback correctly.
	double llGDA94 [3] = { 132.000, -20.000, 0.0 };					// Center of Northern Territory AGD84 hole
	double llAGD84 [3] = {  0.0,  0.0,  0.0 };

// Test point from gda-v2.4.pdf, page 39, checking inverse (GDA94 -> AGD84)
//	double llAGD84 [3] = { 128.79901513889, -17.529231,     258.81 };	// 128 47 56.4545E, 17 31 45.2316S  AGD84 Base
//	double llGDA94 [3] = { 128.80027169444, -17.5277981667,   0.00 };	// 128 48 00.9781E, 17 31 40.0734S  GDA84 via Grid
//	double llGDA94 [3] = { 128.80027444444, -17.5277972222, 258.81 };	// 128 48 00.988E,  17 31 40.070S   GDA84 via Similarity

	// Mostly to keep lint/compiler happy.
	nmStartClock = clock ();


	srcCS = CS_csloc ("LL-GDA94");
	trgCS = CS_csloc ("LL-AGD84-Grid");			// via grid
//	trgCS = CS_csloc ("LL-AGD84");				// via ASTRLA84 directly

	if (srcCS != NULL && trgCS != NULL)
	{
		dtcprm = CS_dtcsu (srcCS,trgCS,cs_DTCFLG_DAT_W1,cs_DTCFLG_BLK_W);
		if (dtcprm != NULL)
		{
			status = CS_dtcvt (dtcprm,llGDA94,llAGD84);
		}
	}
	nmDoneClock = clock ();
#endif


#ifdef __SKIP__
	// Test implementation of GDA2020

	// Test case extracted from Geocentric Datum of Australia 2020 -- Interim Report

	double gda94LL [3] = { 133.88551329, -23.67012389, 603.3466 };
	// Debug Aid: GDA94   XYZ =  -4052051.7643, 4212836.2017, -2545106.0245
	//            GDA2020 XYZ =  -4052052.7379, 4212835.9897, -2545104.5898
	double gda2020LL [3];		// expected: 133.8855216, -23.67011014, 603.2489

	// Mostly to keep lint/compiler happy.
	nmStartClock = clock ();

	srcCS = CS_csloc ("LL-GDA94");
	trgCS = CS_csloc ("LL-GDA2020");
	dtcprm = CS_dtcsu (srcCS,trgCS,cs_DTCFLG_DAT_W1,cs_DTCFLG_DAT_W1);
	if (dtcprm != NULL)
	{
		status = CS_dtcvt3D (dtcprm,gda94LL,gda2020LL);
	}
	else
	{
		err_cnt = 1;
	}
	nmDoneClock = clock ();
#endif

#ifdef __SKIP__
	// Test code for investigating Ticket #199
	struct cs_Csprm_ *srcCS;
	struct cs_Csprm_ *trgCS;
	struct cs_Dtcprm_ *dtcprm;

	// Mostly to keep lint/compiler happy.
	nmStartClock = clock ();

	// Selection of the source coordinate system here is crucial.  Needs to be
	// one that gets us into phase three of bridge building.  I had to comment
	// a path definition out in order to get a good case for debugging/testing
	// this ticket.

	srcCS = CS_csloc ("CH1903/GSB.LL");
	trgCS = CS_csloc ("CHTRF95.LL");
//	srcCS = CS_csloc ("Ireland1965.ING");
//	trgCS = CS_csloc ("ITM");
//	srcCS = CS_csloc ("Reunion47.LL");
//	trgCS = CS_csloc ("LL84");

	dtcprm = CS_dtcsu (srcCS,trgCS,cs_DTCFLG_DAT_W1,cs_DTCFLG_DAT_W1);
	if (dtcprm != NULL)
	{
		CS_dtcls (dtcprm);
	}
	dtcprm = CS_dtcsu (srcCS,trgCS,cs_DTCFLG_DAT_W1,cs_DTCFLG_DAT_W1);
	if (dtcprm != NULL)
	{
		CS_dtcls (dtcprm);
	}
	dtcprm = CS_dtcsu (srcCS,trgCS,cs_DTCFLG_DAT_W1,cs_DTCFLG_DAT_W1);
	if (dtcprm != NULL)
	{
		CS_dtcls (dtcprm);
	}
	dtcprm = CS_dtcsu (srcCS,trgCS,cs_DTCFLG_DAT_W1,cs_DTCFLG_DAT_W1);
	if (dtcprm != NULL)
	{
		CS_dtcls (dtcprm);
	}

	// Mostly to keep lint/compiler happy.
	nmDoneClock = clock ();
#endif

#ifdef __SKIP__

	// Test code for investigating Ticket #177

	int idx;
	int status;
	char baseName [64];

	for (idx = 0;idx < 500;idx+=1)
	{
		printf ("idx = %d\r",idx);
		status = CSllCsFromDt (baseName,sizeof baseName,"LEIGON");
	}
	nmDoneClock = clock ();
#endif

#ifdef __SKIP__

	// Testing fix for Trac Ticket #137.

	int status;

	// Tolerance is rather large here.  This is because inverting a Molodensky
	// by flipping the signs is not really all that precise an inverse.  This is
	// what we did to fix Ticket #7.
	double tolerance = 1.5E-07;
	double deltaLng, deltaLat, deltaHgt;

	double rgf93LL [3];
	double workLL  [3];
	double ntfgLL  [3];

	/* Point outside the coverage of the gr3d97a.txt */
	rgf93LL [LNG] =  -6.00000;
	rgf93LL [LAT] =   53.00;
	rgf93LL [HGT] =   0.000;

	ntfgLL [LNG] =  -5.9988497435;
	ntfgLL [LAT] =  53.0001373603;
	ntfgLL [HGT] =  0.0;

	workLL [LNG] = rgf93LL [LNG];
	workLL [LAT] = rgf93LL [LAT];
	workLL [HGT] = rgf93LL [HGT];
	/* Use debugger to verify that the fallback in the inverse direction
	   is being used. */
	status = CS_cnvrt ("LL-RGF93","NTF.LL",workLL);
	if (status >= 0)
	{
		deltaLng = fabs (workLL [LNG] - ntfgLL [LNG]);
		deltaLat = fabs (workLL [LAT] - ntfgLL [LAT]);
		deltaHgt = fabs (workLL [HGT] - ntfgLL [HGT]);
		if ((deltaLng > tolerance) || (deltaLat > tolerance) || (deltaHgt > tolerance))
		{
			err_cnt++;
		}
	}
	else
	{
		err_cnt++;
	}

#endif

#ifdef __SKIP__

	// Creating a test point to verify fix for Trac Ticket #137.

	int status;
	double rgf93LL [3];
	double workLL  [3];
	double ntfgLL  [3];

	/* Point outside the coverage of the gr3d97a.txt */
	rgf93LL [LNG] =  -6.00000;
	rgf93LL [LAT] =   53.00;
	rgf93LL [HGT] =   0.000;

	workLL [LNG] = rgf93LL [LNG];
	workLL [LAT] = rgf93LL [LAT];
	workLL [HGT] = rgf93LL [HGT];
	/* Use debugger to verify that the fallback in the inverse direction
	   is being used. */
	status = CS_cnvrt ("LL-RGF93","NTF.LL",workLL);
	if (status >= 0)
	{
		ntfgLL [LNG] = workLL [LNG];
		ntfgLL [LAT] = workLL [LAT];
		ntfgLL [HGT] = workLL [HGT];
	}
	else
	{
		err_cnt++;
	}

#endif

#ifdef __SKIP__
	// Working a regression report on the fix for Trac Ticket #157.
	bool ok (true);
	int status;
	char srcCrsName [32] = {"Reunion.LL"};
	char trgCrsName [32] = {"LL84"};
	char wktBufr [512];

	struct cs_Csprm_* srcCrs;
	struct cs_Csprm_* trgCrs;
	struct cs_Dtcprm_ *dtc_ptr;

	struct cs_Csdef_ wktCsDef;
	struct cs_Dtdef_ wktDtDef;
	struct cs_Eldef_ wktElDef;

	double reunionLL  [3];
	double workLL [3];
	double targetLL [3];

	reunionLL [LNG] =  55.50000;
	reunionLL [LAT] = -21.00000;
	reunionLL [HGT] =   0.000;

	srcCrs = NULL;
	trgCrs = CS_csloc (trgCrsName);
	dtcPtr = NULL;

	ok = (trgCrs != NULL);
	if (ok)
	{
		status = CS_cs2WktEx (wktBufr,sizeof (wktBufr),srcCrsName,wktFlvrAutodesk,0);
		ok = (status == 0);
	}
	if (ok)
	{
		status = CS_wktToCsEx (&wktCsDef,&wktDtDef,&wktElDef,wktFlvrAutodesk,wktBufr,0);
		ok = (status == 0);
	}
	if (ok)
	{
		srcCrs = CScsloc1 (&wktCsDef);
		ok = (srcCrs != NULL);
	}
	if (ok)
	{
		dtc_ptr = CS_dtcsu (srcCrs,trgCrs,cs_DTC_DAT_F,cs_DTC_BLK_F);
		ok = (dtc_ptr != NULL);
	}
	if (ok)
	{
		status = CS_cs2ll (srcCrs,workLL,reunionLL);
		ok = (status == 0);
	}
	if (ok)
	{
		status = CS_dtcvt (dtc_ptr,workLL,workLL);
		ok = (status == 0);
	}
	if (ok)
	{
		status = CS_ll2cs (trgCrs,targetLL,workLL);
		ok = (status == 0);
	}
	if (!ok)
	{
		err_cnt += 1;
	}
#endif

#ifdef __SKIP__

	/* Working Trac Ticket #157 */

	int status;
	double reunionLL  [3];
	double etrf89LL [3];

	reunionLL [LNG] =  55.50000;
	reunionLL [LAT] = -21.00000;
	reunionLL [HGT] =   0.000;
	etrf89LL [LNG] = reunionLL [LNG];	etrf89LL [LAT] = reunionLL [LAT];	etrf89LL [HGT] = reunionLL [HGT];

	status = CS_cnvrt ("Reunion47.LL","Reunion92.LL",etrf89LL);
	etrf89LL [LNG] = reunionLL [LNG];	etrf89LL [LAT] = reunionLL [LAT];	etrf89LL [HGT] = reunionLL [HGT];
	status = CS_cnvrt ("Reunion47.LL","LL-ETRF89",etrf89LL);
	etrf89LL [LNG] = reunionLL [LNG];	etrf89LL [LAT] = reunionLL [LAT];	etrf89LL [HGT] = reunionLL [HGT];
	status = CS_cnvrt ("Reunion47.LL","LL84",etrf89LL);
	etrf89LL [LNG] = reunionLL [LNG];	etrf89LL [LAT] = reunionLL [LAT];	etrf89LL [HGT] = reunionLL [HGT];
	status = CS_cnvrt ("Reunion47.LL","LL84",etrf89LL);
#endif

#ifdef __SKIP__
	// Test case for Trac ticket #129.
	int st;
	double ll84  [3];
	double llE50 [3];
	ll84  [LNG] = 180.000000000;
	ll84  [LAT] =  40.000000000;
	ll84  [HGT] =   0.00;

//	llE50 [LNG] = ll84 [LNG]; llE50 [LAT] = ll84 [LAT]; llE50 [HGT] = ll84 [HGT];
//	st = CS_cnvrt ("ED50/ES.LL","LL84",llE50);

	llE50 [LNG] = ll84 [LNG]; llE50 [LAT] = ll84 [LAT]; llE50 [HGT] = ll84 [HGT];
	st = CS_cnvrt ("LL84","ED50/ES.LL",llE50);

	llE50 [LNG] = ll84 [LNG]; llE50 [LAT] = ll84 [LAT]; llE50 [HGT] = ll84 [HGT];
	st = CS_cnvrt ("ED50/ES.LL","LL84",llE50);

	llE50 [LNG] = ll84 [LNG]; llE50 [LAT] = ll84 [LAT]; llE50 [HGT] = ll84 [HGT];
	st = CS_cnvrt ("LL84","ED50/ES.LL",llE50);

	err_cnt += (st != 0);
#endif

#ifdef __SKIP__

	// Test case for Trac ticket #130.
	int st;
	double ll27 [3];
	double ll83 [3];
	ll27 [LNG] = -44.0000000001;
	ll27 [LAT] =  84.0000000000;
	ll27 [HGT] =   0.00;
	ll83 [LNG] = ll27 [LNG];
	ll83 [LAT] = ll27 [LAT];
	ll83 [HGT] = ll27 [HGT];

	// Simulate a conversion very close to the boundary.  Using debug mode,
	// force an intermediary result outside of the boundary and see how the
	// code handles it.  In this case, the fallback NAD27-49_to_NAD83 was
	// called to generate an approximate result, and a +2 status was
	// returned.
	st = CS_cnvrt ("LL83","LL27",ll83);
	st = CS_cnvrt ("LL83","LL27",ll83);
	st = CS_cnvrt ("LL83","LL27",ll83);
	st = CS_cnvrt ("LL83","LL27",ll83);
	st = CS_cnvrt ("LL83","LL27",ll83);
	st = CS_cnvrt ("LL83","LL27",ll83);
	err_cnt += (st != 0);
#endif

#ifdef __SKIP__
	// Working Trac Ticket #146, #155, and fixes for #151.  Note, to execute this test,
	// one needs to edit the GeoidHeight.gdc file and make the .\WW15MGH.GRD
	// reference the first one in the file (temporarily).

	int status;
	
	double geoidHeight;
	double ll84 [3];

	geoidHeight = -9999.999;
	ll84 [0] = (359.875 - 360.00);
	ll84 [1] = 89.875;
	status = CS_geoidHgt (ll84,&geoidHeight);
	if (status != 0 || geoidHeight < -1.0E+10)
	{
		char errMessage [MAXPATH];
		if (status > 0 && status <= 4)
		{
			CS_stncp (errMessage,"Geoid location outside the range of data files specified in \"GeoidHeight.gdc\".",sizeof (errMessage));
		}
		else
		{
			CS_errmsg (errMessage,MAXPATH);
		}
		printf ("Failure: st = %d.  Reason: %s\n",status,errMessage);
		err_cnt += 1;
	}

	/* geoidHgt should equal ~13.75475 per author of the Trac Ticket 145 */
	if (fabs (geoidHeight - 13.75475) > 0.00003)
	{
		printf ("Trac Ticket 145, 146, & 155 test failed [%.6f]\n",geoidHeight);
		err_cnt += 1;
	}

	CS_geoidCls ();

	/* Test other ASCII/binary related changes by simply constructing the
	   objects. The constructors now have a simple test case built into
	   them.*/

	CS_stcpy (cs_DirP,cs_OSTN02_NAME);
	struct cs_Ostn02_ *ostn02 = CSnewOstn02 (cs_Dir);
	if (ostn02 != NULL)
	{
		CSdeleteOstn02 (ostn02);
	}
	else
	{
		char errMessage [MAXPATH];

		err_cnt += 1;
		CS_errmsg (errMessage,MAXPATH);
		printf ("OSTN02 construction failed; Reason: %s\n",errMessage);
	}

	CS_stcpy (cs_DirP,cs_OSTN97_NAME);
	struct cs_Ostn97_ *ostn97 = CSnewOstn97 (cs_Dir);
	if (ostn97 != NULL)
	{
		CSdeleteOstn97 (ostn97);
	}
	else
	{
		char errMessage [MAXPATH];

		err_cnt += 1;
		CS_errmsg (errMessage,MAXPATH);
		printf ("OSTN97 construction failed; Reason: %s\n",errMessage);
	}

	CS_stcpy (cs_DirP,"OSGM91.TXT");
	struct cs_Osgm91_ *osgm91 = CSnewOsgm91 (cs_Dir,0,0,cs_Zero);
	if (osgm91 != NULL)
	{
		CSdeleteOsgm91 (osgm91);
	}
	else
	{
		char errMessage [MAXPATH];

		err_cnt += 1;
		CS_errmsg (errMessage,MAXPATH);
		printf ("OSGM91 construction failed; Reason: %s\n",errMessage);
	}

	// TO DO:  Should add a test of CS_japan.c here.  The Japan
	// constructor needs a test function before that will do much good.
#endif

#ifdef __SKIP__
	// Testing Trac Ticket 131
	int st;
	double ll27 [3];
	double ll83 [3];
	double utms [3];
	double lngLat [3];

	struct cs_Eldef_ *elPtr;
	struct cs_Mgrs_ *mgrsPtr;

	char mgrsTxt [256];
	char errMsg [256];

	ll83 [LNG] = ll27 [LNG] = -10.0;
	ll83 [LAT] = ll27 [LAT] = -10.0;
	ll83 [HGT] = ll27 [HGT] =   0.0;
	st = CS_cnvrt ("LL27","LL83",ll83);
	if (st != 0)
	{
		CS_errmsg (errMsg,sizeof (errMsg));
		printf ("Expected failure, st = %d, msg = %s\n", st, errMsg);
	}
	else
	{
		printf ("Unexpected success.\n");
		err_cnt += 1;
	}


	ll83 [LNG] = -191.000;
	ll83 [LAT] = -91.000;
	ll83 [HGT] =   0.000;

	mgrsPtr = CSnewMgrsE ("GRS1980",0);
	if (mgrsPtr == NULL)
	{
		CS_errmsg (errMsg,sizeof (errMsg));
		printf ("MGRS OBject construction failed. Reason: %s\n",errMsg);
		err_cnt += 1;
	}
	else
	{
		st = CScalcUtmUps (mgrsPtr,utms,ll83);
		if (st == 0)
		{
			CS_errmsg (errMsg,sizeof (errMsg));
			printf ("Expected failure, st = %d, msg = %s\n", st, errMsg);
		}
		else
		{
			printf ("Unexpected success.\n");
			err_cnt += 1;
		}
		
		utms [XX] =   555000.000;
		utms [YY] = 11444000.000;
		utms [ZZ] =        0.000;
		st = CScalcUtmUps (mgrsPtr,utms,lngLat);
		if (st == 0)
		{
			CS_errmsg (errMsg,sizeof (errMsg));
			printf ("Expected failure, st = %d, msg = %s\n", st, errMsg);
		}
		else
		{
			printf ("Unexpected success.\n");
			err_cnt += 1;
		}

		utms [XX] =  555000.000;
		utms [YY] = 4444000.000;
		utms [ZZ] =       0.000;
		st = CScalcMgrsFromLlUtm (mgrsPtr,mgrsTxt,sizeof (mgrsTxt),ll83,utms,3);
		if (st < 0)
		{
			CS_errmsg (errMsg,sizeof (errMsg));
			printf ("Expected failure, st = %d, msg = %s\n", st, errMsg);
		}
		else
		{
			printf ("Unexpected success.\n");
			err_cnt += 1;
		}
	}
#endif

#ifdef __SKIP__
	EcsCsvStatus status = csvEndOfTable;

	std::wifstream inStrm;
	TcsCsvStatus csvStatus;
	TcsNameMapper lclNameMapper;

	std::locale lclLocale ("english_UK");

	// Release any name mapper which may be present.
	cmGetNameMapperPtr (true);

	nmStartClock = clock ();
	inStrm.imbue (lclLocale);
	inStrm.open ("C:\\Temp\\NameMapper.csv",std::ios_base::in);
	if (inStrm.good ())
	{
		status = lclNameMapper.ReadFromStream (inStrm,csvStatus);
	}
	inStrm.close ();
	nmDoneClock = clock ();
	if (status == csvOk)
	{
		nmLoadTime = (double)(nmDoneClock - nmStartClock) / (double)CLOCKS_PER_SEC;
		printf ("NameMapper loaded in %.3f seconds.\n",nmLoadTime);
	}
	else
	{
		std::wstring reason = csvStatus.GetMessage ();
		wprintf (L"NameMapper load failed: %s\n",reason.c_str());
	}
#endif

#ifdef __SKIP__
	int status;

	/* Working Trac ticket 102. */

	double delta;
	double xy [2];
	double ll [2];
	
	struct cs_Csprm_ *csPrm;

	delta = cs_LlNoise;
	ll [0] = 171.37625;
	ll [1] = 7.0872222;
	csPrm = CS_csloc ("MAJURO");
	if (csPrm != NULL)
	{
		for (int i = 0;i <= 100;i+= 1)
		{
			status = CS_ll2cs (csPrm,xy,ll);
			if (status != 0)
			{
				err_cnt += 1;
			}
			printf ("%13.10f::%13.10f == %13.5f::%13.5f\n",ll [0],ll[1],xy[0],xy[1]);
			ll [0] += delta * 10.0;
			ll [1] += delta * 10.0;
		}
		status = CS_ll2cs (csPrm,xy,ll);
	}
#endif

#ifdef __SKIP__
	int status;
	double geoidHeight;
	double ll84 [2];

	/* Working Trac ticket 100.  We build a scenario which reliably
	   replicates the problem in order to be sure we fix the real
	   problem.

	   OK, modified the GeoidHeight.gdc file to list the WW15MGH.GRD file
	   first.  Having done that, the rest is fairly simple:
	    */
	ll84 [0] = (269.779155 - 360.00);
	ll84 [1] = 38.6281550;
	status = CS_geoidHgt (ll84,&geoidHeight);
	if (status != 0)
	{
		char errMessage [MAXPATH];
		CS_errmsg (errMessage,MAXPATH);
		printf ("Failure: st = %d.  Reason: %s.\n",status,errMessage);
		err_cnt += 1;
	}
	/* geoidHgt should equal ~-31.628   Egm84 numbers */

	ll84 [0] = (305.0211140 - 360.0000);
	ll84 [1] = -14.6212170;
	status = CS_geoidHgt (ll84,&geoidHeight);
	if (status != 0)
	{
		char errMessage [MAXPATH];
		CS_errmsg (errMessage,MAXPATH);
		printf ("Failure: st = %d.  Reason: %s.\n",status,errMessage);
		err_cnt += 1;
	}
	/* geoidHgt should equal ~-2.969   Egm84 numbers */

#endif

#ifdef __SKIP__

	int st;

	double llhIn  [3];
	double llhOk  [3];
	double llhDel [3];

	char errMsg [MAXPATH + MAXPATH];


	// Testing NSRS conversions, especially the height conversion.  Two
	// tests coded, test results generated by geocon.exe
	llhIn [0] = -105.0;
	llhIn [1] =   39.0;
	llhIn [2] =  100.0;

	llhOk [0] = -105.0000001305;
	llhOk [1] =   38.99999999166666666666666666666667;
	llhOk [2] =   99.974;

	st = CS_cnvrt3D ("NAD83/HARN.LL","NSRS07.LL",llhIn);
	if (st != 0)
	{
		CS_errmsg (errMsg,sizeof (errMsg));
		printf ("Test T failed: %s\n",errMsg);
		err_cnt += 1;
	}
	llhDel [0] = fabs (llhIn [0] - llhOk [0]);
	llhDel [1] = fabs (llhIn [1] - llhOk [1]);
	llhDel [2] = fabs (llhIn [2] - llhOk [2]);

	printf ("Deltas: %g %g %g\n",llhDel [0],llhDel [1], llhDel [2]); 

	llhIn [0] = -102.11223344444444;
	llhIn [1] =   35.77665544444444;
	llhIn [2] =  100.0;

	llhOk [0] = -102.112233516666667;
	llhOk [1] =   35.776655502777777;
	llhOk [2] =   99.995;

	st = CS_cnvrt3D ("NAD83/HARN.LL","NSRS07.LL",llhIn);
	if (st != 0)
	{
		CS_errmsg (errMsg,sizeof (errMsg));
		printf ("Test T failed: %s\n",errMsg);

		err_cnt += 1;
	}
	llhDel [0] = fabs (llhIn [0] - llhOk [0]);
	llhDel [1] = fabs (llhIn [1] - llhOk [1]);
	llhDel [2] = fabs (llhIn [2] - llhOk [2]);

	printf ("Deltas: %g %g %g\n",llhDel [0],llhDel [1], llhDel [2]); 

#endif

#ifdef __SKIP__

	double llhIn  [3];
	double llhOk  [3];
	double llhDel [3];

	llhIn [0] = -105.0;
	llhIn [1] =   39.0;
	llhIn [2] =  100.0;

	llhOk [0] = -105.0000001305;
	llhOk [1] =   38.99999999166666666666666666666667;
	llhOk [2] =   99.974;

	int st = CS_cnvrt3D ("LL-HPGN","NSRS2007.LL",llhIn);
	if (st != 0)
	{
		err_cnt += 1;
	}
	
	llhDel [0] = fabs (llhIn [0] - llhOk [0]);
	llhDel [1] = fabs (llhIn [1] - llhOk [1]);
	llhDel [2] = fabs (llhIn [2] - llhOk [2]);
	
	printf ("Deltas: %g %g %g\n",llhDel [0],llhDel [1], llhDel [2]); 

#endif

#ifdef __SKIP__

	double xyz [3];

	xyz [0] = 0.0;
	xyz [1] = 20000000.000;
	xyz [2] = 0.0;

	int st = CS_cnvrt ("WGS84.PseudoMercator","LL",xyz);
	if (st != 0)
	{
		err_cnt += 1;
	}
#endif

#ifdef __SKIP__

	char wktOne   [1024] = "GEOGCS [ \"NAD83\", DATUM [\"NAD 83\", SPHEROID [\"GRS 80\", 6378137.000000, 298.257222]], PRIMEM [ \"Greenwich\", 0.000000 ], UNIT [\"Degrees\", 0.01745329251994330]]";
//	char wktOne   [1024] = "PROJCS[\"DHDN / Gauss-Kruger zone 5\",GEOGCS[\"DHDN\",DATUM[\"Deutsches_Hauptdreiecksnetz\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128,AUTHORITY[\"EPSG\",\"7004\"]],AUTHORITY[\"EPSG\",\"6314\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4314\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",15],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",5500000],PARAMETER[\"false_northing\",0],AUTHORITY[\"EPSG\",\"31469\"]]";
	char wktTwo   [1024] = "GEOGCS[\"LL84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137.000,298.25722293]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.01745329251994]]";
	char wktThree [1024] = "PROJCS[\"NAD83 / California zone 3 (ftUS)\",GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4269\"]],UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],PROJECTION[\"Lambert_Conformal_Conic_2SP\"],PARAMETER[\"standard_parallel_1\",38.43333333333333],PARAMETER[\"standard_parallel_2\",37.06666666666667],PARAMETER[\"latitude_of_origin\",36.5],PARAMETER[\"central_meridian\",-120.5],PARAMETER[\"false_easting\",6561666.667],PARAMETER[\"false_northing\",1640416.667],AUTHORITY[\"EPSG\",\"2227\"],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH]]";
	char wktFour  [1024] = "PROJCS[\"DHDN.Berlin/Cassini\",GEOGCS[\"DHDN.LL\",DATUM[\"DHDN\",SPHEROID[\"BESSEL\",6377397.155,299.15281535],TOWGS84[582.0000,105.0000,414.0000,-1.040000,-0.350000,3.080000,8.30000000]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Cassini-Soldner\"],PARAMETER[\"false_easting\",40000.000],PARAMETER[\"false_northing\",10000.000],PARAMETER[\"central_meridian\",13.62720366666667],PARAMETER[\"latitude_of_origin\",52.41864827777778],UNIT[\"Meter\",1.00000000000000]]";
	char wktFive  [1024] = "PROJCS[\"NAD83 / UTM zone 19N\",GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4269\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-69],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],AUTHORITY[\"EPSG\",\"26919\"],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]";
	char wktSix   [1024] = "PROJCS[\"NAD_1983_HARN_StatePlane_Hawaii_3_FIPS_5103_Feet\",GEOGCS[\"GCS_North_American_1983_HARN\",DATUM[\"D_North_American_1983_HARN\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",1640416.666666667],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",-158.0],PARAMETER[\"Scale_Factor\",0.99999],PARAMETER[\"Latitude_Of_Origin\",21.16666666666667],UNIT[\"Foot_US\",0.3048006096012192]]";
	char wktSeven [1024] = "PROJCS[\"NAD_1983_UTM_Zone_12N\",GEOGCS[\"GCS_North_American_1983\",DATUM[\"D_North_American_1983\",SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",-111.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";
	char wktEight [1024] = "PROJCS[\"Italian National System (Gauss-Boaga), Zone 1 (West) Peninsular\",GEOGCS [\"Rome 1940\",DATUM [\"Rome 1940\",SPHEROID [\"International 1924\", 6378388, 297], -104.1, -49.1, -9.9, 0.971, -2.917, 0.714, -11.68],PRIMEM [ \"Greenwich\", 0.000000 ],UNIT [\"Decimal Degree\", 0.01745329251994330]],PROJECTION [\"Transverse Mercator\"],PARAMETER [\"Scale_Factor\", 0.999600],PARAMETER [\"Central_Meridian\", 9.000000],PARAMETER [\"False_Easting\", 150000000],UNIT [\"Centimeter\", 0.01]]";
	char wktNine  [1024] = "PROJCS[\"ENEL GB\",GEOGCS[\"ROME1940-IT-7P\",DATUM[\"ROME1940-IT-7P\",SPHEROID[\"International 1924 (EPSG ID 7022)\",6378388.000,297.00000000],TOWGS84[-104.1000,-49.1000,-9.9000,0.971000,-2.917000,0.714000,-11.68000000]],PRIMEM[\"Greenwich\",0],UNIT[\"Decimal Degree\",0.017453292519943295]],PROJECTION[\"Transverse Mercator\"],PARAMETER[\"False_Easting\",1500000.000],PARAMETER[\"False_Northing\",0.000],PARAMETER[\"Scale_Factor\",1.000000000000],PARAMETER[\"Central_Meridian\",9.00000000000000],PARAMETER[\"Latitude_Of_Origin\",0.00000000000000],UNIT[\"Centimeter\",0.01000000000000]]";

	int stOne;
	struct cs_Csdef_ csDefOne;
	struct cs_Dtdef_ dtDefOne;
	struct cs_Eldef_ elDefOne;

	int stTwo;
	struct cs_Csdef_ csDefTwo;
	struct cs_Dtdef_ dtDefTwo;
	struct cs_Eldef_ elDefTwo;

	int stThree;
	struct cs_Csdef_ csDefThree;
	struct cs_Dtdef_ dtDefThree;
	struct cs_Eldef_ elDefThree;

	int stFour;
	struct cs_Csdef_ csDefFour;
	struct cs_Dtdef_ dtDefFour;
	struct cs_Eldef_ elDefFour;

	int stFive;
	struct cs_Csdef_ csDefFive;
	struct cs_Dtdef_ dtDefFive;
	struct cs_Eldef_ elDefFive;

	int stSix;
	struct cs_Csdef_ csDefSix;
	struct cs_Dtdef_ dtDefSix;
	struct cs_Eldef_ elDefSix;

	int stSeven;
	struct cs_Csdef_ csDefSeven;
	struct cs_Dtdef_ dtDefSeven;
	struct cs_Eldef_ elDefSeven;

	int stEight;
	struct cs_Csdef_ csDefEight;
	struct cs_Dtdef_ dtDefEight;
	struct cs_Eldef_ elDefEight;

	int stNine;
	struct cs_Csdef_ csDefNine;
	struct cs_Dtdef_ dtDefNine;
	struct cs_Eldef_ elDefNine;

	csErrmsg [0] = '\0';
	stOne = CS_wktToCsEx (&csDefOne,&dtDefOne,&elDefOne,wktFlvrOgc,wktOne,TRUE);
	if (verbose && stOne < 0)
	{
		printf ("WKT1 processing failed! Status = %d; Reason: %s\n",stOne,csErrmsg);
	}

	csErrmsg [0] = '\0';
	stTwo = CS_wktToCsEx (&csDefTwo,&dtDefTwo,&elDefTwo,wktFlvrNone,wktTwo,FALSE);
	if (verbose && stTwo < 0)
	{
		printf ("WKT2 processing failed! Status = %d; Reason: %s\n",stTwo,csErrmsg);
	}


	csErrmsg [0] = '\0';
	stThree = CS_wktToCsEx (&csDefThree,&dtDefThree,&elDefThree,wktFlvrOgc,wktThree,TRUE);
	if (verbose && stThree < 0)
	{
		printf ("WKT2 processing failed! Status = %d; Reason: %s\n",stThree,csErrmsg);
	}

	csErrmsg [0] = '\0';
	stFour = CS_wktToCsEx (&csDefFour,&dtDefFour,&elDefFour,wktFlvrOgc,wktFour,TRUE);
	if (verbose && stFour < 0)
	{
		printf ("WKT2 processing failed! Status = %d; Reason: %s\n",stFour,csErrmsg);
	}

	csErrmsg [0] = '\0';
	stFive = CS_wktToCsEx (&csDefFive,&dtDefFive,&elDefFive,wktFlvrOgc,wktFive,TRUE);
	if (verbose && stFive < 0)
	{
		printf ("WKT2 processing failed! Status = %d; Reason: %s\n",stFive,csErrmsg);
	}

	csErrmsg [0] = '\0';
	stSix = CS_wktToCsEx (&csDefSix,&dtDefSix,&elDefSix,wktFlvrEsri,wktSix,TRUE);
	if (verbose && stSix < 0)
	{
		printf ("WKT2 processing failed! Status = %d; Reason: %s\n",stSix,csErrmsg);
	}

	csErrmsg [0] = '\0';
	stSeven = CS_wktToCsEx (&csDefSeven,&dtDefSeven,&elDefSeven,wktFlvrEsri,wktSeven,TRUE);
	if (verbose && stSeven < 0)
	{
		printf ("WKT2 processing failed! Status = %d; Reason: %s\n",stSeven,csErrmsg);
	}

	csErrmsg [0] = '\0';
	stEight = CS_wktToCsEx (&csDefEight,&dtDefEight,&elDefEight,wktFlvrEsri,wktEight,TRUE);
	if (verbose && stEight < 0)
	{
		printf ("WKT2 processing failed! Status = %d; Reason: %s\n",stEight,csErrmsg);
	}

	csErrmsg [0] = '\0';
	stNine = CS_wktToCsEx (&csDefNine,&dtDefNine,&elDefNine,wktFlvrEsri,wktNine,TRUE);
	if (verbose && stNine < 0)
	{
		printf ("WKT2 processing failed! Status = %d; Reason: %s\n",stNine,csErrmsg);
	}

	err_cnt += (stOne   != 0);
	err_cnt += (stTwo   != 0);
	err_cnt += (stThree != 0);
	err_cnt += (stFour  != 0);
	err_cnt += (stFive  != 0);
	err_cnt += (stSix   != 0);
	err_cnt += (stSeven != 0);
	err_cnt += (stEight != 0);
	err_cnt += (stNine != 0);

#endif
#ifdef __SKIP__
	int st;

	unsigned idx;
	unsigned gxIdxCnt;

	Const struct cs_GxIndex_* gxIdxPtr;
	struct cs_GeodeticTransform_ *gxDefPtr;

	int err_list [8];

	gxIdxCnt = CS_getGxIndexCount ();
	for (idx = 0;idx < gxIdxCnt;idx++)
	{
		gxIdxPtr = CS_getGxIndexEntry (idx);
		if (gxIdxPtr == NULL)
		{
			err_cnt += 1;
		}
		else
		{
			gxDefPtr = CS_gxdef (gxIdxPtr->xfrmName);
			if (gxDefPtr == NULL)
			{
				err_cnt += 1;
			}
			else
			{
				st = CS_gxchk (gxDefPtr,cs_GXCHK_DATUM | cs_GXCHK_REPORT,err_list,sizeof (err_list) / sizeof (int));
				if (st != 0)
				{
					printf ("CS_gxchk failed on geodetic transformation named %s.\n",gxDefPtr->xfrmName);
					err_cnt += 1;
				}
				CS_free (gxDefPtr);
			}
		}
	}
#endif
#ifdef __SKIP__
	int st;
	printf ("Running temporary test code module.\n");


	struct cs_GeodeticTransform_* gx_def1;
	struct cs_GeodeticTransform_* gx_def2;


	gx_def1 = CS_gxdef ("NAD27_to_NAD83");
	gx_def2 = CS_gxdef ("ABIDJAN-87_to_WGS84");
	
	st = CS_gxupd (gx_def1);
	st = CS_gxupd (gx_def2);

	gx_def1 = CS_gxdef ("NAD27_to_NAD83");
	gx_def2 = CS_gxdef ("ABIDJAN-87_to_WGS84");
#endif
#ifdef __SKIP__
	int st;

	const char* csOneName = "LL27";
	const char* csTwoName = "Tokyo";

	struct cs_Csprm_ *csOne;
	struct cs_Csprm_ *csTwo;
	struct cs_Dtcprm_ *dtcPrm;

	double llTmp [3];

	printf ("Running temporary test code module.\n");

	csOne = CS_csloc (csOneName);
	csTwo = CS_csloc (csTwoName);
	if (csOne == NULL || csTwo == NULL)
	{
		return 1;
	}

	dtcPrm = CS_dtcsu (csOne,csTwo,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_W);
	if (dtcPrm == NULL)
	{
		return 1;
	}

	llTmp [0] = -122.1509375;
	llTmp [1] = 36.10875;
	llTmp [2] = 0.0;

	st = CS_dtcvt3D (dtcPrm,llTmp,llTmp);

	if (st != 0)
	{
		err_cnt += 1;
	}

	CS_dtcls (dtcPrm);
#endif
#ifdef __SKIP__
	int st;
	int counter;
	FILE* tstStrm;
	struct cs_Csprm_ *csOne;
	struct cs_Csprm_ *csTwo;
	struct cs_Dtcprm_ *dtcPrm;

	double lngMin = -5.5000;
	double lngMax = 10.0000;
	double latMin = 41.0000;
	double latMax = 52.0000;

	double llOne [3];
	double llTmp [3];
	double llTwo [3];

	const char* csOneName = "LL-RGF93";
	const char* csTwoName = "NTF.LL";

	tstStrm = fopen ("C:\\Tmp\\TestPoints.txt","wt");
	if (tstStrm == NULL)
	{
		return 1;
	}

	csOne = CS_csloc (csOneName);
	csTwo = CS_csloc (csTwoName);
	if (csOne == NULL || csTwo == NULL)
	{
		return 1;
	}
	dtcPrm = CS_dtcsu (csOne,csTwo,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_W);
	if (dtcPrm == NULL)
	{
		return 1;
	}

	for (counter = 0;counter < duration;counter += 1)
	{
		st = 0;
		llOne [0] = CStestRN (lngMin,lngMax);
		llOne [1] = CStestRN (latMin,latMax);
		llOne [2] = 0.0;
		st  = CS_cs3ll (csOne,llTmp,llOne);
		st |= CS_dtcvt (dtcPrm,llTmp,llTmp);
		st |= CS_ll3cs (csTwo,llTwo,llTmp);
	    
		fprintf (tstStrm,"%s,%.9f,%.9f,%s,%.9f,%.9f,1.0E-08,1.0E-08\n",csOneName,llOne [0],
																				 llOne [1],
																				 csTwoName,
																				 llTwo [0],
																				 llTwo [1]);
		fprintf (tstStrm,"%s,%.9f,%.9f,%s,%.9f,%.9f,1.0E-08,1.0E-08\n",csTwoName,llTwo [0],
																				 llTwo [1],
																				 csOneName,
																				 llOne [0],
																				 llOne [1]);
		if (st != 0)
		{
			err_cnt += 1;
		}
	}
	fclose (tstStrm);
#endif

#ifdef __SKIP__
	int st;

	const char* dtOneName = "AFGOOYE";
	const char* dtTwoName = "WGS84";

	struct cs_Datum_ *dtOne;
	struct cs_Datum_ *dtTwo;
	struct cs_Dtcprm_ *dtcPrm;

	double llTmp [3];

	printf ("Running temporary test code module.\n");

	dtOne = CS_dtloc (dtOneName);
	dtTwo = CS_dtloc (dtTwoName);
	if (dtOne == NULL || dtTwo == NULL)
	{
		return 1;
	}

	dtcPrm = CSdtcsu (dtOne,dtTwo,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_W);
	if (dtcPrm == NULL)
	{
		return 1;
	}

	CS_dtcls (dtcPrm);
#endif

	/* Mostly to keep lint/compiler hgappy. */
	nmExecTime = (double)(nmDoneClock - nmStartClock) / (double)CLOCKS_PER_SEC;
	printf ("Execution time: %f seconds.\n",nmExecTime);
	return err_cnt;
}
