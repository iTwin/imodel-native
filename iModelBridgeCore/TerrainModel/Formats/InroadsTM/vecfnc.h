//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecVec_pointPositionInLine /* <=                               */
    (
    DPoint3d *p0,                        /* =>                                  */
    DPoint3d *p1,                        /* =>                                  */
    DPoint3d *p2                         /* =>                                  */
    );

int aecVec_doLinesIntersect /* <= TRUE if lines intersect          */
    (
    DPoint3d *p0,                        /* => line 1, point 1                  */
    DPoint3d *p1,                        /* => line 1, point 2                  */
    DPoint3d *q0,                        /* => line 2, point 1                  */
    DPoint3d *q1                         /* => line 2, point 2                  */
    );

double aecVec_pythag
    (
    double a,
    double b
    );

BOOL aecVec_projectToLine               // return TRUE if within segment
    (
    double *p1,                         // 2D line begin point.
    double *p2,                         // 2D line end point.
    double *p,                          // 2D point to project.
    double *oI,                         // offset of projected point [or NULL.]
    double *tI,                         // parameter of projected point [or NULL.]
    double *pI,                         // the projected point [or NULL.]
    BOOL bExtended = FALSE              // Calculate extended projections as needed.
    );

