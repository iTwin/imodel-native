/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ClassificationGroup.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationSystem.h"

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
    ClassificationSystemCR system,
    Utf8CP name
)
    {
    Dgn::DgnDbR db = system.GetDgnDb();

    Dgn::DgnModelPtr model = system.GetSubModel();
    if (model.IsNull())
        model = Dgn::DefinitionModel::CreateAndInsert(system);

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
Dgn::DgnElementId ClassificationGroup::GetClassificationSystemId() const
    {
    return GetModel()->GetModeledElementId();
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
