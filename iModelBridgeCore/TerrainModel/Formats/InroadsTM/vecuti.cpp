//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* vecuti.c                                            dgc    15-Jan-1993     */
/*----------------------------------------------------------------------------*/
/* Various vector utilities.                                                  */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include <vecfnc.h>

/*%-----------------------------------------------------------------------------
FUNC: aecVec_pointPositionInLine
DESC: Computes point position relative to a line.
Taken from Sedgewick, Algorithms in C.
HIST: Original - dgc 15-Jan-1993
MISC:
KEYW: VECTOR POINT POSITION
-----------------------------------------------------------------------------%*/

int aecVec_pointPositionInLine /* <=                               */
    (
    DPoint3d *p0,                        /* =>                                  */
    DPoint3d *p1,                        /* =>                                  */
    DPoint3d *p2                         /* =>                                  */
    )
    {
    double dx1, dx2, dy1, dy2;

    dx1 = p1->x - p0->x;
    dy1 = p1->y - p0->y;
    dx2 = p2->x - p0->x;
    dy2 = p2->y - p0->y;

    if( dx1 * dy2 > dy1 * dx2 )
        return ( 1 );
    if( dx1 * dy2 < dy1 * dx2 )
        return ( -1 );
    if( ( dx1 * dx2 < 0.0 ) || ( dy1 * dy2 < 0.0 ) )
        return ( -1 );
    if( ( dx1 * dx1 + dy1 * dy1 ) < ( dx2 * dx2 + dy2 * dy2 ) )
        return ( 1 );

    return ( 0 );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecVec_doLinesIntersect
DESC: Returns TRUE if the lines intersect and FALSE if not.
Taken from Sedgewick, Algorithms in C.
HIST: Original - dgc 15-Jan-1993
MISC:
KEYW: VECTOR LINES INTERSECTION CHECK
-----------------------------------------------------------------------------%*/

int aecVec_doLinesIntersect /* <= TRUE if lines intersect          */
    (
    DPoint3d *p0,                        /* => line 1, point 1                  */
    DPoint3d *p1,                        /* => line 1, point 2                  */
    DPoint3d *q0,                        /* => line 2, point 1                  */
    DPoint3d *q1                         /* => line 2, point 2                  */
    )
    {
    return( ( ( aecVec_pointPositionInLine( p0, p1, q0 ) * aecVec_pointPositionInLine( p0, p1, q1 ) ) <= 0 ) &&
        ( ( aecVec_pointPositionInLine( q0, q1, p0 ) * aecVec_pointPositionInLine( q0, q1, p1 ) ) <= 0 ) );
    }


//---------------------------------------------------------------------------
// DESC: Computes sqrt(a*a + b*b) without destructive underflow/overflow.
// HIST: Original - MAH - 05/02/00
// MISC: 
//---------------------------------------------------------------------------
double aecVec_pythag( double a, double b )
    {
    double absa = a < 0.0 ? -a : a;
    double absb = b < 0.0 ? -b : b;

    if( absa > absb )
        return absa * sqrt( 1.0 + (absb/absa)*(absb/absa) );
    else
        return(absb <= 1.0e-15 ? 0.0 : absb * sqrt( 1.0 + (absa/absb)*(absa/absb) ) );
    }

//---------------------------------------------------------------------------
// DESC: Projects a point to a plane defined by three points.
// HIST: Original - MAH - 01/20/04
// MISC: 
//---------------------------------------------------------------------------
BOOL aecVec_projectToLine               // return TRUE if within segment
    (
    double *p1,                         // 2D line begin point.
    double *p2,                         // 2D line end point.
    double *p,                          // 2D point to project.
    double *oI,                         // offset of projected point [or NULL.]
    double *tI,                         // parameter of projected point [or NULL.]
    double *pI,                         // the projected point [or NULL.]
    BOOL bExtended                      // Calculate extended projections as needed.
    )
    {
    // REM: projection formulas from Linear Algebra.
    //
    // Projection:
    // projection of ( p - p1 ) onto ( p2 - p1 ) = ( p - p1 ) * (p2 - p1 ) / || p2 - p1 ||.
    //
    // Projected point:
    // pI = p1 + [ ( p - p1 ) * ( p2 - p1 ) / || p2 - p1 || ] * ( p2 - p1 ) / || p2 - p1 ||
    //
    // Parameter along line with (+/-) and (0 <= t <= 1 if within line segment):
    // tI = ( p - p1 ) * ( p2 - p1 ) / || p2 - p1 ||^2.
    // tI = ( pI - p1 ) * ( p2 - p1 ) / || p2 - p1 ||^2
    // since ( pI - p ) is orthogonal to ( p2 - p1 )

    double dotv = ( p2[0] - p1[0] ) * ( p2[0] - p1[0] ) + ( p2[1] - p1[1] ) * ( p2[1] - p1[1] );
    double dot  = ( p[0] - p1[0] ) * ( p2[0] - p1[0] ) + ( p[1] - p1[1] ) * ( p2[1] - p1[1] );
    double t = ( dotv <= 1.0e-15 ) ? 0.0 : ( dot / dotv );

    if( tI )
        *tI = t;

    if( pI )
        {
        pI[0] = p1[0];
        pI[1] = p1[1];

        if( ( -1.0E-15 < t && t < 1.0 + 1.0E-15 ) || bExtended )
            {
            pI[0] = pI[0] + t * ( p2[0] - p1[0] );
            pI[1] = pI[1] + t * ( p2[1] - p1[1] );
            }
        }

    if( oI )
        if( ( -1.0E-15 < t && t < 1.0 + 1.0E-15 ) || bExtended )
            *oI = aecVec_pythag( p1[0] + t * ( p2[0] - p1[0] ) - p[0], p1[1] + t * ( p2[1] - p1[1] ) - p[1] );
        else
            *oI = aecVec_pythag( p1[0] - p[0], p1[1] - p[1] );

    return ( -1.0E-15 < t && t < 1.0 + 1.0E-15 );
    }

