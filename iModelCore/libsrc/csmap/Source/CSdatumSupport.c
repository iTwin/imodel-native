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

/****************************************************************************
	Coverage Implementation
*/

void CSinitCoverage (struct csGridCoverage_* thisPtr)
{
	extern double cs_Zero;
	extern double cs_K90;
	extern double cs_Km90;
	extern double cs_K180;
	extern double cs_Km180;

	/* Set the coverage to something that won't match anything. */
	thisPtr->southWest [LNG] = cs_K180;
	thisPtr->southWest [LAT] = cs_K90;
	thisPtr->northEast [LNG] = cs_Km180;
	thisPtr->northEast [LAT] = cs_Km90;
	thisPtr->density = cs_Zero;
}

double CStestCoverage (struct csGridCoverage_* thisPtr,Const double point [2])
{
	extern double cs_LlNoise;			/* 1.0E-12 */

	/* Return value is the density value used to select a specific source in
	   the case of overlap which is rare, but possible.  A zero return value
	   indicates that the current coverage window does not include the
	   provided location.

	   The coverage is inclusive on the south and west, exclusive on the
	   north and east.  This is standard for all grid interpolation files
	   except the Canadian National Transformation (version 1 and 2).  The
	   north and east edges of the typical grid file are not part of the
	   coverage of such a file.
	   
	   This rather simple algorithm needs help in the case where the given
	   point is very very close to the north or eastern edges.  In the grid
	   cell calculator, the calculations which compute record number and
	   element number include a noise factor neccessary as floating point
	   results such as 0.999999999999999 need to be considered by the
	   (long32_t) type cast to produce an integer value of 1L.  Thus, to
	   prevent an attempt to convert a point on the northern or eastern
	   edges of the grid file, we must also take this noise factor into
	   effect.
	   
	   Could it be that we should reduce the northEast extent values by this
	   much on construction of the grid file object???? */
	double returnValue = 0.0;
	if (point [LNG] >= thisPtr->southWest [LNG] &&
		point [LAT] >= thisPtr->southWest [LAT] &&
		(point [LNG] - cs_LlNoise) <  thisPtr->northEast [LNG] &&
		(point [LAT] - cs_LlNoise) <  thisPtr->northEast [LAT])
	{
		returnValue = thisPtr->density;
	}
	return returnValue;
}
void CSsetCoverage (struct csGridCoverage_* thisPtr,Const double* swLL,Const double* neLL)
{
	extern double cs_Zero;

	thisPtr->southWest [LNG] = swLL [LNG];
	thisPtr->southWest [LAT] = swLL [LAT];
	thisPtr->northEast [LNG] = neLL [LNG];
	thisPtr->northEast [LAT] = neLL [LAT];
	thisPtr->density = cs_Zero;
}

/****************************************************************************
	Grid Cell Implementation

	Again, this is an implementation of a grid cell for typical grid
	interpolation 
*/
void CSinitGridCell (struct csGridCell_* thisPtr)
{
	extern double cs_Zero;

	CSinitCoverage (&thisPtr->coverage);
	thisPtr->deltaLng = cs_Zero;
	thisPtr->deltaLat = cs_Zero;
	thisPtr->currentAA = cs_Zero;
	thisPtr->currentBB = cs_Zero;
	thisPtr->currentCC = cs_Zero;
	thisPtr->currentDD = cs_Zero;
	thisPtr->sourceId [0] = '\0';
}

double CScalcGridCellUS (struct csGridCell_* __This,Const double *sourceLL)
{
	/* This function performs the interpolation of the grid cell, and
	   returns the result.  Type of result depends upon the application.
	   Calculation algorithm is the same for all applications. */

	double gridDeltaLng;
	double gridDeltaLat;
	double returnValue;

	gridDeltaLng  = (sourceLL [LNG] - __This->coverage.southWest [LNG]) / __This->deltaLng;
	gridDeltaLat  = (sourceLL [LAT] - __This->coverage.southWest [LAT]) / __This->deltaLat;
	returnValue = __This->currentAA + 
				  __This->currentBB * gridDeltaLng +
				  __This->currentCC * gridDeltaLat +
				  __This->currentDD * gridDeltaLng * gridDeltaLat;

	/* Note return value for datum shift grid files is usually in seconds.
	   However, this varies from application to application. */
	return returnValue;	
}

double CScalcGridCellCA (struct csGridCell_* __This,Const double *sourceLL)
{
	double myLng, myLat;
	double gridDeltaLng;
	double gridDeltaLat;
	double returnValue;

	/* This function performs the interpolation of the grid cell, and
	   returns the result.  Type of result depends upon the application.
	   Calculation algorithm is the same for all applications. */

	myLng = -sourceLL [LNG];
	myLat =  sourceLL [LAT];
	gridDeltaLng  = (myLng - __This->coverage.southWest [LNG]) / __This->deltaLng;
	gridDeltaLat  = (myLat - __This->coverage.southWest [LAT]) / __This->deltaLat;
	returnValue = __This->currentAA + 
				  __This->currentBB * gridDeltaLng +
				  __This->currentCC * gridDeltaLat +
				  __This->currentDD * gridDeltaLng * gridDeltaLat;

	/* Note return value for datum shift grid files is usually in seconds.
	   However, this varies from application to application. */
	return returnValue;	
}

const char *CSsourceGridCell (struct csGridCell_* __This)
{
	return __This->sourceId;
}
/******************************************************************************
	Returns NULL if an error, otherwise returns an initialized cs_DtcXform
	structure.
*/
struct cs_DtcXform_ *CSnewFallback (Const char *dtKeyName,Const char *catalog)
{
	extern int cs_Error;
	extern char csErrnam [];
	extern double cs_Zero;
	extern struct cs_Datum_ cs_Wgs84Dt;

	char *cp;
	struct cs_DtcXform_ *__This;
	struct cs_Datum_ *dt_ptr;
	struct cs_Datum_ *wgs84Ptr;

	/* Prepare for an error of some sort. */
	__This = NULL;
	dt_ptr = NULL;

	/* Just in case, we do the simple case.  This is better than crashing. */
	if (dtKeyName == NULL || *dtKeyName == '\0')
	{
		CS_stncp (csErrnam,catalog,MAXPATH);
		CS_erpt (cs_FLBK_NOSET);
		return NULL;
	}

	/* Get a copy of the WGS84 datum definition.  We use that internally. */
	if (cs_Wgs84Dt.key_nm [0] == '\0')
	{
        wgs84Ptr = CS_dtloc ("WGS84");
		if (wgs84Ptr == NULL)
		{
			return NULL;
		}
		memcpy (&cs_Wgs84Dt,wgs84Ptr,sizeof (cs_Wgs84Dt));
		CS_free (wgs84Ptr);
		cs_Wgs84Dt.rot_X = cs_Zero;
		cs_Wgs84Dt.rot_Y = cs_Zero;
		cs_Wgs84Dt.rot_Z = cs_Zero;
		cs_Wgs84Dt.bwscale = cs_Zero;
	}

	/* Get the definition of the proposed fallabck. */
	dt_ptr = CS_dtloc (dtKeyName);
	if (dt_ptr == NULL)
	{
		if (cs_Error == cs_DT_NOT_FND)
		{
			CS_stncp (csErrnam,catalog,MAXPATH);
			CS_erpt (cs_FLBK_NTFND);
		}
		goto error;
	}

	/* Allocate the fallback object. */
	__This = CS_malc (sizeof (struct cs_DtcXform_));
	if (__This == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	__This->sourceId [0] = '\0';

	/* Properly initialize it. */
	cp = CS_stcpy (__This->sourceId,"Fallback: ");
	CS_stncp (cp,dt_ptr->key_nm,cs_KEYNM_DEF);
	switch (dt_ptr->to84_via) {
	case cs_DTCTYP_MOLO:
		__This->xfrmType = dtcTypMolodensky;
		__This->parms.molo = CS_moInit (dt_ptr,&cs_Wgs84Dt);
		if (__This->parms.molo == NULL) goto error;
		break;

	case cs_DTCTYP_MREG:
		__This->xfrmType = dtcTypDMAMulReg;
		__This->parms.mreg = CS_dmaMrInit (dt_ptr);
		if (__This->parms.mreg == NULL) goto error;
		break;

	case cs_DTCTYP_BURS:
		__This->xfrmType = dtcTypBursaWolf;
		__This->parms.bursa = CS_bwInit (dt_ptr,&cs_Wgs84Dt);
		if (__This->parms.bursa == NULL) goto error;
		break;

	case cs_DTCTYP_7PARM:
		__This->xfrmType = dtcTypSevenParm;
		__This->parms.parm7 = CS_7pInit (dt_ptr,&cs_Wgs84Dt);
		if (__This->parms.parm7 == NULL) goto error;
		break;

	case cs_DTCTYP_6PARM:
		__This->xfrmType = dtcTypSixParm;
		__This->parms.parm6 = CS_6pInit (dt_ptr,&cs_Wgs84Dt);
		if (__This->parms.parm6 == NULL) goto error;
		break;

	case cs_DTCTYP_GEOCTR:
		__This->xfrmType = dtcTypGeoCtr;
		__This->parms.geoctr = CS_gcInit (dt_ptr,&cs_Wgs84Dt);
		if (__This->parms.geoctr == NULL) goto error;
		break;

	case cs_DTCTYP_3PARM:
		__This->xfrmType = dtcTypThreeParm;
		__This->parms.parm3 = CS_3pInit (dt_ptr,&cs_Wgs84Dt);
		if (__This->parms.parm3 == NULL) goto error;
		break;

	case cs_DTCTYP_4PARM:
		__This->xfrmType = dtcTypFourParm;
		__This->parms.parm4 = CS_4pInit (dt_ptr,&cs_Wgs84Dt);
		if (__This->parms.parm4 == NULL) goto error;
		break;

	default:
		CS_stncp (csErrnam,catalog,MAXPATH);
		CS_erpt (cs_FLBK_WRNGT);
		goto error;
	}
	CS_free (dt_ptr);
	return __This;

error:
	if (dt_ptr != NULL) CS_free (dt_ptr);
	if (__This != NULL) CS_free (__This);
	return NULL;
}
/******************************************************************************
	Deletes the internal transformation, then deletes the fallback guy
	itself.
*/
void CSdeleteFallback (struct cs_DtcXform_ *__This)
{
	switch (__This->xfrmType) {
	case dtcTypMolodensky:
	case dtcTypMolodenskyInv:
		CS_free (__This->parms.molo);
        __This->parms.molo = NULL;
		break;
	case dtcTypDMAMulReg:
	case dtcTypDMAMulRegInv:
        {
			switch (__This->parms.mreg->fallback) 
            {
			case dtcTypMolodensky:
				CS_free(__This->parms.mreg->fallbackXfrm.molo);
                __This->parms.mreg->fallbackXfrm.molo = NULL;
				break;
			case dtcTypSixParm:
				CS_free(__This->parms.mreg->fallbackXfrm.parm6);
                __This->parms.mreg->fallbackXfrm.parm6 = NULL;
				break;
			case dtcTypSevenParm:
				CS_free(__This->parms.mreg->fallbackXfrm.parm7);
                __This->parms.mreg->fallbackXfrm.parm7 = NULL;
				break;
			default:
				break;
            }
        }
		CS_free (__This->parms.mreg);
        __This->parms.mreg = NULL;
		break;

	case dtcTypBursaWolf:
	case dtcTypBursaWolfInv:
		CS_free (__This->parms.bursa);
        __This->parms.bursa = NULL;
		break;

	case dtcTypSevenParm:
	case dtcTypSevenParmInv:
		CS_free (__This->parms.parm7);
        __This->parms.parm7 = NULL;
		break;

	case dtcTypThreeParm:
	case dtcTypThreeParmInv:
		CS_free (__This->parms.parm3);
        __This->parms.parm3 = NULL;
		break;

	case dtcTypSixParm:
	case dtcTypSixParmInv:
		CS_free (__This->parms.parm6);
        __This->parms.parm6 = NULL;
		break;

	case dtcTypFourParm:
	case dtcTypFourParmInv:
		CS_free (__This->parms.parm4);
        __This->parms.parm4 = NULL;
		break;
	default:
		/* Should never get here.  What would we do if we did? */
		break;
	} /*lint !e788 */  /* not all enumeration values appear in switch */

	CS_free (__This);
	return;
}
/* Used for datum calculation logging. */
Const char *CSsourceFallback (struct cs_DtcXform_ *__This)
{
	return __This->sourceId;
}
/* Forward calculation. */
int CScalcFallbackForward (struct cs_DtcXform_ *__This,double trg [3],Const double src [3])
{
	extern char csErrnam [];

	int status;

	if (__This == NULL)
	{
		CS_erpt (cs_FLBK_NOSET);
		return -1;
	}

	switch (__This->xfrmType) {
	case dtcTypMolodensky:
		status = CS_mo2dFowrd (trg,src,__This->parms.molo);
		break;

	case dtcTypDMAMulReg:
		status = CS_dmaMr2dFowrd (trg,src,__This->parms.mreg);
		break;

	case dtcTypBursaWolf:
		status = CS_bw2dFowrd (trg,src,__This->parms.bursa);
		break;

	case dtcTypSevenParm:
		status = CS_7p2dFowrd (trg,src,__This->parms.parm7);
		break;

	case dtcTypThreeParm:
		status = CS_3p2dFowrd (trg,src,__This->parms.parm3);
		break;

	case dtcTypSixParm:
		status = CS_6p2dFowrd (trg,src,__This->parms.parm6);
		break;

	case dtcTypFourParm:
		status = CS_4p2dFowrd (trg,src,__This->parms.parm4);
		break;

	default:
		CS_stncp (csErrnam,"CScalcFallbackForward:1",MAXPATH);
		CS_erpt (cs_ISER);
		goto error;
	}  /*lint !e788 */  /* not all enumeration values appear in switch */
	return (status == 0) ? 2 : 1;

error:
	return -1;
}

int CScalcFallbackInverse (struct cs_DtcXform_ *__This,double trg [3],Const double src [3])
{
	extern char csErrnam [];

	int status;

	if (__This == NULL)
	{
		CS_erpt (cs_FLBK_NOSET);
		return -1;
	}

	switch (__This->xfrmType) {

	case dtcTypMolodensky:
		status = CS_mo2dInvrs (trg,src,__This->parms.molo);
		break;

	case dtcTypDMAMulReg:
		status = CS_dmaMr2dInvrs (trg,src,__This->parms.mreg);
		break;

	case dtcTypBursaWolf:
		status = CS_bw2dInvrs (trg,src,__This->parms.bursa);
		break;

	case dtcTypSevenParm:
		status = CS_7p2dInvrs (trg,src,__This->parms.parm7);
		break;

	case dtcTypThreeParm:
		status = CS_3p2dInvrs (trg,src,__This->parms.parm3);
		break;

	case dtcTypSixParm:
		status = CS_6p2dInvrs (trg,src,__This->parms.parm6);
		break;

	case dtcTypFourParm:
		status = CS_4p2dInvrs (trg,src,__This->parms.parm4);
		break;

	default:
		CS_stncp (csErrnam,"CScalcFallbackInverse:1",MAXPATH);
		CS_erpt (cs_ISER);
		goto error;
	} /*lint !e788 */  /* not all enumeration values appear in switch */

	return (status == 0) ? 2 : 1;

error:
	return -1;
}

#ifdef __TEST__
/******************************************************************************
	A quick and dirty set of test code used to build a test module for
	debugging the new datum stuff.  Paths to catalog files are hard
	coded in this test program; you'll need to change them before you
	can use it.
*/
int main (int argc,char *argv [])
{
	struct csNad27ToNad83_ *nadObj;
	struct csNad83ToHarn_ *harnObj;
	struct csGeoidHeight_ *ghgtObj;
	struct csVertconUS_ *vconObj;

	double ll27 [2];
	double ll83 [2];
	double llHarn [2];
	double geoidHgt;
	double v88Mv29;

	char ctemp [256];

	ll27 [0] = -115.123456789;
	ll27 [1] = 35.123456789;

	nadObj = CSnewNad27ToNad83 ("H:\\CS_MAP\\DATA\\TEST\\Nad27ToNad83.gdc");
	if (nadObj == NULL)
	{
		CS_errmsg (ctemp,sizeof (ctemp));
		printf ("%s\n",ctemp);
		goto error;
	}
	harnObj = CSnewNad83ToHarn ("H:\\CS_MAP\\DATA\\TEST\\Nad83ToHarn.gdc");
	if (harnObj == NULL)
	{
		CS_errmsg (ctemp,sizeof (ctemp));
		printf ("%s\n",ctemp);
		goto error;
	}
	ghgtObj = CSnewGeoidHeight ("H:\\CS_MAP\\DATA\\TEST\\GeoidHeight.gdc");
	if (harnObj == NULL)
	{
		CS_errmsg (ctemp,sizeof (ctemp));
		printf ("%s\n",ctemp);
		goto error;
	}
	vconObj = CSnewVertconUS ("H:\\CS_MAP\\DATA\\TEST\\Vertcon.gdc");
	if (harnObj == NULL)
	{
		CS_errmsg (ctemp,sizeof (ctemp));
		printf ("%s\n",ctemp);
		goto error;
	}

	CScalcNad27ToNad83 (nadObj,ll83,ll27);
	CScalcNad83ToHarn (harnObj,llHarn,ll83);
	CScalcGeoidHeight (ghgtObj,&geoidHgt,ll83);
	CScalcVertconUS (vconObj,&v88Mv29,ll83);

	CSdeleteNad27ToNad83 (nadObj);
	CSdeleteNad83ToHarn (harnObj);
	CSdeleteGeoidHeight (ghgtObj);
	CSdeleteVertconUS (vconObj);
	return 0;

error:
	return 1;
}
#endif
