/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#if defined (BENTLEYCONFIG_PARASOLID)
#include <BRepCore/PSolidUtil.h>
#endif
#include <Bentley/ByteStream.h>
#include <GeomJsonWireFormat/JsonUtils.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment()
    {
    m_useColor = m_useMaterial = false;
    m_color = 0;
    m_transparency = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator == (struct FaceAttachment const& rhs) const
    {
    if (m_useColor      != rhs.m_useColor ||
        m_useMaterial   != rhs.m_useMaterial ||
        m_color         != rhs.m_color ||
        m_transparency  != rhs.m_transparency ||
        m_material      != rhs.m_material)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator < (struct FaceAttachment const& rhs) const
    {
    return (m_useColor         < rhs.m_useColor ||
            m_useMaterial      < rhs.m_useMaterial ||
            m_color            < rhs.m_color ||
            m_transparency     < rhs.m_transparency ||
            m_material         < rhs.m_material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr BRepUtil::FacetEntity(IBRepEntityCR entity, IFacetOptionsR facetOptions)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::FacetEntity(entity, facetOptions);
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<FaceAttachment>& params, IFacetOptionsR facetOptions)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::FacetEntity(entity, polyfaces, params, facetOptions);
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BRepUtil::GetBodyFaces(bvector<ISubEntityPtr>* subEntities, IBRepEntityCR in)
    {
    int         nFaces = 0;
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_FACE_t*  faceTags = NULL;

    PK_BODY_ask_faces(PSolidUtil::GetEntityTag(in), &nFaces, subEntities ? &faceTags : NULL);

    if (subEntities)
        {
        for (int iFace = 0; iFace < nFaces; ++iFace)
            subEntities->push_back(PSolidSubEntity::CreateSubEntity(faceTags[iFace], in));

        PK_MEMORY_free(faceTags);
        }
#endif

    return nFaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BRepUtil::GetBodyEdges(bvector<ISubEntityPtr>* subEntities, IBRepEntityCR in)
    {
    int         nEdges = 0;
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_EDGE_t*  edgeTags = NULL;

    PK_BODY_ask_edges(PSolidUtil::GetEntityTag(in), &nEdges, subEntities ? &edgeTags : NULL);

    if (subEntities)
        {
        for (int iEdge = 0; iEdge < nEdges; ++iEdge)
            subEntities->push_back(PSolidSubEntity::CreateSubEntity(edgeTags[iEdge], in));

        PK_MEMORY_free(edgeTags);
        }
#endif

    return nEdges;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BRepUtil::GetBodyVertices(bvector<ISubEntityPtr>* subEntities, IBRepEntityCR in)
    {
    int           nVertex = 0;
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_VERTEX_t*  vertexTags = NULL;

    PK_BODY_ask_vertices(PSolidUtil::GetEntityTag (in), &nVertex, subEntities ? &vertexTags : NULL);

    if (subEntities)
        {
        for (int iVertex = 0; iVertex < nVertex; ++iVertex)
            subEntities->push_back(PSolidSubEntity::CreateSubEntity(vertexTags[iVertex], in));

        PK_MEMORY_free(vertexTags);
        }
#endif

    return nVertex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetFaceEdges(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    int           nEntity = 0;
    PK_ENTITY_t*  entities = NULL;

    if (SUCCESS != PK_FACE_ask_edges(PSolidSubEntity::GetSubEntityTag(subEntity), &nEntity, &entities))
        return ERROR;

    for (int iEntity = 0; iEntity < nEntity; ++iEntity)
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(entities[iEntity], PSolidSubEntity::GetSubEntityTransform(subEntity)));

    PK_MEMORY_free(entities);

    return (subEntities.empty() ? ERROR : SUCCESS);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetFaceVertices(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    int           nEntity = 0;
    PK_ENTITY_t*  entities = NULL;

    if (SUCCESS != PK_FACE_ask_vertices(PSolidSubEntity::GetSubEntityTag(subEntity), &nEntity, &entities))
        return ERROR;

    for (int iEntity = 0; iEntity < nEntity; ++iEntity)
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(entities[iEntity], PSolidSubEntity::GetSubEntityTransform(subEntity)));

    PK_MEMORY_free(entities);

    return (subEntities.empty() ? ERROR : SUCCESS);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetEdgeFaces(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    int           nEntity = 0;
    PK_ENTITY_t*  entities = NULL;

    if (SUCCESS != PK_EDGE_ask_faces(PSolidSubEntity::GetSubEntityTag(subEntity), &nEntity, &entities))
        return ERROR;

    for (int iEntity = 0; iEntity < nEntity; ++iEntity)
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(entities[iEntity], PSolidSubEntity::GetSubEntityTransform(subEntity)));

    PK_MEMORY_free(entities);

    return (subEntities.empty() ? ERROR : SUCCESS);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetEdgeVertices(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t entities[2];

    if (SUCCESS != PK_EDGE_ask_vertices(PSolidSubEntity::GetSubEntityTag(subEntity), entities))
        return ERROR;

    if (PK_ENTITY_null != entities[0])
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(entities[0], PSolidSubEntity::GetSubEntityTransform(subEntity)));

    if (PK_ENTITY_null != entities[1])
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(entities[1], PSolidSubEntity::GetSubEntityTransform(subEntity)));

    return (subEntities.empty() ? ERROR : SUCCESS);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetVertexFaces(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    int           nEntity = 0;
    PK_ENTITY_t*  entities = NULL;

    if (SUCCESS != PK_VERTEX_ask_faces(PSolidSubEntity::GetSubEntityTag(subEntity), &nEntity, &entities))
        return ERROR;

    for (int iEntity = 0; iEntity < nEntity; ++iEntity)
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(entities[iEntity], PSolidSubEntity::GetSubEntityTransform(subEntity)));

    PK_MEMORY_free(entities);

    return (subEntities.empty() ? ERROR : SUCCESS);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetVertexEdges(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    int           nEntity = 0;
    PK_ENTITY_t*  entities = NULL;

    if (SUCCESS != PK_VERTEX_ask_oriented_edges(PSolidSubEntity::GetSubEntityTag(subEntity), &nEntity, &entities, NULL))
        return ERROR;

    for (int iEntity = 0; iEntity < nEntity; ++iEntity)
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(entities[iEntity], PSolidSubEntity::GetSubEntityTransform(subEntity)));

    PK_MEMORY_free(entities);

    return (subEntities.empty() ? ERROR : SUCCESS);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetTangentBlendEdges(bvector <ISubEntityPtr>& smoothEdges, ISubEntityCR edge)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_EDGE_t   edgeTag;

    if (0 == (edgeTag = PSolidSubEntity::GetSubEntityTag(edge)))
        return ERROR;

    bvector<PK_EDGE_t> edges;

    if (SUCCESS != PSolidTopo::GetTangentBlendEdges(edges, edgeTag))
        return ERROR;

    Transform   entityTransform = PSolidSubEntity::GetSubEntityTransform(edge);

    for (PK_EDGE_t smoothEdge : edges)
        smoothEdges.push_back(PSolidSubEntity::CreateSubEntity(smoothEdge, entityTransform));

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetLoopEdgesFromEdge(bvector<ISubEntityPtr>& loopEdges, ISubEntityCR edge, ISubEntityCR face)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_EDGE_t   edgeTag;
    PK_FACE_t   faceTag;

    if (0 == (edgeTag = PSolidSubEntity::GetSubEntityTag(edge)) || 0 == (faceTag = PSolidSubEntity::GetSubEntityTag(face)))
        return ERROR;

    bvector<PK_EDGE_t> edges;

    if (SUCCESS != PSolidTopo::GetLoopEdgesFromEdge(edges, edgeTag, faceTag))
        return ERROR;

    Transform   entityTransform = PSolidSubEntity::GetSubEntityTransform(face);

    for (PK_EDGE_t loopEdge : edges)
        loopEdges.push_back(PSolidSubEntity::CreateSubEntity(loopEdge, entityTransform));

    return SUCCESS;
#else
    return ERROR;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool facesAreRedundant(PK_FACE_t faceTag1, PK_FACE_t faceTag2)
    {
    PK_FACE_t facePair[2];

    facePair[0] = faceTag1;
    facePair[1] = faceTag2;

    int nTopols = 0;
    PK_TOPOL_identify_redundant_o_t options;

    PK_TOPOL_identify_redundant_o_m(options);
    options.want_redundant_topols = PK_LOGICAL_false;

    return (SUCCESS == PK_TOPOL_identify_redundant(2, facePair, &options, &nTopols, nullptr) && 0 != nTopols);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool facesAreSmooth(PK_FACE_t faceTag1, PK_FACE_t faceTag2, bool checkCommonVertex)
    {
    int         nEdges = 0;
    PK_EDGE_t*  edgesP = nullptr;

    if (SUCCESS != PK_FACE_find_edges_common(faceTag1, faceTag2, &nEdges, &edgesP))
        return false;

    bvector<PK_EDGE_t> edges;

    if (0 == nEdges) // Find common vertex for faces that aren't edge adjacent...
        {
        if (!checkCommonVertex)
            return false;

        bvector<PK_VERTEX_t> vertices;

        if (SUCCESS != PSolidTopo::GetFaceVertices(vertices, faceTag1))
            return false;

        bvector<PK_VERTEX_t> adjacentVertices;

        if (SUCCESS != PSolidTopo::GetFaceVertices(adjacentVertices, faceTag2))
            return false;

        for (PK_VERTEX_t vertexTag : adjacentVertices)
            {
            if (vertices.end() == std::find(vertices.begin(), vertices.end(), vertexTag))
                continue;

            PSolidTopo::GetVertexEdges(edges, vertexTag);
            break;
            }

        if (0 == edges.size())
            return false;
        }
    else
        {
        edges.resize(nEdges);

        for (int iEdge=0; iEdge < nEdges; iEdge++)
            edges[iEdge] = edgesP[iEdge];

        PK_MEMORY_free(edgesP);
        }

    for (PK_EDGE_t edgeTag : edges)
        {
        if (!PSolidUtil::IsSmoothEdge(edgeTag))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus getAdjacentFaces(bset<PK_FACE_t>& adjacentFaceTags, PK_FACE_t faceTag, bool includeVertex, bool includeRedundant, bool includeSmoothOnly, bool oneLevel)
    {
    int         nFaces = 0;
    PK_FACE_t*  facesP = nullptr;
    PK_FACE_ask_faces_adjacent_o_t options;

    PK_FACE_ask_faces_adjacent_o_m(options);
    options.include_vertex_connected = (includeSmoothOnly && !oneLevel) ? false : includeVertex; // Only need edge connected when not stopping at one level for smoothly connected faces...

    if (SUCCESS != PK_FACE_ask_faces_adjacent(1, &faceTag, &options, &nFaces, &facesP) || 0 == nFaces)
        return ERROR;

    for (int iFace=0; iFace < nFaces; iFace++)
        {
        if (!includeRedundant && facesAreRedundant(faceTag, facesP[iFace]))
            continue;

        if (includeSmoothOnly && !facesAreSmooth(faceTag, facesP[iFace], PK_LOGICAL_true == options.include_vertex_connected))
            continue;

        if (!oneLevel && includeSmoothOnly)
            {
            if (adjacentFaceTags.end() != std::find(adjacentFaceTags.begin(), adjacentFaceTags.end(), facesP[iFace]))
                continue;

            adjacentFaceTags.insert(facesP[iFace]);

            if (SUCCESS != getAdjacentFaces(adjacentFaceTags, facesP[iFace], includeVertex, includeRedundant, includeSmoothOnly, oneLevel))
                continue;
            }
        else
            {
            adjacentFaceTags.insert(facesP[iFace]);
            }
        }

    PK_MEMORY_free(facesP);

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetAdjacentFaces(bvector<ISubEntityPtr>& adjacentFaces, ISubEntityCR face, bool includeVertex, bool includeRedundant, bool includeSmoothOnly, bool oneLevel)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_EDGE_t   faceTag;

    if (0 == (faceTag = PSolidSubEntity::GetSubEntityTag(face)))
        return ERROR;

    bset<PK_FACE_t> adjacentFaceTags;

    if (SUCCESS != getAdjacentFaces(adjacentFaceTags, faceTag, includeVertex, includeRedundant, includeSmoothOnly, oneLevel))
        return ERROR;

    Transform   entityTransform = PSolidSubEntity::GetSubEntityTransform(face);

    for (PK_FACE_t adjFaceTag : adjacentFaceTags)
        {
        if (faceTag == adjFaceTag)
            continue;

        adjacentFaces.push_back(PSolidSubEntity::CreateSubEntity(adjFaceTag, entityTransform));
        }

    return (adjacentFaces.empty() ? ERROR : SUCCESS);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepUtil::GetSubEntityVertices(bvector<ISubEntityPtr>& vertices, bvector<ISubEntityPtr> const& subEntities)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    bset<PK_VERTEX_t> vertexTags;

    for (ISubEntityPtr subEntityPtr : subEntities)
        {
        PK_ENTITY_t entityTag;

        if (0 == (entityTag = PSolidSubEntity::GetSubEntityTag(*subEntityPtr)))
            continue;

        switch (subEntityPtr->GetSubEntityType())
            {
            case ISubEntity::SubEntityType::Vertex:
                {
                vertexTags.insert(entityTag);
                break;
                }

            case ISubEntity::SubEntityType::Edge:
                {
                PK_ENTITY_t entities[2];

                if (SUCCESS != PK_EDGE_ask_vertices(entityTag, entities))
                    break;

                if (PK_ENTITY_null != entities[0])
                    vertexTags.insert(entities[0]);

                if (PK_ENTITY_null != entities[1])
                    vertexTags.insert(entities[1]);
                break;
                }

            case ISubEntity::SubEntityType::Face:
                {
                int           nEntity = 0;
                PK_ENTITY_t*  entities = nullptr;

                if (SUCCESS != PK_FACE_ask_vertices(entityTag, &nEntity, &entities))
                    break;

                for (int iEntity = 0; iEntity < nEntity; ++iEntity)
                    vertexTags.insert(entities[iEntity]);

                PK_MEMORY_free(entities);
                break;
                }
            }
        }

    if (0 == vertexTags.size())
        return;

    Transform entityTransform = PSolidSubEntity::GetSubEntityTransform(*subEntities.front());

    for (PK_VERTEX_t vertexTag : vertexTags)
        vertices.push_back(PSolidSubEntity::CreateSubEntity(vertexTag, entityTransform));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepUtil::GetSubEntityEdges(bvector<ISubEntityPtr>& edges, bvector<ISubEntityPtr> const& subEntities)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    bset<PK_EDGE_t> edgeTags;

    for (ISubEntityPtr subEntityPtr : subEntities)
        {
        PK_ENTITY_t entityTag;

        if (0 == (entityTag = PSolidSubEntity::GetSubEntityTag(*subEntityPtr)))
            continue;

        switch (subEntityPtr->GetSubEntityType())
            {
            case ISubEntity::SubEntityType::Edge:
                {
                edgeTags.insert(entityTag);
                break;
                }

            case ISubEntity::SubEntityType::Face:
                {
                int           nEntity = 0;
                PK_ENTITY_t*  entities = nullptr;

                if (SUCCESS != PK_FACE_ask_edges(entityTag, &nEntity, &entities))
                    break;

                for (int iEntity = 0; iEntity < nEntity; ++iEntity)
                    edgeTags.insert(entities[iEntity]);

                PK_MEMORY_free(entities);
                break;
                }
            }
        }

    if (0 == edgeTags.size())
        return;

    Transform entityTransform = PSolidSubEntity::GetSubEntityTransform(*subEntities.front());

    for (PK_EDGE_t edgeTag : edgeTags)
        edges.push_back(PSolidSubEntity::CreateSubEntity(edgeTag, entityTransform));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetFaceParameterRange(ISubEntityCR subEntity, DRange1dR uRange, DRange1dR vRange)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_FACE_t   faceTag = PSolidSubEntity::GetSubEntityTag(subEntity);

    if (PK_ENTITY_null == faceTag)
        return ERROR;

    PK_UVBOX_t  uvBox;

    if (SUCCESS != PK_FACE_find_uvbox(faceTag, &uvBox))
        return ERROR;

    uRange = DRange1d::From(uvBox.param[0], uvBox.param[2]);
    vRange = DRange1d::From(uvBox.param[1], uvBox.param[3]);

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::GetEdgeParameterRange(ISubEntityCR subEntity, DRange1dR uRange)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_EDGE_t   edgeTag = PSolidSubEntity::GetSubEntityTag(subEntity);

    if (PK_ENTITY_null == edgeTag)
        return ERROR;

    PK_CURVE_t  curveTag;

    if (SUCCESS != PSolidTopo::GetCurveOfEdge(curveTag, &uRange.low, &uRange.high, nullptr, edgeTag))
        return ERROR;

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::EvaluateFace(ISubEntityCR subEntity, DPoint3dR point, DVec3dR normal, DVec3dR uDir, DVec3dR vDir, DPoint2dCR uvParam)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_VERTEX_t entityTag = PSolidSubEntity::GetSubEntityTag(subEntity);

    if (PK_ENTITY_null == entityTag || SUCCESS != PSolidUtil::EvaluateFace(point, normal, uDir, vDir, uvParam, entityTag))
        return ERROR;

    Transform entityTransform = PSolidSubEntity::GetSubEntityTransform(subEntity);

    entityTransform.Multiply(point);
    entityTransform.MultiplyMatrixOnly(normal);
    entityTransform.MultiplyMatrixOnly(uDir);
    entityTransform.MultiplyMatrixOnly(vDir);
    normal.Normalize();
    uDir.Normalize();
    vDir.Normalize();

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::EvaluateEdge(ISubEntityCR subEntity, DPoint3dR point, DVec3dR uDir, double uParam)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_VERTEX_t entityTag = PSolidSubEntity::GetSubEntityTag(subEntity);

    if (PK_ENTITY_null == entityTag || SUCCESS != PSolidUtil::EvaluateEdge(point, uDir, uParam, entityTag))
        return ERROR;

    Transform entityTransform = PSolidSubEntity::GetSubEntityTransform(subEntity);

    entityTransform.Multiply(point);
    entityTransform.MultiplyMatrixOnly(uDir);
    uDir.Normalize();

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::EvaluateVertex(ISubEntityCR subEntity, DPoint3dR point)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_VERTEX_t entityTag = PSolidSubEntity::GetSubEntityTag(subEntity);

    if (PK_ENTITY_null == entityTag || SUCCESS != PSolidUtil::GetVertex(point, entityTag))
        return ERROR;

    PSolidSubEntity::GetSubEntityTransform(subEntity).Multiply(point);

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::IsDisjointBody(IBRepEntityCR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    int          nRegions = 0;
    PK_REGION_t* regions = nullptr;

    if (SUCCESS != PK_BODY_ask_regions(PSolidUtil::GetEntityTag(entity), &nRegions, &regions) || 0 == nRegions)
        return false;

    uint32_t nSolid = 0;

    for (int iRegion=0; iRegion < nRegions; iRegion++)
        {
        PK_LOGICAL_t isSolid = PK_LOGICAL_false;

        if (SUCCESS == PK_REGION_is_solid(regions[iRegion], &isSolid) && PK_LOGICAL_true == isSolid)
            nSolid++;
        }

    PK_MEMORY_free(regions);

    return (nSolid > 1);
#else
    return false;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPlanarSurfaceClass(PK_CLASS_t surfaceClass)
    {
    switch (surfaceClass)
        {
        case PK_CLASS_plane:
        case PK_CLASS_circle:
        case PK_CLASS_ellipse:
            return true;

        default:
            return false;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::IsSingleFacePlanarSheetBody(IBRepEntityCR entity, bool& hasHoles)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t     entityTag = PSolidUtil::GetEntityTag(entity);
    PK_BODY_type_t  bodyType;

    PK_BODY_ask_type(entityTag, &bodyType);

    if (PK_BODY_type_sheet_c != bodyType)
        return false;

    int nFaces = 0;

    if (SUCCESS != PK_BODY_ask_faces(entityTag, &nFaces, nullptr) || 1 != nFaces)
        return false;

    PK_FACE_t faceTag = PK_ENTITY_null;

    if (SUCCESS != PK_BODY_ask_first_face(entityTag, &faceTag) || PK_ENTITY_null == faceTag)
        return false;

    PK_SURF_t    surfaceTag;
    PK_CLASS_t   surfaceClass;
    int          nLoops = 0;
    PK_LOOP_t*   loops = nullptr;
    PK_LOGICAL_t orientation;

    if (SUCCESS != PK_FACE_ask_oriented_surf(faceTag, &surfaceTag, &orientation) ||
        SUCCESS != PK_ENTITY_ask_class(surfaceTag, &surfaceClass) || !isPlanarSurfaceClass(surfaceClass) ||
        SUCCESS != PK_FACE_ask_loops(faceTag, &nLoops, &loops) || nLoops < 1)
        return false;

    PK_MEMORY_free(loops);
    hasHoles = (nLoops > 1);

    return true;
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::HasOnlyPlanarFaces(IBRepEntityCR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::HasOnlyPlanarFaces(PSolidUtil::GetEntityTag(entity));
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::HasCurvedFaceOrEdge(IBRepEntityCR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::HasCurvedFaceOrEdge(PSolidUtil::GetEntityTag(entity));
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::IsPlanarFace(ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::IsPlanarFace(PSolidSubEntity::GetSubEntityTag(subEntity));
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::IsSmoothEdge(ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::IsSmoothEdge(PSolidSubEntity::GetSubEntityTag(subEntity));
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::IsLaminarEdge(ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_EDGE_ask_type_t edgeTypes;

    return (SUCCESS == PK_EDGE_ask_type(PSolidSubEntity::GetSubEntityTag(subEntity), &edgeTypes) && PK_EDGE_type_laminar_c == edgeTypes.fins_type);
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::IsLinearEdge(ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_CURVE_t  curveTag = PK_ENTITY_null;

    if (SUCCESS != PSolidTopo::GetCurveOfEdge(curveTag, nullptr, nullptr, nullptr, PSolidSubEntity::GetSubEntityTag(subEntity)))
        return false;

    PK_CLASS_t  curveClass = 0;

    PK_ENTITY_ask_class(curveTag, &curveClass);

    return (PK_CLASS_line == curveClass);
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::Locate(IBRepEntityCR entity, DRay3dCR boresite, bvector<ISubEntityPtr>& intersectEntities, size_t maxFace, size_t maxEdge, size_t maxVertex, double maxEdgeDistance, double maxVertexDistance)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    bvector<PK_ENTITY_t> tmpIntersectEntityTags;
    bvector<DPoint3d> tmpIntersectPts;
    bvector<DPoint2d> tmpIntersectParams;
    bool hitFound = false;

    if (!PSolidUtil::LocateSubEntities(PSolidUtil::GetEntityTag(entity), entity.GetEntityTransform(), tmpIntersectEntityTags, tmpIntersectPts, tmpIntersectParams, maxFace, maxEdge, maxVertex, boresite, maxEdgeDistance, maxVertexDistance))
        return false;

    for (size_t iHit = 0; iHit < tmpIntersectEntityTags.size(); ++iHit)
        {
        ISubEntityPtr subEntity = PSolidSubEntity::CreateSubEntity(tmpIntersectEntityTags.at(iHit), entity);

        if (!subEntity.IsValid())
            continue;

        PSolidSubEntity::SetLocation(*subEntity, tmpIntersectPts.at(iHit), tmpIntersectParams.at(iHit));
        intersectEntities.push_back(subEntity);
        hitFound = true;
        }

    return hitFound;
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
ISubEntityPtr BRepUtil::ClosestSubEntity(IBRepEntityCR entity, DPoint3dCR testPt)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t entityTag;
    double      distance;
    DPoint2d    param;
    DPoint3d    point;

    if (!PSolidUtil::ClosestPoint(PSolidUtil::GetEntityTag(entity), entity.GetEntityTransform(), entityTag, point, param, distance, testPt))
        return nullptr;

    ISubEntityPtr subEntity = PSolidSubEntity::CreateSubEntity(entityTag, entity);

    if (subEntity.IsValid())
        PSolidSubEntity::SetLocation(*subEntity, point, param);

    return subEntity;
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ISubEntityPtr BRepUtil::ClosestFace(IBRepEntityCR entity, DPoint3dCR testPt, DVec3dCP preferredDir)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (IBRepEntity::EntityType::Wire == entity.GetEntityType())
        return nullptr;

    ISubEntityPtr closeEntity = BRepUtil::ClosestSubEntity(entity, testPt);

    if (!closeEntity.IsValid())
        return nullptr;

    switch (closeEntity->GetSubEntityType())
        {
        case ISubEntity::SubEntityType::Edge:
            {
            bvector<ISubEntityPtr> subEntities;

            if (SUCCESS != BRepUtil::GetEdgeFaces(subEntities, *closeEntity))
                return nullptr;

            if (nullptr == preferredDir)
                {
                closeEntity = subEntities.front();
                break;
                }

            DVec3d      normal, uDir, vDir, closeNormal;
            DPoint2d    param;
            DPoint3d    point, testPt;

            closeEntity->GetEdgeLocation(testPt, param.x);
            closeEntity = nullptr;

            for (ISubEntityPtr& subEntity : subEntities)
                {
                if (!BRepUtil::ClosestPointToFace(*subEntity, testPt, point, param))
                    continue;

                PSolidSubEntity::SetLocation(*subEntity, point, param);

                if (SUCCESS != BRepUtil::EvaluateFace(*subEntity, point, normal, uDir, vDir, param))
                    continue;

                if (!closeEntity.IsValid() || (normal.DotProduct(*preferredDir) > closeNormal.DotProduct(*preferredDir)))
                    {
                    closeEntity = subEntity;
                    closeNormal = normal;
                    }
                }
            break;
            }

        case ISubEntity::SubEntityType::Vertex:
            {
            bvector<ISubEntityPtr> subEntities;

            if (SUCCESS != BRepUtil::GetVertexFaces(subEntities, *closeEntity))
                return nullptr;

            if (nullptr == preferredDir)
                {
                closeEntity = subEntities.front();
                break;
                }

            DVec3d      normal, uDir, vDir, closeNormal;
            DPoint2d    param;
            DPoint3d    point, testPt;

            closeEntity->GetVertexLocation(testPt);
            closeEntity = nullptr;

            for (ISubEntityPtr& subEntity : subEntities)
                {
                if (!BRepUtil::ClosestPointToFace(*subEntity, testPt, point, param))
                    continue;

                PSolidSubEntity::SetLocation(*subEntity, point, param);

                if (SUCCESS != BRepUtil::EvaluateFace(*subEntity, point, normal, uDir, vDir, param))
                    continue;

                if (!closeEntity.IsValid() || (normal.DotProduct(*preferredDir) > closeNormal.DotProduct(*preferredDir)))
                    {
                    closeEntity = subEntity;
                    closeNormal = normal;
                    }
                }
            break;
            }
        }

    return closeEntity;
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::IsPointInsideBody(IBRepEntityCR entity, DPoint3dCR testPoint)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    DPoint3d    solidPoint = testPoint;
    Transform   dgnToSolid;

    dgnToSolid.InverseOf(entity.GetEntityTransform());
    dgnToSolid.Multiply(solidPoint);

    PK_TOPOL_t      topo;
    PK_VECTOR_t     point;
    PK_enclosure_t  encl;

    point.coord[0] = solidPoint.x;
    point.coord[1] = solidPoint.y;
    point.coord[2] = solidPoint.z;

    if (SUCCESS != PK_BODY_contains_vector(PSolidUtil::GetEntityTag(entity), point, &encl, &topo))
        return false;

    return (PK_enclosure_outside_c != encl);
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::LocateFace(ISubEntityCR subEntity, DRay3dCR boresite, bvector<DPoint3d>& intersectPts, bvector<DPoint2d>& intersectParams)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::RayTestFace(PSolidSubEntity::GetSubEntityTag(subEntity), PSolidSubEntity::GetSubEntityTransform(subEntity), intersectPts, intersectParams, boresite);
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::ClosestPointToFace(ISubEntityCR subEntity, DPoint3dCR testPt, DPoint3dR point, DPoint2dR param)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t faceTag = PSolidSubEntity::GetSubEntityTag(subEntity);
    Transform   entityTransform = PSolidSubEntity::GetSubEntityTransform(subEntity);
    double      distance;

    return PSolidUtil::ClosestPointToFace(faceTag, entityTransform, point, param, distance, testPt);
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::ClosestPointToEdge(ISubEntityCR subEntity, DPoint3dCR testPt, DPoint3dR point, double& param)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t edgeTag = PSolidSubEntity::GetSubEntityTag(subEntity);
    Transform   entityTransform = PSolidSubEntity::GetSubEntityTransform(subEntity);
    double      distance;

    return PSolidUtil::ClosestPointToEdge(edgeTag, entityTransform, point, param, distance, testPt);
#else
    return false;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void transformInertiaTensor(double inertia[3][3], RotMatrixCR rMatrix, double scale, bool isSolid)
    {
    // The new inertia tensor that accounts for rotation/scale is I1 = Q * I0 * Q^T...
    RotMatrix   q = rMatrix, i0, i1;

    memcpy (&i0, inertia, sizeof(i0));

    i1.InitProduct(q, i0);
    i1.InitProductRotMatrixRotMatrixTranspose(i1, q);

    memcpy(inertia, &i1, sizeof(i1));

    double  power = (isSolid ? 5.0 : 4.0);

    inertia[0][0] *= pow(scale, power);
    inertia[1][1] *= pow(scale, power);
    inertia[2][2] *= pow(scale, power);
    inertia[0][1] *= pow(scale, power);
    inertia[0][2] *= pow(scale, power);
    inertia[1][2] *= pow(scale, power);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::MassProperties(IBRepEntityCR entity, double* amount, double* periphery, DPoint3dP centroid, double inertia[3][3], double tolerance)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    Transform   entityTransform = entity.GetEntityTransform();
    bool        nonUniform = false;
    double      scale;
    DVec3d      scaleVector;
    DPoint3d    translation;
    RotMatrix   rMatrix;
    Transform   scaleTransform;

    entityTransform.GetTranslation(translation);
    entityTransform.GetMatrix(rMatrix);
    rMatrix.NormalizeColumnsOf(rMatrix, scaleVector);

    // Check for non-uniform scaling...it will need to be applied to the body...
    if (!DoubleOps::WithinTolerance(scaleVector.x, scaleVector.y, 1.0e-12) || !DoubleOps::WithinTolerance(scaleVector.x, scaleVector.z, 1.0e-12))
        {
        scale = DoubleOps::MinAbs(scaleVector.x, scaleVector.y, scaleVector.z);
        scaleVector.Scale(1.0 / scale);
        scaleTransform = Transform::FromFixedPointAndScaleFactors(DPoint3d::From(0.0, 0.0, 0.0), scaleVector.x, scaleVector.y, scaleVector.z);

        entityTransform.InitFrom(rMatrix, translation);
        nonUniform = true;
        }
    else
        {
        scale = scaleVector.x;
        }

    if (SUCCESS != PSolidUtil::MassProperties(amount, periphery, centroid, inertia, PSolidUtil::GetEntityTag(entity), nonUniform ? &scaleTransform : NULL, tolerance))
        return ERROR;

    switch (entity.GetEntityType())
        {
        case IBRepEntity::EntityType::Solid:
            {
            if (amount)
                *amount *= pow(scale, 3.0);

            if (periphery)
                *periphery *= pow(scale, 2.0);

            break;
            }

        case IBRepEntity::EntityType::Sheet:
            {
            if (amount)
                *amount *= pow(scale, 2.0);

            if (periphery)
                *periphery *= pow(scale, 1.0);

            break;
            }

        case IBRepEntity::EntityType::Wire:
            {
            if (amount)
                *amount *= pow(scale, 1.0);

            if (periphery)
                *periphery = 0.0;

            break;
            }

        default:
            return ERROR;
        }

    if (centroid)
        entityTransform.Multiply(*centroid);

    if (inertia)
        transformInertiaTensor(inertia, rMatrix, scale, IBRepEntity::EntityType::Solid == entity.GetEntityType());

    return SUCCESS;
#else
    return ERROR;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  01/2018
//---------------------------------------------------------------------------------------
BentleyStatus BRepUtil::UpdateFaceMaterialAttachments(IBRepEntityR target, bvector<ISubEntityPtr>& faces, FaceAttachment const* baseParams, FaceAttachment const* faceParams)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    IFaceMaterialAttachmentsP attachments = target.GetFaceMaterialAttachmentsP();

    if (nullptr == attachments)
        {
        if (nullptr == faceParams || faces.empty())
            return SUCCESS; // Caller isn't adding face attachments...and there aren't any currently, return SUCCESS...

        if (nullptr == baseParams)
            return ERROR; // Caller wants to update face attachments but didn't supply a valid baseParams which is required for a target that doesn't currently have face attachments...

        IFaceMaterialAttachmentsPtr newAttachments = PSolidUtil::CreateNewFaceAttachments(PSolidUtil::GetEntityTag(target), *baseParams);

        if (!newAttachments.IsValid())
            return ERROR; // Target entity type is invalid for face attachments (i.e. no faces)...

        PSolidUtil::SetFaceAttachments(target, newAttachments.get());

        if (nullptr == (attachments = target.GetFaceMaterialAttachmentsP()))
            return ERROR;
        }

    if (nullptr == baseParams && faces.empty())
        {
        PSolidAttrib::DeleteFaceMaterialIndexAttribute(PSolidUtil::GetEntityTag(target)); // Caller wants to remove all existing face attachments...
        PSolidUtil::SetFaceAttachments(target, nullptr); // Clear face attachments vector...

        return SUCCESS;
        }

    T_FaceAttachmentsVec& faceAttachmentsVec = attachments->_GetFaceAttachmentsVecR();
    size_t attachmentIndex = 0;
    bool baseChanged = false;

    if (nullptr != baseParams && faceAttachmentsVec.size() > 1 && faces.empty())
        {
        if (!(faceAttachmentsVec[0] == *baseParams))
            {
            faceAttachmentsVec[0] = *baseParams; // Update base symbology...
            baseChanged = true;
            }
        }

    if (nullptr != faceParams) // Adding/Updating face attachments, see if this symbology is already present...
        {
        T_FaceAttachmentsVec::iterator foundAttachment = std::find(faceAttachmentsVec.begin(), faceAttachmentsVec.end(), *faceParams);

        if (foundAttachment == faceAttachmentsVec.end())
            {
            faceAttachmentsVec.push_back(*faceParams);
            attachmentIndex = faceAttachmentsVec.size()-1;
            }
        else
            {
            attachmentIndex = std::distance(faceAttachmentsVec.begin(), foundAttachment);
            }
        }

    for (ISubEntityPtr subEntity : faces)
        PSolidAttrib::SetFaceMaterialIndexAttribute(PSolidSubEntity::GetSubEntityTag(*subEntity), (int32_t) attachmentIndex);

    T_FaceToAttachmentIndexMap faceToIndexMap;
    bool haveInvalidAttachment = !PSolidAttrib::PopulateFaceMaterialIndexMap(faceToIndexMap, PSolidUtil::GetEntityTag(target), faceAttachmentsVec.size());
    bool haveNonBaseAttachment = false; // See if all face assignments have been removed...

    for (T_FaceToAttachmentIndexMap::const_iterator curr = faceToIndexMap.begin(); curr != faceToIndexMap.end(); ++curr)
        {
        if (0 == curr->second)
            continue;

        haveNonBaseAttachment = true;
        break;
        }

    if (haveInvalidAttachment)
        {
        bvector<PK_FACE_t> allFaces;

        if (SUCCESS == PSolidTopo::GetBodyFaces(allFaces, PSolidUtil::GetEntityTag(target)))
            {
            for (PK_FACE_t faceTag : allFaces)
                {
                int32_t attachmentIndex = 0;

                if (SUCCESS == PSolidAttrib::GetFaceMaterialIndexAttribute(attachmentIndex, faceTag) && (attachmentIndex <= 0 || attachmentIndex >= faceAttachmentsVec.size()))
                    PSolidAttrib::DeleteFaceMaterialIndexAttribute(faceTag); // Remove invalid attribute index from face...
                }
            }
        }

    if (!haveNonBaseAttachment)
        {
        PSolidUtil::SetFaceAttachments(target, nullptr); // No face assignments remain to anything other than the base symbology, clear face attachments vector...

        return SUCCESS;
        }

    if (faceAttachmentsVec.size() > 2 || baseChanged) // Cull attachment symbology that is no longer used by any face...
        {
        bvector<bool>   matchesBase;
        bvector<bool>   usedIndices;
        bvector<size_t> newIndices;

        matchesBase.insert(matchesBase.begin(), faceAttachmentsVec.size(), false);
        usedIndices.insert(usedIndices.begin(), faceAttachmentsVec.size(), false);
        newIndices.insert(newIndices.begin(), faceAttachmentsVec.size(), 0);

        if (baseChanged)
            {
            // FaceAttachment will no longer be considered used if it matches new base...
            for (size_t i = 0; i < matchesBase.size(); ++i)
                matchesBase[i] = (0 == i ? true : (faceAttachmentsVec[0] == faceAttachmentsVec[i]));
            }

        for (T_FaceToAttachmentIndexMap::const_iterator curr = faceToIndexMap.begin(); curr != faceToIndexMap.end(); ++curr)
            usedIndices.at(curr->second) = !matchesBase.at(curr->second);

        bool    unusedAssignment = false;
        size_t  newIndex = 0;

        for (size_t i = 0; i < usedIndices.size(); ++i)
            {
            if (i == 0 || usedIndices.at(i))
                newIndices[i] = newIndex++;
            else
                unusedAssignment = true;
            }

        if (unusedAssignment)
            {
            T_FaceAttachmentsVec cleanAttachments;

            for (size_t i = 0; i < usedIndices.size(); ++i)
                {
                if (i == 0 || usedIndices.at(i))
                    cleanAttachments.push_back(faceAttachmentsVec.at(i));
                }

            if (cleanAttachments.size() < 2)
                {
                PSolidAttrib::DeleteFaceMaterialIndexAttribute(PSolidUtil::GetEntityTag(target)); // Base symbology was changed to match all face attachments...
                PSolidUtil::SetFaceAttachments(target, nullptr); // Clear face attachments vector...

                return SUCCESS;
                }

            faceAttachmentsVec = cleanAttachments;

            for (T_FaceToAttachmentIndexMap::const_iterator curr = faceToIndexMap.begin(); curr != faceToIndexMap.end(); ++curr)
                {
                size_t remappedIndex = newIndices.at(curr->second);

                if (remappedIndex == curr->second)
                    continue;

                PSolidAttrib::SetFaceMaterialIndexAttribute(curr->first, (int32_t) remappedIndex);
                }
            }
        }

    return SUCCESS;
#else
    return ERROR;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  11/2016
//---------------------------------------------------------------------------------------
uint32_t BRepUtil::TopologyID::AssignNewTopologyIds(IBRepEntityR entity, uint32_t nodeId)
    {
    if (0 == nodeId)
        {
        uint32_t highestNodeId, lowestNodeId;

        if (SUCCESS != FindNodeIdRange(entity, highestNodeId, lowestNodeId))
            return 0;

        nodeId = highestNodeId+1;
        }

    return (SUCCESS == AddNodeIdAttributes(entity, nodeId, false) ? nodeId : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::AddNodeIdAttributes(IBRepEntityR entity, uint32_t nodeId, bool overrideExisting)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (0 == nodeId)
        return ERROR;

    PK_ENTITY_t bodyTag = PSolidUtil::GetEntityTagForModify(entity);

    if (PK_ENTITY_null == bodyTag)
        return ERROR;

    return PSolidTopoId::AddNodeIdAttributes(bodyTag, nodeId, overrideExisting);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::ChangeNodeIdAttributes(IBRepEntityR entity, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (0 == nodeId)
        return ERROR;

    PK_ENTITY_t bodyTag = PSolidUtil::GetEntityTagForModify(entity);

    if (PK_ENTITY_null == bodyTag)
        return ERROR;

    return PSolidTopoId::ChangeNodeIdAttributes(bodyTag, nodeId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::DeleteNodeIdAttributes(IBRepEntityR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t bodyTag = PSolidUtil::GetEntityTagForModify(entity);

    if (PK_ENTITY_null == bodyTag)
        return ERROR;

    return PSolidTopoId::DeleteNodeIdAttributes(bodyTag);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::IncrementNodeIdAttributes(IBRepEntityR entity, int32_t increment)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (0 == increment)
        return ERROR;

    PK_ENTITY_t bodyTag = PSolidUtil::GetEntityTagForModify(entity);

    if (PK_ENTITY_null == bodyTag)
        return ERROR;

    return PSolidTopoId::IncrementNodeIdAttributes(bodyTag, increment);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::FindNodeIdRange(IBRepEntityCR entity, uint32_t& highestNodeId, uint32_t& lowestNodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidTopoId::FindNodeIdRange(PSolidUtil::GetEntityTag(entity), highestNodeId, lowestNodeId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::AddNodeIdAttribute(ISubEntityR subEntity, FaceId faceId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (ISubEntity::SubEntityType::Face != subEntity.GetSubEntityType())
        return ERROR;

    PK_FACE_t   faceTag = PSolidSubEntity::GetSubEntityTag(subEntity);

    return PSolidTopoId::AttachEntityId(faceTag, faceId.nodeId, faceId.entityId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::DeleteNodeIdAttribute(ISubEntityR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (ISubEntity::SubEntityType::Face != subEntity.GetSubEntityType())
        return ERROR;

    PK_FACE_t   faceTag = PSolidSubEntity::GetSubEntityTag(subEntity);

    PSolidTopoId::DeleteEntityId(faceTag);

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::IdFromFace(FaceId& faceId, ISubEntityCR subEntity, bool useHighestId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (ISubEntity::SubEntityType::Face != subEntity.GetSubEntityType())
        return ERROR;

    return PSolidTopoId::IdFromFace(faceId, PSolidSubEntity::GetSubEntityTag(subEntity), useHighestId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::IdFromEdge(EdgeId& edgeId, ISubEntityCR subEntity, bool useHighestId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (ISubEntity::SubEntityType::Edge != subEntity.GetSubEntityType())
        return ERROR;

    return PSolidTopoId::IdFromEdge(edgeId, PSolidSubEntity::GetSubEntityTag(subEntity), useHighestId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::IdFromVertex(VertexId& vertexId, ISubEntityCR subEntity, bool useHighestId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (ISubEntity::SubEntityType::Vertex != subEntity.GetSubEntityType())
        return ERROR;

    return PSolidTopoId::IdFromVertex(vertexId, PSolidSubEntity::GetSubEntityTag(subEntity), useHighestId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::FacesFromId(bvector<ISubEntityPtr>& subEntities, FaceId const& faceId, IBRepEntityCR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    bvector<PK_FACE_t> faceVector;

    if (SUCCESS != PSolidTopoId::FacesFromId(faceVector, faceId, PSolidUtil::GetEntityTag(entity)))
        return ERROR;

    for (PK_FACE_t faceTag: faceVector)
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(faceTag, entity));

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::EdgesFromId(bvector<ISubEntityPtr>& subEntities, EdgeId const& edgeId, IBRepEntityCR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    bvector<PK_EDGE_t> edgeVector;

    if (SUCCESS != PSolidTopoId::EdgesFromId(edgeVector, edgeId, PSolidUtil::GetEntityTag(entity)))
        return ERROR;

    for (PK_EDGE_t edgeTag: edgeVector)
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(edgeTag, entity));

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::VerticesFromId(bvector<ISubEntityPtr>& subEntities, VertexId const& vertexId, IBRepEntityCR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    bvector<PK_VERTEX_t> vertexVector;

    if (SUCCESS != PSolidTopoId::VerticesFromId(vertexVector, vertexId, PSolidUtil::GetEntityTag(entity)))
        return ERROR;

    for (PK_VERTEX_t vertexTag: vertexVector)
        subEntities.push_back(PSolidSubEntity::CreateSubEntity(vertexTag, entity));

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromCurveVector(IBRepEntityPtr& entityOut, CurveVectorCR curveVector, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidGeom::BodyFromCurveVector(entityOut, curveVector, nullptr, nodeId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr BRepUtil::Create::BodyToCurveVector(IBRepEntityCR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    switch (entity.GetEntityType())
        {
        case IBRepEntity::EntityType::Wire:
            return PSolidGeom::WireBodyToCurveVector(entity);

        case IBRepEntity::EntityType::Sheet:
            return PSolidGeom::PlanarSheetBodyToCurveVector(entity);

        default:
            return nullptr;
        }
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/18
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr BRepUtil::Create::PlanarFaceToCurveVector(ISubEntityCR face)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (ISubEntity::SubEntityType::Face != face.GetSubEntityType())
        return nullptr;

    PK_ENTITY_t faceTag = PSolidSubEntity::GetSubEntityTag(face);

    if (PK_ENTITY_null == faceTag)
        return nullptr;

    CurveVectorPtr curves = PSolidGeom::PlanarFaceToCurveVector(faceTag);

    if (!curves.IsValid())
        return nullptr;

    curves->TransformInPlace(PSolidSubEntity::GetSubEntityTransform(face));

    return curves;
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::CutProfileBodyFromOpenCurveVector(IBRepEntityPtr& entityOut, CurveVectorCR curves, DRange3dCR targetRange, DVec3dCP defaultNormal, bool reverseClosure, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (curves.IsAnyRegionType())
        return PSolidGeom::BodyFromCurveVector(entityOut, curves, nullptr, nodeId);

    if (!curves.IsOpenPath() || targetRange.IsNull())
        return ERROR;

    DRay3d rayS, rayE;

    if (!curves.GetStartEnd(rayS.origin, rayE.origin, rayS.direction, rayE.direction))
        return ERROR;

    rayS.direction.Negate();

    bool              haveIntercept, treatAsHole = false;
    double            fractionS, fractionE;
    DPoint3d          intercept;
    bvector<DPoint3d> closurePts;

    haveIntercept = DRay3d::ClosestApproachUnboundedRayUnboundedRay(fractionS, fractionE, intercept, intercept, rayS, rayE);

    if (haveIntercept && fractionS > 0.0 && fractionE > 0.0)
        {
        closurePts.push_back(rayE.origin);
        closurePts.push_back(intercept);
        closurePts.push_back(rayS.origin);

        treatAsHole = reverseClosure;
        }
    else
        {
        DRange3d    localRange;
        Transform   localToWorld, worldToLocal;

        if (!curves.IsPlanarWithDefaultNormal(localToWorld, worldToLocal, localRange, defaultNormal))
            return ERROR;

        DVec3d      planeDir = DVec3d::From(0.0, 0.0, 1.0);

        localToWorld.MultiplyMatrixOnly(planeDir);
        planeDir.Normalize();

        if (!haveIntercept)
            intercept.Interpolate(rayS.origin, 0.5, rayE.origin);

        DRay3d      testRay = DRay3d::FromOriginAndVector(intercept, planeDir);
        DPoint3d    corners[8];
        double      clearDist = 0.0;

        targetRange.Get8Corners(corners);

        for (int i=0; i<8; i++)
            {
            double   closeParam;
            DPoint3d closePt;

            if (!testRay.ProjectPointUnbounded(closePt, closeParam, corners[i]))
                continue;

            double thisDist = closePt.Distance(corners[i]);

            if (thisDist > clearDist)
                clearDist = thisDist;
            }

        clearDist = 1.5 * DoubleOps::Max(clearDist, intercept.Distance(rayE.origin), intercept.Distance(rayS.origin));

        DPoint3d    extE = DPoint3d::FromSumOf(rayE.origin, rayE.direction, clearDist);
        DPoint3d    extS = DPoint3d::FromSumOf(rayS.origin, rayS.direction, clearDist);

        closurePts.push_back(rayE.origin);

        if (rayE.origin.Distance(extE) > 1.0e-5)
            closurePts.push_back(extE);

        bool        evaluateArcPts = true;
        DEllipse3d  arc;

        if (haveIntercept)
            {
            arc = DEllipse3d::FromArcCenterStartEnd(intercept, extS, extE);

            if (reverseClosure)
                arc.ComplementSweep();
            }
        else if (rayE.direction.DotProduct(rayS.direction) < 0.999)
            {
            arc = DEllipse3d::FromPointsOnArc(extS, DPoint3d::FromSumOf(intercept, DVec3d::FromNormalizedCrossProduct(rayE.direction, planeDir), clearDist), extE);

            if (reverseClosure)
                arc.ComplementSweep();
            }
        else if (reverseClosure)
            {
            arc = DEllipse3d::FromPointsOnArc(extS, DPoint3d::FromSumOf(intercept, rayE.direction, -clearDist), extE);
            }
        else
            {
            evaluateArcPts = false;
            }

        if (evaluateArcPts)
            {
            DPoint3d  extA;

            arc.FractionParameterToPoint(extA, 0.75);
            closurePts.push_back(extA);

            arc.FractionParameterToPoint(extA, 0.5);
            closurePts.push_back(extA);

            arc.FractionParameterToPoint(extA, 0.25);
            closurePts.push_back(extA);
            }

        if (rayS.origin.Distance(extS) > 1.0e-5)
            closurePts.push_back(extS);

        closurePts.push_back(rayS.origin);
        }

    CurveVectorPtr tmpCurves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

    for (size_t iCurve = 0; iCurve < curves.size(); iCurve++)
        {
        ICurvePrimitivePtr curvePrimitive = curves.at(iCurve);

        if (0 == iCurve || iCurve == curves.size()-1)
            {
            bvector<DPoint3d> linearPts;

            switch (curvePrimitive->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    linearPts.push_back(curvePrimitive->GetLineCP()->point[0]);
                    linearPts.push_back(curvePrimitive->GetLineCP()->point[1]);
                    break;

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    linearPts = *curvePrimitive->GetLineStringCP();
                    break;

                default:
                    break;
                }

            if (0 != linearPts.size())
                {
                if (0 == iCurve && !closurePts.empty())
                    {
                    if (closurePts.size() > 1)
                        closurePts.pop_back();

                    linearPts[0] = closurePts.back();
                    closurePts.pop_back();
                    }

                if (iCurve == curves.size()-1 && !closurePts.empty())
                    {
                    if (closurePts.size() > 1)
                        closurePts.erase(closurePts.begin());

                    linearPts[linearPts.size()-1] = closurePts.front();
                    closurePts.erase(closurePts.begin());
                    }

                tmpCurves->Add(ICurvePrimitive::CreateLineString(linearPts));
                continue;
                }
            }

        tmpCurves->Add(curvePrimitive);
        }

    if (closurePts.size() > 1)
        tmpCurves->Add(ICurvePrimitive::CreateLineString(closurePts));

    if (SUCCESS != PSolidGeom::BodyFromCurveVector(entityOut, *tmpCurves, nullptr, nodeId))
        return ERROR;

    if (treatAsHole)
        {
        IBRepEntityPtr planeEntity;
        CurveVectorPtr planeCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine(DSegment3d::From(rayE.origin, rayS.origin)));
        DRange3d       expandedRange = entityOut->GetEntityRange();

        // NOTE: Always create a hole and not a notch in order to have more stable ids on inner loops...
        expandedRange.Extend(1.0e-3);
        expandedRange.Extend(targetRange);

        if (SUCCESS != BRepUtil::Create::SweptBodyFromOpenCurveVector(planeEntity, *planeCurve, expandedRange, &rayE.direction, true, 0L))
            return ERROR;

        if (SUCCESS != PSolidUtil::DoBoolean(planeEntity, &entityOut, 1, PK_boolean_subtract, PKI_BOOLEAN_OPTION_AllowDisjoint))
            return ERROR;

        if (nodeId)
            PSolidTopoId::AssignProfileBodyIds(PSolidUtil::GetEntityTag(*planeEntity), nodeId, true);

        entityOut = planeEntity;
        }

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::SweptBodyFromOpenCurveVector(IBRepEntityPtr& entityOut, CurveVectorCR curves, DRange3dCR targetRange, DVec3dCP defaultNormal, bool extend, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (!curves.IsOpenPath() || targetRange.IsNull())
        return ERROR;

    DVec3d      planeDir = DVec3d::From(0.0, 0.0, 1.0);
    DPoint3d    planePt = DPoint3d::From(0.0, 0.0, 0.0);
    DRange3d    localRange;
    Transform   localToWorld, worldToLocal;

    if (!curves.IsPlanarWithDefaultNormal(localToWorld, worldToLocal, localRange, defaultNormal))
        return ERROR;

    localToWorld.MultiplyMatrixOnly(planeDir);
    planeDir.Normalize();
    localToWorld.Multiply(planePt);

    DRay3d      sweepVector = DRay3d::FromOriginAndVector(planePt, planeDir);
    DRange1d    depthRange = targetRange.GetCornerRange(sweepVector);

    if (depthRange.Length() < 1.0e-5)
        return ERROR;

    DRay3d      rayS, rayE;

    if (!curves.GetStartEnd(rayS.origin, rayE.origin, rayS.direction, rayE.direction))
        return ERROR;

    rayS.direction.Negate();

    bool              isPhysicallyClosed = false;
    double            fractionS, fractionE;
    DPoint3d          intercept;
    bvector<DPoint3d> extendPtsS;
    bvector<DPoint3d> extendPtsE;

    if (DRay3d::ClosestApproachUnboundedRayUnboundedRay(fractionS, fractionE, intercept, intercept, rayS, rayE) && fractionS > 0.0 && fractionE > 0.0)
        {
        extendPtsS.push_back(intercept);
        extendPtsS.push_back(rayS.origin);

        extendPtsE.push_back(rayE.origin);
        extendPtsE.push_back(intercept);

        isPhysicallyClosed = true;
        }
    else
        {
        DRange1d rangeS = targetRange.GetCornerRange(rayS);
        DRange1d rangeE = targetRange.GetCornerRange(rayE);

        if (rangeS.high > 0.0)
            {
            extendPtsS.push_back(DPoint3d::FromSumOf(rayS.origin, rayS.direction, rangeS.high));
            extendPtsS.push_back(rayS.origin);
            }

        if (rangeE.high > 0.0)
            {
            extendPtsE.push_back(rayE.origin);
            extendPtsE.push_back(DPoint3d::FromSumOf(rayE.origin, rayE.direction, rangeE.high));
            }
        }

    CurveVectorPtr tmpCurves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    for (size_t iCurve = 0; iCurve < curves.size(); iCurve++)
        {
        ICurvePrimitivePtr curvePrimitive = curves.at(iCurve);

        if (0 == iCurve || iCurve == curves.size()-1)
            {
            bvector<DPoint3d> linearPts;

            switch (curvePrimitive->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    linearPts.push_back(curvePrimitive->GetLineCP()->point[0]);
                    linearPts.push_back(curvePrimitive->GetLineCP()->point[1]);
                    break;

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    linearPts = *curvePrimitive->GetLineStringCP();
                    break;

                default:
                    break;
                }

            if (0 != linearPts.size())
                {
                if (0 == iCurve && !extendPtsS.empty())
                    {
                    linearPts[0] = extendPtsS.front();
                    extendPtsS.clear();
                    }

                if (iCurve == curves.size()-1 && !extendPtsE.empty())
                    {
                    linearPts[linearPts.size()-1] = extendPtsE.back();
                    extendPtsE.clear();
                    }

                tmpCurves->Add(ICurvePrimitive::CreateLineString(linearPts));
                continue;
                }
            }

        tmpCurves->Add(curvePrimitive);
        }

    if (isPhysicallyClosed)
        {
        if (extendPtsE.size() > 1)
            tmpCurves->Add(ICurvePrimitive::CreateLineString(extendPtsE));

        if (extendPtsS.size() > 1)
            tmpCurves->Add(ICurvePrimitive::CreateLineString(extendPtsS));
        }
    else
        {
        if (extendPtsE.size() > 1)
            tmpCurves->Add(ICurvePrimitive::CreateLineString(extendPtsE));

        // NOTE: Can't insert start extension to front of curve as we don't want it to get entity id 1...
        }

    if (SUCCESS != PSolidGeom::BodyFromCurveVector(entityOut, *tmpCurves, nullptr, nodeId))
        return ERROR;

    // NOTE: Can now insert start extension and assign it the entity for the last segment...
    if (!isPhysicallyClosed && extendPtsS.size() > 1)
        {
        PK_BODY_t   bodyTag = PSolidUtil::GetEntityTag(*entityOut);
        PK_EDGE_t   edgeTag = PK_ENTITY_null;
        Transform   worldToSolid;

        worldToSolid.InverseOf(entityOut->GetEntityTransform());
        worldToSolid.Multiply (&extendPtsS.front(), (int) extendPtsS.size());

        if (SUCCESS != PSolidUtil::ImprintSegment(bodyTag, &edgeTag, &extendPtsS.front()))
            return ERROR;

        if (nodeId)
            {
            int numEdges = 0; // First edge of wire body will have been assigned an entity id of 1...
            PK_BODY_ask_edges(bodyTag, &numEdges, nullptr);
            PSolidTopoId::AttachEntityId (edgeTag, nodeId, numEdges);
            }
        }

    if (0.0 != depthRange.low)
        {
        DPoint3d translation;

        translation.Scale(sweepVector.direction, depthRange.low);

        if (!entityOut->ApplyTransform(Transform::From(translation)))
            return ERROR;
        }

    sweepVector.direction.ScaleToLength(depthRange.Length());

    return BRepUtil::Modify::SweepBody(*entityOut, sweepVector.direction);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromSolidPrimitive(IBRepEntityPtr& entityOut, ISolidPrimitiveCR primitive, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidGeom::BodyFromSolidPrimitive(entityOut, primitive, nodeId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromBSurface(IBRepEntityPtr& entityOut, MSBsplineSurfaceCR surface, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidGeom::BodyFromBSurface(entityOut, surface, nodeId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromPolyface(IBRepEntityPtr& entityOut, PolyfaceQueryCR meshData, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidGeom::BodyFromPolyface(entityOut, meshData, nodeId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromLoft(IBRepEntityPtr& entityOut, bvector<CurveVectorPtr>& profiles, bvector<CurveVectorPtr>* guides, bool periodic, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidGeom::BodyFromLoft(entityOut, &profiles.front(), profiles.size(), guides ? &guides->front() : nullptr, guides ? guides->size() : 0, periodic, nodeId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromSweep(IBRepEntityPtr& entityOut, CurveVectorCR profile, CurveVectorCR path, bool alignParallel, bool selfRepair, bool createSheet, DVec3dCP lockDirection, double const* twistAngle, double const* scale, DPoint3dCP scalePoint, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidGeom::BodyFromSweep(entityOut, profile, path, alignParallel, selfRepair, createSheet, lockDirection, twistAngle, scale, scalePoint, nodeId);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromExtrusionToBody(IBRepEntityPtr& entityOut, IBRepEntityCR extrudeTo, IBRepEntityCR profile, bool reverseDirection, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidGeom::BodyFromExtrusionToBody(entityOut, extrudeTo, profile, reverseDirection, nodeId);
#else
    return ERROR;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/2016
+===============+===============+===============+===============+===============+======*/
struct BRepRollbackMark : RefCountedBase
{
PK_MARK_t m_markTag = PK_ENTITY_null;

BRepRollbackMark() {PK_MARK_create(&m_markTag);}
~BRepRollbackMark() {PK_MARK_goto(m_markTag); PK_MARK_delete(m_markTag);}

}; // BRepRollbackMark
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/16
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<IRefCounted> BRepUtil::Modify::CreateRollbackMark()
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return new BRepRollbackMark();
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonas.Valiunas  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::IntersectSheetFaces(CurveVectorPtr& vectorOut, IBRepEntityCR sheet1, IBRepEntityCR sheet2)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (IBRepEntity::EntityType::Sheet != sheet1.GetEntityType())
        return ERROR;

    if (IBRepEntity::EntityType::Sheet != sheet2.GetEntityType())
        return ERROR;

    Transform   invTargetTransform;
    invTargetTransform.InverseOf(sheet1.GetEntityTransform());
    Transform   toolTransform;
    toolTransform.InitProduct(invTargetTransform, sheet2.GetEntityTransform());
    IBRepEntityPtr sheet2Clone = sheet2.Clone();
    PK_ENTITY_t sheet2Tag = PSolidUtil::GetEntityTag(*sheet2Clone);
    PK_ENTITY_t sheet1Tag = PSolidUtil::GetEntityTag(sheet1);
    PSolidUtil::TransformBody(sheet2Tag, toolTransform);

    if (PK_ENTITY_null == sheet1Tag || PK_ENTITY_null == sheet2Tag)
        return ERROR;

    PK_FACE_t  sheet2FaceTag = PK_ENTITY_null;
    PK_FACE_t  sheet1FaceTag = PK_ENTITY_null;

    if (SUCCESS != PK_BODY_ask_first_face(sheet1Tag, &sheet1FaceTag) || PK_ENTITY_null == sheet1FaceTag)
        return ERROR;

    if (SUCCESS != PK_BODY_ask_first_face(sheet2Tag, &sheet2FaceTag) || PK_ENTITY_null == sheet2FaceTag)
        return ERROR;

    PK_FACE_intersect_face_o_t options;
    PK_FACE_intersect_face_o_m(options);

    int numVectors = 0;
    PK_VECTOR_t* vectors = nullptr;
    int numCurves = 0;
    PK_CURVE_t* curves = nullptr;
    PK_INTERVAL_t* bounds = nullptr;
    PK_intersect_curve_t* curvesTypes = nullptr;

    BentleyStatus   status = (SUCCESS == PK_FACE_intersect_face(sheet1FaceTag, sheet2FaceTag, &options, &numVectors, &vectors, &numCurves, &curves, &bounds, &curvesTypes) ? SUCCESS : ERROR);

    if (SUCCESS == status)
        {
        vectorOut = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

        for (int i =0; i< numCurves; i++)
            {
            ICurvePrimitivePtr curvePrimitive = PSolidGeom::GetAsCurvePrimitive(curves[i], bounds[i], false);
            vectorOut->push_back(curvePrimitive);
            }

        // apply sheet1 trans to curvevec here
        vectorOut->TransformInPlace(sheet1.GetEntityTransform());
        }

    PK_ENTITY_delete(numCurves, curves);
    PK_MEMORY_free(curves);
    PK_MEMORY_free(bounds);
    PK_MEMORY_free(vectors);
    PK_MEMORY_free(curvesTypes);

    return status;
#else
    return ERROR;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_boolean_function_t getBooleanFunction(BRepUtil::Modify::BooleanMode op)
    {
    switch (op)
        {
        case BRepUtil::Modify::BooleanMode::Unite:
            return PK_boolean_unite;

        case BRepUtil::Modify::BooleanMode::Subtract:
            return PK_boolean_subtract;

        case BRepUtil::Modify::BooleanMode::Intersect:
            return PK_boolean_intersect;

        default:
            return PK_boolean_unite;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::BooleanOperation(IBRepEntityPtr& targetEntity, IBRepEntityPtr& toolEntity, BooleanMode op)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::DoBoolean(targetEntity, &toolEntity, 1, getBooleanFunction(op), PKI_BOOLEAN_OPTION_AllowDisjoint);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::BooleanOperation(IBRepEntityPtr& targetEntity, bvector<IBRepEntityPtr>& toolEntities, BooleanMode op)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::DoBoolean(targetEntity, &toolEntities.front(), toolEntities.size(), getBooleanFunction(op), PKI_BOOLEAN_OPTION_AllowDisjoint);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::BooleanCut(IBRepEntityPtr& target, IBRepEntityCR planarTool, CutDirectionMode directionMode, CutDepthMode depthMode, double depth, bool inside)
    {
    // NOTE: The MicroStation Connect version of this method specifies the profile as a CurveVector instead of a sheet body.
    //       There are some issues with that implementation worth mentioning in case anyone believes it's a good idea to bring that code over.
    //       1) AddNodeIdAttributes is being called instead of AssignProfileBodyIds; that results in less robust face ids.
    //       2) Open profiles are only extended not closed, so depth options other than "through all" aren't supported.
    //       3) The face ids of swept open profiles change depending on whether the start point is currently in or out of the target range.
    //       4) Does not resolve node Id conflicts between the target and tool body, this is likely just a bug and not intended.
    //       I prefer keeping this method simple and instead providing helper methods for creating the tool body.
    //       See BRepUtil::Create::CutProfileBodyFromOpenCurveVector and BRepUtil::Create::SweptBodyFromOpenCurveVector.
    //       The Connect result can be achieved using BRepUtil::Create::SweptBodyFromOpenCurveVector and a solid/surface boolean subtract.
#if defined (BENTLEYCONFIG_PARASOLID)
    if (!target.IsValid())
        return ERROR;

    PK_ENTITY_t planarToolTag = PSolidUtil::GetEntityTag(planarTool);

    if (PK_ENTITY_null == planarToolTag)
        return ERROR;

    bvector<PK_FACE_t> toolFaces;
    DRay3d faceRay;

    if (SUCCESS != PSolidTopo::GetBodyFaces(toolFaces, planarToolTag) ||
        SUCCESS != PSolidUtil::GetPlanarFaceData(&faceRay.origin, &faceRay.direction, toolFaces.front()))
        return ERROR;

    planarTool.GetEntityTransform().Multiply(faceRay.origin);
    planarTool.GetEntityTransform().MultiplyMatrixOnly(faceRay.direction); // This is probably not necessary.
    faceRay.direction.Normalize();

    DRange1d depthRange;

    if (CutDepthMode::Blind == depthMode)
        {
        depthRange.low  = (directionMode == CutDirectionMode::Forward)  ? 0.0 : -depth;
        depthRange.high = (directionMode == CutDirectionMode::Backward) ? 0.0 : depth;
        }
    else
        {
        DRange3d targetRange = target->GetEntityRange();

        if (targetRange.IsNull())
            return ERROR;

        depthRange = targetRange.GetCornerRange(faceRay);
        double margin = 1.0E-5 * depthRange.Length();

        switch (directionMode)
            {
            case CutDirectionMode::Forward:
                depthRange.low = 0.0;
                depthRange.high += margin;
                break;

            case CutDirectionMode::Backward:
                depthRange.high = 0.0;
                depthRange.low  -= margin;
                break;

            default:
                depthRange.ExtendBySignedShift(margin);
                break;
            }
        }

     if (!depthRange.IsPositiveLength())
        return ERROR;

    IBRepEntityPtr toolCopy = planarTool.Clone();

    if (!toolCopy.IsValid())
        return ERROR;

    if (depthRange.low != 0.0)
        {
        DPoint3d translation;

        translation.Scale(faceRay.direction, depthRange.low);

        if (!toolCopy->ApplyTransform(Transform::From(translation)))
            return ERROR;
        }

    DVec3d sweepVector;

    sweepVector.Scale(faceRay.direction, depthRange.Length());

    if (SUCCESS != SweepBody(*toolCopy, sweepVector))
        return ERROR;

    // NOTE: We do want to resolve id conflicts between target and tool...node ids won't be assigned/modified unless target already has node ids...
    return PSolidUtil::DoBoolean(target, &toolCopy, 1, inside ? PK_boolean_subtract : PK_boolean_intersect, PKI_BOOLEAN_OPTION_AllowDisjoint);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::SewBodies(bvector<IBRepEntityPtr>& sewnEntities, bvector<IBRepEntityPtr>& unsewnEntities, bvector<IBRepEntityPtr>& toolEntities, double gapWidthBound, size_t nIterations)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (toolEntities.size() < 2)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    bool                 isFirst = true;
    Transform            targetTransform, invTargetTransform;
    bvector<PK_ENTITY_t> toolEntityTags;

    // Get tool bodies in coordinates of target...
    for (IBRepEntityPtr& toolEntity : toolEntities)
        {
        PK_ENTITY_t toolEntityTag = PSolidUtil::GetEntityTagForModify(*toolEntity);

        if (PK_ENTITY_null == toolEntityTag)
            continue;

        if (isFirst)
            {
            isFirst = false;
            targetTransform = toolEntity->GetEntityTransform();
            invTargetTransform.InverseOf(targetTransform);
            invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&gapWidthBound, 1);
            }
        else
            {
            Transform   toolTransform;

            toolTransform.InitProduct(invTargetTransform, toolEntity->GetEntityTransform());
            PSolidUtil::TransformBody(toolEntityTag, toolTransform);
            }

        toolEntityTags.push_back(toolEntityTag);
        }

    int                       nSewnBodies = 0, nUnsewnBodies = 0, nProblems = 0;
    PK_BODY_t*                sewnBodyTags = NULL;
    PK_BODY_t*                unsewnBodyTags = NULL;
    PK_BODY_problem_group_t*  problemGroup = NULL;
    PK_BODY_sew_bodies_o_t    options;

    PK_BODY_sew_bodies_o_m(options);

    options.allow_disjoint_result = PK_LOGICAL_true;
    options.number_of_iterations  = (int) nIterations;

    BentleyStatus   status = (SUCCESS == PK_BODY_sew_bodies((int) toolEntityTags.size(), &toolEntityTags.front(), gapWidthBound, &options, &nSewnBodies, &sewnBodyTags, &nUnsewnBodies, &unsewnBodyTags, &nProblems, &problemGroup) ? SUCCESS : ERROR);

    if (SUCCESS == status)
        {
        if (sewnBodyTags)
            {
            for (int iSewn = 0; iSewn < nSewnBodies; ++iSewn)
                sewnEntities.push_back(PSolidUtil::CreateNewEntity(sewnBodyTags[iSewn], targetTransform, true));
            }

        if (unsewnBodyTags)
            {
            for (int iUnsewn = 0; iUnsewn < nUnsewnBodies; ++iUnsewn)
                unsewnEntities.push_back(PSolidUtil::CreateNewEntity(unsewnBodyTags[iUnsewn], targetTransform, true));
            }

        // Invalidate tool entities that are now reflected in sewn and unsewn lists...
        for (IBRepEntityPtr& toolEntity : toolEntities)
            toolEntity = nullptr;
        }
    else
        {
        // Undo transform of input entities...
        PK_MARK_goto(markTag);
        }

    PK_MEMORY_free(sewnBodyTags);
    PK_MEMORY_free(unsewnBodyTags);

    if (problemGroup)
        {
        for (int iProblem = 0; iProblem < nProblems; ++iProblem)
            PK_MEMORY_free(problemGroup[iProblem].edges);

        PK_MEMORY_free(problemGroup);
        }

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::DisjoinBody(bvector<IBRepEntityPtr>& output, IBRepEntityR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(entity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    bvector<PK_BODY_t> bodies;

    if (SUCCESS != PSolidUtil::DisjoinBody(bodies, targetEntityTag))
        return ERROR;

    Transform entityTransform = entity.GetEntityTransform();

    for (PK_BODY_t thisBody : bodies)
        {
        if (thisBody == targetEntityTag)
            continue;

        output.push_back(PSolidUtil::CreateNewEntity(thisBody, entityTransform, true));
        }

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::DeleteRedundantTopology(IBRepEntityR targetEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    PK_TOPOL_track_r_t tracking;
    PK_TOPOL_delete_redundant_2_o_s options;

    memset(&tracking, 0, sizeof(tracking));
    PK_TOPOL_delete_redundant_2_o_m(options);

    BentleyStatus   status = (SUCCESS == PK_TOPOL_delete_redundant_2(1, &targetEntityTag, &options, &tracking) ? SUCCESS : ERROR);

    PK_TOPOL_track_r_f(&tracking);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::ReverseOrientation(IBRepEntityR targetEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (IBRepEntity::EntityType::Sheet != targetEntity.GetEntityType())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    BentleyStatus   status = (SUCCESS == PK_BODY_reverse_orientation(targetEntityTag) ? SUCCESS : ERROR);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::SweepBody(IBRepEntityR targetEntity, DVec3dCR path)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    double      distance;
    DVec3d      pathVec;
    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    invTargetTransform.MultiplyMatrixOnly(pathVec, path);
    distance = pathVec.Normalize();

    BentleyStatus   status = PSolidUtil::SweepBodyVector(targetEntityTag, pathVec, distance);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::SpinBody(IBRepEntityR targetEntity, DRay3dCR axis, double angle)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    DRay3d      revolveAxis;
    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());

    invTargetTransform.Multiply(&revolveAxis.origin, &axis.origin, 1);
    invTargetTransform.MultiplyMatrixOnly(revolveAxis.direction, axis.direction);
    revolveAxis.direction.Normalize();

    BentleyStatus   status = PSolidUtil::SweepBodyAxis(targetEntityTag, revolveAxis.direction, revolveAxis.origin, angle);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::Emboss(IBRepEntityR targetEntity, IBRepEntityCR toolEntity, bool reverseDirection, DVec3dCP direction)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (IBRepEntity::EntityType::Sheet != toolEntity.GetEntityType())
        return ERROR;

    PK_ENTITY_t targetTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetTag)
        return ERROR;

    PK_ENTITY_t toolTag = PSolidUtil::GetEntityTag(toolEntity);

    if (PK_ENTITY_null == toolTag)
        return ERROR;

    Transform   invTargetTransform, toolTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    toolTransform.InitProduct(invTargetTransform, toolEntity.GetEntityTransform());

    bvector<PK_FACE_t> toolFaces;

    if (SUCCESS != PSolidTopo::GetBodyFaces(toolFaces, toolTag))
        return ERROR;

    DVec3d      drawDir = DVec3d::UnitZ();

    if (nullptr == direction)
        {
        PSolidUtil::GetPlanarFaceData(nullptr, &drawDir, toolFaces.front()); // Don't check status, allow non-planar surface...
        toolTransform.MultiplyMatrixOnly(drawDir);
        }
    else
        {
        invTargetTransform.MultiplyMatrixOnly(drawDir, *direction);
        }

    drawDir.Normalize();

    if (reverseDirection)
        drawDir.Negate();

    PK_BODY_emboss_o_t  options;
    PK_TOPOL_track_r_t  tracking;
    PK_TOPOL_local_r_t  results;

    PK_BODY_emboss_o_m(options);
    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    options.sidewall_data.sidewall = PK_emboss_sidewall_swept_c;
    options.convexity = PK_emboss_convexity_both_c; // Let pad or pocket be determined by whether cap is "above" or "below" target body according to emboss direction...
    options.overflow_data.laminar_walled = true;
    drawDir.GetComponents(options.sidewall_data.draw_direction.coord[0], options.sidewall_data.draw_direction.coord[1], options.sidewall_data.draw_direction.coord[2]);
    options.overflow_data.sweep_direction = options.sidewall_data.draw_direction;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    PK_ENTITY_copy(toolTag, &toolTag);
    PSolidUtil::TransformBody(toolTag, toolTransform);

    if (reverseDirection)
        PK_BODY_reverse_orientation(toolTag);

    BentleyStatus   status = (SUCCESS == PK_BODY_emboss(targetTag, toolTag, toolTag, &options, &tracking, &results) ? SUCCESS : ERROR);

    PK_ENTITY_delete(1, &toolTag);
    PK_TOPOL_track_r_f(&tracking);
    PK_TOPOL_local_r_f(&results);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::ThickenSheet(IBRepEntityR targetEntity, double frontDistance, double backDistance)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (IBRepEntity::EntityType::Sheet != targetEntity.GetEntityType())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    PK_BODY_thicken_o_t options;
    PK_TOPOL_track_r_t  tracking;
    PK_TOPOL_local_r_t  results;

    PK_BODY_thicken_o_m(options);
    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&frontDistance, 1);
    invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&backDistance, 1);

    BentleyStatus   status = (SUCCESS == PK_BODY_thicken_3(targetEntityTag, frontDistance, backDistance, 1.0e-6, &options, &tracking, &results) ? SUCCESS : ERROR);

    PK_TOPOL_local_r_f(&results);
    PK_TOPOL_track_r_f(&tracking);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::BlendEdges(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& edges, bvector<double> const& radii, bool propagate)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (edges.empty() || edges.size() != radii.size())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());

    for (size_t iEdge = 0; iEdge < edges.size(); ++iEdge)
        {
        ISubEntityR edge = *edges.at(iEdge);

        if (ISubEntity::SubEntityType::Edge != edge.GetSubEntityType())
            continue;

        double      radius = radii.at(iEdge);
        PK_EDGE_t   edgeTag = PSolidSubEntity::GetSubEntityTag(edge);

        invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&radius, 1);

        PK_EDGE_set_blend_constant_o_t options;

        PK_EDGE_set_blend_constant_o_m(options);
        options.properties.propagate = propagate ? PK_blend_propagate_yes_c : PK_blend_propagate_no_c;

        int         nBlend = 0;
        PK_EDGE_t*  blends = nullptr;

        PK_EDGE_set_blend_constant(1, &edgeTag, radius, &options, &nBlend, &blends);
        PK_MEMORY_free(blends);
        }

    BentleyStatus   status = PSolidUtil::FixBlends(targetEntityTag);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus computeChamferFaceData(DPoint3dR edgePoint, DVec3dR edgeTangent, DVec3dR faceNormal1, DVec3dR faceNormal2, double& edgeFacesAngle, PK_EDGE_t edgeTag)
    {
    bvector<PK_FACE_t> faces;

    if (SUCCESS != PSolidTopo::GetEdgeFaces(faces, edgeTag) || 2 != faces.size())
        return ERROR;

    bool        reversed;
    DRange1d    uRange;
    PK_CURVE_t  curveTag;

    if (SUCCESS != PSolidTopo::GetCurveOfEdge(curveTag, &uRange.low, &uRange.high, &reversed, edgeTag))
        return ERROR;

    double      uParam = (uRange.low + uRange.high) / 2.0;

    if (SUCCESS != PSolidUtil::EvaluateEdge(edgePoint, edgeTangent, uParam, edgeTag))
        return ERROR;

    edgeTangent.Normalize();

    if (reversed)
        edgeTangent.Negate();

    double   faceDist1, faceDist2;
    DPoint2d faceParam1, faceParam2;
    DPoint3d facePoint1, facePoint2;

    if (!PSolidUtil::ClosestPointToFace(faces.at(0), Transform::FromIdentity(), facePoint1, faceParam1, faceDist1, edgePoint) ||
        !PSolidUtil::ClosestPointToFace(faces.at(1), Transform::FromIdentity(), facePoint2, faceParam2, faceDist2, edgePoint))
        return ERROR;

    DVec3d uDir1, uDir2, vDir1, vDir2;

    if (SUCCESS != PSolidUtil::EvaluateFace(facePoint1, faceNormal1, uDir1, vDir1, faceParam1, faces.at(0)) ||
        SUCCESS != PSolidUtil::EvaluateFace(facePoint2, faceNormal2, uDir2, vDir2, faceParam2, faces.at(1)))
        return ERROR;

    DVec3d      axis;

    axis.CrossProduct(faceNormal1, edgeTangent);
    edgeFacesAngle = atan2(faceNormal2.DotProduct(axis), faceNormal2.DotProduct(faceNormal1));

    if (edgeFacesAngle < 0.0)
        edgeFacesAngle = Angle::Pi() + edgeFacesAngle;
    else
        edgeFacesAngle = Angle::Pi() - edgeFacesAngle;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void computeChamferRangesFromDistances(double& rightDist, double& leftDist, DPoint3dCR edgePoint, DVec3dCR edgeTangent, DVec3dCR faceNormal1, DVec3dCR faceNormal2)
    {
    DPoint3d    facePt1, facePt2, faceNormPt1, faceNormPt2;
    DVec3d      faceDir1, faceDir2;

    faceDir1.CrossProduct(faceNormal1, edgeTangent);
    faceDir2.CrossProduct(faceNormal2, edgeTangent);

    facePt1.SumOf(edgePoint, faceDir1, rightDist);
    facePt2.SumOf(edgePoint, faceDir2, -leftDist);

    faceNormPt1.SumOf(facePt1, faceNormal1, -1.0E-3);
    faceNormPt2.SumOf(facePt2, faceNormal2, -1.0E-3);

    double      fraction1, fraction2;
    DPoint3d    intercept1, intercept2;
    DRay3d      ray = DRay3d::FromOriginAndTarget(facePt1, faceNormPt1);
    DSegment3d  segment = DSegment3d::From(facePt2, faceNormPt2);

    DRay3d::ClosestApproachUnboundedRayBoundedSegment(fraction1, fraction2, intercept1, intercept2, ray, segment);

    rightDist = facePt1.Distance(intercept1);
    leftDist = facePt2.Distance(intercept1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool computeChamferReversed(bvector<PK_EDGE_t>& edgeTags, bvector<bool>& reversed, size_t nEdges, PK_EDGE_t testEdgeTag)
    {
    bvector<PK_FACE_t> testFaces;

    if (SUCCESS != PSolidTopo::GetEdgeFaces(testFaces, testEdgeTag) || 2 != testFaces.size())
        return false;

    for (size_t iEdge = 0; iEdge < nEdges; ++iEdge)
        {
        if (testEdgeTag == edgeTags[iEdge])
            continue;

        bvector<PK_FACE_t>  faces;

        if (SUCCESS != PSolidTopo::GetEdgeFaces(faces, edgeTags[iEdge]) || 2 != faces.size())
            continue;

        if (faces.at(0) == testFaces.at(0))
            return reversed.at(iEdge);

        if (faces.at(1) == testFaces.at(1))
            return reversed.at(iEdge);

        if (faces.at(0) == testFaces.at(1))
            return !reversed.at(iEdge);

        if (faces.at(1) == testFaces.at(0))
            return !reversed.at(iEdge);
        }

    return false;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::ChamferEdges (IBRepEntityR targetEntity, bvector<ISubEntityPtr>& edges, bvector<double> const& values1, bvector<double> const* values2, ChamferMode mode, bool propagate)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (edges.empty() || edges.size() != values1.size())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());

    bvector<bool>       reversed;
    bvector<PK_EDGE_t>  edgeTags;

    reversed.insert(reversed.begin(), edges.size(), false); // Default reversed flags per edge...

    for (size_t iEdge = 0; iEdge < edges.size(); ++iEdge)
        {
        if (ISubEntity::SubEntityType::Edge != edges[iEdge]->GetSubEntityType())
            continue;

        edgeTags.push_back(PSolidSubEntity::GetSubEntityTag(*edges[iEdge]));
        }

    for (size_t iEdge = 0; iEdge < edgeTags.size (); ++iEdge)
        {
        double      dist1 = 0.0, dist2 = 0.0, angle = 0.0, edgeFacesAngle = 0.0;
        DVec3d      edgeTangent, faceNormal1, faceNormal2;
        DPoint3d    edgePoint;
        PK_EDGE_t   edgeTag = edgeTags.at(iEdge);

        if (ChamferMode::Ranges != mode && SUCCESS != computeChamferFaceData(edgePoint, edgeTangent, faceNormal1, faceNormal2, edgeFacesAngle, edgeTag))
            continue;

        switch (mode)
            {
            case ChamferMode::Ranges:
                {
                dist1 = values1.at(iEdge);
                dist2 = values2 ? values2->at(iEdge) : dist1;
                invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&dist1, 1);
                invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&dist2, 1);
                break;
                }

            case ChamferMode::Length:
                {
                dist1 = values1.at(iEdge);
                invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&dist1, 1);
                dist1 = dist2 = (dist1 / sqrt(2.0 * (1.0 - cos(edgeFacesAngle))));
                break;
                }

            case ChamferMode::Distances:
                {
                dist1 = values1.at(iEdge);
                dist2 = values2 ? values2->at(iEdge) : dist1;
                invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&dist1, 1);
                invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&dist2, 1);
                break;
                }

            case ChamferMode::DistanceAngle:
                {
                dist1 = values1.at(iEdge);
                invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&dist1, 1);
                angle = values2 ? values2->at(iEdge) : Angle::PiOver2();
                dist2 = dist1 * sin(angle) / sin(acos(faceNormal1.DotProduct(faceNormal2) - angle));
                break;
                }

            case ChamferMode::AngleDistance:
                {
                if (nullptr == values2)
                    continue;

                dist2 = values2->at(iEdge);
                invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&dist2, 1);
                angle = values1.at(iEdge);
                dist1 = dist2 * sin(angle) / sin(acos(faceNormal1.DotProduct(faceNormal2) - angle));
                break;
                }
            }

        if (ChamferMode::Ranges != mode)
            {
            computeChamferRangesFromDistances(dist1, dist2, edgePoint, edgeTangent, faceNormal1, faceNormal2);

            // Make right/left distances for face loop consistent...check up to current edge for another edge on the same face...
            if (0 != iEdge && !DoubleOps::WithinTolerance(dist1, dist2, 1.0e-8) && computeChamferReversed(edgeTags, reversed, iEdge, edgeTag))
                {
                double  distT = dist2;

                dist2 = dist1;
                dist1 = distT;

                reversed[iEdge] = true; // Save reversed state for subsequent edge compares...
                }
            }

        PK_EDGE_set_blend_chamfer_o_t options;

        PK_EDGE_set_blend_chamfer_o_m(options);
        options.properties.propagate = propagate ? PK_blend_propagate_yes_c : PK_blend_propagate_no_c;

        double  minLength = (dist1 < dist2) ? dist1 : dist2;

        if (minLength < 100.0 * options.properties.tolerance)
            options.properties.tolerance = minLength * 0.01;

        int         nBlend = 0;
        PK_EDGE_t*  blends = nullptr;

        PK_EDGE_set_blend_chamfer(1, &edgeTag, dist1, dist2, NULL, &options, &nBlend, &blends);
        PK_MEMORY_free(blends);
        }

    BentleyStatus   status = PSolidUtil::FixBlends(targetEntityTag);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void coverRubberFaces(PK_BODY_t bodyTag)
    {
    int         nFaces = 0;
    PK_FACE_t*  faceTags = nullptr;

    PK_BODY_ask_faces(bodyTag, &nFaces, &faceTags);

    for (int iFace = 0; iFace < nFaces; ++iFace)
        {
        PK_SURF_t surfTag = PK_ENTITY_null;

        if (SUCCESS != PK_FACE_ask_surf(faceTags[iFace], &surfTag) || PK_ENTITY_null != surfTag)
            continue;

        PK_local_check_t localCheck;

        PK_FACE_attach_surf_fitting(faceTags[iFace], PK_LOGICAL_true, &localCheck);
        }

    PK_MEMORY_free(faceTags);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::HollowFaces(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& faces, double defaultDistance, bvector<double> const& distances, StepFacesOption addStep)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    double      shellOffset = defaultDistance;
    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&shellOffset, 1);

    int         nBodyFaces = 0;
    PK_FACE_t*  bodyFaceTags = NULL;

    PK_BODY_ask_faces(targetEntityTag, &nBodyFaces, &bodyFaceTags);

    // NOTE: Wanted to try not having the shell feature add it's node id to the product faces. Regardless of whether shelling
    //       inwards or outwards, the original faces are always the outer faces, so you won't be able to identity the shell by face
    //       reliably anyway. I'm currently incrementing the offset faces using the highest entity id for a given face's node id.
    //       The offset faces are then unique, but still identify the original feature(s). These ids will be more stable than just
    //       assigning the faces sequentially...and the user can still easily modify the shell thickness through handles or properties.
    //       A shell is something that is typically done once per solid anyway...
    bmap<uint32_t, uint32_t> nodeIdToHighestEntityIdMap;

    for (int iFace=0; iFace < nBodyFaces; iFace++)
        {
        FaceId faceId;

        if (SUCCESS != PSolidTopoId::IdFromEntity(faceId, bodyFaceTags[iFace], true))
            continue;

        bmap<uint32_t, uint32_t>::iterator found = nodeIdToHighestEntityIdMap.find(faceId.nodeId);

        if (found == nodeIdToHighestEntityIdMap.end())
            nodeIdToHighestEntityIdMap[faceId.nodeId] = faceId.entityId;
        else if (faceId.entityId > found->second)
            found->second = faceId.entityId;
        }

    bvector<double>     offsets;
    bvector<PK_FACE_t>  faceTags;

    faceTags.insert(faceTags.begin(), &bodyFaceTags[0], &bodyFaceTags[nBodyFaces]);
    offsets.insert(offsets.begin(), nBodyFaces, shellOffset);

    PK_MEMORY_free(bodyFaceTags);

    for (size_t iFace = 0; iFace < faces.size(); ++iFace)
        {
        if (ISubEntity::SubEntityType::Face != faces[iFace]->GetSubEntityType())
            continue;

        bvector<PK_FACE_t>::iterator  location;

        if (faceTags.end() == (location = std::find(faceTags.begin(), faceTags.end(), PSolidSubEntity::GetSubEntityTag(*faces[iFace]))))
            continue; // Not found?!?

        double  distance = distances[iFace];

        invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&distance, 1);
        offsets[location - faceTags.begin()] = distance;
        }

    PK_FACE_hollow_o_t options;

    PK_FACE_hollow_o_m(options);

    switch (addStep)
        {
        case StepFacesOption::AddNone:
            options.offset_step = PK_offset_step_no_c;
            break;

        case StepFacesOption::AddSmooth:
            options.offset_step = PK_offset_step_site_c;
            break;

        case StepFacesOption::AddNonCoincident:
            options.offset_step = PK_offset_step_pierce_c;
            break;

        case StepFacesOption::AddAll:
            options.offset_step = PK_offset_step_all_c;
            break;
        }

    PK_TOPOL_track_r_t tracking;
    PK_TOPOL_local_r_t results;

    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    BentleyStatus   status = (SUCCESS == PK_FACE_hollow_3((int) faceTags.size(), &faceTags.front(), &offsets.front(), 1.0e-6, &options, &tracking, &results) && tracking.n_track_records > 0) ? SUCCESS : ERROR; // TFS# 157939 - treat no changes as error.

    if (SUCCESS == status && !nodeIdToHighestEntityIdMap.empty())
        {
        for (int i=0; i<tracking.n_track_records; i++)
            {
            if (PK_TOPOL_track_create_c != tracking.track_records[i].track)
                continue; // Only want new IDs for offset faces...

            for (int j = 0; j < tracking.track_records[i].n_product_topols; j++)
                {
                FaceId faceId;

                if (SUCCESS != PSolidTopoId::IdFromEntity(faceId, tracking.track_records[i].product_topols[j], true))
                    continue;

                PSolidTopoId::DeleteEntityId(tracking.track_records[i].product_topols[j]);

                PK_CLASS_t entityClass;

                PK_ENTITY_ask_class(tracking.track_records[i].product_topols[j], &entityClass);

                if (PK_CLASS_face != entityClass)
                    continue;

                bmap<uint32_t, uint32_t>::iterator found = nodeIdToHighestEntityIdMap.find(faceId.nodeId);

                if (found == nodeIdToHighestEntityIdMap.end())
                    continue;

                PSolidTopoId::AttachEntityId(tracking.track_records[i].product_topols[j], faceId.nodeId, faceId.entityId + found->second);
                }
            }
        }

    PK_TOPOL_local_r_f(&results);
    PK_TOPOL_track_r_f(&tracking);

    if (SUCCESS == status && StepFacesOption::AddAll != addStep)
        coverRubberFaces(targetEntityTag);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::OffsetFaces(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& faces, bvector<double> const& distances, StepFacesOption addStep)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (faces.empty() || faces.size() != distances.size())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());

    bvector<double>    offsets;
    bvector<PK_FACE_t> faceTags;

    for (size_t iFace = 0; iFace < faces.size(); ++iFace)
        {
        if (ISubEntity::SubEntityType::Face != faces[iFace]->GetSubEntityType())
            continue;

        double  distance = distances[iFace];

        invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&distance, 1);
        offsets.push_back(distance);
        faceTags.push_back(PSolidSubEntity::GetSubEntityTag(*faces[iFace]));
        }

    PK_FACE_offset_o_t options;

    PK_FACE_offset_o_m(options);
    options.grow = PK_FACE_grow_auto_c;
    options.allow_disjoint = PK_LOGICAL_true;

    switch (addStep)
        {
        case StepFacesOption::AddNone:
            options.offset_step = PK_offset_step_no_c;
            break;

        case StepFacesOption::AddSmooth:
            options.offset_step = PK_offset_step_site_c;
            break;

        case StepFacesOption::AddNonCoincident:
            options.offset_step = PK_offset_step_pierce_c;
            break;

        case StepFacesOption::AddAll:
            options.offset_step = PK_offset_step_all_c;
            break;
        }

    PK_TOPOL_track_r_t tracking;
    PK_TOPOL_local_r_t results;

    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    BentleyStatus   status = (SUCCESS == PK_FACE_offset_2((int) faceTags.size(), &faceTags.front(), &offsets.front(), 1.0e-6, &options, &tracking, &results) && PK_local_status_ok_c == results.status) ? SUCCESS : ERROR;

    PK_TOPOL_local_r_f(&results);
    PK_TOPOL_track_r_f(&tracking);

    if (SUCCESS == status && StepFacesOption::AddAll != addStep)
        coverRubberFaces(targetEntityTag);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getBodyCurves(bvector<PK_CURVE_t>& curves, bvector<PK_INTERVAL_t>& intervals, PK_BODY_t bodyTag)
    {
    bvector<PK_EDGE_t>  edges;

    if (SUCCESS != PSolidTopo::GetBodyEdges(edges, bodyTag))
        return ERROR;

    for (PK_EDGE_t edgeTag : edges)
        {
        PK_CURVE_t      curveTag;

        if (SUCCESS != PK_EDGE_ask_curve(edgeTag, &curveTag)) // NOTE: Shouldn't need to check null curve/fin for current use cases (simple wire/sheets from CurveVectors)...
            continue;

        PK_INTERVAL_t   interval;

        if (SUCCESS != PK_EDGE_find_interval(edgeTag, &interval) && SUCCESS != PK_CURVE_ask_interval(curveTag, &interval))
            continue;

        curves.push_back(curveTag);
        intervals.push_back(interval);
        }

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::OffsetEdges(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& selectedEdges, DVec3dCR offsetDir, double offset, bool propagateSmooth, StepFacesOption addStep)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (selectedEdges.empty())
        return ERROR;

    ISubEntityPtr refEdgePtr = selectedEdges.front();

    if (ISubEntity::SubEntityType::Edge != refEdgePtr->GetSubEntityType())
        return ERROR;

    // Evaluate reference edge at pick location or center if not present...
    double      uParam;
    DVec3d      refEdgeTangent;
    DPoint3d    refEdgePoint;

    if (!refEdgePtr->GetEdgeLocation(refEdgePoint, uParam))
        {
        DRange1d    uRange;

        if (SUCCESS != BRepUtil::GetEdgeParameterRange(*refEdgePtr, uRange))
            return ERROR;

        uParam = (uRange.low + uRange.high) / 2.0;
        }

    if (SUCCESS != BRepUtil::EvaluateEdge(*refEdgePtr, refEdgePoint, refEdgeTangent, uParam))
        return ERROR;

    // Find face of edge whose normal is closest to offsetDir...this will be the face to apply the taper to...
    bvector<ISubEntityPtr> refEdgeFaces;

    if (SUCCESS != BRepUtil::GetEdgeFaces(refEdgeFaces, *refEdgePtr))
        return ERROR;

    double          lastDot = 0.0;
    DVec3d          refFaceNormal;
    ISubEntityPtr   refFacePtr;

    for (ISubEntityPtr facePtr : refEdgeFaces)
        {
        DPoint2d    uvParam;
        DVec3d      faceNormal, uDir, vDir;
        DPoint3d    facePoint;

        if (!BRepUtil::ClosestPointToFace(*facePtr, refEdgePoint, facePoint, uvParam))
            continue;

        if (SUCCESS != BRepUtil::EvaluateFace(*facePtr, facePoint, faceNormal, uDir, vDir, uvParam))
            continue;

        double      thisDot = offsetDir.DotProduct(faceNormal);

        if (refFacePtr.IsValid() && fabs(thisDot) < fabs(lastDot))
            continue;

        lastDot = thisDot;
        refFaceNormal = faceNormal;
        refFacePtr = facePtr;
        }

    if (!refFacePtr.IsValid())
        return ERROR;

    // Find an edge on the other side of the face to taper from the reference edge...
    PK_ENTITY_t refFaceTag = PSolidSubEntity::GetSubEntityTag(*refFacePtr);
    DRange3d    faceRange;

    if (SUCCESS != PSolidUtil::GetEntityRange(faceRange, refFaceTag))
        return ERROR;

    Transform   fwdTargetTransform, invTargetTransform;

    fwdTargetTransform = targetEntity.GetEntityTransform();
    invTargetTransform.InverseOf(fwdTargetTransform);

    DVec3d      extremeDir = DVec3d::FromNormalizedCrossProduct(refFaceNormal, refEdgeTangent);
    DPoint3d    points[2];

    extremeDir.ScaleToLength(faceRange.DiagonalDistance()); // Make sure projected curve completely splits face...
    points[0].SumOf(refEdgePoint, extremeDir);
    points[1].SumOf(refEdgePoint, DVec3d::FromScale(extremeDir, -1.0));

    PK_POINT_t      pointVec;
    PK_POINT_sf_t   pointSF;
    PK_ENTITY_t     toolTag = PK_ENTITY_null;
    PK_ENTITY_t     toolEdgeTag = PK_ENTITY_null;

    memset(&pointSF, 0, sizeof(pointSF));
    PK_POINT_create(&pointSF, &pointVec);

    if (SUCCESS != PK_POINT_make_minimum_body(pointVec, &toolTag))
        return ERROR;

    invTargetTransform.Multiply(points, 2);

    if (SUCCESS != PSolidUtil::ImprintSegment(toolTag, &toolEdgeTag, points))
        {
        PK_ENTITY_delete(1, &toolTag);
        return ERROR;
        }

    bvector<PK_CURVE_t>     toolCurves;
    bvector<PK_INTERVAL_t>  toolIntervals;

    getBodyCurves(toolCurves, toolIntervals, toolTag);

    PK_CURVE_project_o_t options;
    PK_CURVE_project_r_t results;
    PK_ENTITY_track_r_t  tracking;

    PK_CURVE_project_o_m(options);
    options.construction = PK_LOGICAL_false; // Create orphan curves instead of construction curves...note sure if those get persisted...

    memset(&results, 0, sizeof(results));
    memset(&tracking, 0, sizeof(tracking));

    PK_ERROR_code_t failureCode = PK_CURVE_project((int) toolCurves.size(), &toolCurves.front(), &toolIntervals.front(), 1, &refFaceTag, &options, &results, &tracking);

    double      lastDist = 0.0;
    DPoint3d    extremePt = DPoint3d::FromZero();

    if (SUCCESS == failureCode && results.n_geoms > 0)
        {
        for (int iResult = 0; iResult < results.n_geoms; ++iResult)
            {
            for (int iPt = 0; iPt < 2; ++iPt)
                {
                PK_VECTOR_t pnt;

                if (SUCCESS != PK_CURVE_eval(results.geoms[iResult].geom, results.geoms[iResult].range.value[iPt], 0, &pnt))
                    continue;

                DPoint3d    point = DPoint3d::From(pnt.coord[0], pnt.coord[1], pnt.coord[2]);

                fwdTargetTransform.Multiply(point);

                double      thisDist = point.Distance(refEdgePoint);

                if (thisDist <= lastDist)
                    continue;

                extremePt = point;
                lastDist = thisDist;
                }

            PK_ENTITY_delete(1, &results.geoms[iResult].geom); // Doesn't seem like PK_ENTITY_track_r_f/PK_CURVE_project_r_f would free orphan geometry?
            }
        }

    PK_ENTITY_delete(1, &toolTag);
    PK_ENTITY_track_r_f(&tracking);
    PK_CURVE_project_r_f(&results);

    if (0.0 == lastDist)
        return ERROR;

    // The closest sub-entity to the extreme point should be an edge if there wasn't a problem above and geometry can be sensibly tapered...
    ISubEntityPtr extremePtr = ClosestSubEntity(targetEntity, extremePt);

    if (!extremePtr.IsValid())
        return ERROR;

    // Get the other face from the opposite edge (not the one being tapered) to use to define the taper direction...
    bvector<ISubEntityPtr> extremeEdgeFaces;

    if (SUCCESS != BRepUtil::GetEdgeFaces(extremeEdgeFaces, *extremePtr))
        return ERROR;

    extremePtr = nullptr;

    for (ISubEntityPtr facePtr : extremeEdgeFaces)
        {
        if (facePtr->IsEqual(*refFacePtr))
            continue;

        extremePtr = facePtr;
        break;
        }

    if (!extremePtr.IsValid())
        return ERROR;

    // Calculate the taper angle using the extreme point, offset point, and taper direction...
    DPoint2d    uvParam;
    DVec3d      normal, uDir, vDir;
    DPoint3d    point;

    if (!BRepUtil::ClosestPointToFace(*extremePtr, extremePt, point, uvParam))
        return ERROR;

    if (SUCCESS != BRepUtil::EvaluateFace(*extremePtr, point, normal, uDir, vDir, uvParam))
        return ERROR;

    DPoint3d    offsetPt = DPoint3d::FromSumOf(refEdgePoint, offsetDir, offset);
    DVec3d      taperDir = DVec3d::FromStartEndNormalize(offsetPt, extremePt);
    double      angle = fabs(normal.AngleTo(taperDir));

    if (angle < Angle::FromDegrees(1.0).Radians() || angle > Angle::FromDegrees(89.0).Radians())
        return ERROR;

    if (offsetDir.DotProduct(refFaceNormal) < 0.0)
        angle = -angle;

    ISubEntityPtr commonFacePtr;
    bvector<ISubEntityPtr> taperFaces;
    bvector<ISubEntityPtr> refEntities;
    bvector<double> angles;

    angles.push_back(angle);

    // Add faces from any other edges (or tangent edges) that share a common face with the taper face found from the reference edge...
    if (selectedEdges.size() > 1 || propagateSmooth)
        {
        for (ISubEntityPtr facePtr : refEdgeFaces)
            {
            if (facePtr->IsEqual(*refFacePtr))
                continue;

            commonFacePtr = facePtr;
            break;
            }
        }

    if (!commonFacePtr.IsValid())
        {
        taperFaces.push_back(refFacePtr);
        refEntities.push_back(extremePtr);
        }
    else
        {
        PK_FACE_t commonFaceTag = PSolidSubEntity::GetSubEntityTag(*commonFacePtr);
        bset<PK_FACE_t> faceTags;

        for (ISubEntityPtr edgePtr : selectedEdges)
            {
            if (ISubEntity::SubEntityType::Edge != edgePtr->GetSubEntityType())
                continue;

            PK_EDGE_t edgeTag;

            if (0 == (edgeTag = PSolidSubEntity::GetSubEntityTag(*edgePtr)))
                continue;

            if (propagateSmooth)
                {
                bvector<PK_EDGE_t> smoothEdges;

                if (SUCCESS != PSolidTopo::GetTangentBlendEdges(smoothEdges, edgeTag))
                    continue;

                for (PK_EDGE_t smoothEdgeTag : smoothEdges)
                    {
                    bvector<PK_FACE_t> otherFaces;

                    if (SUCCESS != PSolidTopo::GetEdgeFaces(otherFaces, smoothEdgeTag))
                        continue;

                    if (otherFaces.end() == std::find(otherFaces.begin(), otherFaces.end(), commonFaceTag))
                        continue;

                    for (PK_FACE_t otherFaceTag : otherFaces)
                        {
                        if (otherFaceTag == commonFaceTag)
                            continue;

                        faceTags.insert(otherFaceTag);
                        }
                    }
                }
            else
                {
                bvector<PK_FACE_t> otherFaces;

                if (SUCCESS != PSolidTopo::GetEdgeFaces(otherFaces, edgeTag))
                    continue;

                if (otherFaces.end() == std::find(otherFaces.begin(), otherFaces.end(), commonFaceTag))
                    continue;

                for (PK_FACE_t otherFaceTag : otherFaces)
                    {
                    if (otherFaceTag == commonFaceTag)
                        continue;

                    faceTags.insert(otherFaceTag);
                    }
                }
            }

        if (0 == faceTags.size())
            return ERROR;

        for (PK_EDGE_t faceTag : faceTags)
            {
            taperFaces.push_back(PSolidSubEntity::CreateSubEntity(faceTag, fwdTargetTransform));
            refEntities.push_back(extremePtr);
            }
        }

    return BRepUtil::Modify::TaperFaces(targetEntity, taperFaces, refEntities, BRepUtil::IsPlanarFace(*extremePtr) ? normal : DVec3d::From(0.0, 0.0, 0.0), angles, addStep);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::TransformFaces(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& faces, bvector<Transform> const& transforms, StepFacesOption addStep)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (faces.empty() || faces.size() != transforms.size())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());

    bvector<PK_TRANSF_t> transfs;
    bvector<PK_FACE_t>   faceTags;

    for (size_t iFace = 0; iFace < faces.size(); ++iFace)
        {
        if (ISubEntity::SubEntityType::Face != faces[iFace]->GetSubEntityType())
            continue;

        PK_TRANSF_t transfTag = PK_ENTITY_null;
        Transform   faceTransform = Transform::FromProduct(invTargetTransform, transforms[iFace], targetEntity.GetEntityTransform());

        PSolidUtil::CreateTransf(transfTag, faceTransform);
        transfs.push_back(transfTag);
        faceTags.push_back(PSolidSubEntity::GetSubEntityTag(*faces[iFace]));
        }

    PK_FACE_transform_o_t options;

    PK_FACE_transform_o_m(options);
    options.grow = PK_FACE_grow_auto_c;

    switch (addStep)
        {
        case StepFacesOption::AddNone:
            options.transform_step = PK_transform_step_no_c;
            break;

        case StepFacesOption::AddSmooth:
            options.transform_step = PK_transform_step_smooth_c; // Change to PK_transform_step_smooth_site_c whenever it's implemented...
            break;

        case StepFacesOption::AddNonCoincident:
            options.transform_step = PK_transform_step_not_coi_c;
            break;

        case StepFacesOption::AddAll:
            options.transform_step = PK_transform_step_all_c;
            break;
        }

    PK_TOPOL_track_r_t tracking;
    PK_TOPOL_local_r_t results;

    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    BentleyStatus   status = (SUCCESS == PK_FACE_transform_2((int) faceTags.size(), &faceTags.front(), &transfs.front(), 1.0e-6, &options, &tracking, &results) && PK_local_status_ok_c == results.status) ? SUCCESS : ERROR;

    // NOTE: Try to remove any redundant step faces that may have been created...
    if (SUCCESS == status && tracking.n_track_records > 0 && StepFacesOption::AddNonCoincident == addStep)
        {
        bvector<PK_FACE_t> resultFaces;

        for (int iResult = 0; iResult < tracking.n_track_records; ++iResult)
            {
            if (PK_TOPOL_track_create_c != tracking.track_records[iResult].track)
                continue;

            for (int iProduct = 0; iProduct < tracking.track_records[iResult].n_product_topols; ++iProduct)
                {
                PK_ENTITY_t entityTag = tracking.track_records[iResult].product_topols[iProduct];
                PK_CLASS_t  entityClass;

                PK_ENTITY_ask_class(entityTag, &entityClass);

                if (PK_CLASS_face != entityClass)
                    continue;

                resultFaces.push_back(entityTag);
                }
            }

        if (resultFaces.size() > 0)
            {
            PK_TOPOL_track_r_t tracking2;
            PK_TOPOL_delete_redundant_2_o_s options2;

            memset(&tracking2, 0, sizeof(tracking2));
            PK_TOPOL_delete_redundant_2_o_m(options2);
            options2.max_topol_dimension = PK_TOPOL_dimension_1_c;
            options2.scope = PK_redundant_merge_out_c;

            // NOTE: I think we're ok not setting a mark...
            PK_TOPOL_delete_redundant_2((int) resultFaces.size(), &resultFaces.front(), &options2, &tracking2);
            PK_TOPOL_track_r_f(&tracking2);
            }
        }

    PK_TOPOL_local_r_f(&results);
    PK_TOPOL_track_r_f(&tracking);

    PK_ENTITY_delete ((int) transfs.size(), &transfs.front());

    if (SUCCESS == status && StepFacesOption::AddAll != addStep)
        coverRubberFaces(targetEntityTag);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::TransformEdges(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& edges, bvector<Transform> const& transforms, StepFacesOption addStep)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (edges.empty())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    Transform   fwdTargetTransform, invTargetTransform;

    fwdTargetTransform = targetEntity.GetEntityTransform();
    invTargetTransform.InverseOf(fwdTargetTransform);

    bvector<ISubEntityPtr> faces;
    bvector<Transform> faceTransforms;
    bvector<PK_CURVE_t> curves;

    for (size_t iEdge = 0; iEdge < edges.size(); ++iEdge)
        {
        if (ISubEntity::SubEntityType::Edge != edges[iEdge]->GetSubEntityType())
            continue;

        PK_EDGE_t  edgeTag = PSolidSubEntity::GetSubEntityTag(*edges[iEdge]);
        PK_CURVE_t curveTag = PK_ENTITY_null;
        DRange1d   uRangeE;

        if (SUCCESS != PSolidTopo::GetCurveOfEdge(curveTag, &uRangeE.low, &uRangeE.high, nullptr, edgeTag))
            continue;

        // Ignore duplicate faces from edges that share the same curve as a previous edge...ex. creating T roof peak...
        bvector<PK_CURVE_t>::iterator it = std::find(curves.begin(), curves.end(), curveTag);
        bool isSameCurve = (it != curves.end());
        curves.push_back(curveTag);

        double      uParam;
        DPoint3d    edgePoint;

        // Choose start, middle, or end of an open edge based on pick location for better curved edge behavior...
        if (edges[iEdge]->GetEdgeLocation(edgePoint, uParam))
            {
            PK_EDGE_ask_type_t edgeTypes;

            // Use pick location for closed or ring edge...
            if (SUCCESS != PK_EDGE_ask_type(edgeTag, &edgeTypes) || PK_EDGE_type_open_c == edgeTypes.vertex_type)
                {
                double  delta = fabs(uRangeE.high-uRangeE.low) * 0.25;

                if (fabs(uParam-uRangeE.high) < delta)
                    uParam = uRangeE.high;
                else if (fabs(uParam-uRangeE.low) < delta)
                    uParam = uRangeE.low;
                else
                    uParam = (uRangeE.high+uRangeE.low) * 0.5;
                }
            }
        else
            {
            uParam = (uRangeE.high+uRangeE.low) * 0.5;
            }

        DVec3d      edgeTangent;

        if (SUCCESS != BRepUtil::EvaluateEdge(*edges[iEdge], edgePoint, edgeTangent, uParam))
            continue;

        bvector<ISubEntityPtr> edgeFaces;

        if (SUCCESS != BRepUtil::GetEdgeFaces(edgeFaces, *edges[iEdge]))
            continue;

        Transform   edgeTransform = transforms[iEdge];

        for (ISubEntityPtr facePtr : edgeFaces)
            {
            bvector<ISubEntityPtr>::iterator it = std::find_if(faces.begin(), faces.end(), [&](ISubEntityPtr const& face) { return facePtr->IsEqual(*face); });

            if (it != faces.end())
                {
                if (!isSameCurve)
                    faceTransforms[it - faces.begin()] = edgeTransform; // If more than 1 edge from same face is selected...just apply input transform to face...
                continue;
                }

            DRange1d uRangeF, vRangeF;

            if (SUCCESS != BRepUtil::GetFaceParameterRange(*facePtr, uRangeF, vRangeF))
                continue;

            DPoint2d    uvParam = DPoint2d::From((uRangeF.high+uRangeF.low) * 0.5, (vRangeF.high+vRangeF.low) * 0.5);
            DVec3d      faceNormal, uDir, vDir;
            DPoint3d    facePoint;

            if (SUCCESS != BRepUtil::EvaluateFace(*facePtr, facePoint, faceNormal, uDir, vDir, uvParam))
                continue;

            DVec3d      pickDir = DVec3d::FromStartEndNormalize(edgePoint, facePoint);
            DVec3d      testDir = DVec3d::FromNormalizedCrossProduct(edgeTangent, faceNormal);

            if (testDir.DotProduct(pickDir) < 0.0)
                testDir.Negate();

            PK_VECTOR_t dir1, dir2, dir3;

            invTargetTransform.MultiplyMatrixOnly((DVec3dR) dir1, testDir);
            invTargetTransform.MultiplyMatrixOnly((DVec3dR) dir2, faceNormal);
            invTargetTransform.MultiplyMatrixOnly((DVec3dR) dir3, edgeTangent);

            PK_VECTOR_t extremeVec;
            PK_TOPOL_t  topolTag = PK_ENTITY_null;

            if (SUCCESS != PK_FACE_find_extreme(PSolidSubEntity::GetSubEntityTag(*facePtr), dir1, dir2, dir3, &extremeVec, &topolTag))
                continue;

            ISubEntityPtr extremePtr = PSolidSubEntity::CreateSubEntity(topolTag, fwdTargetTransform);

            if (!extremePtr.IsValid())
                continue;

            DPoint3d refDirPt, extremePt = DPoint3d::From(extremeVec.coord[0], extremeVec.coord[1], extremeVec.coord[2]);

            edgeTransform.Multiply(refDirPt, edgePoint);
            fwdTargetTransform.Multiply(extremePt);
            DPlane3d::FromOriginAndNormal(edgePoint, edgeTangent).ProjectPoint(extremePt, extremePt);
            DPlane3d::FromOriginAndNormal(edgePoint, faceNormal).ProjectPoint(extremePt, extremePt);

            DVec3d xVec = DVec3d::FromStartEndNormalize(extremePt, edgePoint);
            DVec3d yVec = DVec3d::FromStartEndNormalize(extremePt, refDirPt);
            DVec3d zVec = DVec3d::FromNormalizedCrossProduct(xVec, yVec);
            double angle = fabs(xVec.PlanarAngleTo(yVec, zVec));

            if (angle < Angle::FromDegrees(1.0).Radians())
                continue;

            RotMatrix rMatrix = RotMatrix::FromVectorAndRotationAngle(zVec, angle);
            Transform faceTransform = Transform::FromMatrixAndFixedPoint(rMatrix, extremePt);

            faceTransforms.push_back(faceTransform);
            faces.push_back(facePtr);
            }
        }

    return TransformFaces(targetEntity, faces, faceTransforms, addStep);
#else
    return ERROR;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
//=======================================================================================
// @bsiclass
//=======================================================================================
struct ImprintIndices {size_t m_indices[2];};
struct SurfaceIndices {size_t m_indices[3];};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FaceVertexData
{
ISubEntityPtr           m_facePtr;
DPoint3d                m_facePoint;
DVec3d                  m_faceNormal;
bvector<DPoint3d>       m_rawPts;
bvector<DPoint3d>       m_adjPts;
bvector<ImprintIndices> m_imprint;
bvector<SurfaceIndices> m_surface;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool SetupFromFace(ISubEntityR face)
    {
    if (!BRepUtil::IsPlanarFace(face))
        return false;

    bvector<PK_LOOP_t> faceLoops;

    if (SUCCESS != PSolidTopo::GetFaceLoops(faceLoops, PSolidSubEntity::GetSubEntityTag(face)) || faceLoops.size() > 1)
        return false;

    DRange1d uRange, vRange;

    if (SUCCESS != BRepUtil::GetFaceParameterRange(face, uRange, vRange))
        return false;

    DPoint2d uvParam = DPoint2d::From((uRange.high+uRange.low) * 0.5, (vRange.high+vRange.low) * 0.5);
    DVec3d   uDir, vDir;

    if (SUCCESS != BRepUtil::EvaluateFace(face, m_facePoint, m_faceNormal, uDir, vDir, uvParam))
        return false;

    m_facePtr = &face;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GetLoopVertices(bvector<ISubEntityPtr>& loopVertices, ISubEntityCR vertex)
    {
    bvector<ISubEntityPtr> vertexEdges;

    if (SUCCESS != BRepUtil::GetVertexEdges(vertexEdges, vertex))
        return;

    ISubEntityPtr faceEdgeLoopPtr;

    for (ISubEntityPtr edgePtr : vertexEdges)
        {
        bvector<ISubEntityPtr> edgeFaces;

        if (SUCCESS != BRepUtil::GetEdgeFaces(edgeFaces, *edgePtr))
            continue;

        bvector<ISubEntityPtr>::iterator it2 = std::find_if(edgeFaces.begin(), edgeFaces.end(), [&](ISubEntityPtr const& face) { return m_facePtr->IsEqual(*face); });

        if (it2 == edgeFaces.end())
            continue;

        faceEdgeLoopPtr = edgePtr;
        break;
        }

    if (!faceEdgeLoopPtr.IsValid())
        return;

    bvector<ISubEntityPtr> loopEdges;

    if (SUCCESS != BRepUtil::GetLoopEdgesFromEdge(loopEdges, *faceEdgeLoopPtr, *m_facePtr) || loopEdges.size() < 3)
        return;

    for (ISubEntityPtr edgePtr : loopEdges)
        {
        bvector<ISubEntityPtr> edgeVertices;

        if (SUCCESS != BRepUtil::GetEdgeVertices(edgeVertices, *edgePtr) || edgeVertices.size() != 2)
            continue;

        ISubEntityPtr vertex1Ptr = edgeVertices.at(0);
        ISubEntityPtr vertex2Ptr = edgeVertices.at(1);

        if (loopVertices.empty())
            {
            loopVertices.push_back(vertex1Ptr);
            loopVertices.push_back(vertex2Ptr);
            }
        else if (vertex1Ptr->IsEqual(*loopVertices.back()))
            {
            if (!vertex2Ptr->IsEqual(*loopVertices.front()))
                loopVertices.push_back(vertex2Ptr);
            }
        else if (vertex2Ptr->IsEqual(*loopVertices.back()))
            {
            if (!vertex1Ptr->IsEqual(*loopVertices.front()))
                loopVertices.push_back(vertex1Ptr);
            }
        else if (vertex1Ptr->IsEqual(*loopVertices.front()))
            {
            if (!vertex2Ptr->IsEqual(*loopVertices.back()))
                loopVertices.insert(loopVertices.begin(), vertex2Ptr);
            }
        else if (vertex2Ptr->IsEqual(*loopVertices.front()))
            {
            if (!vertex1Ptr->IsEqual(*loopVertices.back()))
                loopVertices.insert(loopVertices.begin(), vertex1Ptr);
            }
        else
            {
            BeAssert(false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsColinearIndices(size_t iPt0, size_t iPt1, size_t iPt2)
    {
    DPoint3d pts[3];

    pts[0] = m_rawPts[iPt0];
    pts[1] = m_rawPts[iPt1];
    pts[2] = m_rawPts[iPt2];

    if (bsiGeom_isDPoint3dArrayColinear(pts, 3, (1.0e-12 * (1.0 + pts[0].Magnitude()))))
        return true;

    DEllipse3d planeArc = DEllipse3d::FromArcCenterStartEnd(m_adjPts.at(iPt1), m_adjPts.at(iPt0), m_adjPts.at(iPt2));
    DVec3d     zAxis = planeArc.CrossProductOfBasisVectors();

    zAxis.Normalize();

    return (fabs(zAxis.DotProduct(m_faceNormal)) > .99999 && DPlane3d::FromOriginAndNormal(m_facePoint, m_faceNormal).Evaluate(planeArc.center) < 1.0e-8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GetNewSurfaceForResultFace(PK_PLANE_sf_t& plane, PK_FACE_t faceTag, TransformCR bodyToCurveTrans, TransformCR curveToBodyTrans)
    {
    bvector<PK_VERTEX_t> resultVertices;

    if (SUCCESS != PSolidTopo::GetFaceVertices(resultVertices, faceTag))
        return false;

    for (PK_VERTEX_t vertexTag : resultVertices)
        {
        DPoint3d point;

        if (SUCCESS != PSolidUtil::GetVertex(point, vertexTag))
            continue;

        bodyToCurveTrans.Multiply(point);

        for (SurfaceIndices& surface : m_surface)
            {
            if (!point.IsEqual(m_rawPts.at(surface.m_indices[1]), 1.0e-5))
                continue;

            DEllipse3d planeArc = DEllipse3d::FromArcCenterStartEnd(m_adjPts.at(surface.m_indices[1]), m_adjPts.at(surface.m_indices[0]), m_adjPts.at(surface.m_indices[2]));

            if (m_faceNormal.DotProduct(planeArc.CrossProductOfBasisVectors()) < 0.0)
                planeArc = DEllipse3d::FromNegateVector90(planeArc); // Preserve original surface nornmal...

            curveToBodyTrans.Multiply(planeArc);

            DVec3d zAxis = planeArc.CrossProductOfBasisVectors();
            DVec3d xAxis = planeArc.vector0;

            zAxis.Normalize();
            xAxis.Normalize();

            plane.basis_set.location.coord[0] = planeArc.center.x;
            plane.basis_set.location.coord[1] = planeArc.center.y;
            plane.basis_set.location.coord[2] = planeArc.center.z;

            plane.basis_set.axis.coord[0] = zAxis.x;
            plane.basis_set.axis.coord[1] = zAxis.y;
            plane.basis_set.axis.coord[2] = zAxis.z;

            plane.basis_set.ref_direction.coord[0] = xAxis.x;
            plane.basis_set.ref_direction.coord[1] = xAxis.y;
            plane.basis_set.ref_direction.coord[2] = xAxis.z;

            return true;
            }
        }

    return false;
    }

}; // FaceVertexData
#endif

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus transformVertices(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& vertices, bvector<Transform> const& transforms)
    {
    if (vertices.empty())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    bvector<ISubEntityPtr>  facesChecked;
    bvector<FaceVertexData> faceData;

    for (size_t iVertex = 0; iVertex < vertices.size(); ++iVertex)
        {
        if (ISubEntity::SubEntityType::Vertex != vertices[iVertex]->GetSubEntityType())
            continue;

        bvector<ISubEntityPtr> vertexFaces;

        if (SUCCESS != BRepUtil::GetVertexFaces(vertexFaces, *vertices[iVertex]))
            continue;

        for (ISubEntityPtr facePtr : vertexFaces)
            {
            bvector<ISubEntityPtr>::iterator it = std::find_if(facesChecked.begin(), facesChecked.end(), [&](ISubEntityPtr const& face) { return facePtr->IsEqual(*face); });

            if (it != facesChecked.end())
                continue;

            facesChecked.push_back(facePtr);

            FaceVertexData data;

            if (!data.SetupFromFace(*facePtr))
                continue;

            bvector<ISubEntityPtr> loopVertices;

            data.GetLoopVertices(loopVertices, *vertices[iVertex]);

            if (loopVertices.size() < 3)
                continue;

            for (ISubEntityPtr vertexPtr : loopVertices)
                {
                DPoint3d    vertexPoint;

                if (SUCCESS != BRepUtil::EvaluateVertex(*vertexPtr, vertexPoint))
                    continue;

                data.m_rawPts.push_back(vertexPoint);

                bvector<ISubEntityPtr>::iterator it2 = std::find_if(vertices.begin(), vertices.end(), [&](ISubEntityPtr const& vertex) { return vertexPtr->IsEqual(*vertex); });

                if (it2 != vertices.end())
                    transforms[it2 - vertices.begin()].Multiply(vertexPoint);

                data.m_adjPts.push_back(vertexPoint);
                }

            if (data.m_rawPts.size() != loopVertices.size())
                continue;

            size_t  iNextLimit = data.m_rawPts.size()-1, iPrevLimit = 0;
            bool    allVerticesMove = true;

            for (size_t iPt = 0; iPt < data.m_rawPts.size(); ++iPt)
                {
                if (!data.m_rawPts[iPt].IsEqual(data.m_adjPts[iPt], 1.0e-10))
                    continue; // If this vertex moves we won't create an imprint from it (may create imprint to it)...

                allVerticesMove = false;

                size_t  iNext = (iPt < data.m_rawPts.size()-1) ? iPt+1 : 0;

                if (!data.m_rawPts[iNext].IsEqual(data.m_adjPts[iNext], 1.0e-10)) // Next point moves, need imprint and new surface for split face...
                    {
                    size_t     iNextNext = (iNext < data.m_rawPts.size()-1) ? iNext+1 : 0;

                    if (iNext == iPrevLimit && !data.m_imprint.empty())
                        {
                        bvector<ImprintIndices>::iterator it2 = std::find_if(data.m_imprint.begin(), data.m_imprint.end(), [&](ImprintIndices const& imprint) { return imprint.m_indices[1] == iNext; });

                        if (it2 != data.m_imprint.end())
                            it2->m_indices[1] = iPt; // Crossing segment, may not require split but still needs replace surface!

                        continue;
                        }

                    iNextLimit = iNextNext;

                    if (data.IsColinearIndices(iPt, iNext, iNextNext))
                        continue;

                    ImprintIndices imprint;

                    imprint.m_indices[0] = iPt;
                    imprint.m_indices[1] = iNextNext;

                    bvector<ImprintIndices>::iterator it2 = std::find_if(data.m_imprint.begin(), data.m_imprint.end(), [&](ImprintIndices const& imprint2) { return imprint.m_indices[0] == imprint2.m_indices[1] && imprint.m_indices[1] == imprint2.m_indices[0]; });

                    if (it2 == data.m_imprint.end()) // NOTE: Parasolid seems ok with duplicate imprint...but it's easy enough to filter out...
                        data.m_imprint.push_back(imprint);

                    SurfaceIndices surface;

                    surface.m_indices[0] = iPt;
                    surface.m_indices[1] = iNext;
                    surface.m_indices[2] = iNextNext;

                    data.m_surface.push_back(surface);
                    }

                size_t  iPrev = (iPt > 0) ? iPt-1 : data.m_rawPts.size()-1;

                if (!data.m_rawPts[iPrev].IsEqual(data.m_adjPts[iPrev], 1.0e-10)) // Previous point moves, need imprint and new surface for split face...
                    {
                    size_t     iPrevPrev = (iPrev > 0) ? iPrev-1 : data.m_rawPts.size()-1;

                    if (iPrev == iNextLimit && !data.m_imprint.empty())
                        {
                        bvector<ImprintIndices>::iterator it2 = std::find_if(data.m_imprint.begin(), data.m_imprint.end(), [&](ImprintIndices const& imprint) { return imprint.m_indices[1] == iPrev; });

                        if (it2 != data.m_imprint.end())
                            it2->m_indices[1] = iPt; // Crossing segment, may not require split but still needs replace surface!

                        continue;
                        }

                    iPrevLimit = iPrevPrev;

                    if (data.IsColinearIndices(iPt, iPrev, iPrevPrev))
                        continue;

                    ImprintIndices imprint;

                    imprint.m_indices[0] = iPt;
                    imprint.m_indices[1] = iPrevPrev;

                    bvector<ImprintIndices>::iterator it2 = std::find_if(data.m_imprint.begin(), data.m_imprint.end(), [&](ImprintIndices const& imprint2) { return imprint.m_indices[0] == imprint2.m_indices[1] && imprint.m_indices[1] == imprint2.m_indices[0]; });

                    if (it2 == data.m_imprint.end()) // NOTE: Parasolid seems ok with duplicate imprint...but it's easy enough to filter out...
                        data.m_imprint.push_back(imprint);

                    SurfaceIndices surface;

                    surface.m_indices[0] = iPt;
                    surface.m_indices[1] = iPrev;
                    surface.m_indices[2] = iPrevPrev;

                    data.m_surface.push_back(surface);
                    }
                }

            if (allVerticesMove)
                {
                SurfaceIndices surface;

                surface.m_indices[0] = 0;
                surface.m_indices[1] = 1;
                surface.m_indices[2] = 2;

                data.m_surface.push_back(surface);
                }

            if (0 == data.m_imprint.size() && 0 == data.m_surface.size())
                continue;

            faceData.push_back(data);
            }
        }

    if (faceData.empty())
        return ERROR;

    bvector<PK_FACE_t>    replaceFaces;
    bvector<PK_SURF_t>    replaceSurfs;
    bvector<PK_LOGICAL_t> replaceSenses;

    for (FaceVertexData& data : faceData)
        {
        PK_ENTITY_t  targetTag = PSolidSubEntity::GetSubEntityTag(*data.m_facePtr);
        Transform    bodyToCurveTrans = PSolidSubEntity::GetSubEntityTransform(*data.m_facePtr), curveToBodyTrans;

        curveToBodyTrans.InverseOf(bodyToCurveTrans);

        if (data.m_imprint.empty() && 1 == data.m_surface.size())
            {
            PK_PLANE_sf_t plane;

            if (!data.GetNewSurfaceForResultFace(plane, targetTag, bodyToCurveTrans, curveToBodyTrans))
                continue;

            PK_PLANE_t planeTag;

            if (SUCCESS != PK_PLANE_create(&plane, &planeTag))
                continue;

            replaceFaces.push_back(targetTag);
            replaceSurfs.push_back(planeTag);
            replaceSenses.push_back(PK_LOGICAL_true);
            continue;
            }

        bvector<PK_ENTITY_t>    toolBodies;
        bvector<PK_CURVE_t>     toolCurves;
        bvector<PK_INTERVAL_t>  toolIntervals;

        for (ImprintIndices& imprint : data.m_imprint)
            {
            DSegment3d segment = DSegment3d::From(data.m_rawPts.at(imprint.m_indices[0]), data.m_rawPts.at(imprint.m_indices[1]));

            if (segment.IsSinglePoint())
                continue;

            PK_ENTITY_t toolTag = PK_ENTITY_null;

            if (SUCCESS != PSolidGeom::BodyFromCurveVector(toolTag, nullptr, *CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine(segment)), curveToBodyTrans, false, nullptr))
                continue;

            toolBodies.push_back(toolTag);
            getBodyCurves(toolCurves, toolIntervals, toolTag);
            }

        if (toolBodies.empty())
            return ERROR;

        PK_CURVE_project_o_t options;
        PK_CURVE_project_r_t results;
        PK_ENTITY_track_r_t  tracking;

        PK_CURVE_project_o_m(options);
        options.function = PK_proj_function_imprint_c;

        memset(&results, 0, sizeof(results));
        memset(&tracking, 0, sizeof(tracking));

        BentleyStatus status = (SUCCESS == PK_CURVE_project((int) toolCurves.size(), &toolCurves.front(), &toolIntervals.front(), 1, &targetTag, &options, &results, &tracking)) ? SUCCESS : ERROR;

        bvector<PK_FACE_t> resultFaces;

        if (SUCCESS == status && tracking.n_track_records > 0)
            {
            for (int iResult = 0; iResult < tracking.n_track_records; ++iResult)
                {
                for (int iProduct = 0; iProduct < tracking.track_records[iResult].n_product_entities; ++iProduct)
                    {
                    PK_ENTITY_t entityTag = tracking.track_records[iResult].product_entities[iProduct];
                    PK_CLASS_t  entityClass;

                    PK_ENTITY_ask_class(entityTag, &entityClass);

                    if (PK_CLASS_edge != entityClass)
                        continue;

                    bvector<PK_FACE_t> edgeFaces;

                    if (SUCCESS != PSolidTopo::GetEdgeFaces(edgeFaces, entityTag))
                        continue;

                    for (PK_FACE_t faceTag : edgeFaces)
                        resultFaces.push_back(faceTag);
                    }
                }
            }

        PK_ENTITY_track_r_f(&tracking);
        PK_CURVE_project_r_f(&results);

        PK_ENTITY_delete((int) toolBodies.size(), &toolBodies.front());

        if (SUCCESS != status)
            {
            PK_ENTITY_delete((int) replaceSurfs.size(), &replaceSurfs.front());

            return ERROR;
            }

        for (size_t iResult = 0; iResult < resultFaces.size(); ++iResult)
            {
            PK_PLANE_sf_t plane;

            if (!data.GetNewSurfaceForResultFace(plane, resultFaces[iResult], bodyToCurveTrans, curveToBodyTrans))
                continue;

            PK_PLANE_t planeTag;

            if (SUCCESS != PK_PLANE_create(&plane, &planeTag))
                continue;

            replaceFaces.push_back(resultFaces[iResult]);
            replaceSurfs.push_back(planeTag);
            replaceSenses.push_back(PK_LOGICAL_true);
            }
        }

    if (replaceFaces.empty())
        return ERROR;

    PK_FACE_replace_surfs_o_t options;

    PK_FACE_replace_surfs_o_m(options);
    options.merge = PK_replace_merge_out_c;

    PK_TOPOL_local_r_t results;
    PK_TOPOL_track_r_t tracking;

    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    BentleyStatus status = (SUCCESS == PK_FACE_replace_surfs_3((int) replaceFaces.size(), &replaceFaces.front(), &replaceSurfs.front(), &replaceSenses.front(), 1.0e-5, &options, &tracking, &results) && PK_local_status_ok_c == results.status) ? SUCCESS : ERROR;

    PK_TOPOL_local_r_f(&results);
    PK_TOPOL_track_r_f(&tracking);

    if (SUCCESS != status)
        PK_ENTITY_delete((int) replaceSurfs.size(), &replaceSurfs.front());

    return status;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::TransformVertices(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& vertices, bvector<Transform> const& transforms, StepFacesOption addStep)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    BentleyStatus status = transformVertices(targetEntity, vertices, transforms);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::SweepFaces(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& faces, DVec3dCR path)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (faces.empty())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    bvector<PK_FACE_t> faceTags;

    for (size_t iFace = 0; iFace < faces.size(); ++iFace)
        {
        if (ISubEntity::SubEntityType::Face != faces[iFace]->GetSubEntityType())
            continue;

        faceTags.push_back(PSolidSubEntity::GetSubEntityTag(*faces[iFace]));
        }

    int              nLaterals;
    PK_local_check_t checkResult;
    PK_VECTOR_t      pathVec;
    Transform        invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    invTargetTransform.MultiplyMatrixOnly((DVec3dR) pathVec, path);

    BentleyStatus   status = (SUCCESS == PK_FACE_sweep((int) faceTags.size(), &faceTags.front(), pathVec, PK_LOGICAL_true, &nLaterals, NULL, NULL, &checkResult) && PK_local_check_ok_c == checkResult) ? SUCCESS : ERROR;

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::SpinFaces(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& faces, DRay3dCR axis, double angle)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (faces.empty())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    bvector<PK_FACE_t> faceTags;

    for (size_t iFace = 0; iFace < faces.size(); ++iFace)
        {
        if (ISubEntity::SubEntityType::Face != faces[iFace]->GetSubEntityType())
            continue;

        faceTags.push_back(PSolidSubEntity::GetSubEntityTag(*faces[iFace]));
        }

    int              nLaterals;
    PK_local_check_t checkResult;
    PK_AXIS1_sf_t    axisSf;
    Transform        invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    invTargetTransform.Multiply((DPoint3dP) &axisSf.location.coord[0], &axis.origin, 1);
    invTargetTransform.MultiplyMatrixOnly(*((DVec3dP) &axisSf.axis.coord[0]), axis.direction);
    ((DVec3dR) (axisSf.axis)).Normalize();

    BentleyStatus   status = (SUCCESS == PK_FACE_spin((int) faceTags.size(), &faceTags.front(), &axisSf, angle, PK_LOGICAL_true, &nLaterals, NULL, NULL, &checkResult) && PK_local_check_ok_c == checkResult) ? SUCCESS : ERROR;

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::TaperFaces(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& faces, bvector<ISubEntityPtr>& refEntities, DVec3dCR direction, bvector<double>& angles, StepFacesOption addStep)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (faces.empty() || refEntities.empty() || angles.empty())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    bvector<PK_FACE_t> faceTags;

    for (ISubEntityPtr facePtr : faces)
        {
        if (ISubEntity::SubEntityType::Face != facePtr->GetSubEntityType())
            continue;

        faceTags.push_back(PSolidSubEntity::GetSubEntityTag(*facePtr));
        }

    bvector<PK_ENTITY_t> refEntityTags;

    for (ISubEntityPtr refEntityPtr : refEntities)
        {
        switch (refEntityPtr->GetSubEntityType())
            {
            case ISubEntity::SubEntityType::Edge:
                {
                refEntityTags.push_back(PSolidSubEntity::GetSubEntityTag(*refEntityPtr));
                break;
                }

            case ISubEntity::SubEntityType::Face:
                {
                PK_SURF_t surfTag = PK_ENTITY_null;

                if (SUCCESS == PK_FACE_ask_surf(PSolidSubEntity::GetSubEntityTag(*refEntityPtr), &surfTag) && PK_ENTITY_null != surfTag)
                    refEntityTags.push_back(surfTag);
                break;
                }
            }
        }

    if (faceTags.size() != refEntityTags.size())
        return ERROR;

    if (angles.size() > 1 && angles.size() != faceTags.size())
        return ERROR;

    PK_VECTOR1_t taperVec;
    Transform    invTargetTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    invTargetTransform.MultiplyMatrixOnly((DVec3dR) taperVec, direction);
    ((DVec3dR) (taperVec)).Normalize();

    PK_FACE_taper_o_t options;

    PK_FACE_taper_o_m(options);

    options.grow = PK_FACE_grow_auto_c;
    options.method = (0.0 != direction.Magnitude() ? PK_taper_method_curve_c : PK_taper_method_normal_c);

    switch (addStep)
        {
        case StepFacesOption::AddNone:
            break;

        case StepFacesOption::AddAll:
        case StepFacesOption::AddSmooth:
            options.taper_step_face = PK_taper_preserve_smooth_c;
            options.taper_smooth_step = PK_taper_smooth_step_yes_c;
            break;

        case StepFacesOption::AddNonCoincident:
            options.taper_step_face = PK_taper_step_face_yes_c;
            break;
        }

    double taperAngle = angles.front();
    bvector<double> taperFaceAngles;
    bvector<PK_FACE_t> taperFaceTags;

    if (angles.size() > 1)
        {
        for (size_t iAngle = 0; iAngle < angles.size(); ++iAngle)
            {
            if (Angle::NearlyEqual(angles[iAngle], taperAngle))
                continue;

            taperFaceTags.push_back(faceTags[iAngle]);
            taperFaceAngles.push_back(angles[iAngle]);
            }

        if (taperFaceAngles.size() > 1)
            {
            options.n_faces = (int) taperFaceAngles.size();
            options.taper_faces = &taperFaceTags.front();
            options.angles = &taperFaceAngles.front();
            }
        }

    PK_TOPOL_track_r_t tracking;
    PK_TOPOL_local_r_t results;

    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    PK_ERROR_code_t failureCode = PK_FACE_taper((int) faceTags.size(), &faceTags.front(), &refEntityTags.front(), taperVec, taperAngle, 1.0e-5, &options, &tracking, &results);
    BentleyStatus   status = (PK_ERROR_no_errors == failureCode && PK_local_status_ok_c == results.status) ? SUCCESS : ERROR;

    PK_TOPOL_local_r_f(&results);
    PK_TOPOL_track_r_f(&tracking);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::DeleteFaces(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& faces, bool createCap)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (faces.empty())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    bvector<PK_FACE_t> faceTags;

    for (size_t iFace = 0; iFace < faces.size(); ++iFace)
        {
        if (ISubEntity::SubEntityType::Face != faces[iFace]->GetSubEntityType())
            continue;

        faceTags.push_back(PSolidSubEntity::GetSubEntityTag(*faces[iFace]));
        }

    PK_TOPOL_track_r_t tracking;
    PK_FACE_delete_o_t options;

    memset(&tracking, 0, sizeof(tracking));
    PK_FACE_delete_o_m(options);
    options.allow_disjoint = true;
    options.repair_fa_fa = PK_repair_fa_fa_yes_c;

    if (createCap)
        {
        options.heal_action = PK_FACE_heal_cap_c;
        options.heal_loops = PK_FACE_heal_loops_together_c;
        }

    PK_ERROR_code_t failureCode = PK_FACE_delete_2((int) faceTags.size(), &faceTags.front(), &options, &tracking);

    PK_TOPOL_track_r_f(&tracking);

    if (PK_ERROR_no_errors != failureCode)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return (SUCCESS == failureCode ? SUCCESS : ERROR);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::DeleteEdges(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& edges)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (edges.empty())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    bvector<PK_EDGE_t> edgeTags;

    for (size_t iEdge = 0; iEdge < edges.size(); ++iEdge)
        {
        if (ISubEntity::SubEntityType::Edge != edges[iEdge]->GetSubEntityType())
            continue;

        edgeTags.push_back(PSolidSubEntity::GetSubEntityTag(*edges[iEdge]));
        }

    PK_TOPOL_track_r_t tracking;
    PK_TOPOL_local_r_t results;
    PK_EDGE_delete_o_t options;

    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));
    PK_EDGE_delete_o_m(options);

    PK_ERROR_code_t failureCode = PK_EDGE_delete((int) edgeTags.size(), &edgeTags.front(), &options, &tracking, &results);

    PK_TOPOL_track_r_f(&tracking);
    PK_TOPOL_local_r_f(&results);

    if (PK_ERROR_no_errors != failureCode)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return (SUCCESS == failureCode ? SUCCESS : ERROR);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::ImprintCurveVectorOnFace(ISubEntityPtr& face, CurveVectorCR curveVector, DVec3dCP direction, bool extend)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (!face.IsValid() || ISubEntity::SubEntityType::Face != face->GetSubEntityType())
        return ERROR;

    Transform   curveToBodyTrans;
    PK_ENTITY_t toolTag = PK_ENTITY_null;

    curveToBodyTrans.InverseOf(PSolidSubEntity::GetSubEntityTransform(*face));

    if (SUCCESS != PSolidGeom::BodyFromCurveVector(toolTag, nullptr, curveVector, curveToBodyTrans, false, nullptr))
        return ERROR;

    DVec3d  projVec = DVec3d::From(0.0, 0.0, 1.0);

    if (direction)
        {
        curveToBodyTrans.MultiplyMatrixOnly(projVec, *direction);
        projVec.Normalize();
        }

    bvector<PK_CURVE_t>     toolCurves;
    bvector<PK_INTERVAL_t>  toolIntervals;

    getBodyCurves(toolCurves, toolIntervals, toolTag);

    BentleyStatus status = PSolidUtil::ImprintCurves(PSolidSubEntity::GetSubEntityTag(*face), toolCurves, toolIntervals, direction ? &projVec : nullptr, extend, true);

    PK_ENTITY_delete(1, &toolTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::ImprintCurveVectorOnBody(IBRepEntityR target, CurveVectorCR curveVector, DVec3dCP direction, bool extend)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (IBRepEntity::EntityType::Wire == target.GetEntityType())
        return ERROR;

    Transform   curveToBodyTrans;
    PK_ENTITY_t toolTag = PK_ENTITY_null;

    curveToBodyTrans.InverseOf(target.GetEntityTransform());

    if (SUCCESS != PSolidGeom::BodyFromCurveVector(toolTag, nullptr, curveVector, curveToBodyTrans, false, nullptr))
        return ERROR;

    DVec3d  projVec = DVec3d::From(0.0, 0.0, 1.0);

    if (direction)
        {
        curveToBodyTrans.MultiplyMatrixOnly(projVec, *direction);
        projVec.Normalize();
        }

    bvector<PK_CURVE_t>     toolCurves;
    bvector<PK_INTERVAL_t>  toolIntervals;

    getBodyCurves(toolCurves, toolIntervals, toolTag);

    BentleyStatus status = PSolidUtil::ImprintCurves(PSolidUtil::GetEntityTagForModify(target), toolCurves, toolIntervals, direction ? &projVec : nullptr, extend, true);

    PK_ENTITY_delete(1, &toolTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::ImprintBodyOnBody(IBRepEntityR targetEntity, IBRepEntityCR toolEntity, bool extend)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (IBRepEntity::EntityType::Wire == targetEntity.GetEntityType() || IBRepEntity::EntityType::Wire == toolEntity.GetEntityType())
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);
    PK_ENTITY_t toolEntityTag = PSolidUtil::GetEntityTag(toolEntity);

    if (PK_ENTITY_null == targetEntityTag || PK_ENTITY_null == toolEntityTag)
        return ERROR;

    Transform   invTargetTransform, toolTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    toolTransform.InitProduct(invTargetTransform, toolEntity.GetEntityTransform());

    PK_ENTITY_copy(toolEntityTag, &toolEntityTag);
    PSolidUtil::TransformBody(toolEntityTag, toolTransform);

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    PK_BODY_imprint_o_t options;
    PK_imprint_r_t results;

    PK_BODY_imprint_o_m(options);
    options.imprint_tool = false;
    options.imprint_complete_targ = ((extend && IBRepEntity::EntityType::Sheet == toolEntity.GetEntityType()) ? PK_imprint_complete_edge_c : PK_imprint_complete_no_c);
    memset(&results, 0, sizeof(results));

    BentleyStatus   status = (SUCCESS == PK_BODY_imprint_body(targetEntityTag, toolEntityTag, &options, &results) ? SUCCESS : ERROR);

    PK_imprint_r_f(&results);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    PK_ENTITY_delete(1, &toolEntityTag);

    return status;
#else
    return ERROR;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus wireBodyFromOffsetEdgesOnPlanarFace(bvector<PK_BODY_t>& wireBodies, ISubEntityCR face, bvector<ISubEntityPtr>& edges, double distance)
    {
    if (0.0 == distance)
        return ERROR;

    DPoint3d    facePoint;
    DVec3d      faceNormal;
    PK_FACE_t   faceTag = PSolidSubEntity::GetSubEntityTag(face);

    if (SUCCESS != PSolidUtil::GetPlanarFaceData(&facePoint, &faceNormal, faceTag))
        return ERROR;

    bvector<PK_CURVE_t> curves;
    bvector<PK_INTERVAL_t> intervals;

    for (ISubEntityPtr subEntityPtr : edges)
        {
        if (ISubEntity::SubEntityType::Edge != subEntityPtr->GetSubEntityType())
            continue;

        PK_EDGE_t edgeTag = PSolidSubEntity::GetSubEntityTag(*subEntityPtr);

        if (0 == edgeTag)
            continue;

        PK_CURVE_t      curveTag = PK_ENTITY_null;
        PK_INTERVAL_t   interval;

        if (SUCCESS == PK_EDGE_ask_curve(edgeTag, &curveTag) && PK_ENTITY_null != curveTag)
            {
            if (SUCCESS != PK_EDGE_find_interval(edgeTag, &interval) && SUCCESS != PK_CURVE_ask_interval(curveTag, &interval))
                curveTag = PK_ENTITY_null;
            }
        else
            {
            bvector<PK_FIN_t> edgeFins;

            PSolidTopo::GetEdgeFins(edgeFins, edgeTag);

            for (PK_FIN_t edgeFin : edgeFins)
                {
                PK_FACE_t finFace;

                if (SUCCESS != PK_FIN_ask_face(edgeFin, &finFace) || finFace != faceTag)
                    continue;

                if (SUCCESS != PK_FIN_ask_curve(edgeFin, &curveTag) || SUCCESS != PK_FIN_find_interval(edgeFin, &interval))
                    curveTag = PK_ENTITY_null;
                break;
                }
            }

        if (PK_ENTITY_null == curveTag)
            continue;

        if (curves.empty()) // Determine sign of offset distance...
            {
            bvector<PK_FIN_t> edgeFins;

            PSolidTopo::GetEdgeFins(edgeFins, edgeTag);
            distance = fabs(distance);

            for (PK_FIN_t edgeFin : edgeFins)
                {
                PK_FACE_t finFace;

                if (SUCCESS != PK_FIN_ask_face(edgeFin, &finFace) || finFace != faceTag)
                    continue;

                PK_LOGICAL_t finPositive, curvePositive;
                PK_CURVE_t   orientedCurve;

                PK_FIN_is_positive(edgeFin, &finPositive);

                if (SUCCESS == PK_FIN_ask_oriented_curve(edgeFin, &orientedCurve, &curvePositive) && finPositive == curvePositive)
                    distance = -distance;
                break;
                }
            }

        curves.push_back(curveTag);
        intervals.push_back(interval);
        }

    if (curves.empty())
        return ERROR;

    PK_BODY_t wireBody = PK_ENTITY_null;
    int nNewEdges = 0;
    int* edgeIndices = nullptr;
    PK_EDGE_t* newEdges = nullptr;
    PK_CURVE_make_wire_body_o_t wireOptions;

    PK_CURVE_make_wire_body_o_m(wireOptions);
    wireOptions.want_indices = PK_LOGICAL_true;
    wireOptions.want_edges = PK_LOGICAL_true;

    if (SUCCESS != PK_CURVE_make_wire_body_2((int) curves.size(), &curves.front(), &intervals.front(), &wireOptions, &wireBody, &nNewEdges, &newEdges, &edgeIndices))
        return ERROR;

    PK_EDGE_t refEdge = newEdges[edgeIndices[0]];

    PK_MEMORY_free(edgeIndices);
    PK_MEMORY_free(newEdges);

    int nNewWires = 0;
    PK_BODY_t* newWires = nullptr;
    PK_BODY_offset_planar_wire_o_t offsetOptions;
    PK_TOPOL_track_r_t tracking;
    PK_VECTOR_t vector;

    PK_BODY_offset_planar_wire_o_m(offsetOptions);
    offsetOptions.gap_fill = PK_BODY_owb_gap_fill_natural_c;
    memset(&tracking, 0, sizeof(tracking));

    faceNormal.GetComponents(vector.coord[0], vector.coord[1], vector.coord[2]);

    Transform invTargetTransform, fwdTargetTransform = PSolidSubEntity::GetSubEntityTransform(face);
    invTargetTransform.InverseOf(fwdTargetTransform);
    invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&distance, 1);

    if (SUCCESS != PK_BODY_offset_planar_wire(wireBody, distance, vector, refEdge, &offsetOptions, &nNewWires, &newWires, &tracking))
        {
        PK_ENTITY_delete(1, &wireBody);
        return ERROR;
        }

    for (int i=0; i<nNewWires; i++)
        wireBodies.push_back(newWires[i]);

    PK_MEMORY_free(newWires);
    PK_TOPOL_track_r_f(&tracking);
    PK_ENTITY_delete(1, &wireBody);

    return (wireBodies.empty() ? ERROR : SUCCESS);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::ImprintOffsetEdgesOnPlanarFace(ISubEntityPtr& face, bvector<ISubEntityPtr>& edges, double distance, bool extend)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    bvector<PK_BODY_t> wireBodies;

    if (SUCCESS != wireBodyFromOffsetEdgesOnPlanarFace(wireBodies, *face, edges, distance))
        return ERROR;

    bvector<PK_CURVE_t>     offsetToolCurves;
    bvector<PK_INTERVAL_t>  offsetToolIntervals;

    for (PK_BODY_t wireBody : wireBodies)
        getBodyCurves(offsetToolCurves, offsetToolIntervals, wireBody);

    BentleyStatus status;
    PK_MARK_t markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    if (SUCCESS != (status = PSolidUtil::ImprintCurves(PSolidSubEntity::GetSubEntityTag(*face), offsetToolCurves, offsetToolIntervals, nullptr, extend, true)))
        {
        PK_MARK_goto(markTag);
        PK_ENTITY_delete((int) wireBodies.size(), &wireBodies.front());
        }

    PK_MARK_delete(markTag);

    return status;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr BRepUtil::Create::OffsetEdgesOnPlanarFaceToCurveVector(ISubEntityCR face, bvector<ISubEntityPtr>& edges, double distance)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    bvector<PK_BODY_t> wireBodies;

    if (SUCCESS != wireBodyFromOffsetEdgesOnPlanarFace(wireBodies, face, edges, distance))
        return nullptr;

    CurveVectorPtr offsetCurves;
    Transform entityTransform = PSolidSubEntity::GetSubEntityTransform(face);

    for (PK_BODY_t wireBodyTag : wireBodies)
        {
        IBRepEntityPtr wireEntity = PSolidUtil::CreateNewEntity(wireBodyTag, entityTransform, true); // <- Will free body...

        if (!wireEntity.IsValid())
            continue;

        CurveVectorPtr wireCurve = BRepUtil::Create::BodyToCurveVector(*wireEntity);

        if (!offsetCurves.IsValid())
            {
            offsetCurves = wireCurve;
            continue;
            }

        if (CurveVector::BOUNDARY_TYPE_None != offsetCurves->GetBoundaryType())
            {
            CurveVectorPtr tmpCurves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

            tmpCurves->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*offsetCurves));
            offsetCurves = tmpCurves;
            }

        offsetCurves->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*wireCurve));
        }

    return offsetCurves;
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (BENTLEYCONFIG_PARASOLID)
static void FaceAttachmentFromJson(FaceAttachment& attachment, JsonValueCR brep, uint32_t iSymb)
    {
    if (!brep["faceSymbology"][iSymb]["color"].isNull())
        {
        uint32_t color = brep["faceSymbology"][iSymb]["color"].asUInt();
        double transparency = !brep["faceSymbology"][iSymb]["transparency"].isNull() ? brep["faceSymbology"][iSymb]["transparency"].asDouble() : 0.0;
        attachment.SetColor(color, transparency);
        }

    if (!brep["faceSymbology"][iSymb]["material"].isNull())
        {
        BeInt64Id materialId(brep["faceSymbology"][iSymb]["material"].asUInt64());
        attachment.SetMaterial(materialId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value FaceAttachmentToJson(FaceAttachment const& attachment)
    {
    Json::Value faceValue; // Shouldn't need to worry about null...either useColor or useMaterial must be set...
    bool useColor = attachment.GetUseColor();
    bool useMaterial = attachment.GetUseMaterial();

    if (useColor)
        {
        faceValue["color"] = attachment.GetColor();
        if (0.0 != attachment.GetTransparency())
            faceValue["transparency"] = attachment.GetTransparency();
        }

    if (useMaterial)
        {
        faceValue["materialId"] = attachment.GetMaterial().ToHexStr();
        }

    return faceValue;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr BRepUtil::Create::BodyFromJson(JsonValueCR brep)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (brep["data"].isNull())
        return nullptr;

    ByteStream byteStream;

    Base64Utilities::Decode(byteStream, brep["data"].asString());

    if (!byteStream.HasData())
        return nullptr;

    Transform entityTransform = Transform::FromIdentity();

    if (!brep["transform"].isNull())
        JsonUtils::TransformFromJson(entityTransform, brep["transform"]);

    IBRepEntityPtr entity;

    if (SUCCESS != PSolidUtil::RestoreEntityFromMemory(entity, byteStream.GetData(), byteStream.GetSize(), entityTransform))
        return nullptr;

    if (!brep["faceSymbology"].isNull() && brep["faceSymbology"].isArray())
        {
        uint32_t nSymb = (uint32_t) brep["faceSymbology"].size();

        for (uint32_t iSymb=0; iSymb < nSymb; iSymb++)
            {
            FaceAttachment attachment;

            FaceAttachmentFromJson(attachment, brep, iSymb);

            if (nullptr == entity->GetFaceMaterialAttachments())
                {
                IFaceMaterialAttachmentsPtr attachments = PSolidUtil::CreateNewFaceAttachments(PSolidUtil::GetEntityTag(*entity), attachment);

                if (!attachments.IsValid())
                    break;

                PSolidUtil::SetFaceAttachments(*entity, attachments.get());
                }
            else
                {
                entity->GetFaceMaterialAttachmentsP()->_GetFaceAttachmentsVecR().push_back(attachment);
                }
            }
        }

    return entity;
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value BRepUtil::Create::BodyToJson(IBRepEntityCR entity)
    {
    Json::Value value(Json::objectValue);

#if defined (BENTLEYCONFIG_PARASOLID)
    size_t      bufferSize = 0;
    uint8_t*    buffer = nullptr;

    if (SUCCESS != PSolidUtil::SaveEntityToMemory(&buffer, bufferSize, entity))
        return value;

    Utf8String dataStr = Base64Utilities::Encode((Utf8CP) buffer, bufferSize);
    value["data"] = dataStr;
    value["type"] = (uint32_t) entity.GetEntityType(); // Can't be IBRepEntity::EntityType::Invalid if SaveEntityToMemory returned SUCCESS...

    Transform entityTransform = entity.GetEntityTransform();
    if (!entityTransform.IsIdentity())
        JsonUtils::TransformToJson(value["transform"], entityTransform);

    IFaceMaterialAttachmentsCP attachments = entity.GetFaceMaterialAttachments();
    if (nullptr != attachments)
        {
        T_FaceAttachmentsVec const& faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();

        for (FaceAttachment attachment : faceAttachmentsVec)
            {
            Json::Value faceValue = FaceAttachmentToJson(attachment);
            value["faceSymbology"].append(faceValue);
            }
        }
#endif

    return value;
    }
