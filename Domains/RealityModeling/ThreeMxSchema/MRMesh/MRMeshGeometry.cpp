/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshGeometry.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ThreeMxSchemaInternal.h"

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr Geometry::GetPolyface() const
    {
    Graphic::TriMeshArgs trimesh;
    trimesh.m_numIndices = (int32_t) m_indices.size();
    trimesh.m_vertIndex = &m_indices.front();
    trimesh.m_numPoints = (int32_t) m_points.size();
    trimesh.m_points = &m_points.front();
    trimesh.m_normals = &m_normals.front();
    trimesh.m_textureUV = &m_textureUV.front();
    return trimesh.ToPolyface();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(Graphic::TriMeshArgs const& args, LoadContextCR loadContext)
    {
    m_indices.resize(args.m_numIndices);
    memcpy(&m_indices.front(), args.m_vertIndex, args.m_numIndices * sizeof(int32_t));

    m_points.resize(args.m_numPoints);
    memcpy(&m_points.front(), args.m_points, args.m_numPoints * sizeof(FPoint3d));
    
    if (nullptr != args.m_normals)
        {
        m_normals.resize(args.m_numIndices);
        memcpy(&m_normals.front(), args.m_normals, args.m_numIndices * sizeof(FPoint3d));
        }

    if (nullptr != args.m_textureUV)
        {
        m_textureUV.resize(args.m_numIndices);
        memcpy(&m_textureUV.front(), args.m_textureUV, args.m_numIndices * sizeof(FPoint2d));
        }

    if (nullptr == loadContext.m_system)
        return;

    m_graphic = loadContext.m_system->_CreateGraphic(Graphic::CreateParams(nullptr, loadContext.m_placement));
    m_graphic->AddTriMesh(args);
    m_graphic->Close();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Geometry::GetMemorySize() const
    {
    size_t size = m_points.size() * sizeof(FPoint3d) + 
                  m_normals.size() * sizeof(FPoint3d) + 
                  m_textureUV.size() * sizeof(FPoint2d);

    if (m_graphic.IsValid())
        {
        size += (m_points.size()  * 3 +
                 m_normals.size() * 3 +
                 m_textureUV.size() * 2) * sizeof(float);     // Account for QVision data (floats).
        }

    return size;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Geometry::Draw(RenderContextR context)
    {
    if (m_graphic.IsValid())
        context.OutputGraphic(*m_graphic, nullptr);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Geometry::GetRange() const    
    {
    DRange3d range = DRange3d::NullRange();
    if (!m_graphic.IsValid())
        return range;

    PolyfaceHeaderPtr polyface = GetPolyface();
    for (DPoint3dCP point = polyface->GetPointCP(), end = point + polyface->GetPointCount(); point < end; )
        {
        DPoint3d    transformedPoint;
        m_graphic->GetLocalToWorldTransform().Multiply(transformedPoint, *point++);
        range.Extend(transformedPoint);
        }

    return range;
    }
