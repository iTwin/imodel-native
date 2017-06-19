/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaWriter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

    SchemaCompareContext compareCtx;
    if (SUCCESS != compareCtx.Prepare(m_ecdb.Schemas(), primarySchemasOrderedByDependencies))
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

    if (SUCCESS != compareCtx.ReloadContextECSchemas(m_ecdb.Schemas()))
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
    const bool isValid = SchemaValidator::ValidateSchemas(m_ctx, m_ecdb.GetECDbImplR().GetIssueReporter(), primarySchemasOrderedByDependencies);
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
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting an ECSchema is not supported.",
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

    // GenerateId
    ECSchemaId schemaId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::SchemaId).GetNextValue(schemaId))
        {
        BeAssert(false && "Could not generate new ECSchemaId");
        return ERROR;
        }

    const_cast<ECSchemaR>(ecSchema).SetId(schemaId);

    if (SUCCESS != InsertSchemaEntry(ecSchema))
        {
        DbResult lastErrorCode;
        m_ecdb.GetLastError(&lastErrorCode);
        if (BE_SQLITE_CONSTRAINT_UNIQUE == lastErrorCode)
            Issues().Report("Failed to import ECSchema '%s'. Alias '%s' is already used by an existing ECSchema.",
                            ecSchema.GetFullSchemaName().c_str(), ecSchema.GetAlias().c_str());
        return ERROR;
        }

    if (SUCCESS != InsertSchemaReferenceEntries(ecSchema))
        {
        Issues().Report("Failed to import ECSchema references for ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    //enums must be imported before ECClasses as properties reference enums
    for (ECEnumerationCP ecEnum : ecSchema.GetEnumerations())
        {
        if (SUCCESS != ImportEnumeration(*ecEnum))
            {
            Issues().Report("Failed to import ECEnumeration '%s'.", ecEnum->GetFullName().c_str());
            return ERROR;
            }
        }

    //KOQs must be imported before ECClasses as properties reference KOQs
    for (KindOfQuantityCP koq : ecSchema.GetKindOfQuantities())
        {
        if (SUCCESS != ImportKindOfQuantity(*koq))
            {
            Issues().Report("Failed to import KindOfQuantity '%s'.", koq->GetFullName().c_str());
            return ERROR;
            }
        }

    //PropertyCategories must be imported before ECClasses as properties reference PropertyCategories
    for (PropertyCategoryCP cat : ecSchema.GetPropertyCategories())
        {
        if (SUCCESS != ImportPropertyCategory(*cat))
            {
            Issues().Report("Failed to import PropertyCategory '%s'.", cat->GetFullName().c_str());
            return ERROR;
            }
        }
    
    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        if (SUCCESS != ImportClass(*ecClass))
            {
            Issues().Report("Failed to import ECClass '%s'.", ecClass->GetFullName());
            return ERROR;
            }
        }

    if (SUCCESS != ImportCustomAttributes(ecSchema, ECContainerId(schemaId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema))
        {
        Issues().Report("Failed to import custom attributes of ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
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
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "INSERT INTO ec_SchemaReference(Id,SchemaId,ReferencedSchemaId) VALUES(?,?,?)"))
        return ERROR;

    for (bpair<SchemaKey, ECSchemaPtr> const& kvPair : references)
        {
        ECSchemaCP reference = kvPair.second.get();
        ECSchemaId referenceId = SchemaPersistenceHelper::GetSchemaId(m_ecdb, reference->GetName().c_str());
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

        BeInt64Id id;
        if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::SchemaReferenceId).GetNextValue(id) != BE_SQLITE_OK)
            {
            BeAssert(false);
            return ERROR;
            }

        if (BE_SQLITE_OK != stmt.BindId(1, id))
            return ERROR;

        if (BE_SQLITE_OK != stmt.BindId(2, schema.GetId()))
            return ERROR;

        if (BE_SQLITE_OK != stmt.BindId(3, referenceId))
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
    if (m_ecdb.Schemas().GetReader().GetClassId(ecClass).IsValid())
        return SUCCESS;

    if (!m_ecdb.Schemas().GetReader().GetSchemaId(ecClass.GetSchema()).IsValid())
        {
        Issues().Report("Failed to import ECClass '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ecClass.GetName().c_str(), ecClass.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import ECClass because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    // GenerateId
    ECClassId ecClassId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::ClassId).GetNextValue(ecClassId))
        return ERROR;

    const_cast<ECClassR>(ecClass).SetId(ecClassId);

    //now import actual ECClass
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Class(Id,SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection,CustomAttributeContainerType) "
                                                  "VALUES(?,?,?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecClass.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecClass.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecClass.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecClass.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!ecClass.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecClass.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(6, Enum::ToInt(ecClass.GetClassType())))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, Enum::ToInt(ecClass.GetClassModifier())))
        return ERROR;

    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass != nullptr)
        {
        if (BE_SQLITE_OK != stmt->BindInt(8, Enum::ToInt(relClass->GetStrength())))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(9, Enum::ToInt(relClass->GetStrengthDirection())))
            return ERROR;
        }

    ECCustomAttributeClassCP caClass = ecClass.GetCustomAttributeClassCP();
    if (caClass != nullptr && caClass->GetContainerType() != CustomAttributeContainerType::Any)
        {
        if (BE_SQLITE_OK != stmt->BindInt(10, Enum::ToInt(caClass->GetContainerType())))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    //release stmt so that it can be reused to insert base classes
    stmt = nullptr;

    //Import All baseCases
    int baseClassIndex = 0;
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        if (SUCCESS != ImportClass(*baseClass))
            return ERROR;

        if (SUCCESS != InsertBaseClassEntry(ecClassId, *baseClass, baseClassIndex++))
            return ERROR;
        }

    int propertyIndex = 0;
    for (ECPropertyCP ecProperty : ecClass.GetProperties(false))
        {
        if (SUCCESS != ImportProperty(*ecProperty, propertyIndex++))
            {
            LOG.errorv("Failed to import ECProperty '%s' of ECClass '%s'.", ecProperty->GetName().c_str(), ecClass.GetFullName());
            return ERROR;
            }
        }

    ECN::ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr)
        {
        if (SUCCESS != ImportRelationshipClass(relationship))
            return ERROR;
        }

    return ImportCustomAttributes(ecClass, ECContainerId(ecClassId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportEnumeration(ECEnumerationCR ecEnum)
    {
    if (m_ecdb.Schemas().GetReader().GetEnumerationId(ecEnum).IsValid())
        return SUCCESS;

    if (!m_ecdb.Schemas().GetReader().GetSchemaId(ecEnum.GetSchema()).IsValid())
        {
        Issues().Report("Failed to import ECEnumeration '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", ecEnum.GetName().c_str(), ecEnum.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import ECEnumeration because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_Enumeration(Id, SchemaId, Name, DisplayLabel, Description, UnderlyingPrimitiveType, IsStrict, EnumValues) VALUES(?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    ECEnumerationId enumId;
    if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::EnumId).GetNextValue(enumId))
        return ERROR;

    const_cast<ECEnumerationR>(ecEnum).SetId(enumId);
    if (BE_SQLITE_OK != stmt->BindId(1, enumId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecEnum.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecEnum.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecEnum.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecEnum.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!ecEnum.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecEnum.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(6, (int) ecEnum.GetType()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindBoolean(7, ecEnum.GetIsStrict()))
        return ERROR;

    Utf8String enumValueJson;
    if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(enumValueJson, ecEnum))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(8, enumValueJson, Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportKindOfQuantity(KindOfQuantityCR koq)
    {
    if (m_ecdb.Schemas().GetReader().GetKindOfQuantityId(koq).IsValid())
        return SUCCESS;

    if (!m_ecdb.Schemas().GetReader().GetSchemaId(koq.GetSchema()).IsValid())
        {
        Issues().Report("Failed to import KindOfQuantity '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", koq.GetName().c_str(), koq.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import KindOfQuantity because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_KindOfQuantity(Id,SchemaId,Name,DisplayLabel,Description,PersistenceUnit,RelativeError,PresentationUnits) VALUES(?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    KindOfQuantityId koqId;
    if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::KoqId).GetNextValue(koqId))
        return ERROR;

    const_cast<KindOfQuantityR>(koq).SetId(koqId);
    if (BE_SQLITE_OK != stmt->BindId(1, koqId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, koq.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, koq.GetName(), Statement::MakeCopy::No))
        return ERROR;
   
    if (koq.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, koq.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }
   
    if (!koq.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, koq.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (koq.GetPersistenceUnit().HasProblem())
        {
        Issues().Report("Failed to import KindOfQuantity '%s'. Its persistence unit is invalid: %s.", koq.GetFullName().c_str(), koq.GetPersistenceUnit().GetProblemDescription().c_str());
        return ERROR;
        }

    Utf8String persistenceUnitStr = koq.GetPersistenceUnit().ToText(false);
    BeAssert(!persistenceUnitStr.empty());
    if (BE_SQLITE_OK != stmt->BindText(6, persistenceUnitStr, Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindDouble(7, koq.GetRelativeError()))
        return ERROR;


    Utf8String presUnitsJsonStr;
    if (!koq.GetPresentationUnitList().empty())
        {
        if (SUCCESS != SchemaPersistenceHelper::SerializeKoqPresentationUnits(presUnitsJsonStr, m_ecdb, koq))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindText(8, presUnitsJsonStr, Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::ImportPropertyCategory(PropertyCategoryCR cat)
    {
    if (m_ecdb.Schemas().GetReader().GetPropertyCategoryId(cat).IsValid())
        return SUCCESS;

    if (!m_ecdb.Schemas().GetReader().GetSchemaId(cat.GetSchema()).IsValid())
        {
        Issues().Report("Failed to import PropertyCategory '%s'. Its ECSchema '%s' hasn't been imported yet. Check the list of ECSchemas passed to ImportSchema for missing schema references.", cat.GetName().c_str(), cat.GetSchema().GetFullSchemaName().c_str());
        BeAssert(false && "Failed to import PropertyCategory because its ECSchema hasn't been imported yet. The schema references of the ECSchema objects passed to ImportSchema might be corrupted.");
        return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_PropertyCategory(Id,SchemaId,Name,DisplayLabel,Description,Priority) VALUES(?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    PropertyCategoryId catId;
    if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::PropertyCategoryId).GetNextValue(catId))
        return ERROR;

    const_cast<PropertyCategoryR>(cat).SetId(catId);
    if (BE_SQLITE_OK != stmt->BindId(1, catId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, cat.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, cat.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (cat.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, cat.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!cat.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, cat.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    //uint32_t persisted as int64 to not lose unsignedness
    if (BE_SQLITE_OK != stmt->BindInt64(6, (int64_t) cat.GetPriority()))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_RelationshipConstraintClass(Id,ConstraintId,ClassId) VALUES(?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    for (ECClassCP constraintClass : relationshipConstraint.GetConstraintClasses())
        {
        if (SUCCESS != ImportClass(*constraintClass))
            return ERROR;

        BeAssert(constraintClass->GetId().IsValid());

        BeInt64Id id;
        if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::RelationshipConstraintClassId).GetNextValue(id))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(1, id))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(2, constraintId))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(3, constraintClass->GetId()))
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
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraint(Id,RelationshipClassId,RelationshipEnd,MultiplicityLowerLimit,MultiplicityUpperLimit,IsPolymorphic,RoleLabel,AbstractConstraintClassId) VALUES(?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::RelationshipConstraintId).GetNextValue(constraintId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, constraintId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, (int) endpoint))
        return ERROR;

    //uint32_t are persisted as int64 to not lose the unsigned-ness.
    if (BE_SQLITE_OK != stmt->BindInt64(4, (int64_t) relationshipConstraint.GetMultiplicity().GetLowerLimit()))
        return ERROR;

    //If unbounded, we persist DB NULL
    if (!relationshipConstraint.GetMultiplicity().IsUpperLimitUnbounded())
        {
        //uint32_t are persisted as int64 to not lose the unsigned-ness.
        if (BE_SQLITE_OK != stmt->BindInt64(5, (int64_t) relationshipConstraint.GetMultiplicity().GetUpperLimit()))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindBoolean(6, relationshipConstraint.GetIsPolymorphic()))
        return ERROR;

    
    if (!relationshipConstraint.GetRoleLabel().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(7, relationshipConstraint.GetRoleLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (relationshipConstraint.IsAbstractConstraintDefined())
        {
        ECClassCR abstractConstraintClass = *relationshipConstraint.GetAbstractConstraint();
        if (SUCCESS != ImportClass(abstractConstraintClass))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(8, abstractConstraintClass.GetId()))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::ImportProperty(ECN::ECPropertyCR ecProperty, int ordinal)
    {
    //Local properties are expected to not be imported at this point as they get imported along with their class.
    BeAssert(!m_ecdb.Schemas().GetReader().GetPropertyId(ecProperty).IsValid());

    // GenerateId
    ECPropertyId ecPropertyId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::PropertyId).GetNextValue(ecPropertyId))
        return ERROR;

    const_cast<ECPropertyR>(ecProperty).SetId(ecPropertyId);

    if (ecProperty.GetIsStruct())
        {
        if (SUCCESS != ImportClass(ecProperty.GetAsStructProperty()->GetType()))
            return ERROR;
        }
    else if (ecProperty.GetIsArray())
        {
        StructArrayECPropertyCP structArrayProperty = ecProperty.GetAsStructArrayProperty();
        if (nullptr != structArrayProperty)
            {
            if (SUCCESS != ImportClass(structArrayProperty->GetStructElementType()))
                return ERROR;
            }
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (SUCCESS != ImportClass(*ecProperty.GetAsNavigationProperty()->GetRelationshipClass()))
            return ERROR;
        }

    //now insert the actual property
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_Property(Id,ClassId,Name,DisplayLabel,Description,IsReadonly,Ordinal,Kind,PrimitiveType,EnumerationId,StructClassId,ExtendedTypeName,KindOfQuantityId,CategoryId,ArrayMinOccurs,ArrayMaxOccurs,NavigationRelationshipClassId,NavigationDirection) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecProperty.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecProperty.GetClass().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecProperty.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecProperty.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecProperty.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }
    
    if (!ecProperty.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecProperty.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindBoolean(6, ecProperty.GetIsReadOnly()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, ordinal))
        return ERROR;

    const int kindIndex = 8;
    const int primitiveTypeIndex = 9;
    const int enumIdIndex = 10;
    const int structClassIdIndex = 11;
    const int extendedTypeIndex = 12;
    const int koqIdIndex = 13;
    const int catIdIndex = 14;
    const int arrayMinIndex = 15;
    const int arrayMaxIndex = 16;
    const int navRelClassIdIndex = 17;
    const int navDirIndex = 18;

    if (ecProperty.GetIsPrimitive())
        {
        PrimitiveECPropertyCP primProp = ecProperty.GetAsPrimitiveProperty();
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(PropertyKind::Primitive)))
            return ERROR;

        ECEnumerationCP ecenum = primProp->GetEnumeration();
        if (ecenum == nullptr)
            {
            if (BE_SQLITE_OK != stmt->BindInt(primitiveTypeIndex, (int) primProp->GetType()))
                return ERROR;
            }
        else
            {
            if (SUCCESS != ImportEnumeration(*ecenum))
                return ERROR;

            BeAssert(ecenum->HasId());
            if (BE_SQLITE_OK != stmt->BindId(enumIdIndex, ecenum->GetId()))
                return ERROR;
            }

        if (SUCCESS != BindPropertyExtendedTypeName(*stmt, extendedTypeIndex, *primProp))
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

            if (BE_SQLITE_OK != stmt->BindInt(primitiveTypeIndex, (int) arrayProp->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType()))
                return ERROR;

            if (SUCCESS != BindPropertyExtendedTypeName(*stmt, extendedTypeIndex, *arrayProp->GetAsPrimitiveArrayProperty()))
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

    //KOQs are allowed for all property kinds except for nav props (this will be caught be ECObjects already
    //and is checked again within this method)
    if (SUCCESS != BindPropertyKindOfQuantityId(*stmt, koqIdIndex, ecProperty))
        return ERROR;

    //PropertyCategories are allowed for all property kinds
    if (SUCCESS != BindPropertyCategoryId(*stmt, catIdIndex, ecProperty))
        return ERROR;

    DbResult stat = stmt->Step();
    if (BE_SQLITE_DONE != stat)
        {
        LOG.fatal(m_ecdb.GetLastError().c_str());
        return ERROR;
        }

    return ImportCustomAttributes(ecProperty, ECContainerId(ecPropertyId), SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
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
BentleyStatus SchemaWriter::InsertSchemaEntry(ECSchemaCR ecSchema)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Schema(Id,Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3) VALUES(?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchema.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, ecSchema.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecSchema.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ecSchema.GetInvariantDisplayLabel(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (!ecSchema.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecSchema.GetInvariantDescription(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ecSchema.GetAlias(), Statement::MakeCopy::No))
        return ERROR;

    //Persist uint32_t as int64 to not lose unsigned-ness
    if (BE_SQLITE_OK != stmt->BindInt64(6, (int64_t) ecSchema.GetVersionRead()))
        return ERROR;

    //Persist uint32_t as int64 to not lose unsigned-ness
    if (BE_SQLITE_OK != stmt->BindInt64(7, (int64_t) ecSchema.GetVersionWrite()))
        return ERROR;

    //Persist uint32_t as int64 to not lose unsigned-ness
    if (BE_SQLITE_OK != stmt->BindInt64(8, (int64_t) ecSchema.GetVersionMinor()))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaWriter::InsertBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int ordinal)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_ClassHasBaseClasses(Id,ClassId,BaseClassId,Ordinal) VALUES(?,?,?,?)"))
        return ERROR;

    BeInt64Id id;
    if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::ClassHasBaseClassesId).GetNextValue(id))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, id))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(3, baseClass.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(4, ordinal))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyExtendedTypeName(Statement& stmt, int paramIndex, PrimitiveECPropertyCR prop)
    {
    if (!prop.HasExtendedType() || prop.GetExtendedTypeName().empty())
        return SUCCESS;

    return stmt.BindText(paramIndex, prop.GetExtendedTypeName(), Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyExtendedTypeName(Statement& stmt, int paramIndex, PrimitiveArrayECPropertyCR prop)
    {
    if (!prop.HasExtendedType() || prop.GetExtendedTypeName().empty())
        return SUCCESS;

    return stmt.BindText(paramIndex, prop.GetExtendedTypeName(), Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::BindPropertyKindOfQuantityId(Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    if (!prop.IsKindOfQuantityDefinedLocally() || prop.GetKindOfQuantity() == nullptr)
        return SUCCESS;

    if (prop.GetIsNavigation())
        {
        Issues().Report("Failed to import Navigation ECProperty '%s.%s' because a KindOfQuantity is assigned to it.", prop.GetClass().GetFullName(), prop.GetName().c_str());
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
BentleyStatus SchemaWriter::BindPropertyCategoryId(Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    if (!prop.IsPropertyCategoryDefinedLocally() || prop.GetPropertyCategory() == nullptr)
        return SUCCESS;

    PropertyCategoryCP cat = prop.GetPropertyCategory();
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
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_CustomAttribute(Id,ContainerId,ContainerType,ClassId,Ordinal,Instance) VALUES(?,?,?,?,?,?)"))
        return ERROR;

    BeInt64Id id;
    if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::CustomAttributeId).GetNextValue(id))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, id))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(4, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(5, ordinal))
        return ERROR;

    Utf8String caXml;
    if (InstanceWriteStatus::Success != customAttribute.WriteToXmlString(caXml, false, //don't write XML description header as we only store an XML fragment
                                                                          true)) //store instance id for the rare cases where the client specified one.
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(6, caXml, Statement::MakeCopy::No))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteCAEntry(int& ordinal, ECClassId ecClassId, ECContainerId containerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "SELECT Ordinal FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(3, ecClassId))
        return ERROR;

    if (stmt->Step() != BE_SQLITE_ROW)
        return ERROR;

    ordinal = stmt->GetValueInt(0);

    stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "DELETE FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?"))
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

    ECPropertyId propertyId = m_ecdb.Schemas().GetReader().GetPropertyId(newProperty);
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
        Issues().Report("ECSchema Upgrade failed. ECProperty %s.%s: Changing the kind of the ECProperty is not supported.",
                                  oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return ERROR;
        }

    if (propertyChange.GetArray().IsValid())
        {
        ArrayChange& arrayChange = propertyChange.GetArray();
        if (arrayChange.MaxOccurs().IsValid() || arrayChange.MinOccurs().IsValid())
            {
            Issues().Report("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'MinOccurs' or 'MaxOccurs' for an Array ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }
        }

    if (propertyChange.GetNavigation().IsValid())
        {
        NavigationChange& navigationChange = propertyChange.GetNavigation();
        if (navigationChange.GetRelationshipClassName().IsValid())
            {
            Issues().Report("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'RelationshipClassName' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

        if (navigationChange.Direction().IsValid())
            {
            Issues().Report("ECSchema Upgrade failed. ECProperty %s.%s: Changing the 'Direction' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }
        }

    SqlUpdateBuilder sqlUpdateBuilder("ec_Property");

    if (propertyChange.GetName().IsValid())
        {
        if (propertyChange.GetName().GetNew().IsNull())
            {
            Issues().Report("ECSchema Upgrade failed. ECProperty %s.%s: 'Name' must always be set for an ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

        sqlUpdateBuilder.AddSetExp("Name", propertyChange.GetName().GetNew().Value().c_str());
        }

    if (propertyChange.GetExtendedTypeName().IsValid())
        {
        if (propertyChange.GetExtendedTypeName().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("ExtendedTypeName");
        else
            sqlUpdateBuilder.AddSetExp("ExtendedTypeName", propertyChange.GetExtendedTypeName().GetNew().Value().c_str());
        }

    if (propertyChange.GetDisplayLabel().IsValid())
        {
        if (propertyChange.GetDisplayLabel().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("DisplayLabel");
        else
            sqlUpdateBuilder.AddSetExp("DisplayLabel", propertyChange.GetDisplayLabel().GetNew().Value().c_str());
        }

    if (propertyChange.GetDescription().IsValid())
        {
        if (propertyChange.GetDescription().GetNew().IsNull())
            sqlUpdateBuilder.AddSetToNull("Description");
        else
            sqlUpdateBuilder.AddSetExp("Description", propertyChange.GetDescription().GetNew().Value().c_str());
        }

    if (propertyChange.IsReadonly().IsValid())
        {
        sqlUpdateBuilder.AddSetExp("IsReadonly", propertyChange.IsReadonly().GetNew().Value());
        }

    if (propertyChange.GetEnumeration().IsValid())
        {
        auto newPrimitiveProperty = newProperty.GetAsPrimitiveProperty();
        if (newPrimitiveProperty == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        if (propertyChange.GetEnumeration().GetNew().IsNull())
            { 
            sqlUpdateBuilder.AddSetExp("PrimitiveType", (int)newPrimitiveProperty->GetType()); //set to null;
            sqlUpdateBuilder.AddSetToNull("EnumerationId"); //set to null;
            }
        else
            {
            auto oldPrimitiveProperty = oldProperty.GetAsPrimitiveProperty();
            if (oldPrimitiveProperty == nullptr)
                {
                Issues().Report("ECSchema Upgrade failed. ECProperty %s.%s: Only Primitive property can be coverted to ECEnumeration",
                                oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
                return ERROR;
                }

            ECEnumerationCP enumCP = newPrimitiveProperty->GetEnumeration();
            ECEnumerationId id = m_ecdb.Schemas().GetReader().GetEnumerationId(*enumCP);
            if (!id.IsValid())
                return ERROR;

            sqlUpdateBuilder.AddSetToNull("PrimitiveType"); //SET TO NULL
            sqlUpdateBuilder.AddSetExp("EnumerationId", id.GetValue());
            }
        }

    if (propertyChange.GetKindOfQuantity().IsValid())
        {
        if (propertyChange.GetKindOfQuantity().GetState() == ChangeState::Deleted)
            {
            sqlUpdateBuilder.AddSetToNull("KindOfQuantityId"); //set to null;
            }
        else
            {
            KindOfQuantityCP koqCP = nullptr;
            if (auto newPrimitiveProperty = newProperty.GetAsPrimitiveProperty())
                {
                koqCP = newPrimitiveProperty->GetKindOfQuantity();
                }
            else if (auto newPrimitivePropertyArray = newProperty.GetAsPrimitiveArrayProperty())
                {
                koqCP = newPrimitivePropertyArray->GetKindOfQuantity();
                }

            if (koqCP == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            KindOfQuantityId id = m_ecdb.Schemas().GetReader().GetKindOfQuantityId(*koqCP);
            if (!id.IsValid())
                return ERROR;

            sqlUpdateBuilder.AddSetExp("KindOfQuantityId", id.GetValue());
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
        Issues().Report("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing 'Multiplicity' of an ECRelationshipConstraint is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.IsPolymorphic().IsValid())
        {
        Issues().Report("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing flag 'IsPolymorphic' of an ECRelationshipConstraint is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.ConstraintClasses().IsValid())
        {
        Issues().Report("ECSchema Upgrade failed. ECRelationshipClass %s - Constraint: %s: Changing the constraint classes is not supported.",
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
BentleyStatus SchemaWriter::TryParseId(Utf8StringR schemaName, Utf8StringR className, Utf8StringCR id) const
    {
    auto n = id.find(':');
    BeAssert(n != Utf8String::npos);
    if (n == Utf8String::npos)
        {
        return ERROR;
        }
    schemaName = id.substr(0, n);
    className = id.substr(n + 1);
    return SUCCESS;
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

        Utf8String schemaName;
        Utf8String className;
        if (TryParseId(schemaName, className, change.GetId()) == ERROR)
            return ERROR;

        if (change.GetParent()->GetState() != ChangeState::New)
            {
            if (m_schemaUpgradeCustomAttributeValidator.HasAnyRuleForSchema(schemaName.c_str()))
                {
                if (m_schemaUpgradeCustomAttributeValidator.Validate(change) == CustomAttributeValidator::Policy::Reject)
                    {
                    Issues().Report("ECSchema Upgrade failed. Adding or modifying %s CustomAttributes is not supported.", schemaName.c_str());
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
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::UpdateClass(ClassChange& classChange, ECClassCR oldClass, ECClassCR newClass)
    {
    if (classChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    ECClassId classId = m_ecdb.Schemas().GetReader().GetClassId(newClass);
    if (!classId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecclass id");
        return ERROR;
        }

    SqlUpdateBuilder updateBuilder("ec_Class");

    if (classChange.GetClassModifier().IsValid())
        {
        Nullable<ECClassModifier> oldValue = classChange.GetClassModifier().GetOld();
        ECClassModifier newValue = classChange.GetClassModifier().GetNew().Value();
        if (oldValue == ECClassModifier::Abstract)
            {
            Issues().Report("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier from 'Abstract' to another value is not supported",
                            oldClass.GetFullName());

            return ERROR;
            }

        if (newValue == ECClassModifier::Sealed)
            {
            if (!newClass.GetDerivedClasses().empty())
                {
                Issues().Report("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier to 'Sealed' is only valid if the class does not have derived classes.",
                                          oldClass.GetFullName());

                return ERROR;
                }
            }
        else if (newValue == ECClassModifier::Abstract)
            {
            Issues().Report("ECSchema Upgrade failed. ECClass %s: Changing the ECClassModifier to 'Abstract' is not supported.",
                                      oldClass.GetFullName());

            return ERROR;
            }

        updateBuilder.AddSetExp("Modifier", Enum::ToInt(classChange.GetClassModifier().GetNew().Value()));
        }

    if (classChange.ClassType().IsValid())
        {
        Issues().Report("ECSchema Upgrade failed. ECClass %s: Changing the ECClassType of an ECClass is not supported.",
                                  oldClass.GetFullName());
        return ERROR;
        }

    if (classChange.GetName().IsValid())
        {
        if (classChange.GetName().GetNew().IsNull())
            {
            Issues().Report("ECSchema Upgrade failed. ECClass %s: Name must always be set for an ECClass.",
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
            Issues().Report("ECSchema Upgrade failed. ECRelationshipClass %s: Changing the 'Strength' of an ECRelationshipClass is not supported.",
                                      oldClass.GetFullName());
            return ERROR;
            }

        if (relationshipChange.GetStrengthDirection().IsValid())
            {
            Issues().Report("ECSchema Upgrade failed. ECRelationshipClass %s: Changing the 'StrengthDirection' of an ECRelationshipClass is not supported.",
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
            if (UpdateRelationshipConstraint(classId,  relationshipChange.GetTarget(), newRel->GetSource(), oldRel->GetTarget(), false, oldRel->GetFullName()) == ERROR)
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
        for (size_t i = 0; i < classChange.BaseClasses().Count(); i++)
            {
            auto& change = classChange.BaseClasses().At(i);
            if (change.GetState() == ChangeState::Deleted)
                {
                Issues().Report("ECSchema Upgrade failed. ECClass %s: Removing a base class from an ECClass is not supported.",
                                          oldClass.GetFullName());
                return ERROR;
                }
            else if (change.GetState() == ChangeState::New)
                {
                Issues().Report("ECSchema Upgrade failed. ECClass %s: Adding a new base class to an ECClass is not supported.",
                                          oldClass.GetFullName());
                return ERROR;
                }
            else if (change.GetState() == ChangeState::Modified)
                {
                Issues().Report("ECSchema Upgrade failed. ECClass %s: Modifying the position of a base class in the list of base classes of an ECClass is not supported.",
                                          oldClass.GetFullName());
                return ERROR;
                }
            }
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
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Failed to parse previous ECSchema reference name.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = SchemaPersistenceHelper::GetSchemaId(m_ecdb, oldRef.GetName().c_str());
            Statement stmt;
            if (stmt.Prepare(m_ecdb, "DELETE FROM ec_SchemaReference WHERE SchemaId=? AND ReferencedSchemaId=?") != BE_SQLITE_OK)
                return ERROR;

            stmt.BindId(1, oldSchema.GetId());
            stmt.BindId(2, referenceSchemaId);

            if (stmt.Step() != BE_SQLITE_DONE)
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Failed to remove ECSchema reference %s.",
                                          oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::New)
            {
            SchemaKey newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Failed to parse new ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Ensure schema exist
            if (!SchemaPersistenceHelper::TryGetSchemaKey(existingRef, m_ecdb, newRef.GetName().c_str()))
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Schema must exist with that or greater version
            if (!existingRef.Matches(newRef, SchemaMatchType::LatestWriteCompatible))
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Could not locate compatible referenced ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = SchemaPersistenceHelper::GetSchemaId(m_ecdb, newRef.GetName().c_str());
            Statement stmt;
            if (stmt.Prepare(m_ecdb, "INSERT INTO ec_SchemaReference(SchemaId, ReferencedSchemaId) VALUES (?,?)") != BE_SQLITE_OK)
                return ERROR;

            stmt.BindId(1, oldSchema.GetId());
            stmt.BindId(2, referenceSchemaId);

            if (stmt.Step() != BE_SQLITE_DONE)
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Failed to add new reference to ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            SchemaKey oldRef, newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(oldRef, change.GetOld().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Failed to parse previous ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Failed to parse new ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Ensure schema exist and also get updated version number.
            if (!SchemaPersistenceHelper::TryGetSchemaKey(existingRef, m_ecdb, oldRef.GetName().c_str()))
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Schema must exist with that or greater version
            if (!existingRef.Matches(newRef, SchemaMatchType::LatestWriteCompatible))
                {
                Issues().Report("ECSchema Upgrade failed. ECSchema %s: Could not locate compatible referenced ECSchema %s.",
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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT NULL FROM ec_RelationshipConstraintClass WHERE ClassId=? LIMIT 1");
    if (stmt == nullptr)
        {
        BeAssert(false && "SQL_SCHEMA_CHANGED");
        return true;
        }

    stmt->BindId(1, deletedClass.GetId());
    return BE_SQLITE_ROW == stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaWriter::DeleteClass(ClassChange& classChange, ECClassCR deletedClass)
    {
    if (!IsMajorChangeAllowedForSchema(deletedClass.GetSchema().GetId()) && m_ctx.GetOptions() != SchemaManager::SchemaImportOptions::Poisoning)
        {
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Cannot delete ECClass '%s'. This is a major ECSchema change which requires the 'Read' version number of the ECSchema to be incremented.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }
    
    if (!m_ecdb.Schemas().GetDerivedClasses(deletedClass).empty())
        {
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' is not supported because it has subclasses.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsStructClass())
        {
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. ECStructClass cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsCustomAttributeClass())
        {
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. ECCustomAttributeClass cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (IsSpecifiedInRelationshipConstraint(deletedClass))
        {
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. A class which is specified in a relationship constraint cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    ClassMapCP deletedClassMap = m_ecdb.Schemas().GetDbMap().GetClassMap(deletedClass);
    if (deletedClassMap == nullptr)
        {
        BeAssert(false && "Failed to find classMap");
        return ERROR;
        }

    if (MapStrategyExtendedInfo::IsForeignKeyMapping(deletedClassMap->GetMapStrategy()))
        {
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Deleting ECRelationshipClass with ForeignKey mapping is not supported.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    //Delete all instances
    bool purgeECInstances = deletedClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy;
    if (purgeECInstances)
        {
        if (DeleteInstances(deletedClass) != SUCCESS)
            return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_Class WHERE Id=?");
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
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql.c_str(), m_ecdb.GetECDbImplR().GetSettings().GetCrudWriteToken()))
        {
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (BE_SQLITE_DONE != stmt.Step())
        {
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_CustomAttribute WHERE ContainerId=? AND ContainerType=?");
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
        Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECProperty '%s.%s' means a major schema change, but the schema's MajorVersion is not incremented. Bump up the major version and try again.",
                        ecClass.GetSchema().GetFullSchemaName().c_str(), ecClass.GetName().c_str(), deletedProperty.GetName().c_str());
        return ERROR;
        }

    ClassMapCP classMap = m_ecdb.Schemas().GetDbMap().GetClassMap(ecClass);
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
        ClassMapCP partitionRootClassMap = m_ecdb.Schemas().GetDbMap().GetClassMap(*rootClass);
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
            Issues().Report("ECSchema Upgrade failed. ECClass %s: Deleting an overridden ECProperty '%s' from an ECClass is not supported.",
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
                CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_Column WHERE Id=?");
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
                Issues().Report("ECSchema Upgrade failed. ECClass %s: Deleting ECProperty '%s' from an ECClass which is not mapped to a shared column is not supported.",
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
        if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql.c_str(), m_ecdb.GetECDbImplR().GetSettings().GetCrudWriteToken()) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            Issues().Report("ECSchema Upgrade failed. ECClass %s: Deleting an ECProperty '%s' from an ECClass failed due error while setting property to null", ecClass.GetFullName(), deletedProperty.GetName().c_str());
            return ERROR;
            }
        }

    //Delete ECProperty entry make sure ec_Column is already deleted or set to null
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_Property WHERE Id=?");
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
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting KindOfQuantity from an ECSchema is not supported.",
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
            Issues().Report("ECSchema Upgrade failed. KindOfQuantity %s in ECSchema %s: Changing KindOfQuantity is not supported.",
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
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting PropertyCategory from an ECSchema is not supported.",
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

            return ImportPropertyCategory(*cat);
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            Issues().Report("ECSchema Upgrade failed. PropertyCategory %s in ECSchema %s: Changing PropertyCategory is not supported.",
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

    //CREATE TABLE ec_Enumeration(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,UnderlyingPrimitiveType INTEGER NOT NULL,IsStrict BOOLEAN NOT NULL CHECK(IsStrict IN (0,1)),EnumValues TEXT NOT NULL);
    SqlUpdateBuilder sqlUpdateBuilder("ec_Enumeration");
    
    if (enumChange.GetName().IsValid())
        {
        if (enumChange.GetName().GetNew().IsNull())
            {
            Issues().Report("ECSchema Upgrade failed. ECEnumeration %s: 'Name' must always be set for an ECEnumeration.",
                oldEnum.GetFullName().c_str());

            return ERROR;
            }

        sqlUpdateBuilder.AddSetExp("Name", enumChange.GetName().GetNew().Value().c_str());
        }

    if (enumChange.GetTypeName().IsValid())
        {
        Issues().Report("ECSchema Upgrade failed. ECEnumeration %s: 'Type' change is not supported.",
            oldEnum.GetFullName().c_str());

        return ERROR;
        }

    bool allowDisruptiveChanges = !newEnum.GetIsStrict();
    if (enumChange.IsStrict().IsValid())
        {
        if (enumChange.IsStrict().GetNew().IsNull())
            {
            Issues().Report("ECSchema Upgrade failed. ECEnumeration %s: 'IsStrict' must always be set for an ECEnumeration.",
                oldEnum.GetFullName().c_str());

            return ERROR;
            }

        //Allow transition from "strict" to "non-strict" but not the other way around.
        if (enumChange.IsStrict().GetOld().Value() == true &&
            enumChange.IsStrict().GetNew().Value() == false)
            {
            allowDisruptiveChanges = true;
            sqlUpdateBuilder.AddSetExp("IsStrict", enumChange.IsStrict().GetNew().Value());
            }
        else
            {
            Issues().Report("ECSchema Upgrade failed. ECEnumeration %s: 'IsStrict' changed. 'None-strict' cannot be change to 'strict'. The other way around is allowed.",
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



    ECEnumeratorChanges enumeratorChanges = enumChange.Enumerators();
    if (enumeratorChanges.IsValid())
        {
        if (allowDisruptiveChanges)
            {
            Utf8String enumValueJson;
            if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(enumValueJson, newEnum))
                return ERROR;

            sqlUpdateBuilder.AddSetExp("EnumValues", enumValueJson.c_str());
            }
        else
            {
            size_t newEnumerators = 0;
            for (size_t i = 0; i < enumeratorChanges.Count(); i++)
                {
                ECEnumeratorChange& change = enumeratorChanges.At(i);
                if (change.GetState() == ChangeState::Deleted)
                    {
                    Issues().Report("ECSchema Upgrade failed. Enumerator %s was deleted from Enumeration %s which is not supported.",
                        change.GetId(), oldEnum.GetFullName().c_str());

                    return ERROR;
                    }
                else if (change.GetState() == ChangeState::New)
                    {
                    newEnumerators++;
                    }
                else if (change.GetState() == ChangeState::Modified)
                    {
                    Issues().Report("ECSchema Upgrade failed. Enumerator %s was updated from Enumeration %s which is not supported.",
                        change.GetId(), oldEnum.GetFullName().c_str());

                    return ERROR;
                    }
                }

            if (newEnumerators > 0)
                {
                Utf8String enumValueJson;
                if (SUCCESS != SchemaPersistenceHelper::SerializeEnumerationValues(enumValueJson, newEnum))
                    return ERROR;

                sqlUpdateBuilder.AddSetExp("EnumValues", enumValueJson.c_str());
                }
            }
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
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Deleting ECEnumerations from an ECSchema is not supported.",
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
   
    ECSchemaId schemaId = m_ecdb.Schemas().GetReader().GetSchemaId(newSchema);
    if (!schemaId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecschema id");
        return ERROR;
        }

    SqlUpdateBuilder updateBuilder("ec_Schema");
    if (schemaChange.GetName().IsValid())
        {
        if (schemaChange.GetName().GetNew().IsNull())
            {
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Name must always be set.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("Name", schemaChange.GetName().GetNew().Value().c_str());
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

    if (schemaChange.GetVersionRead().IsValid())
        {
        if (schemaChange.GetVersionRead().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionRead().GetValue(ValueId::New).Value())
            {
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionRead' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        m_majorChangesAllowedForSchemas.insert(oldSchema.GetId());
        updateBuilder.AddSetExp("VersionDigit1", schemaChange.GetVersionRead().GetNew().Value());
        }

    if (schemaChange.GetVersionWrite().IsValid())
        {
        if (schemaChange.GetVersionWrite().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionWrite().GetValue(ValueId::New).Value())
            {
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionWrite' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit2", schemaChange.GetVersionWrite().GetNew().Value());
        }

    if (schemaChange.GetVersionMinor().IsValid())
        {
        if (schemaChange.GetVersionMinor().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionMinor().GetValue(ValueId::New).Value())
            {
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Decreasing 'VersionMinor' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit3", schemaChange.GetVersionMinor().GetNew().Value());
        }

    if (schemaChange.GetAlias().IsValid())
        {
        if (schemaChange.GetAlias().GetNew().IsNull())
            {
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Alias must always be set.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        
        if (SchemaPersistenceHelper::ContainsSchemaWithAlias(m_ecdb, schemaChange.GetAlias().GetNew().Value().c_str()))
            {
            Issues().Report("ECSchema Upgrade failed. ECSchema %s: Alias is already used by another existing ECSchema.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("Alias", schemaChange.GetAlias().GetNew().Value().c_str());
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

    if (UpdateClasses(schemaChange.Classes(), oldSchema, newSchema) == ERROR)
        return ERROR;

    return UpdateCustomAttributes(SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema, schemaId, schemaChange.CustomAttributes(), oldSchema, newSchema);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
