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
FaceAttachment::FaceAttachment ()
    {
    m_useColor = m_useMaterial = false;
    m_color = ColorDef::Black();
    m_transparency = 0.0;
    m_uv.Init(0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment (GeometryParamsCR sourceParams)
    {
    m_categoryId    = sourceParams.GetCategoryId();
    m_subCategoryId = sourceParams.GetSubCategoryId();
    m_transparency  = sourceParams.GetTransparency();

    m_useColor = !sourceParams.IsLineColorFromSubCategoryAppearance();
    m_color = m_useColor ? sourceParams.GetLineColor() : ColorDef::Black();

    if (m_useMaterial = !sourceParams.IsMaterialFromSubCategoryAppearance())
        m_material = sourceParams.GetMaterialId();

    m_uv.Init(0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void FaceAttachment::ToGeometryParams (GeometryParamsR elParams) const
    {
    elParams.SetCategoryId(m_categoryId);
    elParams.SetSubCategoryId(m_subCategoryId);
    elParams.SetTransparency(m_transparency);

    if (m_useColor)
        elParams.SetLineColor(m_color);

    if (m_useMaterial)
        elParams.SetMaterialId(m_material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator==(struct FaceAttachment const& rhs) const
    {
    if (m_useColor      != rhs.m_useColor ||
        m_useMaterial   != rhs.m_useMaterial ||
        m_categoryId    != rhs.m_categoryId ||
        m_subCategoryId != rhs.m_subCategoryId ||
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
bool FaceAttachment::operator< (struct FaceAttachment const& rhs) const
    {
    return (m_useColor         < rhs.m_useColor ||
            m_useMaterial      < rhs.m_useMaterial ||
            m_categoryId       < rhs.m_categoryId ||
            m_subCategoryId    < rhs.m_subCategoryId ||
            m_color.GetValue() < rhs.m_color.GetValue() ||
            m_transparency     < rhs.m_transparency ||
            m_material         < rhs.m_material ||
            m_uv.x             < rhs.m_uv.x || 
            m_uv.y             < rhs.m_uv.y);
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr BRepUtil::FacetEntity(IBRepEntityCR entity, double pixelSize, DRange1dP pixelSizeRange)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidUtil::FacetEntity(entity, pixelSize, pixelSizeRange);
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
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
#else
    return nullptr;
#endif
    }

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
bool BRepUtil::FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<GeometryParams>& params, double pixelSize, DRange1dP pixelSizeRange)
    {
#if defined (BENTLEYCONFIG_PARASOLID) 
    return PSolidUtil::FacetEntity(entity, polyfaces, params, pixelSize, pixelSizeRange);
#elif defined (BENTLEYCONFIG_OPENCASCADE) 
    return false;
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool BRepUtil::FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<GeometryParams>& params, IFacetOptionsR facetOptions)
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

    


