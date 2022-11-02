/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/SchemaMerger.h>
#include <ECObjects/SchemaConflictHelper.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

void DumpSchemasToFile(bvector<ECSchemaCP> const& schemas, Utf8CP directory, Utf8CP subdir)
    {
    for (auto schema: schemas)
        {
        if(schema == nullptr)
            continue;

        BeFileName fileName;
        fileName.AppendUtf8(directory);
        WString wSubdir(subdir, BentleyCharEncoding::Utf8);
        fileName.AppendToPath(wSubdir.c_str());
        fileName.AppendSeparator();
        BeFileName::CreateNewDirectory(fileName.c_str()); // create the directory if it doesn't exist.
        fileName.AppendUtf8(schema->GetName().c_str());
        fileName.append(L".ecschema.xml");
        if(fileName.DoesPathExist())
            fileName.BeDeleteFile();
        schema->WriteToXmlFile(fileName.c_str());
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECSchemaCP SchemaMerger::FindSchemaByName(bvector<ECSchemaCP> const& schemas, Utf8CP schemaName)
    {
    for(auto schema : schemas)
        {
        if(schema->GetName().EqualsI(schemaName))
            return schema;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
template <typename T, typename TSetter, typename TParent> //TSetter may differ from T, being the const or reference version of T
BentleyStatus SchemaMerger::MergePrimitive(PrimitiveChange<T>& change, TParent* parent, ECObjectsStatus(TParent::*setPrimitive)(TSetter), Utf8CP parentKey, SchemaMergeResult& result, SchemaMergeOptions const& options, bool preferLeftValue)
    {
    if(!change.IsChanged())
        return BentleyStatus::SUCCESS;

    auto opCode = change.GetOpCode();
    if(opCode == ECChange::OpCode::Deleted)
        return BentleyStatus::SUCCESS;

    //if we prefer left values, and there is a valid old value, we keep it.
    if(preferLeftValue && opCode == ECChange::OpCode::Modified && change.GetOld().IsValid())
        return BentleyStatus::SUCCESS;

    auto newValue = change.GetNew();

    if(!newValue.IsValid())
        { //IsValid() implicitly also checks IsNull()
        result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "Changed %s has an invalid value on item %s.", change.GetChangeName(), parentKey);
        return BentleyStatus::ERROR;
        }

    if ((parent->*setPrimitive)(newValue.Value()) != ECObjectsStatus::Success)
        {
        result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "The setter for %s on item %s returned an error.", change.GetChangeName(), parentKey);
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

void SchemaMerger::MergeCustomAttributes(SchemaMergeResult& result, CustomAttributeChanges& changes, Utf8CP scopeDescription, IECCustomAttributeContainerP left, IECCustomAttributeContainerCP right)
    {
    if(!changes.IsChanged() || changes.IsEmpty())
        return;

    for (auto change : changes)
        {
        if (!change->IsChanged())
            continue;

        auto opCode = change->GetOpCode();
        Utf8CP customAttributeName = change->GetChangeName();
        if (opCode == ECChange::OpCode::Deleted)
            continue;
        else if (opCode == ECChange::OpCode::Modified)
            {
            result.Issues().ReportV(IssueSeverity::Warning, IssueCategory::BusinessProperties, IssueType::ECSchema, "Custom Attribute %s on %s has been modified. This is currently unsupported and will be ignored",
                                    customAttributeName, scopeDescription);
            continue;
            }
        // New
        Utf8String className;
        Utf8String schemaName;
        ECClass::ParseClassName(schemaName, className, customAttributeName);
        auto inst = right->GetCustomAttribute(className);
        ECSchemaCR caSchema = inst->GetClass().GetSchema();
        ECSchemaP leftContainerSchema = left->GetContainerSchema();
        if(leftContainerSchema == nullptr)
            {
            result.Issues().ReportV(IssueSeverity::Warning, IssueCategory::BusinessProperties, IssueType::ECSchema, "Custom Attribute %s on %s cannot be added. The left container schema is null.",
                    customAttributeName, scopeDescription);
            continue;
            }
        ECSchemaP mergedCASchema = leftContainerSchema;
        if(!leftContainerSchema->GetName().EqualsI(caSchema.GetName().c_str())) //if the ca is not defined inside this current schema
            {
            //Take the CA schema from the merged schemas, make sure it's referenced
            mergedCASchema = result.GetSchema(schemaName.c_str());
            if(mergedCASchema == nullptr)
                {
                result.Issues().ReportV(IssueSeverity::Warning, IssueCategory::BusinessProperties, IssueType::ECSchema, "Custom Attribute %s on %s cannot be added. The CA schema was not found in the merged schemas.",
                        customAttributeName, scopeDescription);
                continue;
                }
            if (!ECSchema::IsSchemaReferenced(*leftContainerSchema, *mergedCASchema))
                {
                leftContainerSchema->AddReferencedSchema(*mergedCASchema);
                }
            }
        
        IECInstancePtr copiedCA = inst->CreateCopyThroughSerialization(*mergedCASchema);
        if (!copiedCA.IsValid())
            {
            result.Issues().ReportV(IssueSeverity::Warning, IssueCategory::BusinessProperties, IssueType::ECSchema, "Failed to copy new custom attribute %s on %s",
                                    customAttributeName, scopeDescription);
            continue;
            }
        left->SetCustomAttribute(*copiedCA);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
template <typename T>
BentleyStatus SchemaMerger::MergeReferencedSchemaItem(SchemaMergeResult& result, StringChange& change, SchemaItemSetterFunc<T> setterFunc, SchemaItemGetterFunc<T> getterFunc, Utf8CP parentKey, SchemaMergeOptions const& options)
    {
    if(!change.IsChanged())
        return BentleyStatus::SUCCESS;

    auto opCode = change.GetOpCode();
    if(opCode == ECChange::OpCode::New || opCode == ECChange::OpCode::Modified)
      {
      auto& newValue = change.GetNew();
      if(newValue.IsNull())
          {
          result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Changed referenced item %s has a null value on item %s.", change.GetChangeName(), parentKey);
          return BentleyStatus::ERROR;
          }

      if(!newValue.IsValid())
          {
          result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "Changed referenced item %s has an invalid value on item %s.", change.GetChangeName(), parentKey);
          return BentleyStatus::ERROR;
          }

      //in schema xml we have the "[alias]:Name"
      //in the Change API the new value refers to the full name "SchemaName:Name" instead.
      Utf8String schemaName;
      Utf8String name;
      SchemaParseUtils::ParseName(schemaName, name, newValue.Value());
      ECSchemaP schema = result.GetSchema(schemaName.c_str());
      if(schema == nullptr)
          {
          result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unable to find Schema '%s' for obtaining %s. Item: %s", schemaName.c_str(), newValue.Value().c_str(), parentKey);
          return BentleyStatus::ERROR;
          }

      if(setterFunc(getterFunc(schema, name)) != ECObjectsStatus::Success)
          {
          result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "Setting %s on %s failed. Was trying to set to %s.", change.GetChangeName(), parentKey, newValue.Value().c_str());
          return BentleyStatus::ERROR;
          }
      }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaMergeResult::ContainsSchema(Utf8CP schemaName) const
    {
    return this->GetSchema(schemaName) != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECSchemaP SchemaMergeResult::GetSchema(Utf8CP schemaName) const
    {
    SchemaKey schemaKey(schemaName, 1, 0, 0);
    return m_schemaCache.GetSchema(schemaKey, SchemaMatchType::Latest);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeSchemas(SchemaMergeResult& result, bvector<ECSchemaCP> const& rawLeft, bvector<ECSchemaCP> const& rawRight, SchemaMergeOptions const& options)
    {
    //Make a copy of the input vectors so we don't modify the original (given to us as const anyways)
    bvector<ECSchemaCP> left(rawLeft);
    bvector<ECSchemaCP> right(rawRight);
    ECSchema::SortSchemasInDependencyOrder(left);
    ECSchema::SortSchemasInDependencyOrder(right);
    bool dumpSchemas = false;
    auto dumpLocation = options.GetDumpSchemaLocation();

    if(options.GetDumpSchemas() && !dumpLocation.empty())
        {
        dumpSchemas = true;
        DumpSchemasToFile(left, dumpLocation.c_str(), "Left");
        DumpSchemasToFile(right, dumpLocation.c_str(), "Right");
        }

    bool failedToFillSchemas = false;
    auto fillSchemasToResult = [&](Utf8CP side, bvector<ECSchemaCP>& input)
        {
        for(auto schema: input)
            {
            if(!result.ContainsSchema(schema->GetName().c_str()))
                {
                ECSchemaPtr copiedSchema;
                if(schema->CopySchema(copiedSchema, &result.GetSchemaCache()) != ECObjectsStatus::Success)
                    {
                    result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Schema '%s' from %s side failed to be copied.", schema->GetFullSchemaName().c_str(), side);
                    failedToFillSchemas = true;
                    }
                copiedSchema->SetOriginalECXmlVersion(schema->GetOriginalECXmlVersionMajor(), schema->GetOriginalECXmlVersionMinor());
                result.GetSchemaCache().AddSchema(*copiedSchema);
                }
            }
        };

    bool mergeOnlyDynamicSchemas = options.GetMergeOnlyDynamicSchemas();
    bool skipStandardSchemas = options.GetSkipStandardSchemas();
    ShouldMergeSchemaFunc shouldMergeSchema = [&](ECSchemaCP schema)
        {
        if(mergeOnlyDynamicSchemas && !schema->IsDynamicSchema())
            return false;

        if(skipStandardSchemas && schema->IsStandardSchema())
            return false;

        return true;
        };

    fillSchemasToResult("left", left);
    if(failedToFillSchemas)
        return BentleyStatus::ERROR;

    if (rawRight.empty())
        return BentleyStatus::SUCCESS;

    fillSchemasToResult("right", right);
    if(failedToFillSchemas)
        return BentleyStatus::ERROR;

    SchemaComparer comparer;
    SchemaComparer::Options comparerOptions = SchemaComparer::Options(SchemaComparer::DetailLevel::NoSchemaElements, SchemaComparer::DetailLevel::NoSchemaElements);
    SchemaDiff diff;
    if (comparer.Compare(diff, result.GetResults(), right, comparerOptions) != BentleyStatus::SUCCESS)
        {
        result.Issues().Report(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "SchemaComparer comparison failed.");
        return BentleyStatus::ERROR;
        }

    for (auto schemaChange : diff.Changes())
        {
        if (!schemaChange->IsChanged())
            continue;

        auto opCode = schemaChange->GetOpCode();
        if (opCode == ECChange::OpCode::Deleted || opCode == ECChange::OpCode::New)
            continue; // skip schemas missing on one side as this was already handled above

        Utf8CP schemaName = schemaChange->GetChangeName(); // naming not intuitive, but the most reliable way to extract the schema name
        ECSchemaP leftSchema = result.GetSchema(schemaName);

        if(!shouldMergeSchema(leftSchema))
            {
            result.Issues().ReportV(IssueSeverity::Info, IssueCategory::BusinessProperties, IssueType::ECSchema, "Merger is skipping schema %s due to configuration.", 
                leftSchema->GetName().c_str());
            continue;
            }

        ECSchemaCP rightSchema = FindSchemaByName(right, schemaName);
        if(MergeSchema(result, leftSchema, rightSchema, schemaChange, options) != BentleyStatus::SUCCESS)
            {
            return BentleyStatus::ERROR;
            }

        result.m_modifiedSchemas.push_back(leftSchema);
        }

    if(dumpSchemas)
        {
        DumpSchemasToFile(result.GetResults(), dumpLocation.c_str(), "Result");
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
template <typename TItemChange, typename TItem>
BentleyStatus SchemaMerger::MergeItems(SchemaMergeResult& result, ECSchemaP left, ECSchemaCP right, SchemaMergeOptions const& options, ECChangeArray<TItemChange>& changes,
    TGetItemCP<TItem> getItemCP, TGetItemP<TItem> getItemP, TCopyItem<TItem> copyItem, TMergeItem<TItemChange, TItem> mergeItem)
    {
    for (const auto& schemaItemChange : changes)
        {
        if (!schemaItemChange->IsChanged())
            continue;

        const auto opCode = schemaItemChange->GetOpCode();
        if (opCode == ECChange::OpCode::Deleted)
            continue;
        
        Utf8CP itemName = schemaItemChange->GetChangeName();
        Utf8String newName(itemName);
        if (opCode == ECChange::OpCode::New)
            { //This may actually work
            auto newSchemaItem = (right->*getItemCP)(itemName);
            if ( left->NamedElementExists(itemName) && 
                ((left->*getItemP)(itemName) == nullptr))
                { // An item of another type exists with the same name
                if(!options.GetRenameSchemaItemOnConflict())
                    {
                    result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Another item with name %s already exists in the merged schema %s. RenameSchemaItemOnConflict is set to false.",
                        newSchemaItem->GetFullName().c_str(), left->GetFullSchemaName().c_str());
                    return BentleyStatus::ERROR;
                    }

                newName = left->FindUniqueSchemaItemName(itemName);
                }
            
            TItem* createdSchemaItem;
            ECObjectsStatus status = (left->*copyItem)(createdSchemaItem, *newSchemaItem, true, newName.c_str());
            if (status != ECObjectsStatus::Success && status != ECObjectsStatus::NamedItemAlreadyExists)
                {
                result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Failed to copy %s into merged schema.", newSchemaItem->GetFullName().c_str());
                return BentleyStatus::ERROR;
                }
                
            continue;
            }

        auto leftItem = (left->*getItemP)(itemName);
        auto rightItem = (right->*getItemCP)(itemName);
        if((*mergeItem)(result, leftItem, rightItem, schemaItemChange, options) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeSchema(SchemaMergeResult& result, ECSchemaP left, ECSchemaCP right, RefCountedPtr<SchemaChange> schemaChange, SchemaMergeOptions const& options)
    {
    if(MergePrimitive(schemaChange->Alias(), left, &ECSchema::SetAlias, left->GetName().c_str(), result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if(schemaChange->VersionRead().IsChanged() || schemaChange->VersionWrite().IsChanged() || schemaChange->VersionMinor().IsChanged())
        { //we are not merging versions like other properties. The highest version always wins and is applied to left
          if(!options.GetKeepVersion() && left->GetSchemaKey().LessThan(right->GetSchemaKey(), SchemaMatchType::Exact))
              {
              left->SetVersionRead(right->GetVersionRead());
              left->SetVersionWrite(right->GetVersionWrite());
              left->SetVersionMinor(right->GetVersionMinor());
              }
        }

    if(schemaChange->OriginalECXmlVersionMajor().IsChanged() || schemaChange->OriginalECXmlVersionMinor().IsChanged())
        { //Apply the highest version being used
        auto leftMajor = left->GetOriginalECXmlVersionMajor();
        auto leftMinor = left->GetOriginalECXmlVersionMinor();
        auto rightMajor = right->GetOriginalECXmlVersionMajor();
        auto rightMinor = right->GetOriginalECXmlVersionMinor();
        if(leftMajor < rightMajor)
            left->SetOriginalECXmlVersion(rightMajor, rightMinor);
        else if(leftMajor == rightMajor && leftMinor < rightMinor)
            left->SetOriginalECXmlVersion(rightMajor, rightMinor);
        }

    if (MergePrimitive(schemaChange->Description(), left, &ECSchema::SetDescription, left->GetName().c_str(), result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(schemaChange->DisplayLabel(), left, &ECSchema::SetDisplayLabel, left->GetName().c_str(), result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if(schemaChange->References().IsChanged())
        {
        for (auto referenceChange : schemaChange->References())
            {
            if (referenceChange->GetOpCode() != ECChange::OpCode::New)
                continue; // we are only interested in new references when merging

            auto& referenceFullName = referenceChange->GetNew().Value();
            SchemaKey newRef;
            SchemaKey::ParseSchemaFullName(newRef, referenceFullName.c_str());
            ECSchemaP newReferencedSchema = result.GetSchema(newRef.GetName().c_str());
            if(newReferencedSchema == nullptr)
                {
                result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Failed to find new referenced schema %s for schema %s", referenceFullName.c_str(), left->GetFullSchemaName().c_str());
                return BentleyStatus::ERROR;
                }
            left->AddReferencedSchema(*newReferencedSchema);
            }
        }

    if (MergeItems(result, left, right, options, schemaChange->Enumerations(), &ECSchema::GetEnumerationCP, &ECSchema::GetEnumerationP, &ECSchema::CopyEnumeration, &SchemaMerger::MergeEnumeration) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergeItems(result, left, right, options, schemaChange->PropertyCategories(), &ECSchema::GetPropertyCategoryCP, &ECSchema::GetPropertyCategoryP, &ECSchema::CopyPropertyCategory, &SchemaMerger::MergePropertyCategory) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergeItems(result, left, right, options, schemaChange->Phenomena(), &ECSchema::GetPhenomenonCP, &ECSchema::GetPhenomenonP, &ECSchema::CopyPhenomenon, &SchemaMerger::MergePhenomenon) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergeItems(result, left, right, options, schemaChange->UnitSystems(), &ECSchema::GetUnitSystemCP, &ECSchema::GetUnitSystemP, &ECSchema::CopyUnitSystem, &SchemaMerger::MergeUnitSystem) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergeItems(result, left, right, options, schemaChange->Units(), &ECSchema::GetUnitCP, &ECSchema::GetUnitP, &ECSchema::CopyUnit, &SchemaMerger::MergeUnit) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergeItems(result, left, right, options, schemaChange->Formats(), &ECSchema::GetFormatCP, &ECSchema::GetFormatP, &ECSchema::CopyFormat, &SchemaMerger::MergeFormat) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergeItems(result, left, right, options, schemaChange->KindOfQuantities(), &ECSchema::GetKindOfQuantityCP, &ECSchema::GetKindOfQuantityP, &ECSchema::CopyKindOfQuantity, &SchemaMerger::MergeKindOfQuantity) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    //First process New classes, then the rest
    for(auto classChange : schemaChange->Classes())
        {
        if (!classChange->IsChanged() || classChange->GetOpCode() != ECChange::OpCode::New)
            continue;
        
        Utf8String className(classChange->GetChangeName());
        auto newClass = right->GetClassCP(className.c_str());
        if ( left->NamedElementExists(className.c_str()) && 
            (left->GetClassCP(className.c_str()) == nullptr))
            { // An item of another type exists with the same name
            if(!options.GetRenameSchemaItemOnConflict())
                {
                result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Another item with name %s already exists in the merged schema %s. RenameSchemaItemOnConflict is set to false.",
                    newClass->GetFullName(), left->GetFullSchemaName().c_str());
                return BentleyStatus::ERROR;
                }


            className = left->FindUniqueSchemaItemName(className.c_str());
            }

        ECClassP createdClass;
        ECObjectsStatus status = left->CopyClass(createdClass, *newClass, true, className.c_str());
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
          {
          result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Failed to copy class %s into merged schema", newClass->GetFullName());
          return BentleyStatus::ERROR;
          }
        }

    for(auto classChange : schemaChange->Classes())
        {
        if (!classChange->IsChanged() || classChange->GetOpCode() != ECChange::OpCode::Modified)
            continue;
        
        Utf8CP className = classChange->GetChangeName();
        auto leftClass = left->GetClassP(className);
        auto rightClass = right->GetClassCP(className);
        if(MergeClass(result, leftClass, rightClass, classChange, options) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    Utf8PrintfString scopeDescription("Schema %s", left->GetName().c_str());
    MergeCustomAttributes(result, schemaChange->CustomAttributes(), scopeDescription.c_str(), left, right);

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeRelationshipConstraint(SchemaMergeResult& result, ECRelationshipClassP left, ECClassCP right, RelationshipConstraintChange& change, SchemaMergeOptions const& options, bool isSource)
    {
    auto& constraint = isSource ? left->GetSource() : left->GetTarget();
    if (MergePrimitive(change.RoleLabel(), &constraint, &ECRelationshipConstraint::SetRoleLabel, left->GetFullName(), result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    //necessary as the many method overloads make deduction not work
    // int (Foo::*mpf)(int) = &Foo::mf; // selects int mf(int)
    ECObjectsStatus (ECRelationshipConstraint::*setMultiplicityPointer)(Utf8StringCR) = &ECRelationshipConstraint::SetMultiplicity;
    if (MergePrimitive(change.Multiplicity(), &constraint, setMultiplicityPointer, left->GetFullName(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(change.IsPolymorphic(), &constraint, &ECRelationshipConstraint::SetIsPolymorphic, left->GetFullName(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if (MergeReferencedSchemaItem<ECClassCP>(result, change.AbstractConstraint(),
      [&](ECClassCP value) { return constraint.SetAbstractConstraint(*value); },
      [&](ECSchemaP schema, Utf8StringCR name) { return schema->GetClassCP(name.c_str()); }, left->GetFullName(), options) != BentleyStatus::SUCCESS)
      return BentleyStatus::ERROR;

    auto& constraintClassChanges = change.ConstraintClasses();
    if(!constraintClassChanges.IsChanged())
        return BentleyStatus::SUCCESS;

    for(auto constraintClassChange : constraintClassChanges)
        {
        if(!constraintClassChange->IsChanged())
            continue;

        auto opCode = constraintClassChange->GetOpCode();
        if (opCode == ECChange::OpCode::Deleted)
            continue;

        if (MergeReferencedSchemaItem<ECClassCP>(result, *constraintClassChange,
            [&](ECClassCP value) {
                if(!constraint.SupportsClass(*value))
                    return ECObjectsStatus::Error;
                
                return constraint.AddClass(*value);
                },
            [&](ECSchemaP schema, Utf8StringCR name) { return schema->GetClassCP(name.c_str()); }, left->GetFullName(), options) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }
    
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeClass(SchemaMergeResult& result, ECClassP left, ECClassCP right, RefCountedPtr<ClassChange> classChange, SchemaMergeOptions const& options)
    {
    if (MergePrimitive(classChange->DisplayLabel(), left, &ECClass::SetDisplayLabel, left->GetFullName(), result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(classChange->Description(), left, &ECClass::SetDescription, left->GetFullName(), result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(classChange->ClassModifier(), left, &ECClass::SetClassModifier, left->GetFullName(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    // check if the classToMerge has the same type
    auto classType = left->GetClassType();
    if (classType != right->GetClassType())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Cannot merge class %s because the type of class is different.", left->GetFullName());
        return BentleyStatus::ERROR;
        }

    switch (left->GetClassType())
        {
        case ECClassType::Relationship:
            {
            auto mergedRelationshipClass = left->GetRelationshipClassP();
            if (MergePrimitive(classChange->Strength(), mergedRelationshipClass, &ECRelationshipClass::SetStrength, left->GetFullName(), result, options, false) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            if (MergePrimitive(classChange->StrengthDirection(), mergedRelationshipClass, &ECRelationshipClass::SetStrengthDirection, left->GetFullName(), result, options, false) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;

            if (classChange->Source().IsChanged())
                {
                if (MergeRelationshipConstraint(result, mergedRelationshipClass, right, classChange->Source(), options, true) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            if (classChange->Target().IsChanged())
                {
                if (MergeRelationshipConstraint(result, mergedRelationshipClass, right, classChange->Target(), options, false) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }

            break;
            }
        case ECClassType::CustomAttribute:
            //TODO: This Container Type is currently not handled by ECSchemaComparer, these have 1 unique property: CustomAttributeContainerType
            break;
        // struct and entity have no special unique fields to set
        }

    for(auto baseClassChange : classChange->BaseClasses())
        {
        if(!baseClassChange->IsChanged())
            continue;

        auto opCode = baseClassChange->GetOpCode();
        if (opCode == ECChange::OpCode::Deleted)
            continue;

        if (opCode == ECChange::OpCode::Modified)
            {
            // SchemaName:ItemName
            Utf8String oldFullName = baseClassChange->GetOld().Value();
            Utf8String schemaName;
            Utf8String name;
            SchemaParseUtils::ParseName(schemaName, name, oldFullName);
            ECSchemaP schema = result.GetSchema(schemaName.c_str());
            if (schema == nullptr)
                {
                result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unable to find schema which holds modified base class '%s' to remove from '%s'.", oldFullName.c_str(), left->GetFullName());
                return BentleyStatus::ERROR;
                }

            ECClassCP baseClassToRemove = schema->GetClassCP(name.c_str());
            if (baseClassToRemove == nullptr)
                {
                result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unable to find modified base class '%s' to remove from '%s'.", oldFullName.c_str(), left->GetFullName());
                return BentleyStatus::ERROR;
                }

            if (left->RemoveBaseClass(*baseClassToRemove) != ECObjectsStatus::Success)
                {
                result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Removing Base Class '%s' from '%s' failed.", oldFullName.c_str(), left->GetFullName());
                return BentleyStatus::ERROR;
                }
            }

        // opCode is Modified or New, so apply the "new" base class
            { // "New base class"-block
            auto& newValue = baseClassChange->GetNew();
            if(newValue.IsNull() || !newValue.IsValid())
                {
                result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Changed base class on item %s has a null or invalid value.", left->GetFullName());
                return BentleyStatus::ERROR;
                }

            //in schema xml we have the "[alias]:Name"
            //in the Change API the new value refers to the full name "SchemaName:Name" instead.
            Utf8String schemaName;
            Utf8String name;
            SchemaParseUtils::ParseName(schemaName, name, newValue.Value());
            ECSchemaP schema = result.GetSchema(schemaName.c_str());
            if(schema == nullptr)
                {
                result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unable to find Schema '%s'. For adding base class to: %s", schemaName.c_str(), left->GetFullName());
                return BentleyStatus::ERROR;
                }

            auto newBaseClass = schema->GetClassCP(name.c_str());
            if(newBaseClass == nullptr)
                {
                result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unable to find Class '%s' in schema %s. For use as base class on: %s", name.c_str(), schemaName.c_str(), left->GetFullName());
                return BentleyStatus::ERROR;
                }

            if(!SchemaConflictHelper::CanBaseClassBeAdded(*left, *newBaseClass))
                {
                result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "New base class %s is incompatible with properties on %s or its derived classes.", newBaseClass->GetFullName(), left->GetFullName());
                return BentleyStatus::ERROR;
                }

            if(left->AddBaseClass(*newBaseClass) != ECObjectsStatus::Success)
                {
                result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "AddBaseClass for base class %s returned an error on %s.", newBaseClass->GetFullName(), left->GetFullName());
                return BentleyStatus::ERROR;
                }
            }
        }

    for(auto propertyChange : classChange->Properties())
        {
        if(!propertyChange->IsChanged())
            continue;

        auto opCode = propertyChange->GetOpCode();
        if (opCode == ECChange::OpCode::Deleted)
            continue;
        
        Utf8CP propertyName = propertyChange->GetChangeName();
        Utf8String validName(propertyName);
        bool renameProperty = false;
        auto rightProperty = right->GetPropertyP(propertyName);
        if (opCode == ECChange::OpCode::New)
            {
            if(!SchemaConflictHelper::CanPropertyBeAdded(*left, *rightProperty))
                { //conflict
                if(!options.GetRenamePropertyOnConflict())
                    {
                    result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Failed to add property %s to class %s because it conflicts with another property. RenamePropertyOnConflict flag is set to false.",
                        propertyName ,left->GetFullName());
                    return BentleyStatus::ERROR;
                    }

                //rename property
                renameProperty = true;
                if(SchemaConflictHelper::FindUniquePropertyName(*left, validName) != BentleyStatus::SUCCESS)
                    {
                    result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Failed to find a valid new name for property %s to class %s. It conflicts with another property. RenamePropertyOnConflict flag is set to true.",
                        propertyName ,left->GetFullName());
                    return BentleyStatus::ERROR;
                    }
                }

            ECPropertyP createdProperty;
            if (ECObjectsStatus::Success != left->CopyProperty(createdProperty, rightProperty, validName.c_str(), true))
                {
                result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Failed to copy property %s on class %s into merged schema", propertyName ,left->GetFullName());
                return BentleyStatus::ERROR;
                }

            if(renameProperty)
                {
                left->AddPropertyMapping(rightProperty->GetName().c_str(), validName.c_str());
                }
              
            continue;
            }

        auto leftProperty = left->GetPropertyP(propertyName);
        if(MergeProperty(result, leftProperty, rightProperty, propertyChange, options) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    Utf8PrintfString scopeDescription("Class %s", left->GetName().c_str());
    MergeCustomAttributes(result, classChange->CustomAttributes(), scopeDescription.c_str(), left, right);
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeProperty(SchemaMergeResult& result, ECPropertyP left, ECPropertyCP right, RefCountedPtr<PropertyChange> propertyChange, SchemaMergeOptions const& options)
    {
    Utf8PrintfString key("%s:%s", left->GetClass().GetFullName(), left->GetName().c_str());
    if (MergePrimitive(propertyChange->DisplayLabel(), left, &ECProperty::SetDisplayLabel, key.c_str(), result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(propertyChange->Description(), left, &ECProperty::SetDescription, key.c_str(), result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(propertyChange->IsReadonly(), left, &ECProperty::SetIsReadOnly, key.c_str(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(propertyChange->Priority(), left, &ECProperty::SetPriority, key.c_str(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(propertyChange->MinimumLength(), left, &ECProperty::SetMinimumLength, key.c_str(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(propertyChange->MaximumLength(), left, &ECProperty::SetMaximumLength, key.c_str(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(propertyChange->MinimumValue(), left, &ECProperty::SetMinimumValue, key.c_str(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(propertyChange->MaximumValue(), left, &ECProperty::SetMaximumValue, key.c_str(), result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if (MergeReferencedSchemaItem<PropertyCategoryCP>(result, propertyChange->Category(),
      [&](PropertyCategoryCP value) { return left->SetCategory(value); },
      [&](ECSchemaP schema, Utf8StringCR name) { return schema->GetPropertyCategoryCP(name.c_str()); }, key.c_str(), options) != BentleyStatus::SUCCESS)
      return BentleyStatus::ERROR;
    
    if (MergeReferencedSchemaItem<KindOfQuantityCP>(result, propertyChange->KindOfQuantity(),
      [&](KindOfQuantityCP value) { return left->SetKindOfQuantity(value); },
      [&](ECSchemaP schema, Utf8StringCR name) { return schema->GetKindOfQuantityCP(name.c_str()); }, key.c_str(), options) != BentleyStatus::SUCCESS)
      return BentleyStatus::ERROR;

    if (propertyChange->IsPrimitive().IsChanged() ||
        propertyChange->IsStruct().IsChanged() ||
        propertyChange->IsStructArray().IsChanged() ||
        propertyChange->IsPrimitiveArray().IsChanged() ||
        propertyChange->IsNavigation().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Property %s has mismatching types between both sides.",
            key.c_str());
        return BentleyStatus::ERROR;
        }
    
    if(propertyChange->TypeName().IsChanged() && !options.GetIgnoreIncompatiblePropertyTypeChanges())
        {
        //TODO: ExtendedTypeName, Enumeration
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Property %s has its type changed.",
            key.c_str());
        return BentleyStatus::ERROR;
        }

    Utf8PrintfString scopeDescription("Property %s", key.c_str());
    MergeCustomAttributes(result, propertyChange->CustomAttributes(), scopeDescription.c_str(), left, right);

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeKindOfQuantity(SchemaMergeResult& result, KindOfQuantityP left, KindOfQuantityCP right, RefCountedPtr<KindOfQuantityChange> change, SchemaMergeOptions const& options)
  {
    Utf8CP key = left->GetFullName().c_str();
    if (MergePrimitive(change->DisplayLabel(), left, &KindOfQuantity::SetDisplayLabel, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(change->Description(), left, &KindOfQuantity::SetDescription, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(change->RelativeError(), left, &KindOfQuantity::SetRelativeError, key, result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if (MergeReferencedSchemaItem<ECUnitCP>(result, change->PersistenceUnit(),
        [&](ECUnitCP value) { return left->SetPersistenceUnit(*value); },
        [&](ECSchemaP schema, Utf8StringCR name) { return schema->GetUnitCP(name.c_str()); }, key, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    auto& presentationFormatsChange = change->PresentationFormats();
    if(presentationFormatsChange.IsChanged())
      { //SchemaComparer compares these by index and returns modified or deleted/new based on which side has more items
      //This isn't particularly useful when merging, we'll just compare both sides ourselves here
      for(auto presentationFormatChange : presentationFormatsChange)
          {
          auto opCode = presentationFormatChange->GetOpCode();
          if (opCode == ECChange::OpCode::Deleted)
              continue;
          
          bvector<Utf8String> mergedPresentationFormatStrings;
          bset<Utf8String> allPresentationFormatStrings;
          for(auto& presFormat : left->GetPresentationFormats())
              {
              auto formatString = presFormat.GetQualifiedFormatString(left->GetSchema());
              mergedPresentationFormatStrings.push_back(formatString);
              allPresentationFormatStrings.insert(formatString);
              }

          for(auto& presFormat : right->GetPresentationFormats())
              {
              auto formatString = presFormat.GetQualifiedFormatString(right->GetSchema());
              if(allPresentationFormatStrings.find(formatString) != allPresentationFormatStrings.end())
                  continue;

              mergedPresentationFormatStrings.push_back(formatString);
              allPresentationFormatStrings.insert(formatString);
              }

          left->RemoveAllPresentationFormats();

          ECSchemaCR leftSchema = left->GetSchema();
          SchemaUnitContextCR unitsContext = leftSchema.GetUnitsContext();
          const auto nameToUnitMapper = [&] (Utf8StringCR alias, Utf8StringCR name)
              {
              return unitsContext.LookupUnit((alias + ":" + name).c_str());
              };

          const auto nameToFormatMapper = [&] (Utf8StringCR alias, Utf8String name)
              {
              return leftSchema.LookupFormat((alias + ":" + name).c_str());
              };
          for(auto& presFormat : mergedPresentationFormatStrings)
              {
              if(left->AddPresentationFormatByString(presFormat, nameToFormatMapper, nameToUnitMapper) != ECObjectsStatus::Success)
                  {
                  result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "PresentationFormat %s failed to be added on kind of quantity %s.", presFormat.c_str(), left->GetFullName().c_str());
                  return BentleyStatus::ERROR;
                  }
              }
          }
      }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeEnumeration(SchemaMergeResult& result, ECEnumerationP left, ECEnumerationCP right, RefCountedPtr<EnumerationChange> change, SchemaMergeOptions const& options)
    {
    Utf8CP key = left->GetFullName().c_str();
    if (MergePrimitive(change->DisplayLabel(), left, &ECEnumeration::SetDisplayLabel, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(change->Description(), left, &ECEnumeration::SetDescription, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    
    if(change->TypeName().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Enumeration '%s' has its Type changed. This is not supported.", key);
        return BentleyStatus::ERROR;
        }

    if (MergePrimitive(change->IsStrict(), left, &ECEnumeration::SetIsStrict, key, result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    auto& enumeratorsChange = change->Enumerators();
    for(auto enumeratorChange : enumeratorsChange)
        {
        if(!enumeratorChange->IsChanged())
            continue;

        auto opCode = enumeratorChange->GetOpCode();
        if (opCode == ECChange::OpCode::Deleted)
            continue;

        Utf8CP enumeratorName = enumeratorChange->GetChangeName();
        auto rightEnumerator = right->FindEnumeratorByName(enumeratorName);
        if (opCode == ECChange::OpCode::New)
            {
            if(rightEnumerator == nullptr)
            return BentleyStatus::ERROR;

            ECEnumeratorP createdEnumerator;
            ECObjectsStatus status;
            if (rightEnumerator->IsInteger())
                status = left->CreateEnumerator(createdEnumerator, enumeratorName, rightEnumerator->GetInteger());
            else
                status = left->CreateEnumerator(createdEnumerator, enumeratorName, rightEnumerator->GetString().c_str());

            if (status != ECObjectsStatus::Success)
                {
                if(status == ECObjectsStatus::NamedItemAlreadyExists)
                    result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Enumeration '%s' ends up having duplicate enumerator values after merge, which is not allowed. Name of new Enumerator: %s", key, enumeratorName);
                else
                    result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Failed to copy Enumerator %s on Enumeration '%s'.", enumeratorName, key);

            return BentleyStatus::ERROR;
            }

            createdEnumerator->SetDescription(rightEnumerator->GetInvariantDescription().c_str());
            if(rightEnumerator->GetIsDisplayLabelDefined())
                createdEnumerator->SetDisplayLabel(rightEnumerator->GetInvariantDisplayLabel());

            continue;
            }

        auto leftEnumerator = left->FindEnumeratorByName(enumeratorName);
        if (MergePrimitive(enumeratorChange->DisplayLabel(), leftEnumerator, &ECEnumerator::SetDisplayLabel, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
        if (MergePrimitive(enumeratorChange->Description(), leftEnumerator, &ECEnumerator::SetDescription, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
        if (MergePrimitive(enumeratorChange->String(), leftEnumerator, &ECEnumerator::SetString, key, result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
        if (MergePrimitive(enumeratorChange->Integer(), leftEnumerator, &ECEnumerator::SetInteger, key, result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
        }
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergePropertyCategory(SchemaMergeResult& result, PropertyCategoryP left, PropertyCategoryCP right, RefCountedPtr<PropertyCategoryChange> change, SchemaMergeOptions const& options)
    {
    Utf8CP key = left->GetFullName().c_str();
    if (MergePrimitive(change->DisplayLabel(), left, &PropertyCategory::SetDisplayLabel, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(change->Description(), left, &PropertyCategory::SetDescription, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(change->Priority(), left, &PropertyCategory::SetPriority, key, result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergePhenomenon(SchemaMergeResult& result, PhenomenonP left, PhenomenonCP right, RefCountedPtr<PhenomenonChange> change, SchemaMergeOptions const& options)
    {
    Utf8CP key = left->GetFullName().c_str();
    if (MergePrimitive(change->DisplayLabel(), left, &Phenomenon::SetDisplayLabel, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(change->Description(), left, &Phenomenon::SetDescription, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if(change->Definition().IsChanged())
      {
      result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Phenomenon '%s' has its definition changed. This is not supported.", key);
      return BentleyStatus::ERROR;
      }
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeUnitSystem(SchemaMergeResult& result, UnitSystemP left, UnitSystemCP right, RefCountedPtr<UnitSystemChange> change, SchemaMergeOptions const& options)
    {
    Utf8CP key = left->GetFullName().c_str();
    if (MergePrimitive(change->DisplayLabel(), left, &UnitSystem::SetDisplayLabel, key, result, options) != BentleyStatus::SUCCESS)
          return BentleyStatus::ERROR;
    if (MergePrimitive(change->Description(), left, &UnitSystem::SetDescription, key, result, options) != BentleyStatus::SUCCESS)
          return BentleyStatus::ERROR;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeUnit(SchemaMergeResult& result, ECUnitP left, ECUnitCP right, RefCountedPtr<UnitChange> change, SchemaMergeOptions const& options)
    {
    Utf8CP key = left->GetFullName().c_str();
    if (MergePrimitive(change->DisplayLabel(), left, &ECUnit::SetDisplayLabel, key, result, options) != BentleyStatus::SUCCESS)
          return BentleyStatus::ERROR;
    if (MergePrimitive(change->Description(), left, &ECUnit::SetDescription, key, result, options) != BentleyStatus::SUCCESS)
          return BentleyStatus::ERROR;

    if(change->Phenomenon().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unit '%s' has its Phenomenon changed. This is not supported.", key);
        return BentleyStatus::ERROR;
        }

    if(change->UnitSystem().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unit '%s' has its UnitSystem changed. This is not supported.", key);
        return BentleyStatus::ERROR;
        }

    if (MergePrimitive(change->IsConstant(), left, &ECUnit::SetConstant, key, result, options, false) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if(change->InvertingUnit().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unit '%s' has its InvertingUnit changed. This is not supported.", key);
        return BentleyStatus::ERROR;
        }
    
    if(change->Definition().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unit '%s' has its Definition changed. This is not supported.", key);
        return BentleyStatus::ERROR;
        }

    if(change->Numerator().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unit '%s' has its Numerator changed. This is not supported.", key);
        return BentleyStatus::ERROR;
        }

    if(change->Denominator().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unit '%s' has its Denominator changed. This is not supported.", key);
        return BentleyStatus::ERROR;
        }

    if(change->Offset().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Unit '%s' has its Offset changed. This is not supported.", key);
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaMerger::MergeFormat(SchemaMergeResult& result, ECFormatP left, ECFormatCP right, RefCountedPtr<FormatChange> change, SchemaMergeOptions const& options)
    {
    Utf8CP key = left->GetFullName().c_str();
    if (MergePrimitive(change->DisplayLabel(), left, &ECFormat::SetDisplayLabel, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if (MergePrimitive(change->Description(), left, &ECFormat::SetDescription, key, result, options) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;
    if(change->NumericSpec().IsChanged())
        {
        //auto newValue = change.GetNew();
        auto newValue = right->GetNumericSpec();
        if (newValue != nullptr)
            {
            if(!left->SetNumericSpec(*newValue))
                {
                result.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, "The setter for %s on item %s returned an error.", change->NumericSpec().GetChangeName(), left->GetFullName().c_str());
                return BentleyStatus::ERROR;
                }
            }
        }

    if(change->CompositeSpec().IsChanged())
        {
        result.Issues().ReportV(IssueSeverity::Fatal, IssueCategory::BusinessProperties, IssueType::ECSchema, "Format '%s' has its CompositeSpec changed. This is not supported when merging schemas.", left->GetFullName().c_str());
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }
END_BENTLEY_ECOBJECT_NAMESPACE
