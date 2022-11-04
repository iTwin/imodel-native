/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define RESERVED_PROPERTY_SchemaName "BisCore"
#define RESERVED_PROPERTY_ClassName "ReservedPropertyNames"
#define RESERVED_PROPERTY_Name "PropertyNames"
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DropSchemaResult SchemaWriter::DropSchema(Utf8StringCR schemaName, SchemaImportContext& ctx, bool logIssue) {
    auto getReferencedBySchemas = [&] (ECSchemaId id) {
        bvector<Utf8String> schemas;
        auto stmt = ctx.GetECDb().GetCachedStatement(R"(
            SELECT [ss].[Name]
            FROM   [ec_SchemaReference] [rc]
                JOIN [ec_Schema] [ss] ON [ss].[Id] = [rc].[SchemaId]
            WHERE  [rc].[ReferencedSchemaId] = ?;)");
        stmt->BindId(1, id);
        while(stmt->Step() == BE_SQLITE_ROW) {
            schemas.push_back(stmt->GetValueText(0));
        }
        return schemas;
    };
    // CustomAttribute has no forign key to container id, following method 
    // gather all customattributes for a given schema and delete them before 
    // schema or its mapping is deleted. 
    auto dropCustomAttributeInstanceAppliedToSchema =[&](ECSchemaId id) {
        auto stmt = ctx.GetECDb().GetCachedStatement(R"sql(
            with [all_schema_custom_attributes]([schema_id], [custom_attribute_id]) as(
                select 
                    [p].[schemaid], 
                    [ca].[id] [caid]
                from   (select 
                            [s].[Id] [schemaId], 
                            [s].[id] [container_id], 
                            1 [container_type]
                        from   [ec_schema] [s]
                        union
                        select 
                            [c].[schemaId], 
                            [c].[id] [container_id], 
                            30 [container_type]
                        from   [ec_class] [c]
                        union
                        select 
                            [c].[schemaid], 
                            [p].[id] [container_id], 
                            992 [container_type]
                        from   [ec_property] [p]
                            join [ec_class] [c] on [c].[Id] = [p].[classid]
                        union
                        select 
                            [c].[schemaid], 
                            [r].[id] [container_id], 
                            1024 [container_type]
                        from   [ec_RelationshipConstraint] [r]
                            join [ec_class] [c] on [c].[Id] = [r].[relationshipclassid]
                        where  [r].[RelationshipEnd] = 0
                        union
                        select 
                            [c].[schemaid], 
                            [r].[id] [container_id], 
                            2048 [container_type]
                        from   [ec_RelationshipConstraint] [r]
                            join [ec_class] [c] on [c].[Id] = [r].[relationshipclassid]
                        where  [r].[RelationshipEnd] = 1) p
                    join [ec_CustomAttribute] [ca] on [ca].[containerid] = [p].[container_id]
                            and [ca].[containertype] = [p].[container_type]
            )
            delete from [ec_customAttribute] where [id] in (select [custom_attribute_id] from   [all_schema_custom_attributes] where  [schema_id] = ?);
        )sql");
        stmt->BindId(1, id);
        return stmt->Step();
    };
    auto dropSchemaAndItsMapping = [&] (ECSchemaId id) {
        auto rc = dropCustomAttributeInstanceAppliedToSchema(id);
        if (rc != BE_SQLITE_DONE)
            return rc;

        auto stmt = ctx.GetECDb().GetCachedStatement("DELETE FROM ec_Schema WHERE Id = ?");
        stmt->BindId(1, id);
        return stmt->Step();
    };
    // make sure the schema exist
    auto schemaCP = ctx.GetECDb().Schemas().GetSchema(schemaName);
    if (schemaCP == nullptr) {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Drop ECSchema failed. ECSchema: Schema %s not found.", schemaName.c_str());        
        return DropSchemaResult(DropSchemaResult::Status::Error);
    }
    const auto schemaId =  schemaCP->GetId();
    // check if the schema is referenced by another schema.
    auto referencedBy = getReferencedBySchemas(schemaId);
    if (!referencedBy.empty()) {
        if (logIssue)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Drop ECSchema failed. ECSchema: Schema %s is referenced by other schemas (%s).",
                                 schemaName.c_str(), BeStringUtilities::Join(referencedBy, ",").c_str());
            }
        return DropSchemaResult(DropSchemaResult::Status::ErrorDeletedSchemaIsReferencedByAnotherSchema, std::move(referencedBy));
    }

    // find if there are any instances belong to schema that is about to be deleted.
    auto results = InstanceFinder::FindInstances(ctx.GetECDb(), schemaId, false);
    if (!results.IsEmpty()) {
        if (logIssue)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Drop ECSchema failed. ECSchema: Schema %s has instances. Make sure to delete them before dropping scheam.", schemaName.c_str());
            }
        return DropSchemaResult(DropSchemaResult::Status::ErrorDeleteSchemaHasClassesWithInstances, std::move(results));
    }

    // drop schema should cascade delete all property maps
    auto rc = dropSchemaAndItsMapping(schemaId);
    if (rc != BE_SQLITE_DONE) {
        if (logIssue)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Drop ECSchema failed. ECSchema: Schema %s fail to drop due to sqlite error %s.",
                                 schemaName.c_str(), BeSQLiteLib::GetLogError(rc).c_str());
            }
        return DropSchemaResult(DropSchemaResult::Status::Error);
    }

    // repopulate cache tables
    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(ctx.GetECDb())) {
        return DropSchemaResult(DropSchemaResult::Status::Error);
    }
    return DropSchemaResult(DropSchemaResult::Status::Success);
}
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
BentleyStatus SchemaWriter::ImportSchemas(bvector<ECN::ECSchemaCP>& schemasToMap, SchemaImportContext& schemaImportCtx, bvector<ECSchemaCP> const& schemasRaw)
    {
    PERFLOG_START("ECDb", "Schema import> Persist schemas");

    Context ctx(schemaImportCtx);
    bvector<ECSchemaCP> schemas;
    if (SUCCESS != ctx.PreprocessSchemas(schemas, schemasRaw))
        {
        LOG.debug("SchemaWriter::ImportSchemas - failed to PreprocessSchemas");
        return ERROR;
        }

    if (SUCCESS != CompareSchemas(ctx, schemas))
        {
        LOG.debug("SchemaWriter::ImportSchemas - failed to CompareSchemas");
        return ERROR;
        }

    if (ctx.GetSchemasToImport().empty())
        return SUCCESS;

    for (ECSchemaCP schema : ctx.GetSchemasToImport())
        {
        if (FeatureManager::SchemaRequiresProfileUpgrade(ctx.GetECDb(), *schema))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import schema '%s'. Current ECDb profile version (%s) only support schemas with EC version < 3.2. ECDb profile version upgrade is required to import schemas with EC Version >= 3.2.",
                                    schema->GetFullSchemaName().c_str(), ctx.GetECDb().GetECDbProfileVersion().ToString().c_str());
            return ERROR;
            }

        if (SUCCESS != ImportSchema(ctx, *schema))
            {
            LOG.debugv("SchemaWriter::ImportSchemas - failed to Import Schema %s", schema->GetFullSchemaName().c_str());
            return ERROR;
            }
        }

    if (SUCCESS != ctx.PostprocessSchemas(schemas, schemasRaw))
        {
        LOG.debug("SchemaWriter::ImportSchemas - failed to PostprocessSchemas");
        return ERROR;
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(ctx.GetECDb()))
        {
        LOG.debug("SchemaWriter::ImportSchemas - Failed to RepopulateClassHierarchyCacheTable");
        return ERROR;
        }

    if (SUCCESS != ReloadSchemas(ctx))
        {
        LOG.debug("SchemaWriter::ImportSchemas - Failed to ReloadSchemas");
        return ERROR;
        }

    schemasToMap.insert(schemasToMap.begin(), ctx.GetSchemasToImport().begin(), ctx.GetSchemasToImport().end());
    PERFLOG_FINISH("ECDb", "Schema import> Persist schemas");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportSchema(Context& ctx, ECN::ECSchemaCR ecSchema)
    {
    SchemaChange* schemaChange = ctx.GetDiff().GetSchemaChange(ecSchema.GetName());
    if (schemaChange != nullptr)
        {
        if (schemaChange->GetStatus() == ECChange::Status::Done)
            return SUCCESS;

        if (schemaChange->GetOpCode() == ECChange::OpCode::Modified)
            {
            ECSchemaCP existingSchema = nullptr;
            for (ECSchemaCP schema : ctx.GetExistingSchemas())
                {
                if (schema->GetName().Equals(schemaChange->GetChangeName()))
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

        if (schemaChange->GetOpCode() == ECChange::OpCode::Deleted)
            {
            schemaChange->SetStatus(ECChange::Status::Done);
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting an ECSchema is not supported.",
                                 ecSchema.GetName().c_str());
            return ERROR;
            }
        }

    if (ctx.GetSchemaManager().ContainsSchema(ecSchema.GetName()))
        return SUCCESS;

    if (SUCCESS != InsertSchemaEntry(ctx.GetECDb(), ecSchema))
        {
        DbResult lastErrorCode;
        ctx.GetECDb().GetLastError(&lastErrorCode);
        if (BE_SQLITE_CONSTRAINT_UNIQUE == lastErrorCode)
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECSchema '%s'. Alias '%s' is already used by an existing ECSchema.",
                                 ecSchema.GetFullSchemaName().c_str(), ecSchema.GetAlias().c_str());
        return ERROR;
        }

    if (SUCCESS != InsertSchemaReferenceEntries(ctx, ecSchema))
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECSchema references for ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    //enums must be imported before ECClasses as properties reference enums
    for (ECEnumerationCP ecEnum : ecSchema.GetEnumerations())
        {
        if (SUCCESS != ImportEnumeration(ctx, *ecEnum))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECEnumeration '%s'.", ecEnum->GetFullName().c_str());
            return ERROR;
            }
        }

    //Unit stuff must be imported before KOQs as KOQ reference them
    for (UnitSystemCP us : ecSchema.GetUnitSystems())
        {
        if (SUCCESS != ImportUnitSystem(ctx, *us))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import UnitSystem '%s'.", us->GetFullName().c_str());
            return ERROR;
            }
        }

    for (PhenomenonCP ph : ecSchema.GetPhenomena())
        {
        if (SUCCESS != ImportPhenomenon(ctx, *ph))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Phenomenon '%s'.", ph->GetFullName().c_str());
            return ERROR;
            }
        }

    for (ECUnitCP unit : ecSchema.GetUnits())
        {
        if (SUCCESS != ImportUnit(ctx, *unit))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Unit '%s'.", unit->GetFullName().c_str());
            return ERROR;
            }
        }

    for (ECFormatCP format : ecSchema.GetFormats())
        {
        if (SUCCESS != ImportFormat(ctx, *format))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Format '%s'.", format->GetFullName().c_str());
            return ERROR;
            }
        }

    //KOQs must be imported before ECClasses as properties reference KOQs
    for (KindOfQuantityCP koq : ecSchema.GetKindOfQuantities())
        {
        if (SUCCESS != ImportKindOfQuantity(ctx, *koq))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import KindOfQuantity '%s'.", koq->GetFullName().c_str());
            return ERROR;
            }
        }

    //PropertyCategories must be imported before ECClasses as properties reference PropertyCategories
    for (PropertyCategoryCP cat : ecSchema.GetPropertyCategories())
        {
        if (SUCCESS != ImportPropertyCategory(ctx, *cat))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import PropertyCategory '%s'.", cat->GetFullName().c_str());
            return ERROR;
            }
        }

    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        if (SUCCESS != ImportClass(ctx, *ecClass))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECClass '%s'.", ecClass->GetFullName());
            return ERROR;
            }
        }

    if (SUCCESS != ImportCustomAttributes(ctx, ecSchema, ECContainerId(ecSchema.GetId()), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema))
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import custom attributes of ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertSchemaReferenceEntries(Context& ctx, ECSchemaCR schema)
    {
    ECSchemaReferenceListCR references = schema.GetReferencedSchemas();
    if (references.empty())
        return SUCCESS;

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_SchemaReference "(SchemaId,ReferencedSchemaId,Id) VALUES(?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    for (auto const& kvPair : references)
        {
        ECSchemaCP reference = kvPair.second.get();
        if (!ctx.IsEC32AvailableInFile())
            {
            Context::LegacySchemaImportHelper::Action importAction = ctx.LegacySchemaImportHelper().GetImportAction(reference->GetSchemaKey());
            if (importAction == Context::LegacySchemaImportHelper::Action::Ignore)
                continue;
            }

        ECSchemaId referenceId = SchemaPersistenceHelper::GetSchemaId(ctx.GetECDb(), DbTableSpace::Main(), reference->GetName().c_str(), SchemaLookupMode::ByName);
        if (!referenceId.IsValid())
            {
            BeAssert(false && "All referenced schemas are expected to be imported before.");
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

        if (BE_SQLITE_OK != stmt->BindId(3, ctx.GetECDb().GetImpl().GetIdFactory().SchemaReference().NextId()))
            return ERROR;   

        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;

        stmt->Reset();
        stmt->ClearBindings();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportClass(Context& ctx, ECN::ECClassCR ecClass)
    {
    if (ctx.GetSchemaManager().GetClassId(ecClass).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(ecClass.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECClass '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ecClass.GetName().c_str(), ecClass.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import ECClass because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    if (ctx.AssertReservedPropertyPolicy(ecClass))
        {
        // BeAssert(false && "Reserved property policy failed");
        return ERROR;
        }

    //now import actual ECClass
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Class "(SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection,CustomAttributeContainerType,Id) VALUES(?,?,?,?,?,?,?,?,?,?)");
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

    if (BE_SQLITE_OK != stmt->BindId(10, ctx.GetECDb().GetImpl().GetIdFactory().Class().NextId()))
        return ERROR;

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

    for (ECPropertyCP ecProperty : ecClass.GetProperties(false))
        {
        const int propertyIndex = ctx.GetNextPropertyOrdinal(classId);
        if (SUCCESS != ImportProperty(ctx, *ecProperty, propertyIndex))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECProperty '%s' of ECClass '%s'.", ecProperty->GetName().c_str(), ecClass.GetFullName());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportEnumeration(Context& ctx, ECEnumerationCR ecEnum)
    {
    if (ctx.GetSchemaManager().GetEnumerationId(ecEnum).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(ecEnum.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECEnumeration '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ecEnum.GetName().c_str(), ecEnum.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import ECEnumeration because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Enumeration "(SchemaId,Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues,Id) VALUES(?,?,?,?,?,?,?,?)");
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
    if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(enumValueJson, ecEnum, ctx.IsEC32AvailableInFile()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(7, enumValueJson, Statement::MakeCopy::Yes))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(8, ctx.GetECDb().GetImpl().GetIdFactory().Enumeration().NextId()))
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportUnitSystem(Context& ctx, UnitSystemCR us)
    {
    if (!ctx.IsEC32AvailableInFile())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import UnitSystem '%s'. UnitSystems cannot be imported in a file that does not support EC3.2 yet.", us.GetFullName().c_str());
        return ERROR;
        }

    if (ctx.GetSchemaManager().GetUnitSystemId(us).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(us.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import UnitSystem '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", us.GetName().c_str(), us.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import UnitSystem because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_UnitSystem "(SchemaId,Name,DisplayLabel,Description,Id) VALUES(?,?,?,?,?)");
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

    if (us.GetIsDescriptionDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, us.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindId(5, ctx.GetECDb().GetImpl().GetIdFactory().UnitSystem().NextId()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const UnitSystemId usId = DbUtilities::GetLastInsertedId<UnitSystemId>(ctx.GetECDb());
    if (!usId.IsValid())
        return ERROR;

    const_cast<UnitSystemR>(us).SetId(usId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportPhenomenon(Context& ctx, PhenomenonCR ph)
    {
    if (!ctx.IsEC32AvailableInFile())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Phenomenon '%s'. Phenomena cannot be imported in a file that does not support EC3.2 yet.", ph.GetFullName().c_str());
        return ERROR;
        }

    if (ctx.GetSchemaManager().GetPhenomenonId(ph).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(ph.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Phenomenon '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ph.GetName().c_str(), ph.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import Phenomenon because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Phenomenon "(SchemaId,Name,DisplayLabel,Description,Definition,Id) VALUES(?,?,?,?,?,?)");
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

    if (ph.GetIsDescriptionDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ph.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ph.GetDefinition(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(6, ctx.GetECDb().GetImpl().GetIdFactory().Phenomenon().NextId()))
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportUnit(Context& ctx, ECUnitCR unit)
    {
    if (!ctx.IsEC32AvailableInFile())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Unit '%s'. Units cannot be imported in a file that does not support EC3.2 yet.", unit.GetFullName().c_str());
        return ERROR;
        }

    if (ctx.GetSchemaManager().GetUnitId(unit).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(unit.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Unit '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", unit.GetName().c_str(), unit.GetSchema().GetFullSchemaName().c_str());
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


    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Unit "(SchemaId,Name,DisplayLabel,Description,PhenomenonId,UnitSystemId,Definition,Numerator,Denominator,Offset,IsConstant,InvertingUnitId,Id) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)");
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
    const int idIx = 13;

    if (BE_SQLITE_OK != stmt->BindId(schemaIdParamIx, unit.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(nameParamIx, unit.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (unit.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(labelParamIx, unit.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (unit.GetIsDescriptionDefined())
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

    if (unit.HasDefinition())
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

    if (BE_SQLITE_OK != stmt->BindBoolean(isConstantParamIx, unit.IsConstant()))
        return ERROR;

    if (invertingUnit != nullptr)
        {
        if (BE_SQLITE_OK != stmt->BindId(invertingUnitIdParamIx, invertingUnit->GetId()))
            return ERROR;
        }
    
    if (BE_SQLITE_OK != stmt->BindId(idIx, ctx.GetECDb().GetImpl().GetIdFactory().Unit().NextId()))
        return ERROR;

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportFormat(Context& ctx, ECFormatCR format)
    {
    if (!ctx.IsEC32AvailableInFile())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Format '%s'. Formats cannot be imported in a file that does not support EC3.2 yet.", format.GetFullName().c_str());
        return ERROR;
        }

    if (ctx.GetSchemaManager().GetFormatId(format).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(format.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Format '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", format.GetName().c_str(), format.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import Format because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_Format "(SchemaId,Name,DisplayLabel,Description,NumericSpec,CompositeSpec,Id) VALUES(?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    const int schemaIdParamIx = 1;
    const int nameParamIx = 2;
    const int labelParamIx = 3;
    const int descParamIx = 4;
    const int numericSpecParamIx = 5;
    const int compositeSpecParamIx = 6;
    const int idx = 7;

    if (BE_SQLITE_OK != stmt->BindId(schemaIdParamIx, format.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(nameParamIx, format.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (format.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(labelParamIx, format.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (format.GetIsDescriptionDefined())
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
    
    if (BE_SQLITE_OK != stmt->BindId(idx, ctx.GetECDb().GetImpl().GetIdFactory().Format().NextId()))
        return ERROR;

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportFormatComposite(Context& ctx, ECFormatCR format, FormatId formatId)
    {
    BeAssert(ctx.IsEC32AvailableInFile());
    if (!format.HasComposite())
        return SUCCESS;

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_FormatCompositeUnit "(FormatId,Label,UnitId,Ordinal,Id) VALUES(?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    Formatting::CompositeValueSpecCR spec = *format.GetCompositeSpec();

    auto insertUnit = [&ctx] (CachedStatement& stmt, FormatId formatId, Nullable<Utf8String> label, ECUnitCR unit, int ordinal)
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

        if (BE_SQLITE_OK != stmt.BindId(5, ctx.GetECDb().GetImpl().GetIdFactory().FormatCompositeUnit().NextId()))
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportKindOfQuantity(Context& ctx, KindOfQuantityCR koq)
    {
    if (ctx.GetSchemaManager().GetKindOfQuantityId(koq).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(koq.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import KindOfQuantity '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", koq.GetName().c_str(), koq.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import KindOfQuantity because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_KindOfQuantity(SchemaId,Name,DisplayLabel,Description,RelativeError,PersistenceUnit,PresentationUnits,Id) VALUES(?,?,?,?,?,?,?,?)");
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
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import KindOfQuantity '%s'. It must have a persistence unit.", koq.GetFullName().c_str());
        return ERROR;
        }

    Utf8String persistenceUnitStr;
    if (ctx.IsEC32AvailableInFile())
        persistenceUnitStr = koq.GetPersistenceUnit()->GetQualifiedName(koq.GetSchema());
    else
        persistenceUnitStr = koq.GetDescriptorCache().first;

    BeAssert(!persistenceUnitStr.empty());
    if (BE_SQLITE_OK != stmt->BindText(6, persistenceUnitStr, Statement::MakeCopy::Yes))
        return ERROR;

    Utf8String presUnitsJsonStr;
    if (!koq.GetPresentationFormats().empty())
        {
        if (SUCCESS != SchemaPersistenceHelper::SerializeKoqPresentationFormats(presUnitsJsonStr, ctx.GetECDb(), koq, ctx.IsEC32AvailableInFile()))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindText(7, presUnitsJsonStr, Statement::MakeCopy::Yes))
            return ERROR;
        }
    
    if (BE_SQLITE_OK != stmt->BindId(8, ctx.GetECDb().GetImpl().GetIdFactory().KindOfQuantity().NextId()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const KindOfQuantityId koqId = DbUtilities::GetLastInsertedId<KindOfQuantityId>(ctx.GetECDb());
    if (!koqId.IsValid())
        return ERROR;

    const_cast<KindOfQuantityR>(koq).SetId(koqId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportPropertyCategory(Context& ctx, PropertyCategoryCR cat)
    {
    if (ctx.GetSchemaManager().GetPropertyCategoryId(cat).IsValid())
        return SUCCESS;

    if (!ctx.GetSchemaManager().GetSchemaId(cat.GetSchema()).IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import PropertyCategory '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", cat.GetName().c_str(), cat.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import PropertyCategory because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_PropertyCategory(SchemaId,Name,DisplayLabel,Description,Priority,Id) VALUES(?,?,?,?,?,?)");
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

    if (BE_SQLITE_OK != stmt->BindId(6, ctx.GetECDb().GetImpl().GetIdFactory().PropertyCategory().NextId()))
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportRelationshipClass(Context& ctx, ECN::ECRelationshipClassCP relationship)
    {
    const ECClassId relClassId = relationship->GetId();
    if (SUCCESS != ImportRelationshipConstraint(ctx, relClassId, relationship->GetSource(), ECRelationshipEnd_Source))
        return ERROR;

    return ImportRelationshipConstraint(ctx, relClassId, relationship->GetTarget(), ECRelationshipEnd_Target);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportRelationshipConstraint(Context& ctx, ECClassId relClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd end)
    {
    BeAssert(relClassId.IsValid());

    ECRelationshipConstraintId constraintId;
    if (SUCCESS != InsertRelationshipConstraintEntry(ctx, constraintId, relClassId, relationshipConstraint, end))
        return ERROR;

    BeAssert(constraintId.IsValid());
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_RelationshipConstraintClass(ConstraintId,ClassId,Id) VALUES(?,?,?)");
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

        if (BE_SQLITE_OK != stmt->BindId(3, ctx.GetECDb().GetImpl().GetIdFactory().RelationshipConstraintClass().NextId()))
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertRelationshipConstraintEntry(Context& ctx, ECRelationshipConstraintId& constraintId, ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_RelationshipConstraint(RelationshipClassId,RelationshipEnd,MultiplicityLowerLimit,MultiplicityUpperLimit,IsPolymorphic,RoleLabel,AbstractConstraintClassId,Id) VALUES(?,?,?,?,?,?,?,?)");
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
        if (BE_SQLITE_OK != stmt->BindText(6, relationshipConstraint.GetRoleLabel(), Statement::MakeCopy::Yes))
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

    if (BE_SQLITE_OK != stmt->BindId(8, ctx.GetECDb().GetImpl().GetIdFactory().RelationshipConstraint().NextId()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    constraintId = DbUtilities::GetLastInsertedId<ECRelationshipConstraintId>(ctx.GetECDb());
    BeAssert(constraintId.IsValid());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportProperty(Context& ctx, ECN::ECPropertyCR ecProperty, int ordinal)
    {
    //Local properties are expected to not be imported at this point as they get imported along with their class.
    BeAssert(!ctx.GetSchemaManager().GetPropertyId(ecProperty).IsValid());

    if (ecProperty.GetIsStruct())
        {
        if (SUCCESS != ImportClass(ctx, ecProperty.GetAsStructProperty()->GetType()))
            {
            LOG.errorv("Failed to import struct property '%s' from class '%s' because its struct type '%s' could not be imported",
                ecProperty.GetName().c_str(), ecProperty.GetClass().GetFullName(), ecProperty.GetAsStructProperty()->GetType().GetFullName());
            return ERROR;
            }
        }
    else if (ecProperty.GetIsStructArray())
        {
        if (SUCCESS != ImportClass(ctx, ecProperty.GetAsStructArrayProperty()->GetStructElementType()))
            {
            LOG.errorv("Failed to import struct array property '%s' from class '%s' because its struct type '%s' could not be imported",
                ecProperty.GetName().c_str(), ecProperty.GetClass().GetFullName(), ecProperty.GetAsStructArrayProperty()->GetStructElementType().GetFullName());
            return ERROR;
            }
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (SUCCESS != ImportClass(ctx, *ecProperty.GetAsNavigationProperty()->GetRelationshipClass()))
            {
            LOG.errorv("Failed to import navigation property '%s' from class '%s' because its relationship'%s' could not be imported",
                ecProperty.GetName().c_str(), ecProperty.GetClass().GetFullName(), ecProperty.GetAsNavigationProperty()->GetRelationshipClass()->GetFullName());
            return ERROR;
            }
        }

    //now insert the actual property
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_Property(ClassId,Name,DisplayLabel,Description,IsReadonly,Priority,Ordinal,Kind,"
                                                        "PrimitiveType,PrimitiveTypeMinLength,PrimitiveTypeMaxLength,PrimitiveTypeMinValue,PrimitiveTypeMaxValue,"
                                                        "EnumerationId,StructClassId,ExtendedTypeName,KindOfQuantityId,CategoryId,ArrayMinOccurs,ArrayMaxOccurs,NavigationRelationshipClassId,NavigationDirection,Id) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
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
    const int idIndex = 23;
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
            {
            LOG.errorv("Failed to insert struct property '%s' from class '%s' because its struct type '%s' does not have an id.\n\n Insert statement: '%s'",
                ecProperty.GetName().c_str(), ecProperty.GetClass().GetFullName(), ecProperty.GetAsStructProperty()->GetType().GetFullName(), stmt->GetSQL());
            return ERROR;
            }
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

    if (BE_SQLITE_OK != stmt->BindId(idIndex, ctx.GetECDb().GetImpl().GetIdFactory().Property().NextId()))
        return ERROR;

    DbResult stat = stmt->Step();
    if (BE_SQLITE_DONE != stat)
        {
        LOG.fatalv("Failed to insert property '%s' for class '%s' due to error: '%s'.  \n\nFailed Insert statement: '%s'",
            ecProperty.GetName().c_str(), ecProperty.GetClass().GetFullName(), ctx.GetECDb().GetLastError().c_str(), stmt->GetSQL());
        return ERROR;
        }

    const ECPropertyId propId = DbUtilities::GetLastInsertedId<ECPropertyId>(ctx.GetECDb());
    if (!propId.IsValid())
        return ERROR;

    const_cast<ECPropertyR>(ecProperty).SetId(propId);

    auto result = ImportCustomAttributes(ctx, ecProperty, ECContainerId(propId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
    if(result != SUCCESS)
      {
      return result;
      }

    ctx.ImportCtx().RemapManager().CollectRemapInfosFromNewProperty(ecProperty.GetClass(), ecProperty.GetName(), ecProperty.GetId());
    return result;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertSchemaEntry(ECDbCR ecdb, ECSchemaCR schema)
    {
    BeAssert(!schema.HasId());

    const bool supportsECVersion = FeatureManager::IsAvailable(ecdb, Feature::ECVersions);
    BeAssert(supportsECVersion || schema.OriginalECXmlVersionLessThan(ECVersion::V3_2) && "Only EC 3.1 schemas can be imported into a file not supporting EC 3.2 yet");
    Utf8CP sql = supportsECVersion ? "INSERT INTO main.ec_Schema(Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3,OriginalECXmlVersionMajor,OriginalECXmlVersionMinor,Id) VALUES(?,?,?,?,?,?,?,?,?,?)" :
        "INSERT INTO main.ec_Schema(Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3,Id) VALUES(?,?,?,?,?,?,?,?)";

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(sql);
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

    if (supportsECVersion)
        {
        //original version of 0.0 is considered an unset version
        if (schema.GetOriginalECXmlVersionMajor() > 0 || schema.GetOriginalECXmlVersionMinor() > 0)
            {
            //Persist uint32_t as int64 to not lose unsigned-ness
            if (BE_SQLITE_OK != stmt->BindInt64(8, (int64_t) schema.GetOriginalECXmlVersionMajor()))
                return ERROR;

            //Persist uint32_t as int64 to not lose unsigned-ness
            if (BE_SQLITE_OK != stmt->BindInt64(9, (int64_t) schema.GetOriginalECXmlVersionMinor()))
                return ERROR;
            }
        
        
        if (BE_SQLITE_OK != stmt->BindId(10, ecdb.GetImpl().GetIdFactory().Schema().NextId()))
            return ERROR;
        }
    else
        {
        if (BE_SQLITE_OK != stmt->BindId(8, ecdb.GetImpl().GetIdFactory().Schema().NextId()))
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertBaseClassEntry(Context& ctx, ECClassId ecClassId, ECClassCR baseClass, int ordinal)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_ClassHasBaseClasses(ClassId,BaseClassId,Ordinal,Id) VALUES(?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, baseClass.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, ordinal))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(4, ctx.GetECDb().GetImpl().GetIdFactory().ClassHasBaseClasses().NextId()))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
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

    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import schema. The ECProperty '%s.%s' has a minimum/maximum value of an unsupported type.",
                                                    prop.GetClass().GetFullName(), prop.GetName().c_str());
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyKindOfQuantity(Context& ctx, Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    if (!prop.IsKindOfQuantityDefinedLocally() || prop.GetKindOfQuantity() == nullptr)
        return SUCCESS;

    if (prop.GetIsNavigation())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import Navigation ECProperty '%s.%s' because a KindOfQuantity is assigned to it.", prop.GetClass().GetFullName(), prop.GetName().c_str());
        return ERROR;
        }

    KindOfQuantityCP koq = prop.GetKindOfQuantity();
    if (SUCCESS != ImportKindOfQuantity(ctx, *koq))
        return ERROR;

    BeAssert(koq->HasId());
    return stmt.BindId(paramIndex, koq->GetId()) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::InsertCAEntry(Context& ctx, IECInstanceR customAttribute, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, int ordinal)
    {
    CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main.ec_CustomAttribute(ContainerId,ContainerType,ClassId,Ordinal,Instance,Id) VALUES(?,?,?,?,?,?)");
    if (stmt == nullptr)
        {
        LOG.error("SchemaWriter::InsertCAEntry - failed to get CachedStatementPtr");
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        {
        LOG.errorv("SchemaWriter::InsertCAEntry - failed to BindId for containerId %s", containerId.IsValid() ? containerId.ToString().c_str() : "\"invalid\"");
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        {
        LOG.errorv("SchemaWriter::InsertCAEntry - failed to BindInt for containerType %d", Enum::ToInt(containerType));
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindId(3, ecClassId))
        {
        LOG.errorv("SchemaWriter::InsertCAEntry - failed to BindId for classId %s", ecClassId.IsValid() ? ecClassId.ToString().c_str() : "\"invalid\"");
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(4, ordinal))
        {
        LOG.errorv("SchemaWriter::InsertCAEntry - failed to BindInt for ordinal %d", ordinal);
        return ERROR;
        }

    Utf8String caXml;
    if (InstanceWriteStatus::Success != customAttribute.WriteToXmlString(caXml, false, //don't write XML description header as we only store an XML fragment
                                                                          true)) //store instance id for the rare cases where the client specified one.
        {
        LOG.errorv("SchemaWriter::InsertCAEntry - failed to write custom attribute to Xml for %s", customAttribute.GetClass().GetFullName());
        return ERROR;
        }


    if (BE_SQLITE_OK != stmt->BindText(5, caXml, Statement::MakeCopy::No))
        {
        LOG.errorv("SchemaWriter::InsertCAEntry - failed to BindText for CA Xml %s", caXml.c_str());
        return ERROR;
        }

    auto id = ctx.GetECDb().GetImpl().GetIdFactory().CustomAttribute().NextId();
    if (BE_SQLITE_OK != stmt->BindId(6, id))
        {
        LOG.errorv("SchemaWriter::InsertCAEntry - failed to BindId for custom attribute %s", id.IsValid() ? id.ToString().c_str() : "\"invalid\"");
        return ERROR;
        }

    DbResult status = stmt->Step();
    if (BE_SQLITE_DONE != status)
        {
        LOG.errorv("SchemaWriter::InsertCAEntry - Failed to execute statement. Return code: %d", (int) status);
        return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ReplaceCAEntry(Context& ctx, IECInstanceR customAttribute, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, int ordinal)
    {
    if (DeleteCAEntry(ordinal, ctx, ecClassId, containerId, containerType) != SUCCESS)
        return ERROR;

    return InsertCAEntry(ctx, customAttribute, ecClassId, containerId, containerType, ordinal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

bool persistenceUnitsMatch (KindOfQuantityCP koqA, KindOfQuantityCP koqB)
    {
    return koqA->GetPersistenceUnit()->GetFullName().EqualsIAscii(koqB->GetPersistenceUnit()->GetFullName());
    }

bool persistenceUnitMatches (KindOfQuantityCP koq, Utf8StringCR unitName)
    {
    return koq->GetPersistenceUnit()->GetQualifiedName(koq->GetSchema()).EqualsIAscii(unitName.c_str());
    }

bool SchemaWriter::UnitChangeAllowed (Context& ctx, ECPropertyCR oldProperty, ECPropertyCR newProperty)
    {
    KindOfQuantityCP oldKoq = oldProperty.GetKindOfQuantity();
    if (nullptr == oldKoq)
        return true;

    KindOfQuantityCP newKoq = newProperty.GetKindOfQuantity();
    if (nullptr != newKoq && persistenceUnitsMatch (oldKoq, newKoq))
        return true;

    IECInstancePtr ca = newProperty.GetCustomAttribute("SchemaUpgradeCustomAttributes", "AllowUnitChange");
    if (!ca.IsValid())
        return false;
    
    ECValue from, to;
    ca->GetValue(from, "From");
    ca->GetValue(to, "To");
    if (from.IsNull() || to.IsNull())
        {
        ctx.Issues().ReportV(IssueSeverity::Info, IssueCategory::BusinessProperties, IssueType::ECDbIssue, 
            "AllowUnitChange custom attribute applied to %s.%s malformed, it must have both 'From' and 'To' properties set.",
            oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return false;
        }
    
    if (!persistenceUnitMatches(oldKoq, from.ToString()))
        {
        ctx.Issues().ReportV(IssueSeverity::Info, IssueCategory::BusinessProperties, IssueType::ECDbIssue, 
            "AllowUnitChange custom attribute applied to %s.%s malformed, the 'From' value must match the persistence unit from the old KindOfQuantity.",
            oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return false;
        }
    
    if (nullptr == newKoq && to.ToString().Equals(""))
        return true;

    return persistenceUnitMatches(newKoq, to.ToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateProperty(Context& ctx, PropertyChange& propertyChange, ECPropertyCR oldProperty, ECPropertyCR newProperty)
    {
    if (propertyChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (propertyChange.Name().IsChanged())
        {
        if (ctx.IgnoreIllegalDeletionsAndModifications())
            {
            LOG.infov("Ignoring upgrade error: ECSchema Upgrade failed. Changing the name of an ECProperty '%s.%s' is not supported.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return SUCCESS;
            }
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of an ECProperty '%s.%s' is not supported.",
                                oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return ERROR;
        }

    ECPropertyId propertyId = ctx.GetSchemaManager().GetPropertyId(newProperty);
    if (!propertyId.IsValid())
        {
        BeAssert(false && "Failed to resolve ECPropertyId");
        return ERROR;
        }

    if (propertyChange.TypeName().IsChanged())
        {
        Utf8String error;
        if (!IsPropertyTypeChangeSupported(error, propertyChange.TypeName(), oldProperty, newProperty))
            {
            if (ctx.IgnoreIllegalDeletionsAndModifications())
                {
                LOG.infov("Ignoring upgrade error: %s", error.c_str());
                return SUCCESS;
                }
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, error.c_str());
            return ERROR;
            }
        }

    if (propertyChange.IsStruct().IsChanged() || propertyChange.IsStructArray().IsChanged() || propertyChange.IsPrimitive().IsChanged() ||
        propertyChange.IsPrimitiveArray().IsChanged() || propertyChange.IsNavigation().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECProperty %s.%s: Changing the kind of the ECProperty is not supported.",
                                  oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return ERROR;
        }

    if (propertyChange.ArrayMaxOccurs().IsChanged() || propertyChange.ArrayMinOccurs().IsChanged())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECProperty %s.%s: Changing 'MinOccurs' or 'MaxOccurs' for an Array ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

    if (propertyChange.NavigationRelationship().IsChanged())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'Relationship' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

    if (propertyChange.NavigationDirection().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'Direction' for a Navigation ECProperty is not supported.",
                             oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return ERROR;
        }

    SqlUpdateBuilder sqlUpdateBuilder("ec_Property");

    if (propertyChange.MinimumLength().IsChanged())
        {
        constexpr Utf8CP kPrimitiveTypeMinLength = "PrimitiveTypeMinLength";
        if (propertyChange.MinimumLength().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kPrimitiveTypeMinLength);
        else
            sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMinLength, propertyChange.MinimumLength().GetNew().Value());
        }

    if (propertyChange.MaximumLength().IsChanged())
        {
        constexpr Utf8CP kPrimitiveTypeMaxLength = "PrimitiveTypeMaxLength";
        if (propertyChange.MaximumLength().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kPrimitiveTypeMaxLength);
        else
            sqlUpdateBuilder.AddSetExp(kPrimitiveTypeMaxLength, propertyChange.MaximumLength().GetNew().Value());
        }

    if (propertyChange.MinimumValue().IsChanged())
        {
        constexpr Utf8CP kPrimitiveTypeMinValue = "PrimitiveTypeMinValue";
        if (propertyChange.MinimumValue().GetNew().IsNull() || propertyChange.MinimumValue().GetNew().Value().IsNull())
            sqlUpdateBuilder.AddSetToNull(kPrimitiveTypeMinValue);
        else
            {
            ECValueCR value = propertyChange.MinimumValue().GetNew().Value();
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
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'MinimumValue' to an unsupported type.",
                                oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
                return ERROR;
                }
            }
        }

    if (propertyChange.MaximumValue().IsChanged())
        {
        constexpr Utf8CP kPrimitiveTypeMaxValue = "PrimitiveTypeMaxValue";
        if (propertyChange.MaximumValue().GetNew().IsNull() || propertyChange.MaximumValue().GetNew().Value().IsNull())
            sqlUpdateBuilder.AddSetToNull(kPrimitiveTypeMaxValue);
        else
            {
            ECValueCR value = propertyChange.MaximumValue().GetNew().Value();
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
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'MaximumValue' to an unsupported type.",
                                oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
                return ERROR;
                }
            }
        }

    if (propertyChange.ExtendedTypeName().IsChanged())
        {
        constexpr Utf8CP kExtendedTypeName = "ExtendedTypeName";
        if (propertyChange.ExtendedTypeName().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kExtendedTypeName);
        else
            sqlUpdateBuilder.AddSetExp(kExtendedTypeName, propertyChange.ExtendedTypeName().GetNew().Value().c_str());
        }

    if (propertyChange.DisplayLabel().IsChanged())
        {
        constexpr Utf8CP kDisplayLabel = "DisplayLabel";
        if (propertyChange.DisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kDisplayLabel);
        else
            sqlUpdateBuilder.AddSetExp(kDisplayLabel, propertyChange.DisplayLabel().GetNew().Value().c_str());
        }

    if (propertyChange.Description().IsChanged())
        {
        constexpr Utf8CP kDescription = "Description";
        if (propertyChange.Description().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull(kDescription);
        else
            sqlUpdateBuilder.AddSetExp(kDescription, propertyChange.Description().GetNew().Value().c_str());
        }

    if (propertyChange.IsReadonly().IsChanged())
        sqlUpdateBuilder.AddSetExp("IsReadonly", propertyChange.IsReadonly().GetNew().Value());

    if (propertyChange.Priority().IsChanged())
        sqlUpdateBuilder.AddSetExp("Priority", propertyChange.Priority().GetNew().Value());

    if (propertyChange.Enumeration().IsChanged())
        {
        if (!newProperty.GetIsPrimitive() && !newProperty.GetIsPrimitiveArray())
            {
            BeAssert(false);
            return ERROR;
            }

        if (propertyChange.Enumeration().GetNew().IsNull())
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
                    {
                    LOG.debugv("SchemaWriter::UpdateProperty - Failed to ImportEnumeration %s", enumCP->GetFullName().c_str());
                    return ERROR;
                    }

                id = enumCP->GetId();
                }

            sqlUpdateBuilder.AddSetToNull("PrimitiveType");
            sqlUpdateBuilder.AddSetExp("EnumerationId", id.GetValue());
            }
        }

    if (propertyChange.KindOfQuantity().IsChanged())
        {
        StringChange& change = propertyChange.KindOfQuantity();

        if (!UnitChangeAllowed(ctx, oldProperty, newProperty))
            {
            if (ctx.IgnoreIllegalDeletionsAndModifications())
                {
                ctx.Issues().ReportV(IssueSeverity::Info, IssueCategory::BusinessProperties, IssueType::ECDbIssue, 
                    "Changes to ECProperty %s.%s skipped because %s a property is not supported without an appropriate 'AllowUnitChange' custom attribute applied the property.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str(), change.GetNew().IsNull() ? "removing a KindOfQuantity from" : "changing the persistence unit of");
                return SUCCESS;
                }
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, 
                "ECSchema Upgrade failed. ECProperty %s.%s: %s a property is not supported without an appropriate 'AllowUnitChange' custom attribute applied the property.",
                        oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str(), change.GetNew().IsNull() ? "Removing a KindOfQuantity from" : "Changing the persistence unit of");
            return ERROR;
            }

        if (change.GetNew().IsNull())
            {
            sqlUpdateBuilder.AddSetToNull("KindOfQuantityId");
            }
        else
            {
            KindOfQuantityCP newKoq = newProperty.GetKindOfQuantity();
            if (newKoq == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            KindOfQuantityId id = ctx.GetSchemaManager().GetKindOfQuantityId(*newKoq);
            if (!id.IsValid())
                {
                if (ImportKindOfQuantity(ctx, *newKoq) != SUCCESS)
                    {
                    LOG.debugv("SchemaWriter::UpdateProperty - Failed to ImportKindOfQuantity %s", newKoq->GetFullName().c_str());
                    return ERROR;
                    }

                id = newKoq->GetId();
                }

            sqlUpdateBuilder.AddSetExp("KindOfQuantityId", id.GetValue());
            }
        }

    if (propertyChange.Category().IsChanged())
        {
        StringChange& change = propertyChange.Category();
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
                    {
                    LOG.debugv("SchemaWriter::UpdateProperty - Failed to ImportPropertyCategory %s", cat->GetFullName().c_str());
                    return ERROR;
                    }

                id = cat->GetId();
                }

            sqlUpdateBuilder.AddSetExp("CategoryId", id.GetValue());
            }
        }

    sqlUpdateBuilder.AddWhereExp("Id", propertyId.GetValue());
    if (sqlUpdateBuilder.IsValid())
        {
        if (sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            {
            LOG.debug("SchemaWriter::UpdateProperty - Failed to ExecuteSql");
            return ERROR;
            }
        }

    if (SUCCESS != UpdateCustomAttributes(ctx, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property, propertyId, propertyChange.CustomAttributes(), oldProperty, newProperty))
        {
        LOG.debug("SchemaWriter::UpdateProperty - Failed to UpdateCustomAttributes");
        return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateRelationshipConstraint(Context& ctx, ECContainerId containerId, RelationshipConstraintChange& constraintChange, ECRelationshipConstraintCR oldConstraint, ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName)
    {
    Utf8CP constraintEndStr = isSource ? "Source" : "Target";

    if (constraintChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (constraintChange.Multiplicity().IsChanged())
        {
        /*  SCHEMA_EVOLUTION_RULE: Allow 'ECRelationshipConstraint' to change property 'Multiplicity' such that upper limit can be increased but not decreased.
            Notes: In case of FK we donot allow multiplicity to change in way that can effect mapping.
            Rules:
                1. Lower limit of multiplicity cannot be changed.
                2. Upper limit of multiplicity can be chanegd as long as the new value is greater then current value.
                3. FK relationship is only allowed to change Upper limit of multiplicity for source/target constraint as long as that is the side where FK is persisted.
        */

        // Rule 1: Lower limit of multiplicity cannot be changed.
        if (oldConstraint.GetMultiplicity().GetLowerLimit() != newConstraint.GetMultiplicity().GetLowerLimit())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing 'Multiplicity' lower limit of an ECRelationshipConstraint is not supported",
                relationshipName, constraintEndStr);
            return ERROR;

            }
        // Rule 2: Upper limit of multiplicity can be chanegd as long as the new value is greater then current value.
        if (oldConstraint.GetMultiplicity().GetUpperLimit() > newConstraint.GetMultiplicity().GetUpperLimit())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing 'Multiplicity' upper limit to be less than its current value is not supported.",
                relationshipName, constraintEndStr);
            return ERROR;
            }
        const auto relMap = ctx.GetSchemaManager().GetClassMap(oldConstraint.GetRelationshipClass());
        if (relMap == nullptr)
            {
            BeDataAssert(relMap != nullptr && "ClassMap for existing relationship must exist");
            return ERROR;
            }            
        // Rule 3: FK relationship is only allowed to change Upper limit of multiplicity for source/target constraint as long as that is the side where FK is persisted.
        if (relMap->GetType() == ClassMap::Type::RelationshipEndTable)
            {
            if (isSource && relMap->GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInTargetTable)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing 'Multiplicity' of an Source ECRelationshipConstraint is not supported if relationship is mapped as 'ForeignKeyRelationshipInTargetTable'",
                    relationshipName, constraintEndStr);
                return ERROR;
                }
            if (!isSource && relMap->GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing 'Multiplicity' of an Target ECRelationshipConstraint is not supported if relationship is mapped as 'ForeignKeyRelationshipInSourceTable'",
                    relationshipName, constraintEndStr);
                return ERROR;
                }
            }

        //Just in case if someone change above rules
        BeAssert(newConstraint.GetMultiplicity().GetUpperLimit() > oldConstraint.GetMultiplicity().GetUpperLimit() &&
            newConstraint.GetMultiplicity().GetLowerLimit() == oldConstraint.GetMultiplicity().GetLowerLimit());

        // Action: Update upper bound for multiplicity for the constraint.
        SqlUpdateBuilder updater(TABLE_RelationshipConstraint);
        updater.AddSetExp("MultiplicityUpperLimit", newConstraint.GetMultiplicity().GetUpperLimit());
        updater.AddWhereExp("RelationshipEnd", isSource ? ECRelationshipEnd_Source : ECRelationshipEnd_Target);
        updater.AddWhereExp("RelationshipClassId", containerId.GetValue());
        if (updater.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            return ERROR;

        return SUCCESS;
        }

    if (constraintChange.IsPolymorphic().IsChanged())
        {
        /*  SCHEMA_EVOLUTION_RULE: Allow 'ECRelationshipConstraint' to change property 'IsPolymorphic' from false -> true.
            Notes: This rule would be applied to relationship that is mapped using LinkTable or EndTable map strategy.
        */
        if (!constraintChange.IsPolymorphic().GetOld().Value() && constraintChange.IsPolymorphic().GetNew().Value())
            {
            SqlUpdateBuilder updater(TABLE_RelationshipConstraint);
            updater.AddSetExp("IsPolymorphic", constraintChange.IsPolymorphic().GetNew().Value());
            updater.AddWhereExp("RelationshipEnd", isSource ? ECRelationshipEnd_Source : ECRelationshipEnd_Target);
            updater.AddWhereExp("RelationshipClassId", containerId.GetValue());
            if (updater.ExecuteSql(ctx.GetECDb()) != SUCCESS)
                return ERROR;
            }
        else
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing flag 'IsPolymorphic' of  from 'true' to 'false' is not supported.", relationshipName, constraintEndStr);
            return ERROR;
            }
        }
    if (constraintChange.ConstraintClasses().IsChanged())
        {
        /*  SCHEMA_EVOLUTION_RULE: Allow 'ECRelationshipConstraintClass' to change from child to parent.
            Notes: This rule would be applied to relationship that is mapped using LinkTable or EndTable map strategy.
        */
        const ECClassCP newConstraintClass = newConstraint.GetConstraintClasses().front();
        ECClassCP resolvedNewConstraintClass = ctx.GetSchemaManager().GetClass(newConstraintClass->GetSchema().GetName(), newConstraintClass->GetName(), SchemaLookupMode::ByName);
        if (resolvedNewConstraintClass == nullptr)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: New constraint class '%s' must be already present in ECDb.",
                                 relationshipName, constraintEndStr, newConstraintClass->GetFullName());
            return ERROR;
            }
        ECSchemaCR newSchema = newConstraintClass->GetSchema();
        for (auto oldConstraintClass : oldConstraint.GetConstraintClasses())
            {
            ECClassCP resolvedOldConstraintClass = nullptr;
            // old constraint in different schema than new constraint
            if (!oldConstraintClass->GetSchema().GetName().EqualsIAscii(newSchema.GetName()))
                {
                auto const schemas = ctx.GetSchemasToImport();
                for (auto const importedSchema : schemas)
                    if (oldConstraintClass->GetSchema().GetName().EqualsIAscii(importedSchema->GetName()))
                        resolvedOldConstraintClass = importedSchema->LookupClass(oldConstraintClass->GetFullName(), true);

                if (resolvedOldConstraintClass == nullptr)
                    {
                    resolvedOldConstraintClass = ctx.GetSchemaManager().GetClass(oldConstraintClass->GetId());
                    }
                }
            else
                resolvedOldConstraintClass = newSchema.GetClassCP(oldConstraintClass->GetName().c_str());
            
            if (resolvedOldConstraintClass == nullptr)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Old constraint class '%s' not found in updated schema.",
                                     relationshipName, constraintEndStr, oldConstraintClass->GetFullName());
                return ERROR;
                }
            if (!newConstraint.SupportsClass(*resolvedOldConstraintClass))
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: New constraints must support Old constraint class '%s'.",
                                     relationshipName, constraintEndStr, oldConstraintClass->GetFullName());
                return ERROR;
                }
            }

        auto stmt = ctx.GetECDb().GetCachedStatement("SELECT Id FROM " TABLE_RelationshipConstraint " WHERE RelationshipEnd=? AND RelationshipClassId=?");
        stmt->BindInt(1, isSource ? ECRelationshipEnd_Source : ECRelationshipEnd_Target);
        stmt->BindId(2, containerId);
        if (stmt->Step() != BE_SQLITE_ROW)
            {
            BeAssert(false);
            return ERROR;
            }

        auto constraintId = stmt->GetValueUInt64(0);
        SqlUpdateBuilder updater(TABLE_RelationshipConstraintClass);
        updater.AddSetExp("ClassId", resolvedNewConstraintClass->GetId().GetValue());
        updater.AddWhereExp("ConstraintId", constraintId);
        if (updater.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            return ERROR;
        }

    if (constraintChange.RoleLabel().IsChanged())
        {
        SqlUpdateBuilder updater(TABLE_RelationshipConstraint);
        updater.AddSetExp("RoleLabel", constraintChange.RoleLabel().GetNew().Value().c_str());
        updater.AddWhereExp("RelationshipEnd", isSource ? ECRelationshipEnd_Source : ECRelationshipEnd_Target);
        updater.AddWhereExp("RelationshipClassId", containerId.GetValue());
        if (updater.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            return ERROR;
        }

    const SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType = isSource ? SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint : SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint;

    // containerId is the ECRelationshipClass - we need the id of the constraint
    CachedStatementPtr stmt = ctx.GetCachedStatement("SELECT Id FROM main." TABLE_RelationshipConstraint " WHERE RelationshipClassId = ? AND RelationshipEnd = ?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, containerId);
    stmt->BindInt(2, isSource ? 0 : 1);

    if (stmt->Step() != BE_SQLITE_ROW)
        {
        return ERROR;
        }
    ECContainerId constraintId = stmt->GetValueId<ECContainerId>(0);

    return UpdateCustomAttributes(ctx, containerType, constraintId, constraintChange.CustomAttributes(), oldConstraint, newConstraint);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateCustomAttributes(Context& ctx, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, ECContainerId containerId, CustomAttributeChanges& caChanges, IECCustomAttributeContainerCR oldContainer, IECCustomAttributeContainerCR newContainer)
    {
    int customAttributeIndex = 0;
    ECCustomAttributeInstanceIterable customAttributes = oldContainer.GetCustomAttributes(false);
    auto itor = customAttributes.begin();
    while (itor != customAttributes.end())
        {
        customAttributeIndex++;
        ++itor;
        }

    if (caChanges.IsEmpty() || caChanges.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    BeAssert(caChanges.GetParent() != nullptr);
    const bool caContainerIsNew = caChanges.GetParent()->GetOpCode() == ECChange::OpCode::New;

    for (size_t i = 0; i < caChanges.Count(); i++)
        {
        CustomAttributeChange& change = caChanges[i];
        if (change.GetStatus() == ECChange::Status::Done)
            continue;

        Utf8String schemaName, className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, change.GetChangeName()))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. CustomAttribute change on container '%s' must have fully qualified class name, but was '%s'",
                                    oldContainer.GetContainerName().c_str(), newContainer.GetContainerName().c_str());
            return ERROR;
            }

        if (!caContainerIsNew)
            {
            //only validate CA rules, if the container has not just been added with this schema import/update
            if (ctx.GetSchemaUpgradeCustomAttributeValidator().Validate(change) == CustomAttributeValidator::Policy::Reject)
                {
                Utf8String changeString = change.ToString();
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Adding or modifying custom attribute '%s' is not supported. \nContainer: %s.  \n Changes: %s",
                                        change.GetChangeName(), oldContainer.GetContainerName().c_str(), changeString.c_str());
                return ERROR;
                }
            }

        if (change.GetOpCode() == ECChange::OpCode::New)
            {
            IECInstancePtr ca = newContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                {
                // Not sure why the SchemaComparer found the new CA but it isn't here now; however, we can't fail the import because of this.
                if (ctx.IgnoreIllegalDeletionsAndModifications())
                    continue;
                return ERROR;
                }
            if (ImportClass(ctx, ca->GetClass()) != SUCCESS)
                {
                LOG.debugv("SchemaWriter::UpdateCustomAttributes - Failed to ImportClass %s", ca->GetClass().GetFullName());
                return ERROR;
                }

            if (InsertCAEntry(ctx, *ca, ca->GetClass().GetId(), containerId, containerType, ++customAttributeIndex) != SUCCESS)
                {
                LOG.debugv("SchemaWriter::UpdateCustomAttributes - Failed to InsertCAEntry for %s on %s", ca->GetClass().GetFullName(), newContainer.GetContainerName().c_str());
                return ERROR;
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            IECInstancePtr ca = oldContainer.GetCustomAttribute(schemaName, className);
            if (ca == nullptr)
                {
                if (ctx.IgnoreIllegalDeletionsAndModifications())
                    continue;

                BeAssert(false);
                return ERROR;
                }

            BeAssert(ca->GetClass().HasId());
            int ordinal;
            if (DeleteCAEntry(ordinal, ctx, ca->GetClass().GetId(), containerId, containerType) != SUCCESS)
                {
                LOG.debugv("SchemaWriter::UpdateCustomAttributes - Failed to DeleteCAEntry for %s", ca->GetClass().GetFullName());
                return ERROR;
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            IECInstancePtr newCA = newContainer.GetCustomAttribute(schemaName, className);
            if (containerType == SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class && 
                    className.EqualsIAscii("IsMixin") && schemaName.EqualsIAscii("CoreCustomAttributes"))
                {
                /* SCHEMA_EVOLUTION_RULE AppliesToEntityClass can modified only if old value is dervied from new value
                    * Both old and new class specified in 'AppliesToEntityClass' is read from ECDb so they must exist by the this CA is processed.
                    * Old  value should be derived class of new class assigned.
                */
                ECClassCR ctxClass = reinterpret_cast<ECClassCR>(newContainer);
                if (containerType == SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class)
                ECValue oldAppliesToValue, newAppliesToValue;
                auto resolveClass = [&](IECCustomAttributeContainerCR container) {
                    ECValue appliesToValue;
                    IECInstancePtr ca = container.GetCustomAttribute(schemaName, className);
                    ca->GetValue(appliesToValue, "AppliesToEntityClass");
                    if (appliesToValue.IsNull() || !appliesToValue.IsString())
                        return (ECClassCP)nullptr;

                    Utf8String targetAlias, targetClassName;
                    if (ECClass::ParseClassName(targetAlias, targetClassName, appliesToValue.GetUtf8CP()) != ECObjectsStatus::Success)
                        return (ECClassCP)nullptr;

                    if (targetAlias.empty())
                        targetAlias = ctxClass.GetSchema().GetName();

                    return ctx.GetSchemaManager().GetClass(targetAlias, targetClassName, SchemaLookupMode::AutoDetect);
                };

                // RULE: Old and new class must already exist in ECDb.
                ECClassCP oldApplyToClass = resolveClass(oldContainer);
                ECClassCP newApplyToClass = resolveClass(newContainer);
                if (!oldApplyToClass)
                    {
                    BeAssert(oldApplyToClass != nullptr);
                    return ERROR;
                    }
                if (newApplyToClass == nullptr)
                    {
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. MixIn %s: Modifing 'AppliesToEntityClass' from %s to new value failed because new class assigned to 'AppliesToEntityClass' must already exist.",
                                            ctxClass.GetFullName(), oldApplyToClass->GetFullName());
                    return ERROR;
                    }
                if (!oldApplyToClass->Is(newApplyToClass))
                    {
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. MixIn %s: Modifing 'AppliesToEntityClass' from %s to %s is only supported %s derived from %s.",
                                            ctxClass.GetFullName(), oldApplyToClass->GetFullName(), newApplyToClass->GetFullName(), oldApplyToClass->GetFullName(), newApplyToClass->GetFullName());
                    return ERROR;
                    }
                }
            
            BeAssert(newCA.IsValid());
            if (newCA.IsNull())
                return ERROR;

            if (ImportClass(ctx, newCA->GetClass()) != SUCCESS)
                {
                LOG.debugv("SchemaWriter::UpdateCustomAttributes - Failed to ImportClass %s", newCA->GetClass().GetFullName());
                return ERROR;
                }

            if (ReplaceCAEntry(ctx, *newCA, newCA->GetClass().GetId(), containerId, containerType, 0) != SUCCESS)
                {
                LOG.debugv("SchemaWriter::UpdateCustomAttributes - Failed to ReplaceCAEntry %s", newCA->GetClass().GetFullName());
                return ERROR;
                }
            }

        change.SetStatus(ECChange::Status::Done);
        }

    caChanges.SetStatus(ECChange::Status::Done);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaWriter::IsChangeToBaseClassIsSupported(ECClassCR baseClass)
    {
    if (ECEntityClassCP entityClass = baseClass.GetEntityClassCP())
        {
        if (entityClass->IsMixin())
            {
            return true;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateBaseClasses(Context& ctx, BaseClassChanges& baseClassChanges, ECN::ECClassCR oldClass, ECN::ECClassCR newClass)
    {
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
        StringChange& change = baseClassChanges[i];
        if (!change.IsChanged())
            continue;

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            ECClassCP oldBaseClass = findBaseClass(oldClass, change.GetOld().Value());
            if (oldBaseClass == nullptr)
                return ERROR;

            if (IsChangeToBaseClassIsSupported(*oldBaseClass))
                overrideAllBaseClasses = true;
            else
                {
                if (!ctx.IgnoreIllegalDeletionsAndModifications())
                    {
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Removing a base class '%s' from an ECClass is not supported.",
                                         oldClass.GetFullName(), oldBaseClass->GetFullName());
                    return ERROR;
                    }
                LOG.infov("Ignoring upgrade error: ECSchema Upgrade failed. ECClass %s: Removing a base class '%s' from an ECClass is not supported.",
                                     oldClass.GetFullName(), oldBaseClass->GetFullName());
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::New)
            {
            ECClassCP newBaseClass = findBaseClass(newClass, change.GetNew().Value());
            if (newBaseClass == nullptr)
                return ERROR;

            if (IsChangeToBaseClassIsSupported(*newBaseClass))
                overrideAllBaseClasses = true;
            else
                {
                if (!ctx.IgnoreIllegalDeletionsAndModifications())
                    {
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Adding a new base class '%s' to an ECClass is not supported.",
                                         oldClass.GetFullName(), newBaseClass->GetFullName());
                    return ERROR;
                    }
                LOG.infov("Ignoring upgrade error: ECSchema Upgrade failed. ECClass %s: Adding a new base class '%s' to an ECClass is not supported.",
                                     oldClass.GetFullName(), newBaseClass->GetFullName());
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            /* SCHEMA_EVOLUTION_RULE We only allow modifying an existing base class, if the new base class *IS of* the old base class.
            Currently, this check has been implemented in the SchemaComparer, so it will only use the OpCode "Modified" here, for base classes
            that are of the previous base class, and additionaly are not mixins.
            All other base class changes will be reflected using the Op codes "new" or "deleted"
            */

            ECClassCP newBaseClass = findBaseClass(newClass, change.GetNew().Value());
            if (newBaseClass == nullptr)
                return ERROR;

            ECClassCP oldBaseClass = findBaseClass(oldClass, change.GetOld().Value());
            if (oldBaseClass == nullptr)
                return ERROR;

            overrideAllBaseClasses = true;

            for(auto newBaseProperty : newBaseClass->GetProperties(true))
              { // new base properties have to be remapped throughout all existing derived classes
              Utf8CP propertyName = newBaseProperty->GetName().c_str();
              if(oldBaseClass->GetPropertyP(propertyName, true) == nullptr)
                { //only need to do this, if the property does not exist on the oldBaseClass
                ctx.ImportCtx().RemapManager().CollectRemapInfosFromModifiedBaseClass(newClass, *newBaseClass);
                }
              }
            }
        }

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateClass(Context& ctx, ClassChange& classChange, ECClassCR oldClass, ECClassCR newClass)
    {
    if (classChange.GetStatus() == ECChange::Status::Done || !classChange.IsChanged())
        return SUCCESS;

    if (classChange.Name().IsChanged())
        {
        if (!ctx.IgnoreIllegalDeletionsAndModifications())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of an ECClass '%s' is not supported.",
                                    oldClass.GetFullName());
            return ERROR;
            }
        LOG.infov("Ignoring update error: ECSchema Upgrade failed: Changing name of an ECClass '%s' is not supported.",
                    oldClass.GetFullName());
        }

    ECClassId classId = ctx.GetSchemaManager().GetClassId(newClass);
    if (!classId.IsValid())
        {
        LOG.debugv("SchemaWriter::UpdateClass - Failed to resolve ecclass id for class %s", newClass.GetFullName());
        return ERROR;
        }

    if (ctx.AssertReservedPropertyPolicy(newClass))
        {
        LOG.debugv("SchemaWriter::UpdateClass - Reserved property policy failed for class %s", newClass.GetFullName());
        return ERROR;
        }

    SqlUpdateBuilder updateBuilder(TABLE_Class);

    if (classChange.ClassModifier().IsChanged())
        {
        Nullable<ECClassModifier> oldValue = classChange.ClassModifier().GetOld();
        ECClassModifier newValue = classChange.ClassModifier().GetNew().Value();
        if (oldValue == ECClassModifier::Abstract)
            {
            auto& derivedClasses = newClass.GetDerivedClasses();
            for (ECClassP derivedClass : derivedClasses)
                {
                if(derivedClass->GetClassModifier() == ECClassModifier::Abstract)
                    {
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier from 'Abstract' requires the class to not have any abstract derived classes.",
                                oldClass.GetFullName());

                    return ERROR;
                    }
                }
            
            auto classMap = ctx.GetSchemaManager().GetClassMap(oldClass);
            bool isTablePerHierarchy = classMap != nullptr && classMap->GetMapStrategy().IsTablePerHierarchy();
            if(!isTablePerHierarchy)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier from 'Abstract' to another value is requires the map strategy to be 'TablePerHierarchy'.",
                               oldClass.GetFullName());

                return ERROR;
                }
            }

        if (newValue == ECClassModifier::Sealed)
            {
            if (!newClass.GetDerivedClasses().empty())
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier to 'Sealed' is only valid if the class does not have derived classes.",
                                     oldClass.GetFullName());

                return ERROR;
                }
            }
        else if (newValue == ECClassModifier::Abstract)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier to 'Abstract' is not supported.",
                                 oldClass.GetFullName());

            return ERROR;
            }

        updateBuilder.AddSetExp("Modifier", Enum::ToInt(classChange.ClassModifier().GetNew().Value()));
        }

    if (classChange.ClassType().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Changing the ECClassType of an ECClass is not supported.",
                             oldClass.GetFullName());
        return ERROR;
        }

    if (classChange.DisplayLabel().IsChanged())
        {
        if (classChange.DisplayLabel().GetNew().IsNull())
            updateBuilder.AddSetToNull("DisplayLabel");
        else
            updateBuilder.AddSetExp("DisplayLabel", classChange.DisplayLabel().GetNew().Value().c_str());
        }

    if (classChange.Description().IsChanged())
        {
        if (classChange.Description().GetNew().IsNull())
            updateBuilder.AddSetToNull("Description");
        else
            updateBuilder.AddSetExp("Description", classChange.Description().GetNew().Value().c_str());
        }

    if (oldClass.IsRelationshipClass())
        {
        if (classChange.Strength().IsChanged())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s: Changing the 'Strength' of an ECRelationshipClass is not supported.",
                                 oldClass.GetFullName());
            return ERROR;
            }

        if (classChange.StrengthDirection().IsChanged())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECRelationshipClass %s: Changing the 'StrengthDirection' of an ECRelationshipClass is not supported.",
                                 oldClass.GetFullName());
            return ERROR;
            }

        ECRelationshipClassCP oldRel = oldClass.GetRelationshipClassCP();
        ECRelationshipClassCP newRel = newClass.GetRelationshipClassCP();
        BeAssert(oldRel != nullptr && newRel != nullptr);
        if (oldRel == nullptr || newRel == nullptr)
            return ERROR;

        if (classChange.Source().IsChanged())
            if (UpdateRelationshipConstraint(ctx, classId, classChange.Source(), oldRel->GetSource(), newRel->GetSource(), true, oldRel->GetFullName()) == ERROR)
                {
                LOG.debugv("SchemaWriter::UpdateClass - failed to UpdateRelationshipConstraint (Source) on %s", oldRel->GetFullName());
                return ERROR;
                }

        if (classChange.Target().IsChanged())
            if (UpdateRelationshipConstraint(ctx, classId, classChange.Target(), oldRel->GetTarget(), newRel->GetTarget(), false, oldRel->GetFullName()) == ERROR)
                {
                LOG.debugv("SchemaWriter::UpdateClass - failed to UpdateRelationshipConstraint (Target) on %s", oldRel->GetFullName());
                return ERROR;
                }
        }

    updateBuilder.AddWhereExp("Id", classId.GetValue());
    if (updateBuilder.IsValid())
        {
        if (updateBuilder.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            {
            LOG.debugv("SchemaWriter::UpdateClass - failed to ExecuteSql for %s", newClass.GetFullName());
            return ERROR;
            }
        }


    if (classChange.BaseClasses().IsChanged())
        {
        if (UpdateBaseClasses(ctx, classChange.BaseClasses(), oldClass, newClass) != SUCCESS)
            {
            LOG.debugv("SchemaWriter::UpdateClass - failed to UpdateBaseClasses for %s", newClass.GetFullName());
            return ERROR;
            }
        }

    if (classChange.Properties().IsChanged())
        {
        if (UpdateProperties(ctx, classChange.Properties(), oldClass, newClass) != SUCCESS)
            {
            LOG.debugv("SchemaWriter::UpdateClass - failed to UpdateProperties for %s", newClass.GetFullName());
            return ERROR;
            }
        }

    if (SUCCESS != UpdateCustomAttributes(ctx, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, classId, classChange.CustomAttributes(), oldClass, newClass))
        {
        LOG.debugv("SchemaWriter::UpdateClass - failed to UpdateCustomAttributes for %s", newClass.GetFullName());
        return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateProperties(Context& ctx, PropertyChanges& propertyChanges, ECClassCR oldClass, ECClassCR newClass)
    {
    for (size_t i = 0; i < propertyChanges.Count(); i++)
        {
        PropertyChange& change = propertyChanges[i];
        if (!change.IsChanged())
            continue;

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            ECPropertyCP oldProperty = oldClass.GetPropertyP(change.GetChangeName(), false);
            if (oldProperty == nullptr)
                {
                BeAssert(false && "Failed to find property");
                return ERROR;
                }

            ECPropertyCP newProperty = newClass.GetPropertyP(change.GetChangeName(), true);
            if (SUCCESS != DeleteProperty(ctx, change, *oldProperty, newProperty))
                {
                LOG.debugv("SchemaWriter::UpdateProperties - Failed to DeleteProperty %s:%s", oldClass.GetFullName(), oldProperty->GetName().c_str());
                return ERROR;
                }

            }
        else if (change.GetOpCode() == ECChange::OpCode::New)
            {
            const int propertyIndex = ctx.GetNextPropertyOrdinal(oldClass.GetId());
            ECPropertyCP newProperty = newClass.GetPropertyP(change.Name().GetNew().Value().c_str(), false);
            if (newProperty == nullptr)
                {
                BeAssert(false && "Failed to find the class");
                return ERROR;
                }

            if (SUCCESS != ImportProperty(ctx, *newProperty, propertyIndex))
                {
                LOG.debugv("SchemaWriter::UpdateProperties - Failed to ImportProperty %s:%s", newClass.GetFullName(), newProperty->GetName().c_str());
                return ERROR;
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            ECPropertyCP oldProperty = oldClass.GetPropertyP(change.GetChangeName(), false);
            ECPropertyCP newProperty = newClass.GetPropertyP(change.GetChangeName(), false);
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
                {
                LOG.debugv("SchemaWriter::UpdateProperties - Failed to UpdateProperty %s:%s", newClass.GetFullName(), newProperty->GetName().c_str());
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateSchemaReferences(Context& ctx, SchemaReferenceChanges& referenceChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < referenceChanges.Count(); i++)
        {
        StringChange& change = referenceChanges[i];
        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            SchemaKey oldRef;
            if (SchemaKey::ParseSchemaFullName(oldRef, change.GetOld().Value().c_str()) != ECObjectsStatus::Success)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Failed to parse previous ECSchema reference name '%s'.",
                                        oldSchema.GetFullSchemaName().c_str(), change.GetOld().Value().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = SchemaPersistenceHelper::GetSchemaId(ctx.GetECDb(), DbTableSpace::Main(), oldRef.GetName().c_str(), SchemaLookupMode::ByName);
            Statement stmt;
            if (stmt.Prepare(ctx.GetECDb(), "DELETE FROM main." TABLE_SchemaReference " WHERE SchemaId=? AND ReferencedSchemaId=?") != BE_SQLITE_OK)
                {
                LOG.debugv("SchemaWriter::UpdateSchemaReferences - failed to prepare delete statement");
                return ERROR;
                }

            stmt.BindId(1, oldSchema.GetId());
            stmt.BindId(2, referenceSchemaId);

            if (stmt.Step() != BE_SQLITE_DONE)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Failed to remove ECSchema reference %s.",
                                        oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            SchemaKey newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Failed to parse new ECSchema reference '%s'.",
                                        oldSchema.GetFullSchemaName().c_str(), change.GetNew().Value().c_str());
                return ERROR;
                }

            if (!SchemaPersistenceHelper::TryGetSchemaKey(existingRef, ctx.GetECDb(), DbTableSpace::Main(), newRef.GetName().c_str()))
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                        oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }


            if (existingRef.LessThan(newRef, SchemaMatchType::Exact))
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s that has newer version than one present in ECDb.",
                                     oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            // no action is taken.
            }
        else if (change.GetOpCode() == ECChange::OpCode::New)
            {
            SchemaKey newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Failed to parse new ECSchema reference '%s'.",
                                          oldSchema.GetFullSchemaName().c_str(), change.GetNew().Value().c_str());
                return ERROR;
                }

            Nullable<Context::LegacySchemaImportHelper::Action> legacyImportAction;
            if (!ctx.IsEC32AvailableInFile())
                legacyImportAction = ctx.LegacySchemaImportHelper().GetImportAction(newRef);

            if (legacyImportAction == Context::LegacySchemaImportHelper::Action::Ignore)
                continue;

            //Ensure schema exist
            if (!SchemaPersistenceHelper::TryGetSchemaKey(existingRef, ctx.GetECDb(), DbTableSpace::Main(), newRef.GetName().c_str()))
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            
            ECSchemaId referenceSchemaId = SchemaPersistenceHelper::GetSchemaId(ctx.GetECDb(), DbTableSpace::Main(), newRef.GetName().c_str(), SchemaLookupMode::ByName);
            CachedStatementPtr stmt = ctx.GetCachedStatement("INSERT INTO main." TABLE_SchemaReference "(SchemaId, ReferencedSchemaId,Id) VALUES (?,?,?)");
            if (stmt == nullptr)
                return ERROR;

            stmt->BindId(1, oldSchema.GetId());
            stmt->BindId(2, referenceSchemaId);
            stmt->BindId(3, ctx.GetECDb().GetImpl().GetIdFactory().SchemaReference().NextId());
            if (stmt->Step() != BE_SQLITE_DONE)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Failed to add new reference to ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            BeAssert(false && "Should never end up here, as schema references cannot be modified, they can only be added or deleted.");
            return ERROR;
            }

        change.SetStatus(ECChange::Status::Done);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteCustomAttributeClass(Context& ctx, ECCustomAttributeClassCR deletedClass)
    {
    BeAssert(ctx.AreMajorSchemaVersionChangesAllowed() && ctx.IsMajorSchemaVersionChange(deletedClass.GetSchema().GetId()) && "Should have been checked before");

    Utf8StringCR schemaName = deletedClass.GetSchema().GetName();
    if (schemaName.EqualsI("ECDbMap") || schemaName.EqualsI("ECDbSchemaPolicies"))
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting ECCustomAttributeClass '%s' failed. Deleting ECCustomAttributeClass from system schemas are not supported.",
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteClass(Context& ctx, ClassChange& classChange, ECClassCR deletedClass)
    {
    if (!ctx.AreMajorSchemaVersionChangesAllowed() || !ctx.IsMajorSchemaVersionChange(deletedClass.GetSchema().GetId()))
        {
        if (ctx.IgnoreIllegalDeletionsAndModifications())
            {
            LOG.infov("Ignoring upgrade error:  ECSchema Upgrade failed. ECSchema %s: Cannot delete ECClass '%s'. This is a major ECSchema change. Either major schema version changes are disabled "
                                 "or the 'Read' version number of the ECSchema was not incremented.",
                                 deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
            return SUCCESS;
            }

        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Cannot delete ECClass '%s'. This is a major ECSchema change. Either major schema version changes are disabled "
                         "or the 'Read' version number of the ECSchema was not incremented.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    ECDerivedClassesList const* subClasses = ctx.GetECDb().Schemas().GetDerivedClassesInternal(deletedClass, "main");
    if (subClasses == nullptr)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Could not delete ECClass '%s' because its subclasses failed to load.",
                         deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (!subClasses->empty())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' is not supported because it has subclasses.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsStructClass())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. ECStructClass cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (IsSpecifiedInRelationshipConstraint(ctx, deletedClass))
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. A class which is specified in a relationship constraint cannot be deleted",
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
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Deleting ECRelationshipClass with ForeignKey mapping is not supported.",
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteInstances(Context& ctx, ECClassCR deletedClass)
    {
    Utf8String ecsql("DELETE FROM ");
    ecsql.append(deletedClass.GetECSqlName());
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ctx.GetECDb(), ecsql.c_str(), ctx.GetECDb().GetImpl().GetSettingsManager().GetCrudWriteToken()))
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (BE_SQLITE_DONE != stmt.Step())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteProperty(Context& ctx, PropertyChange& propertyChange, ECPropertyCR deletedProperty, ECPropertyCP newBaseProperty)
    {
    ECClassCR ecClass = deletedProperty.GetClass();
    bool isOverriddenProperty = newBaseProperty != nullptr;

    // fail if major version changes are not allowed or major version has not been incremented, but continue if newBaseProperty is available, meaning we are merely removing an overridden property 
    if ((!ctx.AreMajorSchemaVersionChangesAllowed() || !ctx.IsMajorSchemaVersionChange(deletedProperty.GetClass().GetSchema().GetId())) && !isOverriddenProperty)
        {
        if (ctx.IgnoreIllegalDeletionsAndModifications())
            {
            LOG.infov("Ignoring update error: ECSchema Upgrade failed. ECSchema %s: Cannot delete ECProperty '%s.%s'. This is a major ECSchema change. Either major schema version changes are disabled "
                                 "or the 'Read' version number of the ECSchema was not incremented.",
                                 ecClass.GetSchema().GetFullSchemaName().c_str(), ecClass.GetName().c_str(), deletedProperty.GetName().c_str());
            return SUCCESS;
            }

        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Cannot delete ECProperty '%s.%s'. This is a major ECSchema change. Either major schema version changes are disabled "
                             "or the 'Read' version number of the ECSchema was not incremented.",
                             ecClass.GetSchema().GetFullSchemaName().c_str(), ecClass.GetName().c_str(), deletedProperty.GetName().c_str());
        return ERROR;
        }

    if (deletedProperty.GetIsNavigation())
        {
        //Blanket error. We do not check if relationship was also deleted. In that case we would allo nav deletion for shared column/ logical relationships
        //Fail we do not want to delete a sql column right now
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Deleting Navigation ECProperty '%s' from an ECClass is not supported.",
                        ecClass.GetFullName(), deletedProperty.GetName().c_str());
        return ERROR;
        }

    ClassMap const* classMap = ctx.GetSchemaManager().GetClassMap(ecClass);
    if (classMap == nullptr)
        {
        BeAssert(false && "Failed to find classMap");
        return ERROR;
        }

    
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

        if (!isOverriddenProperty)
          { // possibly delete DbTable entries
          bool sharedColumnFound = false;
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
                  ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Deleting ECProperty '%s' from an ECClass which is not mapped to a shared column is not supported.",
                                  ecClass.GetFullName(), deletedProperty.GetName().c_str());
                  return ERROR;
                  }
              }

            if(sharedColumnFound)
                {
                Utf8String ecsql;
                ecsql.Sprintf("UPDATE %s SET [%s]=NULL", ecClass.GetECSqlName().c_str(), deletedProperty.GetName().c_str());
                ECSqlStatement stmt;
                if (ECSqlStatus::Success != stmt.Prepare(ctx.GetECDb(), ecsql.c_str(), ctx.GetECDb().GetImpl().GetSettingsManager().GetCrudWriteToken()) ||
                    BE_SQLITE_DONE != stmt.Step())
                    {
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECClass %s: Deleting an ECProperty '%s' from an ECClass failed due error while setting property to null", ecClass.GetFullName(), deletedProperty.GetName().c_str());
                    return ERROR;
                    }
                }
            }
        }

    if(isOverriddenProperty)
        ctx.ImportCtx().RemapManager().CollectRemapInfosFromDeletedPropertyOverride(deletedProperty.GetId());
    //Delete ECProperty entry make sure ec_Column is already deleted or set to null
    auto res = ctx.GetECDb().ExecuteSql(SqlPrintfString("DELETE FROM main.ec_Property WHERE Id=%s",
        deletedProperty.GetId().ToString(BeInt64Id::UseHex::Yes).c_str()));
    if (res != BE_SQLITE_DONE && res != BE_SQLITE_OK)
        {
        BeAssert(false && "Failed to delete ecproperty");
        return ERROR;
        }

    return DeleteCustomAttributes(ctx, deletedProperty.GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static BentleyStatus DeletePropertyCategoryById(SchemaWriter::Context& ctx, PropertyCategoryId propertyCategoryToDeleteId)
    {
    // if we did only delete, cascading delete would drop properties that are referenced, so we drop all references first
    CachedStatementPtr dropCategoryRefsStmt = ctx.GetCachedStatement("UPDATE main." TABLE_Property " SET CategoryId=NULL WHERE CategoryId=?");

    if (dropCategoryRefsStmt == nullptr ||
        dropCategoryRefsStmt->BindId(1, propertyCategoryToDeleteId) != BE_SQLITE_OK ||
        dropCategoryRefsStmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false && "failed to drop references to PropertyCategory before deletion");
        return ERROR;
        }

    CachedStatementPtr deleteCategoryStmt = ctx.GetCachedStatement("DELETE FROM main." TABLE_PropertyCategory " WHERE Id=?");

    if (deleteCategoryStmt == nullptr ||
        deleteCategoryStmt->BindId(1, propertyCategoryToDeleteId) != BE_SQLITE_OK ||
        deleteCategoryStmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false && "failed to delete PropertyCategory");
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeletePropertyCategory(Context& ctx, PropertyCategoryCR propertyCategoryToDelete)
    {
    if (!propertyCategoryToDelete.HasId())
        {
        BeAssert(false && "PropertyCategory had no Id");
        return ERROR;
        }
    return DeletePropertyCategoryById(ctx, propertyCategoryToDelete.GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateClasses(Context& ctx, ClassChanges& classChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < classChanges.Count(); i++)
        {
        ClassChange& change = classChanges[i];
        if (!change.IsChanged())
            continue;

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            ECClassCP oldClass = oldSchema.GetClassCP(change.GetChangeName());
            if (oldClass == nullptr)
                {
                BeAssert(false && "Failed to find class");
                return ERROR;
                }

            if (SUCCESS != DeleteClass(ctx, change, *oldClass))
                {
                LOG.debugv("SchemaWriter::UpdateClasses - failed to DeleteClass %s", oldClass->GetFullName());
                return ERROR;
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::New)
            {
            ECClassCP newClass = newSchema.GetClassCP(change.Name().GetNew().Value().c_str());
            if (newClass == nullptr)
                {
                BeAssert(false && "Failed to find the class");
                return ERROR;
                }

            if (ImportClass(ctx, *newClass) == ERROR)
                {
                LOG.debugv("SchemaWriter::UpdateClasses - failed to ImportClass %s", newClass->GetFullName());
                return ERROR;
                }
            }
        else if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            ECClassCP oldClass = oldSchema.GetClassCP(change.GetChangeName());
            ECClassCP newClass = newSchema.GetClassCP(change.GetChangeName());
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
                {
                LOG.debugv("SchemaWriter::UpdateClasses - failed to UpdateClass %s", newClass->GetFullName());
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateKindOfQuantities(Context& ctx, KindOfQuantityChanges& koqChanges, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < koqChanges.Count(); i++)
        {
        KindOfQuantityChange& change = koqChanges[i];
        if (!change.IsChanged())
            continue;

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            if (ctx.IgnoreIllegalDeletionsAndModifications())
                {
                LOG.infov("Ignoring update error: ECSchema Upgrade failed. ECSchema %s: Deleting KindOfQuantity '%s' from an ECSchema is not supported.",
                                     oldSchema.GetFullSchemaName().c_str(), change.Name());
                continue;
                }

            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting KindOfQuantity '%s' from an ECSchema is not supported.",
                            oldSchema.GetFullSchemaName().c_str(), change.Name());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::New)
            {
            KindOfQuantityCP koq = newSchema.GetKindOfQuantityCP(change.GetChangeName());
            if (koq == nullptr)
                {
                BeAssert(false && "Failed to find kind of quantity");
                return ERROR;
                }

            if (SUCCESS != ImportKindOfQuantity(ctx, *koq))
                return ERROR;

            continue;
            }

        if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            if (ctx.IgnoreIllegalDeletionsAndModifications())
                {
                LOG.infov("Ignoring update error: ECSchema Upgrade failed. ECSchema %s: Modifying KindOfQuantity '%s' is not supported.",
                                     oldSchema.GetFullSchemaName().c_str(), change.Name());
                continue;
                }

            KindOfQuantityCP oldKoq = oldSchema.GetKindOfQuantityCP(change.GetChangeName());
            KindOfQuantityCP newKoq = newSchema.GetKindOfQuantityCP(change.GetChangeName());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateKindOfQuantity(Context& ctx, KindOfQuantityChange& change, ECN::KindOfQuantityCR oldKoq, ECN::KindOfQuantityCR newKoq)
    {
    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.Name().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of a KindOfQuantity '%s' is not supported.",
                                change.Name());
        return ERROR;
        }

    size_t actualChanges = 0;
    SqlUpdateBuilder sqlUpdateBuilder(TABLE_KindOfQuantity);
    if (change.DisplayLabel().IsChanged())
        {
        actualChanges++;
        if (change.DisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.DisplayLabel().GetNew().Value().c_str());
        }

    if (change.Description().IsChanged())
        {
        actualChanges++;
        if (change.Description().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.Description().GetNew().Value().c_str());
        }

    if (change.RelativeError().IsChanged())
        {
        if (change.RelativeError().GetNew().IsNull())
            {
            ctx.Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Removing the RelativeError of a KindOfQuantity is not valid. A KindOfQuantity must always have a RelativeError.");
            return ERROR;
            }

        actualChanges++;
        sqlUpdateBuilder.AddSetExp("RelativeError", change.RelativeError().GetNew().Value());
        }

    if (change.PresentationFormats().IsChanged())
        {
        actualChanges++;

        if (newKoq.GetPresentationFormats().empty())
            sqlUpdateBuilder.AddSetToNull("PresentationUnits");
        else
            {
            Utf8String presFormatsJson;
            if (SUCCESS != SchemaPersistenceHelper::SerializeKoqPresentationFormats(presFormatsJson, ctx.GetECDb(), newKoq, ctx.IsEC32AvailableInFile()))
                return ERROR;

            sqlUpdateBuilder.AddSetExp("PresentationUnits", presFormatsJson.c_str());
            }
        }

    if (change.MemberChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing properties of KindOfQuantity '%s' is not supported except for RelativeError, PresentationFormats, DisplayLabel and Description.",
                         oldKoq.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldKoq.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }


static bool IsPropertyCategoryDeletionValid(SchemaWriter::Context& ctx, PropertyCategoryChange& deletingChange, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema)
    {
    const auto& toDeleteCategoryName = deletingChange.Name().GetOld();
    BeAssert(toDeleteCategoryName.IsValid());
    if (!toDeleteCategoryName.IsValid())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Old name of PropertyCategory '%s' to delete did not exist",
                                oldSchema.GetFullSchemaName().c_str(), toDeleteCategoryName.Value().c_str());
        return false;
        }

    const auto SchemaHasPropertyCategoryReferences = [&](ECN::ECSchemaCR schema) -> bool
        {
        for (const auto* ecClass : schema.GetClasses())
            for (const auto* property : ecClass->GetProperties(false))
                {
                if (property->GetCategory() != nullptr &&
                    property->GetCategory()->GetName() == toDeleteCategoryName.Value())
                    {
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Cannot delete a PropertyCategory, %s, "
                                        "which is still referenced in the schema by a property, %s.",
                                        oldSchema.GetFullSchemaName().c_str(),
                                        toDeleteCategoryName.Value().c_str(),
                                        property->GetCategory()->GetName().c_str());
                    return true;
                    }
                }
        return false;
        };

    const auto& schemasToImportList = ctx.GetSchemasToImportR();
    std::set<std::string> schemaNamesToImportSet{};
        {
        for (const auto& schema : schemasToImportList)
            schemaNamesToImportSet.insert(schema->GetName());
        }

    for (const auto* schemaToImport : schemasToImportList)
        if (ECN::ECSchema::IsSchemaReferenced(*schemaToImport, newSchema, ECN::SchemaMatchType::Latest) &&
            SchemaHasPropertyCategoryReferences(*schemaToImport))
            return false;

    bvector<ECN::ECSchemaCP> allExistingSchemas;
    ctx.GetSchemaManager().GetSchemas(allExistingSchemas);
    for (const auto* schema : allExistingSchemas)
        {
        const bool thisSchemaIsBeingImported = schemaNamesToImportSet.find(schema->GetName()) != schemaNamesToImportSet.end();
        if (!thisSchemaIsBeingImported &&
            ECN::ECSchema::IsSchemaReferenced(*schema, newSchema, ECN::SchemaMatchType::Latest) &&
            SchemaHasPropertyCategoryReferences(*schema))
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePropertyCategories(Context& ctx, PropertyCategoryChanges& changes, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < changes.Count(); i++)
        {
        PropertyCategoryChange& change = changes[i];
        if (!change.IsChanged())
            continue;

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            if (ctx.IgnoreIllegalDeletionsAndModifications())
                {
                LOG.infov("Ignoring update error: ECSchema Upgrade failed. ECSchema %s: Deleting PropertyCategory '%s' from an ECSchema is not supported.",
                                     oldSchema.GetFullSchemaName().c_str(), change.GetChangeName());
                continue;
                }

            if (!IsPropertyCategoryDeletionValid(ctx, change, oldSchema, newSchema))
                return ERROR;

            if (!change.Name().GetOld().IsValid())
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: old PropertyCategory '%s' name was unexpectedly invalid",
                                oldSchema.GetFullSchemaName().c_str(), change.GetChangeName());
                return ERROR;
                }

            PropertyCategoryCP propertyCategoryToDelete = oldSchema.GetPropertyCategoryCP(change.Name().GetOld().Value().c_str());
            if (propertyCategoryToDelete == nullptr)
                {
                BeAssert(false && "Failed to find PropertyCategory");
                return ERROR;
                }

            DeletePropertyCategory(ctx, *propertyCategoryToDelete);
            }

        if (change.GetOpCode() == ECChange::OpCode::New)
            {
            PropertyCategoryCP cat = newSchema.GetPropertyCategoryCP(change.GetChangeName());
            if (cat == nullptr)
                {
                BeAssert(false && "Failed to find property category");
                return ERROR;
                }

            if (ImportPropertyCategory(ctx, *cat) != SUCCESS)
                return ERROR;

            continue;
            }

        if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            if (ctx.IgnoreIllegalDeletionsAndModifications())
                {
                LOG.infov("Ignoring update error: ECSchema Upgrade failed. ECSchema %s: Modifying PropertyCategory '%s' from an ECSchema is not supported.",
                                     oldSchema.GetFullSchemaName().c_str(), change.GetChangeName());
                continue;
                }

            PropertyCategoryCP oldCat = oldSchema.GetPropertyCategoryCP(change.GetChangeName());
            PropertyCategoryCP newCat = newSchema.GetPropertyCategoryCP(change.GetChangeName());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePropertyCategory(Context& ctx, PropertyCategoryChange& change, ECN::PropertyCategoryCR oldCat, ECN::PropertyCategoryCR newCat)
    {
    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.Name().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of a PropertyCategory '%s' is not supported.",
                                oldCat.GetFullName().c_str());
        return ERROR;
        }

    size_t actualChanges = 0;

    SqlUpdateBuilder sqlUpdateBuilder(TABLE_PropertyCategory);
    if (change.DisplayLabel().IsChanged())
        {
        actualChanges++;
        if (change.DisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.DisplayLabel().GetNew().Value().c_str());
        }

    if (change.Description().IsChanged())
        {
        actualChanges++;
        if (change.Description().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.Description().GetNew().Value().c_str());
        }

    if (change.Priority().IsChanged())
        {
        actualChanges++;
        if (change.Priority().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Priority");
        else
            sqlUpdateBuilder.AddSetExp("Priority", change.Priority().GetNew().Value());
        }

    if (change.MemberChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing properties of PropertyCategory '%s' is not supported except for Priority, DisplayLabel and Description.",
                                oldCat.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldCat.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateEnumeration(Context& ctx, EnumerationChange& enumChange, ECN::ECEnumerationCR oldEnum, ECN::ECEnumerationCR newEnum)
    {
    if (enumChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (enumChange.Name().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of a ECEnumeration '%s' is not supported.",
                                oldEnum.GetFullName().c_str());
        return ERROR;
        }

    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Enumeration);

    if (enumChange.TypeName().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECEnumeration %s: 'Type' change is not supported.",
            oldEnum.GetFullName().c_str());

        return ERROR;
        }

    if (enumChange.IsStrict().IsChanged())
        {
        if (enumChange.IsStrict().GetNew().IsNull())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECEnumeration %s: 'IsStrict' must always be set for an ECEnumeration.",
                oldEnum.GetFullName().c_str());

            return ERROR;
            }

        //Allow transition from "strict" to "non-strict" but not the other way around.
        if (enumChange.IsStrict().GetOld().Value() == true &&
            enumChange.IsStrict().GetNew().Value() == false)
            sqlUpdateBuilder.AddSetExp("IsStrict", enumChange.IsStrict().GetNew().Value());
        else
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECEnumeration %s: 'IsStrict' changed. 'Unstrict' cannot be change to 'strict'. The other way around is allowed.",
                oldEnum.GetFullName().c_str());

            return ERROR;
            }
        }

    if (enumChange.DisplayLabel().IsChanged())
        {
        if (enumChange.DisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", enumChange.DisplayLabel().GetNew().Value().c_str());
        }

    if (enumChange.Description().IsChanged())
        {
        if (enumChange.Description().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", enumChange.Description().GetNew().Value().c_str());
        }

    EnumeratorChanges& enumeratorChanges = enumChange.Enumerators();
    if (enumeratorChanges.IsChanged())
        {
        if (SUCCESS != VerifyEnumeratorChanges(ctx, oldEnum, enumeratorChanges))
            return ERROR;

        Utf8String enumValueJson;
        if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(enumValueJson, newEnum, ctx.IsEC32AvailableInFile()))
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::VerifyEnumeratorChanges(Context& ctx, ECEnumerationCR oldEnum, EnumeratorChanges& enumeratorChanges)
    {
    //The schema comparer compares enumerators by their name. So it cannot detect a name change directly.
    //this algorithm is based on the assumption that a "deleted" enumerator in the old enum, and a "new" enumerator in the new enum
    //are the same if their integer value is the same.
    //this is only done for int enums. For string enums, enumerator name changes are not allowed at all.
    //Once the enumerator name changes are detected, any regular deleted enumerators are invalid, any regular new enumerators are fine.
    const bool isIntEnum = oldEnum.GetType() == PRIMITIVETYPE_Integer;
    bmap<int, EnumeratorChange*> deletedIntEnumerators, newIntEnumerators;
    bmap<Utf8CP, EnumeratorChange*, CompareUtf8> deletedStringEnumerators, newStringEnumerators;
    for (size_t i = 0; i < enumeratorChanges.Count(); i++)
        {
        EnumeratorChange& change = enumeratorChanges[i];
        switch (change.GetOpCode())
            {
                case ECChange::OpCode::New:
                    if (isIntEnum)
                        newIntEnumerators[change.Integer().GetNew().Value()] = &change;
                    else
                        newStringEnumerators[change.String().GetNew().Value().c_str()] = &change;

                    continue;
                case ECChange::OpCode::Deleted:
                    if (isIntEnum)
                        deletedIntEnumerators[change.Integer().GetOld().Value()] = &change;
                    else
                        deletedStringEnumerators[change.String().GetOld().Value().c_str()] = &change;

                    break;
                case ECChange::OpCode::Modified:
                    if (change.Integer().IsChanged() || change.String().IsChanged())
                        {
                        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. The value of one or more enumerators of Enumeration '%s' was modified which is not supported.",
                                                oldEnum.GetFullName().c_str());
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
    if (isIntEnum)
        {
        for (auto const& kvPair : deletedIntEnumerators)
            {
            const int val = kvPair.first;
            //We consider this a name change as the int values are equal.
            if (enumeratorNameChangeAllowed && newIntEnumerators.find(val) != newIntEnumerators.end())
                continue;

            if (!ctx.IgnoreIllegalDeletionsAndModifications())
                {
                //no counterpart with matching value found or old name is not the auto-generated EC3.2 conversion default name
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. An enumerator was deleted from Enumeration %s which is not supported.", oldEnum.GetFullName().c_str());
                return ERROR;
                }
            }
        }
    else
        {
        for (auto const& kvPair : deletedStringEnumerators)
            {
            Utf8CP val = kvPair.first;
            //We consider this a name change as the int values are equal.
            if (enumeratorNameChangeAllowed && newStringEnumerators.find(val) != newStringEnumerators.end())
                continue;

            if (!ctx.IgnoreIllegalDeletionsAndModifications())
                {
                //no counterpart with matching value found or old name is not the auto-generated EC3.2 conversion default name
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. An enumerator was deleted from Enumeration %s which is not supported.", oldEnum.GetFullName().c_str());
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateEnumerations(Context& ctx, EnumerationChanges& enumChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < enumChanges.Count(); i++)
        {
        EnumerationChange& change = enumChanges[i];
        if (!change.IsChanged())
            continue;

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            ECEnumerationCP ecEnum = oldSchema.GetEnumerationCP(change.GetChangeName());
            if (ctx.IgnoreIllegalDeletionsAndModifications())
                {
                LOG.infov("Ignoring upgrade error: ECSchema Upgrade failed. ECSchema %s: Deleting ECEnumerations '%s' from an ECSchema is not supported.",
                            oldSchema.GetFullSchemaName().c_str(), ecEnum->GetFullName().c_str());
                continue;
                }
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting ECEnumerations '%s' from an ECSchema is not supported.",
                                    oldSchema.GetFullSchemaName().c_str(), ecEnum->GetFullName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::New)
            {
            ECEnumerationCP ecEnum = newSchema.GetEnumerationCP(change.GetChangeName());
            if (ecEnum == nullptr)
                {
                BeAssert(false && "Failed to find enum");
                return ERROR;
                }

            if (SUCCESS != ImportEnumeration(ctx, *ecEnum))
                {
                LOG.debugv("SchemaWriter::UpdateEnumerations - failed to ImportEnumeration %s", ecEnum->GetFullName().c_str());
                return ERROR;
                }

            continue;
            }

        if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            ECEnumerationCP oldEnum = oldSchema.GetEnumerationCP(change.GetChangeName());
            ECEnumerationCP newEnum = newSchema.GetEnumerationCP(change.GetChangeName());
            BeAssert(oldEnum != nullptr && newEnum != nullptr);
            if (oldEnum == nullptr || newEnum == nullptr)
                {
                return ERROR;
                }

            if (UpdateEnumeration(ctx, change, *oldEnum, *newEnum) != SUCCESS)
                {
                LOG.debugv("SchemaWriter::UpdateEnumerations - failed to UpdateEnumeration %s", newEnum->GetFullName().c_str());
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePhenomena(Context& ctx, PhenomenonChanges& changes, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < changes.Count(); i++)
        {
        PhenomenonChange& change = changes[i];
        if (!change.IsChanged())
            continue;

        if (!ctx.IsEC32AvailableInFile())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Modifying phenomena is not supported in a file that does not support EC3.2 yet.",
                                    oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            PhenomenonCP phen = oldSchema.GetPhenomenonCP(change.GetChangeName());
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting Phenomena '%s' from an ECSchema is not supported.",
                                    oldSchema.GetFullSchemaName().c_str(), phen->GetFullName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::New)
            {
            PhenomenonCP phen = newSchema.GetPhenomenonCP(change.GetChangeName());
            if (phen == nullptr)
                {
                BeAssert(false && "Failed to find phenomenon");
                return ERROR;
                }

            if (SUCCESS != ImportPhenomenon(ctx, *phen))
                {
                LOG.debugv("SchemaWriter::UpdatePhenomena - failed to ImportPhenomena %s", phen->GetFullName().c_str());
                return ERROR;
                }
            }

        if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            PhenomenonCP oldVal = oldSchema.GetPhenomenonCP(change.GetChangeName());
            PhenomenonCP newVal = newSchema.GetPhenomenonCP(change.GetChangeName());
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
                {
                LOG.debugv("SchemaWriter::UpdatePhenomena - failed to UpdatePhenomenon %s", newVal->GetFullName().c_str());
                return ERROR;
                }

            continue;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePhenomenon(Context& ctx, PhenomenonChange& change, ECN::PhenomenonCR oldVal, ECN::PhenomenonCR newVal)
    {
    if (!ctx.IsEC32AvailableInFile())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to upgrade Phenomenon '%s'. Phenomena are not supported in a file that does not support EC3.2 yet.", oldVal.GetFullName().c_str());
        return ERROR;
        }

    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.Name().IsChanged())
        {
        ctx.Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of a Phenomenon is not supported.");
        return ERROR;
        }

    size_t actualChanges = 0;
    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Phenomenon);
    if (change.DisplayLabel().IsChanged())
        {
        actualChanges++;
        if (change.DisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.DisplayLabel().GetNew().Value().c_str());
        }

    if (change.Description().IsChanged())
        {
        actualChanges++;
        if (change.Description().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.Description().GetNew().Value().c_str());
        }

    if (change.MemberChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing properties of Phenomenon '%s' is not supported except for DisplayLabel and Description.",
                         oldVal.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldVal.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateUnitSystems(Context& ctx, UnitSystemChanges& changes, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < changes.Count(); i++)
        {
        UnitSystemChange& change = changes[i];
        if (!change.IsChanged())
            continue;

        if (!ctx.IsEC32AvailableInFile())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Modifying unit systems is not supported in a file that does not support EC3.2 yet.",
                                    oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            UnitSystemCP system = oldSchema.GetUnitSystemCP(change.GetChangeName());
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting UnitSystems '%s' from an ECSchema is not supported.",
                             oldSchema.GetFullSchemaName().c_str(), system->GetFullName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::New)
            {
            UnitSystemCP system = newSchema.GetUnitSystemCP(change.GetChangeName());
            if (system == nullptr)
                {
                BeAssert(false && "Failed to find unit system");
                return ERROR;
                }

            if (SUCCESS != ImportUnitSystem(ctx, *system))
                return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            UnitSystemCP oldVal = oldSchema.GetUnitSystemCP(change.GetChangeName());
            UnitSystemCP newVal = newSchema.GetUnitSystemCP(change.GetChangeName());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateUnitSystem(Context& ctx, UnitSystemChange& change, ECN::UnitSystemCR oldVal, ECN::UnitSystemCR newVal)
    {
    if (!ctx.IsEC32AvailableInFile())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to upgrade UnitSystem '%s'. UnitSystems are not supported in a file that does not support EC3.2 yet.",
                                oldVal.GetFullName().c_str());
        return ERROR;
        }

    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.Name().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of a UnitSystem '%s' is not supported.",
                                oldVal.GetFullName().c_str());
        return ERROR;
        }

    size_t actualChanges = 0;
    SqlUpdateBuilder sqlUpdateBuilder(TABLE_UnitSystem);
    if (change.DisplayLabel().IsChanged())
        {
        actualChanges++;
        if (change.DisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.DisplayLabel().GetNew().Value().c_str());
        }

    if (change.Description().IsChanged())
        {
        actualChanges++;
        if (change.Description().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.Description().GetNew().Value().c_str());
        }

    if (change.MemberChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing properties of UnitSystem '%s' is not supported except for DisplayLabel and Description.",
                         oldVal.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldVal.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateUnits(Context& ctx, UnitChanges& changes, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < changes.Count(); i++)
        {
        UnitChange& change = changes[i];
        if (!change.IsChanged())
            continue;

        if (!ctx.IsEC32AvailableInFile())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Modifying units is not supported in a file that does not support EC3.2 yet.", oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting Units from an ECSchema is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::New)
            {
            ECUnitCP unit = newSchema.GetUnitCP(change.GetChangeName());
            if (unit == nullptr)
                {
                BeAssert(false && "Failed to find unit");
                return ERROR;
                }

            if (SUCCESS != ImportUnit(ctx, *unit))
                return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            ECUnitCP oldVal = oldSchema.GetUnitCP(change.GetChangeName());
            ECUnitCP newVal = newSchema.GetUnitCP(change.GetChangeName());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateUnit(Context& ctx, UnitChange& change, ECN::ECUnitCR oldVal, ECN::ECUnitCR newVal)
    {
    if (!ctx.IsEC32AvailableInFile())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to upgrade Unit '%s'. Units are not supported in a file that does not support EC3.2 yet.",
                                oldVal.GetFullName().c_str());
        return ERROR;
        }

    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.Name().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of a Unit '%s' is not supported.",
                                oldVal.GetFullName().c_str());
        return ERROR;
        }

    size_t actualChanges = 0;
    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Unit);
    if (change.DisplayLabel().IsChanged())
        {
        actualChanges++;
        if (change.DisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.DisplayLabel().GetNew().Value().c_str());
        }

    if (change.Description().IsChanged())
        {
        actualChanges++;
        if (change.Description().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.Description().GetNew().Value().c_str());
        }

    if (change.Numerator().IsChanged())
        {
        if (oldVal.HasNumerator() == false && newVal.GetNumerator() == 1.0)
            {
            sqlUpdateBuilder.AddSetExp("numerator", change.Numerator().GetNew().Value());
            actualChanges++;
            }
        }

    if (change.Denominator().IsChanged())
        {
        if (oldVal.HasDenominator() == false && newVal.GetDenominator() == 1.0)
            {
            sqlUpdateBuilder.AddSetExp("denominator", change.Denominator().GetNew().Value());
            actualChanges++;
            }
        }

    if (change.Offset().IsChanged())
        {
        if (oldVal.HasOffset() == false && newVal.GetOffset() == 0.0)
            {
            sqlUpdateBuilder.AddSetExp("offset", change.Offset().GetNew().Value());
            actualChanges++;
            }
        }

    if (change.MemberChangesCount() > actualChanges)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing properties of Unit '%s' is not supported except for DisplayLabel and Description.",
                         oldVal.GetFullName().c_str());
        return ERROR;
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldVal.GetId().GetValue());
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateFormats(Context& ctx, FormatChanges& changes, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    for (size_t i = 0; i < changes.Count(); i++)
        {
        FormatChange& change = changes[i];
        if (!change.IsChanged())
            continue;

        if (!ctx.IsEC32AvailableInFile())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Modifying formats is not supported in a file that does not support EC3.2 yet.", oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::Deleted)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Deleting Formats from an ECSchema is not supported.",
                                 oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::New)
            {
            ECFormatCP format = newSchema.GetFormatCP(change.GetChangeName());
            if (format == nullptr)
                {
                BeAssert(false && "Failed to find format");
                return ERROR;
                }

            if (SUCCESS != ImportFormat(ctx, *format))
                return ERROR;
            }

        if (change.GetOpCode() == ECChange::OpCode::Modified)
            {
            ECFormatCP oldVal = oldSchema.GetFormatCP(change.GetChangeName());
            ECFormatCP newVal = newSchema.GetFormatCP(change.GetChangeName());
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateFormat(Context& ctx, FormatChange& change, ECN::ECFormatCR oldVal, ECN::ECFormatCR newVal)
    {
    if (!ctx.IsEC32AvailableInFile())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to upgrade Unit '%s'. Formats are not supported in a file that does not support EC3.2 yet.", oldVal.GetFullName().c_str());
        return ERROR;
        }

    if (change.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (change.Name().IsChanged())
        {
        ctx.Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of a Format is not supported.");
        return ERROR;
        }

    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Format);
    if (change.DisplayLabel().IsChanged())
        {
        if (change.DisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", change.DisplayLabel().GetNew().Value().c_str());
        }

    if (change.Description().IsChanged())
        {
        if (change.Description().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", change.Description().GetNew().Value().c_str());
        }

    NumericFormatSpecChange& numSpecChange = change.NumericSpec();
    if (numSpecChange.IsChanged())
        {
        if (numSpecChange.GetOpCode() == ECN::ECChange::OpCode::Deleted)
            sqlUpdateBuilder.AddSetToNull("NumericSpec");
        else
            {
            BeAssert(newVal.HasNumeric());
            sqlUpdateBuilder.AddSetExp("NumericSpec", SchemaPersistenceHelper::SerializeNumericSpec(*newVal.GetNumericSpec()).c_str());
            }
        }

    CompositeValueSpecChange& compSpecChange = change.CompositeSpec();
    if (compSpecChange.IsChanged())
        {
        if (compSpecChange.MajorUnit().IsChanged() || compSpecChange.MiddleUnit().IsChanged() || compSpecChange.MinorUnit().IsChanged() ||
            compSpecChange.SubUnit().IsChanged())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the composite units of Format '%s' is not supported.",
                                 oldVal.GetFullName().c_str());
            return ERROR;
            }

        if (compSpecChange.Spacer().IsChanged() || compSpecChange.IncludeZero().IsChanged())
            sqlUpdateBuilder.AddSetExp("CompositeSpec", SchemaPersistenceHelper::SerializeCompositeSpecWithoutUnits(*newVal.GetCompositeSpec()).c_str());
        }

    FormatId formatId = oldVal.GetId();
    sqlUpdateBuilder.AddWhereExp("Id", formatId.GetValue());
    if (SUCCESS != sqlUpdateBuilder.ExecuteSql(ctx.GetECDb()))
        return ERROR;

    if (SUCCESS != UpdateFormatCompositeUnitLabel(ctx, formatId, compSpecChange.MajorLabel(), 0))
        return ERROR;

    if (SUCCESS != UpdateFormatCompositeUnitLabel(ctx, formatId, compSpecChange.MiddleLabel(), 1))
        return ERROR;

    if (SUCCESS != UpdateFormatCompositeUnitLabel(ctx, formatId, compSpecChange.MinorLabel(), 2))
        return ERROR;

    return UpdateFormatCompositeUnitLabel(ctx, formatId, compSpecChange.SubLabel(), 3);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateFormatCompositeUnitLabel(Context& ctx, FormatId formatId, StringChange& unitLabelChange, int ordinal)
    {
    BeAssert(ctx.IsEC32AvailableInFile());
    if (!unitLabelChange.IsChanged())
        return SUCCESS;

    SqlUpdateBuilder sqlUpdateBuilder(TABLE_FormatCompositeUnit);
    if (unitLabelChange.GetNew().IsNull())
        sqlUpdateBuilder.AddSetToNull("Label");
    else
        sqlUpdateBuilder.AddSetExp("Label", unitLabelChange.GetNew().Value().c_str());

    sqlUpdateBuilder.AddWhereExp("FormatId", formatId.GetValue());
    sqlUpdateBuilder.AddWhereExp("Ordinal", ordinal);
    return sqlUpdateBuilder.ExecuteSql(ctx.GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateSchema(Context& ctx, SchemaChange& schemaChange, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (schemaChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (schemaChange.Name().IsChanged())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. Changing the name of an ECSchema '%s' is not supported.",
                                oldSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    ECSchemaId schemaId = ctx.GetSchemaManager().GetSchemaId(newSchema);
    if (!schemaId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecschema id");
        return ERROR;
        }

    SqlUpdateBuilder updateBuilder(TABLE_Schema);

    if (schemaChange.DisplayLabel().IsChanged())
        {
        if (schemaChange.DisplayLabel().GetNew().IsNull())
            updateBuilder.AddSetToNull("DisplayLabel");
        else
            updateBuilder.AddSetExp("DisplayLabel", schemaChange.DisplayLabel().GetNew().Value().c_str());
        }

    if (schemaChange.Description().IsChanged())
        {
        if (schemaChange.Description().GetNew().IsNull())
            updateBuilder.AddSetToNull("Description");
        else
            updateBuilder.AddSetExp("Description", schemaChange.Description().GetNew().Value().c_str());
        }

    const bool readVersionHasChanged = schemaChange.VersionRead().IsChanged();
    if (readVersionHasChanged)
        {
        if (schemaChange.VersionRead().GetOld().Value() > schemaChange.VersionRead().GetNew().Value())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionRead' of an ECSchema is not supported.",
                                    oldSchema.GetName().c_str());
            return ERROR;
            }

        if (!ctx.AreMajorSchemaVersionChangesAllowed())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Major schema version changes are disabled.",
                                    oldSchema.GetName().c_str());
            return ERROR;
            }

        ctx.AddSchemaWithMajorVersionChange(oldSchema.GetId());
        updateBuilder.AddSetExp("VersionDigit1", schemaChange.VersionRead().GetNew().Value());
        }

    const bool writeVersionHasChanged = schemaChange.VersionWrite().IsChanged();
    if (writeVersionHasChanged)
        {
        if (!readVersionHasChanged && schemaChange.VersionWrite().GetOld().Value() > schemaChange.VersionWrite().GetNew().Value())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionWrite' of an ECSchema is not supported.",
                                    oldSchema.GetName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit2", schemaChange.VersionWrite().GetNew().Value());
        }

    if (schemaChange.VersionMinor().IsChanged())
        {
        if (!readVersionHasChanged && !writeVersionHasChanged && schemaChange.VersionMinor().GetOld().Value() > schemaChange.VersionMinor().GetNew().Value())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionMinor' of an ECSchema is not supported.",
                                    oldSchema.GetName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit3", schemaChange.VersionMinor().GetNew().Value());
        }

    if (schemaChange.Alias().IsChanged())
        {
        if (!ctx.IgnoreIllegalDeletionsAndModifications())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Modifying the Alias is not supported.",
                                 oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        LOG.infov("Ignoring upgrade error: ECSchema Upgrade failed. ECSchema %s: Modifying the Alias is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
        }

    if (schemaChange.ECVersion().IsChanged())
        {
        if (schemaChange.ECVersion().GetOld().Value() > schemaChange.ECVersion().GetNew().Value())
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Decreasing 'ECVersion' of an ECSchema is not supported.",
                                    oldSchema.GetName().c_str());
            return ERROR;
            }
        }

    if (FeatureManager::IsAvailable(ctx.GetECDb(), Feature::ECVersions))
        {
        const bool originalVersionMajorHasChanged = schemaChange.OriginalECXmlVersionMajor().IsChanged();
        if (originalVersionMajorHasChanged)
            {
            uint32_t newVal = schemaChange.OriginalECXmlVersionMajor().GetNew().Value();
            if (schemaChange.OriginalECXmlVersionMajor().GetOld().Value() > newVal)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Decreasing 'OriginalECXmlVersionMajor' of an ECSchema is not supported.",
                                        oldSchema.GetName().c_str());
                return ERROR;
                }

            updateBuilder.AddSetExp("OriginalECXmlVersionMajor", newVal);
            }

        if (schemaChange.OriginalECXmlVersionMinor().IsChanged())
            {
            uint32_t newVal = schemaChange.OriginalECXmlVersionMinor().GetNew().Value();
            //if the higher digits have changed, minor version may be decremented
            if (!originalVersionMajorHasChanged && schemaChange.OriginalECXmlVersionMinor().GetOld().Value() > newVal)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECSchema Upgrade failed. ECSchema %s: Decreasing 'OriginalECXmlVersionMinor' of an ECSchema is not supported.",
                                        oldSchema.GetName().c_str());
                return ERROR;
                }
            updateBuilder.AddSetExp("OriginalECXmlVersionMinor", newVal);
            }
        }
    else
        {
        BeAssert(newSchema.OriginalECXmlVersionLessThan(ECVersion::V3_2) && "Only EC 3.1 schemas can be imported into a file not supporting EC 3.2 yet");
        }

    updateBuilder.AddWhereExp("Id", schemaId.GetValue());//this could even be on name
    if (updateBuilder.IsValid())
        {
        if (updateBuilder.ExecuteSql(ctx.GetECDb()) != SUCCESS)
            {
            LOG.debugv("SchemaWriter::UpdateSchema - failed to ExecuteSql for %s", newSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        }

    schemaChange.SetStatus(ECChange::Status::Done);

    if (SUCCESS != UpdateSchemaReferences(ctx, schemaChange.References(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdateSchemaReferences for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdateEnumerations(ctx, schemaChange.Enumerations(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdateEnumerations for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdatePhenomena(ctx, schemaChange.Phenomena(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdatePhenomena for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdateUnitSystems(ctx, schemaChange.UnitSystems(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdateUnitSystems for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdateUnits(ctx, schemaChange.Units(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdateUnits for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdateFormats(ctx, schemaChange.Formats(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdateFormats for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdateKindOfQuantities(ctx, schemaChange.KindOfQuantities(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdateKindOfQuantities for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdatePropertyCategories(ctx, schemaChange.PropertyCategories(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdatePropertyCategories for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdateClasses(ctx, schemaChange.Classes(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdateClasses for %s", newSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    if (SUCCESS != UpdateCustomAttributes(ctx, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema, schemaId, schemaChange.CustomAttributes(), oldSchema, newSchema))
        {
        LOG.debugv("SchemaWriter::UpdateSchema - failed to UpdateCustomAttributes for %s", newSchema.GetFullSchemaName().c_str());
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool NumericChangeIsDecrease(UInt32Change const& change)
    {
    if (!change.IsChanged())
        return false;

    Nullable<uint32_t> oldV = change.GetOld();
    Nullable<uint32_t> newV = change.GetNew();
    return oldV != nullptr && newV != nullptr && newV.Value() < oldV.Value();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool IsSupportedSchemaDowngradeChange(SchemaChange& change)
    {
    // only minor version may be decreased so that the older version is fully compatible with the new one
    if (change.VersionRead().IsChanged() || change.VersionWrite().IsChanged())
        return false;

    return NumericChangeIsDecrease(change.VersionMinor());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::CompareSchemas(Context& ctx, bvector<ECSchemaCP> const& schemas)
    {
    if (schemas.empty())
        {
        BeAssert(false);
        return ERROR;
        }

    BeAssert(ctx.GetExistingSchemas().empty() && ctx.GetSchemasToImport().empty());

    std::set<Utf8String> doneList;
    for (ECSchemaCP schema : schemas)
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
        //We do not require detail if schema is added or deleted. the name and version suffices.
        SchemaComparer::Options options = SchemaComparer::Options(SchemaComparer::DetailLevel::NoSchemaElements, SchemaComparer::DetailLevel::NoSchemaElements);
        if (SUCCESS != comparer.Compare(ctx.GetDiff(), ctx.GetExistingSchemas(), ctx.GetSchemasToImport(), options))
            return ERROR;

        std::set<Utf8CP, CompareIUtf8Ascii> schemaOfInterest;
        if (ctx.GetDiff().Changes().IsChanged())
            {
            for (size_t i = 0; i < ctx.GetDiff().Changes().Count(); i++)
                {
                SchemaChange& schemaChange = ctx.GetDiff().Changes()[i];
                if (IsSupportedSchemaDowngradeChange(schemaChange))
                    {
                    LOG.infov("Schema %s is skipped for the schema upgrade because a newer compatible version of it already exists in the file.", schemaChange.GetChangeName());
                    continue;
                    }

                schemaOfInterest.insert(schemaChange.GetChangeName());
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
// @bsimethod
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
* @bsimethod
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
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Schema import failed. Failed to read schema '%s' from ECDb.", name.c_str());
            return ERROR;
            }

        if (!ValidateSchema(*schema))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Schema import failed. Failed to validate previously imported schema '%s'.", name.c_str());
            return ERROR;
            }

        ctx.AddExistingSchema(*schema);
        }

    for (Utf8StringCR name : importingSchemaNames)
        {
        ECSchemaCP schema = ctx.GetSchemaManager().GetSchema(name);
        if (schema == nullptr)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Schema import failed. Failed to read imported schema '%s' from ECDb.", name.c_str());
            return ERROR;
            }

        if (!ValidateSchema(*schema))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Schema import failed. Failed to validate imported schema '%s'.", name.c_str());
            return ERROR;
            }

        ctx.AddSchemaToImport(*schema);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::Context::PreprocessSchemas(bvector<ECN::ECSchemaCP>& out, bvector<ECN::ECSchemaCP> const& in)
    {
    bvector<ECSchemaCP> schemasToImport = FindAllSchemasInGraph(in);
    for (ECSchemaCP schema : schemasToImport)
        {
        if (schema == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        //this is the in-memory version of ECSchemas. ECDb only supports the latest in-memory version.
        //Deserializing into older versions is not needed in ECDb and therefore not supported.
        if (schema->GetECVersion() != ECVersion::Latest)
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECSchemas. The in-memory version of the ECSchema '%s' must be %s, but is %s.", schema->GetFullSchemaName().c_str(), ECSchema::GetECVersionString(ECVersion::Latest), ECSchema::GetECVersionString(schema->GetECVersion()));
            return ERROR;
            }

        if (schema->HasId())
            {
            ECSchemaId id = SchemaPersistenceHelper::GetSchemaId(GetECDb(), DbTableSpace::Main(), schema->GetName().c_str(), SchemaLookupMode::ByName);
            if (!id.IsValid() || id != schema->GetId())
                {
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECSchemas. ECSchema %s is owned by some other ECDb file. (Incoming schemaId %s != existing schemaId %s", schema->GetFullSchemaName().c_str(), 
                                 schema->GetId().ToString().c_str(), id.IsValid() ? id.ToString().c_str() : "(0)");
                return ERROR;
                }
            }
        }

    bvector<ECSchemaCP> primarySchemas;
    bvector<ECSchemaP> suppSchemas;
    for (ECSchemaCP schema : schemasToImport)
        {
        if (schema->IsSupplementalSchema())
            {
            suppSchemas.push_back(const_cast<ECSchemaP> (schema));
            }
        else
            primarySchemas.push_back(schema);
        }

    schemasToImport.clear();

    if (!suppSchemas.empty())
        {
        for (ECSchemaCP primarySchema : primarySchemas)
            {
            if (primarySchema->IsSupplemented())
                continue;

            ECSchemaP primarySchemaP = const_cast<ECSchemaP> (primarySchema);
            SupplementedSchemaBuilder builder;
            SupplementedSchemaStatus status = builder.UpdateSchema(*primarySchemaP, suppSchemas, false /*dont create ca copy while supplementing*/);
            if (SupplementedSchemaStatus::Success != status)
                {
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to import ECSchemas. Failed to supplement ECSchema %s. See log file for details.", primarySchema->GetFullSchemaName().c_str());
                return ERROR;
                }

            //All consolidated custom attribute must be reference. But Supplemental Provenance in BSCA is not
            //This bug could also be fixed in SupplementSchema builder but its much safer to do it here for now.
            if (primarySchema->GetSupplementalInfo().IsValid())
                {
                IECInstancePtr provenance = primarySchema->GetCustomAttribute("SupplementalProvenance");
                if (provenance.IsValid())
                    {
                    auto& bsca = provenance->GetClass().GetSchema();
                    if (!ECSchema::IsSchemaReferenced(*primarySchema, bsca))
                        {
                        primarySchemaP->AddReferencedSchema(const_cast<ECSchemaR>(bsca));
                        }
                    }
                }
            }
        }

    // The dependency order may have *changed* due to supplementation adding new ECSchema references! Re-sort them.
    bvector<ECN::ECSchemaCP> sortedSchemas = Sort(primarySchemas);

    //If we import into pre-EC3.2 files, we must not import the units and formats schema
    //as they are only deserialized temporarily by ECObjects
    if (IsEC32AvailableInFile())
        out.insert(out.begin(), sortedSchemas.begin(), sortedSchemas.end());
    else
        m_legacyHelper.RemoveSchemasToSkip(out, sortedSchemas);

    return SchemaValidator::ValidateSchemas(ImportCtx(), Issues(), out) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus PruneNamelessPropertyCategories(SchemaWriter::Context& ctx)
    {
    CachedStatementPtr namelessCategoriesStmt = ctx.GetCachedStatement("SELECT id FROM main." TABLE_PropertyCategory " WHERE Name=''");
    auto stmtStatus = BE_SQLITE_OK;

    while (namelessCategoriesStmt != nullptr && (stmtStatus = namelessCategoriesStmt->Step()) == BE_SQLITE_ROW)
        {
        const PropertyCategoryId namelessCategoryId = namelessCategoriesStmt->GetValueId<PropertyCategoryId>(0);
        BentleyStatus status = DeletePropertyCategoryById(ctx, namelessCategoryId);
        if (status != SUCCESS)
            return status;
        }

    if (namelessCategoriesStmt == nullptr || stmtStatus != BE_SQLITE_DONE)
        {
        BeAssert(false && "failed to find nameless categories");
        return ERROR;
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::Context::PostprocessSchemas(bvector<ECN::ECSchemaCP>& out, bvector<ECN::ECSchemaCP> const& in)
    {
    if (SUCCESS != PruneNamelessPropertyCategories(*this))
        {
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bvector<ECN::ECSchemaCP> SchemaWriter::Context::Sort(bvector<ECN::ECSchemaCP> const& in)
    {
    bvector<ECN::ECSchemaCP> sortedList;
    bvector<ECN::ECSchemaCP> layer;

    do
        {
        layer = GetNextLayer(in, layer);
        for (auto schemaItor = layer.rbegin(); schemaItor != layer.rend(); ++schemaItor)
            sortedList.push_back(*schemaItor);
        } while (!layer.empty());

    std::reverse(sortedList.begin(), sortedList.end());
    return sortedList;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bvector<ECN::ECSchemaCP> SchemaWriter::Context::GetNextLayer(bvector<ECN::ECSchemaCP> const& schemas, bvector<ECN::ECSchemaCP> const& referencedBy)
    {
    bvector<ECN::ECSchemaCP> list;
    bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> map;

    const auto RemoveReachableInSet = [&](const bvector<ECN::ECSchemaCP>& inSchemas)
        {
        for (auto schema : inSchemas)
            for (const auto& ref : FindAllSchemasInGraph(*schema, false))
                {
                auto itor = map.find(ref.first);
                if (map.end() != itor)
                    map.erase(itor);
                }
        };

    if (referencedBy.empty())
        {
        for (auto schema : schemas)
            if (map.find(schema->GetSchemaKey()) == map.end())
                map[schema->GetSchemaKey()] = schema;

        RemoveReachableInSet(schemas);
        }
    else
        {
        for (auto schema : referencedBy)
            for (const auto& ref : schema->GetReferencedSchemas())
                if (map.end() == map.find(ref.first))
                    map[ref.first] = ref.second.get();

        // a map value iterator would remove the need for this copy
        bvector<ECN::ECSchemaCP> schemasInMap;
        std::transform(map.begin(), map.end(), std::back_inserter(schemasInMap), [](const auto& s) { return s.second; });
        RemoveReachableInSet(schemasInMap);
        }

    for (const auto& ref : map)
        list.push_back(ref.second);

    return list;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bvector<ECN::ECSchemaCP> SchemaWriter::Context::FindAllSchemasInGraph(bvector<ECN::ECSchemaCP> const& schemas)
    {
    bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> map;
    for (ECN::ECSchemaCP schema : schemas)
        for (const auto& entry : FindAllSchemasInGraph(*schema, true))
            if (map.find(entry.first) == map.end())
                map[entry.first] = entry.second;

    bvector<ECN::ECSchemaCP> temp;
    for (const auto& entry : map)
        temp.push_back(entry.second);

    return temp;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> SchemaWriter::Context::FindAllSchemasInGraph(ECN::ECSchemaCR schema, bool includeThisSchema)
    {
    bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> schemaMap;
    if (includeThisSchema)
        schemaMap[schema.GetSchemaKey()] = &schema;

    for (const auto& entry : schema.GetReferencedSchemas())
        FindAllSchemasInGraph(schemaMap, entry.second.get());

    return schemaMap;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void SchemaWriter::Context::FindAllSchemasInGraph(bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>>& schemaMap, ECN::ECSchemaCP schema)
    {
    if (schemaMap.find(schema->GetSchemaKey()) != schemaMap.end())
        return;

    schemaMap[schema->GetSchemaKey()] = schema;
    for (const auto& entry : schema->GetReferencedSchemas())
        FindAllSchemasInGraph(schemaMap, entry.second.get());
    }
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
ECClassCP SchemaWriter::Context::ReservedPropertyNamesPolicy::FindCustomAttributeOwner(ECN::ECClassCP entityClass, IECInstanceCP ca)
    {
    if (entityClass->GetCustomAttributeLocal(ca->GetClass().GetName()).get() == ca)
        return entityClass;

    // ignore mixin. Only traverse base class hierarchy
    if (entityClass->HasBaseClasses())
        {
        if (ECClassCP baseClass = entityClass->GetBaseClasses()[0])
            {
            if (ECClassCP result = FindCustomAttributeOwner(baseClass, ca))
                return result;
            }
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void SchemaWriter::Context::ReservedPropertyNamesPolicy::FindInhertiedPolicyCustomAttribute(std::vector<std::pair<ECN::ECClassCP, ECN::IECInstanceCP>>& policyAttributes,  ECN::ECClassCP entityClass)
    {
    IECInstancePtr ca = entityClass->GetCustomAttribute(RESERVED_PROPERTY_SchemaName, RESERVED_PROPERTY_ClassName);
    if (ca.IsValid())
        {
        if (ECN::ECClassCP owner = FindCustomAttributeOwner(entityClass, ca.get()))
            {
            policyAttributes.push_back(std::make_pair(owner, ca.get()));
            if (owner->HasBaseClasses())
                {
                if (ECClassCP baseClass = owner->GetBaseClasses()[0])
                    FindInhertiedPolicyCustomAttribute(policyAttributes, baseClass);
                }
            }
        }
    }
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriter::Context::ReservedPropertyNamesPolicy::PolicyCacheMap::iterator SchemaWriter::Context::ReservedPropertyNamesPolicy::RegisterPolicy(IssueDataSource const& issues, ECN::ECClassCR entityClass) const
    {
    std::vector<std::pair<ECN::ECClassCP, ECN::IECInstanceCP>> policyAttributes;
    FindInhertiedPolicyCustomAttribute(policyAttributes, &entityClass);
    if (policyAttributes.empty())
        return m_reservedProperties.end();

    ECN::IECInstanceCP ownerPolicyCA = policyAttributes[0].second;
    auto it = m_reservedProperties.insert(std::make_pair(ownerPolicyCA, PolicyCacheEntry())).first;
    PolicyCacheEntry& reservedProps = it->second;

    for (auto const& entry : policyAttributes)
        {
        ECN::IECInstanceCP ca = entry.second;
        ECClassCP ownerClass = entry.first;
        uint32_t propertyNamesIdx = 0;
        if (ca->GetEnablerR().GetPropertyIndex(propertyNamesIdx, RESERVED_PROPERTY_Name) != ECObjectsStatus::Success)
            {
            BeAssert(false && "Expecting property 'PropertyNames'");
            }

        ECValue propertyNamesVal;
        uint32_t arrayCount = 0;
        if (ca->GetValue(propertyNamesVal, propertyNamesIdx) == ECObjectsStatus::Success && !propertyNamesVal.IsNull())
            arrayCount = propertyNamesVal.GetArrayInfo().GetCount();

        if (arrayCount == 0)
            {
            issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECDbSchemaPolicies:ReservedPropertyNames customAttribute defined on ECClass '%s' is invalid. It must have atleast one reserved property name declared."
                           , ownerClass->GetFullName());
            return m_reservedProperties.end();
            }

        ECValue propertyNameVal;
        for (uint32_t i = 0; i < arrayCount; ++i)
            {
            if (ECObjectsStatus::Success != ca->GetValue(propertyNameVal, propertyNamesIdx, i) && propertyNameVal.IsNull())
                continue;

            if (!ECNameValidation::IsValidName(propertyNameVal.GetUtf8CP()))
                continue;

            reservedProps.insert(std::make_pair(propertyNameVal.GetUtf8CP(), ownerClass));
            }
        }
    return it;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int SchemaWriter::Context::GetNextPropertyOrdinal(ECClassId classId) {
    auto it = m_propertyOrdinal.find(classId);
    if (it == m_propertyOrdinal.end()) {
        auto stmt = GetECDb().GetCachedStatement("select max(ordinal) from " TABLE_Property " where classId=?");
        stmt->BindId(1, classId);
        int ordinal = -1;
        if (stmt->Step() == BE_SQLITE_ROW && !stmt->IsColumnNull(0)) {
            ordinal = stmt->GetValueInt(0);
        } 
        it = m_propertyOrdinal.insert(make_bpair(classId, ordinal)).first;
    }
    it->second = it->second + 1;
    return it->second;
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaWriter::Context::AssertReservedPropertyPolicy(ECN::ECClassCR entityClass, ECN::ECPropertyCP property) const
    {
    return m_reservedPropertyNamePolicy.Evaluate(GetECDb().GetImpl().Issues(), entityClass, property);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaWriter::Context::ReservedPropertyNamesPolicy::Evaluate(IssueDataSource const& issues, ECN::ECClassCR entityClass, ECN::ECPropertyCP property) const
    {
    IECInstancePtr ca = entityClass.GetCustomAttribute(RESERVED_PROPERTY_SchemaName, RESERVED_PROPERTY_ClassName);
    if (ca.IsValid())
        {
        auto it = m_reservedProperties.find(ca.get());
        if (it == m_reservedProperties.end())
            {
            it = RegisterPolicy(issues, entityClass);
            if (it == m_reservedProperties.end())
                return true;
            }

        PolicyCacheEntry& reservedProps = it->second;
        if (property)
            {
            auto itor = reservedProps.find(property->GetName());
            if (itor != reservedProps.end())
                {
                ECClassCP ownerClass = itor->second;
                issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECDbSchemaPolicies:ReservedPropertyNames policy defined on '%s' class prohibit use of property with name '%s' (origin: %s) in its dervied hierarchy. Reserved property policy failed for class %s. The property could have been inherited.",
                               ownerClass->GetFullName(), property->GetName().c_str(), property->GetClass().GetFullName());
                return true;
                }
            return false;
            }

        bool reservedPropFound = false;
        for (auto const prop : entityClass.GetProperties(true))
            {
            auto itor = reservedProps.find(prop->GetName());
            if (itor != reservedProps.end())
                {
                ECClassCP ownerClass = itor->second;
                issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "ECDbSchemaPolicies:ReservedPropertyNames policy defined on '%s' class prohibit use of property with name '%s' (origin: %s) in its dervied hierarchy. Reserved property policy failed for class %s. The property could have been inherited.",
                               ownerClass->GetFullName(), prop->GetName().c_str(), prop->GetClass().GetFullName(), entityClass.GetFullName());
                reservedPropFound = true;
                }
            }
        return reservedPropFound;
        }
    return false;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
