/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentProfileViewDefinition.h>
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>

HANDLER_DEFINE_MEMBERS(AlignmentProfileViewDefinitionHandler)

AlignmentProfileViewDefinition::IViewControllerFactory AlignmentProfileViewDefinition::s_factory = nullptr;
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        08/2017
//---------------------------------------------------------------------------------------
void AlignmentProfileViewDefinition::RegisterControllerFactory(IViewControllerFactory factory)
    {
    s_factory = factory;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        08/2017
//---------------------------------------------------------------------------------------
ViewControllerPtr AlignmentProfileViewDefinition::_SupplyController() const
    {
    if (nullptr != s_factory)
        return s_factory(*this);

    ROADRAILALIGNMENT_LOGE("AlignmentProfileViewDefinition - ViewControllerFactory not registered");
    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
// Adjusts the AspectRatio by changing the y-axis only
//---------------------------------------------------------------------------------------
void AlignmentProfileViewDefinition::_AdjustAspectRatio(double windowAspect)
    {
    static bool s_useSuper = true;
    if (s_useSuper)
        {
        T_Super::_AdjustAspectRatio(windowAspect);
        return;
        }

    //&&AG needswork. Platform only calls this code when Fitting the view, but not when changing the aspect ratio skew
    // In that last case, it is handled by the DgnViewport::_AdjustAspectRatio code.
    DVec3d extents = GetExtents();
    const double viewAspect = extents.x / extents.y;

    windowAspect *= GetAspectRatioSkew();

    if (fabs(1.0 - (viewAspect / windowAspect)) < 1.0e-9)
        return;
    
    const DVec3d oldDelta = extents;
    if (viewAspect > windowAspect)
        extents.y = extents.x / windowAspect;

    DPoint3d origin = GetOrigin();
    DPoint3d newOrigin;
    GetRotation().Multiply(&newOrigin, &origin, 1);
    newOrigin.x += ((oldDelta.x - extents.x) / 2.0);
    newOrigin.y += ((oldDelta.y - extents.y) / 2.0);
    GetRotation().MultiplyTranspose(origin, newOrigin);
    SetOrigin(origin);
    SetExtents(extents);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AlignmentProfileViewDefinition::_SetRotation(RotMatrixCR rot)
    {
    if (m_allowRotation)
        m_rotation = rot;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        10/2017
//---------------------------------------------------------------------------------------
bool AlignmentProfileViewDefinition::_ViewsModel(DgnModelId modelId)
    {
    // Manipulator code checks if the model is viewed. Since we're using a model that's not persisted, its Id is '0'
    // return true when that happens
    if (!modelId.IsValid())
        return true;

    return T_Super::_ViewsModel(modelId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        08/2017
//---------------------------------------------------------------------------------------
AlignmentProfileViewDefinition::AlignmentProfileViewDefinition(ConfigurationModelCR model, Utf8StringCR name, CategorySelectorR categories, DisplayStyle3dR displayStyle, ModelSelectorR modelSelector): 
    T_Super(T_Super::CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name), categories, displayStyle, modelSelector))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
//---------------------------------------------------------------------------------------
DgnViewId AlignmentProfileViewDefinition::QuerySystemViewId(SubjectCR subject)
    {
    auto configurationModelPtr = ConfigurationModel::Query(subject);
    const DgnElementId eid = subject.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(*configurationModelPtr, SYSTEM_VIEW_NAME));
    return DgnViewId(eid.GetValueUnchecked());
    }
