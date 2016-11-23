/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/SolidKernelUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment()
    {
    m_useColor = m_useMaterial = false;
    m_color = ColorDef::Black();
    m_transparency = 0.0;
    m_uv.Init(0.0, 0.0);
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
        m_material      != rhs.m_material ||
        m_uv.x          != rhs.m_uv.x || 
        m_uv.y          != rhs.m_uv.y)
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
            m_color.GetValue() < rhs.m_color.GetValue() ||
            m_transparency     < rhs.m_transparency ||
            m_material         < rhs.m_material ||
            m_uv.x             < rhs.m_uv.x || 
            m_uv.y             < rhs.m_uv.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment(GeometryParamsCR sourceParams)
    {
    if (m_useColor = !sourceParams.IsLineColorFromSubCategoryAppearance())
        {
        m_color = sourceParams.GetLineColor();
        m_transparency = sourceParams.GetTransparency();
        }

    if (m_useMaterial = !sourceParams.IsMaterialFromSubCategoryAppearance())
        {
        m_material = sourceParams.GetMaterialId();
        // NEEDSWORK_WIP_MATERIAL...m_uv???
        }

    m_uv.Init(0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void FaceAttachment::ToGeometryParams(GeometryParamsR faceParams, GeometryParamsCR baseParams) const
    {
    faceParams = baseParams;

    if (m_useColor)
        {
        faceParams.SetLineColor(m_color);
        faceParams.SetTransparency(m_transparency);
        }

    if (m_useMaterial)
        {
        faceParams.SetMaterialId(m_material);
        // NEEDSWORK_WIP_MATERIAL...m_uv???
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void FaceAttachment::CookFaceAttachment(ViewContextR context, GeometryParamsCR baseParams) const
    {
    GeometryParams faceParams;

    ToGeometryParams(faceParams, baseParams);
    context.CookGeometryParams(faceParams, m_graphicParams);

    m_haveGraphicParams = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t FaceAttachment::GetFaceIdentifierFromSubEntity(ISubEntityCR subEntity)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    PK_ENTITY_t entityTag = PSolidSubEntity::GetSubEntityTag(subEntity);

    if (PK_ENTITY_null == entityTag)
        return 0;

    PK_CLASS_t  entityClass;

    PK_ENTITY_ask_class(entityTag, &entityClass);

    switch (entityClass)
        {
        case PK_CLASS_face:
            return entityTag;

        case PK_CLASS_edge:
            return PSolidUtil::GetPreferredFaceAttachmentFaceForEdge(entityTag);

        case PK_CLASS_vertex:
            return PSolidUtil::GetPreferredFaceAttachmentFaceForVertex(entityTag);
        }
#endif

    return 0;
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
BentleyStatus BRepUtil::ClipCurveVector(bvector<CurveVectorPtr>& output, CurveVectorCR input, ClipVectorCR clipVector, TransformCP transform)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidUtil::ClipCurveVector(output, input, clipVector, transform);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::ClipBody(bvector<IBRepEntityPtr>& output, bool& clipped, IBRepEntityCR input, ClipVectorCR clipVector)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidUtil::ClipBody(output, clipped, input, clipVector);
#else
    return ERROR;
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

    return SUCCESS;
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

    return SUCCESS;
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

    return SUCCESS;
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

    return SUCCESS;
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

    return SUCCESS;
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

    return SUCCESS;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::TopologyID::AddNodeIdAttributes(IBRepEntityR entity, uint32_t nodeId, bool overrideExisting)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t bodyTag = PSolidUtil::GetEntityTagForModify(entity);

    if (PK_ENTITY_null == bodyTag)
        return ERROR;

    return PSolidTopoId::AddNodeIdAttributes(bodyTag, nodeId, overrideExisting);
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

        if (SUCCESS != PSolidUtil::DoBoolean(*planeEntity, &entityOut, 1, PK_boolean_subtract, PKI_BOOLEAN_OPTION_AllowDisjoint))
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
BentleyStatus BRepUtil::Create::BodyFromLoft(IBRepEntityPtr& entityOut, bvector<CurveVectorPtr>& profiles, bvector<CurveVectorPtr>* guides, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidGeom::BodyFromLoft(entityOut, &profiles.front(), profiles.size(), guides ? &guides->front() : nullptr, guides ? guides->size() : 0, nodeId);
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
BentleyStatus BRepUtil::Modify::BooleanOperation(IBRepEntityR targetEntity, IBRepEntityR toolEntity, BooleanMode op)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    IBRepEntityPtr tmpToolEntityPtr = &toolEntity;

    return PSolidUtil::DoBoolean(targetEntity, &tmpToolEntityPtr, 1, getBooleanFunction(op), PKI_BOOLEAN_OPTION_AllowDisjoint);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::BooleanOperation(IBRepEntityR targetEntity, bvector<IBRepEntityPtr>& toolEntities, BooleanMode op)
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
BentleyStatus BRepUtil::Modify::BooleanCut(IBRepEntityR target, IBRepEntityCR planarTool, CutDirectionMode directionMode, CutDepthMode depthMode, double depth, bool inside)
    {
    // NOTE: The MicroStation Connect version of this method specifies the profile as a CurveVector instead of a sheet body.
    //       There are some issues with that implementation worth mentioning in case anyone believes it's a good idea to bring that code over.
    //       1) AddNodeIdAttributes is being called instead of AssignProfileBodyIds; that results in less robust face ids.
    //       2) Open profiles are only extended not closed, so depth options other than "through all" aren't supported.
    //       3) The face ids of swept open profiles change depending on whether the start point is currently in or out of the target range.
    //       4) Does not resolve node Id conflicts between the target and tool body, this is likely just a bug and not intended.
    //       I prefer keeping this method simple and instead providing helper methods for creating the tool body.
    //       See BRepUtil::Create::CutProfileBodyFromOpenCurveVector and BRepUtil::Create::SweptBodyFromOpenCurveVector.
#if defined (BENTLEYCONFIG_PARASOLID)
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
        DRange3d targetRange = target.GetEntityRange();

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
            PSolidUtil::ExtractEntityTag(*toolEntity);
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
BentleyStatus BRepUtil::Modify::Emboss(IBRepEntityR targetEntity, IBRepEntityCR toolEntity, bool reverseDirection)
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

    DRange3d targetRange;

    if (SUCCESS != PSolidUtil::GetEntityRange(targetRange, targetTag))
        return ERROR;
 
    DRay3d      toolRay;
    Transform   invTargetTransform, toolTransform;

    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());
    toolTransform.InitProduct(invTargetTransform, toolEntity.GetEntityTransform());

    bvector<PK_FACE_t> toolFaces;

    if (SUCCESS != PSolidTopo::GetBodyFaces(toolFaces, toolTag) ||
        SUCCESS != PSolidUtil::GetPlanarFaceData(&toolRay.origin, &toolRay.direction, toolFaces.front()))
        return ERROR;

    toolTransform.Multiply(toolRay.origin);
    toolTransform.MultiplyMatrixOnly(toolRay.direction);
    toolRay.direction.Normalize();

    if (reverseDirection)
        toolRay.direction.Negate();
            
    DRange1d  targetDepthRange = targetRange.GetCornerRange(toolRay);
    double    toolDepth = toolRay.direction.DotProduct(toolRay.origin);

    PK_BODY_emboss_o_t  options;
    PK_TOPOL_track_r_t  tracking;
    PK_TOPOL_local_r_t  results;

    PK_BODY_emboss_o_m(options);
    memset(&tracking, 0, sizeof(tracking));
    memset(&results, 0, sizeof(results));

    options.sidewall_data.sidewall = PK_emboss_sidewall_swept_c;
    options.convexity = PK_emboss_convexity_both_c; // Let pad or pocket be determined by whether cap is "above" or "below" target body according to emboss direction...
    options.overflow_data.laminar_walled = true;
    toolRay.direction.GetComponents(options.sidewall_data.draw_direction.coord[0], options.sidewall_data.draw_direction.coord[1], options.sidewall_data.draw_direction.coord[2]);

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
BentleyStatus BRepUtil::Modify::BlendEdges(IBRepEntityR targetEntity, bvector<ISubEntityPtr>& edges, bvector<double>& radii, bool propagate)
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

        if (ISubEntity::SubEntityType::Edge != edge.GetSubEntityType ())
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

