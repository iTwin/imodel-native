/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaWriter::CreateECSchemaEntry(ECSchemaCR ecSchema, ECSchemaId ecSchemaId)
    {
    DbECSchemaInfo info;
    info.ColsInsert =
        DbECSchemaInfo::COL_Id |
        DbECSchemaInfo::COL_Name |
        DbECSchemaInfo::COL_VersionMajor |
        DbECSchemaInfo::COL_VersionMinor |
        DbECSchemaInfo::COL_Description |
        DbECSchemaInfo::COL_NamespacePrefix;

    info.m_ecSchemaId   = ecSchemaId;
    info.m_versionMajor = ecSchema.GetVersionMajor();
    info.m_versionMinor = ecSchema.GetVersionMinor();

    info.m_namespacePrefix = ecSchema.GetNamespacePrefix().c_str();
    info.m_name = ecSchema.GetName().c_str();
    info.m_description = ecSchema.GetDescription().c_str();

    if (ecSchema.GetIsDisplayLabelDefined())
        {
        info.m_displayLabel = ecSchema.GetDisplayLabel().c_str();
        info.ColsInsert |= DbECSchemaInfo::COL_DisplayLabel;
        }

    return ECDbSchemaPersistence::InsertECSchema (m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int index)
    {
    DbBaseClassInfo info;

    info.ColsInsert =
        DbBaseClassInfo::COL_ClassId     |
        DbBaseClassInfo::COL_BaseClassId |
        DbBaseClassInfo::COL_Ordinal;

    info.m_ecClassId      = ecClassId;
    info.m_baseECClassId  = baseClass.GetId();
    info.m_ecIndex        = index;

    //save to db
    return ECDbSchemaPersistence::InsertBaseClass (m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECPropertyEntry(ECPropertyCR ecProperty, ECPropertyId ecPropertyId, ECClassId ecClassId, int32_t index)
    {
    DbECPropertyInfo info;
    info.ColsInsert =
        DbECPropertyInfo::COL_Id           |
        DbECPropertyInfo::COL_ClassId    |
        DbECPropertyInfo::COL_IsArray      |
        DbECPropertyInfo::COL_Description  |
        DbECPropertyInfo::COL_Ordinal      |
        DbECPropertyInfo::COL_IsReadonly   |
        DbECPropertyInfo::COL_Name;

    info.m_ecClassId    = ecClassId;
    info.m_ecPropertyId = ecPropertyId;
    info.m_isArray      = ecProperty.GetIsArray();
    info.m_ordinal      = index;
    info.m_isReadOnly   = ecProperty.GetIsReadOnly();
    info.m_name = ecProperty.GetName().c_str();
    info.m_description = ecProperty.GetDescription().c_str();
    if (ecProperty.GetIsDisplayLabelDefined())
        {
        info.ColsInsert |= DbECPropertyInfo::COL_DisplayLabel;
        info.m_displayLabel = ecProperty.GetDisplayLabel().c_str();
        }
    if (ecProperty.GetIsPrimitive())
        {
        info.ColsInsert |= DbECPropertyInfo::COL_PrimitiveType;
        info.m_primitiveType = ecProperty.GetAsPrimitiveProperty()->GetType();            
        }
    else if( ecProperty.GetIsStruct())
        {
        info.ColsInsert |= DbECPropertyInfo::COL_StructType;
        info.m_structType = ecProperty.GetAsStructProperty()->GetType().GetId();
        }
    else if( ecProperty.GetIsArray())
        {
        ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
        StructArrayECPropertyCP structArrayProperty = ecProperty.GetAsStructArrayProperty();
        info.ColsInsert |= DbECPropertyInfo::COL_MinOccurs;
        info.ColsInsert |= DbECPropertyInfo::COL_MaxOccurs;

        if (nullptr == structArrayProperty)
            {
            info.ColsInsert |= DbECPropertyInfo::COL_PrimitiveType;
            info.m_primitiveType = arrayProperty->GetPrimitiveElementType();            
            }
        else // ARRAYKIND_Struct
            {
            info.ColsInsert |= DbECPropertyInfo::COL_StructType;
            info.m_structType = structArrayProperty->GetStructElementType()->GetId();
            }

        info.m_minOccurs = arrayProperty->GetMinOccurs ();
        //until the max occurs bug in ECObjects (where GetMaxOccurs always returns "unbounded")
        //has been fixed, we need to call GetStoredMaxOccurs to retrieve the proper max occurs
        info.m_maxOccurs = arrayProperty->GetStoredMaxOccurs ();
        }

    return ECDbSchemaPersistence::InsertECProperty (m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECRelationshipConstraintEntry(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    DbECRelationshipConstraintInfo info;

    info.ColsInsert =
        DbECRelationshipConstraintInfo::COL_RelationshipClassId   |
        DbECRelationshipConstraintInfo::COL_RelationshipEnd |
        DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit |
        DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit |
        DbECRelationshipConstraintInfo::COL_IsPolymorphic;

    info.m_relationshipClassId = relationshipClassId;
    info.m_ecRelationshipEnd = endpoint;
    info.m_cardinalityLowerLimit = relationshipConstraint.GetCardinality().GetLowerLimit();
    info.m_cardinalityUpperLimit = relationshipConstraint.GetCardinality().GetUpperLimit();
    info.m_isPolymorphic         = relationshipConstraint.GetIsPolymorphic();

    if (relationshipConstraint.IsRoleLabelDefined())
        {
        info.m_roleLabel = relationshipConstraint.GetRoleLabel().c_str();
        info.ColsInsert |= DbECRelationshipConstraintInfo::COL_RoleLabel;
        }
    //save to db
    return ECDbSchemaPersistence::InsertECRelationshipConstraint (m_ecdb, info);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaWriter::InsertCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, ECContainerId overridenContainerId, int index)
    {
    BeAssert (overridenContainerId == 0 && "OverriddenContainerId was removed from ec_CustomAttribute. We don't expect that to be set during schema import/update.");
    DbCustomAttributeInfo insertInfo;
    insertInfo.ColsInsert =
        DbCustomAttributeInfo::COL_ContainerId |
        DbCustomAttributeInfo::COL_ContainerType |
        DbCustomAttributeInfo::COL_ClassId |
        DbCustomAttributeInfo::COL_Ordinal;


    insertInfo.m_containerId = containerId;
    insertInfo.m_containerType = containerType;
    insertInfo.m_ecClassId = ecClassId;
    insertInfo.m_index = index;

    if (customAttribute != nullptr)
        {
        if (SUCCESS != insertInfo.SerializeCaInstance(*customAttribute))
            return ERROR;

        insertInfo.ColsInsert |=DbCustomAttributeInfo::COL_Instance;
        }

    return ECDbSchemaPersistence::InsertCustomAttribute (m_ecdb, insertInfo);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECRelationshipConstraintClassEntry(ECClassId relationshipClassId, ECClassId constraintClassId, ECRelationshipEnd endpoint)
    {
    DbECRelationshipConstraintClassInfo info;
    info.ColsInsert =
        DbECRelationshipConstraintClassInfo::COL_RelationshipClassId |
        DbECRelationshipConstraintClassInfo::COL_RelationshipEnd |
        DbECRelationshipConstraintClassInfo::COL_ConstraintClassId;
    info.m_relationshipClassId = relationshipClassId;
    info.m_ecRelationshipEnd = endpoint;
    info.m_constraintClassId = constraintClassId;
    return ECDbSchemaPersistence::InsertECRelationshipConstraintClass(m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::Import(ECN::ECSchemaCR ecSchema)
    {
    BeMutexHolder lock(m_mutex);
    ECSchemaId ecSchemaId = ECDbSchemaPersistence::GetECSchemaId(m_ecdb, ecSchema);
    if (0 != ecSchemaId)
        {
        BeAssert(false && "Did not import ECSchema (it already exists in the ECDb). We should have checked earlier, that it already exists");
        return SUCCESS;
        }

    // GenerateId
    BeBriefcaseBasedId nextId;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetECSchemaIdSequence().GetNextValue(nextId))
        {
        BeAssert(false && "Could not generate new ECSchemaId");
        return ERROR;
        }

    ecSchemaId = nextId.GetValue ();
    const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);

    DbResult stat = CreateECSchemaEntry(ecSchema, ecSchemaId);
    if (BE_SQLITE_OK != stat)
        {
        if (BE_SQLITE_CONSTRAINT_UNIQUE == stat)
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchema '%s'. Namespace prefix '%s' is already used by an existing ECSchema.", 
                                                            ecSchema.GetFullSchemaName().c_str(), ecSchema.GetNamespacePrefix().c_str());
        return ERROR;
        }

    ECSchemaReferenceListCR referencedSchemas = ecSchema.GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaCP reference = iter->second.get();
        ECSchemaId referenceId = ECDbSchemaPersistence::GetECSchemaId (m_ecdb, *reference); 
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

        if (!ECDbSchemaPersistence::ContainsECSchemaReference (m_ecdb, ecSchemaId, referenceId))
            {
            if (SUCCESS != CreateECSchemaReferenceEntry(ecSchemaId, referenceId))
                {
                BeAssert(false && "Could not insert schema reference entry");
                return ERROR;
                }
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

    if (SUCCESS != ImportCustomAttributes(ecSchema, ecSchemaId, ECContainerType::Schema))
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import custom attributes of ECSchema '%s'.", ecSchema.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::CreateECSchemaReferenceEntry(ECSchemaId ecSchemaId, ECSchemaId ecReferencedSchemaId)
    {
    DbECSchemaReferenceInfo  info;
    info.ColsInsert          = DbECSchemaReferenceInfo::COL_SchemaId | DbECSchemaReferenceInfo::COL_ReferencedSchemaId ;
    info.m_ecSchemaId          = ecSchemaId;
    info.m_referencedECSchemaId = ecReferencedSchemaId;

    return ECDbSchemaPersistence::InsertECSchemaReference(m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, Utf8CP onlyImportCAWithClassName)
    {
    if (SUCCESS != ImportECCustomAttributeECClass(sourceContainer))
        return ERROR;

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

        if (SUCCESS != InsertCAEntry (customAttribute, customAttributeClassId, sourceContainerId, containerType, 0 /*Overriden continer id is nullptr=0*/, index++))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::EnsureECSchemaExists(ECClassCR ecClass)
    {
    ECSchemaId ecSchemaId = ecClass.GetSchema().GetId();
    if (ECDbSchemaPersistence::ContainsECSchemaWithId(m_ecdb, ecSchemaId))
        return SUCCESS;
    BeAssert(false && "I think we just should assume that the entry already exists, rather than relying on just-in-time? Or is this for when we branch off into related ECClasses?");
    //Add ECSchema entry but do not traverse its ECClasses.
    if (BE_SQLITE_OK != CreateECSchemaEntry(ecClass.GetSchema(), ecSchemaId))
        return ERROR;

    return ImportECCustomAttributeECClass(ecClass.GetSchema());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECClass(ECN::ECClassCR ecClass)
    {
    if (ECDbSchemaPersistence::ContainsECClass(m_ecdb, ecClass))
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

    if (SUCCESS != ECDbSchemaPersistence::InsertECClass(m_ecdb,ecClass))
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
        if (SUCCESS != ImportECProperty(*ecProperty, ecClassId, propertyIndex++))
            {
            LOG.errorv("Failed to import ECProperty '%s' of ECClass '%s'.", ecProperty->GetName().c_str(), ecClass.GetFullName());
            return ERROR;
            }
        }

    ECN::ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr)
        {
        if (SUCCESS != ImportECRelationshipClass(relationship, ecClassId))
            return ERROR;
        }

    return ImportCustomAttributes(ecClass, ecClassId, ECContainerType::Class);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECRelationshipClass(ECN::ECRelationshipClassCP relationship, ECClassId relationshipClassId)
    {
    if (SUCCESS != ImportECRelationshipConstraint(relationshipClassId, relationship->GetSource(), ECRelationshipEnd_Source))
        return ERROR;

    return ImportECRelationshipConstraint(relationshipClassId, relationship->GetTarget(), ECRelationshipEnd_Target);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECRelationshipConstraint(ECClassId relClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    if (SUCCESS != CreateECRelationshipConstraintEntry(relClassId, relationshipConstraint, endpoint))
        return ERROR;

    for (ECRelationshipConstraintClassCP constraintClassObj : relationshipConstraint.GetConstraintClasses())
        {
        ECClassCR constraintClass = constraintClassObj->GetClass();
        if (SUCCESS != ImportECClass(constraintClass))
            return ERROR;

        ECClassId constraintClassId = constraintClass.GetId();
        if (SUCCESS != CreateECRelationshipConstraintClassEntry(relClassId, constraintClassId, endpoint))
            return ERROR;

        for (Utf8StringCR key : constraintClassObj->GetKeys())
            {
            //key validation done later at mapping time
            DbECRelationshipConstraintClassKeyPropertyInfo keyPropertyInfo;
            keyPropertyInfo.ColsInsert =
                DbECRelationshipConstraintClassKeyPropertyInfo::COL_RelationshipClassId |
                DbECRelationshipConstraintClassKeyPropertyInfo::COL_RelationshipEnd |
                DbECRelationshipConstraintClassKeyPropertyInfo::COL_ConstraintClassId;
            keyPropertyInfo.m_relationECClassId = relClassId;
            keyPropertyInfo.m_ecRelationshipEnd = endpoint;
            keyPropertyInfo.m_constraintClassId = constraintClassId;
            keyPropertyInfo.m_keyPropertyName.assign(key);

            if (SUCCESS != ECDbSchemaPersistence::InsertECRelationshipConstraintClassKeyProperty(m_ecdb, keyPropertyInfo))
                return ERROR;
            }
        }
    ECContainerType containerType = endpoint == ECRelationshipEnd_Source ? ECContainerType::RelationshipConstraintSource : ECContainerType::RelationshipConstraintTarget;
    return ImportCustomAttributes(relationshipConstraint, relClassId, containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaWriter::ImportECProperty(ECN::ECPropertyCR ecProperty, ECClassId ecClassId, int32_t index)
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

    if (SUCCESS != CreateECPropertyEntry(ecProperty, ecPropertyId, ecClassId, index))
        return ERROR;

    return ImportCustomAttributes(ecProperty, ecPropertyId, ECContainerType::Property);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  ECDbSchemaWriter::ImportECCustomAttributeECClass(ECN::IECCustomAttributeContainerCR caContainer)
    {
    for (IECInstancePtr ca : caContainer.GetCustomAttributes(false))
        {
        if (SUCCESS != ImportECClass (ca->GetClass()))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaWriterPtr ECDbSchemaWriter::Create (ECDbR ecdb)
    {
    return new ECDbSchemaWriter(ecdb);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
