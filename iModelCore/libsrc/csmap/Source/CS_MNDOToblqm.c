/*-------------------------------------------------------------------------------------+
|
|     $Source: Source/CS_MNDOToblqm.c $
|    $RCSfile: CS_MNDOToblqm.c,v $
|   $Revision: 1.1 $
|       $Date: 2011/06/06 16:12:25 $
|     $Author: Alain.Robert $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "cs_map.h"

/*lint -esym(613,err_list) */
/*lint -esym(715,prj_code) */


/************************************************************************************************
**
** The present file contains the implementation of the MNDOT variation
** of the Oblique Mercator. This variation, alike the Transverse Mercator
** and Lambert conformal Conic MNDOT variation is meant to correct
** variations of distance measurements on maps caused by the fact the ellipsoid
** does not fit exactly with the ground surface. This correction is performed
** by application of a scale factor very close to one computed based on the
** mean elevation of the ground relative to the ellipsoid in a specific area.
**
** The Transverse Mercator and Lambert Conformal Conic MNDOT
** variations apply a scale factor applicable to the whole zone definition
** this scale factor being computed at some foundation point in the zone.
** The oblique Mercator variation instead computes a scale factor locally 
** at every transformed point. Since the scale factor computations is
** based on the latitude of the point, when converting from cartesian to 
** geographic latitude/longitude an arbitrary latitude is first used in the calculations
** resulting in a latitude which is an approximation then this latitude is used
** to reevaluate the local scale factor. The implementation is limited to
** 2 iterations and the variation between iteration is not computed nor used
** in assesing the accuracy of the transformation. This implementation
** which must comply with the results provided by the MNCon.exe Minnesota
** DOT software was coded exactly the same way.
**
** Although the code was provided, some ellipsoid and datum related numbers
** were hardcoded and required use of NAD83 or NAD27. Note that there does not
** appear to be any common use of the NAD27 datum from these zones. The limitations
** to a specific datum using pre-computed ellipsoid based parameters did not appear
** to very easily maintainable. Additional computations were not provided by the MnCON.exe
** code provided and that were required for the CSMAP interface. For this reason,
** we went back to the original definition of the MNDOT Oblique Mercator and
** copied then modified the HOM1XY 1 point Oblique Mercator to add the scale factor
** computations and application. The present code is thus very different from the 
** original MNDOT code but is far more general in its application. It will produce proper results
** whatever the ellipsoid definition is use and wherever it is applied on the Earth.
**
** Although the Minnesota DOT variations of the three projections were designed specifically
** for use in Minnesota, the Transverse Mercator and Lambert Conformal Conic variations
** can be used elsewhere as an efficient way to correct ground and grid based distance
** measurements. Although the Oblique Mercator variation is less general in its application
** as a result of the iterative process and the local computation of the scale factor at
** every transformed point.
**
** Although the present file shared mostly the same code as the CS_oblqm.c CSMAP file
** it has been placed in a specific file mainly because of the structural change
** related to the iterative process of the conversion to latitude/longitude. If this
** change had not been required we would certainly have integrated the modifications in the
** original CSMAP source code file.
**
** A version of the original MNDOT source code pretinent to the transformation has been pasted
** below in the file as a means of documenting the original process. The original code is in 
** Visual Basic but pertinent portions were easily converted to C.
**
****************************************************************************************************/

int EXP_LVL9 CSMNDOToblqmIWithScaleLatitudeAdjustments (Const struct cs_MNDOTOblqm_ *oblqm,double ll [2],Const double xy [2], double scale_latitude);

/**********************************************************************
**	err_cnt = CSMNDOToblqmQ (cs_def,prj_code,err_list,list_sz);
**
**	struct cs_Csdef_ *cs_def;	the coordinate system to be checked.
**	unsigned short prj_code;	indicates the specific variation for which the
**								parameters are to be checked.
**	int err_list [];			an array of integers in which error codes are
**								returned if not NULL.
**	int list_sz;				the size of the array pointed to be err_list,
**								may be zero.
**	int pnt_cnt;				the number of points used to define the great
**								circle:   point and azimuth := pnt_cnt = 1;
**								two points := mode = 2.
**	int rect_flg;				TRUE says the coordinate system is rectified to
**								cartesian X and Y, were Y is due north.
**	int err_cnt;				returns the number of errors detected.
**
**	Set err_list to NULL, and/or list_sz to zero, to simply get
**	a yea or nay status of the definition (i.e. err_cnt != 0).
**
**	All lat/longs in definitions must be referenced to Greennwich,
**	and in the range of greater than -180/-90, and less than or
**	equal to +180/+90.
**
**	The rectification flag has no effect on the validity of the
**	coordinate system as far as we know.  However, we have included
**	it in thew calling sequence in case we get smart some time in
**	the future.
**********************************************************************/

int EXP_LVL9 CSMNDOToblqmQ (	Const struct cs_Csdef_ *cs_def,unsigned short prj_code,int err_list [],int list_sz)
{
	extern double cs_Km180;			/* -180.0 */
	extern double cs_K360;			/*  360.0 */
	extern double cs_MinLng;		/* -180.0 */
	extern double cs_MaxLng;		/* +180.0 */
	extern double cs_MaxLngFz;		/* + 179.99 */
	extern double cs_MinLatFz;		/* - 89.99 */
	extern double cs_MaxLatFz;		/* + 89.99 */
	extern double cs_SclRedMin;		/* 0.75 */
	extern double cs_SclRedMax;		/* 1.10 */
	extern double cs_ParmTest;		/* .1 seconds of arc in degrees */

	int err_cnt;

	int one_pnt;

	/* Extract the essentials of the specific variation at hand. */

    one_pnt = TRUE;

	/* We will return (err_cnt + 1) below. */
	
	err_cnt = -1;
	if (err_list == NULL) list_sz = 0;

	/* We could check for the consistency of the variations. However, since
	   this is all hard coded above, such as error would be a software
	   error and not a user error. Therefore, we trust the programmer and
	   ignore such checks. */

	/* Check the Oblique Mercator specific stuff.  Prj_prm1 and prj_prm2
	   are always a lat long pair.  Can't allow a pole for the
	   latitude. */
	   
	if (cs_def->prj_prm1 <= cs_MinLng || cs_def->prj_prm1 > cs_MaxLng)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_LNG;
	}
	if (cs_def->prj_prm2 <= cs_MinLatFz || cs_def->prj_prm2 >= cs_MaxLatFz)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_LAT;
	}

	if (cs_def->scl_red < cs_SclRedMin || cs_def->scl_red > cs_SclRedMax)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_SCLRED;
	}

	/* The stuff specific to the two alternatives. */
	
	/* In this case, prj_prm3 is an azimuth.  We treat it like
	   a longitude.  However, values of 0.0, +-90.0, and +-180.0
	   are not good.
	   
	   Actually, values of +- 90 should be OK. However, we reject
	   them here and suggest that the user use Swiss Oblique Mercator. */

	if (fabs (cs_def->prj_prm3) < cs_ParmTest ||
			  cs_def->prj_prm3 >= cs_K360 ||
			  cs_def->prj_prm3 <= cs_Km180)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_AZM;
	}
	if (fabs (fabs (cs_def->prj_prm3) - cs_MaxLatFz) < cs_ParmTest ||
		fabs (fabs (cs_def->prj_prm3) - cs_MaxLngFz) < cs_ParmTest)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_USESW;
	}
	
	/* Also, the great circle defined must not experience maximum
	   or minimum latitude at the defined point.  This will be
	   tricky.  I don't know how to check that yet. */


	/* That's it for the MNDOT Oblique Mercator. */
	
	return (err_cnt + 1);
}

/**********************************************************************
**	CSMNDOToblqmS (csprm,mode,rect_flg);
**
**	struct cs_Csprm_ *csprm;	structure which defines the parameters
**								in effect for the transformation.
**	int mode;					specifies the number of points used to
**								define the projection.  1 means one
**								point and an azimuth, 2 means two points
**								and a central latiude.
**	int rect_flg;				TRUE indicates that rectified X and Y coordinates
**								are to be returned.  FALSE indicates that U and V
**								coordinates are to be returned.
**
**	The Oblique Mercator used is the Hotine version thereof.
**
**	There are several variations of this projection. The specific
**	variation desired is specified by the prj_code element of the
**	cs_Csprm_ structure which is passed as the only argument to
**	this function. There are six unique values of prj_code which
**	are understood by this module:
**
**	This projection is a variation of cs_PRJCOD_HOM1XY
**		Defined by a single point and an azimuth, the
**		projection produces rectified X and Y coordinates
**		with the origin at the intersection of the central
**		geodesic with the equator of the aposphere. This is
**		the variation which produces the correct results for
**		Zone 1 of the Alaska state plane system.
**********************************************************************/

void EXP_LVL9 CSMNDOToblqmS (struct cs_Csprm_ *csprm)
{
	extern short cs_QuadMin;			/* -4 */
	extern short cs_QuadMap [];
	extern double cs_Degree;			/* 1.0 / 57.29577... */
	extern double cs_Radian;			/* 57.29577...       */
	extern double cs_Pi;				/*  3.14159...       */
	extern double cs_Pi_o_2;			/* pi over two       */
	extern double cs_Zero;				/* 0.0               */
	extern double cs_Half;				/* 0.5               */
	extern double cs_One;				/* 1.0               */
	extern double cs_Two;				/* 2.0               */
	extern double cs_Ten;				/* 10.0 */
	extern double cs_AnglTest;			/* 0.001 seconds of arc
										   short of pi/2 in
										   radians. */
	extern double cs_AnglTest1;			/* 1.0 - cs_AnglTest */

	struct cs_MNDOTOblqm_ *oblqm;

	double t_0;
	double t_1;
	double t_2;
	double D, Dsq;
	double F;
	double G;
	double H;
	double J;
	double L;
	double P;
	double cos_lat_0;
	double sin_lat_0;

	double tmp1;
	double tmp2;
	double tmp3;
	double tmp4;
	double tmp5;
	double tmp6;
	double zone_wd;

	oblqm = &csprm->proj_prms.oblMN;

	/* Extract the essentials of the specific variation at hand. */


	oblqm->one_pnt = TRUE;
	oblqm->uv_ctr = FALSE;
	oblqm->rect_flg = csOBLQM_ALASKA;
	oblqm->azmIsSkew = FALSE;

	

	
	/* Transfer the necessary arguments to the "oblqm"
	   structure. */

    /* In the one point mode. */

	oblqm->lng_c = csprm->csdef.prj_prm1 * cs_Degree;
	oblqm->lat_c = csprm->csdef.prj_prm2 * cs_Degree;
	oblqm->lng_0 = oblqm->lng_c;
	oblqm->lat_0 = oblqm->lat_c;
	/* Legacy dictates that parm3 is the azimuth of the cntral line
	   at the projection origin.  As of 11.03, parm3 could be
	   the "skew azimuth of the rectified grid" at the projection
	   origin. */
	oblqm->az = csprm->csdef.prj_prm3 * cs_Degree;
	oblqm->gamma_0 = csprm->csdef.prj_prm3 * cs_Degree;
	
    // The height is provided in the linear units of the coordinate system definition
    oblqm->height = csprm->csdef.prj_prm4 * csprm->csdef.unit_scl;

	oblqm->k0 = csprm->csdef.scl_red;
	oblqm->k = csprm->csdef.scale * csprm->csdef.scl_red;
	oblqm->x_off = csprm->csdef.x_off;
	oblqm->y_off = csprm->csdef.y_off;
	oblqm->uc = cs_Zero;					/* until we know different */
	oblqm->ecent = csprm->datum.ecent;
	oblqm->e_sq = oblqm->ecent * oblqm->ecent;
	oblqm->e_rad = csprm->datum.e_rad;
	oblqm->ak = oblqm->e_rad * oblqm->k;
	oblqm->e_ovr_2 = oblqm->ecent * cs_Half;
	oblqm->quad = cs_QuadMap [csprm->csdef.quad - cs_QuadMin];

	/* Much of the following is used only in the spherical mode,
	   but is useful stuff for general computations, such as
	   limit checking. */

	/* If we've been given one point and an azimuth.  Remember, much of
	   this is valid only for the spherical case. */

	oblqm->sin_az = sin (oblqm->az);
	oblqm->cos_az = cos (oblqm->az);

	tmp1 = sin (oblqm->lat_c);
	tmp2 = cos (oblqm->lat_c);
	oblqm->lat_p = asin (tmp2 * oblqm->sin_az);
	tmp3 = -tmp1 * oblqm->sin_az;
	tmp4 = atan2 (-oblqm->cos_az,tmp3) + oblqm->lng_c;
	oblqm->lng_p = CS_adj2pi (tmp4);
	

	/* Select the northern most pole. */

	if (oblqm->lat_p < 0.0)
	{
		oblqm->lat_p = -oblqm->lat_p;
		oblqm->lng_p += CS_adj2pi (oblqm->lng_p + cs_Pi);
	}

	/* Compute the sine and cosine of the pole latitude. */

	oblqm->sin_lat_p = sin (oblqm->lat_p);
	oblqm->cos_lat_p = cos (oblqm->lat_p);

	if (oblqm->ecent == 0.0)
	{
		/* lat_0,lng_0 is the point where the great circle
		   intersects the equator.  This is the origin of
		   the U V system of coordinates in the spherical
		   case. */

		tmp1 = oblqm->lng_p + cs_Pi_o_2;
		oblqm->lng_0 = CS_adj2pi (tmp1);
		oblqm->lat_0 = cs_Zero;

		/* Vv_max is what we use for the V coordinate of the
		   oblique pole.  Uu_max is what we use for the U
		   coordinate of the oblique pole. In the following
		   three lines, read cs_AnglTest1 as the cosine of
		   the angular distance from the pole to the point
		   in question (i.e. a point very close,but not
		   exactly at, the pole). */

		tmp1 = (cs_One + cs_AnglTest1) / (cs_One - cs_AnglTest1);
		oblqm->vv_max = cs_Half * oblqm->ak * log (tmp1);

		oblqm->uu_max = oblqm->ak * (cs_Pi - cs_AnglTest);

		/* That's it for the spherical case. */
	}
	else
	{
		/* Now for the real stuff.  Much of the calcuations
		   are the same for either 1 or 2 point mode.  Of
		   course, there are some differences.  The differences
		   are, however, limited to the setup function.  Note,
		   that lat_c and lat_0 are the same thing.  This
		   was taken care of above where the projection
		   parameters were extracted from the coordinate
		   system definition.
		   
		   First, we compute some values which are used more
		   than once. */

		cos_lat_0 = cos (oblqm->lat_0);
		sin_lat_0 = sin (oblqm->lat_0);
		oblqm->e_ovr_2 = oblqm->ecent * cs_Half;
		tmp1 = cos_lat_0 * cos_lat_0;
		tmp1 = tmp1 * tmp1 * oblqm->e_sq;
		tmp2 = cs_One - oblqm->e_sq;
		tmp3 = sqrt (tmp2);
		tmp4 = oblqm->ecent * sin_lat_0;
		tmp5 = cs_One - (sin_lat_0 * sin_lat_0 * oblqm->e_sq);

		/* Now for some of the real projection constants.
		   A, B, t_0, and D are the same for both modes. */

		oblqm->B = sqrt (cs_One + (tmp1 / tmp2));
		oblqm->one_ovr_B = cs_One / oblqm->B;
		oblqm->A = oblqm->ak * oblqm->B * tmp3 / tmp5;
		oblqm->A_ovr_B = oblqm->A * oblqm->one_ovr_B;

		tmp6 = (cs_One - tmp4) / (cs_One + tmp4);
		tmp6 = pow (tmp6,oblqm->e_ovr_2);
		t_0 = tan (PI_OVR_4 - (oblqm->lat_0 * cs_Half)) / tmp6;

		tmp6 = sqrt (cs_One - tmp4 * tmp4);
		D = oblqm->B * tmp3 / (cos_lat_0 * tmp6);
		Dsq = D * D;
		if (Dsq < cs_One) Dsq = cs_One;

		/* Now for some stuff which is different between
		   the two modes. */

		if (oblqm->one_pnt)
		{
			tmp6 = sqrt (Dsq - 1);
			if (oblqm->lat_0 < 0.0) tmp6 = -tmp6;
			F = D + tmp6;

			oblqm->E = F * pow (t_0,oblqm->B);
			G = (F - (cs_One / F)) * cs_Half;

			/* Now for the azimuth.  In one variation, we are given
			   the "skew azimuth of the rectified grid" which is
			   essentially gamma0.  In this case, we need to compute
			   az (which is often called "alpha C").  In the normal
			   case (normal for CS-MAP anyway) we are given "alpha C",
			   which is the azimuth of the central geodesic at the
			   projection center, and we need to compute gamma0. */
			if (oblqm->azmIsSkew)
			{
				oblqm->az = asin (D * sin (oblqm->gamma_0));
				oblqm->sin_az = sin (oblqm->az);
				oblqm->cos_az = cos (oblqm->az);
			}
			else
			{
				oblqm->gamma_0 = asin (oblqm->sin_az / D);
			}
			tmp6 = G * tan (oblqm->gamma_0);
			oblqm->lng_0 = oblqm->lng_c - (asin (tmp6) / oblqm->B);

			/* Compute the value of u at the projection center. We do
			   this only for the  */

			if (oblqm->uv_ctr)
			{
				if (fabs (oblqm->cos_az) > cs_AnglTest)
				{
					tmp2 = sqrt (Dsq - cs_One);
					oblqm->uc = oblqm->A_ovr_B * atan (tmp2 / oblqm->cos_az);
					if (oblqm->lat_0 < 0.0) oblqm->uc = -oblqm->uc;
				}
				else
				{
					/* Primarily for Switzerland/Hungary */
					
					oblqm->uc = oblqm->A * (oblqm->lng_c - oblqm->lng_0);
				}
			}
		}
		else
		{
			/* Here for the two point case.  These are significantly
			   different from the above. */

			tmp4 = oblqm->ecent * sin (oblqm->lat_1);
			tmp5 = (cs_One - tmp4) / (cs_One + tmp4);
			tmp6 = pow (tmp5,oblqm->e_ovr_2);
			t_1 = tan (PI_OVR_4 - (oblqm->lat_1 * cs_Half)) / tmp6;
			tmp4 = oblqm->ecent * sin (oblqm->lat_2);
			tmp5 = (cs_One - tmp4) / (cs_One + tmp4);
			tmp6 = pow (tmp5,oblqm->e_ovr_2);
			t_2 = tan (PI_OVR_4 - (oblqm->lat_2 * cs_Half)) / tmp6;

			tmp4 = sqrt (Dsq - cs_One);
			if (oblqm->lat_0 < 0.0) tmp4 = -tmp4;
			oblqm->E = (D + tmp4) * pow (t_0,oblqm->B);
			H = pow (t_1,oblqm->B);
			L = pow (t_2,oblqm->B);
			F = oblqm->E / H;
			G = (F - (cs_One / F)) * cs_Half;
			tmp4 = oblqm->E * oblqm->E;
			tmp5 = L * H;
			J = (tmp4 - tmp5) / (tmp4 + tmp5);
			P = (L - H) / (L + H);
			tmp4 = (oblqm->lng_1 + oblqm->lng_2) * cs_Half;
			tmp5 = oblqm->lng_1 - oblqm->lng_2;
			tmp5 = CS_adj2pi (tmp5);
			tmp5 *= oblqm->B * cs_Half;
			tmp5 = J * tan (tmp5);
			oblqm->lng_0 = tmp4 - atan2 (tmp5,P) * oblqm->one_ovr_B;
			oblqm->lng_0 = CS_adj2pi (oblqm->lng_0);
			tmp4 = oblqm->lng_1 - oblqm->lng_0;
			tmp4 = CS_adj2pi (tmp4);
			tmp4 = sin (oblqm->B * tmp4) / G;
			oblqm->gamma_0 = atan (tmp4);
			oblqm->az = asin (D * sin (oblqm->gamma_0));
			oblqm->sin_az = sin (oblqm->az);
			oblqm->cos_az = cos (oblqm->az);
		}

		/* For the inverse, we need the following, which are
		   the same for all four variations. */

		oblqm->sin_gam_0 = sin (oblqm->gamma_0);
		oblqm->cos_gam_0 = cos (oblqm->gamma_0);

		/* Vv_max is what we use for the V coordinate of the
		   oblique pole.  Uu_max is what we use for the U
		   coordinate of the oblique pole.

		   In the following three lines, read cs_AnglTest1 as
		   the cosine of the angular distance from the pole
		   to the point in question (i.e. a point very close
		   to the pole). */

		tmp1 = (cs_One + cs_AnglTest1) / (cs_One - cs_AnglTest1);
		oblqm->vv_max = cs_Half * oblqm->A_ovr_B * log (tmp1);

		oblqm->uu_max = oblqm->A_ovr_B * (cs_Pi - cs_AnglTest);

		/* Calculate the isometric latitude series coefficients
		   for this ellipsoid. */

		CSchiIsu (&oblqm->chicofI,oblqm->e_sq);
	}

	/* Set up the coordinate checking information.  If the user has
	   specified a useful range, we use it without checking it.
	   Otherwise, we compute what I, the programmer, consider to
	   be the useful range of the projection.  Note, values are in
	   degrees and longitude values are relative to the central
	   meridian, which is where the central great circle crosses
	   the equator. */

	csprm->cent_mer = oblqm->lng_c * cs_Radian;
	if (csprm->csdef.ll_min [LNG] == 0.0 &&
		csprm->csdef.ll_max [LNG] == 0.0)
	{
		/* We're to calculate the useful range.  Since the
		   range is very orthogonal (i.e. a min/max) and the
		   projection is oblique, this is very subjective. */

		zone_wd = (cs_Two * acos (oblqm->k0) * cs_Radian) + 0.25;
		tmp1 = cs_Ten * fabs (oblqm->sin_az) + zone_wd * fabs (oblqm->cos_az);
		csprm->min_ll [LNG] = -tmp1;
		csprm->max_ll [LNG] =  tmp1;
		tmp1 = cs_Ten * fabs (oblqm->cos_az) + zone_wd * fabs (oblqm->sin_az);
		csprm->min_ll [LAT] = oblqm->lat_0 * cs_Radian - tmp1;
		csprm->max_ll [LAT] = oblqm->lat_0 * cs_Radian + tmp1;
	}
	else
	{
		/* The definition includes a useful range specification.
		   We use these values without checking.  We expect the
		   user to give us absolute values, and we convert
		   to values relative to the central meridian. */

		csprm->min_ll [LNG] = CS_adj180 (csprm->csdef.ll_min [LNG] - csprm->cent_mer);
		csprm->min_ll [LAT] = csprm->csdef.ll_min [LAT];
		csprm->max_ll [LNG] = CS_adj180 (csprm->csdef.ll_max [LNG] - csprm->cent_mer);
		csprm->max_ll [LAT] = csprm->csdef.ll_max [LAT];
	}

	/* Similarly with the X's and Y's.  If the coordinate system
	   definition carries some values, we use them.  If not, we
	   calculate some appropriate values.  When calculating values,
	   we have to consider if this coordinate system is rectified
	   or not. */

	if (csprm->csdef.xy_min [XX] == 0.0 &&
	    csprm->csdef.xy_max [XX] == 0.0)
	{
		/* No specification in the coordinate system definition.
		   If not rectified, we'll just use uu_max and vv_max.
		   If rectified, we'll use the rectified versions of
		   these values. */

		if (oblqm->rect_flg)
		{
			csprm->min_xy [XX] = -oblqm->vv_max * oblqm->cos_az +
						 		 -oblqm->uu_max * oblqm->sin_az;
			csprm->min_xy [YY] = -oblqm->uu_max * oblqm->cos_az -
								 -oblqm->vv_max * oblqm->sin_az;
			csprm->max_xy [XX] =  oblqm->vv_max * oblqm->cos_az +
						 		  oblqm->uu_max * oblqm->sin_az;
			csprm->max_xy [YY] =  oblqm->uu_max * oblqm->cos_az -
								  oblqm->vv_max * oblqm->sin_az;
		}
		else
		{
			csprm->min_xy [XX] = -oblqm->vv_max;
			csprm->min_xy [YY] = -oblqm->uu_max;
			csprm->max_xy [XX] =  oblqm->vv_max;
			csprm->max_xy [YY] =  oblqm->uu_max;
		}
		CS_quadMM (csprm->min_xy,csprm->max_xy,oblqm->x_off,
											   oblqm->y_off,
											   oblqm->quad);
	}
	else
	{
		/* Use what ever the user has given us.  No adjustment
		   necessary.  Note: we don't check anything. */

		csprm->min_xy [XX] = csprm->csdef.xy_min [XX];
		csprm->min_xy [YY] = csprm->csdef.xy_min [YY];
		csprm->max_xy [XX] = csprm->csdef.xy_max [XX];
		csprm->max_xy [YY] = csprm->csdef.xy_max [YY];
	}


	/* That's all the calculations.  Stuff some function
	   addresses and we are done.  All four projections use
	   the same functions.  Since the projection is conformal,
	   the k and h scale factors are the same. */

	csprm->ll2cs    = (cs_LL2CS_CAST)CSMNDOToblqmF;
	csprm->cs2ll    = (cs_CS2LL_CAST)CSMNDOToblqmI;
	csprm->cs_scale = (cs_SCALE_CAST)CSMNDOToblqmK;
	csprm->cs_sclk  = (cs_SCALK_CAST)CSMNDOToblqmK;
	csprm->cs_sclh  = (cs_SCALH_CAST)CSMNDOToblqmK;
	csprm->cs_cnvrg = (cs_CNVRG_CAST)CSMNDOToblqmC;
	csprm->llchk    = (cs_LLCHK_CAST)CSMNDOToblqmL;
	csprm->xychk    = (cs_XYCHK_CAST)CSMNDOToblqmX;

	return;
}

/**********************************************************************
**	rtn_val = CSMNDOToblqmF (oblqm,xy,ll);
**
**	struct cs_MNDOTOblqm_ *oblqm;	structure which defines the parameters
**								in effect for the transformation.
**	double xy [2];				the results are returned here, x ([0])
**								and y ([1]).
**	double ll [2];				the longitude ([0]) and latitude ([1]),
**								in degrees, to be converted.
**	int rtn_val;				returns cs_CNVRT_NRML if result is normal;
**								cs_CNVRT_RNG if value to be converted is
**								outside of mathematical scope of the
**								projection.
**
**	The ll and xy arguments of this function may point
**	to the same array with no adverse affects.
**
**	This function calculates the Oblique Mercator
**	projection and returns values in the coordinate system
**	described in the cs_MNDOTOblqm_ structure.  If the ecentricity
**	of the datum is zero, the formulas for the earth as
**	a sphere are used.
**
**	All the formulas used here were extracted from the
**	USGS publication "Map Projections Used by the
**	U. S. Geological Survey", Second Edition, reprinted
**	1984, pages 73-84.
**
**	In the arrays of doubles, x and longitude values always
**	occur first.  Negative values indicate values West or
**	South of the origin.
**********************************************************************/

int EXP_LVL9 CSMNDOToblqmF (Const struct cs_MNDOTOblqm_ *oblqm,double xy [2],Const double ll [2])
{
	extern double cs_Degree;			/* 1.0 / 57.29577... */
	extern double cs_Pi_o_2;			/* pi over two       */
	extern double cs_Pi;				/* 3.14159...        */
	extern double cs_Mpi;				/* 3.14159...        */
	extern double cs_Two_pi;			/* 2 Pi              */
	extern double cs_NPTest;			/* 0.001 seconds of arc
										   short of the north
										   pole in radians. */
    extern double cs_One;
	int rtn_val;
	int rtn_valUV;

	double lng;			/* The given longitude, after conversion
						   to radians. */
	double lat;			/* The given latitude after conversion
						   to radians. */
	double del_lng;		/* delta longitude from the longitude of
						   the origin point. */

	double uu;		/* unrectified U coordinate */
	double vv;		/* unrectified V coordinate */
    double sin_lat;
    double tmp1;
    double scaleAdjustment;


	rtn_val = cs_CNVRT_NRML;

	/* There are two formulae, one for the sphere and
	   one for the ellipsoid.  If the ecentricity
	   of the dataum in use is 0.0 exactly, we
	   shall use the spherical formulae.  There
	   is a miminal amount of stuff which is
	   common to both which we perform first.

	   Convert the latitude and longitude to radians. */

	lng = cs_Degree * ll [LNG];
	lat = cs_Degree * ll [LAT];

	if (fabs (lat) > cs_NPTest)
	{
		/* The actual true poles themselves have definite
		   coordinates.  However, longitude at the poles
		   is undefined mathematically. */

		rtn_val = cs_CNVRT_INDF;
		if (fabs (lat) > cs_Pi_o_2)
		{
			rtn_val = cs_CNVRT_RNG;
			lat = CS_adj1pi (lat);
		}
	}

	del_lng = lng - oblqm->lng_0;
	if      (del_lng > cs_Pi  && oblqm->lng_0 < 0.0) del_lng -= cs_Two_pi;
	else if (del_lng < cs_Mpi && oblqm->lng_0 > 0.0) del_lng += cs_Two_pi;
	if (fabs (del_lng) > cs_Pi)
	{
		rtn_val = cs_CNVRT_RNG;
		del_lng = CS_adj2pi (del_lng);
	}

	/* uu and vv calculations are isolated in a separate function, as
	   CSMNDOToblqmK also needs access to them. Thus, substantial code
	   duplication is avoided. */

	rtn_valUV = CSMNDOToblqmFuv (oblqm,lng,lat,del_lng,&uu,&vv);
	if (rtn_valUV != 0) rtn_val = rtn_valUV;

	/* Apply the automatic centereing if active. Note, that this
	   applies only to the one point ellipsoidal forms. In all
	   other cases, oblqm->uc is set to 0.0 */

	uu -= oblqm->uc;

	/* Rectify, as appropriate. */

	switch (oblqm->rect_flg) {

	case csOBLQM_ALASKA:

		xy [XX] = vv * oblqm->cos_az + uu * oblqm->sin_az;
		xy [YY] = uu * oblqm->cos_az - vv * oblqm->sin_az;
		break;

	case csOBLQM_RECT:

		xy [XX] = vv * oblqm->cos_gam_0 + uu * oblqm->sin_gam_0;
		xy [YY] = uu * oblqm->cos_gam_0 - vv * oblqm->sin_gam_0;
		break;

	default:

		/* Not rectified. */

		xy [XX] = uu;
		xy [YY] = vv;
		break;
	}




    /* NOW THE MINNESOTA SPECIAL PROCESSING */
    sin_lat = sin(lat);
	tmp1 = oblqm->e_sq * sin_lat * sin_lat;
    tmp1 = oblqm->e_rad / sqrt (cs_One - tmp1);
    scaleAdjustment = (tmp1 + oblqm->height) / tmp1;
	xy [XX] *= scaleAdjustment;
	xy [YY] *= scaleAdjustment;
    /* END OF MINNESOTA special processing */

	/* Apply non-standard quadrant. */

	if (oblqm->quad == 0)
	{
		xy [XX] += oblqm->x_off;
		xy [YY] += oblqm->y_off;
	}
	else
	{
		CS_quadF (xy,xy [XX],xy [YY],oblqm->x_off,oblqm->y_off,oblqm->quad);
	}	
    /* That's that. */

	return (rtn_val);
}

/*
	To prevent a large scale duplication of code, we have the following
	calculation of u and v in a separate function. This is necessary as
	the scale function (CSMNDOToblqmK) needs to be able to calculate u.

	Note, for this function, lat, lng, and del_lng arguments are in
	radians, and have already been checked and normalized.
*/

int EXP_LVL9 CSMNDOToblqmFuv (Const struct cs_MNDOTOblqm_ *oblqm,double lng,double lat,double del_lng,double* uu,double *vv)
{
	extern double cs_Pi_o_4;			/* pi over four      */
	extern double cs_Half;				/* 0.5               */
	extern double cs_One;				/* 1.0               */
	extern double cs_NPTest;			/* 0.001 seconds of arc
										   short of the north
										   pole in radians. */
	extern double cs_AnglTest;			/* 0.001 seconds of arc
										   in radians. */
	extern double cs_AnglTest1;			/* 1.0 - cs_AnglTest */

	int rtn_val;

	double sin_lat;
	double cos_lat;

	double sin_del;
	double cos_del;

	double t;
	double A;
	double Bdel;
	double Q;
	double S;
	double T;
	double U;
	double V;

	double tmp1;
	double tmp2;
	double tmp3;

	rtn_val = cs_CNVRT_NRML;

	/* There are two formulae, one for the sphere and
	   one for the ellipsoid.  If the ecentricity
	   of the dataum in use is 0.0 exactly, we
	   shall use the spherical formulae.  There
	   is a miminal amount of stuff which is
	   common to both which we perform first.

	   Some other values common to both the spherical
	   and ellipsoidal. */

	sin_lat = sin (lat);
	cos_lat = cos (lat);

	/* Are we to perform the spherical calculations. */

	if (oblqm->ecent == 0.0)
	{
		/* Here for the sphere. */

		sin_del = sin (del_lng);
		cos_del = cos (del_lng);

		/* The uu value.  (tmp1 + tmp2) AND cos_del will BOTH
		   be zero at the oblique pole. */

		tmp1 = tan (lat) * oblqm->cos_lat_p;
		tmp2 = oblqm->sin_lat_p * sin_del;

		if (fabs (cos_del) < cs_AnglTest)
		{
			/* Adjust cos_del by just enough to keep things
			   from blowing up. The tmp3 result is going to
			   be pi/2 regardless. */

			rtn_val = cs_CNVRT_RNG;
			cos_del = cs_AnglTest;
			if ((lng - oblqm->lng_0) < 0.0)
			{
				cos_del = -cos_del;
			}
		}

		tmp3 = atan2 ((tmp1 + tmp2),cos_del);
		*uu = oblqm->ak * tmp3;

		/* Now for vv.  Problems if A == 1.0.  This
		   corresponds to 90 degrees of angular distance
		   from the central line, i.e. the oblique pole.
		   Since A is the sine of an angular distance,
		   cs_AnglTest1 (1.0 - cs_AnglTest) is an appropriate
		   test value. */

		A = oblqm->sin_lat_p * sin_lat -
		    oblqm->cos_lat_p * cos_lat * sin_del;

		if (fabs (A) > cs_AnglTest1)
		{
			rtn_val = cs_CNVRT_RNG;
			if (A > 0.0) *vv =  oblqm->vv_max;
			else	     *vv = -oblqm->vv_max;
		}
		else
		{
			tmp1 = (cs_One + A) / (cs_One - A);
			*vv = cs_Half * oblqm->ak * log (tmp1);
		}
	}
	else
	{
		/* Here for the ellisoid.  If the latitude is either
		   plus 90 or minus 90, we have a default condition. */

		if (fabs (lat) > cs_NPTest)
		{
			/* Here if latitude is very close to 90
			   degrees. */

			tmp1 = oblqm->gamma_0 * cs_Half;
			if (lat > 0.0) tmp1 = - tmp1;
			tmp2 = tan (cs_Pi_o_4 + tmp1);
			*vv = oblqm->A_ovr_B * log (tmp2);
			*uu = oblqm->A_ovr_B * lat;
		}
		else
		{
			/* Here for all other latitudes. */

			tmp1 = tan (cs_Pi_o_4 - (lat * cs_Half));
			tmp2 = oblqm->ecent * sin_lat;
			tmp3 = (cs_One - tmp2) / (cs_One + tmp2);
			tmp3 = pow (tmp3,oblqm->e_ovr_2);
			t = tmp1 / tmp3;

			/* B is never zero for a vaild ellipsoid.
			   E should never be zero either. */

			Q = oblqm->E / pow (t,oblqm->B);
			tmp1 = cs_One / Q;
			S = cs_Half * (Q - tmp1);
			T = cs_Half * (Q + tmp1);
			Bdel = oblqm->B * del_lng;
			V = sin (Bdel);

			U = (S * oblqm->sin_gam_0 - V * oblqm->cos_gam_0) / T;

			if (fabs (U) > cs_AnglTest1)
			{
				rtn_val = cs_CNVRT_RNG;
				if (U > 0.0) *vv =  oblqm->vv_max;
				else         *vv = -oblqm->vv_max;
			}
			else
			{
				tmp1 = (cs_One - U) / (cs_One + U);
				*vv = cs_Half * oblqm->A_ovr_B * log (tmp1);
			}
			tmp1 = S * oblqm->cos_gam_0 +
				   V * oblqm->sin_gam_0;
			tmp2 = cos (Bdel);
			if (fabs (tmp2) < cs_AnglTest)
			{
				rtn_val = cs_CNVRT_RNG;
				*uu = oblqm->A * Bdel;
			}
			else
			{
				/* Due to the above, tmp2 is never zero. */

				*uu = oblqm->A_ovr_B * atan2 (tmp1,tmp2);
			}
		}
	}
	return rtn_val;
}

/**********************************************************************
**	rtn_val = CSMNDOToblqmI (oblqm,ll,xy);
**
**	struct cs_MNDOTOblqm_ *oblqm;	structure which defines the parameters
**								in effect for the transformation.
**	double ll [2];				the longitude ([0]) and latitude ([1]),
**								in degrees, are returnrd here.
**	double xy [2];				the coordinates to be converted, x ([0])
**								and y ([1]).
**	int rtn_val;				returns cs_CNVRT_NRML if result is normal;
**								cs_CNVRT_RNG if value to be converted is
**								outside of mathematical scope of the
**								projection.
**
**	The ll and xy arguments of this function may point
**	to the same array with no adverse affects.
**
**	All the formulae used here were extracted from the
**	USGS publication "Map Projections Used by the
**	U. S. Geological Survey", Second Edition, reprinted
**	1984, pages 73-84.
**
**	In the arrays of doubles, x and longitude values always
**	occur first.  Negative values indicate values West or
**	South of the origin.
**********************************************************************/

int EXP_LVL9 CSMNDOToblqmI (Const struct cs_MNDOTOblqm_ *oblqm,double ll [2],Const double xy [2])
{
    extern double cs_Degree;			/* 1.0 / 57.29577... */

    // First we make a first evaluation of the latitude/longitude
    CSMNDOToblqmIWithScaleLatitudeAdjustments (oblqm, ll, xy, oblqm->lat_0);

    // Then we use the approximate latitude/longitude
    return CSMNDOToblqmIWithScaleLatitudeAdjustments (oblqm, ll , xy, cs_Degree * ll[LAT]);

}

/* Same as above except that the scale latitude is used to compute the local scale reduction factor.
  The scale latitude must be provided in radians
*/
int EXP_LVL9 CSMNDOToblqmIWithScaleLatitudeAdjustments (Const struct cs_MNDOTOblqm_ *oblqm,double ll [2],Const double xy [2], double scale_latitude)

{
	extern double cs_Pi;				/*  3.14159....   */
	extern double cs_Pi_o_2;			/* pi over two    */
	extern double cs_Radian;			/* 57.29577...    */
	extern double cs_Zero;				/* 0.0 */
	extern double cs_Half;				/* 0.5            */
	extern double cs_One;				/* 1.0            */
	extern double cs_Mone;				/* -1.0           */
	extern double cs_Two;				/* 2.0            */
	extern double cs_AnglTest1;			/* 1.0 - cs_AnglTest */
	extern double cs_Degree;			/* 1.0 / 57.29577... */

	int rtn_val;

	double xx;
	double yy;
	double uu;
	double vv;

	double lat;
	double del_lng;

	double t;
	double chi;
	double Q;
	double S;
	double T;
	double V;
	double U;

	double tmp1;
	double tmp2;
	double tmp3;
	double tmp4;
	double tmp5;

    double sin_lat;
	double tmp6;
    double scaleAdjustment;

	rtn_val = cs_CNVRT_NRML;





	/* There are two formulae, one for the sphere and
	   one for the ellipsoid.  If the ecentricity
	   of the dataum in use is 0.0 exactly, we
	   shall use the spherical formulae. */

	if (oblqm->quad == 0)
	{
		xx = xy [XX] - oblqm->x_off;
		yy = xy [YY] - oblqm->y_off;
	}
	else
	{
		CS_quadI (&xx,&yy,xy,oblqm->x_off,oblqm->y_off,oblqm->quad);
	}

    /* NOW THE MINNESOTA SPECIAL PROCESSING */
    sin_lat = sin(scale_latitude);
	tmp6 = oblqm->e_sq * sin_lat * sin_lat;
    tmp6 = oblqm->e_rad / sqrt (cs_One - tmp6);
    scaleAdjustment = (tmp6 + oblqm->height) / tmp6;
	xx /= scaleAdjustment;
	yy /= scaleAdjustment;
    /* END OF MINNESOTA special processing */

	/* First, if the coordinates are rectified, we must unrectify them. */

	switch (oblqm->rect_flg) {

	case csOBLQM_ALASKA:

		uu = yy * oblqm->cos_az + xx * oblqm->sin_az;
		vv = xx * oblqm->cos_az - yy * oblqm->sin_az;
		break;

	case csOBLQM_RECT:

		uu = yy * oblqm->cos_gam_0 + xx * oblqm->sin_gam_0;
		vv = xx * oblqm->cos_gam_0 - yy * oblqm->sin_gam_0;
		break;

	default:
	
		/* Unrectified system. */
	
		uu = xx;
		vv = yy;
		break;
	}

	/* Undo the affect of the automatic centering feature, if
	   applicable. oblqm->uc is zero in all cases where it is
	   not applicable. */

	uu += oblqm->uc;

	/* Verify that we are in suitable range.  If not, force a
	   reasonable condition. */

	if (fabs (uu) > oblqm->uu_max)
	{
		rtn_val = cs_CNVRT_RNG;
		if (uu > 0.0) uu =  oblqm->uu_max;
		else	      uu = -oblqm->uu_max;
	}
	if (fabs (vv) > oblqm->vv_max)
	{
		rtn_val = cs_CNVRT_RNG;
		if (vv > 0.0) vv =  oblqm->vv_max;
		else	      vv = -oblqm->vv_max;
	}

	/* See if we have the spherical case. */

	if (oblqm->ecent == 0.0)
	{
		/* Here for the sphere. */

		tmp1 = uu / oblqm->ak;
		tmp2 = vv / oblqm->ak;
		tmp3 = sin (tmp1);

		tmp4 = oblqm->sin_lat_p * tanh (tmp2);
		/* cosh is never zero. */
		tmp4 += oblqm->cos_lat_p * tmp3 / cosh (tmp2);
		if (fabs (tmp4) > cs_One)
		{
			rtn_val = cs_CNVRT_RNG;
			tmp4 = (tmp4 > 0.0) ? cs_One : cs_Mone;
		}
		lat = asin (tmp4);

		tmp4 = (oblqm->sin_lat_p * tmp3) - (oblqm->cos_lat_p * sinh (tmp2));
		tmp5 = cos (tmp1);

		/* If uu is forced to be less than or equal uu_max; tmp1
		   should never be one, and tmp5 should never be zero.
		   Since atan2 will always have at least one non-zero
		   argument, it should be safe. */

		del_lng = atan2 (tmp4,tmp5);
	}
	else
	{
		/* Here for the ellisoid. */

		tmp1 = vv / oblqm->A_ovr_B;
		tmp2 = exp (tmp1);	/* exp is valid for all real numbers */
		Q = cs_One / tmp2;
		S = (Q - tmp2) * cs_Half;
		T = (Q + tmp2) * cs_Half;
		V = sin (uu / oblqm->A_ovr_B);
		U = (V * oblqm->cos_gam_0 + S * oblqm->sin_gam_0) / T;
		if (fabs (U) > cs_AnglTest1)
		{
			rtn_val = cs_CNVRT_INDF;
			del_lng = cs_Zero;
			lat = cs_Pi_o_2;
			if (U < 0.0) lat = -lat;
		}
		else
		{
			/* We handled U = -+1 above; and as a result,
			   tmp3 should never be zero. */

			tmp3 = (cs_One + U) / (cs_One - U);
			tmp3 = oblqm->E / sqrt (tmp3);
			t = pow (tmp3,oblqm->one_ovr_B);

			/* Calculate chi, the isometric latitude, and hence
			   the geographic latitude. */

			chi = cs_Pi_o_2 - cs_Two * atan (t);
			lat = CSchiIcal (&oblqm->chicofI,chi);

			tmp4 = (S * oblqm->cos_gam_0) - (V * oblqm->sin_gam_0);

			/* Magnitude of uu will never be greater than uu_max,
			   which means tmp5 will never be zero.  Close, but
			   never zero. */

			tmp5 = cos (uu / oblqm->A_ovr_B);
			del_lng = -atan2 (tmp4,tmp5) / oblqm->B;
		}
	}

	/* Convert the results to degrees. */

	if (fabs (del_lng) > cs_Pi)
	{
		rtn_val = cs_CNVRT_RNG;
		del_lng = CS_adj2pi (del_lng);
	}

	ll [LNG] = (del_lng + oblqm->lng_0) * cs_Radian;
	ll [LAT] = lat * cs_Radian;
	return (rtn_val);
}

/**********************************************************************
**	gamma = CSMNDOToblqmC (oblqm,ll);
**
**  THIS FUNCTION iS VIRTUIALLY UNCHANGED FROM ORIGINAL CSoblqmC
**  EXCEPT THAT PROPER Forward and Inverse FUNCTIONS ARE CALLED 
**
**	struct cs_MNDOTOblqm_ *oblqm;	structure which carries all parameters
**								in effect for the coordinate system
**								being used, assuming the Hotine Oblique Mercator
**								projection.
**	double ll [2];				the longitude ([0]) and the latitude ([1])
**								of the point at which the true scale of
**								the coordinate system is to be computed.
**								Values are in degrees.
**	double gamma;				returns the computed convergence angle for
**								the coordinate system as the specified
**								location in degrees east of north.
**
**	Returns cs_Km360 as an error indication; caused by a point
**	outside the domain of the projection.
**
**	We have not as yet found or deduced an analytical equation
**	for the convergence angle for this projection.  We calculate
**	it empicially. The convergence angle is defined as the
**	arctangent of the partial derivative of Y with respect to
**	latitude (read delta Y when the latitude changes a skosh)
**	divied by the partial derivative of X with repsect to
**	latitude (i.e. delta X).  See Synder/Bugayevskiy, page 16.
**********************************************************************/

double EXP_LVL9 CSMNDOToblqmC (Const struct cs_MNDOTOblqm_ *oblqm,Const double ll [2])
{
	extern double cs_Radian;			/* 57.2957... */
	extern double cs_Km360;				/* -360.0, the value which
										   we return if provided
										   with a bogus point. */
	int status;

	double del_xx;
	double del_yy;
	double gamma;				/* some folks call this alpha */

	double xy1 [3];
	double xy2 [3];
	double my_ll [3];

	my_ll [LNG] = ll [LNG];
	my_ll [LAT] = ll [LAT];

	/* Compute the cartesian coordinates of the end points of a
	   linear line segment whose end points are on the same meridian,
	   but separated by a small amount of latitude.  The degree of
	   latitude separation is rather arbitrary.  Technically, the
	   smaller the better, but if its too small, we end up with
	   noise problems in the trig functions.  0.00005 gives us a
	   line of about 10 meters on the surface of the earth.  We
	   use literal constants as this may need to be adjusted to
	   the mathemagics of the specific projection involved. */

	my_ll [LAT] -= 0.00005;
	status = CSMNDOToblqmF (oblqm,xy1,my_ll);
	if (status != cs_CNVRT_NRML)
	{
		return (cs_Km360);
	}
	my_ll [LAT] += 0.0001;		/* 2 * 0.00005 */
	status = CSMNDOToblqmF (oblqm,xy2,my_ll);
	if (status != cs_CNVRT_NRML)
	{
		return (cs_Km360);
	}

	/* Some atan2's (not all) don't like it when both arguments are
	   zero. Normally, it is safe to assume that del_yy is never
	   zero given the above. However, testing has shown that when
	   provided with unrealistic locations, both del_xx and del_yy
	   can be zero (exactly on the pole where the pole is
	   the origin). */

	del_xx = xy2 [XX] - xy1 [XX];
	del_yy = xy2 [YY] - xy1 [YY];
	if ((fabs (del_xx) + fabs (del_yy)) > 0.0)
	{
		gamma = -atan2 (del_xx,del_yy) * cs_Radian;
	}
	else
	{
		gamma = cs_Km360;
	}
	return (gamma);
}

/**********************************************************************
**	kk = CSMNDOToblqmK (oblqm,ll);
**
**	struct cs_MNDOTOblqm_ *oblqm;	structure which carries all parameters
**								in effect for the coordinate system
**								being used, assuming the Lambert projection.
**	double ll [2];				the longitude ([0]) and the latitude ([1])
**								of the point at which the true scale of
**								the coordinate system is to be computed.
**								Values are in degrees.
**	double kk;					returns the grid scale factor along a
**								parallel of the projected coordinate system
**								at the specified point.
**********************************************************************/

double EXP_LVL9 CSMNDOToblqmK (Const struct cs_MNDOTOblqm_ *oblqm,Const double ll [2])
{
	extern double cs_Degree;			/* 1.0 / 57.29577... */
	extern double cs_One;				/* 1.0               */
	extern double cs_AnglTest;			/* 0.001 seconds of arc,
										   in radians. */
	extern double cs_SclInf;			/* 9.9E+04, the value
										   we return for infinite
										   scale. */
	double lng;
	double lat;
	double kk;

	double sin_lat;
	double cos_lat;
	double del_lng;
	double A;

	double tmp1;
	double tmp2;
	double tmp3;

	double uu;
	double vv;

	lng = ll [LNG] * cs_Degree;
	lat = ll [LAT] * cs_Degree;
	sin_lat = sin (lat);
	cos_lat = cos (lat);
	del_lng = CS_adj2pi (lng - oblqm->lng_0);

	/* Set the error condition until we know different. */

	kk = cs_SclInf;
	if (oblqm->ecent == 0.0)
	{
		/* Here for the sphere. Note, this blows up at the
		   oblique poles. */

		tmp1 = oblqm->cos_lat_p * cos_lat * sin (del_lng);
		A = oblqm->sin_lat_p * sin_lat - tmp1;
		tmp1 = cs_One - (A * A);

		/* Read tmp1 as the sine squared  of the angluar
		   distance of the point from the oblique pole. */

		if (tmp1 > cs_AnglTest)
		{
			kk = oblqm->k0 / sqrt (tmp1);
		}
	}
	else
	{
		/* Here for the ellipsoid.  We need the u coordinate
		   for this one.  We call CSMNDOToblqmF to compute it for
		   us. */

		if (CSMNDOToblqmFuv (oblqm,lng,lat,del_lng,&uu,&vv) == 0)
		{
			tmp1 = oblqm->A * cos (uu / oblqm->A_ovr_B);
			tmp2 = oblqm->ecent * sin_lat;
			tmp2 = cs_One - tmp2 * tmp2;
			tmp2 = sqrt (tmp2);
			tmp3 = cos (oblqm->B * del_lng);
			tmp3 = oblqm->ak * cos_lat * tmp3;
			if (tmp3 > cs_AnglTest)
			{
				kk = oblqm->k0 * tmp1 * tmp2 / tmp3;
			}
		}
	}
	return (kk);
}

/**********************************************************************
**	status = CSMNDOToblqmL (oblqm,cnt,pnts);
**
**	struct cs_MNDOTOblqm_ *oblqm;	the coordinate system against which the check is
**								to be performed.
**	int cnt;					the number of points in the region to be
**								checked.
**	double pnts [][2];			the list of coordinates to be checked.
**	int status;					returns cs_CNVRT_OK if the point, line, or region
**								is completely within the domain of the
**								coordinate system.  Otherwise, cs_CNVRT_DOMN
**								is returned.
**
**	This function simply checks the mathematical viability of
**	a coordinate conversion.  It has nothing to do with the
**	useful limits of the coordinate system.
**
**	This function expects that the input lat/longs are normalized
**	if appropriate.
**********************************************************************/

/*
	The following function is static, not visible outside
	this source module, so the name may not adhere to the
	normal naming conventtion.

	The function verifies that the point supplied to it
	is indeed within the geographic domain of the coordinate
	system; in this case, not within a small distance of
	either pole.
*/

static int CSMNDOToblqmLP (Const struct cs_MNDOTOblqm_ *oblqm,Const double ll [2])
{
	extern double cs_Degree;		/* 1.0 / 57.2... */
	extern double cs_Pi;			/* PI */
	extern double cs_AnglTest;		/* 0.001 seconds of arc in
									   radians */

	double cc;						/* Angular distance between
									   two points on the sphere,
									   in radians. */

	double pole_ll [2];
	double test_ll [2];

	/* We'll compute the angular distance from the point to the
	   two poles.  If within .001 seconds of arc of a pole, we
	   report a problem.  We do this calculation on a sphere.
	   This should be OK for most every application. */

	pole_ll [LNG] = oblqm->lng_p;
	pole_ll [LAT] = oblqm->lat_p;

	test_ll [LNG] = ll [LNG] * cs_Degree;
	test_ll [LAT] = ll [LAT] * cs_Degree;

	cc = CSccsphrR (pole_ll,test_ll);
	if (cc < cs_AnglTest) return (cs_CNVRT_DOMN);

	/* The other pole. */

	pole_ll [LNG] = CS_adj2pi (pole_ll [LNG] + cs_Pi);
	pole_ll [LAT] = -pole_ll [LAT];
	cc = CSccsphrR (pole_ll,test_ll);
	if (cc < cs_AnglTest) return (cs_CNVRT_DOMN);

	return (cs_CNVRT_OK);
}

int EXP_LVL9 CSMNDOToblqmL (Const struct cs_MNDOTOblqm_ *oblqm,int cnt,Const double pnts [][3])

{
	extern double cs_Degree;
	extern double cs_EETest;			/* .001 seconds of arc less
										   than PI/2 in radians. */
	extern double cs_WETest;			/* -cs_EETest */

	int ii;
	int status;

	double tmp;
	double del_lng;

	/* Check all the points. */

	status = cs_CNVRT_OK;
	for (ii = 0;ii < cnt && status == cs_CNVRT_OK;ii++)
	{
		status = CSMNDOToblqmLP (oblqm,pnts [ii]);
	}
	if (cnt <= 1 || status != cs_CNVRT_OK) return (status);

	/* If cnt is 2, we have a line which we must check. */

	if (cnt == 2)
	{
		for (ii = 0;ii < cnt;ii++)
		{
			tmp = pnts [ii][LNG] * cs_Degree;
			del_lng = CS_adj2pi (tmp - oblqm->lng_0);
			if (del_lng > cs_EETest || del_lng < cs_WETest)
			{
				status = cs_CNVRT_DOMN;
				break;
			}
		}
	}
	else if (cnt == 3)
	{
		/* Can't handle a three point list, the region must
		   be closed. */

		CS_erpt (cs_RGN_PNTCNT);
		return (cs_CNVRT_ERR);
	}
	else
	{
		/* WHAT WE SHOULD BE DOING:
		   The great circle between the two singularity points
		   is a 180 degree segment of a great circle.  We should:
		   1) use the technique described for the line case
		      to see if any segment of the region actually
		      goes through either of the singularity points.
		   2) count the intersections of all segments in the
		      boundary with the great circle segment between the
		      two singularity points.  If the count is even
		      (or zero) we are OK.  Otherwise, we have a problem.
		      That is, the region includes a singularity point.

		   WHAT WE DO NOW:
		   We simply see if any of the points has a longitude
		   which puts it outside of the range of central
		   longitude +- pi/2. */

		for (ii = 0;ii < cnt;ii++)
		{
			tmp = pnts [ii][LNG] * cs_Degree;
			del_lng = CS_adj2pi (tmp - oblqm->lng_0);
			if (del_lng > cs_EETest || del_lng < cs_WETest)
			{
				status = cs_CNVRT_DOMN;
				break;
			}
		}
	}
	return (status);
}

/**********************************************************************
**	status = CSMNDOToblqmX (oblqm,cnt,pnts);
**
**	struct cs_MNDOTOblqm_ *oblqm;	coordinate system definition
**	int cnt;					number of points in the pnts array.
**	double pnts [][3];			an array of three dimensional points which
**								define a point, line, or region to be checked.
**	int status;					returns the status of the check; see REMARKS
**								below.
**
**	The values provided in the pnts array are required to be
**	cartesian coordinates.  Use CSMNDOToblqmL to check lat/long
**	values.
**
**	What gets checked depends upon the cnt argument.  A value
**	of zero (or less than that) checks nothing successfully.
**	A value of 1 indicates that a single point is to be checked.
**	a value of 2 indicates that a line is to be checked.  The
**	entire line must reside within the domain of the coordinate
**	system to pass.
**
**	A value of 4 or more for cnt indicates that a region is to
**	be checked.  The pnts array must contain the indicated
**	number of points which defines a region.  The last point
**	in the array must duplicate the first point.  The resulting
**	polygon must be simply connected (no bow ties).  The region
**	passes if the entire region is within the domain of the
**	coordinate system.
**
**	The return value refers to either the point, or the line
**	segment, provided. Return values are as follows:
**
**	cs_CNVRT_OK	the point, entire line segment, or entire region,
**			is within the cartesian domain of the coordinate
**			system.
**	cs_CNVRT_DOMN	the point, all or a portion of the line segment,
**			or all or a portion of the region, is not within
**			the cartesian domain of the coordinate system.
**********************************************************************/

int EXP_LVL9 CSMNDOToblqmX (Const struct cs_MNDOTOblqm_ *oblqm,int cnt,Const double pnts [][3])
{
	int rtn_val;

	rtn_val = cs_CNVRT_OK;

	return (rtn_val);
}							/*lint !e715 */




//======================================================================================================================
// This part contains the code originally provided by the Minnesota DOT
//======================================================================================================================


#if (0)


// This particular function is not necessary in the context of CSMAP where parameters are provided through the database

void Parameters(int ProjNum, double& SouthPar, double& NorthPar, double& LongMerid, double& LatGrid,
                double& EllipsShift, double& FNorth, double& FEast, double& ScaleFactor,
                double& A, double& f)
//
// This sub supplies parameters for the given map projection
//
{
  // Set defaults
  A = 6378137;                     // Length of semi-major axis of the GRS80 ellipsoid
  f = 3.35281068118364E-03;        // Flattening for GRS80 ellipsoid
  //if ProjNum <= 102 Or ProjNum >= 266 Then
  //  Datum = 83
  //Else
  //  Datum = 27
  //End If


  SouthPar = 0;
  NorthPar = 0;
  LongMerid = 0;
  LatGrid = 0;
  ScaleFactor = 0.0;
  
  if (ProjNum = 24) 
  {
    //ProjName = "Cook County North Shore Zone";
    //StatePlane = "N";
    //ProjType = "O";
    SouthPar = 62.0;           // Skew
    NorthPar = 0.0;
    LongMerid = -89.3277777777778;
    LatGrid = 46.5;
    EllipsShift = -1693.167;
    FNorth = -3505207.011;    // Added northing
    FEast = -6187452.376;     // Added easting
    ScaleFactor = 1.0;
  }
  else if (ProjNum = 48) 
  {
    //ProjName = "Lake County North Shore Zone";
    //StatePlane = "N";
    //ProjType = "O";
    SouthPar = 46.0;         // Skew
    NorthPar = 0.0;
    LongMerid = -89.9761111111111;
    LatGrid = 46.1666666666667;
    EllipsShift = -1697.739;
    FNorth = -4389128.779;  // Added northing
    FEast = -4206248.413;   // Added easting
    ScaleFactor = 1.0;
  }
  else if (ProjNum = 83) 
  {
    //ProjName = "St. Louis County North Shore Zone";
    //StatePlane = "N";
    //ProjType = "O";
    SouthPar = 45.0;         // Skew
    NorthPar = 0.0;
    LongMerid = -90.6916666666667;
    LatGrid = 45.75;
    EllipsShift = -1703.835;
    FNorth = -4358648.718;  // Added northing
    FEast = -4114808.23;    // Added easting
    ScaleFactor = 1.0;
  }
  
  // Convert SouthPar, NorthPar, LongMerid and LatGrid to radians
  SouthPar = SouthPar * 3.14159265358979 / 180; // Convert to radians
  NorthPar = NorthPar * 3.14159265358979 / 180; //Convert to radians
  LongMerid = LongMerid * 3.14159265358979 / 180; // Convert to radians
  LatGrid = LatGrid * 3.14159265358979 / 180; // Convert to radians
}


// This function computes projection constants from the provided parameters
// Phi - IN The latitude at which the corrected map radius is to be computed.
//          Contrary to normal Oblique Mercator
void ObliqueConstants(double Phi, double LatOrig, double Skew, double LongOrig, double ScaleFactor, double A, double f, double& R, double& E0, double& N0, 
                      double& D, double& Ff, double& G, double& Lamda0, double& C, double& B, double& F0, double& f2, double& F4, double& F6, double& e)
{
//    
// Subroutine to determine constants for ObliqueGeo and GeoOblique
//
  // Declare variables
  double Aa;       // Intermediate step
  double e2;       // Eccentricity squared
  double epsqd;    // e' squared
  double i;        // Intermediate step
  double Qc;       // Intermediate step
  double Salpha0;  // Sine of the azimuth of positive skew axis at equator
  double temp;     // Intermediate step
  double Wc;       // Intermediate step
  
  N0 = 0.0;                          // False northing
  E0 = 0.0;                          // False easting
  e2 = 6.69438002290342E-03;         // eccentricity squared
  e = 8.18191910428318E-02;          // eccentricity.0818191910428318
  epsqd = 6.73949677548162E-03;       // e squared
  R = A / sqrt(1 - e2 * sin(Phi) * sin(Phi)); // mapping radius at the point's latitude
  double tempA = 1 /sqrt(1 - e2 * sin(Phi) * sin(Phi));
  F0 = 0.006686920927;
  f2 = 0.000052014584;
  F4 = 0.00000055443;
  F6 = 0.00000000682;
  B = sqrt(1 + epsqd * pow((cos(LatOrig)), 4));
  Wc = sqrt(1 - e2 * sin(LatOrig) * sin(LatOrig));
  Aa = A * B * sqrt(1 - e2) / (Wc * Wc);
  Qc = 0.5 * ((log((1 + sin(LatOrig)) / (1 - sin(LatOrig)))) - (e * log((1 + e * sin(LatOrig)) / (1 - e * sin(LatOrig)))));
  temp = B * sqrt(1 - e2) / (Wc * cos(LatOrig));
  C = log(temp + sqrt(temp * temp - 1)) - B * Qc;
  D = ScaleFactor * Aa / B;
  Salpha0 = A * sin(Skew) * cos(LatOrig) / (Aa * Wc);
  temp = Salpha0 * (0.5 * (exp(B * Qc + C) - exp(-(B * Qc + C)))) / cos(atan(Salpha0 / sqrt(1 - Salpha0 * Salpha0))); // ASN(x)=ATN(X/sqr(1-X^2))
  Lamda0 = -LongOrig + atan(temp / sqrt(1 - temp * temp)) / B; // ASN(x)=ATN(x/SQR(1-x^2))
  Ff = Salpha0;
  G = cos(atan(Salpha0 / sqrt(1 - Salpha0 * Salpha0))); // ASN(x)=ATN(x/SQR(1-x^2))
  i = ScaleFactor * Aa / A;
}

void GeoOblique(double& OutLongEastX, double& OutLatNorthY, double Phi, double Lamda, )
//
// Subroutine to compute oblique mercator from lat/long
//
{
  // Declare variables
  double B = 0.0;      // Intermediate step
  double C = 0.0;      // Intermediate step
  double D = 0.0;      // Intermediate step
  double e = 0.0;      // First eccentricity
  double E0 = 0.0;     // False easting
  double F0 = 0.0;     // GRS 80 constant
  double f2 = 0.0;     // GRS 80 constant
  double F4 = 0.0;     // GRS 80 constant
  double F6 = 0.0;     // GRS 80 constant
  double Ff = 0.0;     // Intermediate step
  double G = 0.0;      // Intermediate step
  double J;      // Intermediate step
  double K;      // Intermediate step
  double L;      // Intermediate step
  double Lamda0 = 0.0; // Longitude of true origin
  double N0 = 0.0;     // False northing
  double Q;      // Intermediate step
  double R = 0.0;      // Radius of curvature in the prime vertical
  double u;      // Intermediate step
  double v;      // Intermediate step
  double X;      // Intermediate easting
  double Y;      // Intermediate northing
  


  double OutSouthPar;
  double OutNorthPar;
  double OutLongMerid;
  double OutLatGrid;
  double OutEllipsShift;
  double OutFNorth;
  double OutFEast;
  double OutScaleFactor;
  double Outa;
  double OutFlat;

  double OutTheta;

  // Extract the parameters
  Parameters(ProjNum, OutSouthPar, 
    OutNorthPar, OutLongMerid, OutLatGrid, OutEllipsShift, OutFNorth, OutFEast,
    OutScaleFactor, Outa, OutFlat);


  ObliqueConstants(Phi, OutLatGrid, OutSouthPar, OutLongMerid, OutScaleFactor, Outa, OutFlat, R, E0, N0, D, Ff, G, Lamda0, C, B, F0, f2, F4, F6, e);
  L = (-Lamda - Lamda0) * B;
  Q = 0.5 * ((log((1 + sin(Phi)) / (1 - sin(Phi)))) - (e * log((1 + e * sin(Phi)) / (1 - e * sin(Phi)))));
  J = 0.5 * (exp(B * Q + C) - exp(-(B * Q + C)));
  K = 0.5 * (exp(B * Q + C) + exp(-(B * Q + C)));
  u = D * atan((J * G - Ff * sin(L)) / cos(L));
  v = 0.5 * D * log((K - Ff * J - G * sin(L)) / (K + Ff * J + G * sin(L)));
  Y = u * cos(OutSouthPar) - v * sin(OutSouthPar) + N0;
  X = u * sin(OutSouthPar) + v * cos(OutSouthPar) + E0;
  OutTheta = atan((Ff - J * G * sin(L)) / (K * G * cos(L))) - OutSouthPar;
  OutLatNorthY = OutFNorth + Y * (R + OutEllipsShift) / R;
  OutLongEastX = OutFEast + X * (R + OutEllipsShift) / R;
}

void ObliqueGeo(double& Phi, double& Lamda, double InLongEastX, double InLatNorthY,int ProjNum)
//
// Subroutine to compute lat/long from oblique mercator
//
{
  // Declare variables
  double B = 0.0;        // Intermediate step
  double C = 0.0;        // Intermediate step
  double Chi = 0.0;      // Conformal latitude
  double D = 0.0;        // Intermediate step
  double e = 0.0;        // First eccentricity
  double E0 = 0.0;       // False easting
  double F0 = 0.0;       // GRS 80 constant
  double f2 = 0.0;       // GRS 80 constant
  double F4 = 0.0;       // GRS 80 constant
  double F6 = 0.0;       // GRS 80 constant
  double Ff = 0.0;       // Intermediate step
  double G = 0.0;        // Intermediate step
  int Inc;         // Incrementer
  double J = 0.0;        // Intermediate step
  double K = 0.0;        // Intermediate step
  double L = 0.0;        // Intermediate step
  double Lamda0 = 0.0;   // Longitude of true origin
  double N0 = 0.0;       // False northing
  double Q = 0.0;        // Intermediate step
  double R = 0.0;        // Radius of curvature in the prime vertical
  double Rr = 0.0;       // Intermediate step
  double S = 0.0;        // Intermediate step
  double t = 0.0;        // Tangent of latitude
  double u = 0.0;        // Intermediate step
  double v = 0.0;        // Intermediate step
  double X = 0.0;        // Intermediate easting
  double Y = 0.0;        // Intermediate northing

  // Initialize Phi to a value in Minnesota
  Phi = 48 * 3.14159265358979 / 180;


  double InSouthPar;
  double InNorthPar;
  double InLongMerid;
  double InLatGrid;
  double InEllipsShift;
  double InFNorth;
  double InFEast;
  double InScaleFactor;
  double Ina;
  double InFlat;
  double InTheta;

  // Extract the parameters
  Parameters(ProjNum, InSouthPar, 
    InNorthPar, InLongMerid, InLatGrid, InEllipsShift, InFNorth, InFEast,
    InScaleFactor, Ina, InFlat);
  // The step is done twice since the Phi value is used internally to ObliqueConstant to compute R
  // The first is therefore an estimate and the second value is quite close to real value. The iteration could have 
  // continued till change is really small but the Minnesota DOT programmer decided with this implementation. My guess is that 
  // twice is far sufficient to obtain final value.
  for (Inc = 0; Inc < 2 ; Inc++)
  {
    ObliqueConstants(Phi, InLatGrid, InSouthPar, InLongMerid, InScaleFactor, Ina, InFlat, R, E0, N0, D, Ff, G, Lamda0, C, B, F0, f2, F4, F6, e);
    Y = R * (InLatNorthY - InFNorth) / (R + InEllipsShift);
    X = R * (InLongEastX - InFEast) / (R + InEllipsShift);
    u = (X - E0) * sin(InSouthPar) + (Y - N0) * cos(InSouthPar);
    v = (X - E0) * cos(InSouthPar) - (Y - N0) * sin(InSouthPar);
    Rr = 0.5 * (exp(v / D) - exp(-(v / D)));
    S = 0.5 * (exp(v / D) + exp(-(v / D)));
    t = sin(u / D);
    Q = (0.5 * log((S - Rr * Ff + G * t) / (S + Rr * Ff - G * t)) - C) / B;
    Chi = 2 * atan((sqrt(Q) - 1) / (sqrt(Q) + 1));
    Phi = Chi + sin(Chi) * cos(Chi) * (F0 + f2 * cos(Chi) * cos(Chi) + F4 * pow(cos(Chi), 4) + F6 * pow(cos(Chi), 6));
    Lamda = ((atan((Rr * G + t * Ff) / cos(u / D))) / B) - Lamda0;
    // Compute convergence (theta)
    J = 0.5 * (exp(B * Q + C) - exp(-(B * Q + C)));
    K = 0.5 * (exp(B * Q + C) + exp(-(B * Q + C)));
    L = (-Lamda - Lamda0) * B;
    InTheta = atan((Ff - J * G * sin(L)) / (K * G * cos(L))) - InSouthPar;
  }
}


#endif








