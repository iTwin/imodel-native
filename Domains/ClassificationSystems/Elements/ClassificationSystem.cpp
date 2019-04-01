/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ClassificationSystem.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/Classification.h"
#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationSystem.h"

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystem)


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
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator ClassificationSystem::MakeClassificationIterator() const
    {
    BeAssert(GetElementId().IsValid());

    Utf8String where("WHERE Model.Id=?");

    Dgn::ElementIterator iterator = GetDgnDb().Elements().MakeIterator(CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_CLASS_Classification), where.c_str());
    iterator.GetStatement()->BindId(1, GetElementId());
    return iterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator ClassificationSystem::MakeClassificationGroupIterator() const
    {
    BeAssert(GetElementId().IsValid());

    Utf8String where("WHERE Model.Id=?");

    Dgn::ElementIterator iterator = GetDgnDb().Elements().MakeIterator(CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_CLASS_ClassificationGroup), where.c_str());
    iterator.GetStatement()->BindId(1, GetElementId());
    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aurimas.Laureckis               07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::GetClassificationDataVerbose
(
    Json::Value& elementData
) const
    { 
    Dgn::DgnDbR db = GetDgnDb();
    elementData["name"] = GetName();
    elementData["id"] = GetElementId().ToHexStr();
    elementData["classifications"] = Json::Value(Json::arrayValue);
    elementData["groups"] = Json::Value(Json::arrayValue);
    for (Dgn::ElementIteratorEntry elementIter : MakeClassificationGroupIterator())
        {
        Json::Value elementValue;
        elementValue["id"] = elementIter.GetElementId().ToHexStr();

        ClassificationGroupCPtr classificationGroup = db.Elements().Get<ClassificationGroup>(elementIter.GetElementId());
        BeAssert (classificationGroup.IsValid());
            
        elementValue["label"] = classificationGroup->GetName();
        elementData["groups"].append(elementValue);
        }

    for (Dgn::ElementIteratorEntry elementIter : MakeClassificationIterator())
        {
        Json::Value elementValue;
        elementValue["id"] = elementIter.GetElementId().ToHexStr();

        ClassificationCPtr classification = db.Elements().Get<Classification>(elementIter.GetElementId());
        BeAssert (classification.IsValid());
        
        elementValue["label"] = classification->GetName();
        elementValue["groupId"] = classification->GetGroupId().ToHexStr();
        elementData["classifications"].append(elementValue);
        }
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
    Utf8CP name
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, BS::BuildingUtils::GetOrCreateDefinitionModel(db, "ClassificationSystems")->GetModelId(), classId);
    ClassificationSystemPtr system = new ClassificationSystem(params, name);
    return system;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationSystemCPtr ClassificationSystem::TryGet
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

END_CLASSIFICATIONSYSTEMS_NAMESPACE
