/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/SolidKernelUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#if defined (BENTLEYCONFIG_OPENCASCADE) 
#include <DgnPlatform/DgnBRep/OCBRep.h>
#elif defined (BENTLEYCONFIG_PARASOLID) 
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

#if defined (BENTLEYCONFIG_OPENCASCADE)    
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  03/16
+===============+===============+===============+===============+===============+======*/
struct OpenCascadeEntity : RefCounted<IBRepEntity>
{
private:

TopoDS_Shape m_shape;

protected:

virtual Transform _GetEntityTransform () const override {Transform transform = OCBRep::ToTransform(m_shape.Location()); return transform;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _SetEntityTransform (TransformCR transform) override
    {
    DPoint3d    origin;
    RotMatrix   rMatrix, rotation, skewFactor;
    Transform   shapeTrans, goopTrans; 

    transform.GetTranslation(origin);
    transform.GetMatrix(rMatrix);

    // NOTE: Don't allow scaled TopoDS_Shape::Location...too many bugs, also non-uniform scale isn't supported...
    if (rMatrix.RotateAndSkewFactors(rotation, skewFactor, 0, 1))
        {
        goopTrans.InitFrom(skewFactor);
        shapeTrans.InitFrom(rotation, origin);
        }
    else
        {
        goopTrans = transform;
        shapeTrans.InitIdentity();
        }

    try
        {
        if (!goopTrans.IsIdentity())
            {
            double  goopScale;

            goopTrans.GetMatrix(rMatrix);

            if (rMatrix.IsUniformScale(goopScale))
                {
                gp_Trsf goopTrsf = OCBRep::ToGpTrsf(goopTrans);

                m_shape.Location(TopLoc_Location()); // NOTE: Need to ignore shape location...
                BRepBuilderAPI_Transform transformer(m_shape, goopTrsf);
    
                if (!transformer.IsDone())
                    {
                    BeAssert(false);
                    return false;
                    }

                m_shape = transformer.ModifiedShape(m_shape);
                }
            else
                {
                gp_GTrsf goopTrsf = OCBRep::ToGpGTrsf(goopTrans);

                m_shape.Location(TopLoc_Location()); // NOTE: Need to ignore shape location...
                BRepBuilderAPI_GTransform transformer(m_shape, goopTrsf);
    
                if (!transformer.IsDone())
                    {
                    BeAssert(false);
                    return false;
                    }

                m_shape = transformer.ModifiedShape(m_shape);
                }

            BeAssert(m_shape.Location().IsIdentity());
            }
        }
    catch (Standard_Failure)
        {
        return false;
        }

    m_shape.Location(OCBRep::ToGpTrsf(shapeTrans));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
EntityType ToEntityType(TopAbs_ShapeEnum shapeType) const
    {
    switch (shapeType)
        {
        case TopAbs_COMPOUND:
            return IBRepEntity::EntityType::Compound;

        case TopAbs_COMPSOLID:
        case TopAbs_SOLID:
            return IBRepEntity::EntityType::Solid;

        case TopAbs_SHELL:
        case TopAbs_FACE:
            return IBRepEntity::EntityType::Sheet;

        case TopAbs_WIRE:
        case TopAbs_EDGE:
            return IBRepEntity::EntityType::Wire;

        case TopAbs_VERTEX:
        default:
            return IBRepEntity::EntityType::Minimal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
EntityType _GetEntityType() const {return ToEntityType(OCBRepUtil::GetShapeType(m_shape));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetEntityRange() const
    {
    Bnd_Box box;

    try
        {
        BRepBndLib::AddOptimal(m_shape, box, false); // Never use triangulation...
        }
    catch (Standard_Failure)
        {
        BRepBndLib::Add(m_shape, box); // Sloppy AABB implementation is apparently more robust...we NEED a valid range!
        }

    return OCBRep::ToDRange3d(box);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (IBRepEntityCR entity) const override
    {
    if (this == &entity)
        return true;

    OpenCascadeEntity const* ocEntity;

    if (NULL == (ocEntity = dynamic_cast <OpenCascadeEntity const*>(&entity)))
        return false;

    return TO_BOOL(m_shape == ocEntity->GetShape());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const override {return nullptr;}
virtual bool _InitFaceMaterialAttachments (Render::GeometryParamsCP baseParams) override {return false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IBRepEntityPtr _Clone() const override
    {
    TopoDS_Shape clone(m_shape);

    return OpenCascadeEntity::CreateNewEntity(clone);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
OpenCascadeEntity(TopoDS_Shape const& shape) : m_shape(shape) {}

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TopoDS_Shape const& GetShape() const {return m_shape;}
TopoDS_Shape& GetShapeR() {return m_shape;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static OpenCascadeEntity* CreateNewEntity(TopoDS_Shape const& shape)
    {
    if (OCBRepUtil::IsEmptyCompoundShape(shape))
        return nullptr; // Don't create OpenCascadeEntity from empty compound (ex. useless result from BRepAlgoAPI_Cut if target is completely inside tool)...

    return new OpenCascadeEntity(shape);
    }

}; // OpenCascadeEntity

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TopoDS_Shape const* SolidKernelUtil::GetShape(IBRepEntityCR entity)
    {
    OpenCascadeEntity const* ocEntity = dynamic_cast <OpenCascadeEntity const*> (&entity);

    if (!ocEntity)
        return nullptr;

    return &ocEntity->GetShape();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TopoDS_Shape* SolidKernelUtil::GetShapeP(IBRepEntityR entity)
    {
    OpenCascadeEntity* ocEntity = dynamic_cast <OpenCascadeEntity*> (&entity);

    if (!ocEntity)
        return nullptr;

    return &ocEntity->GetShapeR();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr SolidKernelUtil::CreateNewEntity(TopoDS_Shape const& shape)
    {
    return OpenCascadeEntity::CreateNewEntity(shape);
    }

#if defined (NOT_NOW_FACET) 
    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(entity);

    if (nullptr == shape)
        return nullptr;

    if (nullptr != pixelSizeRange)
        pixelSizeRange->InitNull();

    IFacetOptionsPtr facetOptions = IFacetOptions::Create();

    facetOptions->SetNormalsRequired(true);
    facetOptions->SetParamsRequired(true);

    if (!OCBRep::HasCurvedFaceOrEdge(*shape))
        {
        facetOptions->SetAngleTolerance(Angle::PiOver2()); // Shouldn't matter...use max angle tolerance...
        facetOptions->SetChordTolerance(1.0); // Shouldn't matter...avoid expense of getting AABB...
        }
    else
        {
        Bnd_Box box;
        Standard_Real maxDimension = 0.0;

        BRepBndLib::Add(*shape, box);
        BRepMesh_ShapeTool::BoxMaxDimension(box, maxDimension);

        if (0.0 >= pixelSize)
            {
            facetOptions->SetAngleTolerance(0.2); // ~11 degrees
            facetOptions->SetChordTolerance(0.1 * maxDimension);
            }
        else
            {
            static double sizeDependentRatio = 5.0;
            static double pixelToChordRatio = 0.5;
            static double minRangeRelTol = 1.0e-4;
            static double maxRangeRelTol = 1.5e-2;
            double minChordTol = minRangeRelTol * maxDimension;
            double maxChordTol = maxRangeRelTol * maxDimension;
            double chordTol = pixelToChordRatio * pixelSize;
            bool isMin = false, isMax = false;

            if (isMin = (chordTol < minChordTol))
                chordTol = minChordTol; // Don't allow chord to get too small relative to shape size...
            else if (isMax = (chordTol > maxChordTol))
                chordTol = maxChordTol; // Don't keep creating coarser and coarser graphics as you zoom out, at a certain point it just wastes memory/time...

            facetOptions->SetChordTolerance(chordTol);
            facetOptions->SetAngleTolerance(Angle::PiOver2()); // Use max angle tolerance...mesh coarseness dictated by pixel size based chord...

            if (nullptr != pixelSizeRange)
                {
                if (isMin)
                    *pixelSizeRange = DRange1d::FromLowHigh(0.0, chordTol * sizeDependentRatio); // Finest tessellation, keep using this as we zoom in...
                else if (isMax)
                    *pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, DBL_MAX); // Coarsest tessellation, keep using this as we zoom out...
                else
                    *pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, chordTol * sizeDependentRatio);
                }
            }
        }

    return OCBRep::IncrementalMesh(*shape, *facetOptions);
#endif

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr BRepUtil::FacetEntity(IBRepEntityCR entity, IFacetOptionsR facetOptions)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidUtil::FacetEntity(entity, facetOptions);
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(entity);
    BeAssert(nullptr != shape);
    return (nullptr != shape ? OCBRep::IncrementalMesh(*shape, facetOptions) : nullptr);
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
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    return false;
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
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    return OCBRep::ClipCurveVector(output, input, clipVector, transform);
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
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(entity);
    BeAssert(nullptr != shape);
    if (nullptr == shape)
        return ERROR;

    bvector<TopoDS_Shape> clipResults;

    if (SUCCESS != OCBRep::ClipTopoShape(clipResults, clipped, shape, clipVector))
        return ERROR;

    for (TopoDS_Shape clipShape : clipResults)
        {
        IBRepEntityPtr entityPtr = SolidKernelUtil::CreateNewEntity(clipShape);

        output.push_back(entityPtr);
        }

    return SUCCESS;
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
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(entity);
    BeAssert(nullptr != shape);
    return (nullptr != shape ? OCBRep::HasCurvedFaceOrEdge(*shape) : false);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::GetFaceLocation(ISubEntityCR subEntity, DPoint3dR point, DPoint2dR param)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidSubEntity::GetFaceLocation(subEntity, point, param);
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::GetEdgeLocation(ISubEntityCR subEntity, DPoint3dR point, double& uParam)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidSubEntity::GetEdgeLocation(subEntity, point, uParam);
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::GetVertexLocation(ISubEntityCR subEntity, DPoint3dR point)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return PSolidSubEntity::GetVertexLocation(subEntity, point);
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
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromCurveVector (IBRepEntityPtr& entityOut, CurveVectorCR curveVector, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidGeom::BodyFromCurveVector (entityOut, curveVector, nullptr, nodeId);
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    TopoDS_Shape shape;

    if (SUCCESS != OCBRep::Create::TopoShapeFromCurveVector(shape, curveVector))
        return ERROR;

    entityOut = SolidKernelUtil::CreateNewEntity(shape);

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromSolidPrimitive (IBRepEntityPtr& entityOut, ISolidPrimitiveCR primitive, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidGeom::BodyFromSolidPrimitive (entityOut, primitive, nodeId);
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    TopoDS_Shape shape;

    if (SUCCESS != OCBRep::Create::TopoShapeFromSolidPrimitive(shape, primitive))
        return ERROR;

    entityOut = SolidKernelUtil::CreateNewEntity(shape);

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromBSurface (IBRepEntityPtr& entityOut, MSBsplineSurfaceCR surface, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidGeom::BodyFromBSurface (entityOut, surface, nodeId);
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    TopoDS_Shape shape;

    if (SUCCESS != OCBRep::Create::TopoShapeFromBSurface(shape, surface))
        return ERROR;

    entityOut = SolidKernelUtil::CreateNewEntity(shape);

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Create::BodyFromPolyface (IBRepEntityPtr& entityOut, PolyfaceQueryCR meshData, uint32_t nodeId)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidGeom::BodyFromPolyface (entityOut, meshData, nodeId);
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    TopoDS_Shape shape;

    if (SUCCESS != OCBRep::Create::TopoShapeFromPolyface(shape, meshData))
        return ERROR;

    entityOut = SolidKernelUtil::CreateNewEntity(shape);

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::BooleanIntersect (IBRepEntityPtr& targetEntity, IBRepEntityPtr* toolEntities, size_t nTools)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidUtil::DoBoolean (targetEntity, toolEntities, nTools, PK_boolean_intersect, PKI_BOOLEAN_OPTION_AllowDisjoint);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::BooleanSubtract (IBRepEntityPtr& targetEntity, IBRepEntityPtr* toolEntities, size_t nTools)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidUtil::DoBoolean (targetEntity, toolEntities, nTools, PK_boolean_subtract, PKI_BOOLEAN_OPTION_AllowDisjoint);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::BooleanUnion (IBRepEntityPtr& targetEntity, IBRepEntityPtr* toolEntities, size_t nTools)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidUtil::DoBoolean (targetEntity, toolEntities, nTools, PK_boolean_unite, PKI_BOOLEAN_OPTION_AllowDisjoint);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepUtil::Modify::SewBodies (bvector<IBRepEntityPtr>& sewnEntities, bvector<IBRepEntityPtr>& unsewnEntities, IBRepEntityPtr* toolEntities, size_t nTools, double gapWidthBound, size_t nIterations)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    if (nTools < 2)
        return ERROR;

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create (&markTag);

    bool                 isFirst = true;
    Transform            targetTransform, invTargetTransform;
    bvector<PK_ENTITY_t> toolEntityTags;

    // Get tool bodies in coordinates of target...
    for (size_t iTool = 0; iTool < nTools; ++iTool)
        {
        bool        isToolOwned;
        PK_ENTITY_t toolEntityTag = PSolidUtil::GetEntityTag (*toolEntities[iTool], &isToolOwned);

        if (!isToolOwned)
            PK_ENTITY_copy (toolEntityTag, &toolEntityTag);
            
        if (isFirst)
            {
            isFirst = false;
            targetTransform = toolEntities[iTool]->GetEntityTransform ();
            invTargetTransform.InverseOf (targetTransform);
            invTargetTransform.ScaleDoubleArrayByXColumnMagnitude(&gapWidthBound,  1);
            }
        else
            {
            Transform   toolTransform;

            toolTransform.InitProduct (invTargetTransform, toolEntities[iTool]->GetEntityTransform ());
            PSolidUtil::TransformBody (toolEntityTag, toolTransform);
            }

        toolEntityTags.push_back (toolEntityTag);
        }

    int                       nSewnBodies = 0, nUnsewnBodies = 0, nProblems = 0;
    PK_BODY_t*                sewnBodyTags = NULL;
    PK_BODY_t*                unsewnBodyTags = NULL;
    PK_BODY_problem_group_t*  problemGroup = NULL;
    PK_BODY_sew_bodies_o_t    options;

    PK_BODY_sew_bodies_o_m (options);

    options.allow_disjoint_result = PK_LOGICAL_true;
    options.number_of_iterations  = (int) nIterations;

    BentleyStatus   status = (SUCCESS == PK_BODY_sew_bodies ((int) toolEntityTags.size (), &toolEntityTags.front (), gapWidthBound, &options, &nSewnBodies, &sewnBodyTags, &nUnsewnBodies, &unsewnBodyTags, &nProblems, &problemGroup) ? SUCCESS : ERROR);

    if (SUCCESS == status)
        {
        if (sewnBodyTags)
            {
            for (int iSewn = 0; iSewn < nSewnBodies; ++iSewn)
                sewnEntities.push_back (PSolidUtil::CreateNewEntity (sewnBodyTags[iSewn], targetTransform, true));
            }

        if (unsewnBodyTags)
            {
            for (int iUnsewn = 0; iUnsewn < nUnsewnBodies; ++iUnsewn)
                unsewnEntities.push_back (PSolidUtil::CreateNewEntity (unsewnBodyTags[iUnsewn], targetTransform, true));
            }

        // Invalidate owned tool entities that are now reflected in sewn and unsewn lists...
        for (size_t iTool = 0; iTool < nTools; ++iTool)
            PSolidUtil::ExtractEntityTag (*toolEntities[iTool]);
        }
    else
        {
        // Undo copy/transform of input entities...
        PK_MARK_goto (markTag);
        }

    PK_MEMORY_free (sewnBodyTags);
    PK_MEMORY_free (unsewnBodyTags);

    if (problemGroup)
        {
        for (int iProblem = 0; iProblem < nProblems; ++iProblem)
            PK_MEMORY_free (problemGroup[iProblem].edges);

        PK_MEMORY_free (problemGroup);
        }

    PK_MARK_delete (markTag);

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
    bool        isOwned;
    PK_ENTITY_t entityTag = PSolidUtil::GetEntityTag(entity, &isOwned);

    if (!isOwned)
        PK_ENTITY_copy(entityTag, &entityTag);

    bvector<PK_BODY_t> bodies;

    if (SUCCESS != PSolidUtil::DisjoinBody(bodies, entityTag))
        {
        if (!isOwned)
            PK_ENTITY_delete(1, &entityTag);

        return ERROR;
        }

    Transform entityTransform = entity.GetEntityTransform();

    if (isOwned)
        PSolidUtil::ExtractEntityTag(entity); // Invalidate input entity, will appear first in output bodies vector...

    for (PK_BODY_t thisBody : bodies)
        output.push_back(PSolidUtil::CreateNewEntity(thisBody, entityTransform, true));

    return ERROR;
#else
    return ERROR;
#endif
    }



