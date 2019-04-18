/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/Classification.h"
#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationSystem.h"
#include "PublicApi/ClassificationTable.h"
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationTable)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnCode ClassificationTable::GetTableCode
(
    ClassificationSystemCR system,
    Utf8CP name
)
    {
    return Dgn::DgnCode(system.GetDgnDb().CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationTable), system.GetElementId(), name);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
ClassificationTable::ClassificationTable
(
    CreateParams const& params, 
    ClassificationSystemCR system,
    Utf8CP name
) : ClassificationTable(params)
    {
    Dgn::DgnCode code = GetTableCode(system, name);
    SetCode(code);
    SetClassificationSystemId(system.GetElementId());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
ClassificationTablePtr ClassificationTable::Create
(
    ClassificationSystemCR system,
    Utf8CP name
)
    {
    Dgn::DgnDbR db = system.GetDgnDb();
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, Building::Shared::BuildingUtils::GetOrCreateDefinitionModel(db, "ClassificationSystems")->GetModelId(), classId);

    return new ClassificationTable(params, system, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationTableCPtr ClassificationTable::TryGet
(
    ClassificationSystemCR system,
    Utf8StringCR name
)
    {
    Dgn::DgnCode code = GetTableCode(system, name.c_str());
    Dgn::DgnElementId id = system.GetDgnDb().Elements().QueryElementIdByCode(code);

    if (id.IsValid())
        return ClassificationTable::Get(system.GetDgnDb(), id);

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationTableCPtr ClassificationTable::GetOrCreateTableByName
(
    ClassificationSystemCR classificationSystem,
    Utf8StringCR name
)
    {
    ClassificationTableCPtr queriedTable = TryGet(classificationSystem, name);

    if (queriedTable.IsValid())
        {
        Dgn::DgnElementId systemId = queriedTable->GetClassificationSystemId();
        if (systemId.IsValid() && systemId == classificationSystem.GetElementId())
            return queriedTable;
        }

    // If we got here that means table is either non existant or does not belong to specified system
    ClassificationTablePtr table = ClassificationTable::Create(classificationSystem, name.c_str());
    if (table.IsNull())
        return nullptr;

    table->Insert();
    return table;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
void ClassificationTable::SetClassificationSystemId
(
    Dgn::DgnElementId systemId
)
    {
    SetParentId(systemId, GetDgnDb().Schemas().GetClassId(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ClassificationSystemContainsClassificationTable));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
Utf8CP ClassificationTable::GetName() const
    {
    return GetCode().GetValueUtf8CP();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementId ClassificationTable::GetClassificationSystemId() const
    {
    return GetParentId();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator ClassificationTable::MakeClassificationIterator() const
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
Dgn::ElementIterator ClassificationTable::MakeClassificationGroupIterator() const
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
void ClassificationTable::GetClassificationDataVerbose
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
        BeAssert(classificationGroup.IsValid());

        elementValue["label"] = classificationGroup->GetName();
        elementData["groups"].append(elementValue);
        }

    for (Dgn::ElementIteratorEntry elementIter : MakeClassificationIterator())
        {
        Json::Value elementValue;
        elementValue["id"] = elementIter.GetElementId().ToHexStr();

        ClassificationCPtr classification = db.Elements().Get<Classification>(elementIter.GetElementId());
        BeAssert(classification.IsValid());

        elementValue["label"] = classification->GetName();
        elementValue["groupId"] = classification->GetGroupId().ToHexStr();
        elementData["classifications"].append(elementValue);
        }
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
