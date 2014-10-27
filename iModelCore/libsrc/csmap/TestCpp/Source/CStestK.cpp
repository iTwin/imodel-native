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
//lint -esym(752,cs_Doserr,cs_Errno,cs_ErrSup,csErrlng,csErrlat)

#include "csTestCpp.hpp"

extern "C"
{
	extern "C" int cs_Error;
	extern "C" int cs_Errno;
	extern "C" int csErrlng;
	extern "C" int csErrlat;
	extern "C" unsigned short cs_ErrSup;

	#if _RUN_TIME <= _rt_UNIXPCC
	extern "C" ulong32_t cs_Doserr;
	#endif

	extern "C" int (*CS_usrCsDefPtr)(struct cs_Csdef_ *ptr,Const char *keyName);
	extern "C" int (*CS_usrDtDefPtr)(struct cs_Dtdef_ *ptr,Const char *keyName);
	extern "C" int (*CS_usrElDefPtr)(struct cs_Eldef_ *ptr,Const char *keyName);
}

extern char cs_TestDir [];
extern char* cs_TestDirP;

struct csTestKIgnores_
{
	ErcWktFlavor flavor;
	char csMapName [32];	
	char* comment [128];
} csTestKIgnores [] =
{
	// The ESRI WKT data we have been using for testing is more than a decade old.  It is unlikely
	// that the discrepancies listed in the following table still exist in any ESRI product.  But
	// the discrepancies do still exist in the ancient test data file we are using.
	{ wktFlvrEsri,        "Carthage.TM-11NE", "ESRI GCS unit is grad, should be degree?  Affects interpretation of central meridian."                           },
	{ wktFlvrEsri,   "Dabola1981.UTM-28N/01", "ESRI Has incorrect ellipsoid.  Is Clrk_1880-RGN, should be(?) Clrk-IGN."                                         },
	{ wktFlvrEsri,   "Dabola1981.UTM-29N/01", "ESRI Has incorrect ellipsoid.  Is Clrk_1880-RGN, should be(?) Clrk-IGN."                                         },
	{ wktFlvrEsri,            "HUN-EOV72-7P", "ESRI uses Hotine approximation to the Oblique Cynlindrical."                                                     },
	{ wktFlvrEsri,                 "IND-0/a", "ESRI uses datum variant EPSG Operation code 1533; we use 1155 (ellipsoid different)."                            },
	{ wktFlvrEsri,                 "IND-I/a", "ESRI uses datum variant EPSG Operation code 1533; we use 1155 (ellipsoid different)."                            },
	{ wktFlvrEsri,               "IND-IIA/a", "ESRI uses datum variant EPSG Operation code 1533; we use 1155 (ellipsoid different)."                            },
	{ wktFlvrEsri,               "IND-IIB/a", "ESRI uses datum variant EPSG Operation code 1533; we use 1155 (ellipsoid different)."                            },
	{ wktFlvrEsri,              "IND-IIIA/a", "ESRI uses datum variant EPSG Operation code 1533; we use 1155 (ellipsoid different)."                            },
	{ wktFlvrEsri,               "IND-IVA/a", "ESRI uses datum variant EPSG Operation code 1533; we use 1155 (ellipsoid different)."                            },
	{ wktFlvrEsri, "Kalianpur1937.India-IIb", "ESRI false origin differs from CS-MAP and EPSG; assuming EPSG is correct."                                       },
	{ wktFlvrEsri,   "Kalianpur1975.India-I", "ESRI false origin differs from CS-MAP and EPSG; assuming EPSG is correct."                                       },
	{ wktFlvrEsri, "Kalianpur1975.India-IIa", "ESRI false origin differs from CS-MAP and EPSG; assuming EPSG is correct."                                       },
	{ wktFlvrEsri, "Kalianpur1975.India-IIb", "ESRI false origin differs from CS-MAP and EPSG; assuming EPSG is correct."                                       },
	{ wktFlvrEsri, "Kalianpur1975.India-III", "ESRI false origin differs from CS-MAP and EPSG; assuming EPSG is correct."                                       },
	{ wktFlvrEsri,  "Kalianpur1975.India-IV", "ESRI false origin differs from CS-MAP and EPSG; assuming EPSG is correct."                                       },
	{ wktFlvrEsri,       "KuwaitUtility.KTM", "ESRI has scale reduction of 0.9996; EPSG says 1.0.  Choosing EPSG over ESRI here."                               },
	{ wktFlvrEsri,       "NAD27.CubaNorte/1", "ESRI uses 1SP Lambert, CS-MAP and EPSG uses the 2SP Lambert. Thus, the differences are difficult to reconcile."  },
	{ wktFlvrEsri,         "NAD27.CubaSur/1", "ESRI uses 1SP Lambert, CS-MAP and EPSG uses the 2SP Lambert. Thus, the differences are difficult to reconcile."  },
	{ wktFlvrEsri,     "OSNI52.IrishNtlGrid", "ESRI has Scale reduction at 1.000035; CS-MAP and EPSG say 1.0.  Who knows?"                                      },
	{ wktFlvrEsri,  "Rassadiran_1.NakhlTaqi", "ESRI says org_lat == 27 34 7.7837; EPSG & CS_MAP say org_lat == 27 31 7.7837."                                   },
	{ wktFlvrEsri,  "RGN/91-93.NewCaledonia", "ESRI ellipsoid is International 1924; should be GRS 1980?."                                                      },
	{ wktFlvrEsri,      "ST87/Ouvea.UTM-58S", "ESRI ellipsoid is International 1924; should be GRS WGS84?."                                                     },
	{ wktFlvrEsri,    "TMBLI-B.RSOBorneo.ch", "ESRI says false arigin is zero:zero;; CS-MAP and EPSG say otherwise."                                            },
	{ wktFlvrEsri,    "TMBLI-B.RSOBorneo.ft", "ESRI says false arigin is zero:zero;; CS-MAP and EPSG say otherwise."                                            },
	{ wktFlvrEsri,     "TMBLI-B.RSOBorneo.m", "ESRI says false arigin is zero:zero;; CS-MAP and EPSG say otherwise."                                            },
	{ wktFlvrEsri,            "WGS84.TM-6NE", "ESRI says false northing is 10,000,000.0; EPSG says zero.  We go with EPSG."                                     },
	{ wktFlvrEsri,             "HD72/7P.EOV", "Does not appear that ESRI has a parameter for Standard parallel as CS-MAP does"                                  },
	{ wktFlvrNone,                        "", "End of table marker."                                                                                            }
};
int CStestK (bool verbose,long32_t duration)
{
	int err_cnt;
	size_t localeSize;

	enum ErcWktFlavor flavor = wktFlvrEsri;
	enum EcsNameFlavor nmFlavor;
	enum EcsMapSt csMapSt;

	char *cp;
	char* localeIdPtr;
	char* localePtr;

	struct cs_Csprm_ *msiCS;
	struct cs_Csprm_ *wktCS;
	FILE* wktStream;
	FILE* parseReport;
	FILE* mapReport;
	FILE* csMapReport;
	FILE* cmpReport;

	long32_t lineNbr;

	double deltaX;
	double deltaY;
	double tolX;
	double tolY;

	char dictName [64];
	char csWktBufr [2048];
	char errMesg   [2048];

	double testInput [3];
	double msiResult [3];
	double wktResult [3];

	localePtr = 0;
	if (verbose)
	{
		printf ("Setting locale to \"C\".\n");
	}
	localeIdPtr = setlocale (LC_ALL,NULL);
	if (localeIdPtr != 0)
	{
		localeSize = strlen (localeIdPtr) + 1;
		localePtr = (char*)malloc (localeSize);
		if (localePtr != 0)
		{
			CS_stncp (localePtr,localeIdPtr,(int)localeSize);
		}
	}
	setlocale (LC_ALL,"C");

	err_cnt = 0;
	parseReport = 0;
	mapReport = 0;
	csMapReport = 0;
	cmpReport = 0;
	duration *= 3;

	printf ("[ K] Checking Well Known Text import.\n");
	err_cnt = 0;
	nmFlavor = csWktFlvrToCsMapFlvr (flavor);

	/* If a file named WktTestFile.wkt exists, we open it and for each entry in
	   the file, we perform the following test:
	   1> Read a WKT definition from the file.
	   2> Parse the WKT definition.
	   3> Extract the WKT (in this case ESRI) name
	   4> Map the ESRI name to an MSI name.
	   5> Get a cs_Csprm_ structure using the MSI name.
	   6> Get a cs_Csprm_ structure from the WKT string.
	   7> Determine a coordinate in the useful range of the definition.
	   8> Convert the point using both definitions.
	   9> Compare the results of the conversion.
	*/
	CS_stncp (cs_TestDirP,"wkts_esri.txt",MAXPATH);
	wktStream = fopen (cs_TestDir,"rt");
	if (wktStream == 0)
	{
		printf ("Couldn't open the WKT test file named %s, test K failed.\n","wkts_esri.txt");
		err_cnt += 1;
	}
	else
	{
		CS_stncp (cs_TestDirP,"WktParseReport.txt",MAXPATH);
		parseReport = fopen (cs_TestDirP,"wt");
		CS_stncp (cs_TestDirP,"WktMapReport.txt",MAXPATH);
		mapReport = fopen (cs_TestDirP,"wt");
		CS_stncp (cs_TestDirP,"WktCsMapFailures.txt",MAXPATH);
		csMapReport = fopen (cs_TestDirP,"wt");
		CS_stncp (cs_TestDirP,"WktCompareReport.txt",MAXPATH);
		cmpReport = fopen (cs_TestDirP,"wt");

		/* Activate the WKT capability for CS_csloc () */
		CS_usrCsDefPtr = CS_wktCsDefFunc;
		CS_usrDtDefPtr = CS_wktDtDefFunc;
		CS_usrElDefPtr = CS_wktElDefFunc;

		/* Read each of the WKT definitions in the test file. */
		wktCS = 0;
		msiCS = 0;
		lineNbr = 0;
		while (fgets (csWktBufr,sizeof (csWktBufr),wktStream) != NULL)
		{
			if (err_cnt >= duration)
			{
				printf ("Abandoning test K at line %ld due to numerous errors.\n",lineNbr);
				break;
			}
			lineNbr += 1;

			/* Skip commented lines */
			CS_trim (csWktBufr);
			if (csWktBufr [0] == '#')
			{
				continue;
			}
			
			/* Trim line comments from the WKT. */
			cp = strchr (csWktBufr,'#');
			if (cp != 0)
			{
				*cp = '\0';
			}

			/* Convert the WKT string to a coordinate conversion. */
			wktCS = CS_csloc (csWktBufr);
			if (wktCS == 0)
			{
				/* Here if the WKT parse and conversion failed.  If due to a
				   datum mapping failure or an unsupported projection we do not
				   report it at the current time.  This is a test of WKT. */
				if (cs_Error != cs_WKT_DTMAP && cs_Error != cs_WKT_INVPROJ)
				{
					CS_errmsg (errMesg,sizeof (errMesg));
					if (parseReport != 0)
					{
						fprintf (parseReport,"WKT parse failed (%ld): %s\n",lineNbr,errMesg);
					}
					printf ("Parsing WKT definition (%ld) failed. (WKT = %.48s)\n",lineNbr,csWktBufr);
					printf ("\tReason: %s\n",errMesg);
					err_cnt += 1;
				}
			}
			else
			{
				/* Map the WKT name to a CS-MAP Name. */
				flavor = (enum ErcWktFlavor)wktCS->csdef.wktFlvr;
				nmFlavor = csWktFlvrToCsMapFlvr (flavor);
				csMapSt = csMapNameToNameC (csMapProjGeoCSys,dictName,sizeof (dictName),csMapFlvrAutodesk,nmFlavor,wktCS->csdef.desc_nm);
				if (csMapSt == csMapNoMatch)
				{
					/* We disable this error at the current time, as it is not
					   a WKT error, but simply means that there are WKT
					   definitions in the test file which we don't have, or
					   which are not being properly mapped by the mapping
					   tables.  The latter case produces most of the
					   instances of the condition being reported here. */
					if (mapReport != 0)
					{
						fprintf (mapReport,"Line %ld: Mapping of WKT name '%s' to CS-MAP failed.\n",lineNbr,wktCS->csdef.desc_nm);
					}
					if (verbose)
					{
						printf ("At line %ld, mapping of WKT name '%s' to CS-MAP failed.\n",lineNbr,wktCS->csdef.desc_nm);
					}
					CS_free (wktCS);
				}
				else
				{
					/* See if this particular system is in the error table.  If
					   so, we skip it as it is a known condition as described in
					   the table. */
					csTestKIgnores_* tblPtr;
					for (tblPtr = csTestKIgnores;tblPtr->flavor != wktFlvrNone;tblPtr += 1)
					{
						if (tblPtr->flavor == flavor && !CS_stricmp (tblPtr->csMapName,dictName))
						{
							break;
						}
					}
					if (tblPtr->flavor != wktFlvrNone)
					{
						if (verbose)
						{
							printf ("Skipping test of %s;; reason %s\n",dictName,tblPtr->comment);
						}
						CS_free (wktCS);
						continue;
					}
					
					/* OK, we need to test this one. */
					msiCS = CS_csloc (dictName);
					if (msiCS == 0)
					{
						if (csMapReport != 0)
						{
							fprintf (csMapReport,"Line %ld: Name '%s' mapped to CS-MAP name '%s', but %s such did not exist.\n",lineNbr,wktCS->csdef.desc_nm,dictName,dictName);
						}
						printf ("WKT name '%s' mapped to MSI name '%s', but an MSI definition with that name did not exist.\n",wktCS->csdef.desc_nm,dictName);
						err_cnt += 1;
						CS_free (wktCS);
					}
					else
					{
						/* If the projection involved is one of the county systems in Wisconsin
						   or Minnesota, we skip it for now.  There are several reasons for this. */
						if (msiCS->prj_code == cs_PRJCOD_WCCSL ||
							msiCS->prj_code == cs_PRJCOD_WCCST ||
							msiCS->prj_code == cs_PRJCOD_MNDOTL ||
							msiCS->prj_code == cs_PRJCOD_MNDOTT)
						{
							CS_free (msiCS);
							CS_free (wktCS);
							continue;
						}

						/* We have two coordinate systems which are supposed to be
						   the same.  We construct a coordinate that is known to be
						   in the useful range, convert it using both conversions
						   and verify that the two results are the same. */
						testInput [0] = msiCS->cent_mer + msiCS->min_ll [0] * 0.5;
						testInput [1] = msiCS->min_ll [1] + (msiCS->max_ll [1] - msiCS->min_ll [1]) * 0.25;
						testInput [2] = 0.0;
						CS_ll2cs (msiCS,msiResult,testInput);
						CS_ll2cs (wktCS,wktResult,testInput);
						deltaX = fabs (msiResult [0] - wktResult [0]);
						deltaY = fabs (msiResult [1] - wktResult [1]);
						/* Ordinarily, I'd use 0.001 (1mm) as the tolerance.
						   However, WKT uses the flattening and EPSG truncates
						   the flattening of the Clarke 1880 (IGN) ellipsoid
						   to the point where is produces an error of about 1.8mm.
						   So, to keep the test from failing due to this, we
						   use a tolerance of 2mm. */
						tolX = 0.002;
						tolY = 0.002;
						if (deltaX > tolX || deltaY > tolY)
						{
							if (cmpReport != 0)
							{
								fprintf (cmpReport,"Line %ld: Comparision of WKT '%s' and CS-MAP '%s' failed [%f  %f].\n",lineNbr,wktCS->csdef.desc_nm,dictName,deltaX,deltaY);
							}
							printf ("Comparision of WKT '%s' and MSI '%s' failed.\n",wktCS->csdef.desc_nm,dictName);
							err_cnt += 1;

							// For debugging convenience.
							CS_ll2cs (msiCS,msiResult,testInput);
							CS_ll2cs (wktCS,wktResult,testInput);
						}
						//status = CScs2WktEx (csWktBufr,sizeof (csWktBufr),flavor,msiCS,0,0,1);
						CS_free (wktCS);
						CS_free (msiCS);
					}
				}
			}
		}

		if (parseReport != 0) { fclose (parseReport); parseReport = 0; }
		if (mapReport   != 0) { fclose (mapReport); mapReport = 0; }
		if (csMapReport != 0) { fclose (csMapReport); csMapReport = 0; }
		if (cmpReport   != 0) { fclose (cmpReport); cmpReport = 0; }

		fclose (wktStream);
		CS_usrCsDefPtr = 0;
		CS_usrDtDefPtr = 0;
		CS_usrElDefPtr = 0;
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
