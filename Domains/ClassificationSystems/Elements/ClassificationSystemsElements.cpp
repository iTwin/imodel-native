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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
Dgn::DgnCode ClassificationSystem::GetSystemCode
(
    Dgn::DgnDbR db,
    Utf8CP name
)
    {
    return Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), db.Elements().GetRootSubjectId(), name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystem::ClassificationSystem
(
CreateParams const& params,
Utf8CP name
) : T_Super(params)
    {
        Dgn::DgnCode code = GetSystemCode(params.m_dgndb, name);
        SetCode(code);
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
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationSystemCPtr ClassificationSystem::TryGetSystem
(
    Dgn::DgnDbR db,
    Utf8CP name
)
    {
    Dgn::DgnCode code = GetSystemCode(db, name);
    Dgn::DgnElementId id = db.Elements().QueryElementIdByCode(code);
    if (id.IsValid())
        {
        ClassificationSystemCPtr system = db.Elements().Get<ClassificationSystem>(id);
        return system;
        }
    else {
        return nullptr;
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              05/2018
//---------------------------------------------------------------------------------------
Utf8CP ClassificationSystem::GetName() const
    {
    return GetCode().GetValueUtf8CP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
Dgn::DgnCode Classification::GetClassificationCode
(
    Dgn::DgnDbR db,
    Utf8CP name,
    Dgn::DgnElementId id
) const
    {
    return Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), id, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Classification::Classification
(
CreateParams const& params,
ClassificationSystemCR system,
Utf8CP name,
Utf8CP id,
Utf8CP description,
ClassificationGroupCP group,
ClassificationCP specializes
) : T_Super(params) 
    {
    Dgn::DgnElementId elemid;
    SetName(name);
    SetDescription(description);
    if(group != nullptr) 
        {
        SetGroupId(group->GetElementId());
        elemid = group->GetElementId();
        }
    else 
        {
        elemid = system.GetElementId();
        }
    Dgn::DgnCode code = GetClassificationCode(params.m_dgndb, id, elemid);
    SetCode(code);
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
    ClassificationPtr classification = new Classification(params, system, name, id, description, group, specializes);
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
    SetUserLabel(name);
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