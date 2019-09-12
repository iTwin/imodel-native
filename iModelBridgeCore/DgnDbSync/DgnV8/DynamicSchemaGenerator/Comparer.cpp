#include "Comparer.h"
#include <ECObjects/ECObjectsAPI.h>
#include <ECObjects/SchemaComparer.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

bool InternalComparer::IsChanged(ECN::ECSchemaCP existing, ECN::ECSchemaCP incoming)
    {
    ECN::SchemaComparer comparer;
    //We do not require detail if schema is added or deleted. the name and version suffices.
    ECN::SchemaComparer::Options options = ECN::SchemaComparer::Options(ECN::SchemaComparer::DetailLevel::NoSchemaElements, ECN::SchemaComparer::DetailLevel::NoSchemaElements);
    bvector<ECN::ECSchemaCP> existingSet = {existing};
    bvector<ECN::ECSchemaCP> newSet = {incoming};
    ECN::SchemaDiff diff;
    if (SUCCESS == comparer.Compare(diff, existingSet, newSet, options))
        {
        for (auto& change : diff.Changes())
            {
            if (change->CustomAttributes().IsChanged() || change->References().IsChanged() || change->Classes().IsChanged() || change->Enumerations().IsChanged() ||
                change->KindOfQuantities().IsChanged() || change->PropertyCategories().IsChanged() || change->Phenomena().IsChanged() || change->UnitSystems().IsChanged() ||
                change->Units().IsChanged() || change->Formats().IsChanged())
                return true;
            }
        //if (diff.Changes().IsChanged())
        //    {
        //    return true;
        //    }
        }
    return false;
    }
END_DGNDBSYNC_DGNV8_NAMESPACE

