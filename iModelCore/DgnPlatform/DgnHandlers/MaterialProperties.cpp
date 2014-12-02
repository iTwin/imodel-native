/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/MaterialProperties.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//=======================================================================================
// @bsiclass                                                    PaulChater      09/11
//=======================================================================================
enum
    {
    MPTOOLS_APPLICATION_SIGNATURE                       = 22239,
    MPTOOLS_XATTRIBUTE_APPVALUE_PROJECTION              = 2,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    09/11
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCP IMaterialPropertiesExtension::FindMaterial (ElementHandleCR eh, LevelId level, UInt32 colorIndex, bool useSymbologyOverride) const
    {
    IMaterialPropertiesExtension *mExtension = IMaterialPropertiesExtension::Cast (eh.GetHandler());

    if (mExtension)
        {
        MaterialCP material = mExtension->FindMaterialAttachment (eh);
        if (material)
            return material;
        }

    BeAssert (false);
    return NULL;
#ifdef WIP_DGNDB_MATERIALS
    return MaterialManager::GetManagerR().FindMaterialBySymbology (level, colorIndex, eh.GetDgnProject());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCP CommonMaterialPropertiesExtension::_FindMaterialAttachment (ElementHandleCR eh) const
    {
    return MaterialManager::GetManagerR().FindMaterialAttachment (eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul Chater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CommonMaterialPropertiesExtension::_AddMaterialAttachment (EditElementHandleR eeh, DgnMaterialId id)
    {
    if (!id.IsValid())
        return ERROR;

    return MaterialManager::GetManagerR().SetMaterialAttachmentId (eeh, id);
    }

#if defined (NEEDS_WORK_DGNITEM)
//=======================================================================================
//! Provides methods for inspecting the current material properties of grouped hole element
// @bsiclass                                                    PaulChater 09/11
//=======================================================================================
struct GroupedHoleMaterialPropertiesExtension : CommonMaterialPropertiesExtension
{
private:

typedef CommonMaterialPropertiesExtension   T_Super;

protected :
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul Chater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCP _FindMaterialAttachment (ElementHandleCR eh) const
    {
    ChildElemIter child (eh, ExposeChildrenReason::Count);
    return T_Super::_FindMaterialAttachment (child);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul Chater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus _AddMaterialAttachment (EditElementHandleR eeh, DgnMaterialId id)
    {
    ChildEditElemIter child (eeh, ExposeChildrenReason::Count);
    return T_Super::_AddMaterialAttachment (child, id);
    }
}; // GroupedHoleMaterialPropertiesExtension

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul Chater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void    CommonMaterialPropertiesExtension::Register ()
    {
    CommonMaterialPropertiesExtension  *commonExtension = new CommonMaterialPropertiesExtension ();

    IMaterialPropertiesExtension::RegisterExtension(EllipseHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(BSplineCurveHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(ShapeHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(ComplexShapeHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(ConeHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(SurfaceOrSolidHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(BSplineSurfaceHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(MeshHeaderHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(BrepCellHeaderHandler::GetInstance(), *commonExtension);
    IMaterialPropertiesExtension::RegisterExtension(ExtendedElementHandler::GetInstance(), *commonExtension);

    IMaterialPropertiesExtension::RegisterExtension(GroupedHoleHandler::GetInstance(), *new GroupedHoleMaterialPropertiesExtension ());
    }

#endif
