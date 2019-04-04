/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/Classification.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/Classification.h"
#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationTable.h"

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

    Dgn::DgnModelPtr model = table.GetSubModel();
    if (model.IsNull())
        model = Dgn::DefinitionModel::CreateAndInsert(table);

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
