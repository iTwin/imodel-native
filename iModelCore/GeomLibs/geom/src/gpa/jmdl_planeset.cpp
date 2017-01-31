/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/jmdl_planeset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/* MAP jmdlPlaneSet_clear=Geom.clear ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_clear                                      |
|                                                                       |
| author        Raghavan Kunigahalli                          3/96      |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_clear
(
DPlane3d_SmallSet   *planeSetP  /* <= plane set to be initialized */
)

    {
    if (planeSetP)
        planeSetP->numPlanes = 0;
    return  SUCCESS;
    }

/* MAP jmdlPlaneSet_setNumPlanes=Geom.setNumPlanes ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_setNumPlanes                               |
|                                                                       |
| author        Raghavan Kunigahalli                          5/96      |
|                                                                       |
| Set the numPlanes variable in the DPlane3d_SmallSet to given value.   |
| All other parts are left unchanged.                                   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_setNumPlanes
(
DPlane3d_SmallSet   *planeSetP,     /* <=> Header to be modified */
int                 numPlanes       /* => count to insert */
)

    {
    if (planeSetP)
        planeSetP->numPlanes = numPlanes;
    return  SUCCESS;
    }

/* MAP jmdlPlaneSet_addByOriginAndNormal=Geom.addByOriginAndNormal ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_addByOriginAndNormal                       |
|                                                                       |
| author        Raghavan Kunigahalli                          3/96      |
|                                                                       |
| Given an Origin and a Normal as DPoint3d's, adds a new plane          |
| into the DPlane3d_SmallSet                                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_addByOriginAndNormal
(
DPlane3d_SmallSet   *planeSetP,     /* <=> Header to be modified */
DPoint3d            *origin,        /* => plane origin */
DPoint3d            *normal         /* => plane normal */
)

    {
    int status = SUCCESS;

    if (planeSetP)
        {
        if (planeSetP->numPlanes == MS_SMALL_PLANE_SET_DIMENSION)
            status = ERROR;
        else
            {
            planeSetP->planes[planeSetP->numPlanes].origin      = *origin;
            planeSetP->planes[planeSetP->numPlanes++].normal    = *(DVec3d*)normal;
            }

        }
    else
        {
        status = ERROR;
        }
    return  status;
    }


/* MAP jmdlPlaneSet_append=Geom.append ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_append                                     |
|                                                                       |
| author        EarlinLutz                                    5/96      |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_append
(
DPlane3d_SmallSet   *pPlaneSet,     /* <=> Plane set to be expanded */
DPlane3d_SmallSet   *pAddedPlanes   /* => Set of planes to add */
)

    {
    int i;
    StatusInt status = SUCCESS;
    for (i = 0; SUCCESS == status && i < pAddedPlanes->numPlanes; i++)
        {
        status = jmdlPlaneSet_addByOriginAndNormal (pPlaneSet,
                                &pAddedPlanes->planes[i].origin, &pAddedPlanes->planes[i].normal);
        }
    return status;
    }
/* MAP jmdlPlaneSet_setSingleOrigin=Geom.setSingleOrigin ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_setSingleOrigin                            |
|                                                                       |
| author        Raghavan Kunigahalli                          5/96      |
|                                                                       |
|DESC sets an Origin given as DPoint3d into a  DPlane3d_SmallSet        |               |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_setSingleOrigin
(
DPlane3d_SmallSet   *planeSetP,     /* <=> Header to be modified */
DPoint3d            *origin,        /* => origin to insert */
int                 location        /* => index within plane set */
)

    {
    int status = SUCCESS;

    if (planeSetP)
        {
        if (location >= MS_SMALL_PLANE_SET_DIMENSION)
            status = ERROR;
        else
            {
            if (origin != NULL)
                planeSetP->planes[location].origin = *origin;
            else
                status = ERROR;
            }

        }
    else
        {
        status = ERROR;
        }
    return  status;
    }

/* MAP jmdlPlaneSet_setSingleNormal=Geom.setSingleNormal ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_setSingleNormal                            |
|                                                                       |
| author        Raghavan Kunigahalli                          5/96      |
|                                                                       |
|DESC sets a Normal given as DPoint3d into a  DPlane3d_SmallSet         |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_setSingleNormal
(
DPlane3d_SmallSet   *planeSetP,     /* <=> Header to be modified */
DPoint3d            *normal,        /* => normal to insert */
int                 location        /* => index within plane set */
)

    {
    int status = SUCCESS;

    if (planeSetP)
        {
        if (location >= MS_SMALL_PLANE_SET_DIMENSION)
            status = ERROR;
        else
            {
            if (normal != NULL)
                planeSetP->planes[location].normal = *(DVec3d*)normal;
            else
                status = ERROR;
            }

        }
    else
        {
        status = ERROR;
        }
    return  status;
    }

/* MAP jmdlPlaneSet_getSingleOrigin=Geom.getSingleOrigin ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_getSingleOrigin                            |
|                                                                       |
| author        Raghavan Kunigahalli                          5/96      |
|                                                                       |
|DESC gets an Origin  stored at a given location in  DPlane3d_SmallSet  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_getSingleOrigin
(
DPoint3d            *origin,        /* <= extracted origin */
DPlane3d_SmallSet   *planeSetP,     /* => Plane set to query */
int                 location        /* => index within plane set */
)

    {
    int status = SUCCESS;

    if (planeSetP)
        {
        if (location >= MS_SMALL_PLANE_SET_DIMENSION)
            status = ERROR;
        else
            {
            if (origin != NULL)
                *origin = planeSetP->planes[location].origin;
            }

        }
    else
        {
        status = ERROR;
        }
    return  status;
    }

/* MAP jmdlPlaneSet_getSingleNormal=Geom.getSingleNormal ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_getSingleNormal                            |
|                                                                       |
| author        Raghavan Kunigahalli                          5/96      |
|                                                                       |
|DESC gets a Normal  stored at a given location in  DPlane3d_SmallSet   |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_getSingleNormal
(
DPoint3d            *normal,        /* <= extracted normal */
DPlane3d_SmallSet   *planeSetP,     /* => Plane set to query */
int                 location        /* => index within plane set */
)

    {
    int status = SUCCESS;

    if (planeSetP)
        {
        if (location >= MS_SMALL_PLANE_SET_DIMENSION)
            status = ERROR;
        else
            {
            if (normal != NULL)
                *normal = planeSetP->planes[location].normal;
            }

        }
    else
        {
        status = ERROR;
        }
    return  status;
    }

/* MAP jmdlPlaneSet_addByThreePoints=Geom.addByThreePoints ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_addByThreePoints                           |
|                                                                       |
| author        Raghavan Kunigahalli                          3/96      |
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
DPlane3d_SmallSet   *planeSetP,     /* <=> Header to be modified */
DPoint3d            *pPoint0,       /* => origin of the plane */
DPoint3d            *pPoint1,       /* => point on plane */
DPoint3d            *pPoint2        /* => point on plane */
)

    {
     DPoint3d firstVect, secondVect, normal;
     double length;
     int status = SUCCESS;

     if (planeSetP)
        {
        if (planeSetP->numPlanes == MS_SMALL_PLANE_SET_DIMENSION)
            status = ERROR;

        else
            {
            bsiDPoint3d_subtractDPoint3dDPoint3d (&firstVect, pPoint1, pPoint0);
            bsiDPoint3d_subtractDPoint3dDPoint3d (&secondVect, pPoint2, pPoint0);
            length = bsiDPoint3d_normalizedCrossProduct (&normal, &secondVect, &firstVect);
            planeSetP->planes[(planeSetP->numPlanes)].origin = *pPoint0;
            planeSetP->planes[(planeSetP->numPlanes)++].normal = *(DVec3d*)&normal;
            }

        }

    else
        {
        status = ERROR;
        }

    return  status;

    }

/* MAP jmdlPlaneSet_getOriginAndNormal=Geom.getCount ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_getCount                                   |
|                                                                       |
| author        Raghavan Kunigahalli                          3/96      |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlPlaneSet_getCount    /* Number of planes in the plane set */
(
DPlane3d_SmallSet   *planeSetP     /* => Header to be queried */
)
    {
    if (planeSetP)
        return  planeSetP->numPlanes;
    else
        return  ERROR;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlPlaneSet_getOriginAndNormal                         |
|                                                                       |
| author        Raghavan Kunigahalli                          3/96      |
|                                                                       |
|DESC returns number of planes currently stored in the plane set        |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_getOriginAndNormal
(
        DPoint3d            *pOrigin,       /* => extracted origin */
        DPoint3d            *pNormal,       /* <= extracted normal */
const   DPlane3d_SmallSet   *planeSetP,     /* => Header to be queried */
        int                 i               /* => plane index */
)

    {
    int status = SUCCESS;

    if (planeSetP && planeSetP->numPlanes > i && i >= 0)
        {
        if (pOrigin)
            *pOrigin = planeSetP->planes[i].origin;
        if (pNormal)
            *pNormal = planeSetP->planes[i].normal;
        }
    else
        {
        status = ERROR;
        }

    return  status;

    }


/* MAP jmdlPlaneSet_reverseNormals=Geom.reverseNormals ENDMAP */

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_reverseNormals                             |
| author        Raghavan Kunigahalli                          06/97      |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPlaneSet_reverseNormals
(
DPlane3d_SmallSet   *pPlaneSet
)

    {
    int i;
    for (i = 0; i < pPlaneSet->numPlanes; i++)
        {
        bsiDPoint3d_scale (&pPlaneSet->planes[i].normal, &pPlaneSet->planes[i].normal, -1.0);
        }
    }


/* MAP jmdlPlaneSet_getByHPoint=Geom.getByHPoint ENDMAP */

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_getByHPoint                                |
| author        Raghavan Kunigahalli                          3/96      |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_getByHPoint
(
DPoint4d            *pHPoint,       /* <= homogeneous vector for plane */
DPlane3d_SmallSet   *planeSetP,     /* => Header to be queried */
int                 location        /* => plane index */
)

    {
    int status;

    if (pHPoint && planeSetP)
        {
         status = CHANGE_TO_BOOL(bsiDPoint4d_planeFromOriginAndNormal(pHPoint,
                                 &planeSetP->planes[location].origin,
                                 &planeSetP->planes[location].normal) ? SUCCESS : ERROR);
        }
    else
        {
        status = ERROR;
        }

    return  status;

    }


/* MAP jmdlPlaneSet_addByHPoint=Geom.addByHPoint ENDMAP */

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_addByHPoint                                |
| author        Raghavan Kunigahalli                          3/96      |
| Add a plane to a plane set, using homogeneous vector form.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_addByHPoint
(
DPlane3d_SmallSet   *planeSetP,     /* <=> plane set to receive plane */
DPoint4d            *pHPoint        /* => homogeneous vector form of plane to add */
)

    {
    int status = SUCCESS;
    DPoint3d denom;
    double lambda, squaredMag;

    if (pHPoint && planeSetP && planeSetP->numPlanes < MS_SMALL_PLANE_SET_DIMENSION)
        {
         planeSetP->planes[planeSetP->numPlanes].normal.x = denom.x = pHPoint->x;
         planeSetP->planes[planeSetP->numPlanes].normal.y = denom.y = pHPoint->y;
         planeSetP->planes[planeSetP->numPlanes].normal.z = denom.z = pHPoint->z;

         squaredMag = bsiDPoint3d_magnitudeSquared(&denom);
         lambda = ((-1.0) * pHPoint->w) / squaredMag;
         bsiDPoint3d_scale (&planeSetP->planes[planeSetP->numPlanes].origin,
                        &planeSetP->planes[planeSetP->numPlanes].normal, lambda);
         planeSetP->numPlanes++;
        }
    else
        {
        status = ERROR;
        }

    return  status;
    }

/* MAP jmdlPlaneSet_setOutside=Geom.setOutside ENDMAP */

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_setOutside                                 |
| author        Raghavan Kunigahalli                          3/96      |
| Sets the inside/outside flag for a plane in a planeset.               |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPlaneSet_setOutside
(
DPlane3d_SmallSet   *planeSetP,     /* <=> Containing set. */
int                 location,       /* => index of plane to flag */
int                 outside         /* => new flag value */
)

    {
    int status = SUCCESS;

    if (planeSetP)
        {
        planeSetP->outside = outside;
        }
    else
        {
        status = ERROR;
        }
    return  status;

    }

/* MAP jmdlPlaneSet_getOutside=Geom.getOutside ENDMAP */

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_getOutside                                 |
| author        Raghavan Kunigahalli                          3/96      |
| Queries the inside/outside flag for a plane in a planeset.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlPlaneSet_getOutside
(
DPlane3d_SmallSet   *planeSetP,     /* => plane set to query */
int                 location        /* => index of plane to query */
)

    {

    if (planeSetP)
        {
        return planeSetP->outside;
        }
    else
        {
        return ERROR;
        }

    }

/*----------------------------------------------------------------------+
| name          jmdlPlaneSet_searchIntArray                             |
| author        Raghavan Kunigahalli                          3/96      |
+----------------------------------------------------------------------*/
static int jmdlPlaneSet_searchIntArray
(
        int     iSearch,    /* => index to search for */
const   int     *pList,     /* => list of indices */
        int     nMember     /* => number of members in list */
)
    {
    int i;
    for (i = 0; i < nMember; i++)
        {
        if (pList[i] == iSearch)
            return i;
        }
    return -1;
    }
/* MAP jmdlVector_intersectRayPartialPolyhedron=Geom.intersectRayPartialPolyhedron ENDMAP */

/*----------------------------------------------------------------------+
| name         jmdlVector_intersectRayPartialPolyhedron                 |
| author       RaghavanKunigahalli                            3/96      |
| Compute the intersections of a ray and a Polyhedron.                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlVector_intersectRayPartialPolyhedron
(
        double              *param0,   /* <= parametric coordinate where ray enters */
        double              *param1,   /* <= parametric coordinate where ray exits */
        DPoint3d            *pPoint0,  /* <= entry point */
        DPoint3d            *pPoint1,  /* <= exit point for infinite ray, this is
                                        direction vector in the direction of infinite
                                        ray, i. e. is either +-1 times pDirection */
        int                 *pType,    /* <= 0 for no intersection,
                                     1 for bounded intersection,
                                     2 for unbounded intersection */
const   DPoint3d            *start,       /* => start of ray */
const   DPoint3d            *pDirection,  /* => direction vector for ray */
const   DPlane3d_SmallSet   *planeSetP,   /* => set of planes bounding the polyhedron */
        int                 *pSkipList,   /* => array of planes to be omitted */
        int                 nSkip         /* => number of planes to skip */
)

    {
    int         i, status = SUCCESS;
    bool        found = false;
    double      minSout = 0.0, maxSin = 0.0, param, dot;
    int         minSoutIsSet, maxSinIsSet;
    DPoint3d    intPoint;

    minSoutIsSet = maxSinIsSet = false;

     for (i = 0; i < planeSetP->numPlanes; i++)
        {
         if (nSkip == 0 || jmdlPlaneSet_searchIntArray (i, pSkipList, nSkip) < 0)
            {
            if (SUCCESS == (status = CHANGE_TO_BOOL(bsiGeom_rayPlaneIntersection(&param, &intPoint, start,
                                                        pDirection, &planeSetP->planes[i].origin,
                                                                &planeSetP->planes[i].normal) ? SUCCESS : ERROR)))
                {
                 found = true;
                 dot = bsiDPoint3d_dotProduct(pDirection, &planeSetP->planes[i].normal);
                 if ((dot < 0.0) && (!maxSinIsSet || param > maxSin))
                    {
                    maxSinIsSet = true;
                    maxSin = param;
                    *pPoint0 = intPoint;
                    }

                 else if ((dot  > 0.0) && (!minSoutIsSet || param < minSout))
                    {
                    minSoutIsSet = true;
                    minSout = param;
                    *pPoint1 = intPoint;
                    }

                }
             else if (status == ERROR)
                {
                dot = bsiDPoint3d_dotDifference(start, &planeSetP->planes[i].origin,
                                                &planeSetP->planes[i].normal);
                if (dot > 0)
                    {
                     found = false;
                     break;
                    }

                }

            }

        }

     if ((found != true) || (!minSoutIsSet && !maxSinIsSet))
        {
        *pType = 0;
        }

     else if ((minSoutIsSet && maxSinIsSet) && (minSout > maxSin))
        {
        *pType = 1;
        if (param0)
            *param0 = maxSin;
        if (param1)
            *param1 = minSout;
        }

     else if ((found == true) && (maxSinIsSet) && (!minSoutIsSet))
        {
        *pType = 2;
        if (param0)
            *param0 = maxSin;
        }

     else if ((found == true) && (minSoutIsSet) && (!maxSinIsSet))
        {
        *pType = 2;
        if (param1)
            *param1 = minSout;
        }

     else if ((minSoutIsSet && maxSinIsSet) && (minSout < maxSin))
        {
        *pType = 0;
        }

     else
        {
        *pType = 3;
        }

     return SUCCESS;

    }

/* MAP jmdlVector_intersectRayPolyhedron=Geom.intersectRayPolyhedron ENDMAP */

/*----------------------------------------------------------------------+
| name         jmdlVector_intersectRayPolyhedron                        |
| author       RaghavanKunigahalli                            3/96      |
| Compute the intersections of a ray and a Polyhedron.                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlVector_intersectRayPolyhedron
(
        double              *pParam0,   /* <= parametric coordinate where ray enters */
        double              *pParam1,   /* <= parametric coordinate where ray exits */
        DPoint3d            *pPoint0,  /* <= entry point */
        DPoint3d            *pPoint1,  /* <= exit point for infinite ray, this is
                                        direction vector in the direction of infinite
                                        ray, i. e. is either +-1 times pDirection */
        int                 *pType,    /* <= 0 for no intersection,
                                     1 for bounded intersection,
                                     2 for unbounded intersection */
const   DPoint3d            *start,       /* => start of ray */
const   DPoint3d            *pDirection,  /* => direction vector for ray */
const   DPlane3d_SmallSet   *pPlaneSet,   /* => set of planes bounding the polyhedron */
        int                 skipPlane     /* => any plane index that is to be omitted */
)

    {
    if (skipPlane >= 0)
        return jmdlVector_intersectRayPartialPolyhedron (pParam0, pParam1, pPoint0, pPoint1, pType, start,
                        pDirection, pPlaneSet, &skipPlane, 1);
    else
        return jmdlVector_intersectRayPartialPolyhedron (pParam0, pParam1, pPoint0, pPoint1, pType, start,
                        pDirection, pPlaneSet, NULL, 0);
    }


/* MAP jmdlPlaneSet_extractEdges=Geom.extractEdges ENDMAP */

/*----------------------------------------------------------------------+
| name         jmdlPlaneSet_extractEdges                                |
| author       RaghavanKunigahalli                            4/96      |
| Compute the edges of a convex polyhedron given its planes.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPlaneSet_extractEdges
(
        EmbeddedDPoint3dArray     *pDPoints,  /* <= intersection edges are appended to this
                                              array, in start/end pairs */
const   DPlane3d_SmallSet   *pPlanes    /* => set of planes bounding the polyhedron */
)

    {
    int i,j;
    DPoint3d rayStart, rayDirection;
    int intersectionType, status;
    double param0, param1;
    DPoint3d point0, point1;
    int     skipPlane[2];

    for (i = 0; i < pPlanes->numPlanes; i++)
        {
        for ( j = 0; j < pPlanes->numPlanes; j++)
            {
            if ( i != j)
                {
                status = CHANGE_TO_BOOL(bsiGeom_planePlaneIntersection(&rayStart, &rayDirection,
                                &pPlanes->planes[i].origin, &pPlanes->planes[i].normal,
                                &pPlanes->planes[j].origin, &pPlanes->planes[j].normal) ? SUCCESS : ERROR);
                if (SUCCESS == status)
                    {
                    skipPlane[0] = i;
                    skipPlane[1] = j;
                    status = jmdlVector_intersectRayPartialPolyhedron (
                            &param0, &param1, &point0, &point1, &intersectionType,
                            &rayStart, &rayDirection, pPlanes, skipPlane, 2);
                    if (intersectionType == 1)
                        {
                        jmdlVArrayDPoint3d_addPoint (pDPoints, &point0);
                        jmdlVArrayDPoint3d_addPoint (pDPoints, &point1);
                        }
                    }
                }
            }
        }
    }



/*----------------------------------------------------------------------+
| name         jmdlHPoints_intersectPlanePolyhedron                     |
| author       RaghavanKunigahalli                            4/96      |
| Compute the intersections of a plane and a Polyhedron.                |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlHPoints_intersectPlanePolyhedron
(
        EmbeddedDPoint3dArray     *pDPoints,       /* <= polygon addes to this array */
        int                 *pType,         /* <= 0 no intersection */
                                            /* <= 1 closed loop intersection */
                                            /* <= 2 unbounded intersection */
                                            /* first & last points are homogeneous vectors */
const   DPoint3d            *pCutOrigin,      /* => cutting plane origin */
const   DPoint3d            *pCutNormal,     /* => cutting plane normal */
const   DPlane3d_SmallSet   *planeSetP,   /* => set of planes bounding the polyhedron */
        int                 skipPlane       /* => index of any plane to skip. */
)

    {
    int                 chainPointCount = 0, i, pTypecheck, numPoints, status = SUCCESS;
    DPoint3d            start, direction, entry, exit, *pTmpPoint, *firstPoint, *lastPoint;

    EmbeddedDPoint3dArray     *pSourcePoints;
    GraphicsPointArrayP pChainPoints = NULL;
    double              largestCoord, epsilon = 0.0;

    pSourcePoints = jmdlVArrayDPoint3d_grab();

    for (i = 0; i < planeSetP->numPlanes;i++)
        {
        if (i != skipPlane)
            {
            status = CHANGE_TO_BOOL(bsiGeom_planePlaneIntersection(&start, &direction, pCutOrigin,
                                        pCutNormal, &planeSetP->planes[i].origin,
                                                    &planeSetP->planes[i].normal) ? SUCCESS : ERROR);

            status = jmdlVector_intersectRayPolyhedron (NULL, NULL, &entry, &exit,
                                        &pTypecheck, &start, &direction, planeSetP, i);
            if (pTypecheck == 1)
                {
                status = jmdlVArrayDPoint3d_addPoint (pSourcePoints, &entry);
                status = jmdlVArrayDPoint3d_addPoint (pSourcePoints, &exit);
                }

            }

        }

    if (0 != (numPoints = jmdlVArrayDPoint3d_getCount(pSourcePoints)))
        {
        int nPoints;
        pChainPoints = jmdlGraphicsPointArray_grab();

        pTmpPoint = jmdlVArrayDPoint3d_getPtr (pSourcePoints, 0);
        jmdlMTGChain_segmentArrayToGPA (pChainPoints, pTmpPoint, numPoints);
        nPoints = jmdlGraphicsPointArray_getCount (pChainPoints);
        status = jmdlVArrayDPoint3d_extend (pDPoints, nPoints);

        status = jmdlGraphicsPointArray_getDPoint3dVarArray (pChainPoints, pDPoints, 0, nPoints);

        pTmpPoint = jmdlEmbeddedDPoint3dArray_getPtr(pDPoints, 0);

        largestCoord = bsiDPoint3d_getLargestCoordinate(pTmpPoint, nPoints);
        epsilon = largestCoord * bsiTrig_smallAngle ();
        }

    if ((chainPointCount = jmdlVArrayDPoint3d_getCount(pDPoints)) == 0 )
        {
        *pType = 0;
        }
    else
        {
        firstPoint = jmdlVArrayDPoint3d_getPtr(pDPoints, 0);
        lastPoint = jmdlVArrayDPoint3d_getPtr(pDPoints, chainPointCount - 1);

        if (fabs (firstPoint->x - lastPoint->x) <= epsilon)
            {
            if ((fabs (firstPoint->y - lastPoint->y) <= epsilon)
                && (fabs (firstPoint->z - lastPoint->z) <= epsilon))
                    *pType =  1;


            }
        else if (chainPointCount > 0)
            *pType = 2;
        }

    jmdlVArrayDPoint3d_drop(pSourcePoints);
    jmdlGraphicsPointArray_drop(pChainPoints);


    return  SUCCESS;
    }


END_BENTLEY_GEOMETRY_NAMESPACE