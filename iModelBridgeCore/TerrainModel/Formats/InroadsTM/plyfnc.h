//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

#include "aecuti.h"

/*----------------------------------------------------------------------------*/
/* Function prototypes.                                                       */
/*----------------------------------------------------------------------------*/

int aecPolygon_removeLoops  /* <= TRUE if ERROR                    */
    (
    long nvts,                           /* => # points in input polygon        */
    DPoint3d *vts                        /* => input polygon coordinates        */
    );

void aecPolygon_planarArea
    (
    double *areaP,                       /* <= computed area                    */
    long nvrt,                           /* => # vertices in poly.              */
    DPoint3d *vrtP                       /* => polygon coordinates              */
    );

void aecPolygon_reverseLongArray
    (
    long nvrt,                           /* => # vertices in poly.              */
    long *vrtP                           /* => polygon coordinates              */
    );

void aecPolygon_reverse
    (
    size_t nvrt,                           /* => # vertices in poly.              */
    DPoint3d *vrtP                       /* => polygon coordinates              */
    );

void aecPolygon_horizontalLength
    (
    double *lenP,                        /* <= computed hor. length             */
    long nvrt,                           /* => # vertices in poly.              */
    DPoint3d *vrtP                       /* => polygon coordinates              */
    );

void aecPolygon_removeDuplicatePoints
    (
    long *nvrtP,                         /* <=> # of vertices in line.          */
    DPoint3d *vrtP,                      /* <=> linestring vertices             */
    double tolerance                     /*  => tolerance to use                */
    );

void aecPolygon_intersect
    (
    DPoint3d *pnt,                       /* <= computed point                   */
    int *par,                            /* <= 1: parallel, 0: not              */
    int *ins,                            /* <= 1: inside, 0: don't              */
    DPoint3d *a,                         /* => line 1 end 1                     */
    DPoint3d *b,                         /* => line 1 end 2                     */
    DPoint3d *c,                         /* => line 2 end 1                     */
    DPoint3d *d                          /* => line 2 end 2                     */
    );

void aecPolygon_removeColinearPoints
    (
    long *nvrtP,                         /* <=> # points in polygon             */
    DPoint3d *vrtP                       /* <=> polygon points.                 */
    );

int aecPolygon_colinear     /* <= TRUE if colinear                 */
    (
    DPoint3d *a,                         /* => line 1 end 1                     */
    DPoint3d *b,                         /* => line 1 end 2                     */
    DPoint3d *c,                         /* => line 2 end 1                     */
    DPoint3d *d                          /* => line 2 end 2                     */
    );

int aecPolygon_isPointInside /* <= TRUE if point inside            */
    (
    long nvrt,                           /* => # of vertices in poly.           */
    DPoint3d *vrtP,                      /* => polygon vertices                 */
    DPoint3d *pntP                       /* => point to check                   */
    );

void aecPolygon_computeRange
    (
    DPoint3d *sminP,                     /* <= lower left range coordinates     */
    DPoint3d *smaxP,                     /* => upper right range coordinates    */
    size_t nvrt,                           /* => # coordinates in input array     */
    DPoint3d *vrtP                       /* => input polyline                   */
    );

