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
#include "cs_Legacy.h"

int EXP_LVL9 CSplynmQ (struct cs_GeodeticTransform_ *gxDef,unsigned short xfrmCode,
														   int err_list [],
														   int list_sz)
{
	extern double cs_K90;
	extern double cs_K180;
	extern double cs_Ten;
	extern double cs_AnglTest;		/* a very small, but not zero number. */

	int err_cnt;

	/* We will return (err_cnt + 1) below. */
	err_cnt = -1;
	if (err_list == NULL) list_sz = 0;

	/* Check the definition stuff specific to csPlynm_ */
	if (gxDef->parameters.polynomialParameters.degree <= 0 ||
		gxDef->parameters.polynomialParameters.degree > cs_PLYNM_MAXDEG)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_GXQ_PLY_DEG;
	}
	/* This is a geodetic transformation!!! */
	if (fabs (gxDef->parameters.polynomialParameters.srcEvalX) > cs_K180 ||
		fabs (gxDef->parameters.polynomialParameters.srcEvalY) > cs_K90)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_GXQ_PLY_SRC;
	}
	if (fabs (gxDef->parameters.polynomialParameters.trgEvalX) > cs_K180 ||
		fabs (gxDef->parameters.polynomialParameters.trgEvalY) > cs_K90)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_GXQ_PLY_TRG;
	}
	if (gxDef->parameters.polynomialParameters.srcNrmlScale < cs_AnglTest ||
		gxDef->parameters.polynomialParameters.trgNrmlScale < cs_AnglTest)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_GXQ_PLY_SCL;
	}

	/* That's it for General Polynomial transformations. */
	return (err_cnt + 1);
}
/*******************************************************************************/
int EXP_LVL9 CSplynmS (struct cs_GxXform_* gxXfrm)
{
	extern char csErrnam [];

	int idx;
	unsigned long bitMapBit;
	unsigned long xBitMap;
	unsigned long yBitMap;
	struct csPlynm_ *plynm;

	plynm = &gxXfrm->xforms.plynm;

	plynm->degree             = gxXfrm->gxDef.parameters.polynomialParameters.degree;
	if (plynm->degree <= 0 || plynm->degree > cs_PLYNM_MAXDEG)
	{
		CS_stncp (csErrnam,gxXfrm->xfrmName,MAXPATH);
		CS_erpt (cs_PLYNM_DEG);
		goto error;
	}
	plynm->srcEvalPnt [LNG] = gxXfrm->gxDef.parameters.polynomialParameters.srcEvalX;
	plynm->srcEvalPnt [LAT] = gxXfrm->gxDef.parameters.polynomialParameters.srcEvalY;
	plynm->trgEvalPnt [LNG] = gxXfrm->gxDef.parameters.polynomialParameters.trgEvalX;
	plynm->trgEvalPnt [LAT] = gxXfrm->gxDef.parameters.polynomialParameters.trgEvalY;
	plynm->deltaEval  [LNG] = plynm->trgEvalPnt [LNG] - plynm->srcEvalPnt [LNG];
	plynm->deltaEval  [LAT] = plynm->trgEvalPnt [LAT] - plynm->srcEvalPnt [LAT];
	plynm->srcNrmlScl       = gxXfrm->gxDef.parameters.polynomialParameters.srcNrmlScale;
	plynm->trgNrmlScl       = gxXfrm->gxDef.parameters.polynomialParameters.trgNrmlScale;
	plynm->validation       = gxXfrm->gxDef.parameters.polynomialParameters.validation;
	/* The following three values are only used in the iterative inverse calculation. */
	plynm->cnvrgValue       = gxXfrm->cnvrgValue;
	plynm->errorValue       = gxXfrm->errorValue;
	plynm->maxIterations    = gxXfrm->maxIterations;

	/* Copy the coefficients. */
	bitMapBit = 1UL;
	xBitMap = yBitMap = 0UL;
	plynm->termCount = (plynm->degree + 1) * (plynm->degree + 2) / 2;
	for (idx = 0;idx < plynm->termCount;idx += 1)
	{
		plynm->xCoeff [idx] = gxXfrm->gxDef.parameters.polynomialParameters.xCoeffs [idx];
		if (plynm->xCoeff [idx] != 0.0) xBitMap |= bitMapBit;

		plynm->yCoeff [idx] = gxXfrm->gxDef.parameters.polynomialParameters.yCoeffs [idx];
		if (plynm->yCoeff [idx] != 0.0) yBitMap |= bitMapBit;

		bitMapBit <<= 1;
	}

	/* Save the bit maps for the calculation function. */
	plynm->xCoeffBitMap = xBitMap;
	plynm->yCoeffBitMap = yBitMap;

	/* Force any remaining slots in the coefficient arrays to zero. */
	while (idx < cs_PLYNM_MAXCOEF)
	{
		plynm->xCoeff [idx] = gxXfrm->gxDef.parameters.polynomialParameters.xCoeffs [idx];
		plynm->yCoeff [idx] = gxXfrm->gxDef.parameters.polynomialParameters.yCoeffs [idx];
		idx += 1;
	}

	/* TODO -- Do we need to set up a fallback here??? */

	/* Set up the "virtual" functions. */
	gxXfrm->frwrd2D = (cs_FRWRD2D_CAST)CSplynmF2;
	gxXfrm->frwrd3D = (cs_FRWRD3D_CAST)CSplynmF3;
	gxXfrm->invrs2D = (cs_INVRS2D_CAST)CSplynmI2;
	gxXfrm->invrs3D = (cs_INVRS3D_CAST)CSplynmI3;
	gxXfrm->inRange = (cs_INRANGE_CAST)CSplynmL;
	gxXfrm->release = (cs_RELEASE_CAST)CSplynmR;
	gxXfrm->destroy = (cs_DESTROY_CAST)CSplynmD;

	return 0;

error:
	/* Specific error indications should already have been reported
	   via CS_erpt (). */
	return -1;
}
/* Three dimensional forward transformation. */
int EXP_LVL9 CSplynmF3 (struct csPlynm_ *plynm,double* trgLl,Const double* srcLl)
{
	int status;

	/* This is a two dimensional transformation.  It may be the horizonatl
	   component of a path wherein the vertical is done by another
	   transformation.  Thus, we just copy the vertical component and
	   use the two dimensional transformation on the horizontal
	   coordinates. */
	if (trgLl != srcLl)
	{
		trgLl [HGT] = srcLl [HGT];
	}

	status = CSplynmF2 (plynm,trgLl,srcLl);

	return (status);
}
/* Two dimensional forward transformation.
   Again, note that geodisists and surveyors use the [latitude,longitude]
   convention whilc GIS foils (like this developer) are accustomed to the
   [longitude,latitude].

   In this code, X implies longitude, and Y implies latitude.  This is just
   the opposite of what much documentation uses.

   However, to keep our sanity, in this module the uu variable will indeed
   be used in the latitude calculations, and the vv variable will be used
   in the longitude calculations.  That is, variable identified by "uu"
   will be normalized latitude coordinates, and "vv" implies longitude
   normalized coordinates.

   Any identifiers with an X refer to longitude, and any identifiers with
   a Y reer to latitude.
*/
int EXP_LVL9 CSplynmF2 (struct csPlynm_ *plynm,double* trgLl,Const double* srcLl)
{
	extern double cs_Zero;

	int status;
	int idx;

	unsigned long xBitMap;
	unsigned long yBitMap;

	double uu, uuSum;		/* latitude related quantities */
	double vv, vvSum;		/* longitude related quantities */

	double pwrArray [cs_PLYNM_MAXCOEF];

	/* If we do not accomplish anything else, we set target equal to source. */
	status = 0;					/* until we know different */
	if (trgLl != srcLl)
	{
		trgLl [LNG] = srcLl [LNG];
		trgLl [LAT] = srcLl [LAT];
	}

	/* Normalize the input coordinates. */
	vv = (srcLl [LNG] - plynm->srcEvalPnt [LNG]) * plynm->srcNrmlScl;
	uu = (srcLl [LAT] - plynm->srcEvalPnt [LAT]) * plynm->srcNrmlScl;

	/* Test uu and vv here.  Typically, the values should be less than
	   unity.  However, EPSG suggests that they can be as large as 10.
	   We have a 'validation' parameter suct that the value is specified
	   in the dictionary definition. */
	if (fabs (vv) > plynm->validation || fabs (uu) > plynm->validation)
	{
		status = 1;
	}

	if (status == 0)
	{
		/* Populate the power array.  This is essentially the dynamic terms
		   of the polynominal.  These are the powers of uu and vv intermixed
		   and returned in an order which matches the coefficients. 

		   In order to get the coefficients as documented to match the
		   coordinate power sequence, the order of the uu and vv arguments
		   to CSplynmPwrArray is crucial.
		*/
		CSplynmPwrArray (pwrArray,plynm->degree,uu,vv);

		/* Perform the polynomial calculation. */
		uuSum = vvSum = cs_Zero;
		xBitMap = plynm->xCoeffBitMap;
		yBitMap = plynm->yCoeffBitMap;
		for (idx = 0;idx < plynm->termCount;++idx)
		{
			if ((xBitMap & 0x01) != 0)		// Skip if coefficient is zero.
			{
				vvSum += pwrArray [idx] * plynm->xCoeff [idx];
			}
			if ((yBitMap & 0x01) != 0)		// Skip if coefficient is zero.
			{
				uuSum += pwrArray [idx] * plynm->yCoeff [idx];
			}
			xBitMap >>= 1;
			yBitMap >>= 1;

			// If both bit maps are zero, we're all done. */
			if ((xBitMap | yBitMap) == 0UL)
			{
				break;
			}
		}

		/* OK, un-normalize the result. */
		trgLl [LNG] = (srcLl [LNG] + plynm->deltaEval [LNG]) + (vvSum / plynm->trgNrmlScl);
		trgLl [LAT] = (srcLl [LAT] + plynm->deltaEval [LAT]) + (uuSum / plynm->trgNrmlScl);
	}
	else
	{
		/* To the fallback.  If successful, set trgLl and set status to +2. */
	}
	return status;
}
/* Three dimensional inverse transformation. */
int EXP_LVL9 CSplynmI3 (struct csPlynm_ *plynm,double* trgLl,Const double* srcLl)
{
	int status;

	/* This is a two dimensional transformation.  It may be the horizonatl
	   component of a path wherein the vertical is done by another
	   transformation.  Thus, we just copy the vertical component and
	   use the two dimensional transformation on the horizontal
	   coordinates. */
	if (trgLl != srcLl)
	{
		trgLl [HGT] = srcLl [HGT];
	}
	status = CSplynmI2 (plynm,trgLl,srcLl);
	return (status);
}
/* Two dimensional inverse transformation. */
int EXP_LVL9 CSplynmI2 (struct csPlynm_ *plynm,double* trgLl,Const double* srcLl)
{
	int status;
	int itr;
	short lngOk;
	short latOk;
	double guess [2];
	double newLl [2];
	double epsilon [2];

	/* If nothing else, we return the source coordinates as the result. */
	status = 0;
	if (trgLl != srcLl)
	{
		trgLl [LNG] = srcLl [LNG];
		trgLl [LAT] = srcLl [LAT];
	}

	/* Establish our best guess as the coordinates provided.  In this scheme
	   of things (i.e. a geodetic transformation) this is a good guess as
	   datum shifts tend to be rather small. */
	guess [LNG] = srcLl [LNG];
	guess [LAT] = srcLl [LAT];

	/* Start a loop which will iterate as many as maxIteration times. */
	epsilon [LNG] = epsilon [LAT] = plynm->cnvrgValue;		/* keep gcc compiler happy */
	for (itr = 0;itr < plynm->maxIterations;itr++)
	{
		/* Until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the shift at out current guess point. */
		status = CSplynmF2 (plynm,newLl,guess);
		if (status != 0)
		{
			/* We have started, or wandered, out of range for this
			   transformation.  Thatis, there are no hard core
			   boundaries or system errors associated with this
			   transformation. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = CS_lngEpsilon (srcLl [LNG],newLl [LNG]);
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   cnvrgValue, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > plynm->cnvrgValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > plynm->cnvrgValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   epsilon of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* Did it fail to converge?  Note, that if something happened above
	   to cause status to be non-zero, the itr counter would be less
	   than maxIterations.  So status should still be zero. */
	if (itr >= plynm->maxIterations)
	{
		/* It didn't converge.  If the either epsilon value is greater
		   than of equal to the 'errorValue' we return an error condition.
		   otherwise, we accept what we've got as being close enough. */
		if ( (fabs (epsilon [LNG]) > plynm->errorValue) || 
			 (fabs (epsilon [LNG]) > plynm->errorValue)
		   )
		{
			CS_erpt (cs_PLYNM_CNVRG);
			status = 1;
		}
	}
	if (status != 0)
	{
		/* We're out of range. */
		CS_erpt (cs_PLYNM_RNG);
		status = 1;
	}
	else
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return status;
}
int EXP_LVL9 CSplynmL (struct csPlynm_ *plynm,int cnt,Const double pnts [][3])
{
	int status;
	int index;

	double uu;			/* Normalized latitude */
	double vv;			/* Normalized longitude. */

	status = cs_CNVRT_OK;
	for (index = 0;index < cnt;index += 1)
	{
		/* Compute the normalized input coordinates, uu and vv. */
		vv = (pnts [index][LAT] + plynm->srcEvalPnt [LNG]) * plynm->srcNrmlScl;
		uu = (pnts [index][LNG] + plynm->srcEvalPnt [LAT]) * plynm->srcNrmlScl;

		if (fabs (vv) > plynm->validation || fabs (uu) > plynm->validation)
		{
			status = cs_CNVRT_USFL;
			break;
		}
	}
	return status;
}
int EXP_LVL9 CSplynmN  (struct csPlynm_ *plynm)
{
	return FALSE;
}
int EXP_LVL9 CSplynmR (struct csPlynm_ *plynm)
{
	return 0;
}
int EXP_LVL9 CSplynmD (struct csPlynm_ *plynm)
{
	return 0;
}
void CSplynmPwrArray (double pwrArray [],short degree,double xx,double yy)
{
	extern double cs_Zero;
	extern double cs_One;

	int ii;
	int jj;
	int kk;
	int idx;

	double xAry [cs_PLYNM_MAXDEG + 1];
	double yAry [cs_PLYNM_MAXDEG + 1];

	/* Compute the powers. */
	xAry [0] = yAry [0] = cs_One;
	xAry [1] = xx;
	yAry [1] = yy;
	for (idx = 2;idx < degree;++idx)
	{
		xAry [idx] = xAry [idx - 1] * xx;
		yAry [idx] = yAry [idx - 1] * yy;
	}
	while (idx <= cs_PLYNM_MAXDEG)		/* Defensive */
	{
		xAry [idx] = cs_Zero;
		yAry [idx] = cs_Zero;
		++idx;
	}
	
	/* Build the term array.
	
	   Defore optimizining, the following loop looks like this:
	
		int idx = 0;
		for (int ii = 0;ii <= degree;++ii)
		{
			int kk = ii;
			for (int jj = 0;jj < degree;++jj,--kk)
			{
				pwrArray [idx++] = xAry [kk] * yAry [jj;
			}
		}

	   Below we optimize out multiplications by 1.0   Cannot
	   rely on the optimizer to do this, as it in not arware
	   of how the fact that X^0 == 1.0, and X^1 == X.
	*/

	idx = 0;
	pwrArray [idx++] = cs_One;		/* X^0 * Y^0   -- always one. */
	for (ii = 1;ii <= degree;++ii)
	{
		pwrArray [idx++] = xAry [ii];		/* X^ii * Y^0  -- always X^ii */
		kk = ii - 1;
		for (jj = 1;jj < ii;++jj,--kk)
		{
			pwrArray [idx++] = xAry [kk] * yAry [jj];
		}
		pwrArray [idx++] = yAry [ii];		/* X^0 * Y^jj  -- always Y^jj */
	}
	return;
}

