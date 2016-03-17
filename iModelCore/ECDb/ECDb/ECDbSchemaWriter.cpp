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
BentleyStatus ECDbSchemaWriter::CreateECSchemaEntry(ECSchemaCR ecSchema)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Schema(Id,Name,DisplayLabel,Description,NamespacePrefix,VersionDigit1,VersionDigit2,VersionDigit3) VALUES(?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1,ecSchema.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(2, ecSchema.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (ecSchema.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(3, ecSchema.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(4, ecSchema.GetDescription().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(5, ecSchema.GetNamespacePrefix().c_str(), Statement::MakeCopy::No))
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
BentleyStatus ECDbSchemaWriter::CreateBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int ordinal)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_BaseClass(ClassId,BaseClassId,Ordinal) VALUES(?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(2, baseClass.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, ordinal))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECRelationshipConstraintEntry(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraint (RelationshipClassId,RelationshipEnd,MultiplicityLowerLimit,MultiplicityUpperLimit,RoleLabel,IsPolymorphic) VALUES (?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, relationshipClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, endpoint))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(3, relationshipConstraint.GetCardinality().GetLowerLimit()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(4, relationshipConstraint.GetCardinality().GetUpperLimit()))
        return ERROR;

    if (relationshipConstraint.IsRoleLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(5, relationshipConstraint.GetRoleLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindInt(6, relationshipConstraint.GetIsPolymorphic() ? 1 : 0))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::InsertCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, int ordinal)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_CustomAttribute(ContainerId,ContainerType,ClassId,Ordinal,Instance) VALUES(?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, containerId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(containerType)))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(3, ecClassId))
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


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::Import(ECN::ECSchemaCR ecSchema)
    {
    BeMutexHolder lock(m_mutex);

    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECSchemaIdSequence().GetNextValue(nextId))
        {
        BeAssert(false && "Could not generate new ECSchemaId");
        return ERROR;
        }

    const ECSchemaId ecSchemaId = nextId.GetValue ();
    const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);

    if (SUCCESS != CreateECSchemaEntry(ecSchema))
        {
        DbResult lastErrorCode;
        m_ecdb.GetLastError(&lastErrorCode);
        if (BE_SQLITE_CONSTRAINT_UNIQUE == lastErrorCode)
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchema '%s'. Namespace prefix '%s' is already used by an existing ECSchema.", 
                                                            ecSchema.GetFullSchemaName().c_str(), ecSchema.GetNamespacePrefix().c_str());
        return ERROR;
        }

    ECSchemaReferenceListCR referencedSchemas = ecSchema.GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaCP reference = iter->second.get();
        ECSchemaId referenceId = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, reference->GetName().c_str());
        if (0ULL == referenceId)
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

        if (SUCCESS != CreateECSchemaReferenceEntry(ecSchemaId, referenceId))
            {
            BeAssert(false && "Could not insert schema reference entry");
            return ERROR;
            }
        }

    //enums must be imported before ECClasses as properties reference enums
    for (ECEnumerationCP ecEnum : ecSchema.GetEnumerations())
        {
        if (SUCCESS != ImportECEnumeration(*ecEnum))
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECEnumeration '%s'.", ecEnum->GetFullName().c_str());
            return ERROR;
            }
        }

    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        if (SUCCESS != ImportECClass(*ecClass))
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECClass '%s'.", ecClass->GetFullName());
            return ERROR;
            }
        }

    if (SUCCESS != ImportCustomAttributes(ecSchema, ECContainerId(ecSchemaId), ECContainerType::Schema))
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import custom attributes of ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECSchemaReferenceEntry(ECSchemaId schemaId, ECSchemaId referencedSchemaId)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_SchemaReference (SchemaId,ReferencedSchemaId) VALUES(?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, schemaId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(2, referencedSchemaId))
        return ERROR;

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, Utf8CP onlyImportCAWithClassName)
    {
    //import CA classes first
    for (IECInstancePtr ca : sourceContainer.GetCustomAttributes(false))
        {
        if (SUCCESS != ImportECClass(ca->GetClass()))
            return ERROR;
        }

    bmap<ECClassCP,bvector<IECInstanceP> > customAttributeMap;
    for (auto& customAttribute: sourceContainer.GetCustomAttributes(false))
        {
        if (onlyImportCAWithClassName == nullptr)
            customAttributeMap [&(customAttribute->GetClass())].push_back(customAttribute.get());
        else
            {
            if (customAttribute->GetClass().GetName().Equals(onlyImportCAWithClassName))
                customAttributeMap [&(customAttribute->GetClass())].push_back(customAttribute.get());
            }
        }
    int index = 0; // Its useless if we enumerate map since it doesn't ensure order in which we added it

    bmap<ECClassCP,bvector<IECInstanceP> >::const_iterator itor = customAttributeMap.begin();

    //Here we consider consolidated attribute a primary. This is lossy operation some overridden primary custom attributes would be lost
    for ( ; itor != customAttributeMap.end(); ++itor)
        {
        bvector<IECInstanceP> const& customAttributes = itor->second;
        IECInstanceP customAttribute = customAttributes.size() == 1 ? customAttributes[0] : customAttributes[1];
        ECClassCP ecClass = itor->first;
        ECClassId customAttributeClassId;
        if (ecClass->HasId())
            customAttributeClassId = ecClass->GetId();
        else
            customAttributeClassId = ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (m_ecdb, *ecClass);

        if (SUCCESS != InsertCAEntry (customAttribute, customAttributeClassId, sourceContainerId, containerType, index++))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::EnsureECSchemaExists(ECClassCR ecClass)
    {
    ECSchemaCR schema = ecClass.GetSchema();
    ECSchemaId ecSchemaId = schema.GetId();

    if (ECDbSchemaPersistenceHelper::ContainsECSchema(m_ecdb, ecSchemaId))
        return SUCCESS;

    BeAssert(false && "I think we just should assume that the entry already exists, rather than relying on just-in-time? Or is this for when we branch off into related ECClasses?");
    //Add ECSchema entry but do not traverse its ECClasses.
    if (SUCCESS != CreateECSchemaEntry(schema))
        return ERROR;

    //import CA classes first
    for (IECInstancePtr ca : schema.GetCustomAttributes(false))
        {
        if (SUCCESS != ImportECClass(ca->GetClass()))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECClass(ECN::ECClassCR ecClass)
    {
    if (ECDbSchemaPersistenceHelper::ContainsECClass(m_ecdb, ecClass))
        {
        if (!ecClass.HasId())
            ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(m_ecdb, ecClass); //Callers will assume it has a valid Id

        return SUCCESS;
        }

    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECClassIdSequence().GetNextValue(nextId))
        return ERROR;

    ECClassId ecClassId = nextId.GetValue ();
    const_cast<ECClassR>(ecClass).SetId (ecClassId);

    EnsureECSchemaExists(ecClass);

    //now import actual ECClass
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Class(Id,SchemaId,Name,DisplayLabel,Description,Type,Modifier,RelationshipStrength,RelationshipStrengthDirection) "
                                              "VALUES(?,?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecClassId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(2, ecClass.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecClass.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (ecClass.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecClass.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ecClass.GetDescription().c_str(), Statement::MakeCopy::No))
        return ERROR;

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

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    //Import All baseCases
    int baseClassIndex = 0;
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        if (SUCCESS != ImportECClass(*baseClass))
            return ERROR;

        if (SUCCESS != CreateBaseClassEntry(ecClassId, *baseClass, baseClassIndex++))
            return ERROR;
        }

    int propertyIndex = 0;
    for (ECPropertyCP ecProperty: ecClass.GetProperties(false))
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

    return ImportCustomAttributes(ecClass, ECContainerId(ecClassId), ECContainerType::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::ImportECEnumeration(ECN::ECEnumerationCR ecEnum)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Enumeration(Id, SchemaId, Name, DisplayLabel, Description, UnderlyingPrimitiveType, IsStrict, EnumValues) VALUES(?,?,?,?,?,?,?,?)"))
        return ERROR;

    BeBriefcaseBasedId enumId;
    if (m_ecdb.GetECDbImplR().GetECEnumIdSequence().GetNextValue(enumId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, enumId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(2, ecEnum.GetSchema().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecEnum.GetName().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (ecEnum.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecEnum.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ecEnum.GetDescription().c_str(), Statement::MakeCopy::No))
        return ERROR;

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

    //cache the id because the ECEnumeration class itself doesn't have an id.
    m_enumIdCache[&ecEnum] = enumId.GetValue();
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
BentleyStatus ECDbSchemaWriter::ImportECRelationshipConstraint(ECClassId relClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd end)
    {
    BeAssert(relClassId != ECClass::UNSET_ECCLASSID);

    if (SUCCESS != CreateECRelationshipConstraintEntry(relClassId, relationshipConstraint, end))
        return ERROR;

    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_RelationshipConstraintClass(RelationshipClassId,RelationshipEnd,ClassId,KeyProperties) VALUES(?,?,?,?)"))
        return ERROR;

    for (ECRelationshipConstraintClassCP constraintClassObj : relationshipConstraint.GetConstraintClasses())
        {
        ECClassCR constraintClass = constraintClassObj->GetClass();
        if (SUCCESS != ImportECClass(constraintClass))
            return ERROR;

        BeAssert(constraintClass.GetId() != ECClass::UNSET_ECCLASSID);

        if (BE_SQLITE_OK != stmt->BindInt64(1, relClassId))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(2, (int) end))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt64(3, constraintClass.GetId()))
            return ERROR;

        bvector<Utf8String> const& keyPropNames = constraintClassObj->GetKeys();
        Utf8String keyPropJson;
        if (!keyPropNames.empty())
            {
            ECDbSchemaPersistenceHelper::SerializeRelationshipKeyProperties(keyPropJson, keyPropNames);
            if (BE_SQLITE_OK != stmt->BindText(4, keyPropJson.c_str(), Statement::MakeCopy::No))
                return ERROR;
            }

        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;

        stmt->Reset();
        stmt->ClearBindings();
        }

    stmt = nullptr;

    ECContainerType containerType = end == ECRelationshipEnd_Source ? ECContainerType::RelationshipConstraintSource : ECContainerType::RelationshipConstraintTarget;
    return ImportCustomAttributes(relationshipConstraint, ECContainerId(relClassId), containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECProperty(ECN::ECPropertyCR ecProperty, int32_t ordinal)
    {
    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECPropertyIdSequence().GetNextValue(nextId))
        return ERROR;

    ECPropertyId ecPropertyId = nextId.GetValue ();
    const_cast<ECPropertyR>(ecProperty).SetId (ecPropertyId);

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
    BeSQLite::CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != m_ecdb.GetCachedStatement(stmt, "INSERT INTO ec_Property(Id,ClassId,Name,DisplayLabel,Description,IsReadonly,Ordinal,Kind,PrimitiveType,NonPrimitiveType,ExtendedType,Enumeration,ArrayMinOccurs,ArrayMaxOccurs,NavigationPropertyDirection) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(1, ecProperty.GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt64(2, ecProperty.GetClass().GetId()))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindText(3, ecProperty.GetName(), Statement::MakeCopy::No))
        return ERROR;

    if (ecProperty.GetIsDisplayLabelDefined())
        {
        if (BE_SQLITE_OK != stmt->BindText(4, ecProperty.GetDisplayLabel().c_str(), Statement::MakeCopy::No))
            return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindText(5, ecProperty.GetDescription().c_str(), Statement::MakeCopy::No))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindInt(6, ecProperty.GetIsReadOnly() ? 1 : 0))
        return ERROR;

    //WIP Ordinal
    if (BE_SQLITE_OK != stmt->BindInt(7, ordinal))
        return ERROR;

    const int kindIndex = 8;
    const int primitiveTypeIndex = 9;
    const int nonPrimitiveTypeIndex = 10;
    const int extendedTypeIndex = 11;
    const int enumTypeIndex = 12;
    const int arrayMinIndex = 13;
    const int arrayMaxIndex = 14;
    const int navDirIndex = 15;

    if (ecProperty.GetIsPrimitive())
        {
        PrimitiveECPropertyCP primProp = ecProperty.GetAsPrimitiveProperty();
        ECEnumerationCP ecenum = primProp->GetEnumeration();
        if (ecenum == nullptr)
            {
            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Primitive)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindInt(primitiveTypeIndex, (int) primProp->GetType()))
                return ERROR;
            }
        else
            {
            auto it = m_enumIdCache.find(ecenum);
            if (it == m_enumIdCache.end())
                {
                BeAssert(false && "ECEnumeration should have been imported before any ECProperty");
                return ERROR;
                }

            const uint64_t enumId = it->second;

            if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Enumeration)))
                return ERROR;

            if (BE_SQLITE_OK != stmt->BindInt64(enumTypeIndex, enumId))
                return ERROR;
            }

        if (primProp->HasExtendedType())
            {
            if (BE_SQLITE_OK != stmt->BindText(extendedTypeIndex, primProp->GetExtendedTypeName().c_str(), Statement::MakeCopy::No))
                return ERROR;
            }
        }
    else if (ecProperty.GetIsStruct())
        {
        if (BE_SQLITE_OK != stmt->BindInt(kindIndex, Enum::ToInt(ECPropertyKind::Struct)))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt64(nonPrimitiveTypeIndex, ecProperty.GetAsStructProperty()->GetType().GetId()))
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

            if (BE_SQLITE_OK != stmt->BindInt64(nonPrimitiveTypeIndex, arrayProp->GetAsStructArrayProperty()->GetStructElementType()->GetId()))
                return ERROR;
            }

        if (arrayProp->HasExtendedType())
            {
            if (BE_SQLITE_OK != stmt->BindText(extendedTypeIndex, arrayProp->GetExtendedTypeName().c_str(), Statement::MakeCopy::No))
                return ERROR;
            }

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
        if (BE_SQLITE_OK != stmt->BindInt64(nonPrimitiveTypeIndex, navProp->GetRelationshipClass()->GetId()))
            return ERROR;

        if (BE_SQLITE_OK != stmt->BindInt(navDirIndex, Enum::ToInt(navProp->GetDirection())))
            return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;   

    return ImportCustomAttributes(ecProperty, ECContainerId(ecPropertyId), ECContainerType::Property);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
