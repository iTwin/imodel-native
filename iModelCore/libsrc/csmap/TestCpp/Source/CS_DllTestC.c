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

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "cs_mapDll.h"

/*lint -esym(534,CS_ftoa)  */
#if defined (_MSC_VER) && _MSC_VER >= 800	/* MS Visual C++ 1.0 or later */
#	pragma warning( disable : 4100 )		// unreferenced formal parameter
#endif

int main (int agrc,char* argv[])
{
	int st;
	int index;
	int index2;
	int unitType;

	unsigned long epsgId;

	double geoidHgt;
	double unitFactor;

	double xyz [3];
	double llh [3];

	char unitName [32];
	char groupName [64];
	char groupDescr [256];

	char lng [32];
	char lat [32];
	char hgt [32];

	struct cs_Csgrplst_ grpItem;

	/* Calling CS_altdr with a NULL argument indicates that the DLL is to be
	   initialized using the environmental variable CS_MAP_DIR. */
	st = CS_altdr (0);

	if (st == 0)
	{
		xyz [0] =  450000.000;
		xyz [1] = 4000000.000;
		xyz [2] = 0.0;

		llh [0] = xyz [0];
		llh [1] = xyz [1];
		llh [2] = xyz [2];
		
		st = CS_cnvrt ("UTM27-13","LL83",llh);
		if (st == 0)
		{
			CS_ftoa (lng,sizeof (lng),llh[0],cs_ATOF_LNGDFLT);
			CS_ftoa (lat,sizeof (lat),llh[1],cs_ATOF_LATDFLT);
			CS_ftoa (hgt,sizeof (hgt),llh[2],4L);
			printf ("%s :: %s :: %s  st = %d\n",lng,lat,hgt,st);
		}

		st = CS_geoidHgt (llh,&geoidHgt);
		printf ("Geoid Height (%f::%f) = %.3f  [st = %d]\n",llh[0],llh[1],geoidHgt,st);
		CS_geoidCls ();

		/* An example of how to retrieve lists of names of things from the DLL
		   without using advanced techniques such as STL. */
		index = 0;
		while (CS_csGrpEnum (index++,groupName,sizeof (groupName),groupDescr,sizeof (groupDescr)))
		{
			printf ("Group: %s   -->  %s\n",groupName,groupDescr);
			index2 = 0;
			while (CS_csEnumByGroup (index2++,groupName,&grpItem))
			{
				epsgId = csMapNameToIdC (csMapProjGeoCSys,csMapFlvrEpsg,csMapFlvrAutodesk,grpItem.key_nm);
				if (epsgId > 32767UL)
				{
					epsgId = 0UL;
				}
				printf ("\t\t%s -> %s [%u]\n",grpItem.key_nm,grpItem.descr,epsgId);
			}
		}

		index = 0;
		unitType = cs_UTYP_LEN;
		while (CS_unEnum (index++,unitType,unitName,sizeof (unitName)))
		{
			unitFactor = CS_unitlu ((short)unitType,unitName);		/* Cast requirement is an unfortunate legacy defect */
			printf ("Linear Unit: %s   -->  %f\n",unitName,unitFactor);
		}
	}

	CS_geoidCls ();
	CS_recvr ();
	csReleaseNameMapper ();

	exit (0);
}
