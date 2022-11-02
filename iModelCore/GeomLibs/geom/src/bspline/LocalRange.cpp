/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include "msbsplinemaster.h"


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

LocalRange::LocalRange (TransformCR localToWorld, TransformCR worldToLocal, DRange3dCR range)
    : m_localToWorld (localToWorld), m_worldToLocal (worldToLocal), m_localRange (range)
    {
    m_localToWorld.Multiply (m_worldRange, m_localRange);
    }

void LocalRange::InitNullRange ()
    {
    m_localToWorld.InitIdentity ();
    m_worldToLocal.InitIdentity ();
    m_localRange.Init ();
    m_worldRange.Init ();
    }

LocalRange::LocalRange ()
    {
    InitNullRange ();
    }
    
bool LocalRange::InitFromPrincipalAxesOfPoints (bvector<DPoint3d> const &xyz)
    {
    InitNullRange ();
    DVec3d centroid, moments; 
    RotMatrix axes;
    if (!DPoint3dOps::PrincipalAxes (xyz, centroid, axes, moments))
        return false;
    m_localToWorld.InitFrom (axes, centroid);
    m_worldToLocal.InvertRigidBodyTransformation (m_localToWorld);
    m_localRange = DRange3d::From (m_worldToLocal, xyz);
    m_worldRange = DRange3d::From (xyz);
    return true;
    }

bool LocalRange::InitFromPrincipalAxesOfPoints (bvector<DPoint4d> const &xyzw)
    {
    InitNullRange ();
    DVec3d centroid, moments; 
    RotMatrix axes;
    if (!DPoint3dOps::PrincipalAxes (xyzw, centroid, axes, moments))
        return false;
    m_localToWorld.InitFrom (axes, centroid);
    m_worldToLocal.InvertRigidBodyTransformation (m_localToWorld);
    m_localRange = DRange3d::From (m_worldToLocal, xyzw);
    m_worldRange = DRange3d::From (xyzw);
    return true;
    }

double LocalRange::DistanceOutside (DPoint3dCR spacePoint) const
    {
    DPoint3d uvw;
    m_worldToLocal.Multiply (uvw, spacePoint);
    return m_localRange.DistanceOutside (uvw);
    }


DPoint3d LocalRange::RangeFractionToLocal (double x, double y, double z) const
    {
    return m_localRange.LocalToGlobal (x, y, z);
    }
    
DPoint3d LocalRange::RangeFractionToWorld (double x, double y, double z) const
    {
    return m_localToWorld * m_localRange.LocalToGlobal (x, y, z);
    }


// LocalRange with size_t and double tags (e.g. for bsurface patch)
TaggedLocalRange::TaggedLocalRange (size_t indexA, size_t indexB, double a)
    : LocalRange (), m_indexA (indexA), m_indexB (indexB), m_a (a)
    {
    }
    




static bool cb_compareTaggedLocalRangeByA (TaggedLocalRange const & dataA, TaggedLocalRange const & dataB)
    {
    return dataA.m_a < dataB.m_a;
    }

void TaggedLocalRange::SortByA (bvector<TaggedLocalRange> &data)
    {
    std::sort (data.begin (), data.end (), cb_compareTaggedLocalRangeByA);
    }
// Compute and save distance from the range to space point.
void TaggedLocalRange::SetDistanceOutside (DPoint3dCR spacePoint)
    {
    m_a = DistanceOutside (spacePoint);
    }

    
    
END_BENTLEY_GEOMETRY_NAMESPACE    