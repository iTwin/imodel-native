/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <math.h>
/*---------------------------------------------------------------------------------**//**
@description Set the x-coordinate of a node.
@param nodeP IN OUT node to update
@param xCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setX
(
VuP             nodeP,          /* => known vertex use */
double          xCoord          /* => coordinate to install */
)
    {
    VU_U(nodeP) = xCoord;
    }

/*---------------------------------------------------------------------------------**//**
@description Set the y-coordinate of a node.
@param nodeP IN OUT node to update
@param yCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setY
(
VuP             nodeP,
double          yCoord
)
    {
    VU_V(nodeP) = yCoord;
    }

/*---------------------------------------------------------------------------------**//**
@description Set the x- and y-coordinates of a node.
@param nodeP IN OUT node to update
@param xCoord IN new coordinate
@param yCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setXY
(
VuP             nodeP,
double          xCoord,
double          yCoord
)
    {
    VU_U(nodeP) = xCoord;
    VU_V(nodeP) = yCoord;
    }

/*---------------------------------------------------------------------------------**//**
@description Set coordinates of a node.
@param nodeP IN OUT node to update
@param xCoord IN new coordinate
@param yCoord IN new coordinate
@param zCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setXYZ
(
VuP             nodeP,
double          xCoord,
double          yCoord,
double          zCoord
)
    {
    VU_U(nodeP) = xCoord;
    VU_V(nodeP) = yCoord;
    VU_W(nodeP) = zCoord;
    }


/*---------------------------------------------------------------------------------**//**
@description Set the x- and y-coordinates of a node.
@param nodeP IN OUT node to update
@param pointP IN new coordinates
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setDPoint2d
(
VuP             nodeP,
DPoint2d *      pointP
)
    {
    VU_U(nodeP) = pointP->x;
    VU_V(nodeP) = pointP->y;
    }


/*---------------------------------------------------------------------------------**//**
@description Set the x- and y-coordinates of all nodes around a vertex.
@param nodeP IN OUT any node at the vertex
@param xCoord IN new coordinate
@param yCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setXYAroundVertex
(
VuP             nodeP,
double          xCoord,
double          yCoord
)
    {
    VU_VERTEX_LOOP (currP, nodeP)
        {
        VU_U(currP) = xCoord;
        VU_V(currP) = yCoord;
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    }

/*---------------------------------------------------------------------------------**//**
@description Set coordinates of all nodes around a vertex.
@param nodeP IN OUT any node at the vertex
@param xCoord IN new coordinate
@param yCoord IN new coordinate
@param zCoord IN new coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setXYZAroundVertex
(
VuP             nodeP,
double          xCoord,
double          yCoord,
double          zCoord
)
    {
    VU_VERTEX_LOOP (currP, nodeP)
        {
        VU_U(currP) = xCoord;
        VU_V(currP) = yCoord;
        VU_W(currP) = zCoord;
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    }

/*---------------------------------------------------------------------------------**//**
@description Set the x- and y-coordinates of all nodes around a vertex.
@param nodeP IN OUT any node at the vertex
@param pointP IN new coordinates
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_setDPoint2dAroundVertex
(
VuP             nodeP,
DPoint2d *      pointP
)
    {
    double xCoord = pointP->x;
    double yCoord = pointP->y;
    VU_VERTEX_LOOP (currP, nodeP)
        {
        VU_U(currP) = xCoord;
        VU_V(currP) = yCoord;
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    }

/*---------------------------------------------------------------------------------**//**
@description Copy coordinates from one node to another.
@param destP IN OUT destination node
@param sourceP IN destination node
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_copyCoordinates
(
VuP destP,
VuP sourceP
)
    {
    VU_UVW(destP) = VU_UVW(sourceP);
    }


/*---------------------------------------------------------------------------------**//**
@description Query the x-coordinate of a node.
@param nodeP IN node to query
@return x-coordinate of node.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getX
(
VuP             nodeP
)
    {
    return VU_U(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query the y-coordinate of a node.
@param nodeP IN node to query
@return y-coordinate of node.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getY
(
VuP             nodeP
)
    {
    return VU_V(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query x- and y-coordinates of a node.
@param xCoordP OUT x-coordinate
@param yCoordP OUT y-coordinate
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_getXY
(
double*         xCoordP,
double*         yCoordP,
VuP             nodeP
)
    {
    *xCoordP = VU_U(nodeP);
    *yCoordP = VU_V(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query x- and y-coordinates of a node.
@param pointP OUT xy-coordinates
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_getDPoint2d
(
DPoint2d *      pointP,         /* <= point to receive coordinates */
VuP             nodeP
)
    {
    pointP->x = VU_U(nodeP);
    pointP->y = VU_V(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@nodoc
@deprecated vu_getDX
@description Query the x-component of the vector from a node to its face successor.
@param nodeP IN node to query
@return x-component of edge vector.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   getDX
(
VuP             nodeP
)
    {
    return vu_getDX (nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query the x-component of the vector from a node to its face successor.
@param nodeP IN node to query
@return x-component of edge vector.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getDX
(
VuP             nodeP
)
    {
    return VU_U(VU_FSUCC(nodeP)) - VU_U(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@nodoc
@deprecated vu_getDY
@description Query the y-component of the vector from a node to its face successor.
@param nodeP IN node to query
@return y-component of edge vector.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   getDY
(
VuP             nodeP
)
    {
    return vu_getDY (nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query the y-component of the vector from a node to its face successor.
@param nodeP IN node to query
@return y-component of edge vector.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getDY
(
VuP             nodeP
)
    {
    return VU_V(VU_FSUCC(nodeP)) - VU_V(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query a single coordinate from a single node, using integer index.
@param nodeP IN node to query
@param index IN coordinate index.  0,1,2 are x,y,z; others are adjusted cyclically.
@return requested coordinate
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_getIndexedXYZ
(
VuP             nodeP,
int             index
)
    {
    if (index == 0)
        return VU_U(nodeP);
    if (index == 1)
        return VU_V(nodeP);
    if (index == 2)
        return VU_W(nodeP);
    index = Angle::Cyclic3dAxis (index);
    return vu_getIndexedXYZ (nodeP, index);
    }

/*---------------------------------------------------------------------------------**//**
@description Store a single coordinate for a single node, using integer index.
@param nodeP IN OUT node to update
@param index IN coordinate index.  0,1,2 are x,y,z; others are adjusted cyclically.
@param a IN coordinate value to store
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  vu_setIndexedXYZ
(
VuP             nodeP,
int             index,
double          a
)
    {
    if (index == 0)
        {
        VU_U(nodeP) = a;
        }
    else if (index == 1)
        {
        VU_V(nodeP) = a;
        }
    else if (index == 2)
        {
        VU_W(nodeP) = a;
        }
    else
        {
        index = Angle::Cyclic3dAxis (index);
        vu_setIndexedXYZ (nodeP, index, a);
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Store a single coordinate for all nodes around a vertex, using integer index.
@param nodeP IN OUT any node at the vertex
@param index IN coordinate index.  0,1,2 are x,y,z; others are adjusted cyclically.
@param a IN coordinate value to store
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  vu_setIndexedXYZAroundVertex
(
VuP             nodeP,
int             index,
double          a
)
    {
    VU_VERTEX_LOOP (currP, nodeP)
        {
        vu_setIndexedXYZ (currP, index, a);
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    }

/*---------------------------------------------------------------------------------**//**
@description Store a single coordinate for all nodes around a face, using integer index.
@param nodeP IN OUT any node in the face.
@param index IN coordinate index.  0,1,2 are x,y,z; others are adjusted cyclically.
@param a IN coordinate value to store
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  vu_setIndexedXYZAroundFace
(
VuP             nodeP,
int             index,
double          a
)
    {
    VU_FACE_LOOP (currP, nodeP)
        {
        vu_setIndexedXYZ (currP, index, a);
        }
    END_VU_FACE_LOOP (currP, nodeP)
    }

/*---------------------------------------------------------------------------------**//**
@description Search a face loop for a node at which the indexed coordinate attains its minimum value.
@remarks There is no lexical treatment of multiple such minima in the face loop, nor of nodes on the other sides of the edges of the face.
@param pSeed IN start node for face loop search
@param index IN coordinate index for ~mvu_getIndexedXYZ calls
@return pointer to any node with minimum coordinate in specified index around the face.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP vu_findIndexedMinAroundFace
(
VuP pSeed,
int index
)
    {
    double bMin = vu_getIndexedXYZ (pSeed, index);
    VuP pMin = pSeed;
    double bCurr;
    VU_FACE_LOOP (pCurr, pSeed)
        {
        bCurr = vu_getIndexedXYZ (pSeed, index);
        if (bCurr < bMin)
            {
            bMin = bCurr;
            pMin = pCurr;
            }
        }
    END_VU_FACE_LOOP (pCurr, pSeed)
    return pMin;
    }

/*---------------------------------------------------------------------------------**//**
@description Search a face loop for a node at which the indexed coordinate attains its maximum value.
@remarks There is no lexical treatment of multiple such minima in the face loop, nor of nodes on the other sides of the edges of the face.
@param pSeed IN start node for face loop search.
@param index IN coordinate index for ~mvu_getIndexedXYZ calls.
@return pointer to any node with maximum coordinate in specified index around the face.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuP vu_findIndexedMaxAroundFace
(
VuP pSeed,
int index
)
    {
    double bMax = vu_getIndexedXYZ (pSeed, index);
    VuP pMax = pSeed;
    double bCurr;
    VU_FACE_LOOP (pCurr, pSeed)
        {
        bCurr = vu_getIndexedXYZ (pSeed, index);
        if (bCurr > bMax)
            {
            bMax = bCurr;
            pMax = pCurr;
            }
        }
    END_VU_FACE_LOOP (pCurr, pSeed)
    return pMax;
    }

/*---------------------------------------------------------------------------------**//**
@description Walk around a face, assigning coordinates so each node is within half a period of its face predecessor.
@remarks It is assumed that no edge traverses more than half a period.
@param pGraph IN OUT graph header
@param pStartNode IN OUT first node of face.
@param a0 IN reference coordinate for first node.  First node is assigned within half a period of this value.
@param indexA IN index of coordinate being changed.
@param periodA IN period for changed coordinate.
@param assignAroundCompleteVertexLoop IN If true, each coordinate update is applied
          around complete vertex loop.  If false, it is only applied for the node in the face,
          and others (outside the face) in the same vertex loop are unchanged.
@return the winding value upon return to start node.  For a normal loop, this should
      be zero.  For polar loop this should be plus or minus the period.
      In any case the value will have tolerance fuzz because it is obtained by from subtraction and addition.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double vu_setPeriodicCoordinatesAroundFaceLoop
(
VuSetP pGraph,
VuP pStartNode,
double a0,
int indexA,
double periodA,
bool    assignAroundCompleteVertexLoop
)
    {
    double aRef;
    double da;
    double halfPeriod = 0.5 * periodA;
    double aCurr;

    VuP pLoopStart = vu_fsucc (pStartNode);
    /* Establish start node in period */
    aCurr = vu_getIndexedXYZ (pStartNode, indexA);
    aCurr = bsiTrig_normalizeToPeriod (aCurr, a0 - halfPeriod, periodA);
    vu_setIndexedXYZ (pStartNode, indexA, aCurr);
    aRef = aCurr;
    da = 0.0;

    VU_FACE_LOOP (pCurrNode, pLoopStart)
        {
        aCurr = vu_getIndexedXYZ (pCurrNode, indexA);
        aCurr = bsiTrig_normalizeToPeriod (aCurr, aRef - halfPeriod, periodA);

        if (assignAroundCompleteVertexLoop)
            vu_setIndexedXYZAroundVertex (pCurrNode, indexA, aCurr);
        else
            vu_setIndexedXYZ (pCurrNode, indexA, aCurr);

        da += (aCurr - aRef);
        aRef = aCurr;
        }
    END_VU_FACE_LOOP (pCurrNode, pLoopStart)

    return da;
    }

/*---------------------------------------------------------------------------------**//**
@description Query x- and y-components of the vector from the given node to its face sucessor.
@param dXP OUT x-component of vector
@param dYP OUT y-component of vector
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDXY
(
double          *dXP,
double          *dYP,
VuP             nodeP
)
    {
    VuP nextP = VU_FSUCC(nodeP);
    *dXP = VU_U (nextP) - VU_U (nodeP);
    *dYP = VU_V (nextP) - VU_V (nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query x- and y-components of the vector from the given node to its face sucessor.
@param vectorP OUT vector components
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDPoint2dDXY
(
DPoint2d        *vectorP,
VuP             nodeP
)
    {
    VuP nextP = VU_FSUCC(nodeP);
    vectorP->x = VU_U (nextP) - VU_U (nodeP);
    vectorP->y = VU_V (nextP) - VU_V (nodeP);
    }


/*---------------------------------------------------------------------------------**//**
@description Test if a node is "below" another, using secondary left right test
    to resolve cases where the nodes are on a horizontal line.
@param node0P IN first node for comparison
@param node1P IN second node for comparison
@return true iff node0P is below node1P in the lexical ordering with y-cooordinate taken as primary sort key
    and x-component as secondary sort key.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_below
(
VuP             node0P,
VuP             node1P
)
    {
    return VU_BELOW(node0P, node1P);
    }

/*---------------------------------------------------------------------------------**//**
@description Test if a node is to the left of another.
@param node0P IN first node
@param node1P IN second node
@return true iff node0P is left of node1P in the lexical ordering with x-cooordinate taken as primary sort key
    and negative y-component as secondary sort key.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_leftOf
(
VuP             node0P,
VuP             node1P
)
    {
    return VU_LEFTOF(node0P, node1P);
    }

/*---------------------------------------------------------------------------------**//**
@description Test for identical coordinates of a node.
@param node0P IN first node for comparison
@param node1P IN second node for comparison
@return true if coordinates of node0P and node1P are identical.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_sameUV
(
VuP             node0P,
VuP             node1P
)
    {
    return VU_SAME_UV(node0P, node1P);
    }

/*---------------------------------------------------------------------------------**//**
@description Compare the coordinates of two vu nodes using vertical sweep lexical ordering.
@remarks Nodes are passed with double indirection, as typcially required when a system sort routine is
    applied to an array of VU pointers.
@remarks Same as ~mvu_compareLexicalUV, only without the optional argument, so that standard qsort can use it.
@param node0PP IN first node (pointer to pointer, qsort style)
@param node1PP IN second node (pointer to pointer, qsort style)
@return -1,0,1 according to sort relationship.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int     vu_compareLexicalUV0
(
const VuP * node0PP,
const VuP * node1PP
)
    {
    int     result;
    VuP     P = *node0PP;
    VuP     Q = *node1PP;

    if (VU_V (P) < VU_V (Q))
        {
        result = -1;
        }
    else if (VU_V (P) > VU_V (Q))
        {
        result = 1;
        }
    else if (VU_U (P) < VU_U (Q))
        {
        result = -1;
        }
    else if (VU_U (P) > VU_U (Q))
        {
        result = 1;
        }
    else
        {
        result = 0;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
@description Compare the coordinates of two vu nodes using vertical sweep lexical ordering.
@remarks Nodes are passed with double indirection, as typcially required when a system sort routine is
    applied to an array of VU pointers.
@param node0PP IN first node (pointer to pointer, qsort style)
@param node1PP IN second node (pointer to pointer, qsort style)
@param optArg IN unused
@return -1,0,1 according to sort relationship.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int             vu_compareLexicalUV
(
const VuP * node0PP,
const VuP * node1PP,
const void *optArg
)
    {
    int             result;
    VuP             P = *node0PP;
    VuP             Q = *node1PP;
    if (VU_V (P) < VU_V (Q))
        {
        result = -1;
        }
    else if (VU_V (P) > VU_V (Q))
        {
        result = 1;
        }
    else if (VU_U (P) < VU_U (Q))
        {
        result = -1;
        }
    else if (VU_U (P) > VU_U (Q))
        {
        result = 1;
        }
    else
        {
        result = 0;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
@description Set the coordinates of a node.
@param nodeP IN OUT node to change
@param pointP IN new coordinates
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_setDPoint3d
(
VuP             nodeP,
DPoint3d *      pointP
)
    {
    VU_UVW(nodeP) = *pointP;
    }

/*---------------------------------------------------------------------------------**//**
@description Set coordinates of all nodes around a vertex.
@param nodeP IN OUT any node in the vertex loop
@param pointP IN point coordinates
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_setDPoint3dAroundVertex
(
VuP             nodeP,
DPoint3d *      pointP
)
    {
    VU_VERTEX_LOOP (currP, nodeP)
        {
        VU_UVW(currP) = *pointP;
        }
    END_VU_VERTEX_LOOP (currP, nodeP)
    }

/*---------------------------------------------------------------------------------**//**
@description Query the z-coordinate of a node.
@param nodeP IN node to query
@return z-coordinate of node
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_getZ
(
VuP             nodeP
)
    {
    return VU_W(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Query the coordinates of a node.
@param xCoordP OUT x-coordinate
@param yCoordP OUT y-coordinate
@param zCoordP OUT z-coordinate
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getXYZ
(
double*         xCoordP,
double*         yCoordP,
double*         zCoordP,
VuP             nodeP
)
    {
    *xCoordP = VU_U(nodeP);
    *yCoordP = VU_V(nodeP);
    *zCoordP = VU_W(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Compute coordinates at a fractional position along an edge.
@param nodeA IN base node of edge
@param fraction IN fractional coordinate
@return computed point.
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP DPoint3d vu_pointAtFraction (VuP nodeA, double fraction)
    {
    VuP nodeB = vu_fsucc (nodeA);
    double f0 = 1.0 - fraction;
    DPoint3d xyz;
    xyz.x = f0 * VU_U(nodeA) + fraction * VU_U(nodeB);
    xyz.y = f0 * VU_V(nodeA) + fraction * VU_V(nodeB);
    xyz.z = f0 * VU_W(nodeA) + fraction * VU_W(nodeB);
    return xyz;
    }

/*---------------------------------------------------------------------------------**//**
@description Query the coordinates of a node.
@param pointP OUT coordinates
@param nodeP IN node to query
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDPoint3d
(
DPoint3d *      pointP,
VuP             nodeP
)
    {
    *pointP = VU_UVW(nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the vector along an edge.
@param dXP OUT x-component of vector
@param dYP OUT y-component of vector
@param dZP OUT z-component of vector
@param nodeP IN base node of edge
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDXYZ
(
double          *dXP,
double          *dYP,
double          *dZP,
VuP             nodeP
)
    {
    VuP nextP = VU_FSUCC(nodeP);
    *dXP = VU_U (nextP) - VU_U (nodeP);
    *dYP = VU_V (nextP) - VU_V (nodeP);
    *dZP = VU_W (nextP) - VU_W (nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the vector along an edge.
@param vectorP OUT computed vector
@param nodeP IN base node of edge
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getDPoint3dDXY
(
DPoint3d        *vectorP,
VuP             nodeP
)
    {
    VuP nextP = VU_FSUCC(nodeP);
    vectorP->x = VU_U (nextP) - VU_U (nodeP);
    vectorP->y = VU_V (nextP) - VU_V (nodeP);
    vectorP->z = VU_W (nextP) - VU_W (nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the vector along an edge, with start and end node coordinates
    adjusted to be within half a period.
@param vectorP OUT computed vector
@param graphP IN graph header (provides periods)
@param nodeP IN base node of edge
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_getPeriodicDPoint3dDXYZ
(
DPoint3d        *vectorP,
VuSetP          graphP,
VuP             nodeP
)
    {
    VuP nextP = VU_FSUCC(nodeP);
    DPoint3d periods = graphP->mPrimitiveData.periods;

    vectorP->x = VU_U (nextP) - VU_U (nodeP);
    vectorP->y = VU_V (nextP) - VU_V (nodeP);
    vectorP->z = VU_W (nextP) - VU_W (nodeP);

    if (periods.x != 0.0)
        vectorP->x = bsiTrig_normalizeToPeriodAroundZero (vectorP->x, periods.x);
    if (periods.y != 0.0)
        vectorP->y = bsiTrig_normalizeToPeriodAroundZero (vectorP->y, periods.y);
    if (periods.z != 0.0)
        vectorP->z = bsiTrig_normalizeToPeriodAroundZero (vectorP->z, periods.z);
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the squared xy-distance between two nodes.
@remarks This funcion is identical to ~mvu_distanceSquared.
@param node0P IN origin of measurement
@param node1P IN target of measurement
@return the squared xy-distance between node0P and node1P.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_getXYDistanceSquared
(
VuP             node0P,
VuP             node1P
)
    {
    double dx = VU_U(node1P) - VU_U(node0P);
    double dy = VU_V(node1P) - VU_V(node0P);

    return dx * dx + dy * dy;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the xy length of an edge.
@param nodeP IN origin of measurement
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_edgeLengthXY
(
VuP             nodeP
)
    {
    return sqrt (vu_getXYDistanceSquared (nodeP, vu_fsucc(nodeP)));
    }



/*---------------------------------------------------------------------------------**//**
@description Compute the squared xy-distance between two nodes.
@remarks This funcion is identical to ~mvu_getXYDistanceSquared.
@param startP IN origin of measurement
@param endP IN target of measurement
@return the squared xy-distance between startP and endP.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_distanceSquared
(
VuP             startP,
VuP             endP
)
    {
    double dx = VU_U(endP) - VU_U(startP);
    double dy = VU_V(endP) - VU_V(startP);
    return dx*dx + dy*dy;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the 2D vector from startP to endP.
@param vectorP OUT vector from startP to endP
@param startP IN base node of vector
@param endP IN target node of vector
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_vector
(
DPoint2d *vectorP,
VuP startP,
VuP endP
)
   {
   vectorP->x = VU_U( endP ) - VU_U( startP );
   vectorP->y = VU_V( endP ) - VU_V( startP );
   }

/*---------------------------------------------------------------------------------**//**
@description Compute the 2D vector from startP to endP, adjusting coordinates so the
    vector spans no more than half a period.
@param vectorP OUT vector from startP to endP
@param graphP IN graph header (provides periods)
@param startP IN base node of vector
@param endP IN target node of vector
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_periodicVector
(
DPoint2d *vectorP,
VuSetP          graphP,
VuP startP,
VuP endP
)
    {
    DPoint3d periods = graphP->mPrimitiveData.periods;

    vectorP->x = VU_U (endP) - VU_U (startP);
    vectorP->y = VU_V (endP) - VU_V (startP);

    if (periods.x != 0.0)
        vectorP->x = bsiTrig_normalizeToPeriodAroundZero (vectorP->x, periods.x);
    if (periods.y != 0.0)
        vectorP->y = bsiTrig_normalizeToPeriodAroundZero (vectorP->y, periods.y);
    }


/*---------------------------------------------------------------------------------**//**
@description Compute the 2D cross product of vectors from node0P to node1P and node1P to node2P.
@remarks Typically, the nodes passed in satisfy, in order: vu_fpred(node), node, vu_fsucc(node).
@param node0P IN first node
@param node1P IN second node
@param node2P IN third node
@return the (scalar) cross product
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_cross
(
VuP node0P,
VuP node1P,
VuP node2P
)
    {
    double          u0, u1, u2, v0, v1, v2;
    double          du0, du1, dv0, dv1;
    VU_GET_UV (node0P, u0, v0);
    VU_GET_UV (node1P, u1, v1);
    VU_GET_UV (node2P, u2, v2);
    du0 = u1 - u0;
    dv0 = v1 - v0;
    du1 = u2 - u1;
    dv1 = v2 - v1;
    return du0 * dv1 - du1 * dv0;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+----------------------------------------------------------------------*/
double VuNode::CrossXY (VuP nodeA, VuP nodeB) const
    {
    return uv.CrossProductToPointsXY (nodeA->uv, nodeB->uv);
    }




/*---------------------------------------------------------------------------------**//**
@description Test if the angle from incoming to outgoing edge of a node is a strict left turn.
@param nodeP IN node to examine
@return true if there is a left hand turn.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_isStrictLeftTurn
(
VuP nodeP
)
    {
    return vu_cross (VU_FPRED(nodeP), nodeP, VU_FSUCC(nodeP)) > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the 2D cross product of vectors from node0P to node1P and node1P to pointP.
@param node0P IN first node
@param node1P IN second node
@param pointP IN third point
@return the (scalar) cross product
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_crossPoint
(
VuP             node0P,
VuP             node1P,
DPoint2d *      pointP
)
    {
    double          u0, u1, v0, v1;
    double          du0, du1, dv0, dv1;
    VU_GET_UV (node0P, u0, v0);
    VU_GET_UV (node1P, u1, v1);
    du0 = u1 - u0;
    dv0 = v1 - v0;
    du1 = pointP->x - u1;
    dv1 = pointP->y - v1;
    return du0 * dv1 - du1 * dv0;
    }

/*---------------------------------------------------------------------------------**//**
@description Test if a node is strictly within the sector determined by another node.
@param nodeP IN node to test
@param sectorP IN node whose incoming and outgoing edges bound the sector
@return true iff nodeP is within (but not on) the sector formed by the two edges at sectorP
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_nodeInSector
(
VuP             nodeP,
VuP             sectorP
)
    {
    VuP         succP = VU_FSUCC (sectorP);
    VuP         predP = VU_FPRED (sectorP);
    double      succCross   = vu_cross (sectorP, succP, nodeP);
    double      predCross   = vu_cross (predP, sectorP, nodeP);
    double      sectorCross = vu_cross (predP, sectorP, succP);

    bool    inSector;

    if (VU_VSUCC(sectorP) == sectorP)
        {
        /* Special case: The sector is 360 degrees. Call it in.
           (Is this safe?  I think so.  The callers should have
           prechecked the ON cases, (I think.)  Everything else is in.
         */
        inSector = true;
        }
    /* Use pred and succ cross products alone first -- this gives
        the right answer on 180 degree sectors without ever looking
        at the sector cross product (which is zero or nearly so)
    */

    else if ( predCross > 0.0 && succCross > 0.0 )
        inSector = true;
    else if (predCross <= 0.0 && succCross <= 0.0 )
        {
        if (predCross == 0.0 && succCross == 0.0 && sectorCross == 0.0)
            {
            DVec2d predVector;
            DVec2d nodeVector;
            /* Everything is on a line.*/
            /* If the sector is a degenerate face, nodeP can only be
                    in if it is the other node in the degenerate face.
            */
            if (predP == succP && vu_vsucc (sectorP) != sectorP)
                return nodeP == predP;
            /* Sector is 360 degrees.  Call it in only if vector from predP
                to sectorP points forward to nodeP.
            */
            vu_vector (&predVector, predP, sectorP);
            vu_vector (&nodeVector, sectorP, nodeP);
            return predVector.DotProduct (nodeVector) > 0.0;
            }
        inSector = false;
        }
    /* Signs are mixed */
    else
        {
        if (sectorCross == 0.0 && predCross != 0.0 && succCross != 0.0)
            {
            /* The incoming and outgoing edges at the sector are identical direction.
                We have to decide if this node is  inside the degenerate face (i.e. a geometrically empty sector)
                or outside (i.e. a nearly complete sector).
              In the inside case, the face is just two nodes.
              Exact equality for zero is ok because cross product should be using identical
                coordinates in subtracted terms.  (All furrow eyebrows in unison ....)
            */
            inSector = predP != succP;
            }
        else
            inSector = (sectorCross < 0.0);
        }
    return inSector;
    }

/*---------------------------------------------------------------------------------**//**
@description Test if a point is strictly within the sector determined by another node.
@param pointP IN point to test
@param sectorP IN node whose incoming and outgoing edges bound the sector
@return true iff pointP is within (but not on) the sector formed by the two edges at sectorP
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_pointInSector
(
DPoint2d *      pointP,
VuP             sectorP
)
    {
    VuP         succP = VU_FSUCC (sectorP);
    VuP         predP = VU_FPRED (sectorP);
    double      sectorCross;
    double      succCross = vu_crossPoint (predP, sectorP, pointP);
    double      predCross = vu_crossPoint (sectorP, succP, pointP);

    bool    inSector;

    if (VU_VSUCC(sectorP) == sectorP)
        {
        /* Special case: The sector is 180 degrees. Call it in.
           (Is this safe?  I think so.  The callers should have
           prechecked the ON cases, (I think.)  Everything else is in.
         */
        inSector = true;
        }
    /* Use pred and succ cross products alone first -- this gives
        the right answer on 180 degree sectors without ever looking
        at the sector cross product (which is zero or nearly so)
    */

    else if ( predCross > 0.0 && succCross > 0.0 )
        inSector = true;
    else if (predCross <= 0.0 && succCross <= 0.0 )
        inSector = false;
    /* Signs are mixed */
    else
        {
        sectorCross = vu_cross (predP, sectorP, succP);
        inSector = (sectorCross < 0.0);
        }
    return inSector;
    }

/*---------------------------------------------------------------------------------**//**
@description Examine the horizontal neighborhood of a node.
@remarks The return code has individual bits VU_LEFT_NEIGHBORHOOD and/or
 VU_RIGHT_NEIGHBORHOOD set according to whether points just to the left and right
 are visible in (but not on) the sector at the node.
@param nodeP IN node to examine.
@return logical OR of left and right visibility conditions on horizontal line
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP VuNeighborhoodMask   vu_horizontalNeighborhood
(
VuP             nodeP
)
    {
    VuNeighborhoodMask mask;
    VuP predP = VU_FPRED (nodeP);
    VuP succP = VU_FSUCC (nodeP);


    double y0 = VU_V (predP);
    double y1 = VU_V (nodeP);
    double y2 = VU_V (succP);

    double dy01 = y1 - y0;
    double dy12 = y2 - y1;
    double dx01;      /* Don't dig these out until needed */

    mask = 0;

    /* Do simple crossing cases as efficiently as possible.
       Do others as concisely as possible.
    */
    if (dy01 > 0.0)
        {
        if (dy12 > 0.0)
            {
            mask = VU_LEFT_NEIGHBORHOOD;
            }
        else if (dy12 < 0.0)
            {
            if (vu_cross (predP, nodeP, succP) < 0)
                mask = VU_LEFT_NEIGHBORHOOD | VU_RIGHT_NEIGHBORHOOD;
            }
        else
            {
            if (VU_U (succP) > VU_U (nodeP))
                mask = VU_LEFT_NEIGHBORHOOD;
            }
        }
    else if (dy01 < 0.0)
        {
        if (dy12 < 0.0)
            {
            mask = VU_RIGHT_NEIGHBORHOOD;
            }
        else if (dy12 > 0.0)
            {
            if (vu_cross (predP, nodeP, succP) < 0)
                mask = VU_LEFT_NEIGHBORHOOD | VU_RIGHT_NEIGHBORHOOD;
            }
        else
            {
            if (VU_U (succP) < VU_U (nodeP))
                mask = VU_RIGHT_NEIGHBORHOOD;
            }
        }
    else
        {
        /* First line is horizontal */
        dx01 = VU_U (nodeP) - VU_U (predP);

        if (dx01 > 0.0)
            {
            if (dy12 < 0.0)
                mask = VU_RIGHT_NEIGHBORHOOD;
            }
        else if (dx01 < 0.0)
            {
            if (dy12 > 0.0)
                mask = VU_LEFT_NEIGHBORHOOD;
            }
        }

    return mask;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the xy-intersection of two edges.
@param fractionAP OUT fractional parameter of intersection along edge starting at nodeAP
@param fractionBP OUT fractional parameter of intersection along edge starting at nodeBP
@param xyzAP OUT xyz coordinates of intersection along edge starting at nodeAP
@param xyzBP OUT xyz coordinates of intersection along edge starting at nodeBP
@param nodeAP IN base node of first edge
@param nodeBP IN base node of second edge
@return true if an intersection is computed
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    vu_intersectXYEdges
(
double          *fractionAP,
double          *fractionBP,
DPoint3d        *xyzAP,
DPoint3d        *xyzBP,
VuP             nodeAP,
VuP             nodeBP
)
    {
    VuP nodeCP, nodeDP;
    DPoint3d vectorAC;
    DPoint3d vectorBD;
    DPoint3d vectorAB;
    DPoint3d xyzA, xyzC;
    DPoint3d xyzB, xyzD;
    double fractionA, fractionB;
    bool    boolstat = false;
    nodeCP = VU_FSUCC (nodeAP);
    nodeDP = VU_FSUCC (nodeBP);

    xyzA = VU_UVW(nodeAP);
    xyzB = VU_UVW(nodeBP);
    xyzC = VU_UVW(nodeCP);
    xyzD = VU_UVW(nodeDP);

    vectorAC.DifferenceOf (xyzC, xyzA);
    vectorBD.DifferenceOf (xyzD, xyzB);
    vectorAB.DifferenceOf (xyzB, xyzA);
    if (bsiSVD_solve2x2
                (
                &fractionA, &fractionB,
                vectorAC.x, -vectorBD.x,
                vectorAC.y, -vectorBD.y,
                vectorAB.x, vectorAB.y
                ))
        {
        boolstat = true;
        }
    else
        {
        fractionA = fractionB = 0.0;
        boolstat = false;
        }

    if (fractionAP)
        *fractionAP = fractionA;
    if (fractionBP)
        *fractionBP = fractionB;

    if (xyzAP)
        xyzAP->SumOf (xyzA, vectorAC, fractionA);
    if (xyzBP)
        xyzBP->SumOf (xyzB, vectorBD, fractionB);

    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
@description Project a point onto the unbounded line containing an edge.
@remarks if the edge is degenerate (single point), the projection is to that point and
    the parameter is zero.
@remarks Only xy parts of coordinates are used.
@param fractionAP OUT fractional parameter of projection along edge starting at nodeAP
@param xyzOutP OUT xyz coordinates of projection along edge starting at nodeAP
@param nodeAP IN base node of edge
@param xyzP IN point to project onto edge
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_projectToUnboundedXYEdge
(
double          *fractionAP,
DPoint3d        *xyzOutP,
VuP             nodeAP,
const DPoint3d  *xyzP
)
    {
    VuP nodeBP;
    DPoint3d vectorAB, vectorAC;
    DPoint3d xyzA, xyzB;
    double dotABAB, dotABAC;
    double fractionA;

    nodeBP = VU_FSUCC (nodeAP);
    xyzA = VU_UVW(nodeAP);
    xyzB = VU_UVW(nodeBP);

    vectorAB.DifferenceOf (xyzB, xyzA);
    vectorAC.DifferenceOf (*xyzP, xyzA);
    dotABAB = vectorAB.DotProductXY (vectorAB);
    dotABAC = vectorAB.DotProductXY (vectorAC);
    DoubleOps::SafeDivide (fractionA, dotABAC, dotABAB, 0.0);

    if (fractionAP)
        *fractionAP = fractionA;
    if (xyzOutP)
        xyzOutP->SumOf (xyzA, vectorAB, fractionA);
    }

/*---------------------------------------------------------------------------------**//**
@description Project a point onto the (bounded) edge.
@remarks Only xy parts of coordinates are used.
@param fractionAP OUT fractional parameter of projection along edge starting at nodeAP
@param xyzOutP OUT xyz coordinates of projection along edge starting at nodeAP
@param nodeAP IN base node of edge
@param xyzP IN point to project onto edge
@return true if projection was within the closed [0,1] fractional range.
      In case of false return, the fraction and distance are still valid, but are capped to
      the nearest endpoint.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    vu_projectToBoundedXYEdge
(
double          *fractionAP,
DPoint3d        *xyzOutP,
VuP             nodeAP,
const DPoint3d  *xyzP
)
    {
    double fraction;
    DPoint3d xyz;
    bool    projectionBounded = true;

    vu_projectToUnboundedXYEdge (&fraction, &xyz, nodeAP, xyzP);

    if (fraction < 0.0)
        {
        fraction = 0.0;
        xyz = VU_UVW (nodeAP);
        projectionBounded = false;
        }
    else if (fraction > 1.0)
        {
        fraction = 1.0;
        xyz = VU_UVW (VU_FSUCC (nodeAP));
        projectionBounded = false;
        }

    if (fractionAP)
        *fractionAP = fraction;
    if (xyzOutP)
        *xyzOutP = xyz;
    return projectionBounded;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the point at the given fractional parameter along an edge.
@param xyzAP OUT xyz coordinates of point along the edge starting at nodeAP
@param nodeAP IN base node of edge.
@param fractionA IN fractional parameter
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_fractionParameterToDPoint3d
(
DPoint3d        *xyzAP,
VuP             nodeAP,
double          fractionA
)
    {
    VuP nodeBP = VU_FSUCC (nodeAP);
    xyzAP->x = VU_U(nodeAP) + fractionA * (VU_U(nodeBP) - VU_U(nodeAP));
    xyzAP->y = VU_V(nodeAP) + fractionA * (VU_V(nodeBP) - VU_V(nodeAP));
    xyzAP->z = VU_W(nodeAP) + fractionA * (VU_W(nodeBP) - VU_W(nodeAP));
    }

/*---------------------------------------------------------------------------------**//**
@description Attempt to interpolate the z-coordinate of a node from those of its neighbors.
@remarks The interpolation is according to the relative xy distances along the incoming and outgoing edges.
@remarks This is most commonly used just after splitting an edge at an internal point, hence the distance logic amounts to simple
    interpolation.
@remarks If the two nodes have identical coordinates, the z-coordinate from the predecessor is installed and the return value is ERROR.
@remarks The formula computes xy-distance along each adjacent edge (i.e., takes sqrt), so it produces a z-coordinate even if the node
    is in a corner.  If doing pure parametric subdivision, there may be a performance benefit to using the parameter directly, as the
    xy-coordinates are computed.
@param nodeP IN OUT node to update
@return SUCCESS if z-coordinate was computed, ERROR if coincident nodes.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    vu_interpolateZFromFaceNeighbors
(
VuP             nodeP
)
    {
    int status = SUCCESS;
    VuP node1P = VU_FPRED(nodeP);
    VuP node2P = VU_FSUCC(nodeP);
    double d1 = sqrt (vu_getXYDistanceSquared(nodeP, node1P));
    double d2 = sqrt (vu_getXYDistanceSquared(nodeP, node2P));
    double dt = d1 + d2;
    double s;

    /* Interpolate using the nearer node as tbe base point in the interpolation formula */
    if (dt == 0.0)
        {
        status = ERROR;
        VU_W(nodeP) = VU_W(node1P);
        }
    else if (d2 > d1)
        {
        s = d1 / dt;
        VU_W(nodeP) = VU_W(node1P) + s * (VU_W(node2P) - VU_W(node1P));
        }
    else
        {
        s = d2 / dt;
        VU_W(nodeP) = VU_W(node2P) + s * (VU_W(node1P) - VU_W(node2P));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
@description Search the graph for the maximal absolute value of any xy-coordinate.
@param graphP IN graph header.
@return maximum x or y found in graph.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double     vu_maxAbsXYInGraph
(
VuSetP          graphP
)
    {
    double aMax = 0.0;
    double a;
    VU_SET_LOOP (currP, graphP)
        {
        a = fabs (VU_U(currP));
        if (a > aMax)
            aMax = a;
        a = fabs (VU_V(currP));
        if (a > aMax)
            aMax = a;
        }
    END_VU_SET_LOOP (currP, graphP)
    return aMax;
    }

/*---------------------------------------------------------------------------------**//**
@description Return a tolerance that is appropriate to the size of coordinates in the graph.
@param graphP IN graph header
@param abstol IN absolute tolerance
@param reltol IN relative tolerance
@return the larger of the given abstol, and the given reltol times the maximum xy-coordinate in the graph
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double     vu_toleranceFromGraphXY
(
VuSetP          graphP,
double          abstol,
double          reltol
)
    {
    double aMax = vu_maxAbsXYInGraph (graphP);
    double a = reltol * aMax;
    if (abstol > a)
        a = abstol;
    return a;
    }

/*---------------------------------------------------------------------------------**//**
@description Collect the array of nodes which are local extrema in either x or y.
@remarks A local extrema is determined by lexical comparison to immediate neighbors.
@remarks Nodes masked by VU_EXTERIOR_EDGE are not considered.
@param nodeArrayP OUT array of extremal nodes
@param graphP IN graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_collectLocalExtrema
(
VuArrayP        nodeArrayP,
VuSetP          graphP
)
    {
    VuMask          mVisited = vu_grabMask (graphP);
    VuMask          mBoth = mVisited | VU_EXTERIOR_EDGE;
    vu_clearMaskInSet (graphP, mVisited);
    vu_arrayClear (nodeArrayP);
    VU_SET_LOOP (faceP, graphP)
        {
        if (!VU_GETMASK (faceP, mBoth))
            {
            double          du0 = VU_dU (faceP);
            double          dv0 = VU_dV (faceP);
            double          du1, dv1, uProd, vProd;
            int             change;
            VuP             startP = VU_FSUCC (faceP);
            /* Find all nodes on this face where adjacent
               edge vectors show sign change
             */
            VU_FACE_LOOP (currP, startP)
                {
                VU_SETMASK (currP, mVisited);
                du1 = VU_dU (currP);
                dv1 = VU_dV (currP);
                uProd = du0 * du1;
                vProd = dv0 * dv1;
                change =
                    uProd < 0.0
                    || vProd < 0.0
                    || (uProd == 0.0 && (du0 != 0.0 || du1 != 0.0))
                    || (vProd == 0.0 && (dv0 != 0.0 || dv1 != 0.0));
                if (change)
                    {
                    vu_arrayAdd (nodeArrayP, currP);
                    }
                /* Advance the lagged vector */
                du0 = du1;
                dv0 = dv1;
                }
            END_VU_FACE_LOOP (currP, startP)
            }
        }
    END_VU_SET_LOOP (faceP, graphP)
        vu_returnMask (graphP, mVisited);
    }

/*---------------------------------------------------------------------------------**//**
@description Collect arrays of nodes which are extremal in their v- (y-) coordinates.
@remarks A local extrema is determined by lexical comparison to immediate neighbors.
@param minArrayP OUT array of local min-v nodes.
@param maxArrayP OUT array of local max-v nodes.
@param graphP IN graph header.
@param mExclude IN mask for edges to be EXCLUDED from acting as starts of face loops.  Typically, this is VU_EXTERIOR_EDGE.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_collectVExtrema
(
VuArrayP        minArrayP,
VuArrayP        maxArrayP,
VuSetP          graphP,
VuMask          mExclude
)
    {
    VuMask          mVisited = vu_grabMask (graphP);
    VuMask          mNoStart = mVisited | mExclude;
    int lexComp0,lexComp1;

    vu_clearMaskInSet (graphP, mVisited);
    vu_arrayClear (minArrayP);
    vu_arrayClear (maxArrayP);
    VU_SET_LOOP (faceP, graphP)
        {
        if (!VU_GETMASK (faceP, mNoStart))
            {
            VuP startP = VU_FSUCC (faceP);
            VuP nextP;
            lexComp0 = vu_compareLexicalUV(&faceP,&startP, NULL);
            /* Find all nodes on this face where adjacent
               edge vectors show sign change
             */
            VU_FACE_LOOP (currP, startP)
                {
                VU_SETMASK (currP, mVisited);
                nextP = VU_FSUCC(currP);
                lexComp1 = vu_compareLexicalUV(&currP,&nextP, NULL);

                if (lexComp0 == lexComp1)
                    {
                    /* Just continue on.  This test first because it
                    is the most common case */
                    }
                else if (lexComp0 > 0)
                    {
                    vu_arrayAdd(maxArrayP,currP);
                    }
                else if (lexComp1 > 0)
                    {
                    vu_arrayAdd(minArrayP,currP);
                    }

                lexComp0 = lexComp1;
                }
            END_VU_FACE_LOOP (currP, startP)
            }
        }
    END_VU_SET_LOOP (faceP, graphP)
    vu_returnMask (graphP, mVisited);
    }

/*---------------------------------------------------------------------------------**//**
@description Search a graph for nodes with extremal coordinates.
@param graphP IN graph header.
@param minUP OUT node with minimum according to ~mvu_below.
@param maxUP OUT node with maximum according to ~mvu_below.
@param minVP OUT node with minimum according to ~mvu_leftOf.
@param maxVP OUT node with maximum according to ~mvu_leftOf.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_findExtremaInGraph
(
VuSetP         graphP,
VuP            *minUP,
VuP            *maxUP,
VuP            *minVP,
VuP            *maxVP
)
    {
    VuP startP = VU_ANY_NODE_IN_GRAPH( graphP );
    _Analysis_assume_(startP != nullptr);
    VuP             lowUP = startP;
    VuP             hiUP = startP;
    VuP             lowVP = startP;
    VuP             hiVP = startP;
    VU_SET_LOOP (currP, graphP)
        {
        if (VU_BELOW (currP, lowVP))
            {
            lowVP = currP;
            }
        else if (VU_BELOW (hiVP, currP))
            {
            hiVP = currP;
            }
        if (VU_LEFTOF (currP, lowUP))
            {
            lowUP = currP;
            }
        else if (VU_LEFTOF (hiUP, currP))
            {
            hiUP = currP;
            }
        }
    END_VU_SET_LOOP (currP, graphP)
    *minUP = lowUP;
    *maxUP = hiUP;
    *minVP = lowVP;
    *maxVP = hiVP;
    }

/*---------------------------------------------------------------------------------**//**
@description Search a single face for nodes with extremal coordinates.
@param startP IN any node on the face.
@param minUP OUT node with minimum according to ~mvu_below.
@param maxUP OUT node with maximum according to ~mvu_below.
@param minVP OUT node with minimum according to ~mvu_leftOf.
@param maxVP OUT node with maximum according to ~mvu_leftOf.
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_findExtrema
(
VuP             startP,         /* <= any start vu on the face */
VuP            *minUP,
VuP            *maxUP,
VuP            *minVP,
VuP            *maxVP
)
    {
    VuP             lowUP = startP;
    VuP             hiUP = startP;
    VuP             lowVP = startP;
    VuP             hiVP = startP;
    VU_FACE_LOOP (currP, startP)
        {
        if (VU_BELOW (currP, lowVP))
            {
            lowVP = currP;
            }
        else if (VU_BELOW (hiVP, currP))
            {
            hiVP = currP;
            }
        if (VU_LEFTOF (currP, lowUP))
            {
            lowUP = currP;
            }
        else if (VU_LEFTOF (hiUP, currP))
            {
            hiUP = currP;
            }
        }
    END_VU_FACE_LOOP (currP, startP)
    *minUP = lowUP;
    *maxUP = hiUP;
    *minVP = lowVP;
    *maxVP = hiVP;
    }

/*---------------------------------------------------------------------------------**//**
@description Rotate all x- and y-coordinates in the graph by 180 degrees around the origin, (i.e., negate each x and y coordinate).
@param graphP IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_rotate180
(
VuSetP          graphP
)
    {
    VU_SET_LOOP (currP, graphP)
        {
        VU_U( currP ) = - VU_U( currP );
        VU_V( currP ) = - VU_V( currP );
        }
    END_VU_SET_LOOP (currP, graphP)
    }

/*---------------------------------------------------------------------------------**//**
@description Apply the given Transform to the xyz-coordinates in each node.
@param graphP IN OUT graph header
@param transformP IN transform to apply
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_transform
(
VuSetP          graphP,
TransformCP transformP
)
    {
    DPoint3d point;
    VU_SET_LOOP (currP, graphP)
        {
        transformP->Multiply (point, VU_U (currP), VU_V (currP), VU_W (currP));
        VU_U (currP) = point.x;
        VU_V (currP) = point.y;
        VU_W (currP) = point.z;
        }
    END_VU_SET_LOOP (currP, graphP)
    }



/*---------------------------------------------------------------------------------**//**
@description Apply the 2d parts of the given DTransform3d to the xy-coordinates in each node.
@param graphP IN OUT graph header
@param transformP IN transform to apply
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_transform2d
(
      VuSetP          graphP,
const Transform  *transformP
)
    {
    double x,y;
    double a = transformP->form3d[0][3];
    double ax = transformP->form3d[0][0];//matrix.column[0].x;
    double ay = transformP->form3d[0][1];//matrix.column[1].x;

    double b = transformP->form3d[1][3];//translation.y;
    double bx = transformP->form3d[1][0];//matrix.column[0].y;
    double by = transformP->form3d[1][1];//matrix.column[1].y;

    VU_SET_LOOP (currP, graphP)
        {
        x = VU_U (currP);
        y = VU_V (currP);

        VU_U( currP ) = a + ax * x + ay * y;
        VU_V( currP ) = b + bx * x + by * y;
        }
    END_VU_SET_LOOP (currP, graphP)
    }

/*---------------------------------------------------------------------------------**//**
@description Swap x- and y-periods in the graph header.
@param graphP IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
static void vu_swapPeriods
(
VuSetP graphP
)
    {
    DPoint3d periods;
    double a;
    vu_getPeriods (graphP, &periods);
    a = periods.x;
    periods.x = periods.y;
    periods.y = a;
    vu_setPeriods (graphP, &periods);
    }

/*---------------------------------------------------------------------------------**//**
@description Rotate the xy-coordinates of all nodes by 90 degrees counter-clockwise around the origin.
@param graphP IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_rotate90CCW
(
VuSetP          graphP
)
    {
    double          a;
    VU_SET_LOOP (currP, graphP)
        {
        a = VU_V (currP);
        VU_V (currP) = VU_U (currP);
        VU_U (currP) = -a;
        }
    END_VU_SET_LOOP (currP, graphP)
    vu_swapPeriods (graphP);
    }

/*---------------------------------------------------------------------------------**//**
@description Rotate the xy-coordinates of all nodes by 90 degrees clockwise around the origin.
@param graphP IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     vu_rotate90CW
(
VuSetP          graphP
)
    {
    double          a;
    VU_SET_LOOP (currP, graphP)
        {
        a = VU_U (currP);
        VU_U (currP) = VU_V (currP);
        VU_V (currP) = -a;
        }
    END_VU_SET_LOOP (currP, graphP)
    vu_swapPeriods (graphP);
    }

/*---------------------------------------------------------------------------------**//**
@description Rotate the xy-coordinates of all nodes by the specified angle around the origin.
@param graphP IN OUT graph header
@param theta IN rotation angle in radians, measured counter-clockwise from global x-axis
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_rotateGraph
(
VuSetP graphP,
double theta
)
    {
    double cc = cos (theta);
    double ss = sin (theta);
    double xx, yy;
    VU_SET_LOOP (currP, graphP)
        {
        xx = VU_U (currP);
        yy = VU_V (currP);
        VU_U(currP) = xx * cc - yy * ss;
        VU_V(currP) = xx * ss + yy * cc;
        }
    END_VU_SET_LOOP (currP, graphP)
    }

/*---------------------------------------------------------------------------------**//**
@description Set the z coordinate in all nodes in the graph.
@param [in] graphP graph header
@group [in] z "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_setZInGraph
(
VuSetP graphP,
double z
)
    {
    VU_SET_LOOP (currP, graphP)
        {
        DPoint3d xyz;
        vu_getDPoint3d (&xyz, currP);
        xyz.z = z;
        vu_setDPoint3d (currP, &xyz);
        }
    END_VU_SET_LOOP (currP, graphP)
    }

static void search_fixPeriodsThisFace
(
VuSetP pGraph,
VuArrayP pStack,
VuP pSeedNode,
DPoint3d *pPeriods,
VuMask visitMask
)
    {
    VuP pMateNode;
    DPoint3d currXYZ, previousXYZ, mateXYZ;
    double halfX = pPeriods->x * 0.5;
    double halfY = pPeriods->y * 0.5;

    vu_setMaskAroundFace (pSeedNode, visitMask);
    vu_getDPoint3d (&previousXYZ, pSeedNode);

    VU_FACE_LOOP (pCurrNode, pSeedNode)
        {
        vu_getDPoint3d (&currXYZ, pCurrNode);
        pMateNode = vu_edgeMate (pCurrNode);
        if (pPeriods->x != 0.0)
            currXYZ.x = bsiTrig_normalizeToPeriod (currXYZ.x, previousXYZ.x - halfX, pPeriods->x);
        if (pPeriods->y != 0.0)
            currXYZ.y = bsiTrig_normalizeToPeriod (currXYZ.y, previousXYZ.y - halfY, pPeriods->y);

        vu_setDPoint3d (pCurrNode, &currXYZ);
        if (!vu_getMask (pMateNode, visitMask))
            {
            /* Push it on the stack, and force mate coordinates to be near here.
                If this is actually the first touch of that face, our coordinates will win.
             */
            vu_arrayAdd (pStack, pMateNode);
            vu_getDPoint3d (&mateXYZ, pMateNode);
            if (pPeriods->x != 0.0)
                mateXYZ.x = bsiTrig_normalizeToPeriod (mateXYZ.x, currXYZ.x - halfX, pPeriods->x);
            if (pPeriods->y != 0.0)
                mateXYZ.y = bsiTrig_normalizeToPeriod (mateXYZ.y, currXYZ.y - halfY, pPeriods->y);
            vu_setDPoint3d (pMateNode, &mateXYZ);
            }
        previousXYZ = currXYZ;
        }
    END_VU_FACE_LOOP (pCurrNode, pSeedNode)
    }

/*---------------------------------------------------------------------------------**//**
@description Sum steps around a face loop, applying periodic adjustments to bring each xy-coordinate to
    the period centered on its predecessor.
@param pGraph IN OUT graph header (provides periods)
@param pSeed IN OUT start node of face
@param index IN coordinate selector: 0 or 1
@return sum of periodic steps
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double vu_sumIndexedPeriodicStepsAroundLoop
(
VuSetP pGraph,
VuP pSeed,
int index
)
    {
    double sum = 0.0;
    double x0, x1, dx;
    DPoint3d periods;
    double period;
    vu_getPeriods (pGraph, &periods);
    period = index == 0 ? periods.x : periods.y;

    x0 = vu_getIndexedXYZ (vu_fpred (pSeed), index);
    VU_FACE_LOOP (pCurr, pSeed)
        {
        x1 = vu_getIndexedXYZ (pCurr, index);
        dx = bsiTrig_normalizeToPeriodAroundZero (x1 - x0, period);
        sum += dx;
        x0 = x1;
        }
    END_VU_FACE_LOOP (pCurr, pSeed)
    return sum;
    }

/*---------------------------------------------------------------------------------**//**
@description Within each face of the graph, apply periodic logic to bring nodes close to the same x- and/or y-periods as their
    face predecessor; except for faces which wrap around a pole this makes each face internally self-consistent with
    respect to the period.
@remarks The search process chooses unvisited faces in arbitrary order; once a start is chosen, a depth-first search
    order is used, so faces are assigned to a period based on a neighboring face.
@param pGraph IN OUT graph header
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_setPeriodicCoordinatesWithinFaces
(
VuSetP pGraph
)
    {
    DPoint3d periods;
    VuMask seedMask;
    VuArrayP pStack = vu_grabArray (pGraph);

    vu_getPeriods (pGraph, &periods);

    if (periods.x != 0.0 || periods.y != 0.0)
        {
        VuMask visitMask = vu_grabMask (pGraph);
        vu_clearMaskInSet (pGraph, visitMask);
        seedMask = visitMask | VU_EXTERIOR_EDGE;
        VU_SET_LOOP (pCurr, pGraph)
            {
            /* Only start at INTERIOR UNVISITED */
            if (!vu_getMask (pCurr, visitMask))
                {
                VuP pSearchNode;
                vu_arrayAdd (pStack, pCurr);
                while (NULL != (pSearchNode = vu_arrayRemoveLast (pStack)))
                    {
                    search_fixPeriodsThisFace (pGraph, pStack, pSearchNode, &periods, visitMask);
                    }
                }
            }
        END_VU_SET_LOOP (pCurr, pGraph)
        vu_returnMask (pGraph, visitMask);
        }

    vu_returnArray (pGraph, pStack);
    }

/*---------------------------------------------------------------------------------**//**
@description Set all xy-coordinates to within one half period of the coordinates of a reference point.
@param pGraph IN graph header (provides periods)
@param pXYZ0 IN reference point coordinates
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_setPeriodicCoordinatesAroundReferencePoint
(
VuSetP pGraph,
DPoint3d *pXYZ0
)
    {
    DPoint3d periods;
    double x0, y0;
    DPoint3d currXYZ;
    vu_getPeriods (pGraph, &periods);

    x0 = pXYZ0->x - periods.x * 0.5;
    y0 = pXYZ0->y - periods.y * 0.5;

    if (periods.x != 0.0 || periods.y != 0.0)
        {
        VU_SET_LOOP (pCurr, pGraph)
            {
            vu_getDPoint3d (&currXYZ, pCurr);
            if (periods.x != 0.0)
                currXYZ.x = bsiTrig_normalizeToPeriod (currXYZ.x, x0, periods.x);
            if (periods.y != 0.0)
                currXYZ.y = bsiTrig_normalizeToPeriod (currXYZ.y, y0, periods.y);
            vu_setDPoint3d (pCurr, &currXYZ);
            }
        END_VU_SET_LOOP (pCurr, pGraph)
        }

    }

/*----------------------------------------------------------------------+
@description Examine the periodic extend of a loop in a given direction.
  Increment counters and record the pointer if periodic.
+----------------------------------------------------------------------*/
static void periodicLoopCandidate
(
VuSetP pGraph,
VuP    pSeed,
int    index,
double tol,
VuArrayP pArray,
int *pNumInterior,
int *pNumExterior
)
    {
    double delta = vu_sumIndexedPeriodicStepsAroundLoop (pGraph, pSeed, index);
    if (fabs (delta) > tol)
        {
        vu_arrayAdd (pArray, pSeed);
        if (vu_getMask (pSeed, VU_EXTERIOR_EDGE))
            {
            if (pNumInterior)
                *pNumInterior += 1;
            if (pNumExterior)
                *pNumExterior += 1;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Count and record faces which traverse a pole.
@param pGraph IN graph header
@param pXArray OUT array of seeds to faces whose x-coordinates circle a pole.
@param pNumInteriorX OUT number of x-polar faces whose seed is not marked exterior.
@param pNumExteriorX OUT number of x-polar faces whose seed is marked exterior.
@param pYArray OUT array of seeds to faces whose y-coordinates circle a pole.
@param pNumInteriorY OUT number of y-polar faces whose seed is not marked exterior.
@param pNumExteriorY OUT number of y-polar faces whose seed is marked exterior.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_countPolarLoops
(
VuSetP pGraph,
VuArrayP pXArray,
int *pNumInteriorX,
int *pNumExteriorX,
VuArrayP pYArray,
int *pNumInteriorY,
int *pNumExteriorY
)
    {
    DPoint3d periods;
    bool    xLoopsMatter, yLoopsMatter;
    double xTol, yTol;
    static double s_relTol = 1.0e-12;
    vu_getPeriods (pGraph, &periods);
    /* Remark: We trust that vu_array funcs will tolerance NULL array pointers. */
    vu_arrayClear (pXArray);
    vu_arrayClear (pYArray);

    xLoopsMatter = periods.x != 0.0 && (pNumInteriorX || pNumExteriorX || pXArray);
    yLoopsMatter = periods.y != 0.0 && (pNumInteriorY || pNumExteriorY || pYArray);;

    if (pNumInteriorX)
        *pNumInteriorX = 0;

    if (pNumExteriorX)
        *pNumExteriorX = 0;

    if (pNumInteriorY)
        *pNumInteriorY = 0;

    if (pNumExteriorY)
        *pNumExteriorY = 0;

    xTol = s_relTol * periods.x;
    yTol = s_relTol * periods.y;

    if (xLoopsMatter || yLoopsMatter)
        {
        VuMask visitMask = vu_grabMask (pGraph);
        vu_clearMaskInSet (pGraph, visitMask);
        VU_SET_LOOP (pCurr, pGraph)
            {
            if (!vu_getMask (pCurr, visitMask))
                {
                vu_setMaskAroundFace (pCurr, visitMask);
                if (xLoopsMatter)
                    periodicLoopCandidate (pGraph, pCurr, 0, xTol, pXArray, pNumInteriorX, pNumExteriorX);
                if (yLoopsMatter)
                    periodicLoopCandidate (pGraph, pCurr, 1, yTol, pYArray, pNumInteriorY, pNumExteriorY);
                }
            }
        END_VU_SET_LOOP (pCurr, pGraph)
        vu_returnMask (pGraph, visitMask);
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the area of a face.
@param startP IN any node around the face
@return face area
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_area
(
VuP             startP
)
    {
    double          area = 0;

    if (! startP)
        return  area;
    // Eliminate any chance of last bit fuzz on trivial faces ...
    if (VU_FSUCC(VU_FSUCC(startP)) == startP)
        return 0.0;

    VU_FACE_LOOP (P, startP)
        {
        VuP             Q = VU_FSUCC (P);
        area -= (VU_U (Q) - VU_U (P)) * (VU_V (P) + VU_V (Q));
        }
    END_VU_FACE_LOOP (P, startP)

    return area * 0.5;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute face area, simultaneously setting a mask bit around the face.
@param startP IN any node around the face
@param mask IN mask to set
@return face area
@group "VU Coordinates"
@see vu_setMaskByArea, vu_markExteriorFacesByArea
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   vu_markFaceAndComputeArea
(
VuP             startP,
VuMask          mask
)
    {
    double          area = 0;
    VU_FACE_LOOP (P, startP)
        {
        VuP             Q = VU_FSUCC (P);
        area -= (VU_U (Q) - VU_U (P)) * (VU_V (P) + VU_V (Q));
        VU_SETMASK( P, mask );
        }
    END_VU_FACE_LOOP (P, startP)
    return area * 0.5;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the centroid and area of a face.
@param uvP OUT centroid of the face.  If zero area, coordinates of start node are used.
@param areaP OUT area of the face
@param nPosP OUT number of positive area triangles as seen from the start node
@param nNegP OUT number of negative area triangles as seen from the start node
@param startP IN any node around the face
@return SUCCESS if the face has nonzero area, ERROR otherwise.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    vu_centroid
(
DPoint2d*       uvP,
double          *areaP,
int             *nPosP,
int             *nNegP,
VuP             startP
)
    {
    double      area = 0.0;
    double      sumX = 0.0;
    double      sumY = 0.0;
    double      xc, yc, dA;
    VuP node0P, node1P;
    int status = ERROR;
    int nPos = 0;
    int nNeg = 0;

    uvP->x = VU_U(startP);
    uvP->y = VU_V(startP);

    node0P = VU_FSUCC(startP);
    node1P = VU_FSUCC(node0P);

    // rule out 1 and 2 edge faces
    while (node1P != startP)
        {
        xc = (VU_U(startP) + VU_U(node0P) + VU_U(node1P)) / 3.0;
        yc = (VU_V(startP) + VU_V(node0P) + VU_V(node1P)) / 3.0;
        dA = 0.5*vu_cross (startP, node0P, node1P);
        sumX += dA * xc;
        sumY += dA * yc;
        area += dA;
        if (dA > 0.0)
            {
            nPos++;
            }
        else
            {
            nNeg++;
            }
        node0P = node1P;
        node1P = VU_FSUCC(node1P);
        }

    if (area != 0.0)
        {
        uvP->x = sumX / area;
        uvP->y = sumY / area;
        area *= 0.5;
        status = SUCCESS;
        }

    *areaP = area;
    *nPosP = nPos;
    *nNegP = nNeg;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the centroid and area of a face, using a specified origin for
    generating the triangles whose signed areas are summed.
@param uvP OUT centroid of face.  If zero area, coordinates of start node are used.
@param areaP OUT area of face
@param nPosP OUT number of positive area triangles as seen from specified origin
@param nNegP OUT number of negative area triangles as seen from specified origin
@param originP IN origin for triangles
@param startP IN any node around the face
@return SUCCESS if the face has nonzero area, ERROR otherwise.
@group "VU Coordinates"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  vu_centroidFromOrigin
(
DPoint2d*       uvP,
double          *areaP,
int             *nPosP,
int             *nNegP,
const DPoint2d *originP,
VuP             startP
)
    {
    double      area = 0.0;
    double      sumX = 0.0;
    double      sumY = 0.0;
    double      xc, yc, dA;
    DPoint2d    uv0, uv1;
    int status = ERROR;
    int nPos = 0;
    int nNeg = 0;

    uvP->x = VU_U(startP);
    uvP->y = VU_V(startP);

    // rule out 1 and 2 edge faces
    if (VU_FSUCC (VU_FSUCC (startP)) != startP)
        {
        vu_getDPoint2d (&uv0, startP);
        uv0.x -= originP->x;
        uv0.y -= originP->y;

        VU_FACE_LOOP (node0P, startP)
            {
            VuP node1P = VU_FSUCC (node0P);
            vu_getDPoint2d (&uv1, node1P);
            uv1.x -= originP->x;
            uv1.y -= originP->y;
            xc = (uv0.x + uv1.x) / 3.0;
            yc = (uv0.y + uv1.y) / 3.0;
            dA = uv0.CrossProduct (uv1);
            sumX += dA * xc;
            sumY += dA * yc;
            area += dA;
            if (dA > 0.0)
                {
                nPos++;
                }
            else
                {
                nNeg++;
                }
            uv0 = uv1;
            }
        END_VU_FACE_LOOP (node0P, startP)

        if (area != 0.0)
            {
            uvP->x = sumX / area;
            uvP->y = sumY / area;
            area *= 0.5;
            status = SUCCESS;
            }
        }

    *areaP = area;
    *nPosP = nPos;
    *nNegP = nNeg;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
@description Test if the specified coordinates of an edge are in the specified closed interval.
@param startP IN base node of edge
@param axisSelect IN component to inspect (0 for u-coordinate, 1 for v-coordinate)
@param cMin IN min coordinate of band
@param cMax IN max coordinate of band
@return true iff the edge is completely within the band
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     vu_isEdgeInBand
(
VuP startP,
int axisSelect,
double cMin,
double cMax
)
    {
    bool inBand = false;
    VuP endP;

    if( axisSelect == 0 )
        {
        if( IN_RANGE(VU_U(startP), cMin, cMax) )
            {
            endP = VU_FSUCC(startP);
            inBand = IN_RANGE( VU_U(endP), cMin, cMax );
            }
        }
    else
        {
        if( IN_RANGE(VU_V(startP), cMin, cMax) )
            {
            endP = VU_FSUCC(startP);
            inBand = IN_RANGE( VU_V(endP), cMin, cMax );
            }
        }
    return inBand;
    }

/*---------------------------------------------------------------------------------**//**
* @description Search a vu graph and mark those edges whose specified coordinates are in the
*       specified closed interval.
* @param graphP       IN OUT  graph header
* @param axisSelect   IN      component to inspect (0 for u-coordinate, 1 for v-coordinate)
* @param cMin         IN      min coordinate of band
* @param cMax         IN      max coordinate of band
* @param mask         IN      mask to apply to nodes of edges within the band
* @group "VU Coordinates"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_markEdgesInBand
(
VuSetP graphP,
int axisSelect,
double cMin,
double cMax,
VuMask mask
)
    {
    VuP nextP;
    if( axisSelect == 0 )
        {
        VU_SET_LOOP(currP,graphP)
            if ( IN_RANGE(VU_U(currP), cMin, cMax) )
                {
                nextP = VU_FSUCC(currP);
                if ( IN_RANGE( VU_U(nextP), cMin, cMax) )
                    {
                    VU_SETMASK(currP, mask);
                    }
                }
        END_VU_SET_LOOP(currP,graphP)
        }
    else
        {
        VU_SET_LOOP(currP,graphP)
            if ( IN_RANGE(VU_V(currP), cMin, cMax) )
                {
                nextP = VU_FSUCC(currP);
                if ( IN_RANGE( VU_V(nextP), cMin, cMax) )
                    {
                    VU_SETMASK(currP, mask);
                    }
                }
        END_VU_SET_LOOP(currP,graphP)
        }
    }

/*---------------------------------------------------------------------------------**//**
@nodoc
@deprecated vu_markEdgesInBand
@description Search a vu graph and mark those edges whose specified coordinates are in the specified closed interval.
@remarks This function's name suggests testing for near horizontal or vertical edges, in which case the
    implementation is probably a bug.
@param graphP IN OUT graph header
@param axisSelect IN component to inspect (0 for u-coordinate, 1 for v-coordinate)
@param cMin IN min coordinate of band
@param cMax IN max coordinate of band
@param mask IN mask to apply to nodes of edges within the band
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_setIsoCoordinateMask
(
VuSetP graphP,
int axisSelect,
double cMin,
double cMax,
VuMask mask
)
    {
    vu_markEdgesInBand (graphP, axisSelect, cMin, cMax, mask);
    }

/*---------------------------------------------------------------------------------**//**
@description For each value in the first array, apply (logical OR) a mask if the same value appears in the second array.
@remarks Don't try to interpret the names x and y geometrically: this is just a search for identical values in presorted arrays of doubles.
@param xP IN array of ascending double values
@param flagP IN OUT array of masks corresponding to values in xP; if mask is applied, a value was found in yP identical to this value in xP.
@param nx IN number of x values
@param yP IN array of ascending double values
@param ny IN number of y values
@param mask IN mask to apply
@param tol IN tolerance for identical values
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void vu_augmentMasks
(
double *xP,
VuMask *flagP,
int nx,
double *yP,
int ny,
VuMask mask,
double tol
)
    {
    int i,j;
    double d,ptol,mtol;
    ptol = fabs(tol);
    mtol = -ptol;
    /*
        Loop invariant: Marks have been set for
        all matches betwen x[0..i-1] and y[0..j-1]
    */
    for( i=0 , j=0 ; i < nx && j < ny ; )
        {
        d = xP[i] - yP[j];
        if ( d < mtol )
            {
            i++;
            }
        else if ( d > ptol )
            {
            j++;
            }
        else
            {
            flagP[i] |= mask;
            i++;        /* leave j where it is so that an x occuring
                        several times gets all its instances hit.
                        (Repeated j doesn't have to be reapplied because
                        the OR operator doesn't change after the first) */
            }
        }
    }

/*------------------------------------------------------------------*//**
@description Compute the 2D cross product of vectors from node0 to node1
  and node1 to node2, reducing each to half-period equivalent as needed.
@param pGraph IN graph header (provides periods)
@param pNode0 IN first node
@param pNode1 IN second node
@param pNode2 IN third node
@return the (scalar) cross product
@group "VU Coordinates"
@bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double vu_periodicCross
(
VuSetP pGraph,
VuP pNode0,
VuP pNode1,
VuP pNode2
)
    {
    DPoint3d periods;
    DPoint3d point0, point1, point2, U, V;
    vu_getPeriods (pGraph, &periods);
    if (periods.x == 0.0 && periods.y == 0.0)
        return vu_cross (pNode0, pNode1, pNode2);

    vu_getDPoint3d (&point0, pNode0);
    vu_getDPoint3d (&point1, pNode1);
    vu_getDPoint3d (&point2, pNode2);
    U.x = bsiTrig_normalizeToPeriodAroundZero (point1.x - point0.x, periods.x);
    U.y = bsiTrig_normalizeToPeriodAroundZero (point1.y - point0.y, periods.y);

    V.x = bsiTrig_normalizeToPeriodAroundZero (point2.x - point1.x, periods.x);
    V.y = bsiTrig_normalizeToPeriodAroundZero (point2.y - point1.y, periods.y);

    return U.x * V.y - U.y * V.x;
    }
/*------------------------------------------------------------------*//**
* @description Compute 2D coordinate range for the entire graph.
* @param graphP IN  graph header
* @param pMin   OUT minimum coordinates
* @param pMax   OUT maximum coordinates
* @return semiperimeter of 2D range box
* @group "VU Coordinates"
* @bsimethod
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   vu_graphCoordinateRange
(
VuSetP          graphP,
DPoint2d        *pMin,
DPoint2d        *pMax
)
    {
    DPoint2d minXY, maxXY, currXY;
    int count = 0;
    minXY.Zero ();  // will be overwritten with count==0
    maxXY.Zero ();  // will be overwritten with count==0
    VU_SET_LOOP (currP, graphP)
        {
        vu_getDPoint2d (&currXY, currP);
        if (count == 0)
            {
            minXY = maxXY = currXY;
            }

        if (currXY.x < minXY.x)
            minXY.x = currXY.x;
        if (currXY.y < minXY.y)
            minXY.y = currXY.y;

        if (currXY.x > maxXY.x)
            maxXY.x = currXY.x;
        if (currXY.y > maxXY.y)
            maxXY.y = currXY.y;
        count++;
        }
    END_VU_SET_LOOP (currP, graphP);

    if (pMin)
        *pMin = minXY;
    if (pMax)
        *pMax = maxXY;

    return (maxXY.x - minXY.x) + (maxXY.y - minXY.y);
    }

Public GEOMDLLIMPEXP DRange3d vu_faceRange (VuP faceSeed)
    {
    DRange3d range;
    range.Init ();
    VU_FACE_LOOP (currNode, faceSeed)
        {
        DPoint3d xyz;
        vu_getDPoint3d (&xyz, currNode);
        range.Extend(xyz);
        }
    END_VU_FACE_LOOP (currNode, faceSeed)
    return range;
    }
    
/*------------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
void VuNode::SetXYZ (DPoint3dCR xyz){ uv = xyz;}
void VuNode::SetXYZ (double x, double y, double z){uv = DPoint3d::From (x,y,z);}
DPoint3d VuNode::GetXYZ () const {return uv;}

void VuNode::SetMask (VuMask targetMask){vu_setMask (this, targetMask);}
void VuNode::ClearMask(VuMask targetMask) { vu_clrMask(this, targetMask); }
void VuNode::ToggleMask(VuMask targetMask)
    {
    if (HasMask(targetMask))
        vu_clrMask (this, targetMask);
    else
        vu_setMask(this, targetMask);
    }

bool VuNode::HasMask(VuMask targetMask) const { return 0 != vu_getMask(const_cast<VuP>(this), targetMask);}

void VuNode::SetMaskAroundEdge(VuMask targetMask)
    {
    vu_setMask (this, targetMask);
    vu_setMask (this->EdgeMate (), targetMask);
    }

void VuNode::ClearMaskAroundEdge(VuMask targetMask)
    {
    vu_clrMask(this, targetMask);
    vu_clrMask(this->EdgeMate(), targetMask);
    }

void VuNode::SetMaskAroundVertex (VuMask targetMask)
    {
    VU_VERTEX_LOOP (node, this)
        {
        vu_setMask(node, targetMask);
        }
    END_VU_VERTEX_LOOP (node, this)
    }

void VuNode::ClearMaskAroundVertex(VuMask targetMask)
    {
    VU_VERTEX_LOOP(node, this)
        {
        vu_clrMask(node, targetMask);
        }
    END_VU_VERTEX_LOOP(node, this)
    }

void VuNode::SetMaskAroundFace(VuMask targetMask)
    {
    VU_FACE_LOOP(node, this)
        {
        vu_setMask(node, targetMask);
        }
    END_VU_FACE_LOOP(node, this)
    }

void VuNode::ClearMaskAroundFace(VuMask targetMask)
    {
    VU_FACE_LOOP(node, this)
        {
        vu_clrMask(node, targetMask);
        }
    END_VU_FACE_LOOP(node, this)
    }

//==   FindMaskAround_____
VuP VuNode::FindMaskAroundVertex(VuMask targetMask)
    {
    VU_VERTEX_LOOP(node, this)
        {
        if (node->HasMask (targetMask))
            return node;
        }
    END_VU_VERTEX_LOOP(node, this)
    return nullptr;
    }

VuP VuNode::FindMaskAroundFace(VuMask targetMask)
    {
    VU_FACE_LOOP(node, this)
        {
        if (node->HasMask(targetMask))
            return node;
        }
    END_VU_FACE_LOOP(node, this)
    return nullptr;
    }

VuP VuNode::FindMaskAroundReverseVertex(VuMask targetMask)
    {
    VU_REVERSE_VERTEX_LOOP(node, this)
        {
        if (node->HasMask (targetMask))
            return node;
        }
    END_VU_REVERSE_VERTEX_LOOP(node, this)
    return nullptr;
    }

VuP VuNode::FindMaskAroundReverseFace(VuMask targetMask)
    {
    VU_REVERSE_FACE_LOOP(node, this)
        {
        if (node->HasMask(targetMask))
            return node;
        }
    END_VU_REVERSE_FACE_LOOP(node, this)
    return nullptr;
    }





// FindNodeAround =========================
VuP VuNode::FindNodeAroundVertex (VuP target)
    {
    VU_VERTEX_LOOP (node, this)
        {
        if (node == target)
            return node;
        }
    END_VU_VERTEX_LOOP (node, this)
    return nullptr;
    }

VuP VuNode::FindNodeAroundFace (VuP target)
    {
    VU_FACE_LOOP (node, this)
        {
        if (node == target)
            return node;
        }
    END_VU_FACE_LOOP (node, this)
    return nullptr;
    }

// SetUserDataAsIntAround =========================
void VuNode::SetUserDataAsIntAroundVertex (int value)
    {
    VU_VERTEX_LOOP (node, this)
        {
        node->SetUserDataAsInt (value);
        }
    END_VU_VERTEX_LOOP (node, this)
    }

void VuNode::SetUserDataAsIntAroundFace (int value)
    {
    VU_FACE_LOOP (node, this)
        {
        node->SetUserDataAsInt (value);
        }
    END_VU_FACE_LOOP (node, this)
    }


// CountMaskAround =========================
int VuNode::CountMaskAroundVertex(VuMask targetMask) const
    {
    int n = 0;
    VU_VERTEX_LOOP(node, const_cast<VuP>(this))
        {
        if (node->HasMask (targetMask))
            n++;
        }
    END_VU_VERTEX_LOOP(node,const_cast<VuP>(this))
    return n;
    }

int VuNode::CountMaskAroundFace (VuMask targetMask) const
    {
    int n = 0;
    VU_FACE_LOOP(node, const_cast<VuP>(this))
        {
        if (node->HasMask(targetMask))
            n++;
        }
    END_VU_FACE_LOOP(node, const_cast<VuP>(this))
    return n;
    }


int VuNode::GetId () const { return id; }

ptrdiff_t VuNode::GetUserData1 () const {return vu_getUserData1 (const_cast<VuP>(this));}
ptrdiff_t VuNode::SetUserData1 (ptrdiff_t value)
    {
    ptrdiff_t oldValue = GetUserData1 ();
    vu_setUserData1 (this, value);
    return oldValue;
    }
int       VuNode::GetUserDataAsInt () const {return vu_getUserDataPAsInt (const_cast<VuP>(this));}
int       VuNode::SetUserDataAsInt(int value)
    {
    int oldValue = GetUserDataAsInt();
    vu_setUserDataPAsInt (this, value);
    return oldValue;
    }

VuP VuNode::FSucc () const {return vu_fsucc (const_cast<VuP>(this));}
VuP VuNode::VSucc () const {return vu_vsucc (const_cast<VuP>(this));}
VuP VuNode::FPred () const {return vu_fpred (const_cast<VuP>(this));}
VuP VuNode::VPred () const {return vu_vpred (const_cast<VuP>(this));}

VuP VuNode::EdgeMate () const {return vu_edgeMate (const_cast<VuP>(this));}

/*---------------------------------------------------------------------------------**//**
@description  Collect xyz coordinates around a face.
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_getFaceLoopCoordinates
(
VuP faceP,
bvector<DPoint3d> &xyz
)
    {
    xyz.clear ();
    VU_FACE_LOOP (node, faceP)
        {
        xyz.push_back (node->GetXYZ ());
        }
    END_VU_FACE_LOOP (node, faceP)
    }

    
END_BENTLEY_GEOMETRY_NAMESPACE
