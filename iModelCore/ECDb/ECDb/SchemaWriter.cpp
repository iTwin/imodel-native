/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaWriter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
BentleyStatus SchemaWriter::ImportSchemas(bvector<ECN::ECSchemaCP>& schemasToMap, ECDbCR ecdb, SchemaImportContext& schemaImportCtx, bvector<ECSchemaCP> const& primarySchemasOrderedByDependencies)
    {
    PERFLOG_START("ECDb", "Schema import> Persist schemas");

    Context ctx(ecdb, schemaImportCtx);
    if (SUCCESS != ValidateSchemasPreImport(ctx, primarySchemasOrderedByDependencies))
        return ERROR;

    if (SUCCESS != CompareSchemas(ctx, primarySchemasOrderedByDependencies))
        return ERROR;

    if (ctx.GetSchemasToImport().empty())
        return SUCCESS;

    for (ECSchemaCP schema : ctx.GetSchemasToImport())
        {
        if (SUCCESS != ImportSchema(ctx, *schema))
            return ERROR;
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(ctx.GetECDb()))
        return ERROR;

    if (SUCCESS != ReloadSchemas(ctx))
        return ERROR;

    schemasToMap.insert(schemasToMap.begin(), ctx.GetSchemasToImport().begin(), ctx.GetSchemasToImport().end());
    PERFLOG_FINISH("ECDb", "Schema import> Persist schemas");
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                     05/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ValidateSchemasPreImport(Context const& ctx, bvector<ECSchemaCP> const& primarySchemasOrderedByDependencies)
    {
    const bool isValid = SchemaValidator::ValidateSchemas(ctx.ImportCtx(), ctx.Issues(), primarySchemasOrderedByDependencies);
    return isValid ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportSchema(Context& ctx, ECN::ECSchemaCR ecSchema)
    {
    SchemaChange* schemaChange = ctx.Changes().Find(ecSchema.GetName().c_str());
    if (schemaChange != nullptr)
        {
        if (schemaChange->GetStatus() == ECChange::Status::Done)
            return SUCCESS;

        if (schemaChange->GetState() == ChangeState::Modified)
            {
            ECSchemaCP existingSchema = nullptr;
            for (ECSchemaCP schema : ctx.GetExistingSchemas())
                {
                if (schema->GetName().Equals(schemaChange->GetId()))
                    {
                    existingSchema = schema;
                    break;
                    }
                }

            BeAssert(existingSchema != nullptr);
            if (existingSchema == nullptr)
                return ERROR;

            return UpdateSchema(ctx, *schemaChange, *existingSchema, ecSchema);
            }

        if (schemaChange->GetState() == ChangeState::Deleted)
            {
            schemaChange->SetStatus(ECChange::Status::Done);
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting an ECSchema is not supported.",
                                 ecSchema.GetName().c_str());
            return ERROR;
            }
        }

    if (ctx.GetSchemaManager().ContainsSchema(ecSchema.GetName()))
        return SUCCESS;

    if (SUCCESS != InsertSchemaEntry(ctx, ecSchema))
        {
        DbResult lastErrorCode;
        ctx.GetECDb().GetLastError(&lastErrorCode);
        if (BE_SQLITE_CONSTRAINT_UNIQUE == lastErrorCode)
            ctx.Issues().ReportV("Failed to import ECSchema '%s'. Alias '%s' is already used by an existing ECSchema.",
                            ecSchema.GetFullSchemaName().c_str(), ecSchema.GetAlias().c_str());
        return ERROR;
        }

    if (SUCCESS != InsertSchemaReferenceEntries(ctx, ecSchema))
        {
        ctx.Issues().ReportV("Failed to import ECSchema references for ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    //enums must be imported before ECClasses as properties reference enums
    for (ECEnumerationCP ecEnum : ecSchema.GetEnumerations())
        {
        if (SUCCESS != ImportEnumeration(ctx, *ecEnum))
            {
            ctx.Issues().ReportV("Failed to import ECEnumeration '%s'.", ecEnum->GetFullName().c_str());
            return ERROR;
            }
        }

    //Unit stuff must be imported before KOQs as KOQ reference them
    for (UnitSystemCP us : ecSchema.GetUnitSystems())
        {
        if (SUCCESS != ImportUnitSystem(ctx, *us))
            {
            ctx.Issues().ReportV("Failed to import UnitSystem '%s'.", us->GetFullName().c_str());
            return ERROR;
            }
        }

    for (PhenomenonCP ph : ecSchema.GetPhenomena())
        {
        if (SUCCESS != ImportPhenomenon(ctx, *ph))
            {
            ctx.Issues().ReportV("Failed to import Phenomenon '%s'.", ph->GetFullName().c_str());
            return ERROR;
            }
        }

    for (ECUnitCP unit : ecSchema.GetUnits())
        {
        if (SUCCESS != ImportUnit(ctx, *unit))
            {
            ctx.Issues().ReportV("Failed to import Unit '%s'.", unit->GetFullName().c_str());
            return ERROR;
            }
        }

    for (ECFormatCP format : ecSchema.GetFormats())
        {
        if (SUCCESS != ImportFormat(ctx, *format))
            {
            ctx.Issues().ReportV("Failed to import Format '%s'.", format->GetFullName().c_str());
            return ERROR;
            }
        }

    //KOQs must be imported before ECClasses as properties reference KOQs
    for (KindOfQuantityCP koq : ecSchema.GetKindOfQuantities())
        {
        if (SUCCESS != ImportKindOfQuantity(ctx, *koq))
            {
            ctx.Issues().ReportV("Failed to import KindOfQuantity '%s'.", koq->GetFullName().c_str());
            return ERROR;
            }
        }

    //PropertyCategories must be imported before ECClasses as properties reference PropertyCategories
    for (PropertyCategoryCP cat : ecSchema.GetPropertyCategories())
        {
        if (SUCCESS != ImportPropertyCategory(ctx, *cat))
            {
            ctx.Issues().ReportV("Failed to import PropertyCategory '%s'.", cat->GetFullName().c_str());
            return ERROR;
            }
        }
    
    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        if (SUCCESS != ImportClass(ctx, *ecClass))
            {
            ctx.Issues().ReportV("Failed to import ECClass '%s'.", ecClass->GetFullName());
            return ERROR;
            }
        }

    if (SUCCESS != ImportCustomAttributes(ctx, ecSchema, ECContainerId(ecSchema.GetId()), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema))
        {
        ctx.Issues().ReportV("Failed to import custom attributes of ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertSchemaReferenceEntries(Context& ctx, ECSchemaCR schema)
    {
    ECSchemaReferenceListCR references = schema.GetReferencedSchemas();
    if (references.empty())
        return SUCCESS;

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_SchemaReference "(SchemaId,ReferencedSchemaId) VALUES(?,?)");
    if (stmt == nullptr)
        return ERROR;

    for (bpair<SchemaKey, ECSchemaPtr> const& kvPair : references)
        {
        ECSchemaCP reference = kvPair.second.get();
        ECSchemaId referenceId = SchemaPersistenceHelper::GetSchemaId(ctx.GetECDb(), DbTableSpace::Main(), reference->GetName().c_str(), SchemaLookupMode::ByName);
        if (!referenceId.IsValid())
            {
            BeAssert(false && "BuildDependencyOrderedSchemaList used by caller should have ensured that all references are already imported");
            return ERROR;
            }

        if (!reference->HasId())
            {
            // We apparently have a second copy of an ECSchema that has already been imported. Ideally, we would have used the SchemaManager
            // as an IECSchemaLocater when we loaded the ECSchemas that we are imported, but since we did not, we simply *hope* that
            // the duplicated loaded from disk matches the one stored in the db.
            // The duplicate copy does not have its Ids set... and so we will have to look them up, here and elsewhere, and set them into
            // the in-memory duplicate copy. In Graphite02, we might risk cleaning this up to force use of the already-persisted ECSchema
            // or else to do a one-shot validation of the ECSchema and updating of its ids.
            // Grep for GetClassIdForECClassFromDuplicateECSchema and GetPropertyIdForECPropertyFromDuplicateECSchema for other ramifications of this.
            const_cast<ECSchema*>(reference)->SetId(referenceId);
            }

        if (BE_SQLITE_OK != stmt->BindId(1, schema.GetId()))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(2, referenceId))
            return ERROR;

        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;

        stmt->Reset();
        stmt->ClearBindings();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportClass(Context& ctx, ECN::ECClassCR ecClass)
    {
    if (ctx.GetSchemaManager().GetClassId(ecClass).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(ecClass.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV("Failed to import ECClass '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ecClass.GetName().c_str(), ecClass.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import ECClass because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }
    
    //now import actual ECClass
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Class "(SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection,CustomAttributeContainerType) VALUES(?,?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClass.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, ecClass.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecClass.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ecClass.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!ecClass.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecClass.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(5, Enum::ToInt(ecClass.GetClassType())))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(6, Enum::ToInt(ecClass.GetClassModifier())))
        return ERROR;

    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass != nullptr)
        {
        if (BE_SQLITE_OK != stmt->BindInt(7, Enum::ToInt(relClass->GetStrength())))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(8, Enum::ToInt(relClass->GetStrengthDirection())))
            return ERROR;
        }

    ECCustomAttributeClassCP caClass = ecClass.GetCustomAttributeClassCP();
    if (caClass != nullptr && caClass->GetContainerType() != CustomAttributeContainerType::Any)
        {
        if (BE_SQLITE_OK != stmt->BindInt(9, Enum::ToInt(caClass->GetContainerType())))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const ECClassId classId = DbUtilities::GetLastInsertedId<ECClassId>(ctx.GetECDb());
    if (!classId.IsValid())
        return ERROR;

    const_cast<ECClassR>(ecClass).SetId(classId);

    //release stmt so that it can be reused to insert base classes
    stmt = nullptr;

    //Import All baseCases
    int baseClassIndex = 0;
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        if (SUCCESS != ImportClass(ctx, *baseClass))
            return ERROR;

        if (SUCCESS != InsertBaseClassEntry(ctx, classId, *baseClass, baseClassIndex++))
            return ERROR;
        }

    int propertyIndex = 0;
    for (ECPropertyCP ecProperty : ecClass.GetProperties(false))
        {
        if (SUCCESS != ImportProperty(ctx, *ecProperty, propertyIndex++))
            {
            ctx.Issues().ReportV("Failed to import ECProperty '%s' of ECClass '%s'.", ecProperty->GetName().c_str(), ecClass.GetFullName());
            return ERROR;
            }
        }

    ECN::ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr)
        {
        if (SUCCESS != ImportRelationshipClass(ctx, relationship))
            return ERROR;
        }

    return ImportCustomAttributes(ctx, ecClass, ECContainerId(classId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportEnumeration(Context& ctx, ECEnumerationCR ecEnum)
    {
    if (ctx.GetSchemaManager().GetEnumerationId(ecEnum).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(ecEnum.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV("Failed to import ECEnumeration '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ecEnum.GetName().c_str(), ecEnum.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import ECEnumeration because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Enumeration "(SchemaId,Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues) VALUES(?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecEnum.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, ecEnum.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecEnum.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ecEnum.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!ecEnum.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecEnum.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(5, (int) ecEnum.GetType()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindBoolean(6, ecEnum.GetIsStrict()))
        return ERROR;

    Utf8String enumValueJson;
    if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(enumValueJson, ecEnum))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(7, enumValueJson, Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const ECEnumerationId enumId = DbUtilities::GetLastInsertedId<ECEnumerationId>(ctx.GetECDb());
    if (!enumId.IsValid())
        return ERROR;

    const_cast<ECEnumerationR>(ecEnum).SetId(enumId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportUnitSystem(Context& ctx, UnitSystemCR us)
    {
    if (ctx.GetSchemaManager().GetUnitSystemId(us).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(us.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV("Failed to import UnitSystem '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", us.GetName().c_str(), us.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import UnitSystem because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_UnitSystem "(SchemaId,Name,DisplayLabel,Description) VALUES(?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, us.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, us.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (us.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, us.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!us.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, us.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const UnitSystemId usId = DbUtilities::GetLastInsertedId<UnitSystemId>(ctx.GetECDb());
    if (!usId.IsValid())
        return ERROR;

    const_cast<UnitSystemR>(us).SetId(usId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportPhenomenon(Context& ctx, PhenomenonCR ph)
    {
    if (ctx.GetSchemaManager().GetPhenomenonId(ph).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(ph.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV("Failed to import Phenomenon '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ph.GetName().c_str(), ph.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import Phenomenon because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Phenomenon "(SchemaId,Name,DisplayLabel,Description,Definition) VALUES(?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ph.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, ph.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ph.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ph.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!ph.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ph.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ph.GetDefinition(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const PhenomenonId phId = DbUtilities::GetLastInsertedId<PhenomenonId>(ctx.GetECDb());
    if (!phId.IsValid())
        return ERROR;

    const_cast<PhenomenonR>(ph).SetId(phId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportUnit(Context& ctx, ECUnitCR unit)
    {
    if (ctx.GetSchemaManager().GetUnitId(unit).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(unit.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV("Failed to import Unit '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", unit.GetName().c_str(), unit.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import Unit because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    PhenomenonCP phen = unit.GetPhenomenon();
    if (SUCCESS != ImportPhenomenon(ctx, *phen))
        return ERROR;

    if (unit.HasUnitSystem())
        {
        UnitSystemCP system = unit.GetUnitSystem();
        if (SUCCESS != ImportUnitSystem(ctx, *system))
            return ERROR;
        }

    ECUnitCP invertingUnit = nullptr;
    if (unit.IsInvertedUnit())
        {
        invertingUnit = unit.GetInvertingUnit();
        BeAssert(invertingUnit != nullptr);
        if (SUCCESS != ImportUnit(ctx, *invertingUnit))
            return ERROR;
        }
    

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Unit "(SchemaId,Name,DisplayLabel,Description,PhenomenonId,UnitSystemId,Definition,Numerator,Denominator,Offset,IsConstant,InvertingUnitId) VALUES(?,?,?,?,?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    const int schemaIdParamIx = 1;
    const int nameParamIx = 2;
    const int labelParamIx = 3;
    const int descParamIx = 4;
    const int phIdParamIx = 5;
    const int usIdParamIx = 6;
    const int defParamIx = 7;
    const int numeratorParamIx = 8;
    const int denominatorParamIx = 9;
    const int offsetParamIx = 10;
    const int isConstantParamIx = 11;
    const int invertingUnitIdParamIx = 12;

    if (BE_SQLITE_OK != stmt->BindId(schemaIdParamIx, unit.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(nameParamIx, unit.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (unit.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(labelParamIx, unit.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!unit.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(descParamIx, unit.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindId(phIdParamIx, phen->GetId()))
        return ERROR;

    if (unit.HasUnitSystem())
        {
        if (BE_SQLITE_OK != stmt->BindId(usIdParamIx, unit.GetUnitSystem()->GetId()))
            return ERROR;
        }

    //WIP_UNITS check for inverted unit must be replaced by HasDefinition
    if (!unit.IsInvertedUnit() && !unit.GetDefinition().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(defParamIx, unit.GetDefinition(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (unit.HasNumerator())
        {
        if (BE_SQLITE_OK != stmt->BindDouble(numeratorParamIx, unit.GetNumerator()))
            return ERROR;
        }

    if (unit.HasDenominator())
        {
        if (BE_SQLITE_OK != stmt->BindDouble(denominatorParamIx, unit.GetDenominator()))
            return ERROR;
        }

    if (unit.HasOffset())
        {
        if (BE_SQLITE_OK != stmt->BindDouble(offsetParamIx, unit.GetOffset()))
            return ERROR;
        }

    // WIP_UNITS replace with unit.HasIsConstant once implemented
    if (!unit.IsInvertedUnit())
        {
        if (BE_SQLITE_OK != stmt->BindBoolean(isConstantParamIx, unit.IsConstant()))
            return ERROR;
        }

    if (invertingUnit != nullptr)
        {
        if (BE_SQLITE_OK != stmt->BindId(invertingUnitIdParamIx, invertingUnit->GetId()))
            return ERROR;
        }

    DbResult stat = stmt->Step();
    if (BE_SQLITE_DONE != stat)
        return ERROR;

    const UnitId unitId = DbUtilities::GetLastInsertedId<UnitId>(ctx.GetECDb());
    if (!unitId.IsValid())
        return ERROR;

    const_cast<ECUnitR>(unit).SetId(unitId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportFormat(Context& ctx, ECFormatCR format)
    {
    if (ctx.GetSchemaManager().GetFormatId(format).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(format.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV("Failed to import Format '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", format.GetName().c_str(), format.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import Format because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Format "(SchemaId,Name,DisplayLabel,Description,NumericSpec,CompositeSpec) VALUES(?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    const int schemaIdParamIx = 1;
    const int nameParamIx = 2;
    const int labelParamIx = 3;
    const int descParamIx = 4;
    const int numericSpecParamIx = 5;
    const int compositeSpecParamIx = 6;

    if (BE_SQLITE_OK != stmt->BindId(schemaIdParamIx, format.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(nameParamIx, format.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (format.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(labelParamIx, format.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!format.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(descParamIx, format.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (format.HasNumeric())
        {
        if (BE_SQLITE_OK != stmt->BindText(numericSpecParamIx, SchemaPersistenceHelper::SerializeNumericSpec(*format.GetNumericSpec()), Statement::MakeCopy::Yes))
            return ERROR;
        }

    if (format.HasComposite())
        {
        //Composite Spec Units are persisted in its own table to leverage FKs to the Units table
        Utf8String specStr = SchemaPersistenceHelper::SerializeCompositeSpecWithoutUnits(*format.GetCompositeSpec());
        if (!specStr.empty())
            {
            if (BE_SQLITE_OK != stmt->BindText(compositeSpecParamIx, specStr, Statement::MakeCopy::Yes))
                return ERROR;
            }
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    stmt = nullptr;

    const FormatId formatId = DbUtilities::GetLastInsertedId<FormatId>(ctx.GetECDb());
    if (!formatId.IsValid())
        return ERROR;

    const_cast<ECFormatR>(format).SetId(formatId);

    if (format.HasComposite())
        return ImportFormatComposite(ctx, format, formatId);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportFormatComposite(Context& ctx, ECFormatCR format, FormatId formatId)
    {
    if (!format.HasComposite())
        return SUCCESS;

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_FormatCompositeUnit "(FormatId,Label,UnitId,Ordinal) VALUES(?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    Formatting::CompositeValueSpecCR spec = *format.GetCompositeSpec();

    auto insertUnit = [] (CachedStatement& stmt, FormatId formatId, Nullable<Utf8String> label, ECUnitCR unit, int ordinal)
        {
        if (BE_SQLITE_OK != stmt.BindId(1, formatId))
            return ERROR;

        if (!label.IsNull())
            {
            if (BE_SQLITE_OK != stmt.BindText(2, label.Value(), Statement::MakeCopy::Yes))
                return ERROR;
            }

        if (BE_SQLITE_OK != stmt.BindId(3, unit.GetId()))
            return ERROR;

        if (BE_SQLITE_OK != stmt.BindInt(4, ordinal))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.Reset();
        stmt.ClearBindings();
        return SUCCESS;
        };

    int ordinal = 0;
    if (spec.HasMajorUnit())
        {
        ECUnitCP unit = (ECUnitCP) spec.GetMajorUnit();
        if (SUCCESS != ImportUnit(ctx, *unit))
            return ERROR;

        Nullable<Utf8String> label = spec.HasMajorLabel() ? spec.GetMajorLabel() : nullptr;
        if (SUCCESS != insertUnit(*stmt, formatId, label, *unit, ordinal))
            return ERROR;
        }
    ordinal++;
    if (spec.HasMiddleUnit())
        {
        ECUnitCP unit = (ECUnitCP) spec.GetMiddleUnit();
        if (SUCCESS != ImportUnit(ctx, *unit))
            return ERROR;

        Nullable<Utf8String> label = spec.HasMiddleLabel() ? spec.GetMiddleLabel() : nullptr;
        if (SUCCESS != insertUnit(*stmt, formatId, label, *unit, ordinal))
            return ERROR;
        }

    ordinal++;
    if (spec.HasMinorUnit())
        {
        ECUnitCP unit = (ECUnitCP) spec.GetMinorUnit();
        if (SUCCESS != ImportUnit(ctx, *unit))
            return ERROR;

        Nullable<Utf8String> label = spec.HasMinorLabel() ? spec.GetMinorLabel() : nullptr;
        if (SUCCESS != insertUnit(*stmt, formatId, label, *unit, ordinal))
            return ERROR;
        }

    ordinal++;
    if (spec.HasSubUnit())
        {
        ECUnitCP unit = (ECUnitCP) spec.GetSubUnit();
        if (SUCCESS != ImportUnit(ctx, *unit))
            return ERROR;

        Nullable<Utf8String> label = spec.HasSubLabel() ? spec.GetSubLabel() : nullptr;
        if (SUCCESS != insertUnit(*stmt, formatId, label, *unit, ordinal))
            return ERROR;
        }

    BeAssert(ordinal != 0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportKindOfQuantity(Context& ctx, KindOfQuantityCR koq)
    {
    if (ctx.GetSchemaManager().GetKindOfQuantityId(koq).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(koq.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV("Failed to import KindOfQuantity '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", koq.GetName().c_str(), koq.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import KindOfQuantity because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_KindOfQuantity(SchemaId,Name,DisplayLabel,Description,RelativeError,PersistenceUnit,PresentationUnits) VALUES(?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, koq.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, koq.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (koq.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, koq.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!koq.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, koq.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindDouble(5, koq.GetRelativeError()))
        return ERROR;

    if (koq.GetPersistenceUnit() == nullptr)
        {
        ctx.Issues().ReportV("Failed to import KindOfQuantity '%s'. It must have a persistence unit.", koq.GetFullName().c_str());
        return ERROR;
        }

    Utf8String persistenceUnitStr = koq.GetPersistenceUnit()->GetQualifiedName(koq.GetSchema());
    BeAssert(!persistenceUnitStr.empty());
    if (BE_SQLITE_OK != stmt->BindText(6, persistenceUnitStr, Statement::MakeCopy::No))
        return ERROR;

    Utf8String presUnitsJsonStr;
    if (!koq.GetPresentationFormats().empty())
        {
        if (SUCCESS != SchemaPersistenceHelper::SerializeKoqPresentationFormats(presUnitsJsonStr, ctx.GetECDb(), koq))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindText(7, presUnitsJsonStr, Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const KindOfQuantityId koqId = DbUtilities::GetLastInsertedId<KindOfQuantityId>(ctx.GetECDb());
    if (!koqId.IsValid())
        return ERROR;

    const_cast<KindOfQuantityR>(koq).SetId(koqId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportPropertyCategory(Context& ctx, PropertyCategoryCR cat)
    {
    if (ctx.GetSchemaManager().GetPropertyCategoryId(cat).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(cat.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV("Failed to import PropertyCategory '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", cat.GetName().c_str(), cat.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import PropertyCategory because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_PropertyCategory(SchemaId,Name,DisplayLabel,Description,Priority) VALUES(?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, cat.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, cat.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (cat.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, cat.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!cat.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, cat.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    //uint32_t persisted as int64 to not lose unsignedness
    if (BE_SQLITE_OK != stmt->BindInt64(5, (int64_t) cat.GetPriority()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    PropertyCategoryId catId = DbUtilities::GetLastInsertedId<PropertyCategoryId>(ctx.GetECDb());
    if (!catId.IsValid())
        return ERROR;

    const_cast<PropertyCategoryR>(cat).SetId(catId);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportRelationshipClass(Context& ctx, ECN::ECRelationshipClassCP relationship)
    {
    const ECClassId relClassId = relationship->GetId();
    if (SUCCESS != ImportRelationshipConstraint(ctx, relClassId, relationship->GetSource(), ECRelationshipEnd_Source))
        return ERROR;

    return ImportRelationshipConstraint(ctx, relClassId, relationship->GetTarget(), ECRelationshipEnd_Target);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportRelationshipConstraint(Context& ctx, ECClassId relClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd end)
    {
    BeAssert(relClassId.IsValid());

    ECRelationshipConstraintId constraintId;
    if (SUCCESS != InsertRelationshipConstraintEntry(ctx, constraintId, relClassId, relationshipConstraint, end))
        return ERROR;

    BeAssert(constraintId.IsValid());
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_RelationshipConstraintClass(ConstraintId,ClassId) VALUES(?,?)");
    if (stmt == nullptr)
        return ERROR;

    for (ECClassCP constraintClass : relationshipConstraint.GetConstraintClasses())
        {
        if (SUCCESS != ImportClass(ctx, *constraintClass))
            return ERROR;

        BeAssert(constraintClass->GetId().IsValid());

        if (BE_SQLITE_OK != stmt->BindId(1, constraintId))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(2, constraintClass->GetId()))
            return ERROR;

        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;

        stmt->Reset();
        stmt->ClearBindings();
        }

    stmt = nullptr;
    SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType = end == ECRelationshipEnd_Source ? SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint : SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint;
    return ImportCustomAttributes(ctx, relationshipConstraint, ECContainerId(constraintId), containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertRelationshipConstraintEntry(Context& ctx, ECRelationshipConstraintId& constraintId, ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_RelationshipConstraint(RelationshipClassId,RelationshipEnd,MultiplicityLowerLimit,MultiplicityUpperLimit,IsPolymorphic,RoleLabel,AbstractConstraintClassId) VALUES(?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, (int) endpoint))
        return ERROR;

    //uint32_t are persisted as int64 to not lose the unsigned-ness.
    if (BE_SQLITE_OK != stmt->BindInt64(3, (int64_t) relationshipConstraint.GetMultiplicity().GetLowerLimit()))
        return ERROR;

    //If unbounded, we persist DB NULL
    if (!relationshipConstraint.GetMultiplicity().IsUpperLimitUnbounded())
        {
        //uint32_t are persisted as int64 to not lose the unsigned-ness.
        if (BE_SQLITE_OK != stmt->BindInt64(4, (int64_t) relationshipConstraint.GetMultiplicity().GetUpperLimit()))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindBoolean(5, relationshipConstraint.GetIsPolymorphic()))
        return ERROR;

    
    if (!relationshipConstraint.GetRoleLabel().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(6, relationshipConstraint.GetRoleLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (relationshipConstraint.IsAbstractConstraintDefined())
        {
        ECClassCR abstractConstraintClass = *relationshipConstraint.GetAbstractConstraint();
        if (SUCCESS != ImportClass(ctx, abstractConstraintClass))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(7, abstractConstraintClass.GetId()))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    constraintId = DbUtilities::GetLastInsertedId<ECRelationshipConstraintId>(ctx.GetECDb());
    BeAssert(constraintId.IsValid());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportProperty(Context& ctx, ECN::ECPropertyCR ecProperty, int ordinal)
    {
    //Local properties are expected to not be imported at this point as they get imported along with their class.
    BeAssert(!ctx.GetSchemaManager().GetPropertyId(ecProperty).IsValid());

    if (ecProperty.GetIsStruct())
        {
        if (SUCCESS != ImportClass(ctx, ecProperty.GetAsStructProperty()->GetType()))
            return ERROR;
        }
    else if (ecProperty.GetIsStructArray())
        {
        if (SUCCESS != ImportClass(ctx, ecProperty.GetAsStructArrayProperty()->GetStructElementType()))
            return ERROR;
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (SUCCESS != ImportClass(ctx, *ecProperty.GetAsNavigationProperty()->GetRelationshipClass()))
            return ERROR;
        }

    //now insert the actual property
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_Property(ClassId,Name,DisplayLabel,Description,IsReadonly,Priority,Ordinal,Kind,"
                                                        "PrimitiveType,PrimitiveTypeMinLength,PrimitiveTypeMaxLength,PrimitiveTypeMinValue,PrimitiveTypeMaxValue,"
                                                        "EnumerationId,StructClassId,ExtendedTypeName,KindOfQuantityId,CategoryId,ArrayMinOccurs,ArrayMaxOccurs,NavigationRelationshipClassId,NavigationDirection) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecProperty.GetClass().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, ecProperty.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecProperty.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ecProperty.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!ecProperty.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecProperty.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindBoolean(5, ecProperty.GetIsReadOnly()))
        return ERROR;

    if (ecProperty.IsPriorityLocallyDefined())
        {
        //priority is persisted as int64 to not lose unsignedness
        if (BE_SQLITE_OK != stmt->BindInt64(6, (int64_t) ecProperty.GetPriority()))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(7, ordinal))
        return ERROR;

    const int kindIndex = 8;
    const int primitiveTypeIndex = 9;
    const int primitiveTypeMinLengthIndex = 10;
    const int primitiveTypeMaxLengthIndex = 11;
    const int primitiveTypeMinValueIndex = 12;
    const int primitiveTypeMaxValueIndex = 13;
    const int enumIdIndex = 14;
    const int structClassIdIndex = 15;
    const int extendedTypeIndex = 16;
    const int koqIdIndex = 17;
    const int catIdIndex = 18;
    const int arrayMinIndex = 19;
    const int arrayMaxIndex = 20;
    const int navRelClassIdIndex = 21;
    const int navDirIndex = 22;

    if (ecProperty.IsMinimumLengthDefined())
        {
        //min length is persisted as int64 to not lose unsignedness
        if (BE_SQLITE_OK != stmt->BindInt64(primitiveTypeMinLengthIndex, (int64_t) ecProperty.GetMinimumLength()))
            return ERROR;
        }

    if (ecProperty.IsMaximumLengthDefined())
        {
        //max length is persisted as int64 to not lose unsignedness
        if (BE_SQLITE_OK != stmt->BindInt64(primitiveTypeMaxLengthIndex, (int64_t) ecProperty.GetMaximumLength()))
            return ERROR;
        }

    if (ecProperty.IsMinimumValueDefined())
        {
        ECValue v;
        if (ECObjectsStatus::Success != ecProperty.GetMinimumValue(v))
            {
            BeAssert(false && "Failed to read MinimumValue from ECProperty");
            return ERROR;
            }

        if (SUCCESS != BindPropertyMinMaxValue(ctx, *stmt, primitiveTypeMinValueIndex, ecProperty, v))
            return ERROR;
        }

    if (ecProperty.IsMaximumValueDefined())
        {
        ECValue v;
        if (ECObjectsStatus::Success != ecProperty.GetMaximumValue(v))
            {
            BeAssert(false && "Failed to read MaximumValue from ECProperty");
            return ERROR;
            }

        if (SUCCESS != BindPropertyMinMaxValue(ctx, *stmt, primitiveTypeMaxValueIndex, ecProperty, v))
            return ERROR;
        }

    if (ecProperty.HasExtendedType())
        {
        if (SUCCESS != BindPropertyExtendedTypeName(*stmt, extendedTypeIndex, ecProperty))
            return ERROR;
        }

    if (SUCCESS != BindPropertyKindOfQuantity(ctx, *stmt, koqIdIndex, ecProperty))
        return ERROR;

    if (SUCCESS != BindPropertyCategory(ctx, *stmt, catIdIndex, ecProperty))
        return ERROR;

    if (ecProperty.GetIsPrimitive())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(PropertyKind::Primitive)))
            return ERROR;

        if (SUCCESS != BindPropertyPrimTypeOrEnumeration(ctx, *stmt, primitiveTypeIndex, enumIdIndex, ecProperty))
            return ERROR;
        }
    else if (ecProperty.GetIsStruct())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(PropertyKind::Struct)))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(structClassIdIndex, ecProperty.GetAsStructProperty()->GetType().GetId()))
            return ERROR;
        }
    else if (ecProperty.GetIsArray())
        {
        ArrayECPropertyCP arrayProp = ecProperty.GetAsArrayProperty();
        if (arrayProp->GetKind() == ARRAYKIND_Primitive)
            {
            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(PropertyKind::PrimitiveArray)))
                return ERROR;

            if (SUCCESS != BindPropertyPrimTypeOrEnumeration(ctx, *stmt, primitiveTypeIndex, enumIdIndex, ecProperty))
                return ERROR;
            }
        else
            {
            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(PropertyKind::StructArray)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindId(structClassIdIndex, arrayProp->GetAsStructArrayProperty()->GetStructElementType().GetId()))
                return ERROR;
            }

        //uint32_t are persisted as int64 to not lose the unsigned-ness.
        if (BE_SQLITE_OK != stmt->BindInt64(arrayMinIndex, (int64_t) arrayProp->GetMinOccurs()))
            return ERROR;

        //If maxoccurs is unbounded, we persist DB NULL
        if (!arrayProp->IsStoredMaxOccursUnbounded())
            {
            //until the max occurs bug in ECObjects (where GetMaxOccurs always returns "unbounded")
            //has been fixed, we need to call GetStoredMaxOccurs to retrieve the proper max occurs
            if (BE_SQLITE_OK != stmt->BindInt64(arrayMaxIndex, (int64_t) arrayProp->GetStoredMaxOccurs()))
                return ERROR;
            }
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(PropertyKind::Navigation)))
            return ERROR;

        NavigationECPropertyCP navProp = ecProperty.GetAsNavigationProperty();
        if (BE_SQLITE_OK != stmt->BindId(navRelClassIdIndex, navProp->GetRelationshipClass()->GetId()))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(navDirIndex, Enum::ToInt(navProp->GetDirection())))
            return ERROR;
        }

    
    DbResult stat = stmt->Step();
    if (BE_SQLITE_DONE != stat)
        {
        LOG.fatal(ctx.GetECDb().GetLastError().c_str());
        return ERROR;
        }

    const ECPropertyId propId = DbUtilities::GetLastInsertedId<ECPropertyId>(ctx.GetECDb());
    if (!propId.IsValid())
        return ERROR;

    const_cast<ECPropertyR>(ecProperty).SetId(propId);

    return ImportCustomAttributes(ctx, ecProperty, ECContainerId(propId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportCustomAttributes(Context& ctx, IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType)
    {
    int ordinal = 0;
    for (IECInstancePtr ca : sourceContainer.GetCustomAttributes(false))
        {
        //import CA classes first
        ECClassCR caClass = ca->GetClass();
        if (SUCCESS != ImportClass(ctx, caClass))
            return ERROR;

        if (SUCCESS != InsertCAEntry(ctx, *ca, caClass.GetId(), sourceContainerId, containerType, ordinal))
            return ERROR;

        ordinal++;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertSchemaEntry(ECDbCR ecdb, ECSchemaCR schema)
    {
    BeAssert(!schema.HasId());

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_Schema(Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3,ECVersion,OriginalECXmlVersionMajor,OriginalECXmlVersionMinor) VALUES(?,?,?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(1, schema.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (schema.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(2, schema.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!schema.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, schema.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(4, schema.GetAlias(), Statement::MakeCopy::No))
        return ERROR;

    //Persist uint32_t as int64 to not lose unsigned-ness
    if (BE_SQLITE_OK != stmt->BindInt64(5, (int64_t) schema.GetVersionRead()))
        return ERROR;

    //Persist uint32_t as int64 to not lose unsigned-ness
    if (BE_SQLITE_OK != stmt->BindInt64(6, (int64_t) schema.GetVersionWrite()))
        return ERROR;

    //Persist uint32_t as int64 to not lose unsigned-ness
    if (BE_SQLITE_OK != stmt->BindInt64(7, (int64_t) schema.GetVersionMinor()))
        return ERROR;

    //Persist uint32_t as int64 to not lose unsigned-ness
    if (BE_SQLITE_OK != stmt->BindInt64(8, (int64_t) schema.GetECVersion()))
        return ERROR;

    //original version of 0.0 is considered an unset version
    if (schema.GetOriginalECXmlVersionMajor() > 0 || schema.GetOriginalECXmlVersionMinor() > 0)
        {
        //Persist uint32_t as int64 to not lose unsigned-ness
        if (BE_SQLITE_OK != stmt->BindInt64(9, (int64_t) schema.GetOriginalECXmlVersionMajor()))
            return ERROR;

        //Persist uint32_t as int64 to not lose unsigned-ness
        if (BE_SQLITE_OK != stmt->BindInt64(10, (int64_t) schema.GetOriginalECXmlVersionMinor()))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const ECSchemaId id = DbUtilities::GetLastInsertedId<ECSchemaId>(ecdb);
    if (!id.IsValid())
        return ERROR;

    const_cast<ECSchemaR>(schema).SetId(id);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertBaseClassEntry(Context& ctx, ECClassId ecClassId, ECClassCR baseClass, int ordinal)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_ClassHasBaseClasses(ClassId,BaseClassId,Ordinal) VALUES(?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, baseClass.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, ordinal))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyMinMaxValue(Context& ctx, Statement& stmt, int paramIndex, ECN::ECPropertyCR prop, ECN::ECValueCR val)
    {
    if (!val.IsPrimitive())
        {
        BeAssert(false && "Min/MaxValue ECValue is expected to always be a primitive value");
        return ERROR;
        }

    Nullable<PrimitiveType> propPrimType;
    PrimitiveECPropertyCP primProp = prop.GetAsPrimitiveProperty();
    if (primProp != nullptr)
        propPrimType = primProp->GetType();
    else
        {
        PrimitiveArrayECPropertyCP primArrayProp = prop.GetAsPrimitiveArrayProperty();
        propPrimType = primArrayProp->GetPrimitiveElementType();
        }

    if (propPrimType.IsNull())
        {
        BeAssert(false && "Min/MaxValue ECValue is expected to only be defined for primitive and primitive array properties");
        return ERROR;
        }

    switch (propPrimType.Value())
        {
            case PRIMITIVETYPE_DateTime:
            {
            if (val.IsDateTime())
                {
                const uint64_t jdMsec = DateTime::CommonEraMillisecondsToJulianDay(val.GetDateTimeTicks());
                return BE_SQLITE_OK == stmt.BindDouble(paramIndex, DateTime::MsecToRationalDay(jdMsec)) ? SUCCESS : ERROR;
                }

            break;
            }

            case PRIMITIVETYPE_Double:
                if (val.IsDouble())
                    return BE_SQLITE_OK == stmt.BindDouble(paramIndex, val.GetDouble()) ? SUCCESS : ERROR;
                else if (val.IsInteger())
                    return BE_SQLITE_OK == stmt.BindInt(paramIndex, val.GetInteger()) ? SUCCESS : ERROR;
                else if (val.IsLong())
                    return BE_SQLITE_OK == stmt.BindInt64(paramIndex, val.GetLong()) ? SUCCESS : ERROR;
                break;

            case PRIMITIVETYPE_Integer:
                if (val.IsInteger())
                    return BE_SQLITE_OK == stmt.BindInt(paramIndex, val.GetInteger()) ? SUCCESS : ERROR;

                break;

            case PRIMITIVETYPE_Long:
                if (val.IsLong())
                    return BE_SQLITE_OK == stmt.BindInt64(paramIndex, val.GetLong()) ? SUCCESS : ERROR;
                else if (val.IsInteger())
                    return BE_SQLITE_OK == stmt.BindInt(paramIndex, val.GetInteger()) ? SUCCESS : ERROR;

                break;

            case PRIMITIVETYPE_String:
                if (val.IsString())
                    return BE_SQLITE_OK == stmt.BindText(paramIndex, val.GetUtf8CP(), Statement::MakeCopy::Yes) ? SUCCESS : ERROR;

                break;

            default:
                break;
        }

    ctx.Issues().ReportV("Failed to import schema. The ECProperty '%s.%s' has a minimum/maximum value of an unsupported type.",
                                                    prop.GetClass().GetFullName(), prop.GetName().c_str());
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyExtendedTypeName(Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    if (!prop.HasExtendedType())
        return SUCCESS;

    Utf8StringCP extendedTypeName = nullptr;
    if (prop.GetIsPrimitive())
        extendedTypeName = &prop.GetAsPrimitiveProperty()->GetExtendedTypeName();
    else if (prop.GetIsPrimitiveArray())
        extendedTypeName = &prop.GetAsPrimitiveArrayProperty()->GetExtendedTypeName();
    else
        {
        BeAssert(false && "Property which is not expected to support extended type names");
        return ERROR;
        }

    if (extendedTypeName->empty())
        return SUCCESS;

    return stmt.BindText(paramIndex, *extendedTypeName, Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyPrimTypeOrEnumeration(Context& ctx, Statement& stmt, int primTypeParamIndex, int enumParamIndex, ECPropertyCR prop)
    {
    ECEnumerationCP ecenum = nullptr;
    Nullable<PrimitiveType> primType;
    if (prop.GetIsPrimitive())
        {
        PrimitiveECPropertyCR primProp = *prop.GetAsPrimitiveProperty();
        ecenum = primProp.GetEnumeration();
        if (ecenum == nullptr)
            primType = primProp.GetType();
        }
    else if (prop.GetIsPrimitiveArray())
        {
        PrimitiveArrayECPropertyCR arrayProp = *prop.GetAsPrimitiveArrayProperty();
        ecenum = arrayProp.GetEnumeration();
        if (ecenum == nullptr)
            primType = arrayProp.GetPrimitiveElementType();
        }
    else
        {
        BeAssert(false && "Property which is not expected to support enumerations");
        return ERROR;
        }

    if (ecenum == nullptr)
        {
        BeAssert(!primType.IsNull());
        return BE_SQLITE_OK == stmt.BindInt(primTypeParamIndex, primType.Value()) ? SUCCESS : ERROR;
        }

    if (SUCCESS != ImportEnumeration(ctx, *ecenum))
        return ERROR;

    BeAssert(ecenum->HasId());
    return stmt.BindId(enumParamIndex, ecenum->GetId()) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyKindOfQuantity(Context& ctx, Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    if (!prop.IsKindOfQuantityDefinedLocally() || prop.GetKindOfQuantity() == nullptr)
        return SUCCESS;

    if (prop.GetIsNavigation())
        {
        ctx.Issues().ReportV("Failed to import Navigation ECProperty '%s.%s' because a KindOfQuantity is assigned to it.", prop.GetClass().GetFullName(), prop.GetName().c_str());
        return ERROR;
        }

    KindOfQuantityCP koq = prop.GetKindOfQuantity();
    if (SUCCESS != ImportKindOfQuantity(ctx, *koq))
        return ERROR;

    BeAssert(koq->HasId());
    return stmt.BindId(paramIndex, koq->GetId()) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyCategory(Context& ctx, Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    if (!prop.IsCategoryDefinedLocally() || prop.GetCategory() == nullptr)
        return SUCCESS;

    PropertyCategoryCP cat = prop.GetCategory();
    if (SUCCESS != ImportPropertyCategory(ctx, *cat))
        return ERROR;

    BeAssert(cat->HasId());
    return stmt.BindId(paramIndex, cat->GetId()) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::InsertCAEntry(Context& ctx, IECInstanceR customAttribute, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, int ordinal)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_CustomAttribute(ContainerId,ContainerType,ClassId,Ordinal,Instance) VALUES(?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(3, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(4, ordinal))
        return ERROR;

    Utf8String caXml;
    if (InstanceWriteStatus::Success != customAttribute.WriteToXmlString(caXml, false, //don't write XML description header as we only store an XML fragment
                                                                          true)) //store instance id for the rare cases where the client specified one.
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(5, caXml, Statement::MakeCopy::No))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteCAEntry(int& ordinal, Context& ctx, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("SELECT Ordinal FROM main.ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(3, ecClassId))
        return ERROR;

    if (stmt->Step() != BE_SQLITE_ROW)
        {
        //If this does not return a row then ECCustomAttributeClass is already deleted and it has caused cascade delete which have deleted all the associated customAttributes
        return SUCCESS;
        }

    ordinal = stmt->GetValueInt(0);

    stmt = ctx.GetCachedStatement("DELETE FROM main.ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(3, ecClassId))
        return ERROR;

    if (stmt->Step() != BE_SQLITE_DONE)
        return ERROR;

    BeAssert(ctx.GetECDb().GetModifiedRowCount() > 0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ReplaceCAEntry(Context& ctx, IECInstanceR customAttribute, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, int ordinal)
    {
    if (DeleteCAEntry(ordinal, ctx, ecClassId, containerId, containerType) != SUCCESS)
        return ERROR;

    return InsertCAEntry(ctx, customAttribute, ecClassId, containerId, containerType, ordinal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaWriter::IsPropertyTypeChangeSupported(Utf8StringR error, StringChange& typeChange, ECPropertyCR oldProperty, ECPropertyCR newProperty)
    {
    //changing from primitive to enum and enum to primitive is supported with same type and enum is unstrict
    if (oldProperty.GetIsPrimitive() && newProperty.GetIsPrimitive())
        {
        PrimitiveECPropertyCP a = oldProperty.GetAsPrimitiveProperty();
        PrimitiveECPropertyCP b = newProperty.GetAsPrimitiveProperty();
        ECEnumerationCP aEnum = a->GetEnumeration();
        ECEnumerationCP bEnum = b->GetEnumeration();
        if (!aEnum && !bEnum)
            {
            error.Sprintf("ECSchema Upgrade failed. ECProperty %s.%s: Changing the type of a Primitive ECProperty is not supported. Cannot convert from '%s' to '%s'",
                          oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str(), typeChange.GetOld().Value().c_str(), typeChange.GetNew().Value().c_str());
            return false;
            }

        if (aEnum && !bEnum)
            {
            if (aEnum->GetType() != b->GetType())
                {
                error.Sprintf("ECSchema Upgrade failed. ECProperty %s.%s: ECEnumeration specified for property must have same primitive type as new primitive property type",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

                return false;
                }

            return true;
            }
        if (!aEnum && bEnum)
            {
            if (a->GetType() != bEnum->GetType())
                {
                error.Sprintf("ECSchema Upgrade failed. ECProperty %s.%s: Primitive type change to ECEnumeration which as different type then existing primitive property",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

                return false;
                }

            if (bEnum->GetIsStrict())
                {
                error.Sprintf("ECSchema Upgrade failed. ECProperty %s.%s: Type change to a Strict ECEnumeration is not supported.",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

                return false;
                }

            return true;
            }

        if (aEnum && bEnum)
            {
            if (aEnum->GetType() != bEnum->GetType())
                {
                error.Sprintf("ECSchema Upgrade failed. ECProperty %s.%s: Exisitng ECEnumeration has different primitive type from the new ECEnumeration specified",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

                return false;
                }
            if (bEnum->GetIsStrict())
                {
                error.Sprintf("ECSchema Upgrade failed. ECProperty %s.%s: Type change to a Strict ECEnumeration is not supported.",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
                }
            return true;
            }
        }

    error.Sprintf("ECSchema Upgrade failed. ECProperty %s.%s: Changing the type of an ECProperty is not supported. Cannot convert from '%s' to '%s'",
                  oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str(), typeChange.GetOld().Value().c_str(), typeChange.GetNew().Value().c_str());

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateProperty(Context& ctx, ECPropertyChange& propertyChange, ECPropertyCR oldProperty, ECPropertyCR newProperty)
    {
    if (propertyChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (propertyChange.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of an ECProperty is not supported.");
        return ERROR;
        }

    ECPropertyId propertyId = ctx.GetSchemaManager().GetPropertyId(newProperty);
    if (!propertyId.IsValid())
        {
        BeAssert(false && "Failed to resolve ECPropertyId");
        return ERROR;
        }

    if (propertyChange.GetTypeName().IsValid())
        {
        Utf8String error;
        if (!IsPropertyTypeChangeSupported(error, propertyChange.GetTypeName(), oldProperty, newProperty))
            {
            ctx.Issues().Report(error.c_str());
            return ERROR;
            }
        }

    if (propertyChange.IsStruct().IsValid() || propertyChange.IsStructArray().IsValid() || propertyChange.IsPrimitive().IsValid() ||
        propertyChange.IsPrimitiveArray().IsValid() || propertyChange.IsNavigation().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the kind of the ECProperty is not supported.",
                                  oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return ERROR;
        }

    if (propertyChange.GetArray().IsValid())
        {
        ArrayChange& arrayChange = propertyChange.GetArray();
        if (arrayChange.MaxOccurs().IsValid() || arrayChange.MinOccurs().IsValid())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing 'MinOccurs' or 'MaxOccurs' for an Array ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }
        }

    if (propertyChange.GetNavigation().IsValid())
        {
        NavigationChange& navigationChange = propertyChange.GetNavigation();
        if (navigationChange.GetRelationshipClassName().IsValid())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'RelationshipClassName' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

        if (navigationChange.Direction().IsValid())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'Direction' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }
        }

    SqlUpdateBuilder sqlUpdateBuilder("ec_Property");

    if (propertyChange.GetMinimumLength().IsValid())
        {
        constexpr Utf8CP kPrimitiveTypeMinLength = "PrimitiveTypeMinLength";
        if (propertyChange.GetMinimumLength().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kPrimitiveTypeMinLength);
        else
            sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMinLength, propertyChange.GetMinimumLength().GetNew().Value());
        }
    
    if (propertyChange.GetMaximumLength().IsValid())
        {
        constexpr Utf8CP kPrimitiveTypeMaxLength = "PrimitiveTypeMaxLength";
        if (propertyChange.GetMaximumLength().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kPrimitiveTypeMaxLength);
        else
            sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMaxLength, propertyChange.GetMaximumLength().GetNew().Value());
        }

    if (propertyChange.GetMinimumValue().IsValid())
        {
        constexpr Utf8CP kPrimitiveTypeMinValue = "PrimitiveTypeMinValue";
        if (propertyChange.GetMinimumValue().GetNew().IsNull() || propertyChange.GetMinimumValue().GetNew().Value().IsNull())
            sqlUpdateBuilder.AddSetToNull(kPrimitiveTypeMinValue);
        else
            {
            ECValueCR value = propertyChange.GetMinimumValue().GetNew().Value();
            if (value.IsInteger())
                sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMinValue, value.GetInteger());
            else if (value.IsLong())
                sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMinValue, value.GetLong());
            else if (value.IsDouble())
                sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMinValue, value.GetDouble());
            else if (value.IsString())
                sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMinValue, value.GetUtf8CP());
            else
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'MinimumValue' to an unsupported type.",
                                oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
                return ERROR;
                }
            }
        }

    if (propertyChange.GetMaximumValue().IsValid())
        {
        constexpr Utf8CP kPrimitiveTypeMaxValue = "PrimitiveTypeMaxValue";
        if (propertyChange.GetMaximumValue().GetNew().IsNull() || propertyChange.GetMaximumValue().GetNew().Value().IsNull())
            sqlUpdateBuilder.AddSetToNull(kPrimitiveTypeMaxValue);
        else
            {
            ECValueCR value = propertyChange.GetMaximumValue().GetNew().Value();
            if (value.IsInteger())
                sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMaxValue, value.GetInteger());
            else if (value.IsLong())
                sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMaxValue, value.GetLong());
            else if (value.IsDouble())
                sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMaxValue, value.GetDouble());
            else if (value.IsString())
                sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMaxValue, value.GetUtf8CP());
            else
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'MaximumValue' to an unsupported type.",
                                oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
                return ERROR;
                }
            }
        }
    
    if (propertyChange.GetExtendedTypeName().IsValid())
        {
        constexpr Utf8CP kExtendedTypeName = "ExtendedTypeName";
        if (propertyChange.GetExtendedTypeName().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kExtendedTypeName);
        else
            sqlUpdateBuilder.AddSetExp(kExtendedTypeName, propertyChange.GetExtendedTypeName().GetNew().Value().c_str());
        }

    if (propertyChange.GetDisplayLabel().IsValid())
        {
        constexpr Utf8CP kDisplayLabel = "DisplayLabel";
        if (propertyChange.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kDisplayLabel);
        else
            sqlUpdateBuilder.AddSetExp(kDisplayLabel, propertyChange.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (propertyChange.GetDescription().IsValid())
        {
        constexpr Utf8CP kDescription = "Description";
        if (propertyChange.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kDescription);
        else
            sqlUpdateBuilder.AddSetExp(kDescription, propertyChange.GetDescription().GetNew().Value().c_str());
        }

    if (propertyChange.IsReadonly().IsValid())
        sqlUpdateBuilder.AddSetExp("IsReadonly", propertyChange.IsReadonly().GetNew().Value());

    if (propertyChange.GetPriority().IsValid())
        sqlUpdateBuilder.AddSetExp("Priority", propertyChange.GetPriority().GetNew().Value());

    if (propertyChange.GetEnumeration().IsValid())
        {
        if (!newProperty.GetIsPrimitive() && !newProperty.GetIsPrimitiveArray())
            {
            BeAssert(false);
            return ERROR;
            }

        if (propertyChange.GetEnumeration().GetNew().IsNull())
            {
            PrimitiveType newPrimType = newProperty.GetIsPrimitive() ? newProperty.GetAsPrimitiveProperty()->GetType() : newProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
            sqlUpdateBuilder.AddSetExp("PrimitiveType", (int) newPrimType);
            sqlUpdateBuilder.AddSetToNull("EnumerationId");
            }
        else
            {
            ECEnumerationCP enumCP = newProperty.GetIsPrimitive() ? newProperty.GetAsPrimitiveProperty()->GetEnumeration() : newProperty.GetAsPrimitiveArrayProperty()->GetEnumeration();
            if (enumCP == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            ECEnumerationId id = ctx.GetSchemaManager().GetEnumerationId(*enumCP);
            if (!id.IsValid())
                {
                if (ImportEnumeration(ctx, *enumCP) != SUCCESS)
                    return ERROR;

                id = enumCP->GetId();
                }

            sqlUpdateBuilder.AddSetToNull("PrimitiveType");
            sqlUpdateBuilder.AddSetExp("EnumerationId", id.GetValue());
            }
        }

    if (propertyChange.GetKindOfQuantity().IsValid())
        {
        StringChange& change = propertyChange.GetKindOfQuantity();

        if (change.GetNew().IsNull())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Removing a KindOfQuantity from a property is not supported.",
                                 oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

        KindOfQuantityCP newKoq = newProperty.GetKindOfQuantity();
        if (newKoq == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        if (!change.GetOld().IsNull())
            {
            KindOfQuantityCP oldKoq = oldProperty.GetKindOfQuantity();
            if (oldKoq == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }
           
            if (!oldKoq->GetPersistenceUnit()->GetFullName().EqualsIAscii(newKoq->GetPersistenceUnit()->GetFullName()))
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Replacing KindOfQuantity '%s' by '%s' is not supported because their persistent units differ.",
                                     oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str(), oldKoq->GetFullName().c_str(), newKoq->GetFullName().c_str());
                return ERROR;
                }
            }

        KindOfQuantityId id = ctx.GetSchemaManager().GetKindOfQuantityId(*newKoq);
        if (!id.IsValid())
            {
            if (ImportKindOfQuantity(ctx, *newKoq) != SUCCESS)
                return ERROR;

            id = newKoq->GetId();
            }

        sqlUpdateBuilder.AddSetExp("KindOfQuantityId", id.GetValue());
        }

    if (propertyChange.GetCategory().IsValid())
        {
        StringChange& change = propertyChange.GetCategory();
        if (change.GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("CategoryId");
        else
            {
            PropertyCategoryCP cat = newProperty.GetCategory();
  
            if (cat == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            PropertyCategoryId id = ctx.GetSchemaManager().GetPropertyCategoryId(*cat);
            if (!id.IsValid())
                {
                if (ImportPropertyCategory(ctx, *cat) != SUCCESS)
                    return ERROR;

                id = cat->GetId();
                }

            sqlUpdateBuilder.AddSetExp("CategoryId", id.GetValue());
            }
        }

    sqlUpdateBuilder.AddWhereExp("Id", propertyId.GetValue());
    if (sqlUpdateBuilder.IsValid())
        {
        if (sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            return ERROR;
        }

    return UpdateCustomAttributes(ctx, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property, propertyId, propertyChange.CustomAttributes(), oldProperty, newProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateRelationshipConstraint(Context& ctx, ECContainerId containerId, ECRelationshipConstraintChange& constraintChange, ECRelationshipConstraintCR oldConstraint, ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName)
    {
    Utf8CP constraintEndStr = isSource ? "Source" : "Target";
    SqlUpdateBuilder updater("ec_RelationshipConstraint");

    if (constraintChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (constraintChange.GetMultiplicity().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing 'Multiplicity' of an ECRelationshipConstraint is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.IsPolymorphic().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing flag 'IsPolymorphic' of an ECRelationshipConstraint is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.ConstraintClasses().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing the constraint classes is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.GetRoleLabel().IsValid())
        {
        updater.AddSetExp("RoleLabel", constraintChange.GetRoleLabel().GetNew().Value().c_str());
        }

    updater.AddWhereExp("RelationshipEnd", isSource ? ECRelationshipEnd_Source : ECRelationshipEnd_Target);
    updater.AddWhereExp("RelationshipClassId", containerId.GetValue());
    if (updater.ExecuteSql(ctx.GetECDb()) != SUCCESS)
        return ERROR;

    const SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType = isSource ? SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint : SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint;
    return UpdateCustomAttributes(ctx, containerType, containerId, constraintChange.CustomAttributes(), oldConstraint, newConstraint);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateCustomAttributes(Context& ctx, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, ECContainerId containerId, ECInstanceChanges& instanceChanges, IECCustomAttributeContainerCR oldContainer, IECCustomAttributeContainerCR newContainer)
    {
    int customAttributeIndex = 0;
    ECCustomAttributeInstanceIterable customAttributes = oldContainer.GetCustomAttributes(false);
    auto itor = customAttributes.begin();
    while (itor != customAttributes.end())
        {
        customAttributeIndex++;
        ++itor;
        }

    if (instanceChanges.Empty() || instanceChanges.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    for (size_t i = 0; i < instanceChanges.Count(); i++)
        {
        ECPropertyValueChange& change = instanceChanges.At(i);
        if (change.GetStatus() == ECChange::Status::Done)
            continue;

        bvector<Utf8String> tokens;
        BeStringUtilities::Split(change.GetId(), ":", tokens);
        if (tokens.size() != 2)
            return ERROR;

        Utf8StringCR schemaName = tokens[0];
        Utf8StringCR className = tokens[1];

        if (change.GetParent()->GetState() != ChangeState::New)
            {
            if (ctx.GetSchemaUpgradeCustomAttributeValidator().HasAnyRuleForSchema(schemaName))
                {
                if (ctx.GetSchemaUpgradeCustomAttributeValidator().Validate(change) == CustomAttributeValidator::Policy::Reject)
                    {
                    ctx.Issues().ReportV("ECSchema Upgrade failed. Adding or modifying %s custom attributes is not supported.", schemaName.c_str());
                    return ERROR;
                    }
                }
            }

        if (change.GetState() == ChangeState::New)
            {
            IECInstancePtr ca = newContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                return ERROR;

            if (ImportClass(ctx, ca->GetClass()) != SUCCESS)
                return ERROR;

            if (InsertCAEntry(ctx, *ca, ca->GetClass().GetId(), containerId, containerType, ++customAttributeIndex) != SUCCESS)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Deleted)
            {
            IECInstancePtr ca = oldContainer.GetCustomAttribute(schemaName, className);
            if (ca == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            BeAssert(ca->GetClass().HasId());
            int ordinal;
            if (DeleteCAEntry(ordinal, ctx, ca->GetClass().GetId(), containerId, containerType) != SUCCESS)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            IECInstancePtr ca = newContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                return ERROR;

            if (ImportClass(ctx, ca->GetClass()) != SUCCESS)
                return ERROR;

            if (ReplaceCAEntry(ctx, *ca, ca->GetClass().GetId(), containerId, containerType, 0) != SUCCESS)
                return ERROR;
            }

        change.SetStatus(ECChange::Status::Done);
        }

    instanceChanges.SetStatus(ECChange::Status::Done);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  07/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaWriter::IsChangeToBaseClassIsSupported(ECClassCR baseClass)
    {
    if (ECEntityClassCP entityClass = baseClass.GetEntityClassCP())
        {
        if (entityClass->IsMixin())
            {
            ECPropertyIterableCR propertyItor = baseClass.GetProperties(true);
            return  propertyItor.begin() == propertyItor.end();
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  07/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateBaseClasses(Context& ctx, BaseClassChanges& baseClassChanges, ECN::ECClassCR oldClass, ECN::ECClassCR newClass)
    {
    if (!baseClassChanges.IsValid())
        return SUCCESS;

    std::function<ECClassCP(ECClassCR, Utf8StringCR)> findBaseClass = [] (ECClassCR ecClass, Utf8StringCR qualifiedName)
        {
        ECClassCP baseClass = nullptr;
        for (ECClassCP bc : ecClass.GetBaseClasses())
            {
            if (qualifiedName == bc->GetFullName())
                {
                baseClass = bc;
                break;
                }
            }
       
        return baseClass;
        };

    bool overrideAllBaseClasses = false;
    for (size_t i = 0; i < baseClassChanges.Count(); i++)
        {
        auto& change = baseClassChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {

            ECClassCP oldBaseClass = findBaseClass(oldClass, change.GetOld().Value());
            if (oldBaseClass == nullptr)
                return ERROR;

            if (IsChangeToBaseClassIsSupported(*oldBaseClass))
                overrideAllBaseClasses = true;
            else
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Removing a base class from an ECClass is not supported.",
                                oldClass.GetFullName());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::New)
            {
            ECClassCP newBaseClass = findBaseClass(newClass, change.GetNew().Value());
            if (newBaseClass == nullptr)
                return ERROR;

            if (IsChangeToBaseClassIsSupported(*newBaseClass))
                overrideAllBaseClasses = true;
            else
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Adding a new base class to an ECClass is not supported.",
                                oldClass.GetFullName());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            ECClassCP newBaseClass = findBaseClass(newClass, change.GetNew().Value());
            if (newBaseClass == nullptr)
                return ERROR;

            ECClassCP oldBaseClass = findBaseClass(oldClass, change.GetOld().Value());
            if (oldBaseClass == nullptr)
                return ERROR;

            if (IsChangeToBaseClassIsSupported(*oldBaseClass) && IsChangeToBaseClassIsSupported(*newBaseClass))
                overrideAllBaseClasses = true;
            else
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Modifying the position of a base class in the list of base classes of an ECClass is not supported.",
                                oldClass.GetFullName());
                return ERROR;
                }
            }
        }
    ///Overide baseClasses
    if (overrideAllBaseClasses)
        {
        Statement stmt;
        if (stmt.Prepare(ctx.GetECDb(), "DELETE FROM main." TABLE_ClassHasBaseClasses " WHERE ClassId=?") != BE_SQLITE_OK)
            return ERROR;

        stmt.BindId(1, newClass.GetId());
        if (stmt.Step() != BE_SQLITE_DONE)
            return ERROR;

        int baseClassIndex = 0;
        for (ECClassCP baseClass : newClass.GetBaseClasses())
            {
            if (SUCCESS != ImportClass(ctx, *baseClass))
                return ERROR;

            if (SUCCESS != InsertBaseClassEntry(ctx, newClass.GetId(), *baseClass, baseClassIndex++))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateClass(Context& ctx, ClassChange& classChange, ECClassCR oldClass, ECClassCR newClass)
    {
    if (classChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (classChange.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of an ECClass is not supported.");
        return ERROR;
        }

    ECClassId classId = ctx.GetSchemaManager().GetClassId(newClass);
    if (!classId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecclass id");
        return ERROR;
        }

    SqlUpdateBuilder updateBuilder(TABLE_Class);

    if (classChange.GetClassModifier().IsValid())
        {
        Nullable<ECClassModifier> oldValue = classChange.GetClassModifier().GetOld();
        ECClassModifier newValue = classChange.GetClassModifier().GetNew().Value();
        if (oldValue == ECClassModifier::Abstract)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier from 'Abstract' to another value is not supported",
                            oldClass.GetFullName());

            return ERROR;
            }

        if (newValue == ECClassModifier::Sealed)
            {
            if (!newClass.GetDerivedClasses().empty())
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier to 'Sealed' is only valid if the class does not have derived classes.",
                                oldClass.GetFullName());

                return ERROR;
                }
            }
        else if (newValue == ECClassModifier::Abstract)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier to 'Abstract' is not supported.",
                            oldClass.GetFullName());

            return ERROR;
            }

        updateBuilder.AddSetExp("Modifier", Enum::ToInt(classChange.GetClassModifier().GetNew().Value()));
        }

    if (classChange.ClassType().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Changing the ECClassType of an ECClass is not supported.",
                        oldClass.GetFullName());
        return ERROR;
        }

    if (classChange.GetDisplayLabel().IsValid())
        {
        if (classChange.GetDisplayLabel().GetNew().IsNull())
            updateBuilder.AddSetToNull("DisplayLabel");
        else
            updateBuilder.AddSetExp("DisplayLabel", classChange.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (classChange.GetDescription().IsValid())
        {
        if (classChange.GetDescription().GetNew().IsNull())
            updateBuilder.AddSetToNull("Description");
        else
            updateBuilder.AddSetExp("Description", classChange.GetDescription().GetNew().Value().c_str());
        }

    if (classChange.GetRelationship().IsValid())
        {
        auto& relationshipChange = classChange.GetRelationship();
        if (relationshipChange.GetStrength().IsValid())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s: Changing the 'Strength' of an ECRelationshipClass is not supported.",
                            oldClass.GetFullName());
            return ERROR;
            }

        if (relationshipChange.GetStrengthDirection().IsValid())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s: Changing the 'StrengthDirection' of an ECRelationshipClass is not supported.",
                            oldClass.GetFullName());
            return ERROR;
            }

        ECRelationshipClassCP oldRel = oldClass.GetRelationshipClassCP();
        ECRelationshipClassCP newRel = newClass.GetRelationshipClassCP();
        BeAssert(oldRel != nullptr && newRel != nullptr);
        if (oldRel == nullptr && newRel == nullptr)
            return ERROR;

        if (relationshipChange.GetSource().IsValid())
            if (UpdateRelationshipConstraint(ctx, classId, relationshipChange.GetSource(), newRel->GetSource(), oldRel->GetSource(), true, oldRel->GetFullName()) == ERROR)
                return ERROR;

        if (relationshipChange.GetTarget().IsValid())
            if (UpdateRelationshipConstraint(ctx, classId, relationshipChange.GetTarget(), newRel->GetSource(), oldRel->GetTarget(), false, oldRel->GetFullName()) == ERROR)
                return ERROR;
        }

    updateBuilder.AddWhereExp("Id", classId.GetValue());
    if (updateBuilder.IsValid())
        {
        if (updateBuilder.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            return ERROR;
        }


    if (classChange.BaseClasses().IsValid())
        {
        if (UpdateBaseClasses(ctx, classChange.BaseClasses(), oldClass, newClass) != SUCCESS)
            return ERROR;
        }

    if (classChange.Properties().IsValid())
        {
        if (UpdateProperties(ctx, classChange.Properties(), oldClass, newClass) != SUCCESS)
            return ERROR;
        }

    return UpdateCustomAttributes(ctx, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, classId, classChange.CustomAttributes(), oldClass, newClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  05/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateProperties(Context& ctx, ECPropertyChanges& propertyChanges, ECClassCR oldClass, ECClassCR newClass)
    {
    int ordinal = (int) oldClass.GetPropertyCount(false);
    for (size_t i = 0; i < propertyChanges.Count(); i++)
        {
        auto& change = propertyChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ECPropertyCP oldProperty = oldClass.GetPropertyP(change.GetId(), false);
            if (oldProperty == nullptr)
                {
                BeAssert(false && "Failed to find property");
                return ERROR;
                }

            if (SUCCESS != DeleteProperty(ctx, change, *oldProperty))
                return ERROR;

            }
        else if (change.GetState() == ChangeState::New)
            {
            ECPropertyCP newProperty = newClass.GetPropertyP(change.GetName().GetNew().Value().c_str(), false);
            if (newProperty == nullptr)
                {
                BeAssert(false && "Failed to find the class");
                return ERROR;
                }

            if (SUCCESS != ImportProperty(ctx, *newProperty, ordinal))
                return ERROR;

            ordinal++;
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            ECPropertyCP oldProperty = oldClass.GetPropertyP(change.GetId(), false);
            ECPropertyCP newProperty = newClass.GetPropertyP(change.GetId(), false);
            if (oldProperty == nullptr)
                {
                BeAssert(false && "Failed to find property");
                return ERROR;
                }
            if (newProperty == nullptr)
                {
                BeAssert(false && "Failed to find property");
                return ERROR;
                }

            if (UpdateProperty(ctx, change, *oldProperty, *newProperty) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateSchemaReferences(Context& ctx, ReferenceChanges& referenceChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!referenceChanges.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < referenceChanges.Count(); i++)
        {
        auto& change = referenceChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            SchemaKey oldRef;
            if (SchemaKey::ParseSchemaFullName(oldRef, change.GetOld().Value().c_str()) != ECObjectsStatus::Success)
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to parse previous ECSchema reference name.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = SchemaPersistenceHelper::GetSchemaId(ctx.GetECDb(), DbTableSpace::Main(), oldRef.GetName().c_str(), SchemaLookupMode::ByName);
            Statement stmt;
            if (stmt.Prepare(ctx.GetECDb(), "DELETE FROM main." TABLE_SchemaReference " WHERE SchemaId=? AND ReferencedSchemaId=?") != BE_SQLITE_OK)
                return ERROR;

            stmt.BindId(1, oldSchema.GetId());
            stmt.BindId(2, referenceSchemaId);

            if (stmt.Step() != BE_SQLITE_DONE)
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to remove ECSchema reference %s.",
                                          oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::New)
            {
            SchemaKey newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to parse new ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Ensure schema exist
            if (!SchemaPersistenceHelper::TryGetSchemaKey(existingRef, ctx.GetECDb(), DbTableSpace::Main(), newRef.GetName().c_str()))
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Schema must exist with that or greater version
            if (!existingRef.Matches(newRef, SchemaMatchType::LatestWriteCompatible))
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Could not locate compatible referenced ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = SchemaPersistenceHelper::GetSchemaId(ctx.GetECDb(), DbTableSpace::Main(), newRef.GetName().c_str(), SchemaLookupMode::ByName);
            CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_SchemaReference "(SchemaId, ReferencedSchemaId) VALUES (?,?)");
            if (stmt == nullptr)
                return ERROR;

            stmt->BindId(1, oldSchema.GetId());
            stmt->BindId(2, referenceSchemaId);

            if (stmt->Step() != BE_SQLITE_DONE)
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to add new reference to ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            SchemaKey oldRef, newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(oldRef, change.GetOld().Value().c_str()) != ECObjectsStatus::Success)
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to parse previous ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to parse new ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Ensure schema exist and also get updated version number.
            if (!SchemaPersistenceHelper::TryGetSchemaKey(existingRef, ctx.GetECDb(), DbTableSpace::Main(), oldRef.GetName().c_str()))
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Schema must exist with that or greater version
            if (!existingRef.Matches(newRef, SchemaMatchType::LatestWriteCompatible))
                {
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Could not locate compatible referenced ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }

        change.SetStatus(ECChange::Status::Done);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaWriter::IsSpecifiedInRelationshipConstraint(Context& ctx, ECClassCR deletedClass)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("SELECT NULL FROM main.ec_RelationshipConstraintClass WHERE ClassId=? LIMIT 1");
    if (stmt == nullptr)
        {
        BeAssert(false && "SQL_SCHEMA_CHANGED");
        return true;
        }

    stmt->BindId(1, deletedClass.GetId());
    return BE_SQLITE_ROW == stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  08/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteCustomAttributeClass(Context& ctx, ECCustomAttributeClassCR deletedClass)
    {
    BeAssert(ctx.AreMajorSchemaVersionChangesAllowed() && ctx.IsMajorSchemaVersionChange(deletedClass.GetSchema().GetId()) && "Should have been checked before");

    Utf8StringCR schemaName = deletedClass.GetSchema().GetName();
    if (schemaName.EqualsI("ECDbMap") || schemaName.EqualsI("ECDbSchemaPolicies"))
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECCustomAttributeClass '%s' failed. Deleting ECCustomAttributeClass from system schemas are not supported.",
                        deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    //Add Type file to ensure we are deleting customattribute class.
    CachedStatementPtr stmt = ctx.GetCachedStatement("DELETE FROM main." TABLE_Class " WHERE Type=" SQLVAL_ECClassType_CustomAttribute " AND Id=?");
    stmt->BindId(1, deletedClass.GetId());
    if (stmt->Step() != BE_SQLITE_DONE)
        return ERROR;

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteClass(Context& ctx, ClassChange& classChange, ECClassCR deletedClass)
    {
    if (!ctx.AreMajorSchemaVersionChangesAllowed() || !ctx.IsMajorSchemaVersionChange(deletedClass.GetSchema().GetId()))
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Cannot delete ECClass '%s'. This is a major ECSchema change. Either major schema version changes are disabled "
                         "or the 'Read' version number of the ECSchema was not incremented.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    ECDerivedClassesList const* subClasses = ctx.GetECDb().Schemas().GetDerivedClassesInternal(deletedClass, "main");
    if (subClasses == nullptr)
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Could not delete ECClass '%s' because its subclasses failed to load.",
                         deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (!subClasses->empty())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' is not supported because it has subclasses.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsStructClass())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. ECStructClass cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (IsSpecifiedInRelationshipConstraint(ctx, deletedClass))
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. A class which is specified in a relationship constraint cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsCustomAttributeClass())
        return DeleteCustomAttributeClass(ctx, *deletedClass.GetCustomAttributeClassCP());

    ClassMap const* deletedClassMap = ctx.GetSchemaManager().GetClassMap(deletedClass);
    if (deletedClassMap == nullptr)
        {
        BeAssert(false && "Failed to find classMap");
        return ERROR;
        }

    if (MapStrategyExtendedInfo::IsForeignKeyMapping(deletedClassMap->GetMapStrategy()))
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Deleting ECRelationshipClass with ForeignKey mapping is not supported.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    //Delete all instances
    bool purgeECInstances = deletedClassMap->GetMapStrategy().IsTablePerHierarchy();
    if (purgeECInstances)
        {
        if (DeleteInstances(ctx, deletedClass) != SUCCESS)
            return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("DELETE FROM main.ec_Class WHERE Id=?");
    stmt->BindId(1, deletedClass.GetId());
    if (stmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false && "Failed to delete the class");
        return ERROR;
        }

    for (ECPropertyCP localProperty : deletedClass.GetProperties(false))
        {
        if (DeleteCustomAttributes(ctx, localProperty->GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        }

    if (auto relationshipClass = deletedClass.GetRelationshipClassCP())
        {
        if (DeleteCustomAttributes(ctx, relationshipClass->GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        if (DeleteCustomAttributes(ctx, relationshipClass->GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        }

    return DeleteCustomAttributes(ctx, deletedClass.GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  05/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteInstances(Context& ctx, ECClassCR deletedClass)
    {
    Utf8String ecsql("DELETE FROM ");
    ecsql.append(deletedClass.GetECSqlName());
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ctx.GetECDb(), ecsql.c_str(), ctx.GetECDb().GetImpl().GetSettingsManager().GetCrudWriteToken()))
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (BE_SQLITE_DONE != stmt.Step())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteCustomAttributes(Context& ctx, ECContainerId id, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType type)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("DELETE FROM main.ec_CustomAttribute WHERE ContainerId=? AND ContainerType=?");
    if (stmt == nullptr ||
        BE_SQLITE_OK != stmt->BindId(1, id) ||
        BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(type)) ||
        BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteProperty(Context& ctx, ECPropertyChange& propertyChange, ECPropertyCR deletedProperty)
    {
    ECClassCR ecClass = deletedProperty.GetClass();
    
    if (!ctx.AreMajorSchemaVersionChangesAllowed() || !ctx.IsMajorSchemaVersionChange(deletedProperty.GetClass().GetSchema().GetId()))
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Cannot delete ECProperty '%s.%s'. This is a major ECSchema change. Either major schema version changes are disabled "
                             "or the 'Read' version number of the ECSchema was not incremented.",
                             ecClass.GetSchema().GetFullSchemaName().c_str(), ecClass.GetName().c_str(), deletedProperty.GetName().c_str());
        return ERROR;
        }

    if (deletedProperty.GetIsNavigation())
        {
        //Blanket error. We do not check if relationship was also deleted. In that case we would allo nav deletion for shared column/ logical relationships
        //Fail we do not want to delete a sql column right now
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Deleting Navigation ECProperty '%s' from an ECClass is not supported.",
                        ecClass.GetFullName(), deletedProperty.GetName().c_str());
        return ERROR;
        }


    ClassMap const* classMap = ctx.GetSchemaManager().GetClassMap(ecClass);
    if (classMap == nullptr)
        {
        BeAssert(false && "Failed to find classMap");
        return ERROR;
        }

    bool sharedColumnFound = false;
    for (Partition const& partition : classMap->GetStorageDescription().GetHorizontalPartitions())
        {
        ECClassCP rootClass = ctx.GetSchemaManager().GetClass(partition.GetRootClassId());
        if (rootClass == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }
        ClassMap const* partitionRootClassMap = ctx.GetSchemaManager().GetClassMap(*rootClass);
        if (partitionRootClassMap == nullptr)
            {
            BeAssert(false && "Failed to find class map");
            return ERROR;
            }

        PropertyMap const* propertyMap = partitionRootClassMap->GetPropertyMaps().Find(deletedProperty.GetName().c_str());
        if (propertyMap == nullptr)
            {
            BeAssert(false && "Failed to find property map");
            return ERROR;
            }

        //Reject overridden property
        if (propertyMap->GetProperty().GetBaseProperty() != nullptr)
            {
            //Fail we do not want to delete a sql column right now
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Deleting an overridden ECProperty '%s' from an ECClass is not supported.",
                                      ecClass.GetFullName(), deletedProperty.GetName().c_str());
            return ERROR;
            }

        //Delete DbTable entries

        GetColumnsPropertyMapVisitor columnVisitor(PropertyMap::Type::Data);
        propertyMap->AcceptVisitor(columnVisitor);
        for (DbColumn const* column : columnVisitor.GetColumns())
            {
            //For shared column do not delete column itself.
            if (column->IsShared())
                {
                if (!sharedColumnFound)
                    sharedColumnFound = true;

                continue;
                }

            //For virtual column delete column from ec_Column.
            if (column->GetPersistenceType() == PersistenceType::Virtual || column->GetTable().GetType() == DbTable::Type::Virtual)
                {
                CachedStatementPtr stmt = ctx.GetCachedStatement("DELETE FROM main.ec_Column WHERE Id=?");
                if (stmt == nullptr ||
                    BE_SQLITE_OK != stmt->BindId(1, column->GetId()) ||
                    BE_SQLITE_DONE != stmt->Step())
                    {
                    BeAssert(false && "Failed to delete row from ec_Column");
                    return ERROR;
                    }
                }
            else
                {
                //Fail we do not want to delete a sql column right now
                ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Deleting ECProperty '%s' from an ECClass which is not mapped to a shared column is not supported.",
                                ecClass.GetFullName(), deletedProperty.GetName().c_str());
                return ERROR;
                }
            }
        }

    if (sharedColumnFound)
        {
        Utf8String ecsql;
        ecsql.Sprintf("UPDATE %s SET [%s]=NULL", ecClass.GetECSqlName().c_str(), deletedProperty.GetName().c_str());
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(ctx.GetECDb(), ecsql.c_str(), ctx.GetECDb().GetImpl().GetSettingsManager().GetCrudWriteToken()) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Deleting an ECProperty '%s' from an ECClass failed due error while setting property to null", ecClass.GetFullName(), deletedProperty.GetName().c_str());
            return ERROR;
            }
        }

    //Delete ECProperty entry make sure ec_Column is already deleted or set to null
    CachedStatementPtr stmt = ctx.GetCachedStatement("DELETE FROM main.ec_Property WHERE Id=?");
    if (stmt == nullptr ||
        BE_SQLITE_OK != stmt->BindId(1, deletedProperty.GetId()) ||
        BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false && "Failed to delete ecproperty");
        return ERROR;
        }

    return DeleteCustomAttributes(ctx, deletedProperty.GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateClasses(Context& ctx, ClassChanges& classChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!classChanges.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < classChanges.Count(); i++)
        {
        auto& change = classChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ECClassCP oldClass = oldSchema.GetClassCP(change.GetId());
            if (oldClass == nullptr)
                {
                BeAssert(false && "Failed to find class");
                return ERROR;
                }

            if (SUCCESS != DeleteClass(ctx, change, *oldClass))
                return ERROR;
            }
        else if (change.GetState() == ChangeState::New)
            {
            ECClassCP newClass = newSchema.GetClassCP(change.GetName().GetNew().Value().c_str());
            if (newClass == nullptr)
                {
                BeAssert(false && "Failed to find the class");
                return ERROR;
                }

            if (ImportClass(ctx, *newClass) == ERROR)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            ECClassCP oldClass = oldSchema.GetClassCP(change.GetId());
            ECClassCP newClass = newSchema.GetClassCP(change.GetId());
            if (oldClass == nullptr)
                {
                BeAssert(false && "Failed to find class");
                return ERROR;
                }
            if (newClass == nullptr)
                {
                BeAssert(false && "Failed to find class");
                return ERROR;
                }

            if (UpdateClass(ctx, change, *oldClass, *newClass) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateKindOfQuantities(Context& ctx, KindOfQuantityChanges& koqChanges, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema)
    {
    if (!koqChanges.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < koqChanges.Count(); i++)
        {
        KindOfQuantityChange& change = koqChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting KindOfQuantity from an ECSchema is not supported.",
                            oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetState() == ChangeState::New)
            {
            KindOfQuantityCP koq = newSchema.GetKindOfQuantityCP(change.GetId());
            if (koq == nullptr)
                {
                BeAssert(false && "Failed to find kind of quantity");
                return ERROR;
                }

            if (SUCCESS != ImportKindOfQuantity(ctx, *koq))
                return ERROR;

            continue;
            }

        if (change.GetState() == ChangeState::Modified)
            {
            KindOfQuantityCP oldKoq = oldSchema.GetKindOfQuantityCP(change.GetId());
            KindOfQuantityCP newKoq = newSchema.GetKindOfQuantityCP(change.GetId());
            if (oldKoq == nullptr || newKoq == nullptr)
                {
                BeAssert(oldKoq != nullptr && newKoq != nullptr);
                return ERROR;
                }

            if (SUCCESS != UpdateKindOfQuantity(ctx, change, *oldKoq, *newKoq))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle  03/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateKindOfQuantity(Context& ctx, KindOfQuantityChange& change, ECN::KindOfQuantityCR oldKoq, ECN::KindOfQuantityCR newKoq)
    {
    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of a KindOfQuantity is not supported.");
        return ERROR;
        }

    size_t actualChanges = 0;
    SqlUpdateBuilder sqlUpdateBuilder(TABLE_KindOfQuantity);
    if (change.GetDisplayLabel().IsValid())
        {
        actualChanges++;
        if (change.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (change.GetDescription().IsValid())
        {
        actualChanges++;
        if (change.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.GetDescription().GetNew().Value().c_str());
        }

    if (change.GetRelativeError().IsValid())
        {
        if (change.GetRelativeError().GetNew().IsNull())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. Removing the RelativeError of a KindOfQuantity is not valid. A KindOfQuantity must always have a RelativeError.");
            return ERROR;
            }

        actualChanges++;
        sqlUpdateBuilder.AddSetExp("RelativeError", change.GetRelativeError().GetNew().Value());
        }
    
    if (change.GetPresentationUnitList().IsValid())
        {
        actualChanges++;

        if (newKoq.GetPresentationFormats().empty())
            sqlUpdateBuilder.AddSetToNull("PresentationUnits");
        else
            {
            Utf8String presFormatsJson;
            if (SUCCESS != SchemaPersistenceHelper::SerializeKoqPresentationFormats(presFormatsJson, ctx.GetECDb(), newKoq))
                return ERROR;

            sqlUpdateBuilder.AddSetExp("PresentationUnits", presFormatsJson.c_str());
            }
        }

    if (change.ChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing properties of KindOfQuantity '%s' is not supported except for RelativeError, PresentationFormats, DisplayLabel and Description.",
                         oldKoq.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldKoq.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePropertyCategories(Context& ctx, PropertyCategoryChanges& changes, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema)
    {
    if (!changes.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < changes.Count(); i++)
        {
        PropertyCategoryChange& change = changes.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting PropertyCategory from an ECSchema is not supported.",
                            oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        
        if (change.GetState() == ChangeState::New)
            {
            PropertyCategoryCP cat = newSchema.GetPropertyCategoryCP(change.GetId());
            if (cat == nullptr)
                {
                BeAssert(false && "Failed to find property category");
                return ERROR;
                }

            if (ImportPropertyCategory(ctx, *cat) != SUCCESS)
                return ERROR;

            continue;
            }

        if (change.GetState() == ChangeState::Modified)
            {
            PropertyCategoryCP oldCat = oldSchema.GetPropertyCategoryCP(change.GetId());
            PropertyCategoryCP newCat = newSchema.GetPropertyCategoryCP(change.GetId());
            if (oldCat == nullptr || newCat == nullptr)
                {
                BeAssert(oldCat != nullptr && newCat != nullptr);
                return ERROR;
                }

            if (SUCCESS != UpdatePropertyCategory(ctx, change, *oldCat, *newCat))
                return ERROR;
            }
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle  03/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePropertyCategory(Context& ctx, PropertyCategoryChange& change, ECN::PropertyCategoryCR oldCat, ECN::PropertyCategoryCR newCat)
    {
    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of a PropertyCategory is not supported.");
        return ERROR;
        }

    size_t actualChanges = 0;

    SqlUpdateBuilder sqlUpdateBuilder(TABLE_PropertyCategory);
    if (change.GetDisplayLabel().IsValid())
        {
        actualChanges++;
        if (change.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (change.GetDescription().IsValid())
        {
        actualChanges++;
        if (change.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.GetDescription().GetNew().Value().c_str());
        }

    if (change.GetPriority().IsValid())
        {
        actualChanges++;
        if (change.GetPriority().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Priority");
        else
            sqlUpdateBuilder.AddSetExp("Priority", change.GetPriority().GetNew().Value());
        }

    if (change.ChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing properties of PropertyCategory '%s' is not supported except for Priority, DisplayLabel and Description.",
                         oldCat.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldCat.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  01/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateEnumeration(Context& ctx, ECEnumerationChange& enumChange, ECN::ECEnumerationCR oldEnum, ECN::ECEnumerationCR newEnum)
    {
    if (enumChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (enumChange.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of a ECEnumeration is not supported.");
        return ERROR;
        }

    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Enumeration);

    if (enumChange.GetTypeName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECEnumeration %s: 'Type' change is not supported.",
            oldEnum.GetFullName().c_str());

        return ERROR;
        }

    if (enumChange.IsStrict().IsValid())
        {
        if (enumChange.IsStrict().GetNew().IsNull())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECEnumeration %s: 'IsStrict' must always be set for an ECEnumeration.",
                oldEnum.GetFullName().c_str());

            return ERROR;
            }

        //Allow transition from "strict" to "non-strict" but not the other way around.
        if (enumChange.IsStrict().GetOld().Value() == true &&
            enumChange.IsStrict().GetNew().Value() == false)
            sqlUpdateBuilder.AddSetExp("IsStrict", enumChange.IsStrict().GetNew().Value());
        else
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECEnumeration %s: 'IsStrict' changed. 'Unstrict' cannot be change to 'strict'. The other way around is allowed.",
                oldEnum.GetFullName().c_str());

            return ERROR;
            }
        }

    if (enumChange.GetDisplayLabel().IsValid())
        {
        if (enumChange.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", enumChange.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (enumChange.GetDescription().IsValid())
        {
        if (enumChange.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", enumChange.GetDescription().GetNew().Value().c_str());
        }

    ECEnumeratorChanges& enumeratorChanges = enumChange.Enumerators();
    if (enumeratorChanges.IsValid())
        {
        if (SUCCESS != VerifyEnumeratorChanges(ctx, oldEnum, enumeratorChanges))
            return ERROR;
    
        Utf8String enumValueJson;
        if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(enumValueJson, newEnum))
            return ERROR;

        sqlUpdateBuilder.AddSetExp("EnumValues", enumValueJson.c_str());
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldEnum.GetId().GetValue());
    if (sqlUpdateBuilder.IsValid())
        {
        if (sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Enumerator name changes are only allowed for int enums when it has been changed from the meaningless auto-generated name that was applied during the conversion of the schema
// to EC3.2. This is a one-way opportunity to modify the auto-generated, meaningless names.
// @bsimethod                                                  Krischan.Eberle 01/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::VerifyEnumeratorChanges(Context& ctx, ECEnumerationCR oldEnum, ECEnumeratorChanges& enumeratorChanges)
    {
    //The schema comparer compares enumerators by their name. So it cannot detect a name change directly.
    //this algorithm is based on the assumption that a "deleted" enumerator in the old enum, and a "new" enumerator in the new enum
    //are the same if their integer value is the same.
    //this is only done for int enums. For string enums, enumerator name changes are not allowed at all.
    //Once the enumerator name changes are detected, any regular deleted enumerators are invalid, any regular new enumerators are fine.
    const bool isIntEnum = oldEnum.GetType() == PRIMITIVETYPE_Integer;
    bmap<int, ECEnumeratorChange*> deletedEnumerators, newEnumerators;
    for (size_t i = 0; i < enumeratorChanges.Count(); i++)
        {
        ECEnumeratorChange& change = enumeratorChanges.At(i);
        switch (change.GetState())
            {
                case ChangeState::New:
                    if (isIntEnum)
                        newEnumerators[change.GetInteger().GetNew().Value()] = &change;

                    continue;
                case ChangeState::Deleted:
                    if (isIntEnum)
                        deletedEnumerators[change.GetInteger().GetOld().Value()] = &change;
                    else
                        {
                        ctx.Issues().ReportV("ECSchema Upgrade failed. An enumerator was deleted from Enumeration %s which is not supported.", oldEnum.GetFullName().c_str());
                        return ERROR;
                        }

                    break;
                case ChangeState::Modified:
                    if (change.GetInteger().IsValid() || change.GetString().IsValid())
                        {
                        ctx.Issues().ReportV("ECSchema Upgrade failed. The value of one or more enumerators of Enumeration %s was modified which is not supported.", oldEnum.GetFullName().c_str());
                        return ERROR;
                        }

                    break;

                default:
                    BeAssert(false);
                    return ERROR;
            }
        }

    const uint32_t oldSchemaOriginalVersionMajor = oldEnum.GetSchema().GetOriginalECXmlVersionMajor();
    const uint32_t oldSchemaOriginalVersionMinor = oldEnum.GetSchema().GetOriginalECXmlVersionMinor();
    const bool enumeratorNameChangeAllowed = oldSchemaOriginalVersionMajor < 3 || (oldSchemaOriginalVersionMajor == 3 && oldSchemaOriginalVersionMinor < 2);

    //only need to iterate over deleted enumerators. Any new Enumerators which this might miss, is fine, as they are always supported
    for (bpair<int, ECEnumeratorChange*> const& kvPair : deletedEnumerators)
        {
        const int val = kvPair.first;
        //We consider this a name change as the int values are equal.
        if (enumeratorNameChangeAllowed && newEnumerators.find(val) != newEnumerators.end())
            continue; 

        //no counterpart with matching value found or old name is not the auto-generated EC3.2 conversion default name
        ctx.Issues().ReportV("ECSchema Upgrade failed. An enumerator was deleted from Enumeration %s which is not supported.", oldEnum.GetFullName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateEnumerations(Context& ctx, ECEnumerationChanges& enumChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!enumChanges.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < enumChanges.Count(); i++)
        {
        ECEnumerationChange& change = enumChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECEnumerations from an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetState() == ChangeState::New)
            {
            ECEnumerationCP ecEnum = newSchema.GetEnumerationCP(change.GetId());
            if (ecEnum == nullptr)
                {
                BeAssert(false && "Failed to find enum");
                return ERROR;
                }

            if (SUCCESS != ImportEnumeration(ctx, *ecEnum))
                return ERROR;

            continue;
            }

        if (change.GetState() == ChangeState::Modified)
            {
            ECEnumerationCP oldEnum = oldSchema.GetEnumerationCP(change.GetId());
            ECEnumerationCP newEnum = newSchema.GetEnumerationCP(change.GetId());
            BeAssert(oldEnum != nullptr && newEnum != nullptr);
            if (oldEnum == nullptr || newEnum == nullptr)
                {
                return ERROR;
                }

            if (UpdateEnumeration(ctx, change, *oldEnum, *newEnum) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePhenomena(Context& ctx, PhenomenonChanges& changes, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!changes.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < changes.Count(); i++)
        {
        PhenomenonChange& change = changes.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting Phenomena from an ECSchema is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetState() == ChangeState::New)
            {
            PhenomenonCP phen = newSchema.GetPhenomenonCP(change.GetId());
            if (phen == nullptr)
                {
                BeAssert(false && "Failed to find phenomenon");
                return ERROR;
                }

            if (SUCCESS != ImportPhenomenon(ctx, *phen))
                return ERROR;
            }

        if (change.GetState() == ChangeState::Modified)
            {
            PhenomenonCP oldVal = oldSchema.GetPhenomenonCP(change.GetId());
            PhenomenonCP newVal = newSchema.GetPhenomenonCP(change.GetId());
            if (oldVal == nullptr)
                {
                BeAssert(false && "Failed to find Phenomenon");
                return ERROR;
                }
            if (newVal == nullptr)
                {
                BeAssert(false && "Failed to find Phenomenon");
                return ERROR;
                }

            if (UpdatePhenomenon(ctx, change, *oldVal, *newVal) != SUCCESS)
                return ERROR;

            continue;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePhenomenon(Context& ctx, PhenomenonChange& change, ECN::PhenomenonCR oldVal, ECN::PhenomenonCR newVal)
    {
    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of a Phenomenon is not supported.");
        return ERROR;
        }

    size_t actualChanges = 0;
    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Phenomenon);
    if (change.GetDisplayLabel().IsValid())
        {
        actualChanges++;
        if (change.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (change.GetDescription().IsValid())
        {
        actualChanges++;
        if (change.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.GetDescription().GetNew().Value().c_str());
        }

    if (change.ChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing properties of Phenomenon '%s' is not supported except for DisplayLabel and Description.",
                         oldVal.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldVal.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateUnitSystems(Context& ctx, UnitSystemChanges& changes, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!changes.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < changes.Count(); i++)
        {
        UnitSystemChange& change = changes.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting UnitSystems from an ECSchema is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetState() == ChangeState::New)
            {
            UnitSystemCP system = newSchema.GetUnitSystemCP(change.GetId());
            if (system == nullptr)
                {
                BeAssert(false && "Failed to find unit system");
                return ERROR;
                }

            if (SUCCESS != ImportUnitSystem(ctx, *system))
                return ERROR;
            }

        if (change.GetState() == ChangeState::Modified)
            {
            UnitSystemCP oldVal = oldSchema.GetUnitSystemCP(change.GetId());
            UnitSystemCP newVal = newSchema.GetUnitSystemCP(change.GetId());
            if (oldVal == nullptr)
                {
                BeAssert(false && "Failed to find UnitSystem");
                return ERROR;
                }
            if (newVal == nullptr)
                {
                BeAssert(false && "Failed to find UnitSystem");
                return ERROR;
                }

            if (UpdateUnitSystem(ctx, change, *oldVal, *newVal) != SUCCESS)
                return ERROR;

            continue;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateUnitSystem(Context& ctx, UnitSystemChange& change, ECN::UnitSystemCR oldVal, ECN::UnitSystemCR newVal)
    {
    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of a UnitSystem is not supported.");
        return ERROR;
        }

    size_t actualChanges = 0;
    SqlUpdateBuilder sqlUpdateBuilder(TABLE_UnitSystem);
    if (change.GetDisplayLabel().IsValid())
        {
        actualChanges++;
        if (change.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (change.GetDescription().IsValid())
        {
        actualChanges++;
        if (change.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.GetDescription().GetNew().Value().c_str());
        }

    if (change.ChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing properties of UnitSystem '%s' is not supported except for DisplayLabel and Description.",
                         oldVal.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldVal.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateUnits(Context& ctx, UnitChanges& changes, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!changes.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < changes.Count(); i++)
        {
        UnitChange& change = changes.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting Units from an ECSchema is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetState() == ChangeState::New)
            {
            ECUnitCP unit = newSchema.GetUnitCP(change.GetId());
            if (unit == nullptr)
                {
                BeAssert(false && "Failed to find unit");
                return ERROR;
                }

            if (SUCCESS != ImportUnit(ctx, *unit))
                return ERROR;
            }

        if (change.GetState() == ChangeState::Modified)
            {
            ECUnitCP oldVal = oldSchema.GetUnitCP(change.GetId());
            ECUnitCP newVal = newSchema.GetUnitCP(change.GetId());
            if (oldVal == nullptr)
                {
                BeAssert(false && "Failed to find Unit");
                return ERROR;
                }
            if (newVal == nullptr)
                {
                BeAssert(false && "Failed to find Unit");
                return ERROR;
                }

            if (UpdateUnit(ctx, change, *oldVal, *newVal) != SUCCESS)
                return ERROR;

            continue;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateUnit(Context& ctx, UnitChange& change, ECN::ECUnitCR oldVal, ECN::ECUnitCR newVal)
    {
    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of a Unit is not supported.");
        return ERROR;
        }

    size_t actualChanges = 0;
    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Unit);
    if (change.GetDisplayLabel().IsValid())
        {
        actualChanges++;
        if (change.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (change.GetDescription().IsValid())
        {
        actualChanges++;
        if (change.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.GetDescription().GetNew().Value().c_str());
        }

    if (change.ChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing properties of Unit '%s' is not supported except for DisplayLabel and Description.",
                         oldVal.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldVal.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateFormats(Context& ctx, FormatChanges& changes, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!changes.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < changes.Count(); i++)
        {
        FormatChange& change = changes.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting Formats from an ECSchema is not supported.",
                                 oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetState() == ChangeState::New)
            {
            ECFormatCP format = newSchema.GetFormatCP(change.GetId());
            if (format == nullptr)
                {
                BeAssert(false && "Failed to find format");
                return ERROR;
                }

            if (SUCCESS != ImportFormat(ctx, *format))
                return ERROR;
            }

        if (change.GetState() == ChangeState::Modified)
            {
            ECFormatCP oldVal = oldSchema.GetFormatCP(change.GetId());
            ECFormatCP newVal = newSchema.GetFormatCP(change.GetId());
            if (oldVal == nullptr)
                {
                BeAssert(false && "Failed to find format");
                return ERROR;
                }
            if (newVal == nullptr)
                {
                BeAssert(false && "Failed to find format");
                return ERROR;
                }

            if (UpdateFormat(ctx, change, *oldVal, *newVal) != SUCCESS)
                return ERROR;

            continue;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateFormat(Context& ctx, FormatChange& change, ECN::ECFormatCR oldVal, ECN::ECFormatCR newVal)
    {
    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of a Format is not supported.");
        return ERROR;
        }

    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Format);
    if (change.GetDisplayLabel().IsValid())
        {
        if (change.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (change.GetDescription().IsValid())
        {
        if (change.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.GetDescription().GetNew().Value().c_str());
        }

    if (change.GetHasNumeric().IsValid())
        {
        if (change.GetHasNumeric().GetNew().IsNull() || !change.GetHasNumeric().GetNew().Value())
            sqlUpdateBuilder.AddSetToNull("NumericSpec");
        else
            sqlUpdateBuilder.AddSetExp("NumericSpec", SchemaPersistenceHelper::SerializeNumericSpec(*newVal.GetNumericSpec()).c_str());
        }

    if (change.GetHasComposite().IsValid())
        {
        if (change.GetCompositeMajorUnit().IsValid() || change.GetCompositeMiddleUnit().IsValid() || change.GetCompositeMinorUnit().IsValid() ||
            change.GetCompositeSubUnit().IsValid())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the composite units of Format '%s' is not supported.",
                                 oldVal.GetFullName().c_str());
            return ERROR;
            }

        if (change.GetCompositeSpacer().IsValid() || change.GetCompositeIncludeZero().IsValid())
            sqlUpdateBuilder.AddSetExp("CompositeSpec", SchemaPersistenceHelper::SerializeCompositeSpecWithoutUnits(*newVal.GetCompositeSpec()).c_str());
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldVal.GetId().GetValue());
    if (SUCCESS != sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()))
        return ERROR;

    if (change.GetCompositeMajorLabel().IsValid())
        {
        SqlUpdateBuilder sqlUpdateBuilder(TABLE_FormatCompositeUnit);
        if (change.GetCompositeMajorLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Label");
        else
            sqlUpdateBuilder.AddSetExp("Label", change.GetCompositeMajorLabel().GetNew().Value().c_str());

        sqlUpdateBuilder.AddWhereExp("FormatId", oldVal.GetId().GetValue());
        sqlUpdateBuilder.AddWhereExp("Ordinal", 0);
        if (SUCCESS != sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()))
            return ERROR;
        }

    if (change.GetCompositeMiddleLabel().IsValid())
        {
        SqlUpdateBuilder sqlUpdateBuilder(TABLE_FormatCompositeUnit);
        if (change.GetCompositeMiddleLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Label");
        else
            sqlUpdateBuilder.AddSetExp("Label", change.GetCompositeMiddleLabel().GetNew().Value().c_str());

        sqlUpdateBuilder.AddWhereExp("FormatId", oldVal.GetId().GetValue());
        sqlUpdateBuilder.AddWhereExp("Ordinal", 1);
        if (SUCCESS != sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()))
            return ERROR;
        }

    if (change.GetCompositeMinorLabel().IsValid())
        {
        SqlUpdateBuilder sqlUpdateBuilder(TABLE_FormatCompositeUnit);
        if (change.GetCompositeMinorLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Label");
        else
            sqlUpdateBuilder.AddSetExp("Label", change.GetCompositeMinorLabel().GetNew().Value().c_str());

        sqlUpdateBuilder.AddWhereExp("FormatId", oldVal.GetId().GetValue());
        sqlUpdateBuilder.AddWhereExp("Ordinal", 2);
        if (SUCCESS != sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()))
            return ERROR;
        }

    if (change.GetCompositeSubLabel().IsValid())
        {
        SqlUpdateBuilder sqlUpdateBuilder(TABLE_FormatCompositeUnit);
        if (change.GetCompositeSubLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Label");
        else
            sqlUpdateBuilder.AddSetExp("Label", change.GetCompositeSubLabel().GetNew().Value().c_str());

        sqlUpdateBuilder.AddWhereExp("FormatId", oldVal.GetId().GetValue());
        sqlUpdateBuilder.AddWhereExp("Ordinal", 3);
        if (SUCCESS != sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateSchema(Context& ctx, SchemaChange& schemaChange, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (schemaChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (schemaChange.GetName().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. Changing the name of an ECSchema is not supported.");
        return ERROR;
        }

    ECSchemaId schemaId = ctx.GetSchemaManager().GetSchemaId(newSchema);
    if (!schemaId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecschema id");
        return ERROR;
        }

    SqlUpdateBuilder updateBuilder(TABLE_Schema);

    if (schemaChange.GetDisplayLabel().IsValid())
        {
        if (schemaChange.GetDisplayLabel().GetNew().IsNull())
            updateBuilder.AddSetToNull("DisplayLabel");
        else
            updateBuilder.AddSetExp("DisplayLabel", schemaChange.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (schemaChange.GetDescription().IsValid())
        {
        if (schemaChange.GetDescription().GetNew().IsNull())
            updateBuilder.AddSetToNull("Description");
        else
            updateBuilder.AddSetExp("Description", schemaChange.GetDescription().GetNew().Value().c_str());
        }

    const bool readVersionHasChanged = schemaChange.GetVersionRead().IsValid();
    if (readVersionHasChanged)
        {
        if (schemaChange.GetVersionRead().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionRead().GetValue(ValueId::New).Value())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionRead' of an ECSchema is not supported.",
                                      oldSchema.GetName().c_str());
            return ERROR;
            }

        if (!ctx.AreMajorSchemaVersionChangesAllowed())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Major schema version changes are disabled.",
                                 oldSchema.GetName().c_str());
            return ERROR;
            }

        ctx.AddSchemaWithMajorVersionChange(oldSchema.GetId());
        updateBuilder.AddSetExp("VersionDigit1", schemaChange.GetVersionRead().GetNew().Value());
        }

    const bool writeVersionHasChanged = schemaChange.GetVersionWrite().IsValid();
    if (writeVersionHasChanged)
        {
        if (!readVersionHasChanged && schemaChange.GetVersionWrite().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionWrite().GetValue(ValueId::New).Value())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionWrite' of an ECSchema is not supported.",
                                      oldSchema.GetName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit2", schemaChange.GetVersionWrite().GetNew().Value());
        }

    if (schemaChange.GetVersionMinor().IsValid())
        {
        if (!readVersionHasChanged && !writeVersionHasChanged && schemaChange.GetVersionMinor().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionMinor().GetValue(ValueId::New).Value())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionMinor' of an ECSchema is not supported.",
                                      oldSchema.GetName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit3", schemaChange.GetVersionMinor().GetNew().Value());
        }

    if (schemaChange.GetAlias().IsValid())
        {
        ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Modifying the Alias is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (schemaChange.GetECVersion().IsValid())
        {
        if (schemaChange.GetECVersion().GetOld().Value() > schemaChange.GetECVersion().GetNew().Value())
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'ECVersion' of an ECSchema is not supported.",
                             oldSchema.GetName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("ECVersion", schemaChange.GetECVersion().GetNew().Value());
        }

    const bool originalVersionMajorHasChanged = schemaChange.GetOriginalECXmlVersionMajor().IsValid();
    if (originalVersionMajorHasChanged)
        {
        uint32_t newVal = schemaChange.GetOriginalECXmlVersionMajor().GetNew().Value();
        if (schemaChange.GetOriginalECXmlVersionMajor().GetOld().Value() > newVal)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'OriginalECXmlVersionMajor' of an ECSchema is not supported.",
                             oldSchema.GetName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("OriginalECXmlVersionMajor", newVal);
        }

    if (schemaChange.GetOriginalECXmlVersionMinor().IsValid())
        {
        uint32_t newVal = schemaChange.GetOriginalECXmlVersionMinor().GetNew().Value();

        //if the higher digits have changed, minor version may be decremented
        if (!originalVersionMajorHasChanged && schemaChange.GetOriginalECXmlVersionMinor().GetOld().Value() > newVal)
            {
            ctx.Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'OriginalECXmlVersionMinor' of an ECSchema is not supported.",
                             oldSchema.GetName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("OriginalECXmlVersionMinor", newVal);
        }


    updateBuilder.AddWhereExp("Id", schemaId.GetValue());//this could even be on name
    if (updateBuilder.IsValid())
        {
        if (updateBuilder.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            return ERROR;
        }

    schemaChange.SetStatus(ECChange::Status::Done);

    if (UpdateSchemaReferences(ctx, schemaChange.References(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdateEnumerations(ctx, schemaChange.Enumerations(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (SUCCESS != UpdatePhenomena(ctx, schemaChange.Phenomena(), oldSchema, newSchema))
        return ERROR;

    if (SUCCESS != UpdateUnitSystems(ctx, schemaChange.UnitSystems(), oldSchema, newSchema))
        return ERROR;

    if (SUCCESS != UpdateUnits(ctx, schemaChange.Units(), oldSchema, newSchema))
        return ERROR;

    if (SUCCESS != UpdateFormats(ctx, schemaChange.Formats(), oldSchema, newSchema))
        return ERROR;

    if (UpdateKindOfQuantities(ctx, schemaChange.KindOfQuantities(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdatePropertyCategories(ctx, schemaChange.PropertyCategories(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdateClasses(ctx, schemaChange.Classes(), oldSchema, newSchema) == ERROR)
        return ERROR;

    return UpdateCustomAttributes(ctx, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema, schemaId, schemaChange.CustomAttributes(), oldSchema, newSchema);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::CompareSchemas(Context& ctx, bvector<ECSchemaCP> const& dependencyOrderedPrimarySchemas)
    {
    if (dependencyOrderedPrimarySchemas.empty())
        {
        BeAssert(false);
        return ERROR;
        }

    BeAssert(ctx.GetExistingSchemas().empty() && ctx.GetSchemasToImport().empty());

    std::set<Utf8String> doneList;
    for (ECSchemaCP schema : dependencyOrderedPrimarySchemas)
        {
        Utf8String schemaFullName = schema->GetFullSchemaName();
        if (doneList.find(schemaFullName) != doneList.end())
            continue;

        doneList.insert(schemaFullName);

        if (ECSchemaCP existingSchema = ctx.GetSchemaManager().GetSchema(schema->GetName()))
            {
            if (existingSchema == schema)
                continue;

            ctx.AddExistingSchema(*existingSchema);
            }

        ctx.AddSchemaToImport(*schema);
        }

    if (!ctx.GetExistingSchemas().empty())
        {
        SchemaComparer comparer;
        //We do not require detail if schema is added or deleted the name and version suffice
        SchemaComparer::Options options = SchemaComparer::Options(SchemaComparer::AppendDetailLevel::Partial, SchemaComparer::AppendDetailLevel::Partial);
        if (SUCCESS != comparer.Compare(ctx.Changes(), ctx.GetExistingSchemas(), ctx.GetSchemasToImport(), options))
            return ERROR;

        std::set<Utf8CP, CompareIUtf8Ascii> schemaOfInterest;
        if (ctx.Changes().IsValid())
            {
            for (size_t i = 0; i < ctx.Changes().Count(); i++)
                {
                schemaOfInterest.insert(ctx.Changes().At(i).GetId());
                }
            }
        //Remove any irrelevant schemas
        auto importItor = ctx.GetSchemasToImportR().begin();
        while (importItor != ctx.GetSchemasToImportR().end())
            {
            if (schemaOfInterest.find((*importItor)->GetName().c_str()) == schemaOfInterest.end())
                importItor = ctx.GetSchemasToImportR().erase(importItor);
            else
                ++importItor;
            }

        //Remove any irrelevant schemas
        auto existingItor = ctx.GetExistingSchemasR().begin();
        while (existingItor != ctx.GetExistingSchemasR().end())
            {
            if (schemaOfInterest.find((*existingItor)->GetName().c_str()) == schemaOfInterest.end())
                existingItor = ctx.GetExistingSchemasR().erase(existingItor);
            else
                ++existingItor;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
//! WIP Only needed until ECObjects' ECSchema::Validate has become a const operation
// @bsimethod                                                 Krischan.Eberle      10/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool ValidateSchema(ECN::ECSchemaCR schema)
    {
    for (ECClassCP ecclass : schema.GetClasses())
        {
        if (!ecclass->Validate())
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ReloadSchemas(Context& ctx)
    {
    //cache names
    std::vector<Utf8String> existingSchemaNames, importingSchemaNames;
    for (ECSchemaCP schema : ctx.GetExistingSchemas())
        existingSchemaNames.push_back(schema->GetName());

    for (ECSchemaCP schema : ctx.GetSchemasToImport())
        importingSchemaNames.push_back(schema->GetName());

    ctx.ClearCache();

    for (Utf8StringCR name : existingSchemaNames)
        {
        ECSchemaCP schema = ctx.GetSchemaManager().GetSchema(name);
        if (schema == nullptr)
            {
            ctx.Issues().ReportV("Schema import failed. Failed to read schema %s from ECDb.", name.c_str());
            return ERROR;
            }

        if (!ValidateSchema(*schema))
            {
            ctx.Issues().ReportV("Schema import failed. Failed to validate previously imported schema %s.", name.c_str());
            return ERROR;
            }

        ctx.AddExistingSchema(*schema);
        }

    for (Utf8StringCR name : importingSchemaNames)
        {
        ECSchemaCP schema = ctx.GetSchemaManager().GetSchema(name);
        if (schema == nullptr)
            {
            ctx.Issues().ReportV("Schema import failed. Failed to read imported schema %s from ECDb.", name.c_str());
            return ERROR;
            }

        if (!ValidateSchema(*schema))
            {
            ctx.Issues().ReportV("Schema import failed. Failed to validate imported schema %s.", name.c_str());
            return ERROR;
            }

        ctx.AddSchemaToImport(*schema);
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
