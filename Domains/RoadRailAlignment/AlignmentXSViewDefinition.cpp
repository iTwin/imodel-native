/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentXSViewDefinition.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentXSViewDefinition.h>
#include <RoadRailAlignment/RoadRailAlignmentDomain.h>

HANDLER_DEFINE_MEMBERS(AlignmentXSViewDefinitionHandler)

AlignmentXSViewDefinition::IViewControllerFactory AlignmentXSViewDefinition::s_factory = nullptr;
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        08/2017
//---------------------------------------------------------------------------------------
void AlignmentXSViewDefinition::RegisterControllerFactory(IViewControllerFactory factory)
    {
    s_factory = factory;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        08/2017
//---------------------------------------------------------------------------------------
ViewControllerPtr AlignmentXSViewDefinition::_SupplyController() const
    {
    if (nullptr != s_factory)
        return s_factory(*this);

    ROADRAILALIGNMENT_LOGE("AlignmentXSViewDefinition - ViewControllerFactory not registered");
    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AlignmentXSViewDefinition::_SetRotation(RotMatrixCR rot)
    {
    if (m_allowRotation)
        m_rotation = rot;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        08/2017
//---------------------------------------------------------------------------------------
AlignmentXSViewDefinition::AlignmentXSViewDefinition(ConfigurationModelCR model, Utf8StringCR name, CategorySelectorR categories, DisplayStyle3dR displayStyle, ModelSelectorR modelSelector): 
    T_Super(T_Super::CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name), categories, displayStyle, modelSelector))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
//---------------------------------------------------------------------------------------
DgnViewId AlignmentXSViewDefinition::QuerySystemViewId(SubjectCR subject)
    {
    auto configurationModelPtr = ConfigurationModel::Query(subject);
    const DgnElementId eid = subject.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(*configurationModelPtr, SYSTEM_VIEW_NAME));
    return DgnViewId(eid.GetValueUnchecked());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        10/2017
//---------------------------------------------------------------------------------------
bool AlignmentXSViewDefinition::_ViewsModel(DgnModelId modelId)
    {
    // Manipulator code checks if the model is viewed. Since we're using a model that's not persisted, its Id is '0'
    // return true when that happens
    if (!modelId.IsValid())
        return true;

    return T_Super::_ViewsModel(modelId);
    }
