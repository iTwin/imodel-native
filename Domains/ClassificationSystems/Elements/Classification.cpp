/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/Classification.h"
#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationTable.h"
#include "PublicApi/ClassificationSystem.h"

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(Classification)


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
    Dgn::DgnElementId elemid;
    SetUserLabel(name);
    SetDescription(description);
    if(group != nullptr) 
        {
        elemid = group->GetElementId();
        }
    else 
        {
        elemid = table.GetElementId();
        }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas                04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationPtr Classification::GetOrCreateBySystemTableNames
(
    Dgn::DgnDbR db,
    Utf8CP name,
    Utf8CP id,
    Utf8CP description,
    Utf8StringCR systemName,
    Utf8StringCR tableName
)
    {
    ClassificationSystemCPtr system = ClassificationSystem::GetOrCreateSystemByName(db, systemName);
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

    Dgn::DgnModelPtr model = GetOrCreateTableSubModel(*table);

    for (auto classificationEntry : table->MakeClassificationIterator())
        {
        ClassificationCPtr classification = Classification::Get(db, classificationEntry.GetElementId());

        if (classification.IsValid())
            return const_cast<Classification*>(classification.get());
        }

    return Classification::CreateAndInsert(*table, name, id, description, nullptr, nullptr);
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
    SetParentId(specializationId, GetDgnDb().Schemas().GetClassId(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ClassificationSpecializesClassification));
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
