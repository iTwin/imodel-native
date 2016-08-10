/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::Import(ECSchemaCompareContext& ctx, ECN::ECSchemaCR ecSchema)
    {
    BeMutexHolder lock(m_mutex);
    m_majorChangesAllowedForSchemas.clear();
    if (ECSchemaChange* schemaChange = ctx.GetChanges().Find(ecSchema.GetName().c_str()))
        {
        if (schemaChange->GetState() == ChangeState::Modified)
            {
            if (schemaChange->GetStatus() == ECChange::Status::Done)
                return SUCCESS;

            ECSchemaCP existingSchema = ctx.FindExistingSchema(schemaChange->GetId());
            BeAssert(existingSchema != nullptr);
            if (existingSchema == nullptr)
                return ERROR;

            return UpdateECSchema(*schemaChange, *existingSchema, ecSchema);
            }
        else if (schemaChange->GetState() == ChangeState::Deleted)
            {
            if (schemaChange->GetStatus() == ECChange::Status::Done)
                return SUCCESS;

            schemaChange->SetStatus(ECChange::Status::Done);
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting an ECSchema is not supported.",
                            ecSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        else
            {
            if (schemaChange->GetStatus() == ECChange::Status::Done)
                return SUCCESS;
            }
        }

    if (ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, ecSchema.GetName().c_str()).IsValid())
        return SUCCESS;

    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECSchemaIdSequence().GetNextValue(nextId))
        {
        BeAssert(false && "Could not generate new ECSchemaId");
        return ERROR;
        }

    const ECSchemaId ecSchemaId(nextId.GetValue());
    const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);

    if (SUCCESS != InsertECSchemaEntry(ecSchema))
        {
        DbResult lastErrorCode;
        m_ecdb.GetLastError(&lastErrorCode);
        if (BE_SQLITE_CONSTRAINT_UNIQUE == lastErrorCode)
            Issues().Report(ECDbIssueSeverity::Error, "Failed to import ECSchema '%s'. Alias '%s' is already used by an existing ECSchema.",
                            ecSchema.GetFullSchemaName().c_str(), ecSchema.GetAlias().c_str());
        return ERROR;
        }

    if (SUCCESS != InsertECSchemaReferenceEntries(ecSchema))
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to import ECSchema references for ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    //enums must be imported before ECClasses as properties reference enums
    for (ECEnumerationCP ecEnum : ecSchema.GetEnumerations())
        {
        if (SUCCESS != ImportECEnumeration(*ecEnum))
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to import ECEnumeration '%s'.", ecEnum->GetFullName().c_str());
            return ERROR;
            }
        }

    //KOQs must be imported before ECClasses as properties reference KOQs
    for (KindOfQuantityCP koq : ecSchema.GetKindOfQuantities())
        {
        if (SUCCESS != ImportKindOfQuantity(*koq))
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to import KindOfQuantity '%s'.", koq->GetFullName().c_str());
            return ERROR;
            }
        }

    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        if (SUCCESS != ImportECClass(*ecClass))
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to import ECClass '%s'.", ecClass->GetFullName());
            return ERROR;
            }
        }

    if (SUCCESS != ImportCustomAttributes(ecSchema, ECContainerId(ecSchemaId), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema))
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to import custom attributes of ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::InsertECSchemaReferenceEntries(ECSchemaCR schema)
    {
    ECSchemaReferenceListCR references = schema.GetReferencedSchemas();
    if (references.empty())
        return SUCCESS;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "INSERT INTO ec_SchemaReference(SchemaId,ReferencedSchemaId) VALUES(?,?)"))
        return ERROR;

    for (bpair<SchemaKey, ECSchemaPtr> const& kvPair : references)
        {
        ECSchemaCP reference = kvPair.second.get();
        ECSchemaId referenceId = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, reference->GetName().c_str());
        if (!referenceId.IsValid())
            {
            BeAssert(false && "BuildDependencyOrderedSchemaList used by caller should have ensured that all references are already imported");
            return ERROR;
            }

        if (!reference->HasId())
            {
            // We apparently have a second copy of an ECSchema that has already been imported. Ideally, we would have used the ECDbSchemaManager
            // as an IECSchemaLocater when we loaded the ECSchemas that we are imported, but since we did not, we simply *hope* that 
            // the duplicated loaded from disk matches the one stored in the db.
            // The duplicate copy does not have its Ids set... and so we will have to look them up, here and elsewhere, and set them into 
            // the in-memory duplicate copy. In Graphite02, we might risk cleaning this up to force use of the already-persisted ECSchema
            // or else to do a one-shot validation of the ECSchema and updating of its ids. 
            // Grep for GetClassIdForECClassFromDuplicateECSchema and GetPropertyIdForECPropertyFromDuplicateECSchema for other ramifications of this.
            const_cast<ECSchemaP>(reference)->SetId(referenceId);
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
BentleyStatus ECDbSchemaWriter::ImportECClass(ECN::ECClassCR ecClass)
    {
    if (ECDbSchemaPersistenceHelper::GetECClassId(m_ecdb, ecClass).IsValid())
        return SUCCESS;

    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECClassIdSequence().GetNextValue(nextId))
        return ERROR;

    ECClassId ecClassId(nextId.GetValue());
    const_cast<ECClassR>(ecClass).SetId(ecClassId);

    BeAssert(ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, ecClass.GetSchema()).IsValid());

    //now import actual ECClass
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Class(Id,SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection,CustomAttributeContainerType) "
                                                  "VALUES(?,?,?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecClass.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecClass.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;

#ifdef ECECSCHEMAUPDATE_INVARIANT
    if (ecClass.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecClass.GetInvariantDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
    if (!ecClass.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecClass.GetInvariantDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
#else
    if (!ecClass.GetDisplayLabel().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecClass.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
    if (!ecClass.GetDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecClass.GetDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
#endif

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
        if (SUCCESS != ImportECClass(*baseClass))
            return ERROR;

        if (SUCCESS != InsertBaseClassEntry(ecClassId, *baseClass, baseClassIndex++))
            return ERROR;
        }

    int propertyIndex = 0;
    for (ECPropertyCP ecProperty : ecClass.GetProperties(false))
        {
        if (SUCCESS != ImportECProperty(*ecProperty, propertyIndex++))
            {
            LOG.errorv("Failed to import ECProperty '%s' of ECClass '%s'.", ecProperty->GetName().c_str(), ecClass.GetFullName());
            return ERROR;
            }
        }

    ECN::ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr)
        {
        if (SUCCESS != ImportECRelationshipClass(relationship))
            return ERROR;
        }

    return ImportCustomAttributes(ecClass, ECContainerId(ecClassId), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::ImportECEnumeration(ECEnumerationCR ecEnum)
    {
    if (ECDbSchemaPersistenceHelper::GetECEnumerationId(m_ecdb, ecEnum).IsValid())
        return SUCCESS;

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_Enumeration(Id, SchemaId, Name, DisplayLabel, Description, UnderlyingPrimitiveType, IsStrict, EnumValues) VALUES(?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    BeBriefcaseBasedId nextId;
    if (m_ecdb.GetECDbImplR().GetECEnumIdSequence().GetNextValue(nextId))
        return ERROR;

    ECEnumerationId enumId(nextId.GetValue());
    const_cast<ECEnumerationR>(ecEnum).SetId(enumId);

    BeAssert(ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, ecEnum.GetSchema()).IsValid());

    if (BE_SQLITE_OK != stmt->BindId(1, enumId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecEnum.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecEnum.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;

#ifdef ECECSCHEMAUPDATE_INVARIANT
    if (ecEnum.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecEnum.GetInvariantDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
    if (!ecEnum.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecEnum.GetInvariantDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
#else
    if (!ecEnum.GetDisplayLabel().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecEnum.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
    if (!ecEnum.GetDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecEnum.GetDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
#endif


    if (BE_SQLITE_OK != stmt->BindInt(6, (int) ecEnum.GetType()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, ecEnum.GetIsStrict() ? 1 : 0))
        return ERROR;

    Utf8String enumValueJson;
    if (SUCCESS != ECDbSchemaPersistenceHelper::SerializeECEnumerationValues(enumValueJson, ecEnum))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(8, enumValueJson.c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::ImportKindOfQuantity(KindOfQuantityCR koq)
    {
    if (ECDbSchemaPersistenceHelper::GetKindOfQuantityId(m_ecdb, koq).IsValid())
        return SUCCESS;

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_KindOfQuantity(Id,SchemaId,Name,DisplayLabel,Description,PersistenceUnit,PersistencePrecision,DefaultPresentationUnit,AlternativePresentationUnits) VALUES(?,?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    BeBriefcaseBasedId nextId;
    if (m_ecdb.GetECDbImplR().GetKindOfQuantityIdSequence().GetNextValue(nextId))
        return ERROR;

    KindOfQuantityId koqId(nextId.GetValue());
    const_cast<KindOfQuantityR>(koq).SetId(koqId);

    BeAssert(ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, koq.GetSchema()).IsValid());

    if (BE_SQLITE_OK != stmt->BindId(1, koqId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, koq.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, koq.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;
   
    if (!koq.GetDisplayLabel().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, koq.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
   
    if (!koq.GetDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, koq.GetDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(6, koq.GetPersistenceUnit().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, koq.GetPrecision()))
        return ERROR;

    if (!koq.GetDefaultPresentationUnit().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(8, koq.GetDefaultPresentationUnit().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    Utf8String altPresUnitsJsonStr;
    if (!koq.GetAlternativePresentationUnitList().empty())
        {
        if (SUCCESS != ECDbSchemaPersistenceHelper::SerializeKoqAlternativePresentationUnits(altPresUnitsJsonStr, koq))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindText(9, altPresUnitsJsonStr.c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECRelationshipClass(ECN::ECRelationshipClassCP relationship)
    {
    const ECClassId relClassId = relationship->GetId();
    if (SUCCESS != ImportECRelationshipConstraint(relClassId, relationship->GetSource(), ECRelationshipEnd_Source))
        return ERROR;

    return ImportECRelationshipConstraint(relClassId, relationship->GetTarget(), ECRelationshipEnd_Target);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECRelationshipConstraint(ECClassId const& relClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd end)
    {
    BeAssert(relClassId.IsValid());

    ECRelationshipConstraintId constraintId;;
    if (SUCCESS != InsertECRelationshipConstraintEntry(constraintId, relClassId, relationshipConstraint, end))
        return ERROR;

    BeAssert(constraintId.IsValid());

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_RelationshipConstraintClass(ConstraintId,ClassId) VALUES(?,?)");
    if (stmt == nullptr)
        return ERROR;

    for (ECRelationshipConstraintClassCP constraintClassObj : relationshipConstraint.GetConstraintClasses())
        {
        ECClassCR constraintClass = constraintClassObj->GetClass();
        if (SUCCESS != ImportECClass(constraintClass))
            return ERROR;

        BeAssert(constraintClass.GetId().IsValid());

        if (BE_SQLITE_OK != stmt->BindId(1, constraintId))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(2, constraintClass.GetId()))
            return ERROR;

        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;

        stmt->Reset();
        stmt->ClearBindings();
        }

    stmt = nullptr;

    ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType = end == ECRelationshipEnd_Source ? ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint : ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint;
    return ImportCustomAttributes(relationshipConstraint, ECContainerId(constraintId), containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECProperty(ECN::ECPropertyCR ecProperty, int32_t ordinal)
    {
    //Local properties are expected to not be imported at this point as they get imported along with their class.
    BeAssert(!ECDbSchemaPersistenceHelper::GetECPropertyId(m_ecdb, ecProperty).IsValid());

    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECPropertyIdSequence().GetNextValue(nextId))
        return ERROR;

    ECPropertyId ecPropertyId(nextId.GetValue());
    const_cast<ECPropertyR>(ecProperty).SetId(ecPropertyId);

    if (ecProperty.GetIsStruct())
        {
        if (SUCCESS != ImportECClass(ecProperty.GetAsStructProperty()->GetType()))
            return ERROR;
        }
    else if (ecProperty.GetIsArray())
        {
        StructArrayECPropertyCP structArrayProperty = ecProperty.GetAsStructArrayProperty();
        if (nullptr != structArrayProperty)
            {
            if (SUCCESS != ImportECClass(*structArrayProperty->GetStructElementType()))
                return ERROR;
            }
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (SUCCESS != ImportECClass(*ecProperty.GetAsNavigationProperty()->GetRelationshipClass()))
            return ERROR;
        }

    //now insert the actual property
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_Property(Id,ClassId,Name,DisplayLabel,Description,IsReadonly,Ordinal,Kind,PrimitiveType,EnumerationId,StructClassId,ExtendedTypeName,KindOfQuantityId,ArrayMinOccurs,ArrayMaxOccurs,NavigationRelationshipClassId,NavigationDirection) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecProperty.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(2, ecProperty.GetClass().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecProperty.GetName(), Statement::MakeCopy::No))
        return ERROR;

#ifdef ECECSCHEMAUPDATE_INVARIANT
    if (ecProperty.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecProperty.GetInvariantDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
    if (!ecProperty.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecProperty.GetInvariantDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
#else
    if (!ecProperty.GetDisplayLabel().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecProperty.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
    if (!ecProperty.GetDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, ecProperty.GetDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
#endif

    if (BE_SQLITE_OK != stmt->BindInt(6, ecProperty.GetIsReadOnly() ? 1 : 0))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, ordinal))
        return ERROR;

    const int kindIndex = 8;
    const int primitiveTypeIndex = 9;
    const int enumIdIndex = 10;
    const int structClassIdIndex = 11;
    const int extendedTypeIndex = 12;
    const int koqIdIndex = 13;
    const int arrayMinIndex = 14;
    const int arrayMaxIndex = 15;
    const int navRelClassIdIndex = 16;
    const int navDirIndex = 17;

    if (ecProperty.GetIsPrimitive())
        {
        PrimitiveECPropertyCP primProp = ecProperty.GetAsPrimitiveProperty();
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Primitive)))
            return ERROR;

        ECEnumerationCP ecenum = primProp->GetEnumeration();
        if (ecenum == nullptr)
            {
            if (BE_SQLITE_OK != stmt->BindInt(primitiveTypeIndex, (int) primProp->GetType()))
                return ERROR;
            }
        else
            {
            if (SUCCESS != ImportECEnumeration(*ecenum))
                return ERROR;

            BeAssert(ecenum->HasId());
            if (BE_SQLITE_OK != stmt->BindId(enumIdIndex, ecenum->GetId()))
                return ERROR;
            }

        if (SUCCESS != BindPropertyExtendedTypeName(*stmt, extendedTypeIndex, ecProperty))
            return ERROR;

        if (SUCCESS != BindPropertyKindOfQuantityId(*stmt, koqIdIndex, ecProperty))
            return ERROR;
        }
    else if (ecProperty.GetIsStruct())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Struct)))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindId(structClassIdIndex, ecProperty.GetAsStructProperty()->GetType().GetId()))
            return ERROR;
        }
    else if (ecProperty.GetIsArray())
        {
        ArrayECPropertyCP arrayProp = ecProperty.GetAsArrayProperty();
        if (arrayProp->GetKind() == ARRAYKIND_Primitive)
            {
            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::PrimitiveArray)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindInt(primitiveTypeIndex, (int) arrayProp->GetPrimitiveElementType()))
                return ERROR;
            }
        else
            {
            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::StructArray)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindId(structClassIdIndex, arrayProp->GetAsStructArrayProperty()->GetStructElementType()->GetId()))
                return ERROR;
            }

        if (SUCCESS != BindPropertyExtendedTypeName(*stmt, extendedTypeIndex, ecProperty))
            return ERROR;

        if (SUCCESS != BindPropertyKindOfQuantityId(*stmt, koqIdIndex, ecProperty))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(arrayMinIndex, (int) arrayProp->GetMinOccurs()))
            return ERROR;

        //until the max occurs bug in ECObjects (where GetMaxOccurs always returns "unbounded")
        //has been fixed, we need to call GetStoredMaxOccurs to retrieve the proper max occurs
        if (BE_SQLITE_OK != stmt->BindInt(arrayMaxIndex, (int) arrayProp->GetStoredMaxOccurs()))
            return ERROR;
        }
    else if (ecProperty.GetIsNavigation())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Navigation)))
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

    return ImportCustomAttributes(ecProperty, ECContainerId(ecPropertyId), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId const& sourceContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, Utf8CP onlyImportCAWithClassName)
    {
    //import CA classes first
    for (IECInstancePtr ca : sourceContainer.GetCustomAttributes(false))
        {
        if (SUCCESS != ImportECClass(ca->GetClass()))
            return ERROR;
        }

    bmap<ECClassCP, bvector<IECInstanceP> > customAttributeMap;
    for (auto& customAttribute : sourceContainer.GetCustomAttributes(false))
        {
        if (onlyImportCAWithClassName == nullptr)
            customAttributeMap[&(customAttribute->GetClass())].push_back(customAttribute.get());
        else
            {
            if (customAttribute->GetClass().GetName().Equals(onlyImportCAWithClassName))
                customAttributeMap[&(customAttribute->GetClass())].push_back(customAttribute.get());
            }
        }
    int index = 0; // Its useless if we enumerate map since it doesn't ensure order in which we added it

    bmap<ECClassCP, bvector<IECInstanceP> >::const_iterator itor = customAttributeMap.begin();

    //Here we consider consolidated attribute a primary. This is lossy operation some overridden primary custom attributes would be lost
    for (; itor != customAttributeMap.end(); ++itor)
        {
        bvector<IECInstanceP> const& customAttributes = itor->second;
        IECInstanceP customAttribute = customAttributes.size() == 1 ? customAttributes[0] : customAttributes[1];
        ECClassCP ecClass = itor->first;
        ECClassId customAttributeClassId = ECDbSchemaPersistenceHelper::GetECClassId(m_ecdb, *ecClass);
        BeAssert(customAttributeClassId.IsValid());
        if (SUCCESS != InsertCAEntry(customAttribute, customAttributeClassId, sourceContainerId, containerType, index++))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::InsertECRelationshipConstraintEntry(ECRelationshipConstraintId& constraintId, ECClassId const& relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraint (RelationshipClassId,RelationshipEnd,MultiplicityLowerLimit,MultiplicityUpperLimit,RoleLabel,IsPolymorphic) VALUES (?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, endpoint))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, relationshipConstraint.GetMultiplicity().GetLowerLimit()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(4, relationshipConstraint.GetMultiplicity().GetUpperLimit()))
        return ERROR;

    if (relationshipConstraint.IsRoleLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, relationshipConstraint.GetRoleLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(6, relationshipConstraint.GetIsPolymorphic() ? 1 : 0))
        return ERROR;

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;


    constraintId = ECRelationshipConstraintId((uint64_t) m_ecdb.GetLastInsertRowId());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::InsertECSchemaEntry(ECSchemaCR ecSchema)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Schema(Id,Name,DisplayLabel,Description,NamespacePrefix,VersionDigit1,VersionDigit2,VersionDigit3) VALUES(?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, ecSchema.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, ecSchema.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;


#ifdef ECECSCHEMAUPDATE_INVARIANT
    if (ecSchema.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ecSchema.GetInvariantDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
    if (!ecSchema.GetInvariantDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecSchema.GetInvariantDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
#else
    if (!ecSchema.GetDisplayLabel().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ecSchema.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
    if (!ecSchema.GetDescription().empty())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecSchema.GetDescription().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }
#endif


    if (BE_SQLITE_OK != stmt->BindText(5, ecSchema.GetAlias().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(6, ecSchema.GetVersionMajor()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(7, ecSchema.GetVersionWrite()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(8, ecSchema.GetVersionMinor()))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::InsertBaseClassEntry(ECClassId const& ecClassId, ECClassCR baseClass, int ordinal)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_ClassHasBaseClasses(ClassId,BaseClassId,Ordinal) VALUES(?,?,?)"))
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
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::BindPropertyExtendedTypeName(Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    Utf8StringCP extendedTypeName = nullptr;
    if (prop.HasExtendedType())
        {
        if (prop.GetIsPrimitive())
            extendedTypeName = &prop.GetAsPrimitiveProperty()->GetExtendedTypeName();
        else if (prop.GetIsPrimitiveArray())
            extendedTypeName = &prop.GetAsArrayProperty()->GetExtendedTypeName();
        else
            BeAssert(false && "Only prim and prim arrays are expected to have extended type names");
        }

    if (extendedTypeName == nullptr || extendedTypeName->empty())
        return SUCCESS;

    return stmt.BindText(paramIndex, extendedTypeName->c_str(), Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::BindPropertyKindOfQuantityId(Statement& stmt, int paramIndex, ECPropertyCR prop)
    {
    KindOfQuantityCP koq = nullptr;
    if (prop.GetIsPrimitive())
        koq = prop.GetAsPrimitiveProperty()->GetKindOfQuantity();
    else if (prop.GetIsPrimitiveArray())
        koq = prop.GetAsArrayProperty()->GetKindOfQuantity();

    if (koq == nullptr)
        return SUCCESS;

    if (SUCCESS != ImportKindOfQuantity(*koq))
        return ERROR;

    BeAssert(koq->HasId());
    return stmt.BindId(paramIndex, koq->GetId()) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::InsertCAEntry(IECInstanceP customAttribute, ECClassId const& ecClassId, ECContainerId const& containerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, int ordinal)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_CustomAttribute(ContainerId,ContainerType,ClassId,Ordinal,Instance) VALUES(?,?,?,?,?)"))
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
    if (InstanceWriteStatus::Success != customAttribute->WriteToXmlString(caXml, false, //don't write XML description header as we only store an XML fragment
                                                                          true)) //store instance id for the rare cases where the client specified one.
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(5, caXml.c_str(), Statement::MakeCopy::No))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::DeleteCAEntry(ECClassId const& ecClassId, ECContainerId const& containerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "DELETE FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ClassId = ?"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, containerId.GetValue()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(3, ecClassId.GetValue()))
        return ERROR;

    if (stmt->Step() != BE_SQLITE_DONE)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::ReplaceCAEntry(IECInstanceP customAttribute, ECClassId const& ecClassId, ECContainerId const& containerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, int ordinal)
    {
    if (DeleteCAEntry(ecClassId, containerId, containerType) != SUCCESS)
        return ERROR;

    return InsertCAEntry(customAttribute, ecClassId, containerId, containerType, ordinal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbSchemaWriter::IsPropertyTypeChangeSupported(Utf8StringR error, StringChange& typeChange, ECPropertyCR oldProperty, ECPropertyCR newProperty) const
    {
    //changing from primitive to enum and enum to primitve is supported with same type and enum is unstrict
    if (oldProperty.GetIsPrimitive() && newProperty.GetIsPrimitive())
        {
        PrimitiveECPropertyCP a = oldProperty.GetAsPrimitiveProperty();
        PrimitiveECPropertyCP b = newProperty.GetAsPrimitiveProperty();
        ECEnumerationCP aEnum = a->GetEnumeration();
        ECEnumerationCP bEnum = b->GetEnumeration();
        if (!aEnum && !bEnum)
            {
            error.Sprintf("ECSchema Update failed. ECProperty %s.%s: Changing the type of a Primitive ECProperty is not supported. Cannot convert from '%s' to '%s'",
                          oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str(), typeChange.GetOld().Value().c_str(), typeChange.GetNew().Value().c_str());
            return false;
            }

        if (aEnum && !bEnum)
            {
            if (aEnum->GetType() != b->GetType())
                {
                error.Sprintf("ECSchema Update failed. ECProperty %s.%s: ECEnumeration specified for property must have same primitive type as new primitive property type",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

                return false;
                }

            return true;
            }
        if (!aEnum && bEnum)
            {
            if (a->GetType() != bEnum->GetType())
                {
                error.Sprintf("ECSchema Update failed. ECProperty %s.%s: Primitive type change to ECEnumeration which as different type then existing primitive property",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

                return false;
                }

            if (bEnum->GetIsStrict())
                {
                error.Sprintf("ECSchema Update failed. ECProperty %s.%s: Type change to a Strict ECEnumeration is not supported.",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

                return false;
                }

            return true;
            }

        if (aEnum && bEnum)
            {
            if (aEnum->GetType() != bEnum->GetType())
                {
                error.Sprintf("ECSchema Update failed. ECProperty %s.%s: Exisitng ECEnumeration has different primitive type from the new ECEnumeration specified",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());

                return false;
                }
            if (bEnum->GetIsStrict())
                {
                error.Sprintf("ECSchema Update failed. ECProperty %s.%s: Type change to a Strict ECEnumeration is not supported.",
                              oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
                }
            return true;
            }
        }

    error.Sprintf("ECSchema Update failed. ECProperty %s.%s: Changing the type of an ECProperty is not supported. Cannot convert from '%s' to '%s'",
                  oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str(), typeChange.GetOld().Value().c_str(), typeChange.GetNew().Value().c_str());

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECProperty(ECPropertyChange& propertyChange, ECPropertyCR oldProperty, ECPropertyCR newProperty)
    {
    if (propertyChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    ECPropertyId propertyId = ECDbSchemaPersistenceHelper::GetECPropertyId(m_ecdb, newProperty);
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
            Issues().Report(ECDbIssueSeverity::Error, error.c_str());
            return ERROR;
            }
        }

    if (propertyChange.IsStruct().IsValid() || propertyChange.IsStructArray().IsValid() || propertyChange.IsPrimitive().IsValid() ||
        propertyChange.IsPrimitiveArray().IsValid() || propertyChange.IsNavigation().IsValid())
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECProperty %s.%s: Changing the kind of the ECProperty is not supported.",
                                  oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
        return ERROR;
        }

    if (propertyChange.GetArray().IsValid())
        {
        ArrayChange& arrayChange = propertyChange.GetArray();
        if (arrayChange.MaxOccurs().IsValid() || arrayChange.MinOccurs().IsValid())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECProperty %s.%s: Changing the 'MinOccurs' or 'MaxOccurs' for an Array ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }
        }

    if (propertyChange.GetNavigation().IsValid())
        {
        NavigationChange& navigationChange = propertyChange.GetNavigation();
        if (navigationChange.GetRelationshipClassName().IsValid())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECProperty %s.%s: Changing the 'RelationshipClassName' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }

        if (navigationChange.Direction().IsValid())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECProperty %s.%s: Changing the 'Direction' for a Navigation ECProperty is not supported.",
                                      oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
            return ERROR;
            }
        }

    SqlUpdateBuilder sqlUpdateBuilder("ec_Property");

    if (propertyChange.GetName().IsValid())
        {
        if (propertyChange.GetName().GetNew().IsNull())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECProperty %s.%s: 'Name' must always be set for an ECProperty is not supported.",
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
            BeAssert(newPrimitiveProperty != nullptr);
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
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECProperty %s.%s: Only Primitive property can be coverted to ECEnumeration",
                                oldProperty.GetClass().GetFullName(), oldProperty.GetName().c_str());
                return ERROR;
                }

            ECEnumerationCP enumCP = newPrimitiveProperty->GetEnumeration();
            ECEnumerationId id = ECDbSchemaPersistenceHelper::GetECEnumerationId(m_ecdb, *enumCP);
            if (!id.IsValid())
                return ERROR;

            sqlUpdateBuilder.AddSetToNull("PrimitiveType"); //SET TO NULL
            sqlUpdateBuilder.AddSetExp("EnumerationId", id.GetValue());
            }
        }

    if (propertyChange.GetKindOfQuanity().IsValid())
        {
        if (propertyChange.GetKindOfQuanity().GetState() == ChangeState::Deleted)
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
            else if (auto newPrimitivePropertyArray = newProperty.GetAsArrayProperty())
                {
                koqCP = newPrimitivePropertyArray->GetKindOfQuantity();
                }

            if (koqCP == nullptr)
                {
                BeAssert(koqCP != nullptr);
                return ERROR;
                }

            KindOfQuantityId id = ECDbSchemaPersistenceHelper::GetKindOfQuantityId(m_ecdb, *koqCP);
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

    return UpdateECCustomAttributes(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property, propertyId, propertyChange.CustomAttributes(), oldProperty, newProperty);;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECRelationshipConstraint(ECContainerId const& containerId, ECRelationshipConstraintChange& constraintChange, ECRelationshipConstraintCR oldConstraint, ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName)
    {
    Utf8CP constraintEndStr = isSource ? "Source" : "Target";
    SqlUpdateBuilder updater("ec_RelationshipConstraint");
 
    if (constraintChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    if (constraintChange.GetMultiplicity().IsValid())
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECRelationshipClass %s - Constraint: %s: Changing 'Multiplicity' of an ECRelationshipConstraint is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.IsPolymorphic().IsValid())
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECRelationshipClass %s - Constraint: %s: Changing flag 'IsPolymorphic' of an ECRelationshipConstraint is not supported.",
                                  relationshipName, constraintEndStr);
        return ERROR;
        }

    if (constraintChange.ConstraintClasses().IsValid())
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECRelationshipClass %s - Constraint: %s: Changing the constraint classes is not supported.",
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

    const ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType = isSource ? ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint : ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint;
    return UpdateECCustomAttributes(containerType, containerId, constraintChange.CustomAttributes(), oldConstraint, newConstraint);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::TryParseId(Utf8StringR schemaName, Utf8StringR className, Utf8StringCR id) const
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
BentleyStatus ECDbSchemaWriter::UpdateECCustomAttributes(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType containerType, ECContainerId const& containerId, ECInstanceChanges& instanceChanges, IECCustomAttributeContainerCR oldContainer, IECCustomAttributeContainerCR newContainer)
    {
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
            if (m_customAttributeValidator.HasAnyRuleForSchema(schemaName.c_str()))
                {
                if (m_customAttributeValidator.Validate(change) == CustomAttributeValidator::Policy::Reject)
                    {
                    Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. Adding or modifying %s CustomAttributes is not supported.",
                                              schemaName.c_str());
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

            if (ImportECClass(ca->GetClass()) != SUCCESS)
                return ERROR;

            if (InsertCAEntry(ca.get(), ca->GetClass().GetId(), containerId, containerType, 0) != SUCCESS)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Deleted)
            {
            IECInstancePtr ca = oldContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                return ERROR;

            ECClassId caClassId = ECDbSchemaPersistenceHelper::GetECClassId(m_ecdb, ca->GetClass());
            BeAssert(caClassId.IsValid());

            if (DeleteCAEntry(ca->GetClass().GetId(), containerId, containerType) != SUCCESS)
                return ERROR;
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            IECInstancePtr ca = newContainer.GetCustomAttribute(schemaName, className);
            BeAssert(ca.IsValid());
            if (ca.IsNull())
                return ERROR;

            if (ImportECClass(ca->GetClass()) != SUCCESS)
                return ERROR;

            if (ReplaceCAEntry(ca.get(), ca->GetClass().GetId(), containerId, containerType, 0) != SUCCESS)
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
BentleyStatus ECDbSchemaWriter::UpdateECClass(ECClassChange& classChange, ECClassCR oldClass, ECClassCR newClass)
    {
    if (classChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;

    ECClassId classId = ECDbSchemaPersistenceHelper::GetECClassId(m_ecdb, newClass);
    if (!classId.IsValid())
        {
        BeAssert(false && "Failed to resolve ecclass id");
        return ERROR;
        }

    SqlUpdateBuilder updateBuilder("ec_Class");

    if (classChange.GetClassModifier().IsValid())
        {
        ECClassModifier newValue = classChange.GetClassModifier().GetNew().Value();
        if (newValue == ECClassModifier::Sealed)
            {
            if (!newClass.GetDerivedClasses().empty())
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Changing the 'Modifier' of ECClass to ECClassModifier::Sealed only acceptable if class has no derived classes",
                                          oldClass.GetFullName());

                return ERROR;
                }
            }
        else if (newValue == ECClassModifier::Abstract)
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Changing the 'Modifier' of ECClass to ECClassModifier::Abstract is not supported",
                                      oldClass.GetFullName());

            return ERROR;
            }

        updateBuilder.AddSetExp("Modifier", Enum::ToInt(classChange.GetClassModifier().GetNew().Value()));
        }

    if (classChange.ClassType().IsValid())
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Changing the ECClassType of an ECClass is not supported.",
                                  oldClass.GetFullName());
        return ERROR;
        }

    if (classChange.GetName().IsValid())
        {
        if (classChange.GetName().GetNew().IsNull())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Name must always be set for an ECClass.",
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
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECRelationshipClass %s: Changing the 'Strength' of an ECRelationshipClass is not supported.",
                                      oldClass.GetFullName());
            return ERROR;
            }

        if (relationshipChange.GetStrengthDirection().IsValid())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECRelationshipClass %s: Changing the 'StrengthDirection' of an ECRelationshipClass is not supported.",
                                      oldClass.GetFullName());
            return ERROR;
            }

        ECRelationshipClassCP oldRel = oldClass.GetRelationshipClassCP();
        ECRelationshipClassCP newRel = newClass.GetRelationshipClassCP();
        BeAssert(oldRel != nullptr && newRel != nullptr);
        if (oldRel == nullptr && newRel == nullptr)
            return ERROR;

        if (relationshipChange.GetSource().IsValid())
            if (UpdateECRelationshipConstraint(classId, relationshipChange.GetSource(), newRel->GetSource(), oldRel->GetSource(), true, oldRel->GetFullName()) == ERROR)
                return ERROR;

        if (relationshipChange.GetTarget().IsValid())
            if (UpdateECRelationshipConstraint(classId,  relationshipChange.GetTarget(), newRel->GetSource(), oldRel->GetTarget(), false, oldRel->GetFullName()) == ERROR)
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
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Removing a base class from an ECClass is not supported.",
                                          oldClass.GetFullName());
                return ERROR;
                }
            else if (change.GetState() == ChangeState::New)
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Adding a new base class to an ECClass is not supported.",
                                          oldClass.GetFullName());
                return ERROR;
                }
            else if (change.GetState() == ChangeState::Modified)
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Modifying the position of a base class in the list of base classes of an ECClass is not supported.",
                                          oldClass.GetFullName());
                return ERROR;
                }
            }
        }

    if (classChange.Properties().IsValid())
        {
        if (UpdateECProperties(classChange.Properties(), oldClass, newClass) != SUCCESS)
            return ERROR;
        }

    return UpdateECCustomAttributes(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class, classId, classChange.CustomAttributes(), oldClass, newClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  05/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECProperties(ECPropertyChanges& propertyChanges, ECClassCR oldClass, ECClassCR newClass)
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

            return DeleteECProperty(change, *oldProperty);
            }
        else if (change.GetState() == ChangeState::New)
            {
            ECPropertyCP newProperty = newClass.GetPropertyP(change.GetName().GetNew().Value().c_str(), false);
            if (newProperty == nullptr)
                {
                BeAssert(false && "Failed to find the class");
                return ERROR;
                }

            if (SUCCESS != ImportECProperty(*newProperty, ordinal))
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

            if (UpdateECProperty(change, *oldProperty, *newProperty) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECSchemaReferences(ReferenceChanges& referenceChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
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
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Failed to parse previous ECSchema reference name.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, oldRef.GetName().c_str());
            Statement stmt;
            if (stmt.Prepare(m_ecdb, "DELETE FROM ec_SchemaReference WHERE SchemaId=? AND ReferencedSchemaId=?") != BE_SQLITE_OK)
                return ERROR;

            stmt.BindId(1, oldSchema.GetId());
            stmt.BindId(2, referenceSchemaId);

            if (stmt.Step() != BE_SQLITE_DONE)
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Failed to remove ECSchema reference %s.",
                                          oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::New)
            {
            SchemaKey newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Failed to parse new ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Ensure schema exist
            if (!ECDbSchemaPersistenceHelper::TryGetECSchemaKey(existingRef, m_ecdb, newRef.GetName().c_str()))
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Schema must exist with that or greater version
            if (!existingRef.Matches(newRef, SchemaMatchType::LatestCompatible))
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Could not locate compatible referenced ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            ECSchemaId referenceSchemaId = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, newRef.GetName().c_str());
            Statement stmt;
            if (stmt.Prepare(m_ecdb, "INSERT INTO ec_SchemaReference(SchemaId, ReferencedSchemaId) VALUES (?,?)") != BE_SQLITE_OK)
                return ERROR;

            stmt.BindId(1, oldSchema.GetId());
            stmt.BindId(2, referenceSchemaId);

            if (stmt.Step() != BE_SQLITE_DONE)
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Failed to add new reference to ECSchema %s.",
                                          oldSchema.GetFullSchemaName().c_str(), newRef.GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            SchemaKey oldRef, newRef, existingRef;
            if (SchemaKey::ParseSchemaFullName(oldRef, change.GetOld().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Failed to parse previous ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            if (SchemaKey::ParseSchemaFullName(newRef, change.GetNew().Value().c_str()) != ECObjectsStatus::Success)
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Failed to parse new ECSchema reference.",
                                          oldSchema.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Ensure schema exist and also get updated version number.
            if (!ECDbSchemaPersistenceHelper::TryGetECSchemaKey(existingRef, m_ecdb, oldRef.GetName().c_str()))
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Referenced ECSchema %s does not exist in the file.",
                                          oldSchema.GetFullSchemaName().c_str(), oldRef.GetFullSchemaName().c_str());
                return ERROR;
                }

            //Schema must exist with that or greater version
            if (!existingRef.Matches(newRef, SchemaMatchType::LatestCompatible))
                {
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Could not locate compatible referenced ECSchema %s.",
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
bool ECDbSchemaWriter::IsSpecifiedInECRelationshipConstraint(ECClassCR deletedClass) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT NULL FROM ec_RelationshipConstraintClass WHERE ClassId = ? LIMIT 1");
    if (stmt == nullptr)
        {
        BeAssert(false && "SQL_SCHEMA_CHANGED");
        return true;
        }

    stmt->BindId(1, deletedClass.GetId());
    return stmt->Step() == BE_SQLITE_ROW;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::DeleteECClass(ECClassChange& classChange, ECClassCR deletedClass)
    {
    if (!IsMajorChangeAllowedForECSchema(deletedClass.GetSchema().GetId()))
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECClass '%s'. This schema include a major change but does not increment the MajorVersion for the schema. Bump up the major version for this schema and try again.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }
    
    if (!m_ecdb.Schemas().GetDerivedECClasses(deletedClass).empty())
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECClass '%s' with derived classes is not supported.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsStructClass())
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECClass '%s' failed. ECStructClass cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (deletedClass.IsCustomAttributeClass())
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECClass '%s' failed. ECCustomAttributeClass cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (IsSpecifiedInECRelationshipConstraint(deletedClass))
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECClass '%s' failed. A class which is specified in a relationship constraint cannot be deleted",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    ClassMapCP deletedClassMap = m_ecdb.GetECDbImplR().GetECDbMap().GetClassMap(deletedClass);
    if (deletedClassMap == nullptr)
        {
        BeAssert(false && "Failed to find classMap");
        return ERROR;
        }

    if (MapStrategyExtendedInfo::IsForeignKeyMapping(deletedClassMap->GetMapStrategy()))
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECClass '%s' failed. Deleting ECRelationshipClass with ForeignKey mapping is not supported.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    //Delete all instances
    bool purgeECInstances = deletedClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy;
    if (purgeECInstances)
        {
        if (DeleteECInstances(deletedClass) != SUCCESS)
            return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_Class WHERE Id = ?");
    stmt->BindId(1, deletedClass.GetId());
    if (stmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false && "Failed to delete the class");
        return ERROR;
        }

    for (ECPropertyCP localProperty : deletedClass.GetProperties(false))
        {
        if (DeleteECCustomAttributes(localProperty->GetId(), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        }

    if (auto relationshipClass = deletedClass.GetRelationshipClassCP())
        {
        if (DeleteECCustomAttributes(relationshipClass->GetId(), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        if (DeleteECCustomAttributes(relationshipClass->GetId(), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint) != SUCCESS)
            {
            BeAssert(false && "Failed to delete property customAttribute ");
            return ERROR;
            }
        }

    return DeleteECCustomAttributes(deletedClass.GetId(), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                         Affan.Khan  05/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::DeleteECInstances(ECClassCR deletedClass)
    {
    ECSqlStatement stmt;
    if (stmt.Prepare(m_ecdb, SqlPrintfString("DELETE FROM %s", deletedClass.GetECSqlName().c_str()).GetUtf8CP()) != ECSqlStatus::Success)
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    if (stmt.Step() != BE_SQLITE_DONE)
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECClass '%s' failed. Failed to delete existing instances for the class.",
                                  deletedClass.GetSchema().GetFullSchemaName().c_str(), deletedClass.GetName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::DeleteECCustomAttributes(ECContainerId const& id, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType type)
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ?");
    stmt->BindId(1, id);
    stmt->BindInt(2, Enum::ToInt(type));
    return stmt->Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::DeleteECProperty(ECPropertyChange& propertyChange, ECPropertyCR deletedProperty)
    {
    if (!IsMajorChangeAllowedForECSchema(deletedProperty.GetClass().GetSchema().GetId()))
        {
        Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECProperty '%s.%s'. This schema include a major change but does not increment the MajorVersion for the schema. Bump up the major version for this schema and try again.",
                                  deletedProperty.GetClass().GetSchema().GetFullSchemaName().c_str(), deletedProperty.GetClass().GetName().c_str(), deletedProperty.GetName().c_str());
        return ERROR;
        }

    ClassMapCP classMap = m_ecdb.GetECDbImplR().GetECDbMap().GetClassMap(deletedProperty.GetClass());
    if (classMap == nullptr)
        {
        BeAssert(false && "Failed to find classMap");
        return ERROR;
        }

    auto setPropertyToNull = [&] ()
        {
        ECSqlStatement setToNullStmt;
        const Utf8CP msg = "ECSchema Update failed. ECClass %s: Deleting an ECProperty '%s' from an ECClass failed due error while setting property to null";
        if (setToNullStmt.Prepare(m_ecdb, SqlPrintfString("UPDATE %s SET [%s] = ?", classMap->GetClass().GetECSqlName().c_str(), deletedProperty.GetName().c_str()).GetUtf8CP()) != ECSqlStatus::Success)
            {
            Issues().Report(ECDbIssueSeverity::Error, msg, deletedProperty.GetClass().GetFullName(), deletedProperty.GetName().c_str());
            return ERROR;
            }

        setToNullStmt.BindNull(1);
        if (setToNullStmt.Step() != BE_SQLITE_DONE)
            {
            Issues().Report(ECDbIssueSeverity::Error, msg, deletedProperty.GetClass().GetFullName(), deletedProperty.GetName().c_str());
            return ERROR;
            }

        return SUCCESS;
        };


    bool sharedColumnFound = false;
    for (Partition const& partition : classMap->GetStorageDescription().GetHorizontalPartitions())
        {
        ClassMapCP partitionRootClassMap = m_ecdb.GetECDbImplR().GetECDbMap().GetClassMap(partition.GetRootClassId());
        if (classMap == nullptr)
            {
            BeAssert(false && "Failed to find classMap");
            return ERROR;
            }

        PropertyMapCP propertyMap = partitionRootClassMap->GetPropertyMap(deletedProperty.GetName().c_str());
        if (propertyMap == nullptr)
            {
            BeAssert(false && "Failed to find propertymap");
            return ERROR;
            }

        if (propertyMap->GetType() == PropertyMap::Type::Navigation && !static_cast<NavigationPropertyMap const&> (*propertyMap).IsSupportedInECSql())
            continue;

        //Reject overriden property
        if (propertyMap->GetProperty().GetBaseProperty() != nullptr)
            {
            //Fail we do not want to delete a sql column right now
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Deleting an ECProperty '%s' from an ECClass is not supported as its overriden",
                                      deletedProperty.GetClass().GetFullName(), deletedProperty.GetName().c_str());
            return ERROR;
            }

        //Delete DbTable entries
        std::vector<DbColumn const*> columns;
        propertyMap->GetColumns(columns);
        for (DbColumn const* column : columns)
            {
            //For shared column do not delete column itself.
            if (column->IsShared())
                {
                if (!sharedColumnFound) sharedColumnFound = true;
                continue;
                }
            //Check for use as key property.
            if (Enum::Contains(column->GetKind(), DbColumn::Kind::SourceECInstanceId) ||
                Enum::Contains(column->GetKind(), DbColumn::Kind::TargetECInstanceId))
                {
                //Fail we do not want to delete a sql column right now
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Deleting an ECProperty '%s' from an ECClass as it is used as KeyProperty in a relationship.",
                                          deletedProperty.GetClass().GetFullName(), deletedProperty.GetName().c_str());
                return ERROR;
                }

            //For virtual column delete column from ec_Column.
            if (column->GetPersistenceType() == PersistenceType::Virtual)
                {
                CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_Column WHERE Id = ?");
                stmt->BindId(1, column->GetId());
                if (stmt->Step() != BE_SQLITE_DONE)
                    {
                    BeAssert(false && "Failed to delete DbColumn");
                    return ERROR;
                    }
                }
            else
                {
                //Fail we do not want to delete a sql column right now
                Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECClass %s: Deleting an ECProperty '%s' from an ECClass is not supported as property mapped to a none-shared column.",
                                          deletedProperty.GetClass().GetFullName(), deletedProperty.GetName().c_str());
                return ERROR;
                }
            }
        }
    
    if (sharedColumnFound)
        {
        if (setPropertyToNull() != SUCCESS)
            return ERROR;
        }

    //Delete ECProperty entry make sure ec_Column is already deleted or set to null
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_Property WHERE ec_Property.Id = ?");
    stmt->BindId(1, deletedProperty.GetId());
    if (stmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false && "Failed to delete ecproperty");
        return ERROR;
        }

    return DeleteECCustomAttributes(deletedProperty.GetId(), ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECClasses(ECClassChanges& classChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
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

            if (DeleteECClass(change, *oldClass) == ERROR)
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

            if (ImportECClass(*newClass) == ERROR)
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

            if (UpdateECClass(change, *oldClass, *newClass) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECKindOfQuanitites(ECKindOfQuantityChanges& koqChanges, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema)
    {
    if (!koqChanges.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < koqChanges.Count(); i++)
        {
        KindOfQuantityChange& change = koqChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting KindOfQuantity from an ECSchema is not supported.",
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
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. KindOfQuantity %s in ECSchema %s: Changing KindOfQuantity is not supported.",
                            change.GetId(), oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECEnumerations(ECEnumerationChanges& enumChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (!enumChanges.IsValid())
        return SUCCESS;

    for (size_t i = 0; i < enumChanges.Count(); i++)
        {
        ECEnumerationChange& change = enumChanges.At(i);
        if (change.GetState() == ChangeState::Deleted)
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Deleting ECEnumerations from an ECSchema is not supported.",
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

            return ImportECEnumeration(*ecEnum);
            }
        else if (change.GetState() == ChangeState::Modified)
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECEnumeration %s in ECSchema %s: Changing ECEnumerations is not supported.",
                                      change.GetId(), oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::UpdateECSchema(ECSchemaChange& schemaChange, ECSchemaCR oldSchema, ECSchemaCR newSchema)
    {
    if (schemaChange.GetStatus() == ECChange::Status::Done)
        return SUCCESS;
   
    ECSchemaId schemaId = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, newSchema);
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
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Name must always be set.",
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

    if (schemaChange.GetVersionMajor().IsValid())
        {
        if (schemaChange.GetVersionMajor().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionMajor().GetValue(ValueId::New).Value())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Decreasing 'VersionMajor' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        m_majorChangesAllowedForSchemas.insert(oldSchema.GetId());
        updateBuilder.AddSetExp("VersionDigit1", schemaChange.GetVersionMajor().GetNew().Value());
        }

    if (schemaChange.GetVersionWrite().IsValid())
        {
        if (schemaChange.GetVersionWrite().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionWrite().GetValue(ValueId::New).Value())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Decreasing 'VersionWrite' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit2", schemaChange.GetVersionWrite().GetNew().Value());
        }

    if (schemaChange.GetVersionMinor().IsValid())
        {
        if (schemaChange.GetVersionMinor().GetValue(ValueId::Deleted).Value() > schemaChange.GetVersionMinor().GetValue(ValueId::New).Value())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Decreasing 'VersionMinor' of an ECSchema is not supported.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("VersionDigit3", schemaChange.GetVersionMinor().GetNew().Value());
        }

    if (schemaChange.GetAlias().IsValid())
        {
        if (schemaChange.GetAlias().GetNew().IsNull())
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Alias must always be set.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        if (ECDbSchemaPersistenceHelper::ContainsECSchemaWithAlias(m_ecdb, schemaChange.GetAlias().GetNew().Value().c_str()))
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECSchema Update failed. ECSchema %s: Alias is already used by another existing ECSchema.",
                                      oldSchema.GetFullSchemaName().c_str());
            return ERROR;
            }

        updateBuilder.AddSetExp("NamespacePrefix", schemaChange.GetAlias().GetNew().Value().c_str());
        }

    updateBuilder.AddWhereExp("Id", schemaId.GetValue());//this could even be on name
    if (updateBuilder.IsValid())
        {
        if (updateBuilder.ExecuteSql(m_ecdb) != SUCCESS)
            return ERROR;
        }

    schemaChange.SetStatus(ECChange::Status::Done);

    if (UpdateECSchemaReferences(schemaChange.References(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdateECEnumerations(schemaChange.Enumerations(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdateECKindOfQuanitites(schemaChange.KindOfQuantities(), oldSchema, newSchema) == ERROR)
        return ERROR;

    if (UpdateECClasses(schemaChange.Classes(), oldSchema, newSchema) == ERROR)
        return ERROR;

    return UpdateECCustomAttributes(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema, schemaId, schemaChange.CustomAttributes(), oldSchema, newSchema);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
//static
DbResult ECDbSchemaWriter::RepopulateClassHierarchyTable(ECDbCR ecdb)
    {
    StopWatch timer(true);
    DbResult r = ecdb.ExecuteSql("DELETE FROM ec_ClassHierarchy");
    if (r != BE_SQLITE_OK)
        return r;

    r = ecdb.ExecuteSql("WITH RECURSIVE "
                        "BaseClassList(ClassId, BaseClassId) AS "
                        "(SELECT Id, Id FROM ec_Class"
                        " UNION"
                        " SELECT DCL.ClassId, BC.BaseClassId FROM BaseClassList DCL"
                        " INNER JOIN ec_ClassHasBaseClasses BC ON BC.ClassId = DCL.BaseClassId"
                        " ORDER BY 2)"
                        " INSERT INTO ec_ClassHierarchy SELECT NULL Id, ClassId, BaseClassId FROM BaseClassList");

    if (r != BE_SQLITE_OK)
        return r;

    timer.Stop();
    LOG.debugv("Re-populated ec_ClassHierarchy in %.4f msecs.", timer.GetElapsedSeconds() * 1000.0);
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
