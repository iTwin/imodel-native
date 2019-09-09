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

extern char cs_MeKynm [128];
extern char cs_MeFunc [128];
extern double cs_Coords [3];

/*
	The following test exercises the WKT routines.  It simply converts
	a coordinate system definition to WKT and then back to our
	own binary interpretation.  We should have what we started with.
*/
int CStestI (int verbose,long32_t duration)
{
	extern struct cs_Grptbl_ cs_CsGrptbl [];
	extern struct cs_Prjtab_ cs_Prjtab [];

	int err_cnt;

	int status;
	size_t localeSize;
	enum ErcWktFlavor flavor;

	struct cs_Grptbl_ *tp;
	struct cs_Csgrplst_ *gp;
	struct cs_Csgrplst_ *grp_list;
	struct cs_Prjtab_ *prjPtr;
	struct cs_Prjtab_ *rsltPrjPtr;
	struct cs_Csdef_ *csDefPtr;
	struct cs_Dtdef_ *dtDefPtr;
	struct cs_Eldef_ *elDefPtr;
	char* localeIdPtr;
	char* localePtr;

	char msgBufr [256];
	char csWktBufr [2048];

	struct cs_Csdef_ csDefOriginal;
	struct cs_Csdef_ csDefImported;
	struct cs_Dtdef_ dtDefOriginal;
	struct cs_Dtdef_ dtDefImported;
	struct cs_Eldef_ elDefOriginal;
	struct cs_Eldef_ elDefImported;

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

	printf ("Using WKT import to check WKT export, flavor = ESRI\n");

	/* As of now, we only do one flavor.  We should loop and do all flavors. */
	flavor = wktFlvrEsri;

	/* Loop through the group table and fetch a linked list
	   for each group.  Count the number in the group and
	   add to the total in the group. */
	err_cnt = 0;
	for (tp = cs_CsGrptbl;tp->group [0] != 0;tp += 1)
	{
		if (err_cnt >= 50)
		{
			printf ("Abandoning test I due to numerous errors.\n");
			break;
		}

		/* We skip the test group, as the test contain some pretty
		   weird stuff. */
		if (!CS_stricmp (tp->group,"TEST")) continue;

		/* Loop through the definitions in this group. */
		CS_csgrp (tp->group,&grp_list);
		for (gp = grp_list;gp != NULL;gp = gp->next)
		{
			csDefPtr = CS_csdef (gp->key_nm);
			if (csDefPtr == NULL)
			{
				printf ("\rCouldn't obtain coordinate system definition for %s.\n",gp->key_nm);
				err_cnt += 1;
				continue;
			}
			memcpy (&csDefOriginal,csDefPtr,sizeof (csDefOriginal));
			CS_free (csDefPtr);
			strcpy (cs_MeKynm,csDefOriginal.key_nm);

			/* Locate the projection in the projection table.  We need this for access
			   to the flag word for this projection. */
			for (prjPtr = cs_Prjtab;prjPtr->code != cs_PRJCOD_END;prjPtr += 1)
			{
				if (!strcmp (csDefOriginal.prj_knm,prjPtr->key_nm)) break;
			}
			if (prjPtr->code == cs_PRJCOD_END) continue;

			/* We skip several projection types which are not generally supported by WKT, if the flavor
			   is not Autodesk. */
			if (flavor != wktFlvrAutodesk &&
				(prjPtr->code == cs_PRJCOD_MODPC    ||
				 prjPtr->code == cs_PRJCOD_HMLSN    ||
				 prjPtr->code == cs_PRJCOD_NACYL    ||
				 prjPtr->code == cs_PRJCOD_TACYL    ||
				 prjPtr->code == cs_PRJCOD_BPCNC    ||
				 prjPtr->code == cs_PRJCOD_SSTRO    ||
				 prjPtr->code == cs_PRJCOD_WCCSL    ||
				 prjPtr->code == cs_PRJCOD_WCCST    ||
				 prjPtr->code == cs_PRJCOD_MNDOTT   ||
				 prjPtr->code == cs_PRJCOD_MNDOTL   ||
				 prjPtr->code == cs_PRJCOD_SOTRM    ||
				 prjPtr->code == cs_PRJCOD_HOM2UV   ||
				 prjPtr->code == cs_PRJCOD_HOM1UV   ||
				 prjPtr->code == cs_PRJCOD_KRVK95   ||
				 prjPtr->code == cs_PRJCOD_TRMERAF  ||
				 prjPtr->code == cs_PRJCOD_OSTN97   ||
				 prjPtr->code == cs_PRJCOD_AZEDE    ||
				 prjPtr->code == cs_PRJCOD_OSTN02   ||
				 prjPtr->code == cs_PRJCOD_SYS34_99 ||
				 prjPtr->code == cs_PRJCOD_TRMRKRG  ||
				 prjPtr->code == cs_PRJCOD_LMBRTAF  ||
			     prjPtr->code == cs_PRJCOD_MSTRO
			    )
			   )
			{
				continue;
			}

			if (verbose)
			{
				printf ("Testing WKT import/export using %s.              \r",gp->key_nm);
			}

			/* We do not attempt to convert anything that is cartographically
			   referenced.  Since there is no real WKT specification, we don't
			   know how to encode a definition which is not referenced to a
			   datum. */
			if (csDefOriginal.dat_knm [0] == '\0') continue;

			/* Locate the associated datum and ellipsoid definitions. */
			dtDefPtr = CS_dtdef (csDefOriginal.dat_knm);
			if (dtDefPtr == NULL)
			{
				char errMsg [256];

				CS_errmsg (errMsg,sizeof (errMsg));
				printf ("\rCouldn't obtain datum definition for %s (Reason: %s).\n",csDefOriginal.dat_knm,errMsg);
				err_cnt += 1;
				continue;
			}
			memcpy (&dtDefOriginal,dtDefPtr,sizeof (dtDefOriginal));
			CS_free (dtDefPtr);

			elDefPtr = CS_eldef (dtDefOriginal.ell_knm);
			if (elDefPtr == NULL)
			{
				char errMsg [256];

				CS_errmsg (errMsg,sizeof (errMsg));
				printf ("\rCouldn't obtain ellipsoid definition for %s (Reason: %s).\n",dtDefOriginal.ell_knm,errMsg);
				err_cnt += 1;
				continue;
			}
			memcpy (&elDefOriginal,elDefPtr,sizeof (elDefOriginal));
			CS_free (elDefPtr);

			/* Convert this definition to WKT.  Please note that (currently)
			   we rely on the CScsToWkt function to access the ellipsoid and
			   datum dictionaries to get those elements which are required. */
			status = CScs2Wkt (csWktBufr,sizeof (csWktBufr),flavor,&csDefOriginal,&dtDefOriginal,&elDefOriginal);
			if (status < 0)
			{
				char errMsg [256];

				err_cnt += 1;
				CS_errmsg (errMsg,sizeof (errMsg));
				printf ("\rCScsToWkt failed on %s; reason given: %s\n",csDefOriginal.key_nm,errMsg);
				continue;
			}

			/* Convert the WKT back to our internal binary form. */
			status = CS_wktToCsEx (&csDefImported,&dtDefImported,&elDefImported,flavor,csWktBufr,0);
			if (status < 0)
			{
				char errMsg [256];

				err_cnt += 1;
				CS_errmsg (errMsg,sizeof (errMsg));
				printf ("\rCS_wktToCs failed on %s; reason given: %s\n",csDefOriginal.key_nm,errMsg);
				continue;
			}

			/* We know that an LMTAN will be converted to LM1SP.  So we filter this
			   out here. */
			if (prjPtr->code == cs_PRJCOD_LMTAN)
			{
				for (rsltPrjPtr = cs_Prjtab;rsltPrjPtr->code != cs_PRJCOD_END;rsltPrjPtr += 1)
				{
					if (!strcmp (csDefImported.prj_knm,rsltPrjPtr->key_nm)) break;
				}
				if (rsltPrjPtr->code == cs_PRJCOD_LM1SP)
				{
					CS_stncp (csDefImported.prj_knm,"LMTAN",sizeof (csDefImported.prj_knm));
				}
			}

			/* See if they compare; if not report the error. */
			status = CS_csDefCmp (&csDefOriginal,&csDefImported,msgBufr,sizeof (msgBufr));
			if (status > 0)
			{
				printf ("\rImported version of coordinate system %s does not match original; %s\n",csDefOriginal.key_nm,msgBufr);
				err_cnt += 1;
			}
			
			/* We skip the datum test as the WKT datum stuff is pretty flakey. */
			//status = CS_dtDefCmp (&dtDefOriginal,&dtDefImported,msgBufr,sizeof (msgBufr));
			//if (status > 0)
			//{
			//	printf ("\rImported version of datum %s does not match original; %s\n",dtDefOriginal.key_nm,msgBufr);
			//	err_cnt += 1;
			//}

			status = CS_elDefCmp (&elDefOriginal,&elDefImported,msgBufr,sizeof (msgBufr));
			if (status > 0)
			{
				printf ("\rImported version of ellipsoid %s does not match original; %s\n",dtDefOriginal.key_nm,msgBufr);
				err_cnt += 1;
			}
		}
		CS_csgrpf (grp_list);
	}
	if (verbose)
	{
		printf ("\r                                                  \n");
	}

	if (localePtr != 0)
	{
		if (verbose)
		{
			printf ("Resetting locale to \"%s\".\n",localePtr);
		}
		setlocale (LC_ALL,localePtr);
		free (localePtr);
	}

	return (err_cnt);
}
