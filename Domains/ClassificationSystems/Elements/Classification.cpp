/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/Classification.h"
#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationTable.h"
#include "PublicApi/ClassificationSystem.h"
#include <BuildingShared/BuildingSharedApi.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE
USING_NAMESPACE_BUILDING_SHARED

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(Classification)


//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
Dgn::DgnCode Classification::GetClassificationCode
(
    Dgn::DgnDbR db,
    Utf8StringCR id,
    Dgn::DgnElementId tableId
)
    {
    return Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), tableId, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnModelPtr Classification::GetOrCreateTableSubModel(ClassificationTableCR table)
    {
    Dgn::DgnModelPtr model = table.GetSubModel();
    if (model.IsNull())
        model = Dgn::DefinitionModel::CreateAndInsert(table);

    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Classification::Classification
(
    CreateParams const& params,
    ClassificationTableCR table,
    Utf8CP name,
    Utf8CP id,
    Utf8CP description,
    ClassificationGroupCP group,
    ClassificationCP specializes
) : T_Super(params) 
    {
    SetUserLabel(name);
    SetDescription(description);

    Dgn::DgnElementId elemid = table.GetElementId();
    Dgn::DgnCode code = GetClassificationCode(params.m_dgndb, id, elemid);
    SetCode(code);
    if(specializes != nullptr) 
        SetSpecializationId(specializes->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationPtr Classification::CreateAndInsert
(
    ClassificationTableCR table,
    Utf8CP name,
    Utf8CP id,
    Utf8CP description,
    ClassificationGroupCP group,
    ClassificationCP specializes
)
    {
    Dgn::DgnDbR db = table.GetDgnDb();

    Dgn::DgnModelPtr model = GetOrCreateTableSubModel(table);

    if (model.IsNull())
        {
        BeAssert(model.IsValid());
        return nullptr;
        }

    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, model->GetModelId(), classId);
    ClassificationPtr classification = new Classification(params, table, name, id, description, group, specializes);
    classification->Insert();
    if (group != nullptr)
        classification->SetGroupId(group->GetElementId());
    return classification;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2019
//---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipClassCR Classification::GetRelClassElementHasClassifications
(
Dgn::DgnDbR db
)
    {
    return static_cast<ECN::ECRelationshipClassCR>(*db.Schemas().GetClass(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ElementHasClassifications));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2019
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator Classification::MakeClassificationsIterator(Dgn::DgnElementCR el)
    {
    Utf8String sql("SELECT TargetECInstanceId FROM " CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_REL_ElementHasClassifications) " where SourceECInstanceId=?");
    Dgn::ElementIterator iterator;
    iterator.Prepare(el.GetDgnDb(), sql.c_str(), 0 /* Index of ECInstanceId */);
    iterator.GetStatement()->BindId(1, el.GetElementId());
    return iterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2019
//---------------+---------------+---------------+---------------+---------------+------
void Classification::RemoveFrom(Dgn::DgnElementCR el) const
    {
    Dgn::DgnDbR db = GetDgnDb();
    Dgn::DgnElementId sourceId = el.GetElementId();
    Dgn::DgnElementId targetId = GetElementId();
    ECN::ECRelationshipClassCR relClass = GetRelClassElementHasClassifications(db);

    BeAssert(sourceId.IsValid());

    RelationshipUtils::DeleteRelationships(db, relClass, sourceId, targetId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  08/2019
//---------------+---------------+---------------+---------------+---------------+------
void Classification::AddTo(Dgn::DgnElementCR el) const
    {
    Dgn::DgnElementId sourceId = el.GetElementId();
    BeAssert(sourceId.IsValid());

    if (ClassifiesElement(el))
        return;

    ClassificationTableCPtr table = ClassificationTable::Get(GetDgnDb(), GetClassificationTableId());
    BeAssert(table.IsValid());

    ClassificationSystemCPtr system = ClassificationSystem::Get(GetDgnDb(), table->GetClassificationSystemId());
    BeAssert(system.IsValid());

    Dgn::DgnDbR db = el.GetDgnDb();
    Dgn::DgnElementId targetId = GetElementId();
    ECN::ECRelationshipClassCR relClass = GetRelClassElementHasClassifications(db);

    RelationshipUtils::InsertRelationship(db, relClass, sourceId, targetId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
bool Classification::ClassifiesElement(Dgn::DgnElementCR el) const
    {
    Dgn::DgnDbR db = el.GetDgnDb();
    Dgn::DgnElementId sourceId = el.GetElementId();
    Dgn::DgnElementId targetId = GetElementId();
    ECN::ECRelationshipClassCR relClass = GetRelClassElementHasClassifications(db);

    BeAssert(sourceId.IsValid());

    return RelationshipUtils::RelationshipExists(db, relClass, sourceId, targetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationCPtr Classification::TryGet
(
Dgn::DgnDbR db,
Utf8StringCR classificationId,
Dgn::DgnElementId tableId
)
    {
    Dgn::DgnCode code = Classification::GetClassificationCode(db, classificationId, tableId);
    Dgn::DgnElementId id = db.Elements().QueryElementIdByCode(code);

    if (id.IsValid())
        return Classification::Get(db, id);

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationPtr Classification::GetOrCreateBySystemTableNames
(
Dgn::DgnDbR db,
Dgn::DgnModelCR model,
Utf8StringCR name,
Utf8StringCR id,
Utf8StringCR description,
Utf8StringCR systemName,
Utf8StringCR systemEdition,
Utf8StringCR tableName
)
    {
    ClassificationSystemCPtr system = ClassificationSystem::GetOrCreateSystemByName(db, model, systemName, systemEdition);
    if (system.IsNull())
        {
        BeAssert(!"Could not get or create system");
        return nullptr;
        }

    ClassificationTableCPtr table = ClassificationTable::GetOrCreateTableByName(*system, tableName);
    if (table.IsNull())
        {
        BeAssert(!"Could not get or create table");
        return nullptr;
        }

    ClassificationCPtr classification = Classification::TryGet(db, id, table->GetElementId());
    
    if(classification.IsValid())
        return classification->GetForEdit(db, classification->GetElementId());

    return Classification::CreateAndInsert(*table, name.c_str(), id.c_str(), description.c_str(), nullptr, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Classification::SetGroupId
(
    Dgn::DgnElementId groupId
)
    {
    Dgn::DgnDbR db = GetDgnDb();
    ECN::ECRelationshipClassCR relClass = (ECN::ECRelationshipClassCR)*db.Schemas().GetClass (CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ClassificationGroupGroupsClassifications);
    BeSQLite::EC::ECInstanceId sourceId (groupId);
    BeSQLite::EC::ECInstanceId targetId (GetElementId());

    BeSQLite::EC::ECInstanceKey rkey;
    db.InsertLinkTableRelationship (rkey, relClass, sourceId, targetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Classification::SetSpecializationId
(
    Dgn::DgnElementId specializationId
)
    {
    SetParentId(specializationId, GetDgnDb().Schemas().GetClassId(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ClassificationOwnsSubClassifications));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId Classification::GetGroupId() const
    {
    BeSQLite::EC::CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("SELECT SourceECInstanceId FROM " CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_REL_ClassificationGroupGroupsClassifications) " WHERE TargetECInstanceId = ?");

    if (!statement.IsValid())
        {
        BeAssert(false);
        return Dgn::DgnElementId();
        }
    statement->BindId(1, GetElementId());

    if (BeSQLite::DbResult::BE_SQLITE_ROW == statement->Step())
        {
        return statement->GetValueId<Dgn::DgnElementId>(0);
        }

    return Dgn::DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId Classification::GetSpecializationId() const
    {
    return GetParentId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId Classification::GetClassificationTableId() const
    {
    return GetModel()->GetModeledElementId();
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
