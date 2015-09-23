/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshGeometry.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void floatToDouble (double* pDouble, float const* pFloat, int n)
    {
    for (double* pEnd = pDouble + n; pDouble < pEnd; )
        *pDouble++ = *pFloat++;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshGeometry::MRMeshGeometry (int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId) : m_textureId (textureId), m_qvElem (NULL)
    {
    m_polyface = PolyfaceHeader::CreateFixedBlockIndexed (3);

    BlockedVectorIntR       pointIndex = m_polyface->PointIndex();

    pointIndex.resize (3* nbTriangles);
    for (int32_t *pIndex = indices, *pEnd = pIndex + 3 * nbTriangles, *pOut = &pointIndex.front(); pIndex < pEnd; )
        *pOut++ = 1 + *pIndex++;

    if (NULL != positions)
        {
        m_polyface->Point().resize (nbVertices);
        floatToDouble (&m_polyface->Point().front().x, positions, 3 * nbVertices);
        }
    if (NULL != normals)
        {
        m_polyface->Normal().resize (nbVertices);
        floatToDouble (&m_polyface->Normal().front().x, normals, 3 * nbVertices);
        }

    if (NULL != textureCoordinates)
        {
        m_polyface->Param().resize (nbVertices);
        floatToDouble (&m_polyface->Param().front().x, textureCoordinates, 2 * nbVertices);
        }
    if (NULL != normals)
        m_polyface->NormalIndex() = pointIndex;

    if (NULL != textureCoordinates)
        m_polyface->ParamIndex() = pointIndex;

    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshGeometry::GetMemorySize() const
    {
    // Approximate QVision float data by 1.5 * the double precision data stored here.
    // Note - we can't release the polyFace data as it is needed for locating/snapping.
    size_t size = m_polyface->GetPointCount ()           * sizeof (DPoint3d) +
                  m_polyface->GetNormalCount ()          * sizeof (DVec3d) +
                  m_polyface->GetParamCount ()           * sizeof (DPoint2d) +
                  m_polyface->GetFaceCount()             * sizeof (FacetFaceData) +
                  m_polyface->PointIndex ().size ()      * sizeof (int32_t) +
                  m_polyface->NormalIndex ().size ()     * sizeof (int32_t) +                           m_polyface->ParamIndex ().size ()      * sizeof (int32_t) +
                  m_polyface->GetFaceIndexCount ()       * sizeof (int32_t);

    if (NULL != m_qvElem)
        {
        size += (m_polyface->GetPointCount()  * 3 +
                 m_polyface->GetNormalCount() * 3 +
                 m_polyface->GetParamCount()  * 2) * 12;     // Account for QVision data (floats).
        }
    return size;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshGeometry::Draw (ViewContextR viewContext, MRMeshNodeR node, MRMeshContextCR host)
    {
    MRMeshTextureP  texture;

    ICachedDrawP    cachedDraw = viewContext.GetICachedDraw();
    if (NULL != (texture = node.GetTexture (m_textureId)))
        {
        texture->Initialize (node, host, viewContext);
        texture->Activate (viewContext);
        }

    if (NULL == cachedDraw || 
        !viewContext.GetIViewDraw ().IsOutputQuickVision ())
        {
        viewContext.GetIDrawGeom().DrawPolyface (*m_polyface);
        return;
        }

    if (NULL == m_qvElem)
        {
        cachedDraw->BeginCacheElement (host.GetQvCache());
        cachedDraw->DrawPolyface (*m_polyface);
        m_qvElem = cachedDraw->EndCacheElement ();
        }

    viewContext.GetIViewDraw().DrawQvElem (m_qvElem);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshGeometry::DrawCut (ViewContextR viewContext, DPlane3dCR plane)
    {
    CurveVectorPtr  slice;
    
    if (m_polyface.IsValid() &&
        (slice = m_polyface->PlaneSlice (plane, false, false)).IsValid())
        viewContext.GetIDrawGeom().DrawCurveVector (*slice, false);
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshGeometry::ReleaseQVisionCache ()
    {
    // shutting down
    if (nullptr == DgnPlatformLib::QueryHost())
        return;

    if (NULL != m_qvElem)
        T_HOST.GetGraphicsAdmin()._DeleteQvElem (m_qvElem);
    m_qvElem = NULL;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshGeometryPtr MRMeshGeometry::Create (int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId)
    {
    return new MRMeshGeometry (nbVertices, positions, normals, nbTriangles, indices, textureCoordinates, textureId);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshGeometry::~MRMeshGeometry ()  { ReleaseQVisionCache (); }



