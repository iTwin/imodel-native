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
BentleyStatus SchemaWriter::ImportSchemas(bvector<ECN::ECSchemaCP>& schemasToMap, bvector<ECSchemaCP> const& primarySchemasOrderedByDependencies)
    {
    PERFLOG_START("ECDb", "Schema import> Persist schemas");

    if (SUCCESS != ValidateSchemasPreImport(primarySchemasOrderedByDependencies))
        return ERROR;

    SchemaCompareContext compareCtx(m_ecdb.Schemas().GetDispatcher().Main());
    if (SUCCESS != compareCtx.Prepare(primarySchemasOrderedByDependencies))
        return ERROR;

    if (compareCtx.GetSchemasToImport().empty())
        return SUCCESS;

    for (ECSchemaCP schema : compareCtx.GetSchemasToImport())
        {
        if (SUCCESS != ImportSchema(compareCtx, *schema))
            return ERROR;
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(m_ecdb))
        return ERROR;

    if (SUCCESS != compareCtx.ReloadContextECSchemas())
        return ERROR;

    schemasToMap.insert(schemasToMap.begin(), compareCtx.GetSchemasToImport().begin(), compareCtx.GetSchemasToImport().end());
    PERFLOG_FINISH("ECDb", "Schema import> Persist schemas");
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                     05/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ValidateSchemasPreImport(bvector<ECSchemaCP> const& primarySchemasOrderedByDependencies) const
    {
    const bool isValid = SchemaValidator::ValidateSchemas(m_ctx, m_ecdb.GetImpl().Issues(), primarySchemasOrderedByDependencies);
    return isValid ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportSchema(SchemaCompareContext& compareCtx, ECN::ECSchemaCR ecSchema)
    {
    m_majorChangesAllowedForSchemas.clear();
    if (SchemaChange* schemaChange = compareCtx.GetChanges().Find(ecSchema.GetName().c_str()))
        {
        if (schemaChange->GetState() == ChangeState::Modified)
            {
            if (schemaChange->GetStatus() == ECChange::Status::Done)
                return SUCCESS;

            ECSchemaCP existingSchema = compareCtx.FindExistingSchema(schemaChange->GetId());
            BeAssert(existingSchema != nullptr);
            if (existingSchema == nullptr)
                return ERROR;

            return UpdateSchema(*schemaChange, *existingSchema, ecSchema);
            }
        else if (schemaChange->GetState() == ChangeState::Deleted)
            {
            if (schemaChange->GetStatus() == ECChange::Status::Done)
                return SUCCESS;

            schemaChange->SetStatus(ECChange::Status::Done);
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting an ECSchema is not supported.",
                            ecSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        else
            {
            if (schemaChange->GetStatus() == ECChange::Status::Done)
                return SUCCESS;
            }
        }

    if (m_ecdb.Schemas().ContainsSchema(ecSchema.GetName().c_str()))
        return SUCCESS;

    if (SUCCESS != InsertSchemaEntry(ecSchema))
        {
        DbResult lastErrorCode;
        m_ecdb.GetLastError(&lastErrorCode);
        if (BE_SQLITE_CONSTRAINT_UNIQUE == lastErrorCode)
            Issues().ReportV("Failed to import ECSchema '%s'. Alias '%s' is already used by an existing ECSchema.",
                            ecSchema.GetFullSchemaName().c_str(), ecSchema.GetAlias().c_str());
        return ERROR;
        }

    if (SUCCESS != InsertSchemaReferenceEntries(ecSchema))
        {
        Issues().ReportV("Failed to import ECSchema references for ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    //enums must be imported before ECClasses as properties reference enums
    for (ECEnumerationCP ecEnum : ecSchema.GetEnumerations())
        {
        if (SUCCESS != ImportEnumeration(*ecEnum))
            {
            Issues().ReportV("Failed to import ECEnumeration '%s'.", ecEnum->GetFullName().c_str());
            return ERROR;
            }
        }

    //KOQs must be imported before ECClasses as properties reference KOQs
    for (KindOfQuantityCP koq : ecSchema.GetKindOfQuantities())
        {
        if (SUCCESS != ImportKindOfQuantity(*koq))
            {
            Issues().ReportV("Failed to import KindOfQuantity '%s'.", koq->GetFullName().c_str());
            return ERROR;
            }
        }

    //PropertyCategories must be imported before ECClasses as properties reference PropertyCategories
    for (PropertyCategoryCP cat : ecSchema.GetPropertyCategories())
        {
        if (SUCCESS != ImportPropertyCategory(*cat))
            {
            Issues().ReportV("Failed to import PropertyCategory '%s'.", cat->GetFullName().c_str());
            return ERROR;
            }
        }
    
    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        if (SUCCESS != ImportClass(*ecClass))
            {
            Issues().ReportV("Failed to import ECClass '%s'.", ecClass->GetFullName());
            return ERROR;
            }
        }

    if (SUCCESS != ImportCustomAttributes(ecSchema, ECContainerId(ecSchema.GetId()), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema))
        {
        Issues().ReportV("Failed to import custom attributes of ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertSchemaReferenceEntries(ECSchemaCR schema)
    {
    ECSchemaReferenceListCR references = schema.GetReferencedSchemas();
    if (references.empty())
        return SUCCESS;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "INSERT INTO main." TABLE_SchemaReference "(SchemaId,ReferencedSchemaId) VALUES(?,?)"))
        return ERROR;

    for (bpair<SchemaKey, ECSchemaPtr> const& kvPair : references)
        {
        ECSchemaCP reference = kvPair.second.get();
        ECSchemaId referenceId = SchemaPersistenceHelper::GetSchemaId(m_ecdb, DbTableSpace::Main(), reference->GetName().c_str(), SchemaLookupMode::ByName);
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

        if (BE_SQLITE_OK != stmt.BindId(1, schema.GetId()))
            return ERROR;

        if (BE_SQLITE_OK != stmt.BindId(2, referenceId))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.Reset();
        stmt.ClearBindings();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportClass(ECN::ECClassCR ecClass)
    {
    if (m_ecdb.Schemas().Main().GetClassId(ecClass).IsValid())
        return SUCCESS;

    if (!m_ecdb.Schemas().Main().GetSchemaId(ecClass.GetSchema()).IsValid())
        {
        Issues().ReportV("Failed to import ECClass '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ecClass.GetName().c_str(), ecClass.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import ECClass because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }
    
    //now import actual ECClass
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_Class(SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection,CustomAttributeContainerType) VALUES(?,?,?,?,?,?,?,?,?)");
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

    const ECClassId classId = DbUtilities::GetLastInsertedId<ECClassId>(m_ecdb);
    if (!classId.IsValid())
        return ERROR;

    const_cast<ECClassR>(ecClass).SetId(classId);

    //release stmt so that it can be reused to insert base classes
    stmt = nullptr;

    //Import All baseCases
    int baseClassIndex = 0;
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        if (SUCCESS != ImportClass(*baseClass))
            return ERROR;

        if (SUCCESS != InsertBaseClassEntry(classId, *baseClass, baseClassIndex++))
            return ERROR;
        }

    int propertyIndex = 0;
    for (ECPropertyCP ecProperty : ecClass.GetProperties(false))
        {
        if (SUCCESS != ImportProperty(*ecProperty, propertyIndex++))
            {
            Issues().ReportV("Failed to import ECProperty '%s' of ECClass '%s'.", ecProperty->GetName().c_str(), ecClass.GetFullName());
            return ERROR;
            }
        }

    ECN::ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr)
        {
        if (SUCCESS != ImportRelationshipClass(relationship))
            return ERROR;
        }

    return ImportCustomAttributes(ecClass, ECContainerId(classId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportEnumeration(ECEnumerationCR ecEnum)
    {
    if (m_ecdb.Schemas().Main().GetEnumerationId(ecEnum).IsValid())
        return SUCCESS;

    if (!m_ecdb.Schemas().Main().GetSchemaId(ecEnum.GetSchema()).IsValid())
        {
        Issues().ReportV("Failed to import ECEnumeration '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ecEnum.GetName().c_str(), ecEnum.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import ECEnumeration because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_Enumeration(SchemaId,Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues) VALUES(?,?,?,?,?,?,?)");
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

    const ECEnumerationId enumId = DbUtilities::GetLastInsertedId<ECEnumerationId>(m_ecdb);
    if (!enumId.IsValid())
        return ERROR;

    const_cast<ECEnumerationR>(ecEnum).SetId(enumId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportKindOfQuantity(KindOfQuantityCR koq)
    {
    if (m_ecdb.Schemas().Main().GetKindOfQuantityId(koq).IsValid())
        return SUCCESS;

    if (!m_ecdb.Schemas().Main().GetSchemaId(koq.GetSchema()).IsValid())
        {
        Issues().ReportV("Failed to import KindOfQuantity '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", koq.GetName().c_str(), koq.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import KindOfQuantity because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_KindOfQuantity(SchemaId,Name,DisplayLabel,Description,PersistenceUnit,RelativeError,PresentationUnits) VALUES(?,?,?,?,?,?,?)");
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

    if (koq.GetPersistenceUnit().HasProblem())
        {
        Issues().ReportV("Failed to import KindOfQuantity '%s'. Its persistence unit is invalid: %s.", koq.GetFullName().c_str(), koq.GetPersistenceUnit().GetProblemDescription().c_str());
        return ERROR;
        }

    Utf8String persistenceUnitStr = koq.GetPersistenceUnit().ToText(false);
    BeAssert(!persistenceUnitStr.empty());
    if (BE_SQLITE_OK != stmt->BindText(5, persistenceUnitStr, Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindDouble(6, koq.GetRelativeError()))
        return ERROR;


    Utf8String presUnitsJsonStr;
    if (!koq.GetPresentationUnitList().empty())
        {
        if (SUCCESS != SchemaPersistenceHelper::SerializeKoqPresentationUnits(presUnitsJsonStr, m_ecdb, koq))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindText(7, presUnitsJsonStr, Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const KindOfQuantityId koqId = DbUtilities::GetLastInsertedId<KindOfQuantityId>(m_ecdb);
    if (!koqId.IsValid())
        return ERROR;

    const_cast<KindOfQuantityR>(koq).SetId(koqId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportPropertyCategory(PropertyCategoryCR cat)
    {
    if (m_ecdb.Schemas().Main().GetPropertyCategoryId(cat).IsValid())
        return SUCCESS;

    if (!m_ecdb.Schemas().Main().GetSchemaId(cat.GetSchema()).IsValid())
        {
        Issues().ReportV("Failed to import PropertyCategory '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", cat.GetName().c_str(), cat.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import PropertyCategory because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_PropertyCategory(SchemaId,Name,DisplayLabel,Description,Priority) VALUES(?,?,?,?,?)");
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

    PropertyCategoryId catId = DbUtilities::GetLastInsertedId<PropertyCategoryId>(m_ecdb);
    if (!catId.IsValid())
        return ERROR;

    const_cast<PropertyCategoryR>(cat).SetId(catId);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportRelationshipClass(ECN::ECRelationshipClassCP relationship)
    {
    const ECClassId relClassId = relationship->GetId();
    if (SUCCESS != ImportRelationshipConstraint(relClassId, relationship->GetSource(), ECRelationshipEnd_Source))
        return ERROR;

    return ImportRelationshipConstraint(relClassId, relationship->GetTarget(), ECRelationshipEnd_Target);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportRelationshipConstraint(ECClassId relClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd end)
    {
    BeAssert(relClassId.IsValid());

    ECRelationshipConstraintId constraintId;
    if (SUCCESS != InsertRelationshipConstraintEntry(constraintId, relClassId, relationshipConstraint, end))
        return ERROR;

    BeAssert(constraintId.IsValid());
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_RelationshipConstraintClass(ConstraintId,ClassId) VALUES(?,?)");
    if (stmt == nullptr)
        return ERROR;

    for (ECClassCP constraintClass : relationshipConstraint.GetConstraintClasses())
        {
        if (SUCCESS != ImportClass(*constraintClass))
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
    return ImportCustomAttributes(relationshipConstraint, ECContainerId(constraintId), containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertRelationshipConstraintEntry(ECRelationshipConstraintId& constraintId, ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_RelationshipConstraint(RelationshipClassId,RelationshipEnd,MultiplicityLowerLimit,MultiplicityUpperLimit,IsPolymorphic,RoleLabel,AbstractConstraintClassId) VALUES(?,?,?,?,?,?,?)");
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
        if (SUCCESS != ImportClass(abstractConstraintClass))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(7, abstractConstraintClass.GetId()))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    constraintId = DbUtilities::GetLastInsertedId<ECRelationshipConstraintId>(m_ecdb);
    BeAssert(constraintId.IsValid());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportProperty(ECN::ECPropertyCR ecProperty, int ordinal)
    {
    //Local properties are expected to not be imported at this point as they get imported along with their class.
    BeAssert(!m_ecdb.Schemas().Main().GetPropertyId(ecProperty).IsValid());

    if (ecProperty.GetIsStruct())
        {
        if (SUCCESS != ImportClass(ecProperty.GetAsStructProperty()->GetType()))
            return ERROR;
        }
    else if (ecProperty.GetIsStructArray())
        {
        if (SUCCESS != ImportClass(ecProperty.GetAsStructArrayProperty()->GetStructElementType()))
            return ERROR;
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (SUCCESS != ImportClass(*ecProperty.GetAsNavigationProperty()->GetRelationshipClass()))
            return ERROR;
        }

    //now insert the actual property
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_Property(ClassId,Name,DisplayLabel,Description,IsReadonly,Priority,Ordinal,Kind,"
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

        if (SUCCESS != BindPropertyMinMaxValue(*stmt, primitiveTypeMinValueIndex, ecProperty, v))
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

        if (SUCCESS != BindPropertyMinMaxValue(*stmt, primitiveTypeMaxValueIndex, ecProperty, v))
            return ERROR;
        }

    if (ecProperty.HasExtendedType())
        {
        if (SUCCESS != BindPropertyExtendedTypeName(*stmt, extendedTypeIndex, ecProperty))
            return ERROR;
        }

    if (SUCCESS != BindPropertyKindOfQuantity(*stmt, koqIdIndex, ecProperty))
        return ERROR;

    if (SUCCESS != BindPropertyCategory(*stmt, catIdIndex, ecProperty))
        return ERROR;

    if (ecProperty.GetIsPrimitive())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(PropertyKind::Primitive)))
            return ERROR;

        if (SUCCESS != BindPropertyPrimTypeOrEnumeration(*stmt, primitiveTypeIndex, enumIdIndex, ecProperty))
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

            if (SUCCESS != BindPropertyPrimTypeOrEnumeration(*stmt, primitiveTypeIndex, enumIdIndex, ecProperty))
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
        LOG.fatal(m_ecdb.GetLastError().c_str());
        return ERROR;
        }

    const ECPropertyId propId = DbUtilities::GetLastInsertedId<ECPropertyId>(m_ecdb);
    if (!propId.IsValid())
        return ERROR;

    const_cast<ECPropertyR>(ecProperty).SetId(propId);

    return ImportCustomAttributes(ecProperty, ECContainerId(propId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType)
    {
    int ordinal = 0;
    for (IECInstancePtr ca : sourceContainer.GetCustomAttributes(false))
        {
        //import CA classes first
        ECClassCR caClass = ca->GetClass();
        if (SUCCESS != ImportClass(caClass))
            return ERROR;

        if (SUCCESS != InsertCAEntry(*ca, caClass.GetId(), sourceContainerId, containerType, ordinal))
            return ERROR;

        ordinal++;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertSchemaEntry(ECSchemaCR schema)
    {
    BeAssert(!schema.HasId());

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_Schema(Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3,ECVersion,OriginalECXmlVersionMajor,OriginalECXmlVersionMinor) VALUES(?,?,?,?,?,?,?,?,?,?)");
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

    const ECSchemaId id = DbUtilities::GetLastInsertedId<ECSchemaId>(m_ecdb);
    if (!id.IsValid())
        return ERROR;

    const_cast<ECSchemaR>(schema).SetId(id);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int ordinal)
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_ClassHasBaseClasses(ClassId,BaseClassId,Ordinal) VALUES(?,?,?)");
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
BentleyStatus SchemaWriter::BindPropertyMinMaxValue(Statement& stmt, int paramIndex, ECN::ECPropertyCR prop, ECN::ECValueCR val)
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

    m_ecdb.GetImpl().Issues().ReportV("Failed to import schema. The ECProperty '%s.%s' has a minimum/maximum value of an unsupported type.",
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
BentleyStatus SchemaWriter::BindPropertyPrimTypeOrEnumeration(Statement& stmt, int primTypeParamIndex, int enumParamIndex, ECPropertyCR prop)
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

    if (SUCCESS != ImportEnumeration(*ecenum))
        return ERROR;

    BeAssert(ecenum->HasId());
    return stmt.BindId(enumParamIndex, ecenum->GetId()) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyKindOfQuantity(Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    if (!prop.IsKindOfQuantityDefinedLocally() || prop.GetKindOfQuantity() == nullptr)
        return SUCCESS;

    if (prop.GetIsNavigation())
        {
        Issues().ReportV("Failed to import Navigation ECProperty '%s.%s' because a KindOfQuantity is assigned to it.", prop.GetClass().GetFullName(), prop.GetName().c_str());
        return ERROR;
        }

    KindOfQuantityCP koq = prop.GetKindOfQuantity();
    if (SUCCESS != ImportKindOfQuantity(*koq))
        return ERROR;

    BeAssert(koq->HasId());
    return stmt.BindId(paramIndex, koq->GetId()) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyCategory(Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    if (!prop.IsCategoryDefinedLocally() || prop.GetCategory() == nullptr)
        return SUCCESS;

    PropertyCategoryCP cat = prop.GetCategory();
    if (SUCCESS != ImportPropertyCategory(*cat))
        return ERROR;

    BeAssert(cat->HasId());
    return stmt.BindId(paramIndex, cat->GetId()) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::InsertCAEntry(IECInstanceR customAttribute, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, int ordinal)
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_CustomAttribute(ContainerId,ContainerType,ClassId,Ordinal,Instance) VALUES(?,?,?,?,?)");
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
BentleyStatus SchemaWriter::DeleteCAEntry(int& ordinal, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType)
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Ordinal FROM main.ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?");
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

    stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("DELETE FROM main.ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?");
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

    BeAssert(m_ecdb.GetModifiedRowCount() > 0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ReplaceCAEntry(IECInstanceR customAttribute, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, int ordinal)
    {
    if (DeleteCAEntry(ordinal, ecClassId, containerId, containerType) != SUCCESS)
        return ERROR;

    return InsertCAEntry(customAttribute, ecClassId, containerId, containerType, ordinal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaWriter::IsPropertyTypeChangeSupported(Utf8StringR error, StringChange& typeChange, ECPropertyCR oldProperty, ECPropertyCR newProperty) const
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
BentleyStatus SchemaWriter::UpdateProperty(ECPropertyChange& propertyChange, ECPropertyCR oldProperty, ECPropertyCR newProperty)
    {
    if (propertyChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    ECPropertyId propertyId = m_ecdb.Schemas().Main().GetPropertyId(newProperty);
    if (!propertyId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecclass id");
        return ERROR;
        }

    if (propertyChange.GetTypeName().IsValid())
        {
        Utf8String error;
        if (!IsPropertyTypeChangeSupported(error, propertyChange.GetTypeName(), oldProperty, newProperty))
            {
            Issues().Report(error.c_str());
            return ERROR;
            }
        }

    if (propertyChange.IsStruct().IsValid() || propertyChange.IsStructArray().IsValid() || propertyChange.IsPrimitive().IsValid() ||
        propertyChange.IsPrimitiveArray().IsValid() || propertyChange.IsNavigation().IsValid())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the kind of the ECProperty is not supported.",
                                  oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return ERROR;
        }

    if (propertyChange.GetArray().IsValid())
        {
        ArrayChange& arrayChange = propertyChange.GetArray();
        if (arrayChange.MaxOccurs().IsValid() || arrayChange.MinOccurs().IsValid())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'MinOccurs' or 'MaxOccurs' for an Array ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }
        }

    if (propertyChange.GetNavigation().IsValid())
        {
        NavigationChange& navigationChange = propertyChange.GetNavigation();
        if (navigationChange.GetRelationshipClassName().IsValid())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'RelationshipClassName' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

        if (navigationChange.Direction().IsValid())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'Direction' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }
        }

    SqlUpdateBuilder sqlUpdateBuilder("ec_Property");
    if (propertyChange.GetName().IsValid())
        {
        if (propertyChange.GetName().GetNew().IsNull())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: 'Name' must always be set for an ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

        sqlUpdateBuilder.AddSetExp("Name", propertyChange.GetName().GetNew().Value().c_str());
        }
    //MinMaxValueChange:
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
                Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'PrimitiveTypeMinValue' to a unsupported type.",
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
                Issues().ReportV("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'PrimitiveTypeMaxValue' to a unsupported type.",
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
        {
        sqlUpdateBuilder.AddSetExp("IsReadonly", propertyChange.IsReadonly().GetNew().Value());
        }

    if (propertyChange.GetPriority().IsValid())
        {
        sqlUpdateBuilder.AddSetExp("Priority", propertyChange.GetPriority().GetNew().Value());
        }

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
            sqlUpdateBuilder.AddSetToNull("EnumerationId"); //set to null;
            }
        else
            {
            ECEnumerationCP enumCP = newProperty.GetIsPrimitive() ? newProperty.GetAsPrimitiveProperty()->GetEnumeration() : newProperty.GetAsPrimitiveArrayProperty()->GetEnumeration();
            ECEnumerationId id = m_ecdb.Schemas().Main().GetEnumerationId(*enumCP);
            if (!id.IsValid())
                return ERROR;

            sqlUpdateBuilder.AddSetToNull("PrimitiveType");
            sqlUpdateBuilder.AddSetExp("EnumerationId", id.GetValue());
            }
        }

    if (propertyChange.GetKindOfQuantity().IsValid())
        {
        StringChange& change = propertyChange.GetKindOfQuantity();
        sqlUpdateBuilder.AddSetToNull("KindOfQuantityId");
        if (change.GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("KindOfQuantityId");
        else
            {
            KindOfQuantityCP koqCP = newProperty.GetKindOfQuantity();
            if (koqCP == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            KindOfQuantityId id = m_ecdb.Schemas().Main().GetKindOfQuantityId(*koqCP);
            if (!id.IsValid())
                {
                if (ImportKindOfQuantity(*koqCP) != SUCCESS)
                    return ERROR;

                id = koqCP->GetId();
                }

            sqlUpdateBuilder.AddSetExp("KindOfQuantityId", id.GetValue());
            }
        }

    if (propertyChange.GetCategory().IsValid())
        {
        StringChange& change = propertyChange.GetCategory();
        sqlUpdateBuilder.AddSetToNull("CategoryId");
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

            PropertyCategoryId id = m_ecdb.Schemas().Main().GetPropertyCategoryId(*cat);
            if (!id.IsValid())
                {
                if (ImportPropertyCategory(*cat) != SUCCESS)
                    return ERROR;

                id = cat->GetId();
                }

            sqlUpdateBuilder.AddSetExp("CategoryId", id.GetValue());
            }
        }

    sqlUpdateBuilder.AddWhereExp("Id", propertyId.GetValue());
    if (sqlUpdateBuilder.IsValid())
        {
        if (sqlUpdateBuilder.ExecuteSql(m_ecdb) != SUCCESS)
            return ERROR;
        }

    return UpdateCustomAttributes(SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property, propertyId, propertyChange.CustomAttributes(), oldProperty, newProperty);;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateRelationshipConstraint(ECContainerId containerId, ECRelationshipConstraintChange& constraintChange, ECRelationshipConstraintCR oldConstraint, ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName)
    {
    Utf8CP constraintEndStr = isSource ? "Source" : "Target";
    SqlUpdateBuilder updater("ec_RelationshipConstraint");

    if (constraintChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (constraintChange.GetMultiplicity().IsValid())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing 'Multiplicity' of an ECRelationshipConstraint is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.IsPolymorphic().IsValid())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing flag 'IsPolymorphic' of an ECRelationshipConstraint is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.ConstraintClasses().IsValid())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing the constraint classes is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.GetRoleLabel().IsValid())
        {
        updater.AddSetExp("RoleLabel", constraintChange.GetRoleLabel().GetNew().Value().c_str());
        }

    updater.AddWhereExp("RelationshipEnd", isSource ? ECRelationshipEnd_Source : ECRelationshipEnd_Target);
    updater.AddWhereExp("RelationshipClassId", containerId.GetValue());
    if (updater.ExecuteSql(m_ecdb) != SUCCESS)
        return ERROR;

    const SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType = isSource ? SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint : SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint;
    return UpdateCustomAttributes(containerType, containerId, constraintChange.CustomAttributes(), oldConstraint, newConstraint);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateCustomAttributes(SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, ECContainerId containerId, ECInstanceChanges& instanceChanges, IECCustomAttributeContainerCR oldContainer, IECCustomAttributeContainerCR newContainer)
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
            if (m_schemaUpgradeCustomAttributeValidator.HasAnyRuleForSchema(schemaName.c_str()))
                {
                if (m_schemaUpgradeCustomAttributeValidator.Validate(change) == CustomAttributeValidator::Policy::Reject)
                    {
                    Issues().ReportV("ECSchema Upgrade failed. Adding or modifying %s custom attributes is not supported.", schemaName.c_str());
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

            if (ImportClass(ca->GetClass()) != SUCCESS)
                return ERROR;

            if (InsertCAEntry(*ca, ca->GetClass().GetId(), containerId, containerType, ++customAttributeIndex) != SUCCESS)
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
            if (DeleteCAEntry(ordinal, ca->GetClass().GetId(), containerId, containerType) != SUCCESS)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            IECInstancePtr ca = newContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                return ERROR;

            if (ImportClass(ca->GetClass()) != SUCCESS)
                return ERROR;

            if (ReplaceCAEntry(*ca, ca->GetClass().GetId(), containerId, containerType, 0) != SUCCESS)
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
BentleyStatus SchemaWriter::UpdateBaseClasses(BaseClassChanges& baseClassChanges, ECN::ECClassCR oldClass, ECN::ECClassCR newClass)
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
                Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Removing a base class from an ECClass is not supported.",
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
                Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Adding a new base class to an ECClass is not supported.",
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
                Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Modifying the position of a base class in the list of base classes of an ECClass is not supported.",
                                oldClass.GetFullName());
                return ERROR;
                }
            }
        }
    ///Overide baseClasses
    if (overrideAllBaseClasses)
        {
        Statement stmt;
        if (stmt.Prepare(m_ecdb, "DELETE FROM main." TABLE_ClassHasBaseClasses " WHERE ClassId=?") != BE_SQLITE_OK)
            return ERROR;

        stmt.BindId(1, newClass.GetId());
        if (stmt.Step() != BE_SQLITE_DONE)
            return ERROR;

        int baseClassIndex = 0;
        for (ECClassCP baseClass : newClass.GetBaseClasses())
            {
            if (SUCCESS != ImportClass(*baseClass))
                return ERROR;

            if (SUCCESS != InsertBaseClassEntry(newClass.GetId(), *baseClass, baseClassIndex++))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateClass(ClassChange& classChange, ECClassCR oldClass, ECClassCR newClass)
    {
    if (classChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    ECClassId classId = m_ecdb.Schemas().Main().GetClassId(newClass);
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
            Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier from 'Abstract' to another value is not supported",
                            oldClass.GetFullName());

            return ERROR;
            }

        if (newValue == ECClassModifier::Sealed)
            {
            if (!newClass.GetDerivedClasses().empty())
                {
                Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier to 'Sealed' is only valid if the class does not have derived classes.",
                                oldClass.GetFullName());

                return ERROR;
                }
            }
        else if (newValue == ECClassModifier::Abstract)
            {
            Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier to 'Abstract' is not supported.",
                            oldClass.GetFullName());

            return ERROR;
            }

        updateBuilder.AddSetExp("Modifier", Enum::ToInt(classChange.GetClassModifier().GetNew().Value()));
        }

    if (classChange.ClassType().IsValid())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Changing the ECClassType of an ECClass is not supported.",
                        oldClass.GetFullName());
        return ERROR;
        }

    if (classChange.GetName().IsValid())
        {
        if (classChange.GetName().GetNew().IsNull())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Name must always be set for an ECClass.",
                            oldClass.GetFullName());
            return ERROR;
            }

        updateBuilder.AddSetExp("Name", classChange.GetName().GetNew().Value().c_str());
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
            Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s: Changing the 'Strength' of an ECRelationshipClass is not supported.",
                            oldClass.GetFullName());
            return ERROR;
            }

        if (relationshipChange.GetStrengthDirection().IsValid())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECRelationshipClass %s: Changing the 'StrengthDirection' of an ECRelationshipClass is not supported.",
                            oldClass.GetFullName());
            return ERROR;
            }

        ECRelationshipClassCP oldRel = oldClass.GetRelationshipClassCP();
        ECRelationshipClassCP newRel = newClass.GetRelationshipClassCP();
        BeAssert(oldRel != nullptr && newRel != nullptr);
        if (oldRel == nullptr && newRel == nullptr)
            return ERROR;

        if (relationshipChange.GetSource().IsValid())
            if (UpdateRelationshipConstraint(classId, relationshipChange.GetSource(), newRel->GetSource(), oldRel->GetSource(), true, oldRel->GetFullName()) == ERROR)
                return ERROR;

        if (relationshipChange.GetTarget().IsValid())
            if (UpdateRelationshipConstraint(classId, relationshipChange.GetTarget(), newRel->GetSource(), oldRel->GetTarget(), false, oldRel->GetFullName()) == ERROR)
                return ERROR;
        }

    updateBuilder.AddWhereExp("Id", classId.GetValue());
    if (updateBuilder.IsValid())
        {
        if (updateBuilder.ExecuteSql(m_ecdb) != SUCCESS)
            return ERROR;
        }


    if (classChange.BaseClasses().IsValid())
        {
        if (UpdateBaseClasses(classChange.BaseClasses(), oldClass, newClass) != SUCCESS)
            return ERROR;
        }

    if (classChange.Properties().IsValid())
        {
        if (UpdateProperties(classChange.Properties(), oldClass, newClass) != SUCCESS)
            return ERROR;
        }

    return UpdateCustomAttributes(SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, classId, classChange.CustomAttributes(), oldClass, newClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  05/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateProperties(ECPropertyChanges& propertyChanges, ECClassCR oldClass, ECClassCR newClass)
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

            if (SUCCESS != DeleteProperty(change, *oldProperty))
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

            if (SUCCESS != ImportProperty(*newProperty, ordinal))
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

            if (UpdateProperty(change, *oldProperty, *newProperty) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateSchemaReferences(ReferenceChanges& referenceChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
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
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to parse previous ECSchema reference name.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = SchemaPersistenceHelper::GetSchemaId(m_ecdb, DbTableSpace::Main(), oldRef.GetName().c_str(), SchemaLookupMode::ByName);
            Statement stmt;
            if (stmt.Prepare(m_ecdb, "DELETE FROM main." TABLE_SchemaReference " WHERE SchemaId=? AND ReferencedSchemaId=?") != BE_SQLITE_OK)
                return ERROR;

            stmt.BindId(1, oldSchema.GetId());
            stmt.BindId(2, referenceSchemaId);

            if (stmt.Step() != BE_SQLITE_DONE)
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to remove ECSchema reference %s.",
                                          oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::New)
            {
            SchemaKey newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to parse new ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Ensure schema exist
            if (!SchemaPersistenceHelper::TryGetSchemaKey(existingRef, m_ecdb, DbTableSpace::Main(), newRef.GetName().c_str()))
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Schema must exist with that or greater version
            if (!existingRef.Matches(newRef, SchemaMatchType::LatestWriteCompatible))
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Could not locate compatible referenced ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = SchemaPersistenceHelper::GetSchemaId(m_ecdb, DbTableSpace::Main(), newRef.GetName().c_str(), SchemaLookupMode::ByName);
            Statement stmt;
            if (stmt.Prepare(m_ecdb, "INSERT INTO main." TABLE_SchemaReference "(SchemaId, ReferencedSchemaId) VALUES (?,?)") != BE_SQLITE_OK)
                return ERROR;

            stmt.BindId(1, oldSchema.GetId());
            stmt.BindId(2, referenceSchemaId);

            if (stmt.Step() != BE_SQLITE_DONE)
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to add new reference to ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            SchemaKey oldRef, newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(oldRef, change.GetOld().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to parse previous ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Failed to parse new ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Ensure schema exist and also get updated version number.
            if (!SchemaPersistenceHelper::TryGetSchemaKey(existingRef, m_ecdb, DbTableSpace::Main(), oldRef.GetName().c_str()))
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Schema must exist with that or greater version
            if (!existingRef.Matches(newRef, SchemaMatchType::LatestWriteCompatible))
                {
                Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Could not locate compatible referenced ECSchema %s.",
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
bool SchemaWriter::IsSpecifiedInRelationshipConstraint(ECClassCR deletedClass) const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT NULL FROM main.ec_RelationshipConstraintClass WHERE ClassId=? LIMIT 1");
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
BentleyStatus SchemaWriter::DeleteCustomAttributeClass(ECClassCR deletedClass)
    {
    Utf8StringCR schemaName = deletedClass.GetSchema().GetName();
    if (schemaName.EqualsI("ECDbMap") || schemaName.EqualsI("ECDbSchemaPolicies"))
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECCustomAttributeClass '%s' failed. Deleting ECCustomAttributeClass from system schemas are not supported.",
                        deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    //Add Type file to ensure we are deleting customattribute class.
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("DELETE FROM main." TABLE_Class " WHERE Type=" SQLVAL_ECClassType_CustomAttribute " AND Id=?");
    stmt->BindId(1, deletedClass.GetId());
    if (stmt->Step() != BE_SQLITE_DONE)
        return ERROR;

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteClass(ClassChange& classChange, ECClassCR deletedClass)
    {
    if (!IsMajorChangeAllowedForSchema(deletedClass.GetSchema().GetId()) && m_ctx.GetOptions() != SchemaManager::SchemaImportOptions::Poisoning)
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Cannot delete ECClass '%s'. This is a major ECSchema change which requires the 'Read' version number of the ECSchema to be incremented.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (!m_ecdb.Schemas().GetDerivedClasses(deletedClass).empty())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' is not supported because it has subclasses.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsStructClass())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. ECStructClass cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (IsSpecifiedInRelationshipConstraint(deletedClass))
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. A class which is specified in a relationship constraint cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsCustomAttributeClass())
        {
        return DeleteCustomAttributeClass(deletedClass);
        }

    ClassMap const* deletedClassMap = m_ecdb.Schemas().Main().GetClassMap(deletedClass);
    if (deletedClassMap == nullptr)
        {
        BeAssert(false && "Failed to find classMap");
        return ERROR;
        }

    if (MapStrategyExtendedInfo::IsForeignKeyMapping(deletedClassMap->GetMapStrategy()))
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Deleting ECRelationshipClass with ForeignKey mapping is not supported.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    //Delete all instances
    bool purgeECInstances = deletedClassMap->GetMapStrategy().IsTablePerHierarchy();
    if (purgeECInstances)
        {
        if (DeleteInstances(deletedClass) != SUCCESS)
            return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("DELETE FROM main.ec_Class WHERE Id=?");
    stmt->BindId(1, deletedClass.GetId());
    if (stmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false && "Failed to delete the class");
        return ERROR;
        }

    for (ECPropertyCP localProperty : deletedClass.GetProperties(false))
        {
        if (DeleteCustomAttributes(localProperty->GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        }

    if (auto relationshipClass = deletedClass.GetRelationshipClassCP())
        {
        if (DeleteCustomAttributes(relationshipClass->GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        if (DeleteCustomAttributes(relationshipClass->GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        }

    return DeleteCustomAttributes(deletedClass.GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  05/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteInstances(ECClassCR deletedClass)
    {
    Utf8String ecsql("DELETE FROM ");
    ecsql.append(deletedClass.GetECSqlName());
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql.c_str(), m_ecdb.GetImpl().GetSettingsManager().GetCrudWriteToken()))
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (BE_SQLITE_DONE != stmt.Step())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteCustomAttributes(ECContainerId id, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType type)
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("DELETE FROM main.ec_CustomAttribute WHERE ContainerId=? AND ContainerType=?");
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
BentleyStatus SchemaWriter::DeleteProperty(ECPropertyChange& propertyChange, ECPropertyCR deletedProperty)
    {
    ECClassCR ecClass = deletedProperty.GetClass();
    
    if (!IsMajorChangeAllowedForSchema(deletedProperty.GetClass().GetSchema().GetId()) && m_ctx.GetOptions() != SchemaManager::SchemaImportOptions::Poisoning)
        {
        Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECProperty '%s.%s' means a major schema change, but the schema's MajorVersion is not incremented. Bump up the major version and try again.",
                        ecClass.GetSchema().GetFullSchemaName().c_str(), ecClass.GetName().c_str(), deletedProperty.GetName().c_str());
        return ERROR;
        }

    if (deletedProperty.GetIsNavigation())
        {
        //Blanket error. We do not check if relationship was also deleted. In that case we would allo nav deletion for shared column/ logical relationships
        //Fail we do not want to delete a sql column right now
        Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Deleting Navigation ECProperty '%s' from an ECClass is not supported.",
                        ecClass.GetFullName(), deletedProperty.GetName().c_str());
        return ERROR;
        }


    ClassMap const* classMap = m_ecdb.Schemas().Main().GetClassMap(ecClass);
    if (classMap == nullptr)
        {
        BeAssert(false && "Failed to find classMap");
        return ERROR;
        }

    bool sharedColumnFound = false;
    for (Partition const& partition : classMap->GetStorageDescription().GetHorizontalPartitions())
        {
        ECClassCP rootClass = m_ecdb.Schemas().GetClass(partition.GetRootClassId());
        if (rootClass == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }
        ClassMap const* partitionRootClassMap = m_ecdb.Schemas().Main().GetClassMap(*rootClass);
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
            Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Deleting an overridden ECProperty '%s' from an ECClass is not supported.",
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
                CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("DELETE FROM main.ec_Column WHERE Id=?");
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
                Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Deleting ECProperty '%s' from an ECClass which is not mapped to a shared column is not supported.",
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
        if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql.c_str(), m_ecdb.GetImpl().GetSettingsManager().GetCrudWriteToken()) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECClass %s: Deleting an ECProperty '%s' from an ECClass failed due error while setting property to null", ecClass.GetFullName(), deletedProperty.GetName().c_str());
            return ERROR;
            }
        }

    //Delete ECProperty entry make sure ec_Column is already deleted or set to null
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("DELETE FROM main.ec_Property WHERE Id=?");
    if (stmt == nullptr ||
        BE_SQLITE_OK != stmt->BindId(1, deletedProperty.GetId()) ||
        BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false && "Failed to delete ecproperty");
        return ERROR;
        }

    return DeleteCustomAttributes(deletedProperty.GetId(), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateClasses(ClassChanges& classChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
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

            if (DeleteClass(change, *oldClass) == ERROR)
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

            if (ImportClass(*newClass) == ERROR)
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

            if (UpdateClass(change, *oldClass, *newClass) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateKindOfQuantities(KindOfQuantityChanges& koqChanges, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema)
    {
    if (!koqChanges.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < koqChanges.Count(); i++)
        {
        KindOfQuantityChange& change = koqChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting KindOfQuantity from an ECSchema is not supported.",
                            oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        else if (change.GetState() == ChangeState::New)
            {
            KindOfQuantityCP koq = newSchema.GetKindOfQuantityCP(change.GetId());
            if (koq == nullptr)
                {
                BeAssert(false && "Failed to find kind of quantity");
                return ERROR;
                }

            return ImportKindOfQuantity(*koq);
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            Issues().ReportV("ECSchema Upgrade failed. KindOfQuantity %s in ECSchema %s: Changing KindOfQuantity is not supported.",
                            change.GetId(), oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdatePropertyCategories(PropertyCategoryChanges& changes, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema)
    {
    if (!changes.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < changes.Count(); i++)
        {
        PropertyCategoryChange& change = changes.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting PropertyCategory from an ECSchema is not supported.",
                            oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        else if (change.GetState() == ChangeState::New)
            {
            PropertyCategoryCP cat = newSchema.GetPropertyCategoryCP(change.GetId());
            if (cat == nullptr)
                {
                BeAssert(false && "Failed to find property category");
                return ERROR;
                }

            if (ImportPropertyCategory(*cat) != SUCCESS)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            Issues().ReportV("ECSchema Upgrade failed. PropertyCategory %s in ECSchema %s was modified which is not supported.",
                            change.GetId(), oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  01/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateEnumeration(ECEnumerationChange& enumChange, ECN::ECEnumerationCR oldEnum, ECN::ECEnumerationCR newEnum)
    {
    if (enumChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    SqlUpdateBuilder sqlUpdateBuilder(TABLE_Enumeration);

    if (enumChange.GetName().IsValid())
        {
        if (enumChange.GetName().GetNew().IsNull())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECEnumeration %s: 'Name' must always be set for an ECEnumeration.",
                oldEnum.GetFullName().c_str());

            return ERROR;
            }

        sqlUpdateBuilder.AddSetExp("Name", enumChange.GetName().GetNew().Value().c_str());
        }

    if (enumChange.GetTypeName().IsValid())
        {
        Issues().ReportV("ECSchema Upgrade failed. ECEnumeration %s: 'Type' change is not supported.",
            oldEnum.GetFullName().c_str());

        return ERROR;
        }

    if (enumChange.IsStrict().IsValid())
        {
        if (enumChange.IsStrict().GetNew().IsNull())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECEnumeration %s: 'IsStrict' must always be set for an ECEnumeration.",
                oldEnum.GetFullName().c_str());

            return ERROR;
            }

        //Allow transition from "strict" to "non-strict" but not the other way around.
        if (enumChange.IsStrict().GetOld().Value() == true &&
            enumChange.IsStrict().GetNew().Value() == false)
            sqlUpdateBuilder.AddSetExp("IsStrict", enumChange.IsStrict().GetNew().Value());
        else
            {
            Issues().ReportV("ECSchema Upgrade failed. ECEnumeration %s: 'IsStrict' changed. 'Non-strict' cannot be change to 'strict'. The other way around is allowed.",
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
        if (SUCCESS != VerifyEnumeratorChanges(oldEnum, enumeratorChanges))
            return ERROR;
    
        Utf8String enumValueJson;
        if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(enumValueJson, newEnum))
            return ERROR;

        sqlUpdateBuilder.AddSetExp("EnumValues", enumValueJson.c_str());
        }

    sqlUpdateBuilder.AddWhereExp("Id", oldEnum.GetId().GetValue());
    if (sqlUpdateBuilder.IsValid())
        {
        if (sqlUpdateBuilder.ExecuteSql(m_ecdb) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Enumerator name changes are only allowed for int enums when it has been changed from the meaningless auto-generated name that was applied during the conversion of the schema
// to EC3.2. This is a one-way opportunity to modify the auto-generated, meaningless names.
// @bsimethod                                                  Krischan.Eberle 01/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::VerifyEnumeratorChanges(ECEnumerationCR oldEnum, ECEnumeratorChanges& enumeratorChanges) const
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
                        Issues().ReportV("ECSchema Upgrade failed. An enumerator was deleted from Enumeration %s which is not supported.", oldEnum.GetFullName().c_str());
                        return ERROR;
                        }

                    break;
                case ChangeState::Modified:
                    if (change.GetInteger().IsValid() || change.GetString().IsValid())
                        {
                        Issues().ReportV("ECSchema Upgrade failed. The value of one or more enumerators of Enumeration %s was modified which is not supported.", oldEnum.GetFullName().c_str());
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
        Issues().ReportV("ECSchema Upgrade failed. An enumerator was deleted from Enumeration %s which is not supported.", oldEnum.GetFullName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateEnumerations(ECEnumerationChanges& enumChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!enumChanges.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < enumChanges.Count(); i++)
        {
        ECEnumerationChange& change = enumChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Deleting ECEnumerations from an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        else if (change.GetState() == ChangeState::New)
            {
            ECEnumerationCP ecEnum = newSchema.GetEnumerationCP(change.GetId());
            if (ecEnum == nullptr)
                {
                BeAssert(false && "Failed to find enum");
                return ERROR;
                }

            return ImportEnumeration(*ecEnum);
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            ECEnumerationCP oldEnum = oldSchema.GetEnumerationCP(change.GetId());
            ECEnumerationCP newEnum = newSchema.GetEnumerationCP(change.GetId());
            BeAssert(oldEnum != nullptr && newEnum != nullptr);
            if (oldEnum == nullptr || newEnum == nullptr)
                {
                return ERROR;
                }

            if (UpdateEnumeration(change, *oldEnum, *newEnum) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateSchema(SchemaChange& schemaChange, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (schemaChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    ECSchemaId schemaId = m_ecdb.Schemas().Main().GetSchemaId(newSchema);
    if (!schemaId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecschema id");
        return ERROR;
        }

    SqlUpdateBuilder updateBuilder(TABLE_Schema);
    if (schemaChange.GetName().IsValid())
        {
        if (schemaChange.GetName().GetNew().IsNull())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Name must always be set.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("Name", schemaChange.GetName().GetNew().Value().c_str());
        }

    if (schemaChange.GetAlias().IsValid())
        {
        if (schemaChange.GetAlias().GetNew().IsNull())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Alias must always be set.",
                             oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (m_ecdb.Schemas().Main().ContainsSchema(schemaChange.GetAlias().GetNew().Value(), SchemaLookupMode::ByAlias))
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Alias is already used by another existing ECSchema.",
                             oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("Alias", schemaChange.GetAlias().GetNew().Value().c_str());
        }

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

    const bool versionReadHasChanged = schemaChange.GetVersionRead().IsValid();
    if (versionReadHasChanged)
        {
        if (schemaChange.GetVersionRead().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionRead().GetValue(ValueId::New).Value())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionRead' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        m_majorChangesAllowedForSchemas.insert(oldSchema.GetId());
        updateBuilder.AddSetExp("VersionDigit1", schemaChange.GetVersionRead().GetNew().Value());
        }

    const bool versionWriteHasChanged = schemaChange.GetVersionWrite().IsValid();
    if (versionWriteHasChanged)
        {
        //if major version has incremented, read version may be decremented
        if (!versionReadHasChanged && schemaChange.GetVersionWrite().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionWrite().GetValue(ValueId::New).Value())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionWrite' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit2", schemaChange.GetVersionWrite().GetNew().Value());
        }

    if (schemaChange.GetVersionMinor().IsValid())
        {
        //if the higher digits have changed, minor version may be decremented
        if (!versionReadHasChanged && !versionWriteHasChanged && schemaChange.GetVersionMinor().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionMinor().GetValue(ValueId::New).Value())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionMinor' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit3", schemaChange.GetVersionMinor().GetNew().Value());
        }

    if (schemaChange.GetECVersion().IsValid())
        {
        if ((int64_t) schemaChange.GetECVersion().GetOld().Value() > (int64_t) schemaChange.GetECVersion().GetNew().Value())
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'ECVersion' of an ECSchema is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("ECVersion", (int64_t) schemaChange.GetECVersion().GetNew().Value());
        }

    const bool originalVersionMajorHasChanged = schemaChange.GetOriginalECXmlVersionMajor().IsValid();
    if (originalVersionMajorHasChanged)
        {
        uint32_t newVal = schemaChange.GetOriginalECXmlVersionMajor().GetNew().Value();
        if (schemaChange.GetOriginalECXmlVersionMajor().GetOld().Value() > newVal)
            {
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'OriginalECXmlVersionMajor' of an ECSchema is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
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
            Issues().ReportV("ECSchema Upgrade failed. ECSchema %s: Decreasing 'OriginalECXmlVersionMinor' of an ECSchema is not supported.",
                             oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("OriginalECXmlVersionMinor", newVal);
        }


    updateBuilder.AddWhereExp("Id", schemaId.GetValue());//this could even be on name
    if (updateBuilder.IsValid())
        {
        if (updateBuilder.ExecuteSql(m_ecdb) != SUCCESS)
            return ERROR;
        }

    schemaChange.SetStatus(ECChange::Status::Done);

    if (UpdateSchemaReferences(schemaChange.References(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdateEnumerations(schemaChange.Enumerations(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdateKindOfQuantities(schemaChange.KindOfQuantities(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdatePropertyCategories(schemaChange.PropertyCategories(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdateClasses(schemaChange.Classes(), oldSchema, newSchema) == ERROR)
        return ERROR;

    return UpdateCustomAttributes(SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema, schemaId, schemaChange.CustomAttributes(), oldSchema, newSchema);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
