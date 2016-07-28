/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/SolidKernelUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#include <DgnPlatformInternal/DgnCore/ElementGraphics.fb.h>
#if defined (BENTLEYCONFIG_OPENCASCADE)
#include <DgnPlatform/DgnBRep/OCBRep.h>
#endif

using namespace flatbuffers;

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
struct OpenCascadeEntity : RefCounted<ISolidKernelEntity>
{
private:

TopoDS_Shape m_shape;

protected:

virtual Transform _GetEntityTransform () const override {Transform transform = OCBRep::ToTransform(m_shape.Location()); return transform;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _SetEntityTransform (TransformCR transform) override
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
                return;
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
                return;
                }

            m_shape = transformer.ModifiedShape(m_shape);
            }

        BeAssert(m_shape.Location().IsIdentity());
        }

    m_shape.Location(OCBRep::ToGpTrsf(shapeTrans));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
KernelEntityType ToEntityType(TopAbs_ShapeEnum shapeType) const
    {
    switch (shapeType)
        {
        case TopAbs_COMPOUND:
            return ISolidKernelEntity::EntityType_Solid; // NEEDSWORK: Add enum value...

        case TopAbs_COMPSOLID:
        case TopAbs_SOLID:
            return ISolidKernelEntity::EntityType_Solid;

        case TopAbs_SHELL:
        case TopAbs_FACE:
            return ISolidKernelEntity::EntityType_Sheet;

        case TopAbs_WIRE:
        case TopAbs_EDGE:
            return ISolidKernelEntity::EntityType_Wire;

        case TopAbs_VERTEX:
        default:
            return ISolidKernelEntity::EntityType_Minimal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
KernelEntityType _GetEntityType() const {return ToEntityType(OCBRepUtil::GetShapeType(m_shape));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetEntityRange() const
    {
    Bnd_Box box;

    BRepBndLib::AddOptimal(m_shape, box, false); // Never use triangulation...

    return OCBRep::ToDRange3d(box);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (ISolidKernelEntityCR entity) const override
    {
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
virtual ISolidKernelEntityPtr _Clone() const override
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
static OpenCascadeEntity* CreateNewEntity(TopoDS_Shape const& shape) {return new OpenCascadeEntity(shape);}

}; // OpenCascadeEntity
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TopoDS_Shape const* SolidKernelUtil::GetShape(ISolidKernelEntityCR entity)
    {
#if defined (BENTLEYCONFIG_OPENCASCADE)
    OpenCascadeEntity const* ocEntity = dynamic_cast <OpenCascadeEntity const*> (&entity);

    if (!ocEntity)
        return nullptr;

    return &ocEntity->GetShape();
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TopoDS_Shape* SolidKernelUtil::GetShapeP(ISolidKernelEntityR entity)
    {
#if defined (BENTLEYCONFIG_OPENCASCADE)
    OpenCascadeEntity* ocEntity = dynamic_cast <OpenCascadeEntity*> (&entity);

    if (!ocEntity)
        return nullptr;

    return &ocEntity->GetShapeR();
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidKernelEntityPtr SolidKernelUtil::CreateNewEntity(TopoDS_Shape const& shape)
    {
#if defined (BENTLEYCONFIG_OPENCASCADE)
    return OpenCascadeEntity::CreateNewEntity(shape);
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr SolidKernelUtil::FacetEntity(ISolidKernelEntityCR entity, double pixelSize, DRange1dP pixelSizeRange)
    {
#if defined (BENTLEYCONFIG_OPENCASCADE)
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
