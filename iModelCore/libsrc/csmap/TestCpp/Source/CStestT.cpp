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
   run it without all the hassels of establishing a solution, a project, set
   all the parameters, etc. etc.
   
   Case in point, I needed to generate a list of test points for a specific
   conversion before a major change so that I could verify that the change
   did not produce any regressions.  Thus, I simply add the code here and
   run the console test module with the /tT option.  Whalla!!!  I got it done
   in 30 minutes instead of two hours.
*/
  
extern "C" char csErrmsg [256];
int CStestT (bool verbose,long32_t duration)
{
	int err_cnt = 0;

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

#ifndef __SKIP__

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

	return err_cnt;
}
