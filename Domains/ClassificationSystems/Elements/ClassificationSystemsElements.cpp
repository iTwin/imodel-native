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
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystemClassDefinition)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystemClassDefinitionGroup)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(CIBSEClassDefinition)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(OmniClassClassDefinition)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ASHRAEClassDefinition)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ASHRAE2004ClassDefinition)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ASHRAE2010ClassDefinition)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(MasterFormatClassDefinition)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(UniFormatClassDefinition)

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
ClassificationSystemClassDefinitionGroup::ClassificationSystemClassDefinitionGroup
(
CreateParams const& params,
Utf8CP name
) : T_Super(params)
    {
    SetName(name);
    //SetCategory(Category);  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystemClassDefinitionGroupPtr ClassificationSystemClassDefinitionGroup::Create
(
Dgn::DgnDbR db,
Utf8CP name
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, db.GetDictionaryModel().GetModelId(), classId);
    ClassificationSystemClassDefinitionGroupPtr group = new ClassificationSystemClassDefinitionGroup(params, name);
    return group;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void                            ClassificationSystemClassDefinition::SetGroupId
(
Dgn::DgnElementId groupId
)
    {
    SetPropertyValue(prop_Group(), groupId, GetDgnDb().Schemas().GetClassId(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ClassificationSystemClassDefinitionIsInClassificationSystemClassDefinitionGroup));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId               ClassificationSystemClassDefinition::GetGroupId
(
) const
    {
    return GetPropertyValueId<Dgn::DgnElementId>(prop_Group());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CIBSEClassDefinition::CIBSEClassDefinition
(
CreateParams const& params,
Utf8CP name,
ClassificationSystemClassDefinitionGroupCR group
) : T_Super(params)
    {
    SetName(name);
    SetGroupId(group.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CIBSEClassDefinitionPtr CIBSEClassDefinition::Create
(
Dgn::DgnDbR db,
Utf8CP name,
ClassificationSystemClassDefinitionGroupCR group
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, db.GetDictionaryModel().GetModelId(), classId);
    CIBSEClassDefinitionPtr definition = new CIBSEClassDefinition(params, name, group);
    return definition;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OmniClassClassDefinition::OmniClassClassDefinition
(
CreateParams const& params,
Utf8CP name, 
Utf8CP omniClassID, 
Utf8CP description
) : T_Super(params)
    {
    SetName(name);
    SetOmniClassID(omniClassID);
    SetDescription(description);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OmniClassClassDefinitionPtr OmniClassClassDefinition::Create
(
Dgn::DgnDbR db,
Utf8CP name, 
Utf8CP omniClassID, 
Utf8CP description
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, db.GetDictionaryModel().GetModelId(), classId);
    OmniClassClassDefinitionPtr definition = new OmniClassClassDefinition(params, name, omniClassID, description);
    return definition;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ASHRAE2004ClassDefinition::ASHRAE2004ClassDefinition
(
CreateParams const& params,
Utf8CP name,
Utf8CP Category
) : T_Super(params)
    {
    SetName(name);
    SetCategory(Category);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ASHRAE2004ClassDefinitionPtr ASHRAE2004ClassDefinition::Create
(
Dgn::DgnDbR db,
Utf8CP name,
Utf8CP Category
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, db.GetDictionaryModel().GetModelId(), classId);
    ASHRAE2004ClassDefinitionPtr definition = new ASHRAE2004ClassDefinition(params, name, Category);
    return definition;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ASHRAE2010ClassDefinition::ASHRAE2010ClassDefinition
(
CreateParams const& params,
Utf8CP name,
Utf8CP Category
) : T_Super(params)
    {
    SetName(name);
    SetCategory(Category);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ASHRAE2010ClassDefinitionPtr ASHRAE2010ClassDefinition::Create
(
Dgn::DgnDbR db,
Utf8CP name,
Utf8CP Category
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, db.GetDictionaryModel().GetModelId(), classId);
    ASHRAE2010ClassDefinitionPtr definition = new ASHRAE2010ClassDefinition(params, name, Category);
    return definition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MasterFormatClassDefinition::MasterFormatClassDefinition
(
CreateParams const& params,
Utf8CP name,
Utf8CP description
) : T_Super(params)
    {
    SetName(name);
    SetDescription(description);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MasterFormatClassDefinitionPtr MasterFormatClassDefinition::Create
(
Dgn::DgnDbR db,
Utf8CP name,
Utf8CP description
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, db.GetDictionaryModel().GetModelId(), classId);
    MasterFormatClassDefinitionPtr definition = new MasterFormatClassDefinition(params, name, description);
    return definition;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
UniFormatClassDefinition::UniFormatClassDefinition
(
CreateParams const& params,
Utf8CP name,
Utf8CP description
) : T_Super(params)
    {
    SetName(name);
    SetDescription(description);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
UniFormatClassDefinitionPtr UniFormatClassDefinition::Create
(
Dgn::DgnDbR db,
Utf8CP name,
Utf8CP description
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, db.GetDictionaryModel().GetModelId(), classId);
    UniFormatClassDefinitionPtr definition = new UniFormatClassDefinition(params, name, description);
    return definition;
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE