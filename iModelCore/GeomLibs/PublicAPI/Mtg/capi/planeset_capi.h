/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_clear                                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_clear
(
DPlane3d_SmallSet   *planeSetP  /* OUT     plane set to be initialized */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_setNumPlanes                               |
|                                                                       |
|                                                                       |
| Set the numPlanes variable in the DPlane3d_SmallSet to given value.   |
| All other parts are left unchanged.                                   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_setNumPlanes
(
DPlane3d_SmallSet   *planeSetP,     /* IN OUT  Header to be modified */
int                 numPlanes       /* IN      count to insert */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_addByOriginAndNormal                       |
|                                                                       |
|                                                                       |
| Given an Origin and a Normal as DPoint3d's, adds a new plane          |
| into the DPlane3d_SmallSet                                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_addByOriginAndNormal
(
DPlane3d_SmallSet   *planeSetP,     /* IN OUT  Header to be modified */
DPoint3d            *origin,        /* IN      plane origin */
DPoint3d            *normal         /* IN      plane normal */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_append                                     |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_append
(
DPlane3d_SmallSet   *pPlaneSet,     /* IN OUT  Plane set to be expanded */
DPlane3d_SmallSet   *pAddedPlanes   /* IN      Set of planes to add */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_setSingleOrigin                            |
|                                                                       |
|                                                                       |
|DESC sets an Origin given as DPoint3d into a  DPlane3d_SmallSet        |               |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_setSingleOrigin
(
DPlane3d_SmallSet   *planeSetP,     /* IN OUT  Header to be modified */
DPoint3d            *origin,        /* IN      origin to insert */
int                 location        /* IN      index within plane set */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_setSingleNormal                            |
|                                                                       |
|                                                                       |
|DESC sets a Normal given as DPoint3d into a  DPlane3d_SmallSet         |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_setSingleNormal
(
DPlane3d_SmallSet   *planeSetP,     /* IN OUT  Header to be modified */
DPoint3d            *normal,        /* IN      normal to insert */
int                 location        /* IN      index within plane set */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_getSingleOrigin                            |
|                                                                       |
|                                                                       |
|DESC gets an Origin  stored at a given location in  DPlane3d_SmallSet  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_getSingleOrigin
(
DPoint3d            *origin,        /* OUT     extracted origin */
DPlane3d_SmallSet   *planeSetP,     /* IN      Plane set to query */
int                 location        /* IN      index within plane set */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_getSingleNormal                            |
|                                                                       |
|                                                                       |
|DESC gets a Normal  stored at a given location in  DPlane3d_SmallSet   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_getSingleNormal
(
DPoint3d            *normal,        /* OUT     extracted normal */
DPlane3d_SmallSet   *planeSetP,     /* IN      Plane set to query */
int                 location        /* IN      index within plane set */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_addByThreePoints                           |
|                                                                       |
|                                                                       |
|DESC Given three points lying on a plane as DPoint3d's adds a new      |
| plane into the DPlane3d_SmallSet                                      |
|                                                                       |
|NOTE: pPoint0 will be the origin of the plane.  The outward normal     |
| is directed along the cross product between the vectors from the      |
| origin to pPoint1 and pPoint2.                                        |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_addByThreePoints
(
DPlane3d_SmallSet   *planeSetP,     /* IN OUT  Header to be modified */
DPoint3d            *pPoint0,       /* IN      origin of the plane */
DPoint3d            *pPoint1,       /* IN      point on plane */
DPoint3d            *pPoint2        /* IN      point on plane */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_getCount                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlPlaneSet_getCount    /* Number of planes in the plane set */
(
DPlane3d_SmallSet   *planeSetP     /* IN      Header to be queried */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_getOriginAndNormal                         |
|                                                                       |
|                                                                       |
|DESC returns number of planes currently stored in the plane set        |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_getOriginAndNormal
(
        DPoint3d            *pOrigin,       /* IN      extracted origin */
        DPoint3d            *pNormal,       /* OUT     extracted normal */
const   DPlane3d_SmallSet   *planeSetP,     /* IN      Header to be queried */
        int                 i               /* IN      plane index */
);

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_reverseNormals                             |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPlaneSet_reverseNormals
(
DPlane3d_SmallSet   *pPlaneSet
);

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_getByHPoint                                |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_getByHPoint
(
DPoint4d            *pHPoint,       /* OUT     homogeneous vector for plane */
DPlane3d_SmallSet   *planeSetP,     /* IN      Header to be queried */
int                 location        /* IN      plane index */
);

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_addByHPoint                                |
| Add a plane to a plane set, using homogeneous vector form.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_addByHPoint
(
DPlane3d_SmallSet   *planeSetP,     /* IN OUT  plane set to receive plane */
DPoint4d            *pHPoint        /* IN      homogeneous vector form of plane to add */
);

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_setOutside                                 |
| Sets the inside/outside flag for a plane in a planeset.               |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_setOutside
(
DPlane3d_SmallSet   *planeSetP,     /* IN OUT  Containing set. */
int                 location,       /* IN      index of plane to flag */
int                 outside         /* IN      new flag value */
);

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_getOutside                                 |
| Queries the inside/outside flag for a plane in a planeset.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlPlaneSet_getOutside
(
DPlane3d_SmallSet   *planeSetP,     /* IN      plane set to query */
int                 location        /* IN      index of plane to query */
);

/*----------------------------------------------------------------------+
| name         jmdlVector_intersectRayPartialPolyhedron                 |
| Compute the intersections of a ray and a Polyhedron.                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlVector_intersectRayPartialPolyhedron
(
        double              *param0,   /* OUT     parametric coordinate where ray enters */
        double              *param1,   /* OUT     parametric coordinate where ray exits */
        DPoint3d            *pPoint0,  /* OUT     entry point */
        DPoint3d            *pPoint1,  /* OUT     exit point for infinite ray, this is
                                        direction vector in the direction of infinite
                                        ray, i. e. is either +-1 times pDirection */
        int                 *pType,    /* OUT     0 for no intersection,
                                     1 for bounded intersection,
                                     2 for unbounded intersection */
const   DPoint3d            *start,       /* IN      start of ray */
const   DPoint3d            *pDirection,  /* IN      direction vector for ray */
const   DPlane3d_SmallSet   *planeSetP,   /* IN      set of planes bounding the polyhedron */
        int                 *pSkipList,   /* IN      array of planes to be omitted */
        int                 nSkip         /* IN      number of planes to skip */
);

/*----------------------------------------------------------------------+
| name         jmdlVector_intersectRayPolyhedron                        |
| Compute the intersections of a ray and a Polyhedron.                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlVector_intersectRayPolyhedron
(
        double              *pParam0,   /* OUT     parametric coordinate where ray enters */
        double              *pParam1,   /* OUT     parametric coordinate where ray exits */
        DPoint3d            *pPoint0,  /* OUT     entry point */
        DPoint3d            *pPoint1,  /* OUT     exit point for infinite ray, this is
                                        direction vector in the direction of infinite
                                        ray, i. e. is either +-1 times pDirection */
        int                 *pType,    /* OUT     0 for no intersection,
                                     1 for bounded intersection,
                                     2 for unbounded intersection */
const   DPoint3d            *start,       /* IN      start of ray */
const   DPoint3d            *pDirection,  /* IN      direction vector for ray */
const   DPlane3d_SmallSet   *pPlaneSet,   /* IN      set of planes bounding the polyhedron */
        int                 skipPlane     /* IN      any plane index that is to be omitted */
);

/*----------------------------------------------------------------------+
| name         jmdlPlaneSet_extractEdges                                |
| Compute the edges of a convex polyhedron given its planes.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPlaneSet_extractEdges
(
        EmbeddedDPoint3dArray     *pDPoints,  /* OUT     intersection edges are appended to this
                                              array, in start/end pairs */
const   DPlane3d_SmallSet   *pPlanes    /* IN      set of planes bounding the polyhedron */
);

/*----------------------------------------------------------------------+
| name         jmdlHPoints_intersectPlanePolyhedron                     |
| Compute the intersections of a plane and a Polyhedron.                |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlHPoints_intersectPlanePolyhedron
(
        EmbeddedDPoint3dArray     *pDPoints,       /* OUT     polygon addes to this array */
        int                 *pType,         /* OUT     0 no intersection */
                                            /* OUT     1 closed loop intersection */
                                            /* OUT     2 unbounded intersection */
                                            /* first & last points are homogeneous vectors */
const   DPoint3d            *pCutOrigin,      /* IN      cutting plane origin */
const   DPoint3d            *pCutNormal,     /* IN      cutting plane normal */
const   DPlane3d_SmallSet   *planeSetP,   /* IN      set of planes bounding the polyhedron */
        int                 skipPlane       /* IN      index of any plane to skip. */
);

END_BENTLEY_GEOMETRY_NAMESPACE

