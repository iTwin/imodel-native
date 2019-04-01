/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ClassificationTable.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/ClassificationTable.h"
#include "PublicApi/ClassificationSystem.h"

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationTable)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
ClassificationTable::ClassificationTable
(
    CreateParams const& params, 
    Dgn::DgnElementId systemId,
    Utf8CP name
) : ClassificationTable(params)
    {
    SetUserLabel(name);
    SetSystemId(systemId);
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

    return new ClassificationTable(params, system.GetElementId(), name);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
void ClassificationTable::SetSystemId
(
    Dgn::DgnElementId systemId
)
    {
    SetParentId(systemId, GetDgnDb().Schemas().GetClassId(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_ClassificationSystemContainsClassificationTable));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnElementId ClassificationTable::GetSystemId() const
    {
    return GetParentId();
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
