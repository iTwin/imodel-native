//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

/*----------------------------------------------------------------------------*/
/* constants.                                                                 */
/*----------------------------------------------------------------------------*/
#define PI12 M_PI_2
#define PI2  (2.0 * M_PI)

#define AEC_C_TOL        1e-6
#define AEC_C_TOLSQR     1e-12
#define AEC_C_TOL1       1e-8
#define AEC_C_TOL1SQR    1e-16
#define AEC_C_TOL2       1e-10
#define AEC_C_TOL3       1e-3
#define AEC_C_TOL3SQR    1e-6

#define AEC_C_MAXDBL     1e308
#define AEC_C_MINDBL     (-1e308)

/*----------------------------------------------------------------------------*/
/* macros.                                                                    */
/*----------------------------------------------------------------------------*/
#ifndef ISSMALL
#define ISSMALL( a, tol ) ( ( (a) <= (tol) ) && ( (a) >= -(tol) )  ? 1 : 0 )
#endif

#define EQUAL1( a, b, tol ) ( ISSMALL( ( (a) - (b) ), (tol) ) ? 1 : 0 )
#define EQUAL2( u, v, tol ) ( ( EQUAL1( u[0], v[0], tol ) && EQUAL1( u[1], v[1], tol ) ) ? 1 : 0 )
#define DP2EQUAL( p1, p2, tol ) ( ( EQUAL1( (p1).x, (p2).x, tol ) && EQUAL1( (p1).y, (p2).y, tol ) ) ? 1 : 0 )
#define DP3EQUAL( p1, p2, tol ) ( ( DP2EQUAL( p1, p2, tol ) && EQUAL1( (p1).z, (p2).z, tol ) ) ? 1 : 0 )

#define VIDENTITY(a)     { (a).x = 0.; (a).y = 0.; (a).z = 0.; }
#define VSCALE(a,b,c)    { (c).x=(a).x*(b);   (c).y=(a).y*(b);	 (c).z=(a).z*(b); }
#define VSCALEXY(a,b,c)  { (c).x=(a).x*(b);   (c).y=(a).y*(b); }
#define VCROSS(a,b,c)    { DPoint3d zzz; (zzz).x=(a).y*(b).z-(a).z*(b).y; (zzz).y=(a).z*(b).x-(a).x*(b).z; (zzz).z=(a).x*(b).y-(a).y*(b).x; (c) = zzz; }
#define VCROSSXY(a,b,c)  { (c).z=(a).x*(b).y-(a).y*(b).x; }
#define VADD(a,b,c)      { (c).x=(a).x+(b).x; (c).y=(a).y+(b).y; (c).z=(a).z+(b).z; }
#define VADDXY(a,b,c)    { (c).x=(a).x+(b).x; (c).y=(a).y+(b).y; }
#define VSUB(a,b,c)      { (c).x=(a).x-(b).x; (c).y=(a).y-(b).y; (c).z=(a).z-(b).z; }
#define VSUBXY(a,b,c)    { (c).x=(a).x-(b).x; (c).y=(a).y-(b).y; }
#define VNEG(a,b)        { (b).x=(-1.)*(a).x; (b).y=(-1.)*(a).y; (b).z=(-1.)*(a).z; }
#define VDOT(a,b)        ( (a).x*(b).x + (a).y*(b).y + (a).z*(b).z )
#define VDOTXY(a,b)      ( (a).x*(b).x + (a).y*(b).y )
#define VLEN(a)          ( sqrt((a).x*(a).x+(a).y*(a).y+(a).z*(a).z) )
#define VLENXY(a)        ( sqrt((a).x*(a).x+(a).y*(a).y) )
#define VEQUAL(a,b,t)    DP3EQUAL( a, b, t )
#define VEQUALXY(a,b,t)  DP2EQUAL( a, b, t )
#define VINSIDEXY(a,b,c,t) ((( (c).x-(a).x>=-t  &&  (c).x-(b).x<=t ) || ( (c).x-(a).x<=t  &&  (c).x-(b).x>=-t ) )  &&  \
			   ( ( (c).y-(a).y>=-t  &&  (c).y-(b).y<=t ) || ( (c).y-(a).y<=t  &&  (c).y-(b).y>=-t ) ) )
#define VCENTROIDXY(a,b,c,d) { (d).x=((a).x+(b).x+(c).x)/3.;  (d).y=((a).y+(b).y+(c).y)/3.;  }

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#define AECMIN(x,y)      MIN(x,y)
#define AECMAX(x,y)      MAX(x,y)
