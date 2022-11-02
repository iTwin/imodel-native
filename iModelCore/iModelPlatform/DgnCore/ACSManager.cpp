/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VecMath.h>

USING_NAMESPACE_BENTLEY_DGN

#define     TRIAD_SIZE_INCHES       0.6
#define     ARROW_BASE_START        0.3
#define     ARROW_BASE_WIDTH        0.2
#define     ARROW_TIP_END           1.25
#define     ARROW_TIP_START         0.85
#define     ARROW_TIP_FLANGE        0.75
#define     ARROW_TIP_WIDTH         0.4

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace ACSElementHandler
{
    HANDLER_DEFINE_MEMBERS(CoordSys2d);
    HANDLER_DEFINE_MEMBERS(CoordSys3d);
    HANDLER_DEFINE_MEMBERS(CoordSysSpatial);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AuxCoordSystemPtr AuxCoordSystem::CreateNew(ViewDefinitionCR def, Utf8StringCR name)
    {
    DefinitionModelPtr model = def.GetDefinitionModel();
    if (!model.IsValid())
        return nullptr;

    if (def.IsSpatialView())
        return new AuxCoordSystemSpatial(*model, name);
    else if (def.IsView3d())
        return new AuxCoordSystem3d(*model, name);
    else
        return new AuxCoordSystem2d(*model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AuxCoordSystemPtr AuxCoordSystem::CreateNew(DgnModelR model, DefinitionModelP defnModel, Utf8StringCR name)
    {
    DefinitionModelPtr scope(defnModel);
    if (!scope.IsValid())
        scope = &model.GetDgnDb().GetDictionaryModel();

    if (model.IsSpatialModel())
        return new AuxCoordSystemSpatial(*scope, name);
    else if (model.Is3dModel())
        return new AuxCoordSystem3d(*scope, name);
    else
        return new AuxCoordSystem2d(*scope, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode AuxCoordSystem::CreateCode(ViewDefinitionCR def, Utf8StringCR name)
    {
    DefinitionModelPtr model = def.GetDefinitionModel();
    if (!model.IsValid())
        return DgnCode();

    if (def.IsSpatialView())
        return AuxCoordSystemSpatial::CreateCode(*model, name);
    else if (def.IsView3d())
        return AuxCoordSystem3d::CreateCode(*model, name);
    else
        return AuxCoordSystem2d::CreateCode(*model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode AuxCoordSystem::CreateCode(DgnModelR model, DefinitionModelP defnModel, Utf8StringCR name)
    {
    DefinitionModelPtr scope(defnModel);
    if (!scope.IsValid())
        scope = &model.GetDgnDb().GetDictionaryModel();

    if (model.IsSpatialModel())
        return AuxCoordSystemSpatial::CreateCode(*scope, name);
    else if (model.Is3dModel())
        return AuxCoordSystem3d::CreateCode(*scope, name);
    else
        return AuxCoordSystem2d::CreateCode(*scope, name);
    }