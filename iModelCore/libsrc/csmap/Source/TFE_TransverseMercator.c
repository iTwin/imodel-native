/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------+
|
|  The algorithms within this module were developed by TotalFinalELF (TOTAL). They requested
|  that our coordinate system package support their Universal Transverse Mercator projection
|  calculations, so we undertook the task of integrating them with CS_Map. This work was 
|  originally done in March 2007, when we were using CS_Map 11.15. TOTAL suggested the
|  name as Transverse Mercator using BF calculation, in reference of the author Bernard Flaceliere.
|
+--------------------------------------------------------------------------------------*/
#include      <math.h>
#include "cs_map.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int EXP_LVL9 TRMBF_ParamCheck ( Const struct cs_Csdef_ *cs_def,unsigned short prj_code,int err_list [],int list_sz)
    {
    extern double cs_One;           /*    1.0 */
    extern double cs_Mone;          /*    1.0 */
    extern double cs_K60;           /*   60.0 */
    extern double cs_MinLng;        /* -180.0 */
    extern double cs_MinLat;        /* - 90.0 */
    extern double cs_MaxLng;        /* +180.0 */
    extern double cs_MaxLat;        /* + 90.0 */
    extern double cs_SclRedMin;     /*    0.75 */
    extern double cs_SclRedMax;     /*    1.1 */

    int err_cnt;

    /* We will return (err_cnt + 1) below. */
    
    err_cnt = -1;
    if (err_list == NULL) 
        list_sz = 0;

    if (prj_code != cs_PRJCOD_UTMZNBF)
        {
        /* Basic Transverse Mercator specific stuff. */
    
        if (cs_def->prj_prm1 <= cs_MinLng || cs_def->prj_prm1 > cs_MaxLng)
            if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_ORGLNG;
        
        if (cs_def->org_lat < cs_MinLat || cs_def->org_lat > cs_MaxLat)
            if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_ORGLAT;
    
        if (cs_def->scl_red < cs_SclRedMin || cs_def->scl_red > cs_SclRedMax)
            if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_SCLRED;
        }
    else
        {
        /* Here for the UTM projection. */
        // zone number
        if (cs_def->prj_prm1 < cs_One || cs_def->prj_prm1 > cs_K60)
            if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_UTMZON;

        // hemisphere
        if (cs_def->prj_prm2 < cs_Mone || cs_def->prj_prm2 > cs_One)
            if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_HMISPHR;
        }

    /* That's it for Transverse Mercator. */
    return (err_cnt + 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void _TRMBFSetup 
(
struct cs_Csprm_ *csprm,
struct cs_Csdef_ *csDef
)
    {
    // here's where we go from the cs_map definition to the TOTAL terminology.
    double  trav1, trav2, trav3, trav4;
    double  orgLatRadians;

    double  test_ll [3];
    double  test_xy [3];
    double  tmp1;
    double  zone_wd;
    extern double cs_Zero;          /* 0.0 */
    extern double cs_One;           /* 1.0 */
    extern double cs_Two;           /* 2.0 */
    extern double cs_Three;         /* 3.0 */
    extern double cs_Degree;        /* 1.0 / 57.29577... */
    extern double cs_EETest;        /* .001 seconds of arc */
    extern double cs_Radian;        /* 57.29577... */

    struct cs_TrmBF_ *trmBFParams = &csprm->proj_prms.trmBF;

    // ecs in TOTAL terminology is eccentricity squared.
    trmBFParams->ellipsExc      = csprm->datum.ecent * csprm->datum.ecent;
    trmBFParams->ellipsGax      = csprm->datum.e_rad;
    trmBFParams->ellipsRacexc   = csprm->datum.ecent;
    trmBFParams->projecFech     = csDef->scl_red;
    trmBFParams->quad           = 0;  // I don't think TOTAL had any provision of non-zero quad.
    trav1                       = 0.75 * trmBFParams->ellipsExc;
    trav2                       = 5.0 * trav1 * trav1 / 12.0;
    trav3                       = 7 * trav1 * trav2 / 18;
    
    trmBFParams->Utmfc          = trmBFParams->ellipsGax * trmBFParams->projecFech;
    trav4                       = trmBFParams->Utmfc * ( 1.0 - trmBFParams->ellipsExc );

    trmBFParams->Utmcf0         = trav4 * ( 1.0 + trav1 + 3 * trav2 + 10 * trav3 );
    trmBFParams->Utmcf2         = trav4 * .5 * ( trav1 + 4 * trav2 + 15 * trav3 );
    trmBFParams->Utmcf4         = trav4 * .25 * ( trav2 + 6 * trav3 );
    trmBFParams->Utmcf6         = trav4 * trav3 / 6;
    trmBFParams->Utmbta         = 0.0;

    /* Calculation of arc of meridian at origin Utmbta */
    orgLatRadians = csDef->org_lat * cs_Degree;
    if ( fabs (orgLatRadians) > 1.0e-12)
          {
          trmBFParams->Utmbta  = trmBFParams->Utmcf0 * orgLatRadians
                    - trmBFParams->Utmcf2 * sin ( 2 * orgLatRadians )
                    + trmBFParams->Utmcf4 * sin ( 4 * orgLatRadians )
                    - trmBFParams->Utmcf6 * sin ( 6 * orgLatRadians );
          }

    trmBFParams->projecMercen   = csDef->prj_prm1 * cs_Degree;
    trmBFParams->projecParori   = orgLatRadians;

    // false easting and northing
    trmBFParams->projecCtx      = csDef->x_off;
    trmBFParams->projecCty      = csDef->y_off;

    // calculate second eccentricity. Only the square of it is ever used.
    //  The second eccentricity in TOTAL terminology is "Ellips.racdex", but
    //  since only the square is ever used in the calculation, I kept the
    //  csmap terminology instead. I replaced the "ep2" with trmBFParams->eprim_sq.
    trmBFParams->eprim_sq = trmBFParams->ellipsExc / (1.0 - trmBFParams->ellipsExc);

    /* Need to compute the maximum value of X which can be
       supported mathematically. */

    test_ll [LNG] = CS_adj2pi (trmBFParams->projecMercen + cs_EETest * 2 /3) * cs_Radian; // Trmer uses 90 deg but it is too much for the BF algorythms so we use 60 degrees.
    test_ll [LAT] = cs_Zero;
    TRMBF_XYFromLatLong (trmBFParams, test_ll, test_ll);
    trmBFParams->xx_max = fabs (test_ll [XX]);

#ifdef GEOCOORD_ENHANCEMENT
    /* Original code was incorrect as the projecMercen is expressed in radians
      and the cent_mer must be in degrees. */
    csprm->cent_mer = trmBFParams->projecMercen * cs_Radian;
#else
    csprm->cent_mer = trmBFParams->projecMercen;
#endif
    if (csDef->ll_min [LNG] == 0.0 &&
        csDef->ll_max [LNG] == 0.0)
        {
        /* We're to calculate the useful range.  We'll assume a
           5 degree width, increased by an amount related to
           scale reduction.  We set the latitude range to that
           customarily used in the UTM system.

           There are some coordinate systems where the scale
           reduction factor is greater than 1; strange as it
           might seem. */

        tmp1 = trmBFParams->projecFech;
        if (tmp1 > cs_One) 
            tmp1 = cs_One;

        zone_wd = cs_Three + cs_Two * acos (tmp1) * cs_Radian;
        csprm->min_ll [LNG] = -zone_wd;
        csprm->max_ll [LNG] =  zone_wd;

        csprm->min_ll [LAT] = -84.0;
        csprm->max_ll [LAT] =  84.0;
        }
    else
        {
        csprm->min_ll [LNG] = CS_adj180 (csDef->ll_min [LNG] - csprm->cent_mer);
        csprm->min_ll [LAT] = csDef->ll_min [LAT];
        csprm->max_ll [LNG] = CS_adj180 (csDef->ll_max [LNG] - csprm->cent_mer);
        csprm->max_ll [LAT] = csDef->ll_max [LAT];
        }


    if (csDef->xy_min [XX] == 0.0 &&
        csDef->xy_max [XX] == 0.0)
        {
        /* No specification in the coordinate system definition.
           The setup is virtually complete, so we can use CStrmrsF
           to calculate some values, if necessary. The curved
           nature of the lat/long lines means we cannot just
           convert the lat/long min/max.

           Need to compute the min/max without the false origin
           first, it will be added back in by the quadMM
           processor. */

        test_ll [LNG] = CS_adj180 (csprm->cent_mer + csprm->max_ll [LNG]);
        test_ll [LAT] = trmBFParams->projecParori;
        TRMBF_XYFromLatLong (trmBFParams,test_xy,test_ll);
        csprm->max_xy [XX] = test_xy [XX] - trmBFParams->projecCtx;
        csprm->min_xy [XX] = -csprm->max_xy [XX];

        /* Origin latitude is not always the equator, need to
           do min and max separately. */

        test_ll [LNG] = csprm->cent_mer;
        test_ll [LAT] = -84.0;
        TRMBF_XYFromLatLong  (trmBFParams,test_xy, test_ll);
        csprm->min_xy [YY] = test_xy [YY] - trmBFParams->projecCty;
        test_ll [LAT] = 84.0;
        TRMBF_XYFromLatLong  (trmBFParams,test_xy,test_ll);
        csprm->max_xy [YY] = test_xy [YY] - trmBFParams->projecCty;

        /* Apply quad processing, e.g. a left handed coordinate
           system. */
        CS_quadMM (csprm->min_xy,csprm->max_xy,trmBFParams->projecCtx, trmBFParams->projecCty, trmBFParams->quad);
        }
    else
        {
        /* Use what ever the user has given us.  No adjustment necessary.  Note: we don't check anything. */
        csprm->min_xy [XX] = csDef->xy_min [XX];
        csprm->min_xy [YY] = csDef->xy_min [YY];
        csprm->max_xy [XX] = csDef->xy_max [XX];
        csprm->max_xy [YY] = csDef->xy_max [YY];
        }

    /* That's all the calculations we can do at this time.
       Set up the internal function calls.  Note, since the
       Transverse Mercator is a conformal projection, the
       h and k scale factors are the same.  Therefore, we
       set all three scale function pointers to the same
       function. */

    csprm->ll2cs = (cs_LL2CS_CAST)TRMBF_XYFromLatLong;
    csprm->cs2ll = (cs_CS2LL_CAST)TRMBF_LatLongFromXY;
    csprm->cs_scale = (cs_SCALE_CAST)TRMBF_GridScale;
    csprm->cs_sclk = (cs_SCALK_CAST)TRMBF_GridScale;
    csprm->cs_sclh = (cs_SCALH_CAST)TRMBF_GridScale;
    csprm->cs_cnvrg = (cs_CNVRG_CAST)TRMBF_Convergence;
    csprm->llchk    = (cs_LLCHK_CAST)TRMBF_LatLongCheck;
    csprm->xychk    = (cs_XYCHK_CAST)TRMBF_XYCheck;

    return;
    }

/**********************************************************************
**  (void) UTMBF_ZoneSetup (csprm);
**
**  struct cs_Csprm_ *csprm;    structure containing all coordinate
**                              system parameters.
**
**  This is adapted from the TOTAL source code. What was global data
**  in the TOTAL implementation is stored in the cs_TrmBF_ structure.
**********************************************************************/
void EXP_LVL9 UTMBF_ZoneSetup 
(
struct cs_Csprm_ *csprm
)
    {
    // first, get the information from the zone, calculate a temporary cs_Csdef_, then call _TRMBFSetup.
    // We have only two parameters, the Zone number and the hemisphere. We figure out everything else.  */
    extern double cs_Zero;          /* 0.0 */

    struct cs_Csdef_ tempCsDef = csprm->csdef;
    int     zoneNbr = (int) csprm->csdef.prj_prm1;

    // central meridian, degrees
    tempCsDef.prj_prm1 = (double)(-183 + (6 * zoneNbr));
    tempCsDef.org_lat  = cs_Zero;
    tempCsDef.x_off    = 500000.0 * csprm->csdef.scale;
    
    // hemisphere
    if (csprm->csdef.prj_prm2 >= 0.0) 
        tempCsDef.y_off = cs_Zero;
    else 
        tempCsDef.y_off = 10000000.0 * csprm->csdef.scale;

    tempCsDef.scl_red   = 0.9996;

    _TRMBFSetup (csprm, &tempCsDef);
    }

/**********************************************************************
**  (void) TRMBF_Setup (csprm);
**
**  struct cs_Csprm_ *csprm;    structure containing all coordinate
**                              system parameters.
**
**  This is adapted from the TOTAL source code. What was global data
**  in the TOTAL implementation is stored in the cs_TrmBF_ structure.
**********************************************************************/
void EXP_LVL9 TRMBF_Setup 
(
struct cs_Csprm_ *csprm
)
    {
    // set up directly from csprm.
    _TRMBFSetup (csprm, &csprm->csdef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static  double dsign
(
double a,
double b
)
  {
  return(((a <= 0.0 && b <= 0.0) || (a >= 0.0 && b >= 0.0)) ? a : -a);
  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* From the TOTAL source code with modifications for CSMap parameter passing and trmBFParams.
+---------------+---------------+---------------+---------------+---------------+------*/
int EXP_LVL9 TRMBF_XYFromLatLong 
(
Const struct cs_TrmBF_ *trmBFParams, 
double                  xy [2], 
Const double            ll [2]
)
    {
    extern double cs_Degree;        /* 1.0 / 57.29577... */
    extern double cs_Pi_o_2;        /* PI / 2 */
    extern double cs_Pi;            /* PI */
    extern double cs_Zero;          /* 0.0 */

    double  lambda  = ll [0] * cs_Degree;
    double  phi     = ll [1] * cs_Degree;
    int     rtn_val = cs_CNVRT_NRML;

    double  beta, sinl, cosl, cosl2, gn, slam, xi, chi, x, y, lam;

    /*  Preliminary computations */

    /*  Calculation of the arc at the BETA meridian */

    beta = trmBFParams->Utmcf0 * phi - trmBFParams->Utmcf2 * sin(2.0 * phi) + trmBFParams->Utmcf4 * sin (4.0 * phi) - trmBFParams->Utmcf6 * sin(6.0 * phi);
 
    /*  Case at right angle (PI/2) */
    if( cs_Pi_o_2 - fabs(phi) < 0.0000000001)
        phi = dsign ( ( cs_Pi_o_2 - 0.0000000001), phi);

    /* Case longitude is +/- PI/2 from central meridian  */

#ifdef GEOCOORD_ENHANCEMENT
    /* Modified from the original Bernard Flaceliere code since the function may be called
    with a lambda adjusted because it passed over the +180 or -180 longitude limit.
    To detect these cases we check if the meridian of the projection are of the same signs
    If they are not then we will adjust the lambda by adding or subtraction 2 time pi so 
    both values are in the same rotation */
    lam = CS_adj2pi(lambda - trmBFParams->projecMercen);

    /* Modified from the original Bernard Flaceliere code since the tolerance is too small
       and results in infinity or undefined values below */
    if (lam - cs_Pi_o_2 > -0.000001)
        {
        lam = cs_Pi_o_2 -0.000001;
        rtn_val = cs_CNVRT_DOMN;
        }
    else if (lam + cs_Pi_o_2 < 0.000001)
        {
        lam = -cs_Pi_o_2 + 0.000001;
        rtn_val = cs_CNVRT_DOMN;
        }
#else
    lam = lambda - trmBFParams->projecMercen;

    if (lam - cs_Pi_o_2 > -0.0000000001)
        lam = cs_Pi_o_2 -0.0000000001;
    else if (lam + cs_Pi_o_2< 0.0000000001)
        lam = -cs_Pi_o_2 + 0.0000000001;
#endif

    /* Precomputation of factors required many times  */
    sinl    = sin(phi);
    cosl    = cos(phi);
    cosl2   = cosl * cosl;
    gn      = trmBFParams->ellipsGax / sqrt (1.0 - trmBFParams->ellipsExc * sinl * sinl);
    slam    = sin(lam);

    /*  Calculation of XI and CHI  */
    xi      = 0.5 * log((1.0 + cosl * slam) / (1.0 - cosl * slam));
    chi     = atan (sinl / cosl / cos (lam)) - phi;


    /*  Calculation X and Y */
    x       = gn * xi * (1.0 + trmBFParams->eprim_sq * cosl2 * (xi * xi - 3.0 * chi * chi) / 6.0);
    y       = gn * chi * (1.0 + trmBFParams->eprim_sq * cosl2 * (3.0 * xi * xi - chi * chi) / 6.0);
 

    /*  Calculation X and Y */
    xy[0] = trmBFParams->projecCtx + trmBFParams->projecFech * x;
    xy[1] = trmBFParams->projecCty + beta - trmBFParams->Utmbta + trmBFParams->projecFech * y;

    return (rtn_val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* From the TOTAL source code with modifications for CSMap parameter passing and trmBFParams.
+---------------+---------------+---------------+---------------+---------------+------*/
int EXP_LVL9 TRMBF_LatLongFromXY 
(
Const struct cs_TrmBF_ *trmBFParams,
double                  ll [2],
Const double            xy [2]
)
    {
    extern double cs_Pi_o_2;        /* PI / 2 */
    extern double cs_Zero;          /* 0.0 */
    extern double cs_Radian;        /* 57.29577... */

    double  beta,sinl,cosl,v2;
    double  phi0,phip,yp,psi;
    double  cosl2,gn,xia,chia;    
    double  xi=cs_Zero;
    double  chi=cs_Zero;
    double  x;
    double  y;
    double  tol = 0.0000001;
    int     rtn_val = cs_CNVRT_NRML;
    double  outLambda;
    double  outPhi;

#define MAX_TRMERBF_ITER1 50
#define MAX_TRMERBF_ITER2 50

    /*  Preliminary computations */
 
    /*  Calculation of E-squared, X and YP */
    x  = (xy[0] - trmBFParams->projecCtx) / trmBFParams->projecFech;
    yp = (xy[1] - trmBFParams->projecCty + trmBFParams->Utmbta) / trmBFParams->projecFech;
 
 	/* Deal with the limiting xx case.  */
	if (fabs (x) > trmBFParams->xx_max)
	{
		rtn_val = cs_CNVRT_RNG;
		x = (x >= 0.0) ? trmBFParams->xx_max : -trmBFParams->xx_max;
	}
	
    /*  Computation of approximate latitude */
    phi0 = yp / trmBFParams->ellipsGax;

    if (fabs(phi0) > cs_Pi_o_2)
        phi0 = dsign(cs_Pi_o_2 - 0.0000000001,phi0);
 
    /* Calculation of arc of meridian beta for approximate latitude */
    int iter2 = 0;
    for (;;)
        {
        if (iter2 > MAX_TRMERBF_ITER2)
            break;

        iter2++;

        beta = trmBFParams->Utmcf0 * phi0 - trmBFParams->Utmcf2 * sin(2.0 * phi0) + trmBFParams->Utmcf4 * sin(4.0 * phi0) - trmBFParams->Utmcf6 * sin(6.0 * phi0);

        /* Calculation of Y */
        y = yp - beta / trmBFParams->projecFech;

        /* Precompute values used many times */
        sinl = sin (phi0);
        cosl = cos (phi0);
        cosl2 = cosl * cosl;
        gn = trmBFParams->ellipsGax / sqrt (1.0 - trmBFParams->ellipsExc * sinl * sinl);
        v2 = 1.0 + trmBFParams->eprim_sq * cosl2;

        /* Calculation of XI and CHI */
        xia  = x/gn;
        chia = y/gn;

        int iter1 = 0;
        for (;;)
            {
            if (iter1 > MAX_TRMERBF_ITER1)
                break;

            iter1++;

            xi = x / gn / (1.0 + trmBFParams->eprim_sq * cosl2 * (xia * xia - 3.0 * chia * chia) / 6.0);
            chi = y / gn / (1.0 + trmBFParams->eprim_sq * cosl2 * (3.0 * xia * xia - chia * chia) / 6.0);

            if (fabs(xi-xia) > 0.0000000001 || fabs(chi - chia) > 0.0000000001)
                {
                xia = xi;
                chia = chi;
                }
            else 
                break;
            }

        /* Calculation of latitude and longitude */
        phip = phi0 + chi;
        if (fabs(phip) > cs_Pi_o_2)
            phip = dsign(cs_Pi_o_2 -0.0000000001,phip);

        outLambda = trmBFParams->projecMercen + atan(sinh(xi) / cos(phip));
        psi = atan(cos((outLambda) - trmBFParams->projecMercen) * tan(phip));
        outPhi = phi0 + v2 * (psi - phi0) - 1.5 * v2 * trmBFParams->eprim_sq * sinl * cosl * (psi - phi0) * (psi - phi0);

        if ( fabs(outPhi - phi0) <= tol) 
            break;
        phi0 = outPhi;
        }

    // CS_Map expects degrees.
    ll [LNG] = outLambda * cs_Radian;
    ll [LAT] = outPhi * cs_Radian;
    return (rtn_val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* returns the scale at the give point.
+---------------+---------------+---------------+---------------+---------------+------*/
double EXP_LVL9 TRMBF_GridScale 
(
Const struct cs_TrmBF_ *trmBFParams,
Const double            ll [2]
)
    {
    extern double cs_Degree;            /* 1.0 / 57.29577... */

    // no doubt it is possible to backtrack through the calculations above and come up with a much more efficient approach to
    // calculating k at a given ll, but rather than do that, I use the equation in "Map Projections, A Working Manual" by Snyder, p. 63
    // that gives k in terms of x and phi (latitude). That requires us to calulate xy from ll, and back out the false easting. That
    // calculation is in fact more expensive than the proper calculation of k, but I don't have enough info about the TOTAL derivation
    // to do it right. Plus, I don't think this function gets called much (if at all) in our system.

    double  phi     = ll[LNG] * cs_Degree;
    double  sinPhi  = sin (phi);
    double  cosPhi  = cos (phi);
    double  N       = trmBFParams->ellipsGax / sqrt (1.0 - (trmBFParams->ellipsExc * sinPhi * sinPhi));
    double  xy[2];
    double  xUnscaled;
    double  k;

    TRMBF_XYFromLatLong (trmBFParams, xy, ll);
    xUnscaled = (xy[0] - trmBFParams->projecCtx) / trmBFParams->projecFech;

    k = trmBFParams->projecFech * (1.0 + ((1.0 + trmBFParams->eprim_sq * cosPhi * cosPhi) * (xUnscaled * xUnscaled) / (2.0 * N * N)));
    return k;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double EXP_LVL9 TRMBF_Convergence 
(
Const struct cs_TrmBF_ *trmBFParams, 
Const double            ll [2]
)
    {
    // this is unmodified from csTrmrsC. It doesn't seem to depend on the parameters much, so I assume it works OK.

    extern double cs_Degree;            /* 1.0 / 57.29577... */
    extern double cs_Radian;            /* 57.29577... */
    extern double cs_Zero;              /* 0.0 */
    extern double cs_One;               /* 1.0 */
    extern double cs_Two;               /* 2.0 */
    extern double cs_Three;             /* 3.0 */
    extern double cs_K15;               /* 15.0 */
    extern double cs_Huge;              /* An approximation of infinity which is small enough to calculate with without producing FP faults. */
    extern double cs_NPTest;            /* 0.001 seconds of arc short of the north pole in radians. */
    extern double cs_SPTest;            /* 0.001 seconds of arc short of the south pole in radians. */
    extern double cs_EETest;            /* 0.001 seconds of arc short of +90.00, in radians. */
    extern double cs_WETest;            /* 0.001 seconds of arc short of -90.00, in radians. */

    double alpha;
    double lng;
    double lat;

    double sin_lat;
    double cos_lat;
    double cos_lat2;
    double tan_lat;
    double tan_lat2;
    double del_lng;
    double del_lng2;

    double tmp1;
    double tmp2;

    lng = ll [LNG] * cs_Degree;
    lat = ll [LAT] * cs_Degree;
    del_lng = CS_adj2pi (lng - trmBFParams->projecMercen);
    if (del_lng < cs_WETest) del_lng = cs_WETest;
    if (del_lng > cs_EETest) del_lng = cs_EETest;
    del_lng2 = del_lng * del_lng;

    if (lat > cs_NPTest)
    {
        tan_lat = cs_Huge;
        cos_lat = cs_Zero;
        sin_lat = cs_One;
    }
    else if (lat < cs_SPTest)
    {
        tan_lat = -cs_Huge;
        cos_lat = cs_Zero;
        sin_lat = -cs_One;
    }
    else
    {
        sin_lat = sin (lat);
        cos_lat = cos (lat);
        tan_lat = tan (lat);
    }
    cos_lat2 = cos_lat * cos_lat;
    tan_lat2 = tan_lat * tan_lat;

    if (trmBFParams->ellipsRacexc == 0.0)
        {
        /* Here for a sphere. */

        tmp1 = del_lng2 * cos_lat2 / cs_Three;
        tmp2 = (del_lng2 * del_lng2) * (cos_lat2 * cos_lat2) *
                (cs_Two - tan_lat2) / cs_K15;
        alpha = del_lng * sin_lat * (cs_One + tmp1 + tmp2);
        }
    else
        {
        tmp1 = cs_One + (cs_Three * trmBFParams->eprim_sq * cos_lat2);
        tmp1 = del_lng2 * cos_lat2 * tmp1 / cs_Three;
        tmp2 = (del_lng2 * del_lng2) * (cos_lat2 * cos_lat2) *
                (cs_Two - tan_lat2) / cs_K15;
        alpha = del_lng * sin_lat * (cs_One + tmp1 + tmp2);
        }
    return (alpha * cs_Radian);
}


/**********************************************************************
    The following function is static, not visible outside
    this source module, so the name may not adhere to the
    normal naming conventtion.

    The function verifies that the point supplied to it
    is indeed within the geographic domain of the coordinate
    system.
**********************************************************************/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int _LatLongCheck 
(
Const struct cs_TrmBF_ *trmBFParams,
Const double            ll [3]
)
    {
    extern double cs_Degree;        /* 1.0 / 57.2... */
    extern double cs_Pi_o_2;        /* PI / 2.0 */
    extern double cs_AnglTest;      /* 0.001 seconds of arc in
                                       radians */
    double del_lng;

    double my_lng, my_lat;

    my_lng =       ll [LNG] * cs_Degree;
    my_lat = fabs (ll [LAT] * cs_Degree);

    if (my_lat > cs_Pi_o_2) return (cs_CNVRT_DOMN);

    del_lng = fabs (CS_adj2pi (my_lng - trmBFParams->projecMercen));
    if (my_lat < cs_AnglTest && fabs (del_lng - cs_Pi_o_2) < cs_AnglTest)
        return (cs_CNVRT_DOMN);

    return (cs_CNVRT_OK);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int EXP_LVL9 TRMBF_LatLongCheck 
(
Const struct cs_TrmBF_ *trmBFParams,
int                     cnt,
Const double            pnts [][3]
)
    {
    extern double cs_Degree;
    extern double cs_EETest;            /* .001 seconds of arc less
                                           than PI/2 in radians. */
    extern double cs_WETest;        /* -cs_EETest */

    int ii;
    int status;

    double tmp;
    double del_lng;


    status = cs_CNVRT_OK;
    for (ii = 0;ii < cnt && status == cs_CNVRT_OK;ii++)
        {
        status = _LatLongCheck (trmBFParams, pnts [ii]);
        }
    if (cnt <= 1 || status != cs_CNVRT_OK) 
        return (status);

    /* If cnt is 2, we have a line which we must check. */

    if (cnt == 2)
        {

        for (ii = 0;ii < cnt && status == cs_CNVRT_OK;ii++)
            {
            tmp = pnts [ii][LNG] * cs_Degree;
            del_lng = CS_adj2pi (tmp - trmBFParams->projecMercen);
            if (del_lng > cs_EETest || del_lng < cs_WETest)
                status = cs_CNVRT_DOMN;
            }
        }
    else if (cnt == 3)
        {
        /* A three point list means the region is not closed. */
        CS_erpt (cs_RGN_PNTCNT);
        return (cs_CNVRT_ERR);
        }
    else
        {

        for (ii = 0;ii < cnt && status == cs_CNVRT_OK;ii++)
            {
            tmp = pnts [ii][LNG] * cs_Degree;
            del_lng = CS_adj2pi (tmp - trmBFParams->projecMercen);
            if (del_lng > cs_EETest || del_lng < cs_WETest)
                {
                status = cs_CNVRT_DOMN;
                }
            }
        }
    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int EXP_LVL9 TRMBF_XYCheck
(
Const struct cs_TrmBF_ *trmBFParams,
int                     cnt,
Const double            pnts [][3]
)
    {
    int ii;
    int rtn_val;

    double test_val;
    double dummy;

    rtn_val = cs_CNVRT_OK;

    for (ii = 0;ii < cnt && rtn_val == cs_CNVRT_OK;ii++)
        {
        if (trmBFParams->quad == 0)
            {
            test_val = pnts [ii][XX] - trmBFParams->projecCtx;
            }
        else
            {
            CS_quadI (&test_val,&dummy,pnts [ii], trmBFParams->projecCtx, trmBFParams->projecCty, trmBFParams->quad);
            }

        if (fabs (test_val) > trmBFParams->xx_max)
            {
            rtn_val = cs_CNVRT_DOMN;
            }
        }
    return (rtn_val);
    }

