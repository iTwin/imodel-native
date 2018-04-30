/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ClassificationSystemsElements.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ClassificationSystems/ClassificationSystemsApi.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystem)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(Classification)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationGroup)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystem::ClassificationSystem
(
CreateParams const& params,
Utf8CP name
) : T_Super(params)
    {
    SetName(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystemPtr ClassificationSystem::Create
(
    Dgn::DgnDbR db,
    Utf8CP name
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, db.GetDictionaryModel().GetModelId(), classId);
    ClassificationSystemPtr system = new ClassificationSystem(params, name);
    return system;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::_OnInserted(Dgn::DgnElementP copiedFrom) const 
    {
        T_Super::_OnInserted(copiedFrom);
        Dgn::DefinitionModelPtr model = Dgn::DefinitionModel::Create(*this);
        if (!model.IsValid()) 
            return;
        Dgn::IBriefcaseManager::Request req;
        GetDgnDb().BriefcaseManager().PrepareForModelInsert(req, *model, Dgn::IBriefcaseManager::PrepareAction::Acquire);
        if (Dgn::DgnDbStatus::Success != model->Insert())
            return;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Classification::Classification
(
CreateParams const& params,
Utf8CP name,
Utf8CP id,
Utf8CP description,
ClassificationGroupCP group,
ClassificationCP specializes
) : T_Super(params) 
    {
    SetName(name);
    SetClassificationId(id);
    SetDescription(description);
    if(group != nullptr)
        SetGroupId(group->GetElementId());
    if(specializes != nullptr) 
        SetSpecializationId(specializes->GetElementId());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationPtr Classification::Create
(
ClassificationSystemCR system,
Utf8CP name,
Utf8CP id,
Utf8CP description,
ClassificationGroupCP group,
ClassificationCP specializes
)
    {
    Dgn::DgnClassId classId = QueryClassId(system.GetDgnDb());
    Dgn::DgnElement::CreateParams params(system.GetDgnDb(), system.GetSubModelId(), classId);
    ClassificationPtr classification = new Classification(params, name, id, description, group, specializes);
    return classification;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationGroup::ClassificationGroup
(
CreateParams const& params,
Utf8CP name
) : T_Super(params)
    {
    SetName(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationGroupPtr ClassificationGroup::Create
(
ClassificationSystemCR system,
Utf8CP name
)
    {
    Dgn::DgnClassId classId = QueryClassId(system.GetDgnDb());
    Dgn::DgnElement::CreateParams params(system.GetDgnDb(), system.GetSubModelId(), classId);
    ClassificationGroupPtr group = new ClassificationGroup(params, name);
    return group;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Classification::SetGroupId
(
Dgn::DgnElementId groupId
)
    {
    SetPropertyValue(prop_Group(), groupId, GetDgnDb().Schemas().GetClassId(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ClassificationIsInClassificationGroup));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Classification::SetSpecializationId
(
Dgn::DgnElementId specializationId
)
    {
    SetPropertyValue(prop_Specialization(), specializationId, GetDgnDb().Schemas().GetClassId(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ClassificationSpecializesClassification));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId Classification::GetGroupId
(
) const
    {
    return GetPropertyValueId<Dgn::DgnElementId>(prop_Group());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId Classification::GetSpecializationId
(
) const
    {
    return GetPropertyValueId<Dgn::DgnElementId>(prop_Specialization());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE