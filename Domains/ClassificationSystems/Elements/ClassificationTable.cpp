/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ClassificationTable.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/ClassificationTable.h"
#include <BuildingShared/BuildingSharedApi.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationTable)

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               03/2019
//---------------+---------------+---------------+---------------+---------------+------
ClassificationTablePtr ClassificationTable::Create
(
    Dgn::DgnDbR db
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, Building::Shared::BuildingUtils::GetOrCreateDefinitionModel(db, "ClassificationSystems")->GetModelId(), classId);
    ClassificationTablePtr table = new ClassificationTable(params);
    return table;
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
