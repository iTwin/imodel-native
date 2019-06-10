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

//lint -esym(534,CS_stncp,setlocale,CS_trim,CS_ll2cs)
//lint -esym(752,cs_Doserr,cs_Errno,cs_Error,cs_ErrSup,csErrlng,csErrlat)

#include <locale.h>
#include "cs_map.h"
#include "cs_wkt.h"
#include "csNameMapperSupport.h"
#include "cs_Test.h"

extern int cs_Error;
extern int cs_Errno;
extern int csErrlng;
extern int csErrlat;
extern unsigned short cs_ErrSup;
#if _RUN_TIME <= _rt_UNIXPCC
extern ulong32_t cs_Doserr;
#endif
extern const unsigned long KcsNmMapNoNumber;
extern const unsigned long KcsNmInvNumber;

int CStestJ (int verbose,long32_t duration)
{
	int status;
	int err_cnt;
	size_t localeSize;
	unsigned epsgIndex;
	unsigned long epsgCode;

	enum ErcWktFlavor flavor = wktFlvrEsri;
	enum EcsMapSt csMapSt;
	enum EcsMapObjType objType;

	char* localeIdPtr;
	char* localePtr;

	const char *msiNamePtr;
	struct cs_Csprm_ *msiCS;
	struct cs_Csprm_ *epsgCS;

	char csMapName [64];
	char csWktBufr [2048];
	char errMesg   [2048];

	double xyz [3];
	double llh [3];
	double msiLlh [3];
	double epsgLlh [3];
	double msiXyz [3];
	double epsgXyz [3];

	struct cs_Csdef_ csDef;
	struct cs_Dtdef_ dtDef;
	struct cs_Eldef_ elDef;

	localePtr = 0;
	if (verbose)
	{
		printf ("Setting locale to \"C\".\n");
	}
	localeIdPtr = setlocale (LC_ALL,NULL);
	if (localeIdPtr != 0)
	{
		localeSize = strlen (localeIdPtr) + 1;
		localePtr = malloc (localeSize);
		if (localePtr != 0)
		{
			CS_stncp (localePtr,localeIdPtr,(int)localeSize);
		}
	}
	setlocale (LC_ALL,"C");

	printf ("Checking Well Known Text export.\n");

	duration *= 3;
	err_cnt = 0;
	epsgIndex = 0;
	objType = csMapGeographicCSysKeyName;
	for (;;)
	{
		msiCS = epsgCS = NULL;
		if (err_cnt >= duration)
		{
			printf ("Abandoning test J due to numerous errors.\n");
			break;
		}

		/* Loop once for each EPSG code in the name mapper database. */
		epsgCode = csGetIdsByIdx (objType,csMapFlvrEpsg,epsgIndex++);
		if (epsgCode == KcsNmInvNumber)
		{
			if (objType != csMapGeographicCSysKeyName)
			{
				/* We're done. */
				break;
			}
			
			/* Done with the geographics.  Do the Projectives. */
			objType = csMapProjectedCSysKeyName;
			epsgIndex = 0;
			continue;
		}
		if (epsgCode == KcsNmMapNoNumber)
		{
			/* This entry did not have an ID of the indicated type??? */
			continue;
		}

		/* Get the CS-MAP name for the associated item. */
		csMapSt = csMapIdToNameC (objType,csMapName,sizeof (csMapName),csMapFlvrAutodesk,csMapFlvrEpsg,epsgCode);
		if (csMapSt != csMapOk)
		{
			/* The name mapper has no CS-MAP name for this EPSG code. */
			continue;
		}
		if (!CS_csIsValid (csMapName))
		{
			/* We don't have a definition for this name.  A problem to be sure,
			   but that will be tested and reported elsewhere.  Here we are
			   testing WKT conversions. */
			continue;
		}
		msiNamePtr = csMapName;

		if (verbose)
		{
			printf ("Comparing %s with EPSG:%ld.          \r",msiNamePtr,epsgCode);
		}

		status = CS_cs2Wkt (csWktBufr,sizeof (csWktBufr),msiNamePtr,flavor);
		if (status != 0)
		{
			err_cnt += 1;
			CS_errmsg (errMesg,sizeof (errMesg));
			printf ("\rHigh level conversion of %s to WKT failed.  Reason: %s\n",msiNamePtr,errMesg);
		}
		else
		{
			status = CS_wktToCsEx (&csDef,&dtDef,&elDef,flavor,csWktBufr,0);
			epsgCS = CScsloc1 (&csDef);
			if (epsgCS == 0)
			{
				err_cnt += 1;
				CS_errmsg (errMesg,sizeof (errMesg));
				printf ("\rCouldn't setup WKT coordinate system.  Reason: %s\n",errMesg);
				continue;
			}
		}
		if (status != 0) continue;

		msiCS = CS_csloc (msiNamePtr);
		if (msiCS == NULL)
		{
			err_cnt += 1;
			CS_free (epsgCS);
			CS_errmsg (errMesg,sizeof (errMesg));
			printf ("\rCouldn't setup coordinate system named %s.\n\tReason: %s\n",msiNamePtr,errMesg);
			continue;
		}

		/* We use the useful range in the MSI definition to compute a suitable
		   pair of x and y coordinates for the test.  (EPSG definitions don't
		   have such information.) */
		llh [0] = msiCS->cent_mer + msiCS->min_ll [0] * 0.65;
		llh [1] = msiCS->min_ll [1] + (msiCS->max_ll [1] - msiCS->min_ll [1]) * 0.65;
		llh [2] = 0.0;
		status = CS_ll2cs (msiCS,xyz,llh);
		if (status != 0)
		{
			printf ("Test J algorithm failure at line %d, CS = %s.\n",__LINE__,msiNamePtr);
			err_cnt += 1;
			CS_free (msiCS);
			CS_free (epsgCS);
			continue;
		}
		
		/* OK, we can now perform the real test. */
		status = CS_cs2ll (msiCS,msiLlh,xyz);
		if (status != 0)
		{
			printf ("\rCouldn't convert %12.3f :: %12.3f to Lat/Lng via %s.\n",xyz [0],xyz [1],msiNamePtr);
			err_cnt += 1;
			CS_free (msiCS);
			CS_free (epsgCS);
			continue;
		}
		status = CS_ll2cs (msiCS,msiXyz,msiLlh);
		if (status != 0)
		{
			printf ("\rCouldn't convert %12.6f :: %12.6f to X/Y/Z via %s.\n",llh [0],llh [1],msiNamePtr);
			err_cnt += 1;
			CS_free (msiCS);
			CS_free (epsgCS);
			continue;
		}

		status = CS_cs2ll (epsgCS,epsgLlh,xyz);
		if (status != 0)
		{
			printf ("\rCouldn't convert %12.3f :: %12.3f to Lat/Lng via %s.\n",xyz [0],xyz [1],msiNamePtr);
			err_cnt += 1;
			CS_free (msiCS);
			CS_free (epsgCS);
			continue;
		}
		status = CS_ll2cs (epsgCS,epsgXyz,epsgLlh);
		if (status != 0)
		{
			printf ("\rCouldn't convert %12.6f :: %12.6f to X/Y/Z via %s.\n",llh [0],llh [1],msiNamePtr);
			err_cnt += 1;
			CS_free (msiCS);
			CS_free (epsgCS);
			continue;
		}

		/* How did we do? */
		if (fabs (msiXyz [0] - epsgXyz [0]) > 0.0005 ||
			fabs (msiXyz [1] - epsgXyz [1]) > 0.0005 ||
			fabs (msiLlh [0] - epsgLlh [0]) > 0.0000002 ||
			fabs (msiLlh [1] - epsgLlh [1]) > 0.0000002)
		{
			printf ("\rComparison of pre/post WKT of %s failed.\n",msiNamePtr);
			err_cnt += 1;
			CS_free (msiCS);
			CS_free (epsgCS);
			continue;
		}
		CS_free (msiCS);	msiCS = NULL;
		CS_free (epsgCS);	epsgCS = NULL;
	}
	csReleaseNameMapper ();
	if (localePtr != 0)
	{
		if (verbose)
		{
			printf ("Resetting locale to \"%s\".\n",localePtr);
		}
		setlocale (LC_ALL,localePtr);
		free (localePtr);
	}
	return err_cnt;
}
