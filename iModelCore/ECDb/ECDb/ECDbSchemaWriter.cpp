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
bool ECDbSchemaWriter::EnsureNamespacePrefixIsUnique (ECSchemaCR ecSchema)
    {
    /*ECDB_RULE: ECSchema Prefix need to be unique so if there is a existing schema with prefix of this schema
                 then we need to to add a number to that to make it unique. If prefix is empty then we assign
                 schema name to it.

    */
    WString ns = ecSchema.GetNamespacePrefix();
    ns.Trim();
    if (ns.empty())
        {
        LOG.warningv(L"Importing ECSchema '%ls' has no NamespacePrefix. Name of this ECSchema will be set as its NamespacePrefix", ecSchema.GetName().c_str());
        ns = ecSchema.GetName();
        }
    //verify ns is unique
    Utf8String currentNSPrefix = Utf8String(ns.c_str());
    Statement stmt;
    stmt.Prepare(m_ecdb,"SELECT Name FROM ec_Schema WHERE NamespacePrefix = ?");
    stmt.BindText(1, currentNSPrefix, Statement::MakeCopy::No);
    if (stmt.Step() == BE_SQLITE_ROW)
        {       
        LOG.warningv (L"Importing ECSchema '%ls' has NamespacePrefix '%ls' which already exist in ECDb for ECSchema '%ls. System will now attempt to generate a unique prefix for importing ECSchema.", ecSchema.GetName().c_str(), ns.c_str(), WString (stmt.GetValueText(0), BentleyCharEncoding::Utf8).c_str ());
        Utf8String newPrefix;
        int prefixIndex = 1;
        do
            {
            stmt.Reset ();
            stmt.ClearBindings ();
            newPrefix.Sprintf ("%s%d", currentNSPrefix.c_str(), prefixIndex++);
            stmt.BindText(1, newPrefix, Statement::MakeCopy::No);
            } while(stmt.Step() == BE_SQLITE_ROW);
        if (currentNSPrefix.Equals(newPrefix))
            {
            BeAssert(false && "Failed to generate a unique schema prefix");
            LOG.errorv (L"Failed to Generate Unique NamespacePrefix for ECSchema %ls", ecSchema.GetName().c_str());    
            return false;
            }
        ns.AssignUtf8(newPrefix.c_str());
        LOG.warningv (L"Generated a new NamespacePrefix='%ls' for newly importing ECSchema '%ls'", ecSchema.GetName().c_str(), ns.c_str());
        }
    if (!ns.Equals(ecSchema.GetNamespacePrefix()))
        const_cast<ECSchemaR>(ecSchema).SetNamespacePrefix(ns);
    return true;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::CreateECSchemaEntry (ECSchemaCR ecSchema, ECSchemaId ecSchemaId)
    {
    //We need to always ensure namespace prefix is always unique and if necessary generate new namespace prefix. 
    if (!EnsureNamespacePrefixIsUnique (ecSchema))
        return BE_SQLITE_ERROR;

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

    BeStringUtilities::WCharToUtf8 (info.m_namespacePrefix, ecSchema.GetNamespacePrefix().c_str());
    BeStringUtilities::WCharToUtf8 (info.m_name, ecSchema.GetName().c_str());
    BeStringUtilities::WCharToUtf8 (info.m_description, ecSchema.GetDescription().c_str());

    if (ecSchema.GetIsDisplayLabelDefined())
        {
        BeStringUtilities::WCharToUtf8 (info.m_displayLabel, ecSchema.GetDisplayLabel().c_str());
        info.ColsInsert |= DbECSchemaInfo::COL_DisplayLabel;
        }

    //save to db
    return ECDbSchemaPersistence::InsertECSchemaInfo (m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::CreateECClassEntry (ECClassCR ecClass, ECClassId ecClassId)
    {
    DbECClassInfo info;

    info.ColsInsert =
        DbECClassInfo::COL_Id                |
        DbECClassInfo::COL_SchemaId        |
        DbECClassInfo::COL_Name              |
        DbECClassInfo::COL_Description       |
        DbECClassInfo::COL_IsCustomAttribute |
        DbECClassInfo::COL_IsStruct          |
        DbECClassInfo::COL_IsDomainClass     |
        DbECClassInfo::COL_IsRelationship;

    info.m_ecClassId         = ecClassId;
    info.m_ecSchemaId        = ecClass.GetSchema().GetId();
    info.m_isCustomAttribute = ecClass.GetIsCustomAttributeClass();
    info.m_isStruct          = ecClass.GetIsStruct();
    info.m_isDomainClass     = ecClass.GetIsDomainClass();
    info.m_isRelationship    = false;
    BeStringUtilities::WCharToUtf8 (info.m_name, ecClass.GetName().c_str());
    BeStringUtilities::WCharToUtf8 (info.m_description, ecClass.GetDescription().c_str());

    if (ecClass.GetIsDisplayLabelDefined())
        {
        BeStringUtilities::WCharToUtf8 (info.m_displayLabel, ecClass.GetDisplayLabel().c_str());
        info.ColsInsert |= DbECClassInfo::COL_DisplayLabel;
        }

    ECN::ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr)
        {
        info.ColsInsert |=
            DbECClassInfo::COL_RelationStrength          |
            DbECClassInfo::COL_RelationStrengthDirection;

        info.m_relationStrength          = relationship->GetStrength();
        info.m_relationStrengthDirection = relationship->GetStrengthDirection();
        info.m_isRelationship            = true; //! relationship->GetIsExplicit(); LINKER ERROR
        }

    //save to db
    return ECDbSchemaPersistence::InsertECClassInfo (m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::CreateBaseClassEntry (ECClassId ecClassId, ECClassCR baseClass, int index)
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
BeSQLite::DbResult ECDbSchemaWriter::CreateECPropertyEntry (ECPropertyCR ecProperty, ECPropertyId ecPropertyId, ECClassId ecClassId, int32_t index)
    {
    DbECPropertyInfo info;
    info.ColsInsert =
        DbECPropertyInfo::COL_Id           |
        DbECPropertyInfo::COL_ClassId    |
        DbECPropertyInfo::COL_IsArray      |
        DbECPropertyInfo::COL_Description  |
        DbECPropertyInfo::COL_Ordinal      |
        DbECPropertyInfo::COL_IsReadOnly   |
        DbECPropertyInfo::COL_Name;

    info.m_ecClassId    = ecClassId;
    info.m_ecPropertyId = ecPropertyId;
    info.m_isArray      = ecProperty.GetIsArray();
    info.m_ordinal      = index;
    info.m_isReadOnly   = ecProperty.GetIsReadOnly();
    BeStringUtilities::WCharToUtf8 (info.m_name, ecProperty.GetName().c_str());
    BeStringUtilities::WCharToUtf8 (info.m_description, ecProperty.GetDescription().c_str());
    if (ecProperty.GetIsDisplayLabelDefined())
        {
        info.ColsInsert |= DbECPropertyInfo::COL_DisplayLabel;
        BeStringUtilities::WCharToUtf8 (info.m_displayLabel, ecProperty.GetDisplayLabel().c_str());
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
        info.ColsInsert |= DbECPropertyInfo::COL_MinOccurs;
        info.ColsInsert |= DbECPropertyInfo::COL_MaxOccurs;

        if (arrayProperty->GetKind() == ARRAYKIND_Primitive)
            {
            info.ColsInsert |= DbECPropertyInfo::COL_PrimitiveType;
            info.m_primitiveType = arrayProperty->GetPrimitiveElementType();            
            }
        else // ARRAYKIND_Struct
            {
            info.ColsInsert |= DbECPropertyInfo::COL_StructType;
            info.m_structType = arrayProperty->GetStructElementType()->GetId();
            }

        info.m_minOccurs = arrayProperty->GetMinOccurs ();
        //until the max occurs bug in ECObjects (where GetMaxOccurs always returns "unbounded")
        //has been fixed, we need to call GetStoredMaxOccurs to retrieve the proper max occurs
        info.m_maxOccurs = arrayProperty->GetStoredMaxOccurs ();
        }

    return ECDbSchemaPersistence::InsertECPropertyInfo (m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::CreateECRelationConstraintEntry (ECClassId ecClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    DbECRelationshipConstraintInfo info;

    info.ColsInsert =
        DbECRelationshipConstraintInfo::COL_ClassId             |
        DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit |
        DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit |
        DbECRelationshipConstraintInfo::COL_RelationshipEnd     |
        DbECRelationshipConstraintInfo::COL_IsPolymorphic;

    info.m_ecClassId             = ecClassId;
    info.m_ecRelationshipEnd  = endpoint;
    info.m_cardinalityLowerLimit = relationshipConstraint.GetCardinality().GetLowerLimit();
    info.m_cardinalityUpperLimit = relationshipConstraint.GetCardinality().GetUpperLimit();
    info.m_isPolymorphic         = relationshipConstraint.GetIsPolymorphic();

    if (relationshipConstraint.IsRoleLabelDefined())
        {
        BeStringUtilities::WCharToUtf8 (info.m_roleLabel, relationshipConstraint.GetRoleLabel().c_str());
        info.ColsInsert |= DbECRelationshipConstraintInfo::COL_RoleLabel;
        }
    //save to db
    return ECDbSchemaPersistence::InsertECRelationConstraintInfo (m_ecdb, info);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BeSQLite::DbResult ECDbSchemaWriter::InsertCAEntry (IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, ECContainerId overridenContainerId, int index)
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
        BentleyStatus stat = insertInfo.SerializeCaInstance (*customAttribute);
        POSTCONDITION (stat == SUCCESS, BE_SQLITE_ERROR);
        insertInfo.ColsInsert |=DbCustomAttributeInfo::COL_Instance;
        }

    return ECDbSchemaPersistence::InsertCustomAttributeInfo (m_ecdb, insertInfo);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::CreateECRelationshipConstraintClassEntry (ECClassId ecClassId, ECClassId constraintClassId, ECRelationshipEnd endpoint)
    {
    DbECRelationshipConstraintClassInfo info;
    info.ColsInsert =
        DbECRelationshipConstraintClassInfo::COL_ClassId           |
        DbECRelationshipConstraintClassInfo::COL_RelationClassId   |
        DbECRelationshipConstraintClassInfo::COL_RelationshipEnd;
    info.m_ecClassId            = ecClassId;
    info.m_relationECClassId    = constraintClassId;
    info.m_ecRelationshipEnd    = endpoint;
    return ECDbSchemaPersistence::InsertECRelationConstraintClassInfo (m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::Import (ECSchemaCR ecSchema)
    {
    BeMutexHolder aGuard (m_aCriticalSection);
    StopWatch timer ("", true);
    ECSchemaId ecSchemaId = ECDbSchemaPersistence::GetECSchemaId(m_ecdb, ecSchema);
    if (0 != ecSchemaId)
        {
        LOG.warningv(L"Did not import ECSchema %ls (it already exists in the ECDb). We should have checked earlier, that it already exists", ecSchema.GetFullSchemaName().c_str());
        return BE_SQLITE_OK;
        }

    DbResult r = ImportECSchema (ecSchema); 
    if (r != BE_SQLITE_DONE)
        {
        Utf8CP msg = m_ecdb.GetLastError();
        LOG.errorv(L"ECSchema import failed for %ls (Error: %ls)", ecSchema.GetFullSchemaName().c_str(), WString(msg, true).c_str());
        BeAssert(false);
        return r;
        }

    timer.Stop();
    LOG.infov("Imported (in %.2f seconds) ECSchema %s into %s", timer.GetElapsedSeconds(), Utf8String (ecSchema.GetFullSchemaName().c_str()).c_str (), m_ecdb.GetDbFileName());
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::ImportECSchema (ECN::ECSchemaCR ecSchema)
    {
    ECSchemaId ecSchemaId = ECDbSchemaPersistence::GetECSchemaId(m_ecdb, ecSchema);
    if (0 != ecSchemaId)
        return BE_SQLITE_DONE;

    // GenerateId
    BeRepositoryBasedId nextId;
    DbResult status = m_ecdb.GetECDbImplR().GetECSchemaIdSequence ().GetNextValue (nextId);
    POSTCONDITION (status == BE_SQLITE_OK, status);
    ecSchemaId = nextId.GetValue ();
    const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);

    DbResult r = CreateECSchemaEntry(ecSchema, ecSchemaId);
    POSTCONDITION (r == BE_SQLITE_DONE, r);

    ECSchemaReferenceListCR referencedSchemas = ecSchema.GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaCP reference = iter->second.get();
        ECSchemaId referenceId = ECDbSchemaPersistence::GetECSchemaId (m_ecdb, *reference); 
        POSTCONDITION (0 != referenceId && "BuildDependencyOrderedSchemaList used by caller should have ensured that all references are already imported", BE_SQLITE_ERROR); 

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
            r = CreateECSchemaReferenceEntry (ecSchemaId, referenceId);
            POSTCONDITION (r == BE_SQLITE_DONE, r);
            }
        }

    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        r = ImportECClass(*ecClass);
        if (r != BE_SQLITE_DONE)
            {
            LOG.errorv(L"Failed to import ECClass '%ls'.", ecClass->GetFullName());
            BeDataAssert(false);
            return r;
            }
        }

    return ImportCustomAttributes(ecSchema, ecSchemaId, ECONTAINERTYPE_Schema);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::CreateECSchemaReferenceEntry(ECSchemaId ecSchemaId, ECSchemaId ecReferencedSchemaId)
    {
    DbECSchemaReferenceInfo  info;
    info.ColsInsert          = DbECSchemaReferenceInfo::COL_SchemaId | DbECSchemaReferenceInfo::COL_ReferencedSchemaId ;
    info.m_ecSchemaId          = ecSchemaId;
    info.m_referencedECSchemaId = ecReferencedSchemaId;

    return ECDbSchemaPersistence::InsertECSchemaReferenceInfo(m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::ImportCustomAttributes (IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, WCharCP onlyImportCAWithClassName)
    {
    DbResult r;
    r = ImportECCustomAttributeECClass(sourceContainer); 
    if (r != BE_SQLITE_DONE)
        return r;

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

        r = InsertCAEntry (customAttribute, customAttributeClassId, sourceContainerId, containerType, 0 /*Overriden continer id is nullptr=0*/, index++);
        if (r != BE_SQLITE_DONE)
            return r;
        }

    return BE_SQLITE_DONE;
     
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::EnsureECSchemaExists (ECClassCR ecClass)
    {
    ECSchemaId ecSchemaId = ecClass.GetSchema().GetId();
    if (ECDbSchemaPersistence::ContainsECSchemaWithId(m_ecdb, ecSchemaId))
        return BE_SQLITE_DONE;
    BeAssert(false && "I think we just should assume that the entry already exists, rather than relying on just-in-time? Or is this for when we branch off into related ECClasses?");
    //Add ECSchema entry but do not traverse its ECClasses.
    DbResult r = CreateECSchemaEntry(ecClass.GetSchema(), ecSchemaId);
    if (r != BE_SQLITE_DONE)
        return r;

    return ImportECCustomAttributeECClass(ecClass.GetSchema());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::Update (ECDiffR diff, ECDbSchemaReaderR schemaReader, ECDbMapR ecdbMap)
    {
    LOG.errorv("Schema update not supported yet in this version of ECDb.");
    return BE_SQLITE_ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::ImportECClass (ECN::ECClassCR ecClass)
    {
    DbResult r;
    
    if (ECDbSchemaPersistence::ContainsECClass(m_ecdb, ecClass))
        {
        if (!ecClass.HasId())
            ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (m_ecdb, ecClass); //Callers will assume it has a valid Id

        return BE_SQLITE_DONE;
        }

    // GenerateId
    BeRepositoryBasedId nextId;
    DbResult status = m_ecdb.GetECDbImplR().GetECClassIdSequence().GetNextValue (nextId);
    POSTCONDITION (status == BE_SQLITE_OK, status);
    ECClassId ecClassId = nextId.GetValue ();
    const_cast<ECClassR>(ecClass).SetId (ecClassId);

    EnsureECSchemaExists(ecClass);

    r = CreateECClassEntry(ecClass, ecClassId);
    if (r != BE_SQLITE_DONE)
        return r;

    //Import All baseCases
    int baseClassIndex = 0;
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        r = ImportECClass (*baseClass);
        if (r != BE_SQLITE_DONE)
            {
            LOG.errorv("Error importing %s: %s", Utf8String (baseClass->GetFullName()).c_str (), m_ecdb.GetLastError());
            return r;
            }

        r = CreateBaseClassEntry (ecClassId, *baseClass, baseClassIndex++);
        if (r != BE_SQLITE_DONE)
            return r;
        }

    int propertyIndex = 0;
    for (ECPropertyCP ecProperty: ecClass.GetProperties(false))
        {
        r = ImportECProperty(*ecProperty, ecClassId, propertyIndex++);
        if (IsConstraintDbResult(r))
            {
            LOG.errorv ("Attempted to add duplicate value for propety or custom attribute"); 
            BeDataAssert(r != BE_SQLITE_DONE && "Attempted to add duplicate value for propety or custom attribute");
            if (r != BE_SQLITE_DONE)
                return r;
            }
        }

    ECN::ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
    if (relationship != nullptr)
        {
        r = ImportECRelationshipClass (relationship, ecClassId);
        if (r != BE_SQLITE_DONE)
            return r;
        }
    return ImportCustomAttributes(ecClass, ecClassId, ECONTAINERTYPE_Class);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::ImportECRelationshipClass (ECN::ECRelationshipClassCP relationship, ECClassId ecClassId)
    {
    DbResult r;
    r = ImportECRelationshipConstraint(ecClassId, relationship->GetSource(), ECRelationshipEnd_Source);
    if (r != BE_SQLITE_DONE)
        return r;
    return  ImportECRelationshipConstraint(ecClassId, relationship->GetTarget(), ECRelationshipEnd_Target);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::ImportECRelationshipConstraint (ECClassId ecClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint)
    {
    DbResult r = CreateECRelationConstraintEntry(ecClassId, relationshipConstraint, endpoint);
    if (r != BE_SQLITE_DONE)
        return r;

    for (ECClassCP ecClass : relationshipConstraint.GetClasses())
        {
        r = ImportECClass(*ecClass);
        if (r != BE_SQLITE_DONE)
            return r;

        r = CreateECRelationshipConstraintClassEntry (ecClassId, ecClass->GetId(), endpoint);

        if (r != BE_SQLITE_DONE)
            return r;
        }
    for (auto ecClass : relationshipConstraint.GetConstraintClasses())
        {
        for (auto key : ecClass->GetKeys())
            {
            DbECRelationshipConstraintClassPropertyInfo propertyInfo;
            propertyInfo.ColsInsert =
                DbECRelationshipConstraintClassInfo::COL_ClassId |
                DbECRelationshipConstraintClassInfo::COL_RelationClassId |
                DbECRelationshipConstraintClassInfo::COL_RelationshipEnd;
            propertyInfo.m_ecClassId = ecClassId;
            propertyInfo.m_relationECClassId = ecClass->GetClass().GetId();
            propertyInfo.m_ecRelationshipEnd = endpoint;
            auto keyProperty = ecClass->GetClass().GetPropertyP(key.c_str());
            if (keyProperty == nullptr)
                {
                LOG.warningv(L"%ls ECProperty not found in %ls class", key.c_str(), ecClass->GetClass().GetName().c_str());
                continue;
                }
            propertyInfo.m_relationECPropertyId = keyProperty->GetId();
            r = ECDbSchemaPersistence::InsertECRelationConstraintClassPropertyInfo(m_ecdb, propertyInfo);
            if (r != BE_SQLITE_DONE)
                return r;
            }
        }
    ECContainerType containerType = endpoint == ECRelationshipEnd_Source ? ECONTAINERTYPE_RelationshipConstraintSource: ECONTAINERTYPE_RelationshipConstraintTarget;
    return ImportCustomAttributes (relationshipConstraint, ecClassId, containerType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::ImportECProperty (ECN::ECPropertyCR ecProperty, ECClassId ecClassId, int32_t index)
    {
    DbResult r;
    
    // GenerateId
    BeRepositoryBasedId nextId;
    r = m_ecdb.GetECDbImplR().GetECPropertyIdSequence().GetNextValue (nextId);
    POSTCONDITION (r == BE_SQLITE_OK, r);
    ECPropertyId ecPropertyId = nextId.GetValue ();
    const_cast<ECPropertyR>(ecProperty).SetId (ecPropertyId);

    if (ecProperty.GetIsStruct())
        {
        r = ImportECClass(ecProperty.GetAsStructProperty()->GetType());
        if (r != BE_SQLITE_DONE)
            return r;
        }
    else if (ecProperty.GetIsArray())
        {
        ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
        if (arrayProperty->GetKind() == ARRAYKIND_Struct)
            {
            r = ImportECClass(*arrayProperty->GetStructElementType());
            if (r != BE_SQLITE_DONE)
                return r;
            }
        }

    r = CreateECPropertyEntry(ecProperty, ecPropertyId, ecClassId, index);
    if (r != BE_SQLITE_DONE)
        return r;

    return ImportCustomAttributes(ecProperty, ecPropertyId, ECONTAINERTYPE_Property);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::ImportECCustomAttributeECClass (ECN::IECCustomAttributeContainerCR caContainer)
    {
    DbResult r;
    for (IECInstancePtr ca : caContainer.GetCustomAttributes(false))
        {
        r = ImportECClass (ca->GetClass());
        if ( r != BE_SQLITE_DONE)
            return r;
        }
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaWriterPtr ECDbSchemaWriter::Create (ECDbR ecdb)
    {
    return new ECDbSchemaWriter(ecdb);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
