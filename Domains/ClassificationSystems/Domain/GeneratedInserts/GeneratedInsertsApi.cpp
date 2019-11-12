/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>
#include "PublicApi/GeneratedInsertsApi.h"

namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              05/2018
//---------------------------------------------------------------------------------------
ClassificationSystemCPtr GeneratedInserts::TryAndGetSystem
(
Dgn::DgnDbR db,
Dgn::DgnModelCR model,
Utf8StringCR name,
Utf8StringCR edition
) const
    {
    //TODO Make this static in elements
    ClassificationSystemCPtr system = ClassificationSystem::TryGet(db, model, name, edition);
    if (system.IsNull())
        {
        return InsertSystem(db, model, name, edition);
        }
    else 
        return system;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas             04/2019
//---------------------------------------------------------------------------------------
ClassificationTableCPtr GeneratedInserts::TryAndGetTable
(
ClassificationSystemCR system,
Utf8CP name
) const
    {
    ClassificationTableCPtr table = nullptr;

    for (auto childId : system.QueryChildren())
        {
        table = ClassificationTable::Get(system.GetDgnDb(), childId);
        if(table.IsNull())
            continue;

        if(table->GetName() == name)
            break;
        else
            table = nullptr;
        }

    if (table.IsNull())
        return InsertTable(system, name);
    else 
        return table;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationSystemPtr GeneratedInserts::InsertSystem
(
Dgn::DgnDbR db,
Dgn::DgnModelCR model,
Utf8StringCR name,
Utf8StringCR edition
) const
    {
    ClassificationSystemPtr classSystem = ClassificationSystem::Create(db, model, name, edition);
    classSystem->Insert();
    return classSystem;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas             04/2019
//---------------------------------------------------------------------------------------
ClassificationTablePtr GeneratedInserts::InsertTable
(
    ClassificationSystemCR system,
    Utf8CP name
) const
    {
    ClassificationTablePtr table = ClassificationTable::Create(system, name);
    table->Insert();
    return table;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationGroupPtr GeneratedInserts::InsertGroup
(
ClassificationTableCR table,
Utf8CP name
) const
    {
    ClassificationGroupPtr classDefinitionGroup = ClassificationGroup::Create(table, name);
    classDefinitionGroup->Insert();
    return classDefinitionGroup;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationPtr GeneratedInserts::InsertClassification
(
    ClassificationTableCR table, 
    Utf8CP name, 
    Utf8CP id, 
    Utf8CP description, 
    ClassificationGroupCP group, 
    ClassificationCP specializes
) const
    {
    ClassificationPtr classification = Classification::CreateAndInsert(table, name, id, description, group, specializes);
    return classification;
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE