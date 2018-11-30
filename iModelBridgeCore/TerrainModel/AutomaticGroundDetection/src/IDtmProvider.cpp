/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/IDtmProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include "IDtmProvider.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL


BEGIN_GROUND_DETECTION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Triangle::IsPointOnPlaneInside(DPoint3d pointOnPlane, bool strictlyInside) const
    {
    //We assume input point is already projected on plane (for optimization purpose)
    //     DPlane3d planeFromTriangle;
    //     planeFromTriangle.initFrom3Points(&m_point[0], &m_point[1], &m_point[2]);
    // 
    //     DPoint3d pointOnPlane;
    //     planeFromTriangle.ProjectPoint(pointOnPlane, point);

    //Compute from Barycentric Technique (http://www.blackpawn.com/texts/pointinpoly/)
    // Compute vectors
    //v0 = C - A
    DVec3d v0(DVec3d::FromStartEnd(m_point[0], m_point[2]));
    //v1 = B - A
    DVec3d v1(DVec3d::FromStartEnd(m_point[0], m_point[1]));
    //v2 = * - A
    DVec3d v2(DVec3d::FromStartEnd(m_point[0], pointOnPlane));

    // Compute dot products
    //dot00 = dot(v0, v0)
    double dot00(v0.DotProduct(v0));
    // dot01 = dot(v0, v1)
    double dot01(v0.DotProduct(v1));
    // dot02 = dot(v0, v2)
    double dot02(v0.DotProduct(v2));
    // dot11 = dot(v1, v1)
    double dot11(v1.DotProduct(v1));
    // dot12 = dot(v1, v2)
    double dot12(v1.DotProduct(v2));

    // Compute barycentric coordinates
    // invDenom = 1 / (dot00 * dot11 - dot01 * dot01)
    // u = (dot11 * dot02 - dot01 * dot12) * invDenom
    // v = (dot00 * dot12 - dot01 * dot02) * invDenom
    double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
    double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle 
    // If we want strictly inside point and don't permit point on triangle lines; u==0 and v==0 is not permited!
    if (strictlyInside)
        return (u > 0) && (v > 0) && (u + v < 1);
    //Otherwise, we accept u or v ==0
    return (u >= 0) && (v >= 0) && (u + v < 1);
    }



END_GROUND_DETECTION_NAMESPACE
