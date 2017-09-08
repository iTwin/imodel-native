/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentXSViewDefinition.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentXSViewDefinition.h>

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
AlignmentXSViewDefinition::AlignmentXSViewDefinition(DefinitionModelR model, Utf8StringCR name, CategorySelectorR categories, DisplayStyle3dR displayStyle, ModelSelectorR modelSelector): 
    T_Super(T_Super::CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name), categories, displayStyle, modelSelector))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
//---------------------------------------------------------------------------------------
DgnViewId AlignmentXSViewDefinition::QuerySystemViewId(DgnDbR db)
    {
    DictionaryModelCR dictionary = db.GetDictionaryModel();
    const DgnElementId eid = db.Elements().QueryElementIdByCode(CreateCode(dictionary, SYSTEM_VIEW_NAME));
    return DgnViewId(eid.GetValueUnchecked());
    }
