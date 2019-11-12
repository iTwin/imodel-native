/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>



BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilder::LightweightPolyfaceBuilder(PolyfaceHeaderR polyface, double pointTolerance, double normalTolerance, double paramTolerance) :
    m_polyface(&polyface), m_pointTolerance(pointTolerance), m_normalTolerance(normalTolerance), m_paramTolerance(paramTolerance)
        {
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilderPtr LightweightPolyfaceBuilder::Create(PolyfaceHeaderR polyface, double pointTolerance, double normalTolerance, double paramTolerance)
    {
        return new LightweightPolyfaceBuilder(polyface, pointTolerance, normalTolerance, paramTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilder::QPoint3d::QPoint3d(DPoint3dCR point, double tol)
    {
    m_x = (int64_t) (point.x / tol);
    m_y = (int64_t) (point.y / tol);
    m_z = (int64_t) (point.z / tol);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilder::QPoint3d::QPoint3d(DVec3dCR point, double tol)
    {
    m_x = (int64_t) (point.x / tol);
    m_y = (int64_t) (point.y / tol);
    m_z = (int64_t) (point.z / tol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LightweightPolyfaceBuilder::QPoint2d::QPoint2d(DPoint2dCR point, double tol)
    {
    m_x = (int64_t) (point.x / tol);
    m_y = (int64_t) (point.y / tol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
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
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LightweightPolyfaceBuilder::QPoint2d::operator < (struct QPoint2d const& rhs) const
    {
    if (m_x < rhs.m_x)
        return true;
    else if (m_x > rhs.m_x)
        return false;

    return m_y < rhs.m_y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  LightweightPolyfaceBuilder::FindOrAddPoint(DPoint3dCR point)
    {
    auto    insert = m_pointMap.Insert(QPoint3d(point, m_pointTolerance), m_polyface->GetPointCount());

    if (insert.second)
        m_polyface->Point().push_back(point);

    return insert.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
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
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  LightweightPolyfaceBuilder::FindOrAddParam(DPoint2dCR param)
    {
    auto    insert = m_paramMap.Insert(QPoint2d(param, m_paramTolerance), m_polyface->GetParamCount());

    if (insert.second)
        m_polyface->Param().push_back(param);

    return insert.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LightweightPolyfaceBuilder::AddPointIndex(size_t zeroBasedIndex, bool visible) 
    { 
    int32_t index = (int32_t) zeroBasedIndex + 1; 
    m_polyface->PointIndex().push_back(visible ? index : - index); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LightweightPolyfaceBuilder::EndFace ()
    {
    m_polyface->SetNewFaceData (&m_currentFaceData);
    m_currentFaceData.Init ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
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
* @bsimethod                                                    Ray.Bentley     04/2018
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
