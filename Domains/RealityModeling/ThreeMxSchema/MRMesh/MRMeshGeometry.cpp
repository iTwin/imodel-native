/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshGeometry.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void floatToDouble(double* pDouble, float const* pFloat, size_t n)
    {
    for (double* pEnd = pDouble + n; pDouble < pEnd; )
        *pDouble++ = *pFloat++;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr MRMeshGeometry::GetPolyface() const
    {
    PolyfaceHeaderPtr polyFace = PolyfaceHeader::CreateFixedBlockIndexed(3);

    BlockedVectorIntR pointIndex = polyFace->PointIndex();
    pointIndex.resize(m_indices.size());
    int32_t const* pIndex = &m_indices.front();
    int32_t const* pEnd = pIndex + m_indices.size();
    int32_t* pOut = &pointIndex.front();
    for (; pIndex < pEnd; )
        *pOut++ = 1 + *pIndex++;

    if (!m_points.empty())
        {
        polyFace->Point().resize(m_points.size());
        floatToDouble(&polyFace->Point().front().x, &m_points.front().x, 3 * m_points.size());
        }
    if (!m_normals.empty())
        {
        polyFace->Normal().resize(m_normals.size());
        floatToDouble(&polyFace->Normal().front().x, &m_normals.front().x, 3 * m_normals.size());
        polyFace->NormalIndex() = pointIndex;
        }

    if (!m_params.empty())
        {
        polyFace->Param().resize(m_params.size());
        floatToDouble(&polyFace->Param().front().x, &m_params.front().x, 2 * m_params.size());
        polyFace->ParamIndex() = pointIndex;
        }

    return polyFace;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshGeometry::MRMeshGeometry(int32_t nbVertices, FPoint3d const* positions, FPoint3dCP normals, int32_t nbTriangles, uint32_t const* indices, FPoint2dCP params, TextureCP texture, SystemP target)
    {
    size_t nIndices = 3 * nbTriangles;

    m_indices.resize(nIndices);
    memcpy(&m_indices.front(), indices, nIndices * sizeof(int32_t));

    m_points.resize(nbVertices);
    memcpy(&m_points.front(), positions, nbVertices * sizeof (FPoint3d));
    
    if (NULL != normals)
        {
        m_normals.resize(nbVertices);
        memcpy(&m_normals.front(), normals, nbVertices * sizeof (FPoint3d));
        }

    if (NULL != params)
        {
        m_params.resize(nbVertices);
        memcpy(&m_params.front(), params, nbVertices * sizeof (FPoint2d));
        }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_qvElem = qv_beginElement(qvCache, 0, NULL);
    qv_addQuickTriMesh(m_qvElem,  3 * nbTriangles, indices, nbVertices, NULL, reinterpret_cast <FloatXYZ*> (positions), 
                                                                               reinterpret_cast <FloatXYZ*> (normals),
                                                                               reinterpret_cast <FloatXY*> (params), qvTextureId, QV_QTMESH_GENNORMALS);
    qv_endElement(m_qvElem);
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MRMeshGeometry::GetMemorySize() const
    {
    // Approximate QVision float data by 1.5 * the double precision data stored here.
    // Note - we can't release the polyFace data as it is needed for locating/snapping.
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    size_t size = m_polyface->GetPointCount()      * sizeof(DPoint3d) +
                  m_polyface->GetNormalCount()     * sizeof(DVec3d) +
                  m_polyface->GetParamCount()      * sizeof(DPoint2d) +
                  m_polyface->GetFaceCount()       * sizeof(FacetFaceData) +
                  m_polyface->PointIndex().size()  * sizeof(int32_t) +
                  m_polyface->NormalIndex().size() * sizeof(int32_t) +
                  m_polyface->ParamIndex().size()  * sizeof(int32_t) +
                  m_polyface->GetFaceIndexCount()  * sizeof(int32_t);
#endif

    size_t size=0;
    if (m_graphic.IsValid())
        {
        size += (m_points.size()  * 3 +
                 m_normals.size() * 3 +
                 m_params.size() * 2) * sizeof(float);     // Account for QVision data (floats).
        }

    return size;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshGeometry::Draw(RenderContextR context, MRMeshNodeR node, MRMeshContextCR host)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    MRMeshTextureP  texture;

    ICachedDrawP    cachedDraw = viewContext.GetICachedDraw();
    if (nullptr != (texture = node.GetTexture(m_textureId)))
        {
        texture->Initialize(node, host, viewContext);
        texture->Activate(viewContext);
        }

    if (nullptr == cachedDraw || 
        !viewContext.GetIViewDraw().IsOutputQuickVision())
        {
        viewContext.GetIDrawGeom().DrawPolyface(*m_polyface);
        return;
        }

    if (nullptr == m_qvElem)
        {
        cachedDraw->BeginCacheElement(host.GetQvCache());
        cachedDraw->DrawPolyface(*m_polyface);
        m_qvElem = cachedDraw->EndCacheElement();
        }

    viewContext.GetIViewDraw().DrawQvElem(m_qvElem);
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshGeometry::GetRange(DRange3dR range, TransformCR transform) const
    {
    PolyfaceHeaderPtr polyface = GetPolyface();
    for (DPoint3dCP point = polyface->GetPointCP(), end = point + polyface->GetPointCount(); point < end; )
        {
        DPoint3d    transformedPoint;
        transform.Multiply(transformedPoint, *point++);
        range.Extend(transformedPoint);
        }

    return SUCCESS;
    }

