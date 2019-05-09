/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/Classification.h"
#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationSystem.h"
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystem)


//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
Dgn::DgnCode ClassificationSystem::GetSystemCode
(
    Dgn::DgnDbR db,
    Utf8StringCR name, 
    Utf8StringCR edition
)
    {
    return Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), db.Elements().GetRootSubjectId(), name + edition);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystem::ClassificationSystem
(
    CreateParams const& params,
    Utf8StringCR name, 
    Utf8StringCR edition
) : T_Super(params)
    {
        Dgn::DgnCode code = GetSystemCode(params.m_dgndb, name, edition);
        SetCode(code);
        SetEdition(edition);
        SetUserLabel(name.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aurimas.Laureckis               06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::_SerializeProperties
(
    Json::Value& elementData
) const
    {
    elementData["name"] = GetName();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Aurimas.Laureckis              06/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator ClassificationSystem::MakeIterator(Dgn::DgnDbR dgnDbR)
    {
    return dgnDbR.Elements().MakeIterator(CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aurimas.Laureckis               06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::_FormatSerializedProperties
(
    Json::Value& elementData
) const
    {
    elementData["name"] = GetName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystemPtr ClassificationSystem::Create
(
    Dgn::DgnDbR db,
    Utf8StringCR name, 
    Utf8StringCR edition
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, BS::BuildingUtils::GetOrCreateDefinitionModel(db, "ClassificationSystems")->GetModelId(), classId);
    ClassificationSystemPtr system = new ClassificationSystem(params, name, edition);
    return system;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystemCPtr ClassificationSystem::TryGet
(
    Dgn::DgnDbR db,
    Utf8StringCR name,
    Utf8StringCR edition
)
    {
    Dgn::DgnCode code = GetSystemCode(db, name, edition);
    Dgn::DgnElementId id = db.Elements().QueryElementIdByCode(code);

    if (id.IsValid())
        return ClassificationSystem::Get(db, id);

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystemCPtr ClassificationSystem::GetOrCreateSystemByName
(
    Dgn::DgnDbR db,
    Utf8StringCR name,
    Utf8StringCR edition
)
    {
    ClassificationSystemCPtr queriedSystem = TryGet(db, name, edition);
    if (queriedSystem.IsValid())
        return queriedSystem;

    ClassificationSystemPtr system = ClassificationSystem::Create(db, name, edition);
    if (system.IsNull())
        return nullptr;

    system->Insert();
    return system;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              05/2018
//---------------------------------------------------------------------------------------
Utf8CP ClassificationSystem::GetName() const
    {
    return GetUserLabel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassificationSystem::GetSource() const
{
    return GetPropertyValueString(prop_Source());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::SetSource(Utf8StringCR source)
{
    SetPropertyValue(prop_Source(), source.c_str());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassificationSystem::GetEdition() const
{
    return GetPropertyValueString(prop_Edition());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::SetEdition(Utf8StringCR edition)
{
    SetPropertyValue(prop_Edition(), edition.c_str());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassificationSystem::GetLocation() const
{
    return GetPropertyValueString(prop_Location());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::SetLocation(Utf8StringCR location)
{
    SetPropertyValue(prop_Location(), location.c_str());
}

END_CLASSIFICATIONSYSTEMS_NAMESPACE
