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

double CStestRN (double low, double high);

extern int cs_Error;
extern int cs_Errno;
extern int csErrlng;
extern int csErrlat;
extern unsigned short cs_ErrSup;
#if _RUN_TIME <= _rt_UNIXPCC
extern ulong32_t cs_Doserr;
#endif

struct _hTable1
{
	char str1 [32];
	char str2 [32];
	int cmpLength;
	int cmpValue;
};
__ALIGNMENT__15				/* Required by some Sun compilers. */
struct _hTable1 hTable1 [] =
{
	{ "abcdefg", "abcdefg", 0,  0},
	{ "ABCDEFG", "abcdefg", 0,  0},
	{ "abcdefg", "ABCDEFG", 0,  0},

	{ "abcdefg", "bcdefgh", 0, -1},
	{ "ABCDEFG", "bcdefgh", 0, -1},
	{ "abcdefg", "BCDEFGH", 0, -1},

	{ "bcdefgh", "abcdefg", 0,  1},
	{ "BCDEFGH", "abcdefg", 0,  1},
	{ "bcdefgh", "ABCDEFG", 0,  1},

	{ "abcd   ", "abcd$$$", 4,  0},
	{ "ABCD   ", "abcd$$$", 4,  0},
	{ "abcd   ", "ABCD$$$", 4,  0},

	{ "abcd   ", "bcde$$$", 4, -1},
	{ "ABCD   ", "bcde$$$", 4, -1},
	{ "abcd   ", "BCDE$$$", 4, -1},

	{ "bcde   ", "abcd!!!", 4,  1},
	{ "BCDE   ", "abcd!!!", 4,  1},
	{ "bcde   ", "ABCD!!!", 4,  1},

	{ "abcd!!!", "abcd   ", 4,  0},
	{ "ABCD!!!", "abcd   ", 4,  0},
	{ "abcd!!!", "ABCD   ", 4,  0},

	{ "abcd$$$", "bcde   ", 4, -1},
	{ "ABCD$$$", "bcde   ", 4, -1},
	{ "abcd$$$", "BCDE   ", 4, -1},

	{ "bcde$$$", "abcd   ", 4,  1},
	{ "BCDE$$$", "abcd   ", 4,  1},
	{ "bcde$$$", "ABCD   ", 4,  1},

	{ "ABC_EFG", "abc_efg", 0,  0},
	{ "ABC[EFG", "abc[efg", 0,  0},
	{ "ABC]EFG", "abc]efg", 0,  0},
	{ "ABC^EFG", "abc^efg", 0,  0},
	{ "ABC`EFG", "abc`efg", 0,  0},
	{ "ABC_EFG", "abc[efg", 0,  1},
	{ "ABC[EFG", "abc_efg", 0, -1},

	{ "",        "Abcdef",  0, -1},
	{ "Abcdef",  "",        0,  1},
	{ "AbcDefG", "aBcDefG", 0,  0},
	{ "",        "",       -1,  0}
};
struct _hTable2
{
	char str1 [32];
	char str2 [32];
	char str3 [32];
	int nullFlag;
};
__ALIGNMENT__16				/* Required by some Sun compilers. */
struct _hTable2 hTable2 [] =
{
	{ "AbCdEfGhIjKlMnO", "efg",            "EfG",            FALSE},
	{ "AbCdEfGhIjKlMnO", "EFG",            "EfG",            FALSE},
	{ "AbCdEfGhIjKlMnO", "EfG",            "EfG",            FALSE},
	{ "AbCdEfGhIjKlMnO", "eFg",            "EfG",            FALSE},

	{ "AbCdEfGhIjKlMnO", "eg",             "",                TRUE},
	{ "AbCdEfGhIjKlMnO", "",               "",                TRUE},
	{ "AbCdEfGhIjKlMnO", "EGF",            "",                TRUE},
	{ "AbCdEfGhIjKlMnO", "egf",            "",                TRUE},

	{ "AbCdEfGhIjKlMnO", "MNO",            "MnO",             FALSE},
	{ "AbCdEfGhIjKlMnO", "AbC",            "AbC",             FALSE},
	{ "AbCdEfGhIjKlMnO", "ABCDEFGHIJKLMNO","AbCdEfGhIjKlMnO", 0},

	{ "CONUS",           "HPGN",           "",                TRUE},
	{ "conus",           "HPGN",           "",                TRUE},
	{ "CONUS",           "hpgn",           "",                TRUE},
	{ "CoNuS",           "HpGn",           "",                TRUE},
	{ "COnuS",           "hPgN",           "",                TRUE},
	{ "COHPGN",          "HPGN",           "HPGN",            FALSE},
	{ "cohpgn",           "HPGN",          "hpgn",            FALSE},
	{ "CoHpgn",           "HPGN",          "Hpgn",            FALSE},
	{ "CoHpgn",           "HPGN",          "Hpgn",            FALSE},
	{ "CoHpGn",           "HPGN",          "HpGn",            FALSE},
	{ "CoHpGn",           "hpgn",          "HpGn",            FALSE},
	{ "CoHpGn.los",       "hpgn",          "HpGn",            FALSE},
	{ "CoHpGn.LAS",       "HPGN",          "HpGn",            FALSE},
	{ "CoHpGn.XXX",       "hpgn",          "HpGn",            FALSE},
	{ "CoHpGn.XXX",       "h",             "H",               FALSE},
	{ "CoHpGn.XXX",       "q",             "",                TRUE},
	{ "CoHpGn.XXX",       "",              "",                TRUE},
	{ "",                 "Q",             "",                TRUE},

	{ "",                "",                "",               -1}
};
int CSusrCsDef (struct cs_Csdef_ *csPtr,Const char *keyName)
{
	if (!strcmp (keyName,"XXX"))
	{
		strcpy (csPtr->desc_nm,"XXX");
		strcpy (csPtr->source,"Source");
		return 0;
	}
	else if (!strcmp (keyName,"YYY"))
	{
		CS_erpt (cs_NO_MEM);
		return -1;
	}
	return 1;
}
int CSusrDtDef (struct cs_Dtdef_ *dtPtr,Const char *keyName)
{
	if (!strcmp (keyName,"XXX"))
	{
		strcpy (dtPtr->name,"XXX");
		strcpy (dtPtr->source,"Source");
		return 0;
	}
	else if (!strcmp (keyName,"YYY"))
	{
		CS_erpt (cs_NO_MEM);
		return -1;
	}
	return 1;
}

int CSusrElDef (struct cs_Eldef_ *elPtr,Const char *keyName)
{
	if (!strcmp (keyName,"XXX"))
	{
		strcpy (elPtr->name,"XXX");
		strcpy (elPtr->source,"Source");
		return 0;
	}
	else if (!strcmp (keyName,"YYY"))
	{
		CS_erpt (cs_NO_MEM);
		return -1;
	}
	return 1;
}
double CSusrUnit (short type,Const char *unitName)
{
	if (!strcmp (unitName,"XXX"))
	{
		return 17.0;
	}
	if (!strcmp (unitName,"YYY"))
	{
		CS_erpt (cs_NO_MEM);
		return -1.0;
	}
	return 0.0;
}

struct _mgrsTable
{
	char mgrs [16];
	char lat  [16];
	char lng  [16];
};
__ALIGNMENT__16				/* Required by some Sun compilers. */
struct _mgrsTable mgrsTable [] =
{
	{ "11SMV3868748955", "35 40 57.34N","117 40 39.28W" },
	{ "19TDL2756711568", "45 15 13.96N","069 55 23.18W" },
	{ "32VLL1890944323", "58 59 59.96N","005 50 50.04E" },
	{ "33XWF2510946986", "77 00 00.01N","015 59 59.84E" },
	{ "35XMK2635439930", "80 30 00.00N","023 00 00.09E" },
	{ "37XCG8408563319", "77 59 59.99N","033 59 59.94E" },
	{ "41XMK4763094558", "81 00 00.00N","060 00 00.07E" },
	{ "24SWH8779806286", "38 00 00.00N","038 00 00.01W" },
	{ "19FDV3603304750", "55 00 00.01S","069 59 59.97W" },
	{ "56HKH2290933784", "34 00 00.01S","150 00 00.02E" },
	{ "27JWG9645080794", "29 59 59.99S","020 00 00.00W" },
	{ "17NRA2840700034", "00 00 01.11N","078 03 00.00W" },
	{ "04NAF6609900034", "00 00 01.11N","161 59 57.48W" },
	{ "04MHE3300000032", "00 54 11.49S","156 00 30.27W" },
	{ "15XWJ5956055217", "79 45 00.01N","090 00 01.79W" },
	{ "58CDS4043044783", "79 44 59.99S","162 00 00.00E" },
	{ "",                "",            ""              }
};

int CStestH (int verbose,long32_t duration)
{
	extern double cs_Degree;
	extern double cs_ParmTest;		/* .1 seconds of arc in degrees. */

	extern int (*CS_usrCsDefPtr)(struct cs_Csdef_ *ptr,Const char *keyName);
	extern int (*CS_usrDtDefPtr)(struct cs_Dtdef_ *ptr,Const char *keyName);
	extern int (*CS_usrElDefPtr)(struct cs_Eldef_ *ptr,Const char *keyName);
	extern double (*CS_usrUnitPtr)(short type,Const char *unitName);

	int err_cnt;
	int idx;
	int status;
	int length;
	int iStat;
	int msDecPnt,csDecPnt;
	int msSign,csSign;
	size_t strLen;
	long32_t lStat;

	Const char *ptr;
	Const char *csEcvtPtr;
	Const char *msEcvtPtr;
	struct cs_Csdef_ *csPtr;
	struct cs_Dtdef_ *dtPtr;
	struct cs_Eldef_ *elPtr;
	struct _mgrsTable *tblPtr;
	struct cs_Mgrs_ *mgrsPtr;
	
	double testValue;

	double testLatLng [2];
	double rsltLatLng [2];
	double delLatLng [2];
	char rsltMgrs [32];

	printf ("Testing miscellaneous functions\n");
	err_cnt = 0;
	duration *= 2000;

	/* The _ecvt test works fine in the Microsoft environment.  IN the Linux
	   environment, we get discrepancies in the least significant digit for
	   some reason.  Haven't yet figured out a way to verify results in the
	   Linux environment. */
#if _RUN_TIME < _rt_UNIXPCC
	/* The _ecvt function is not ANSI standard, so we had to write our own.
	   This gismo pretty rigorously test our version against the version
	   in the C run time library.  Your C run time library may not have
	   an _ecvt function, but mine does; so we use it to perform the test.
	   In this manner, we can geta very rigorous test with a minimal amount
	   of code. */
	for (idx = 0;idx < duration;idx += 1)
	{
		testValue = CStestRN (-9.999E+20,9.999E+20);
		for (length = 5;length < 18;length += 1)
		{
			msEcvtPtr = _ecvt (testValue,length,&msDecPnt,&msSign);
			csEcvtPtr = CS_ecvt (testValue,length,&csDecPnt,&csSign);
			if (csDecPnt != msDecPnt || csSign != msSign || strcmp (csEcvtPtr,msEcvtPtr))
			{
				err_cnt += 1;
				printf ("CS_ecvt failure on %18.4f, length = %d.\n",testValue,length);
				if (verbose)
				{
					printf ("                CS-MAP (%s :: %d)\n",csEcvtPtr,csDecPnt);
					printf ("                   CRT (%s :: %d)\n",msEcvtPtr,msDecPnt);
				}
			}
			if (err_cnt > 10) break;
		}
		/* Use this to test values less than one. */
		testValue = CStestRN (-1.0,1.0);
		for (length = 5;length < 18;length += 1)
		{
			msEcvtPtr = _ecvt (testValue,length,&msDecPnt,&msSign);
			csEcvtPtr = CS_ecvt (testValue,length,&csDecPnt,&csSign);
			if (csDecPnt != msDecPnt || csSign != msSign || strcmp (csEcvtPtr,msEcvtPtr))
			{
				err_cnt += 1;
				printf ("CS_ecvt failure on %18.4f, length = %d.\n",testValue,length);
				if (verbose)
				{
					printf ("                CS-MAP (%s :: %d)\n",csEcvtPtr,csDecPnt);
					printf ("                   CRT (%s :: %d)\n",msEcvtPtr,msDecPnt);
				}
			}
			if (err_cnt > 10) break;
		}
		if (err_cnt > 10) break;
	}
	/* Zero is a problematical case. */
	testValue = 0.0;
	for (length = 5;length < 18;length += 1)
	{
		msEcvtPtr = _ecvt (testValue,length,&msDecPnt,&msSign);
		csEcvtPtr = CS_ecvt (testValue,length,&csDecPnt,&csSign);
		if (csDecPnt != msDecPnt || csSign != msSign || strcmp (csEcvtPtr,msEcvtPtr))
		{
			err_cnt += 1;
			printf ("CS_ecvt failure on %18.4f, length = %d.\n",testValue,length);
			if (verbose)
			{
				printf ("                CS-MAP (%s :: %d)\n",csEcvtPtr,csDecPnt);
				printf ("                   CRT (%s :: %d)\n",msEcvtPtr,msDecPnt);
			}
		}
		if (err_cnt > 10) break;
	}
#endif

	/* Tests the CS_strnicmp function.  There is no strinicmp function in the
	   ANSI library, so we had to write our own. */
	for (idx = 0;hTable1 [idx].cmpLength >= 0 && err_cnt < 4;idx += 1)
	{
		if (hTable1 [idx].cmpLength > 0)
		{
			status = CS_strnicmp (hTable1 [idx].str1,hTable1 [idx].str2,hTable1 [idx].cmpLength);
		}
		else
		{
			status = CS_stricmp (hTable1 [idx].str1,hTable1 [idx].str2);
		}
		if (((hTable1 [idx].cmpValue <  0) && (status >= 0)) ||
			((hTable1 [idx].cmpValue == 0) && (status != 0)) ||
			((hTable1 [idx].cmpValue >  0) && (status <= 0))
		   )
		{
			printf ("Case insensitive functions failure, index = %d.\n",idx);
			err_cnt += 1;
		}
	}
	for (idx = 0;hTable2 [idx].nullFlag >= 0 && err_cnt < 5;idx += 1)
	{
		strLen = strlen (hTable2 [idx].str3);
		ptr = CS_stristr (hTable2 [idx].str1,hTable2 [idx].str2);
		if (( hTable2 [idx].nullFlag && ptr != NULL) ||
			(!hTable2 [idx].nullFlag && strncmp (ptr,hTable2 [idx].str3,strLen))
		   )
		{
			printf ("Case insensitive functions 2 failure, index = %d.\n",idx);
			err_cnt += 1;
		}
	}

	CS_usrCsDefPtr = CSusrCsDef;
	CS_usrDtDefPtr = CSusrDtDef;
	CS_usrElDefPtr = CSusrElDef;
	CS_usrUnitPtr = CSusrUnit;

	csPtr = CS_csdef ("XXX");
	if (csPtr == NULL || strcmp (csPtr->desc_nm,"XXX"))
	{
		printf ("CS_usrCsDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (csPtr != NULL) CS_free (csPtr);

	csPtr = CS_csdef ("YYY");
	if (csPtr != NULL)
	{
		printf ("CS_usrCsDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (csPtr != NULL) CS_free (csPtr);

	csPtr = CS_csdef ("LL");
	if (csPtr == NULL || strcmp (csPtr->key_nm,"LL"))
	{
		printf ("CS_usrCsDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (csPtr != NULL) CS_free (csPtr);

	dtPtr = CS_dtdef ("XXX");
	if (dtPtr == NULL || strcmp (dtPtr->name,"XXX"))
	{
		printf ("CS_usrDtDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (dtPtr != NULL) CS_free (dtPtr);

	dtPtr = CS_dtdef ("YYY");
	if (dtPtr != NULL)
	{
		printf ("CS_usrDtDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (dtPtr != NULL) CS_free (dtPtr);

	dtPtr = CS_dtdef ("WGS84");
	if (dtPtr == NULL || strcmp (dtPtr->key_nm,"WGS84"))
	{
		printf ("CS_usrDtDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (dtPtr != NULL) CS_free (dtPtr);

	elPtr = CS_eldef ("XXX");
	if (elPtr == NULL || strcmp (elPtr->name,"XXX"))
	{
		printf ("CS_usrElDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (elPtr != NULL) CS_free (elPtr);

	elPtr = CS_eldef ("YYY");
	if (elPtr != NULL)
	{
		printf ("CS_usrElDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (elPtr != NULL) CS_free (elPtr);

	elPtr = CS_eldef ("WGS84");
	if (elPtr == NULL || strcmp (elPtr->key_nm,"WGS84"))
	{
		printf ("CS_usrElDefPtr function failure.\n");
		err_cnt += 1;
	}
	if (elPtr != NULL) CS_free (elPtr);

	if (CS_unitlu (cs_UTYP_LEN,"XXX") != 17.0)
	{
		printf ("CS_usrUnitPtr function failure.\n");
		err_cnt += 1;
	}
	if (CS_unitlu (cs_UTYP_LEN,"YYY") != 0.0)
	{
		printf ("CS_usrUnitPtr function failure.\n");
		err_cnt += 1;
	}
	if (CS_unitlu (cs_UTYP_LEN,"METER") != 1.0)
	{
		printf ("CS_usrUnitPtr function failure.\n");
		err_cnt += 1;
	}

	CS_usrCsDefPtr = NULL;
	CS_usrDtDefPtr = NULL;
	CS_usrElDefPtr = NULL;
	CS_usrUnitPtr = NULL;

	/* Test the MGRS stuff.  We have a table which we got from an
	   external source.  We simply work through the table.  The
	   table is based on WGS84. */
	mgrsPtr = CSnewMgrs (6378137.0,0.00669438000426089,FALSE);
	if (mgrsPtr == NULL)
	{
		printf ("cs_Mgrs_ construction failure.\n");
		err_cnt += 1;
		return (err_cnt);
	}
	for (tblPtr = mgrsTable;tblPtr->mgrs [0] != '\0';tblPtr += 1)
	{
		lStat = CS_atof (&testLatLng [0],tblPtr->lng);
		if (lStat < 0L)
		{
			printf ("MGRS test table internal failure.\n");
			err_cnt += 1;
			continue;
		}
		lStat = CS_atof (&testLatLng [1],tblPtr->lat);
		if (lStat < 0L)
		{
			printf ("MGRS test table internal failure.\n");
			err_cnt += 1;
			continue;
		}

		/* We have a lat/long,  Convert the MGRS string to
		   lat/long, and compare the results. */
		iStat =  CScalcLlFromMgrs (mgrsPtr,rsltLatLng,tblPtr->mgrs);
		if (iStat < 0L)
		{
			printf ("CScalcLlFromMgrs failure.\n");
			err_cnt += 1;
			continue;
		}
		delLatLng [0] = fabs (testLatLng [0] - rsltLatLng [0]); 
		delLatLng [1] = fabs (testLatLng [1] - rsltLatLng [1]);
		testValue = cs_ParmTest / cos (testLatLng [1] * cs_Degree);
		if (delLatLng [0] > testValue || delLatLng [1] > testValue)
		{
			printf ("CScalcLlFromMgrs failure.\n");
			err_cnt += 1;
		}

		/* Convert the lat/long to MGRS and check the result. */
		iStat = CScalcMgrsFromLl (mgrsPtr,rsltMgrs,sizeof (rsltMgrs),testLatLng,5); 
		if (iStat < 0L)
		{
			printf ("CScalcMgrsFromLl failure.\n");
			err_cnt += 1;
			continue;
		}
		if (CS_stricmp (rsltMgrs,tblPtr->mgrs))
		{
			printf ("CScalcMgrsFromLl failure.\n");
			err_cnt += 1;
			continue;
		}
	}
	CS_free (mgrsPtr);

#ifdef __SKIP__
	/* Test the Geoid height functions. */
	testLatLng [0] = -105.123456789;
	testLatLng [1] = 39.123456789;
	status = CS_geoidHgt (testLatLng,&testValue);
	if (status < 0)
	{
		char errMsg [256];
		CS_errmsg (errMsg,sizeof (errMsg));
		printf ("CS_geoidHgt fatal error at %d. Reason: %s\n",__LINE__,errMsg);
		err_cnt += 1;
	}
	else if (status != 0)
	{
		printf ("CS_geoidHgt range error at %d.\n",__LINE__);
		err_cnt += 1;
	}
	else if (fabs (testValue + 14.795) > 0.0005)	/* Geoid99 numbers */
	{
		printf ("CS_geoidHgt calculation failure at %d (%f).\n",__LINE__,testValue);
		err_cnt += 1;
	}
	testLatLng [0] = -65.123456789;
	testLatLng [1] = 27.123456789;
	status = CS_geoidHgt (testLatLng,&testValue);
	if (status < 0)
	{
		char errMsg [256];
		CS_errmsg (errMsg,sizeof (errMsg));
		printf ("CS_geoidHgt fatal error at %d. Reason: %s\n",__LINE__,errMsg);
		err_cnt += 1;
	}
	else if (status != 0)
	{
		printf ("CS_geoidHgt range error at %d.\n",__LINE__);
		err_cnt += 1;
	}
	else if (fabs (testValue + 47.053) > 0.0005)	/* Geoid99 numbers */
	{
		printf ("CS_geoidHgt calculation failure at %d (%f).\n",__LINE__,testValue);
		err_cnt += 1;
	}
	CS_geoidCls ();
#endif

	return (err_cnt);
}
