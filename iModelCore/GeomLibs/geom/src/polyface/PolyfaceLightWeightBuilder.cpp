/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>



BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilder::LightweightPolyfaceBuilder(PolyfaceHeaderR polyface, double pointTolerance, double normalTolerance, double paramTolerance) :
    m_polyface(&polyface), m_pointTolerance(pointTolerance), m_normalTolerance(normalTolerance), m_paramTolerance(paramTolerance)
        {
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilderPtr LightweightPolyfaceBuilder::Create(PolyfaceHeaderR polyface, double pointTolerance, double normalTolerance, double paramTolerance)
    {
        return new LightweightPolyfaceBuilder(polyface, pointTolerance, normalTolerance, paramTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilder::QPoint3d::QPoint3d(DPoint3dCR point, double tol)
    {
    m_x = (int64_t) floor (point.x / tol);
    m_y = (int64_t) floor (point.y / tol);
    m_z = (int64_t) floor (point.z / tol);
    }
LightweightPolyfaceBuilder::QPoint3d::QPoint3d(int64_t x, int64_t y, int64_t z)
    {
    m_x = x;
    m_y = y;
    m_z = z;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilder::QPoint3d::QPoint3d(DVec3dCR point, double tol)
    {
    m_x = (int64_t) (point.x / tol);
    m_y = (int64_t) (point.y / tol);
    m_z = (int64_t) (point.z / tol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilder::QPoint2d::QPoint2d(DPoint2dCR point, double tol)
    {
    m_x = (int64_t) (point.x / tol);
    m_y = (int64_t) (point.y / tol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LightweightPolyfaceBuilder::QPoint3d::operator < (struct QPoint3d const& rhs) const
    {                                                                                                                                              
    if (m_x < rhs.m_x)
        return true;
    else if (m_x > rhs.m_x)
        return false;

    if (m_y < rhs.m_y)
        return true;
    else if (m_y > rhs.m_y)
        return false;

    return m_z < rhs.m_z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LightweightPolyfaceBuilder::QPoint2d::operator < (struct QPoint2d const& rhs) const
    {
    if (m_x < rhs.m_x)
        return true;
    else if (m_x > rhs.m_x)
        return false;

    return m_y < rhs.m_y;
    }
static void CreateNeighborHoodRange(int64_t &q, int64_t &q0, int64_t &q1, double x, double tol)
    {
    double y = x / tol;
    q = (int64_t)floor(y + 0.5);
    q0 = q - 1;
    q1 = q;
    }
// Return a range of (integer) values that includes the octant containing point,
// plus nearest neighbors above or below in each direction.
static void CreateNeighborhoodRange(
LightweightPolyfaceBuilder::QPoint3d &q,    // closest grid point (rounded!)
LightweightPolyfaceBuilder::QPoint3d &q0,   // low limits for 8-box search
LightweightPolyfaceBuilder::QPoint3d &q1,   // high limits for 8-box search.
DPoint3dCR point, double tol)
    {
    CreateNeighborHoodRange (q.m_x, q0.m_x, q1.m_x, point.x, tol);
    CreateNeighborHoodRange(q.m_y, q0.m_y, q1.m_y, point.y, tol);
    CreateNeighborHoodRange(q.m_z, q0.m_z, q1.m_z, point.z, tol);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  LightweightPolyfaceBuilder::FindOrAddPointCluster(DPoint3dCR point)
    {
    QPoint3d qPoint0, qPoint1, qPoint;
    CreateNeighborhoodRange(qPoint, qPoint0, qPoint1, point, m_pointTolerance);
    // search primary voxel first . . .
    auto it = m_pointMap.find(qPoint);
    if (it != m_pointMap.end())
        return it->second;
    for (int64_t ix = qPoint0.m_x; ix <= qPoint1.m_x; ix++)
        {
        for (int64_t iy = qPoint0.m_y; iy <= qPoint1.m_y; iy++)
            {
            for (int64_t iz = qPoint0.m_z; iz <= qPoint1.m_z; iz++)
                {
                // don't repeat search at primary voxel . ..
                if (ix == qPoint.m_x && iy == qPoint.m_y && iz == qPoint.m_z)
                    continue;
                QPoint3d q1 (ix, iy, iz);
                auto iit = m_pointMap.find (q1);
                if (iit != m_pointMap.end ())
                    {
                    return iit->second;
                    }
                }
            }
        }
    auto    insert = m_pointMap.Insert(qPoint, m_polyface->GetPointCount());

    if (insert.second)
        m_polyface->Point().push_back(point);
    return insert.first->second;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  LightweightPolyfaceBuilder::FindOrAddPoint(DPoint3dCR point)
    {
    auto    insert = m_pointMap.Insert(QPoint3d(point, m_pointTolerance), m_polyface->GetPointCount());

    if (insert.second)
        m_polyface->Point().push_back(point);
    return insert.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  LightweightPolyfaceBuilder::FindOrAddNormal (DVec3dCR normal)
    {
    BeAssert(DoubleOps::WithinTolerance(normal.MagnitudeSquared(), 1.0, 0.001));

    auto    insert = m_normalMap.Insert(QPoint3d(normal, m_normalTolerance), m_polyface->GetNormalCount());
    if (insert.second)
        m_polyface->Normal().push_back(normal);

    return insert.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  LightweightPolyfaceBuilder::FindOrAddParam(DPoint2dCR param)
    {
    auto    insert = m_paramMap.Insert(QPoint2d(param, m_paramTolerance), m_polyface->GetParamCount());

    if (insert.second)
        m_polyface->Param().push_back(param);

    return insert.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LightweightPolyfaceBuilder::AddPointIndex(size_t zeroBasedIndex, bool visible) 
    { 
    int32_t index = (int32_t) zeroBasedIndex + 1; 
    m_polyface->PointIndex().push_back(visible ? index : - index); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LightweightPolyfaceBuilder::EndFace ()
    {
    m_polyface->SetNewFaceData (&m_currentFaceData);
    m_currentFaceData.Init ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LightweightPolyfaceBuilder::AddAuxDataByIndex (PolyfaceAuxData::ChannelsCR channels, size_t index)
    {
    PolyfaceAuxDataPtr        auxData;
    
    if (m_polyface->GetAuxDataCP().IsValid())
        {
        auxData = const_cast <PolyfaceAuxDataP> (m_polyface->GetAuxDataCP().get());
        }
    else
        {
        auxData = new PolyfaceAuxData();
        m_polyface->SetAuxData(auxData);
        }
    auxData->AppendDataByIndex(channels, index);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void  LightweightPolyfaceBuilder::AddIndexTerminators()
    {
    AddPointIndexTerminator();
    if (!m_polyface->Normal().empty())
        AddNormalIndexTerminator();

    if (!m_polyface->Param().empty())
        AddParamIndexTerminator();

    if (m_polyface->GetAuxDataCP().IsValid())
        (const_cast <PolyfaceAuxDataP> (m_polyface->GetAuxDataCP().get()))->AddIndexTerminator();
    }
            

void LightweightPolyfaceBuilder::AddNormalIndex(size_t zeroBasedIndex)  { m_polyface->NormalIndex().push_back((int32_t) zeroBasedIndex + 1); }
void LightweightPolyfaceBuilder::AddParamIndex (size_t zeroBasedIndex)  { m_polyface->ParamIndex().push_back((int32_t) zeroBasedIndex + 1); }
void LightweightPolyfaceBuilder::AddPointIndexTerminator()              { m_polyface->PointIndex().push_back(0);  }
void LightweightPolyfaceBuilder::AddNormalIndexTerminator()             { m_polyface->NormalIndex().push_back(0); }
void LightweightPolyfaceBuilder::AddParamIndexTerminator()              { m_polyface->ParamIndex().push_back(0);  }
void LightweightPolyfaceBuilder::SetFaceData (FacetFaceDataCR data)     { m_currentFaceData = data; }



END_BENTLEY_GEOMETRY_NAMESPACE
