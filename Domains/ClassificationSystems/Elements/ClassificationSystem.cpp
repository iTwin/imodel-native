/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
Dgn::DgnModelCR model,
Utf8StringCR name, 
Utf8StringCR edition
)
    {
    return Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), model.GetModeledElementId(), name + edition);
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
        Dgn::DgnCode code = GetSystemCode(params.m_dgndb, *params.m_dgndb.Models().Get<Dgn::DgnModel>(params.m_modelId), name, edition);
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
Dgn::DgnModelCR model,
Utf8StringCR name, 
Utf8StringCR edition
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, model.GetModelId(), classId);
    ClassificationSystemPtr system = new ClassificationSystem(params, name, edition);
    return system;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystemCPtr ClassificationSystem::TryGet
(
Dgn::DgnDbR db,
Dgn::DgnModelCR model,
Utf8StringCR name,
Utf8StringCR edition
)
    {
    Dgn::DgnCode code = GetSystemCode(db, model, name, edition);
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
Dgn::DgnModelCR model,
Utf8StringCR name,
Utf8StringCR edition
)
    {
    ClassificationSystemCPtr queriedSystem = TryGet(db, model, name, edition);
    if (queriedSystem.IsValid())
        return queriedSystem;

    ClassificationSystemPtr system = ClassificationSystem::Create(db, model, name, edition);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2019
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator ClassificationSystem::MakeClassificationsIterator(Dgn::DgnElementCR el)
    {
    Utf8String sql("SELECT TargetECInstanceId FROM " CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_REL_ElementHasClassifications) " as clsrel join " CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_CLASS_Classification) " as cls on cls.ECInstanceId = clsrel.TargetECInstanceId join biscore.model as mdl on mdl.ECInstanceId = cls.Model.id join " CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_CLASS_ClassificationTable)" as clstbl on clstbl.ECInstanceId=mdl.ModeledElement.Id where clsrel.SourceECInstanceId=? and clstbl.Parent.Id=?");
    Dgn::ElementIterator iterator;
    iterator.Prepare(el.GetDgnDb(), sql.c_str(), 0 /* Index of ECInstanceId */);
    iterator.GetStatement()->BindId(1, el.GetElementId());
    iterator.GetStatement()->BindId(2, GetElementId());
    return iterator;
    }


END_CLASSIFICATIONSYSTEMS_NAMESPACE
