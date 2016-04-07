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

#if defined (BENTLEYCONFIG_OPENCASCADE)
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  03/16
+===============+===============+===============+===============+===============+======*/
struct OpenCascadeEntity : RefCounted<ISolidKernelEntity>
{
private:

TopoDS_Shape m_shape;

protected:

virtual Transform _GetEntityTransform () const override {Transform transform = OCBRepUtil::ToTransform(m_shape.Location()); return transform;}

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
            gp_Trsf goopTrsf = OCBRepUtil::ToGpTrsf(goopTrans);

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
            gp_GTrsf goopTrsf = OCBRepUtil::ToGpGTrsf(goopTrans);

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

    m_shape.Location(OCBRepUtil::ToGpTrsf(shapeTrans));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
KernelEntityType _GetEntityType () const
    {
    switch (m_shape.ShapeType())
        {
        case TopAbs_SOLID:
            return ISolidKernelEntity::EntityType_Solid;

        case TopAbs_SHELL:
            return ISolidKernelEntity::EntityType_Sheet;

        case TopAbs_WIRE:
            return ISolidKernelEntity::EntityType_Wire;

        default:
            return ISolidKernelEntity::EntityType_Minimal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetEntityRange() const
    {
    Bnd_Box box;

    BRepBndLib::Add(m_shape, box, false); // Never use triangulation...

    return OCBRepUtil::ToDRange3d(box);
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
