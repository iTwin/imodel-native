//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* plyuti.c                                            tmi    30-Dec-1992     */
/*----------------------------------------------------------------------------*/
/* Various polyline and polygon utilities.                                    */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include <assert.h>
#include <plyfnc.h>
#include <vecfnc.h>
#include <intsct.h>

//---------------------------------------------------------------------------
// Macros and Constants
//---------------------------------------------------------------------------
#define POLYBUF_REALLOC_SIZE 32

#define YZ_SWAP(pt)  {double tmp = pt.y; pt.y = pt.z; pt.z = tmp;}
#define XZ_SWAP(pt)  {double tmp = pt.x; pt.x = pt.z; pt.z = tmp;}


/*----------------------------------------------------------------------------*/
/* Private data types                                                         */
/*----------------------------------------------------------------------------*/
typedef struct
    {
    double totlLength;
    DPoint3d *vtsP;
    long nVts;
    long closed;
    } AECPointList;

typedef struct
    {
    long     nPolygons;
    DPoint3d **ppPolygons;
    long     * npPolygons;
    } AECConvexAccumInfo;

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int  aecPolygon_sendConvexPolygonsVerify(DPoint3d *,DPoint3d *,DPoint3d *,long,DPoint3d *);
static int  intersectFunc(void*,void*,void*);
//static int convexAccumFunc ( long numPoints, DPoint3d *points, void *userData );
static int aecPolygon_clipLinestringClip(void *,DPoint3d *,DPoint3d *);
static int aecPolygon_clipLinestringSegment(void *,int,long,DPoint3d *);
static int aecPolygon_areLineSegmentsColinear( DPoint3d *, DPoint3d *, DPoint3d *, DPoint3d * );


/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_removeLoops
DESC:
HIST: Original - dgc
MISC:
KEYW: POLYGON LOOPS REMOVE
-----------------------------------------------------------------------------%*/

int aecPolygon_removeLoops
    (
    long nVts,                           /* => # points in input polygon        */
    DPoint3d *vtsP                        /* => input polygon coordinates        */
    )
    {
    double xlo, xhi, ylo, yhi;
    AECPointList pntList;
    long *indexP = NULL;
    long i, nIndex = 0;
    void *intObjP = NULL;
    int sts = SUCCESS;

    pntList.vtsP = vtsP;
    pntList.nVts = nVts;
    aecPolygon_horizontalLength( &pntList.totlLength, nVts, vtsP );
    pntList.closed = VEQUAL ( vtsP[0], vtsP[nVts-1], AEC_C_TOL3 );

    for( i = 0; sts == SUCCESS && i < ( nVts - 1 ); i++ )
        sts = aecTable_insert( (void**)&indexP, (int*)&nIndex, &i, sizeof( i ) );

    if( sts == SUCCESS )
        aecIntersect_begin( &intObjP, intersectFunc, NULL, &pntList, nVts );

    for( i = 0; sts == SUCCESS && i < ( nVts - 1 ); i++ )
        {
        xlo = vtsP[i].x < vtsP[i+1].x ? vtsP[i].x : vtsP[i+1].x;
        ylo = vtsP[i].y < vtsP[i+1].y ? vtsP[i].y : vtsP[i+1].y;
        xhi = vtsP[i].x > vtsP[i+1].x ? vtsP[i].x : vtsP[i+1].x;
        yhi = vtsP[i].y > vtsP[i+1].y ? vtsP[i].y : vtsP[i+1].y;
        sts = aecIntersect_insert( intObjP, &indexP[i], xlo, ylo, xhi, yhi );
        }

    if( sts == SUCCESS )
        aecIntersect_end( intObjP );

    if( indexP ) free( indexP );

    return( sts );
    }




static int intersectFunc
    (
    void *tmp1P,
    void *tmp2P,
    void *tmp3P
    )
    {
    long i = *( long * )tmp1P;
    long j = *( long * )tmp2P;
    AECPointList *datP = ( AECPointList * )tmp3P;
    DPoint3d *vtsP = datP->vtsP;
    long nVts = datP->nVts;
    long closed = datP->closed;
    double totlLength = datP->totlLength;
    long min = i < j ? i : j;
    long max = i > j ? i : j;

    if( ( max - min ) < 2 )
        return( SUCCESS );

    if( aecVec_doLinesIntersect( &datP->vtsP[i], &datP->vtsP[i+1], &datP->vtsP[j], &datP->vtsP[j+1] ) )
        {
        DPoint3d pnt;
        double loopLength;
        long k;

        aecPolygon_intersect( &pnt, NULL, NULL, &vtsP[min], &vtsP[min+1], &vtsP[max], &vtsP[max+1] );

        aecPolygon_horizontalLength( &loopLength, max - min + 1, &vtsP[min] );

        /* keep smaller loop if dealing with closed list */
        if( !closed || ( loopLength < totlLength - loopLength ) )
            {
            for( k = min + 1; k < max + 1; k++ )
                vtsP[k] = pnt;
            }
        else
            {
            for( k = 0; k < min + 1; k++ )
                vtsP[k] = pnt;
            for( k = max + 1; k < nVts; k++ )
                vtsP[k] = pnt;
            }
        }

    return( SUCCESS );
    }


/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_planarArea
DESC: It computes the planar area of the input polygon.  The first and
points do not need to be the same.
HIST: Original - tmi 05-Mar-1991
MISC:
KEYW: POLYGON AREA PLANAR
-----------------------------------------------------------------------------%*/

void aecPolygon_planarArea
    (
    double *areaP,                       /* <= computed area                    */
    long nvrt,                           /* => # vertices in poly.              */
    DPoint3d *vrtP                       /* => polygon coordinates              */
    )
    {
    long i;

    *areaP = 0.;

    if ( nvrt > 2 )
        {
        for ( i = 1; i < nvrt; i++ )
            *areaP += vrtP[i-1].x * vrtP[i].y - vrtP[i-1].y * vrtP[i].x;

        if ( !VEQUALXY ( vrtP[0], vrtP[nvrt-1], 0. ) )
            *areaP += vrtP[nvrt-1].x * vrtP[0].y - vrtP[nvrt-1].y * vrtP[0].x;

        *areaP = 0.5 * fabs(*areaP);
        }
    }



/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_reverseLongArray
DESC: Reverse the order of an array of longs.
HIST: Original - tmi 01-Aug-1991
MISC:
KEYW: POLYGON POLYLINE ARRAY LONG REVERSE
-----------------------------------------------------------------------------%*/

void aecPolygon_reverseLongArray
    (
    long nvrt,                           /* => # vertices in poly.              */
    long *vrtP                           /* => polygon coordinates              */
    )
    {
    long i, half, tmp;

    half = nvrt / 2;
    nvrt -= 1;

    for ( i = 0; i < half; i++ )
        {
        tmp = vrtP[i];
        vrtP[i] = vrtP[nvrt-i];
        vrtP[nvrt-i] = tmp;
        }
    }



/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_reverse
DESC: It reverse the points in an array.
HIST: Original - tmi 28-Dec-1991
MISC:
KEYW: POLYGON POLYLINE ARRAY DPoint3d REVERSE
-----------------------------------------------------------------------------%*/

void aecPolygon_reverse
    (
    size_t nvrt,                           /* => # vertices in poly.              */
    DPoint3d* vrtP                       /* => polygon coordinates              */
    )
    {
    size_t half = nvrt / 2;

    for( size_t i = 0; i < half; i++ )
        {
        DPoint3d tmp = vrtP[ i ];
        vrtP[i] = vrtP[ nvrt - 1 - i ];
        vrtP[ nvrt - 1 - i ] = tmp;
        }
    }


/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_horizontalLength
DESC: Computes the length of the segments in an array (XY only).
HIST: Original - tmi 11-Jan-1992
MISC:
KEYW: POLYGON POLYLINE LENGTH HORIZONTAL
-----------------------------------------------------------------------------%*/

void aecPolygon_horizontalLength
    (
    double *lenP,                        /* <= computed hor. length             */
    long nvrt,                           /* => # vertices in poly.              */
    DPoint3d *vrtP                       /* => polygon coordinates              */
    )
    {
    DPoint3d tmp;
    long i;

    for ( *lenP = 0., i = 1; i < nvrt; i++ )
        {
        VSUBXY ( vrtP[i], vrtP[i-1], tmp );
        *lenP += VLENXY ( tmp );
        }
    }


/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_removeDuplicatePoints
DESC: It remove duplicate points in the input line string.
HIST: Original - tmi 03-Oct-1992
MISC:
KEYW: POLYGIN POLYLINE POINTS DUPLICATE REMOVE
-----------------------------------------------------------------------------%*/

void aecPolygon_removeDuplicatePoints
    (
    long *nvrtP,                         /* <=> # of vertices in line.          */
    DPoint3d *vrtP,                      /* <=> linestring vertices             */
    double tolerance                     /*  => tolerance to use                */
    )
    {
    int i, j = *nvrtP;

    if( *nvrtP > 1 )
        {
        for( i = 1, j = 1; i < *nvrtP; i++ )
            {
            if( !VEQUAL( vrtP[i-1], vrtP[i], tolerance ) )
                {
                vrtP[j] = vrtP[i];
                ++j;
                }
            }
        }

    *nvrtP = j;
    }

/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_intersect
DESC: Call this one to intersect two lines.
HIST: Original - tmi 07-Jan-1993
MISC:
KEYW: POLYLINE INTERSECT
-----------------------------------------------------------------------------%*/

void aecPolygon_intersect
    (
    DPoint3d *pnt,                       /* <= computed point                   */
    int *par,                            /* <= 1: parallel, 0: not              */
    int *ins,                            /* <= 1: inside, 0: don't              */
    DPoint3d *a,                         /* => line 1 end 1                     */
    DPoint3d *b,                         /* => line 1 end 2                     */
    DPoint3d *c,                         /* => line 2 end 1                     */
    DPoint3d *d                          /* => line 2 end 2                     */
    )
    {
    DPoint3d a0, b0, c0, d0;

    if ( par ) *par = 0;
    if ( ins ) *ins = 0;

    a0 = *a;
    b0 = *b;
    c0 = *c;
    d0 = *d;

    a0.z = b0.z = c0.z = d0.z = 0.;

    if ( VEQUAL ( a0, c0, AEC_C_TOL3 ) || VEQUAL ( b0, c0, AEC_C_TOL3 ) )
        {
        *pnt = *c;
        if( par && aecPolygon_colinear( &a0, &b0, &c0, &d0 ) )
            *par = 1;
        if( ins ) 
            *ins = 1;
        }
    else if ( VEQUAL ( a0, d0, AEC_C_TOL3 ) || VEQUAL ( b0, d0, AEC_C_TOL3 ) )
        {
        *pnt = *d;
        if( par && aecPolygon_colinear( &a0, &b0, &c0, &d0 ) )
            *par = 1;
        if( ins ) 
            *ins = 1;
        }
    else
        {
        DPoint3d vec, nrm;
        double t1, t2;
        double mag;

        VSUB ( d0, c0, vec );
        nrm = vec;
        nrm.z = ( fabs(nrm.x) + fabs(nrm.y) ) / 2.0;
        VCROSS ( nrm, vec, nrm );
        mag = VLEN(nrm);
        if (mag == 0.0)
            mag = 1.0;
        nrm.x /= mag;
        nrm.y /= mag;
        nrm.z /= mag;

        VSUB ( a0, c0, vec );
        t1 = VDOT ( nrm, vec );

        VSUB ( b0, c0, vec );
        t2 = VDOT ( nrm, vec );

        if ( EQUAL1( t1, t2, AEC_C_TOL ) )
            {
            if ( par ) *par = 1;
            if ( ins ) *ins = 0;

            // This will give intersection points that are not on the overlap - need to check each combination - DJS 10/26/09
            pnt->x = 0.5 * ( a->x + c->x );
            pnt->y = 0.5 * ( a->y + c->y );
            pnt->z = 0.5 * ( a->z + c->z );

            if ( (pnt->x < a0.x && pnt->x < b0.x) || (pnt->x > a0.x && pnt->x > b0.x) || (pnt->y < a0.y && pnt->y < b0.y) || (pnt->y > a0.y && pnt->y > b0.y) ||
                (pnt->x < c0.x && pnt->x < d0.x) || (pnt->x > c0.x && pnt->x > d0.x) || (pnt->y < c0.y && pnt->y < d0.y) || (pnt->y > c0.y && pnt->y > d0.y)
                )
                {
                pnt->x = 0.5 * ( b->x + c->x );
                pnt->y = 0.5 * ( b->y + c->y );
                pnt->z = 0.5 * ( b->z + c->z );

                if ( (pnt->x < a0.x && pnt->x < b0.x) || (pnt->x > a0.x && pnt->x > b0.x) || (pnt->y < a0.y && pnt->y < b0.y) || (pnt->y > a0.y && pnt->y > b0.y) ||
                    (pnt->x < c0.x && pnt->x < d0.x) || (pnt->x > c0.x && pnt->x > d0.x) || (pnt->y < c0.y && pnt->y < d0.y) || (pnt->y > c0.y && pnt->y > d0.y)
                    )
                    {
                    pnt->x = 0.5 * ( a->x + d->x );
                    pnt->y = 0.5 * ( a->y + d->y );
                    pnt->z = 0.5 * ( a->z + d->z );

                    if ( (pnt->x < a0.x && pnt->x < b0.x) || (pnt->x > a0.x && pnt->x > b0.x) || (pnt->y < a0.y && pnt->y < b0.y) || (pnt->y > a0.y && pnt->y > b0.y) ||
                        (pnt->x < c0.x && pnt->x < d0.x) || (pnt->x > c0.x && pnt->x > d0.x) || (pnt->y < c0.y && pnt->y < d0.y) || (pnt->y > c0.y && pnt->y > d0.y)
                        )
                        {
                        pnt->x = 0.5 * ( b->x + d->x );
                        pnt->y = 0.5 * ( b->y + d->y );
                        pnt->z = 0.5 * ( b->z + d->z );

                        if ( (pnt->x < a0.x && pnt->x < b0.x) || (pnt->x > a0.x && pnt->x > b0.x) || (pnt->y < a0.y && pnt->y < b0.y) || (pnt->y > a0.y && pnt->y > b0.y) ||
                            (pnt->x < c0.x && pnt->x < d0.x) || (pnt->x > c0.x && pnt->x > d0.x) || (pnt->y < c0.y && pnt->y < d0.y) || (pnt->y > c0.y && pnt->y > d0.y)
                            )
                            {
                            pnt->x = 0.5 * ( a->x + b->x );
                            pnt->y = 0.5 * ( a->y + b->y );
                            pnt->z = 0.5 * ( a->z + b->z );

                            if ( (pnt->x < a0.x && pnt->x < b0.x) || (pnt->x > a0.x && pnt->x > b0.x) || (pnt->y < a0.y && pnt->y < b0.y) || (pnt->y > a0.y && pnt->y > b0.y) ||
                                (pnt->x < c0.x && pnt->x < d0.x) || (pnt->x > c0.x && pnt->x > d0.x) || (pnt->y < c0.y && pnt->y < d0.y) || (pnt->y > c0.y && pnt->y > d0.y)
                                )
                                {
                                pnt->x = 0.5 * ( c->x + d->x );
                                pnt->y = 0.5 * ( c->y + d->y );
                                pnt->z = 0.5 * ( c->z + d->z );
                                }
                            }
                        }
                    }
                }

            if( ins && aecPolygon_colinear( &a0, &b0, &c0, &d0 ) )
                *ins = VINSIDEXY( c0, d0, a0, AEC_C_TOL ) ||
                VINSIDEXY( c0, d0, b0, AEC_C_TOL );
            }
        else
            {
            double t = t1 / ( t1 - t2 );

            VSUB ( b[0], a[0], vec );
            VSCALE ( vec, t, vec );
            VADD ( a[0], vec, *pnt );

            if ( par )
                *par = 0;

            if ( ins )
                *ins = VINSIDEXY( a0, b0, *pnt, AEC_C_TOL ) &&
                VINSIDEXY( c0, d0, *pnt, AEC_C_TOL );
            }
        }
    }



/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_removeColinearPoints
DESC: Removes all colinear points from the input polygon.  Only the XY-
plane is considered.
HIST: Original - tmi 14-Jan-1993
MISC:
KEYW: POLYGON POLYLINE POINTS COLINEAR REMOVE
-----------------------------------------------------------------------------%*/

void aecPolygon_removeColinearPoints
    (
    long *nvrtP,                         /* <=> # points in polygon             */
    DPoint3d *vrtP                       /* <=> polygon points.                 */
    )
    {
    DPoint3d a, b, c;
    long i;

    for ( i = 1; i < *nvrtP - 1; i++ )
        {
        VSUBXY ( vrtP[i-1], vrtP[i], a );
        VSUBXY ( vrtP[i+1], vrtP[i], b );
        VCROSSXY ( a, b, c );
        if ( fabs(c.z) < AEC_C_TOL )
            {
            memcpy ( &vrtP[i], &vrtP[i+1], (*nvrtP-i-1)*sizeof(DPoint3d) );
            i -= 1;
            *nvrtP -= 1;
            }
        }
    }

/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_colinear
DESC: Determines if the two input lines are colinear. Only the XY-plane
is considered.  This function returns TRUE if they are colinear.
HIST: Original - tmi 14-Jan-1993
MISC:
KEYW: POLYLINE COLINEAR
-----------------------------------------------------------------------------%*/

int aecPolygon_colinear
    (
    DPoint3d* a,                         /* => line 1 end 1                     */
    DPoint3d* b,                         /* => line 1 end 2                     */
    DPoint3d* c,                         /* => line 2 end 1                     */
    DPoint3d* d                          /* => line 2 end 2                     */
    )
    {
    DPoint3d u, v, w, x;
    VSUBXY( a[0], b[0], u );
    VSUBXY( d[0], b[0], v );
    VCROSSXY( u, v, w );
    VSUBXY( a[0], c[0], u );
    VSUBXY( d[0], c[0], v );
    VCROSSXY( u, v, x );
    return ( fabs( w.z ) < AEC_C_TOL && fabs( x.z ) < AEC_C_TOL ? 1 : 0 );
    }


/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_isPointInside
DESC: Returns TRUE if the point is inside the polygon and FALSE if not.
The function returns TRUE if the point falls on the polygon and also
sets the 'onside' variable to TRUE. Taken from the Web.
HIST: Original - tmi 08-Nov-1996
MISC: Implemented because the other Sedgewick based algorithm failed in
certain geometries.
KEYW: POLYGON POINT INSIDE FIND
-----------------------------------------------------------------------------%*/

int aecPolygon_isPointInside /* <= TRUE if point inside            */
    (
    long nvrt,                           /* => # of vertices in poly.           */
    DPoint3d *vrtP,                      /* => polygon vertices                 */
    DPoint3d *pntP                       /* => point to check                   */
    )
    {
    long i, counter = 0L;
    double xInters;
    DPoint3d p1,p2;

    if ( nvrt > 0L && vrtP )
        {
        p1 = vrtP[0];
        for ( i = 0; i < nvrt; i++)
            {
            p2 = vrtP[i % nvrt];
            if (pntP->y > AECMIN(p1.y,p2.y))
                {
                if (pntP->y <= AECMAX(p1.y,p2.y))
                    {
                    if (pntP->x <= AECMAX(p1.x,p2.x))
                        {
                        if (p1.y != p2.y)
                            {
                            xInters = (pntP->y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
                            if (p1.x == p2.x || pntP->x <= xInters)
                                counter++;
                            }
                        }
                    }
                }
            p1 = p2;
            }
        }
    return ( counter & 1 );
    }

/*%-----------------------------------------------------------------------------
FUNC: aecPolygon_computeRange
DESC: Computes the range of the input polygon.
HIST: Original - tmi 02-Mar-1991
MISC:
KEYW: POLYGON POLYLINE RANGE COMPUTE
-----------------------------------------------------------------------------%*/

void aecPolygon_computeRange
    (
    DPoint3d *sminP,                     /* <= lower left range coordinates     */
    DPoint3d *smaxP,                     /* => upper right range coordinates    */
    size_t nvrt,                           /* => # coordinates in input array     */
    DPoint3d *vrtP                       /* => input polyline                   */
    )
    {
    sminP->x = sminP->y = sminP->z =  AEC_C_MAXDBL;
    smaxP->x = smaxP->y = smaxP->z = -AEC_C_MAXDBL;

    for ( size_t i = 0; i < nvrt; i++ )
        {
        if ( vrtP[i].x < sminP->x ) sminP->x = vrtP[i].x;
        if ( vrtP[i].y < sminP->y ) sminP->y = vrtP[i].y;
        if ( vrtP[i].z < sminP->z ) sminP->z = vrtP[i].z;
        if ( vrtP[i].x > smaxP->x ) smaxP->x = vrtP[i].x;
        if ( vrtP[i].y > smaxP->y ) smaxP->y = vrtP[i].y;
        if ( vrtP[i].z > smaxP->z ) smaxP->z = vrtP[i].z;
        }
    }

#ifdef NOTUSED
//---------------------------------------------------------------------------
// DESC: static function to accumulate the polygons into an array of polygons
//       and an array of points
// HIST: Original - dakloske - 6/15/98
// MISC: 
//---------------------------------------------------------------------------
static int convexAccumFunc
    (
    long numPoints,
    DPoint3d *points,
    void *userData
    )
    {
    AECConvexAccumInfo *aInfo = (AECConvexAccumInfo *)userData;
    DPoint3d *dPointValues = 0;

    if ( !aInfo->nPolygons )
        {
        aInfo->ppPolygons = (DPoint3d **)calloc ( POLYBUF_REALLOC_SIZE, sizeof ( DPoint3d* ) );
        aInfo->npPolygons = (long *)     calloc ( POLYBUF_REALLOC_SIZE, sizeof ( long ) );
        }
    else if ( ( aInfo->nPolygons % POLYBUF_REALLOC_SIZE ) == 0 )
        {
        aInfo->ppPolygons = (DPoint3d **)realloc ( aInfo->ppPolygons, (aInfo->nPolygons + POLYBUF_REALLOC_SIZE) * sizeof ( DPoint3d* ) );
        aInfo->npPolygons = (long *)     realloc ( aInfo->npPolygons, (aInfo->nPolygons + POLYBUF_REALLOC_SIZE) * sizeof ( long ) );
        }

    if ( aInfo->ppPolygons && aInfo->npPolygons )
        {
        dPointValues = (DPoint3d *)malloc ( numPoints * sizeof ( DPoint3d ) );
        memcpy ( dPointValues, points, numPoints * sizeof ( DPoint3d ) );

        aInfo->ppPolygons[aInfo->nPolygons] = dPointValues;
        aInfo->npPolygons[aInfo->nPolygons] = numPoints;
        aInfo->nPolygons ++;
        }

    return SUCCESS;
    }
#endif
