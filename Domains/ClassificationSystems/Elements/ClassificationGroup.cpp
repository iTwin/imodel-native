/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationTable.h"

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationGroup)

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
    ClassificationTableCR table,
    Utf8CP name
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
    ClassificationGroupPtr group = new ClassificationGroup(params, name);
    return group;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementId ClassificationGroup::GetClassificationTableId() const
    {
    return GetModel()->GetModeledElementId();
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
