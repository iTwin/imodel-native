/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
    stmt.Prepare(m_ecdb,"SELECT Name FROM ec_Schema WHERE NameSpacePrefix = ?");
    stmt.BindText(1, currentNSPrefix, Statement::MAKE_COPY_No);
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
            stmt.BindText(1, newPrefix, Statement::MAKE_COPY_No);
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
        DbECSchemaInfo::COL_ECSchemaId      |
        DbECSchemaInfo::COL_Name            |
        DbECSchemaInfo::COL_VersionMajor    |
        DbECSchemaInfo::COL_VersionMinor    |
        DbECSchemaInfo::COL_Description     |
        DbECSchemaInfo::COL_NamespacePrefix |
        DbECSchemaInfo::COL_SchemaType;
    //ecSchema.IsSupplemented
    info.m_ecSchemaId   = ecSchemaId;
    info.m_versionMajor = ecSchema.GetVersionMajor();
    info.m_versionMinor = ecSchema.GetVersionMinor();

    info.m_schemaType = PERSISTEDSCHEMATYPE_Primary;
    if (ecSchema.IsStandardSchema())
        info.m_schemaType = PERSISTEDSCHEMATYPE_Standard;
    else if (ecSchema.IsSupplemented())
        info.m_schemaType = PERSISTEDSCHEMATYPE_Supplemented;
    else
        {
         SupplementalSchemaMetaDataPtr supplementalSchemaMetaData;
         if (ECOBJECTS_STATUS_Success == SupplementalSchemaMetaData::TryGetFromSchema(supplementalSchemaMetaData, ecSchema))
             if (supplementalSchemaMetaData.IsValid())
                info.m_schemaType = PERSISTEDSCHEMATYPE_Supplemental;
        }

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
        DbECClassInfo::COL_ECClassId         |
        DbECClassInfo::COL_ECSchemaId        |
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
        DbBaseClassInfo::COL_ECClassId     |
        DbBaseClassInfo::COL_BaseECClassId |
        DbBaseClassInfo::COL_ECIndex;

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
        DbECPropertyInfo::COL_ECClassId    |
        DbECPropertyInfo::COL_ECPropertyId |
        DbECPropertyInfo::COL_IsArray      |
        DbECPropertyInfo::COL_Description  |
        DbECPropertyInfo::COL_ECIndex      |
        DbECPropertyInfo::COL_IsReadOnly   |
        DbECPropertyInfo::COL_Name;

    info.m_ecClassId    = ecClassId;
    info.m_ecPropertyId = ecPropertyId;
    info.m_isArray      = ecProperty.GetIsArray();
    info.m_ecIndex      = index;
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
        info.ColsInsert |= DbECPropertyInfo::COL_TypeECPrimitive;
        info.m_typeECPrimitive = ecProperty.GetAsPrimitiveProperty()->GetType();            
        }
    else if( ecProperty.GetIsStruct())
        {
        info.ColsInsert |= DbECPropertyInfo::COL_TypeECStruct;
        info.m_typeECStruct = ecProperty.GetAsStructProperty()->GetType().GetId();
        }
    else if( ecProperty.GetIsArray())
        {
        ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
        info.ColsInsert |= DbECPropertyInfo::COL_MinOccurs;
        info.ColsInsert |= DbECPropertyInfo::COL_MaxOccurs;

        if (arrayProperty->GetKind() == ARRAYKIND_Primitive)
            {
            info.ColsInsert |= DbECPropertyInfo::COL_TypeECPrimitive;
            info.m_typeECPrimitive = arrayProperty->GetPrimitiveElementType();            
            }
        else // ARRAYKIND_Struct
            {
            info.ColsInsert |= DbECPropertyInfo::COL_TypeECStruct;
            info.m_typeECStruct = arrayProperty->GetStructElementType()->GetId();
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
        DbECRelationshipConstraintInfo::COL_ECClassId             |
        DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit |
        DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit |
        DbECRelationshipConstraintInfo::COL_ECRelationshipEnd     |
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
    DbCustomAttributeInfo insertInfo;
    insertInfo.ColsInsert =
        DbCustomAttributeInfo::COL_ContainerId |
        DbCustomAttributeInfo::COL_ContainerType |
        DbCustomAttributeInfo::COL_ECClassId |
        DbCustomAttributeInfo::COL_Index;


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

    if (overridenContainerId != 0 )
        {
        insertInfo.m_overridenByContainerId = overridenContainerId;
        insertInfo.ColsInsert |=DbCustomAttributeInfo::COL_OverridenByContainerId;
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
        DbECRelationshipConstraintClassInfo::COL_ECClassId           |
        DbECRelationshipConstraintClassInfo::COL_RelationECClassId   |
        DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd;
    info.m_ecClassId            = ecClassId;
    info.m_relationECClassId    = constraintClassId;
    info.m_ecRelationshipEnd    = endpoint;

    return ECDbSchemaPersistence::InsertECRelationConstraintClassInfo (m_ecdb, info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaWriter::ECDbSchemaWriter (ECDbR ecdb) : m_ecdb(ecdb){}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::Import (ECSchemaCR ecSchema, CustomAttributeTrackerP tracker)
    {
    BeCriticalSectionHolder aGuard (m_aCriticalSection);
    StopWatch timer ("", true);
    ECSchemaId ecSchemaId = ECDbSchemaPersistence::GetECSchemaId(m_ecdb, ecSchema);
    if (0 != ecSchemaId)
        {
        LOG.warningv(L"Did not import ECSchema %ls (it already exists in the ECDb). We should have checked earlier, that it already exists", ecSchema.GetFullSchemaName().c_str());
        return BE_SQLITE_OK;
        }

    m_customAttributeTracker = tracker;
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
    DbResult status = m_ecdb.GetImplR ().GetECSchemaIdSequence ().GetNextValue (nextId);
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
BeSQLite::DbResult ECDbSchemaWriter::CreateECSchemaReferenceEntry(ECSchemaId ecSchemaId, ECSchemaId ecReferenceSchemaId)
    {
    DbECSchemaReferenceInfo  info;
    info.ColsInsert          = DbECSchemaReferenceInfo::COL_ECSchemaId | DbECSchemaReferenceInfo::COL_ReferenceECSchemaId ;
    info.m_ecSchemaId          = ecSchemaId;
    info.m_referenceECSchemaId = ecReferenceSchemaId;

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
//  1. Lossy saving of supplemented schema
//     ===================================
    if (m_customAttributeTracker == nullptr)
        {
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

//  2. Lossless saving of supplemented schema
//     ======================================
    //1. Primary attribute comes first and followed by consolidated while enumerating GetCustomAttributes()
    //2. So in customAttributeMap the first value would be primary instance and second would be consolidated
    //3. If it only has one, then either it was primary and not overridden, or it didn’t exist on the primary and was cleanly supplemented (with no real override). 
    //4. We need the pair of consolidated and primary container id at same time so we can use just one insertion to put both.
    //5. Supplemental schema must already exist for above to work
    for ( ; itor != customAttributeMap.end(); ++itor)
        {
        bvector<IECInstanceP> const& customAttributes = itor->second;
        IECInstanceP consolidatedCustomAttribute = nullptr;
        IECInstanceP primaryCustomAttribute = nullptr;
        IECCustomAttributeContainerCP consolidatedContainer = nullptr;
        IECCustomAttributeContainerCP primaryContainer = &sourceContainer;
        ECClassCP     customAttributeClass = itor->first;

        if (customAttributes.size() == 1) // Either primary or supplemental CA.
            {
            IECInstanceP customAttribute = customAttributes[0];
            IECCustomAttributeContainerCP container;
            if (m_customAttributeTracker->TryGetContainer(container, *customAttribute))
                {
                if (container == primaryContainer)
                    primaryCustomAttribute = customAttribute;
                else
                    consolidatedContainer = container;
                }
            }
        else if (customAttributes.size() == 2)// Supplemental CA override Primary CA so both exists
            {
            primaryCustomAttribute = customAttributes[0];
            consolidatedCustomAttribute = customAttributes[1];
            if (!m_customAttributeTracker->TryGetContainer(consolidatedContainer, *consolidatedCustomAttribute))
                {
                LOG.error("Failed to determine consolidated container id");
                return BE_SQLITE_ERROR;
                }
           BeAssert(consolidatedContainer != primaryContainer);
#ifndef NDEBUG
            // The first custom attribute has to be from primary
            if (!m_customAttributeTracker->TryGetContainer(primaryContainer, *primaryCustomAttribute))
                {
                //Error
                LOG.error("Failed to determine  primary container id");
                BeAssert(false && "Fail to determine primary container id");
                return BE_SQLITE_ERROR;
                }
            BeAssert( primaryContainer == &sourceContainer); // check to ensure it always primary
#endif
            }
        else
            {
            BeAssert(false && "Something wrong with SupplementedSchemaBuilder. We must either get one or two custom attributes"); 
            }

        ECClassId customAttributeClassId;
        if (customAttributeClass->HasId())
            customAttributeClassId = customAttributeClass->GetId();
        else
            customAttributeClassId = ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (m_ecdb, *customAttributeClass);
        BeAssert(customAttributeClassId != 0);

        int64_t primaryContainerId;
        int64_t consolidatedContainerId;
        switch (containerType)
            {
            case ECONTAINERTYPE_Schema:
                BeAssert (dynamic_cast<ECSchemaCP>(primaryContainer));
                primaryContainerId = static_cast<ECSchemaCP>(primaryContainer)->GetId();
                consolidatedContainerId = consolidatedContainer ? static_cast<ECSchemaCP>(consolidatedContainer)->GetId() : 0;
                break;
            case ECONTAINERTYPE_Class:
            case ECONTAINERTYPE_RelationshipConstraintSource:
            case ECONTAINERTYPE_RelationshipConstraintTarget:
                BeAssert (dynamic_cast<ECClassCP>(primaryContainer));
                primaryContainerId = static_cast<ECClassCP>(primaryContainer)->GetId();
                consolidatedContainerId = consolidatedContainer ? static_cast<ECClassCP>(consolidatedContainer)->GetId() : 0;
                break;
            case ECONTAINERTYPE_Property:
                BeAssert (dynamic_cast<ECPropertyCP>(primaryContainer));
                primaryContainerId = static_cast<ECPropertyCP>(primaryContainer)->GetId();
                consolidatedContainerId = consolidatedContainer ? static_cast<ECPropertyCP>(consolidatedContainer)->GetId() : 0;
                break;
            default:
                BeAssert (false);
                return BE_SQLITE_ERROR;
            }

        r = InsertCAEntry (primaryCustomAttribute, customAttributeClassId, primaryContainerId, containerType, consolidatedContainerId, index++);
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
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateCustomAttribute (IECInstanceCR ecInstance, IECCustomAttributeContainerCR container, ECContainerId sourceContainerId, ECContainerType containerType)
    {
    if (!ecInstance.GetClass().HasId())
        ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (m_ecdb, ecInstance.GetClass()); //Callers will assume it has a valid Id

    if (ECDbSchemaPersistence::IsCustomAttributeDefined(m_ecdb, ecInstance.GetClass().GetId(), sourceContainerId, containerType))
        {
        DbCustomAttributeInfo info;
        info.ColsUpdate = DbCustomAttributeInfo::COL_Instance;
        info.SerializeCaInstance(const_cast<IECInstanceR>(ecInstance));
        info.ColsWhere = DbCustomAttributeInfo::COL_ContainerId | DbCustomAttributeInfo::COL_ContainerType | DbCustomAttributeInfo::COL_ECClassId;
        info.m_containerId = sourceContainerId;
        info.m_containerType = containerType;
        info.m_ecClassId = ecInstance.GetClass().GetId();
        return ECDbSchemaPersistence::UpdateCustomAttributeInfo(m_ecdb, info);
        }
    
    auto r = ImportCustomAttributes(container, sourceContainerId, containerType, ecInstance.GetClass().GetName().c_str());
    if (r != BE_SQLITE_DONE)
        {
        LOG.error(L"Failed while importing customAttribute");
        return r;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaWriter::DiffExist (IECDiffNodeCR diff, WCharCP name)
    {
    auto child = diff.GetChildByAccessString (name);
    if (child != nullptr)
        {
        return child->GetDiffType () != DIFFTYPE_Empty && child->GetDiffType () != DIFFTYPE_Left;
        }

    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateCustomAttributes 
    (
    IECCustomAttributeContainerCR sourceContainer, 
    IECDiffNodeCR customAttributesDN,
    ECContainerId sourceContainerId, 
    ECContainerType containerType
    )
    {
    DbResult r = BE_SQLITE_DONE;
    for (auto customAttributeDN :customAttributesDN.GetChildren())
        {
        //The name of customAttribute is FullName() for the class which contain schema name as well;
        auto  n = customAttributeDN->GetName().find(L":");
        auto caSchemaName = customAttributeDN->GetName().substr(0, n);
        auto caClassName = customAttributeDN->GetName().substr(n + 1);
        //auto caSchemName = customAttributeDN->GetName().substr(0, n - 1);

        auto  type = customAttributeDN->GetDiffType(true); 
        switch(type)
            {
            case DIFFTYPE_Conflict: //Property is conflicting 
            case DIFFTYPE_Right:    //Property added in new schema
            case DIFFTYPE_Left:
            case DIFFTYPE_Empty:    //Not used
                {
                auto ca = sourceContainer.GetCustomAttributeLocal (caClassName.c_str ());
                if (caClassName.Equals (L"ECDbClassHint") ||
                    caClassName.Equals (L"ECDbPropertyHint") ||
                    caClassName.Equals (L"ECDbRelationshipClassHint") ||
                    caClassName.Equals (L"ECDbSchemaHint"))
                    {
                    if (DiffExist (*customAttributeDN, L"MapStrategy")
                        || DiffExist (*customAttributeDN, L"Indexes")
                        || DiffExist (*customAttributeDN, L"PreferredDirection")
                        || DiffExist (*customAttributeDN, L"AllowDuplicateRelationships")
                        || DiffExist (*customAttributeDN, L"IsNullable")
                        || DiffExist (*customAttributeDN, L"IsUnique")
                        || DiffExist (*customAttributeDN, L"TablePrefix")
                        || DiffExist (*customAttributeDN, L"NamedGroupIsAssembly"))
                        {
                        LOG.warningv (L"Updating ECDb mapping hints failed. One of the property changed is not allowed to update. Skipping '%ls'", caClassName.c_str ());
                        continue;
                        }
                    }

                if (ca.IsNull())
                    {
                    auto caClassId = ECDbSchemaPersistence::GetECClassIdBySchemaName(m_ecdb, Utf8String(caSchemaName.c_str()).c_str(), Utf8String(caClassName.c_str()).c_str());
                    if (caClassId != 0)
                        {
                        r = ECDbSchemaPersistence::DeleteCustomAttribute(sourceContainerId, containerType, caClassId, m_ecdb);
                        if (r != BE_SQLITE_DONE)
                            {
                            LOG.errorv(L"Failed to delete existing custom attribute with class name '%ls' deleted", caClassName.c_str());
                            return r;
                            }
                        return BE_SQLITE_DONE;
                        }
                    else 
                        {
                        BeDataAssert(caClassId != 0 && "Failed to delete existing custom attribute");
                        LOG.errorv(L"Failed to delete existing custom attribute with class name '%ls' deleted", caClassName.c_str());
                        return r;
                        }
                    }
                else
                    {
                    //We do not support replacing mapping hints
                    r = UpdateCustomAttribute (*ca, sourceContainer, sourceContainerId, containerType);
                    if (r != BE_SQLITE_DONE)
                        {
                        LOG.errorv(L"Failed while updating customAttribute %ls", caClassName.c_str());
                        return r;
                        }

                    if (caClassName.Equals (L"ECDbClassHint") && containerType == ECContainerType::ECONTAINERTYPE_Class)
                        {                  
                        if (auto tableName = customAttributeDN->GetChildByAccessString (L"TableName"))
                            {
                            Statement stmt;
                            stmt.Prepare (m_ecdb, "UPDATE ec_ClassMap SET MapToDbTable =?  WHERE ECClassId = ?");
                            if (!tableName->GetValueRight ().IsNull ())
                                stmt.BindText (1, tableName->GetValueRight ().GetUtf8CP (), Statement::BindMakeCopy::MAKE_COPY_Yes);

                            stmt.BindInt64 (2, sourceContainerId);
                            if ((r = stmt.Step ()) != BE_SQLITE_DONE)
                                {
                                LOG.error (L"Failed while updating TableName");
                                return r;
                                }
                            }

                        if (auto ecInstanceIdColumn = customAttributeDN->GetChildByAccessString (L"ECIdColumn"))
                            {
                            Statement stmt;
                            stmt.Prepare (m_ecdb, "UPDATE ec_ClassMap SET PrimaryKeyColumnName =?  WHERE ECClassId = ?");
                            if (!ecInstanceIdColumn->GetValueRight ().IsNull ())
                                stmt.BindText (1, ecInstanceIdColumn->GetValueRight ().GetUtf8CP (), Statement::BindMakeCopy::MAKE_COPY_Yes);
                            
                            stmt.BindInt64 (2, sourceContainerId);
                            if ((r = stmt.Step ()) != BE_SQLITE_DONE)
                                {
                                LOG.error (L"Failed while updating PrimaryKeyColumnName");
                                return r;
                                }
                            }                            
                        }
                    }

                break;
                }
            case DIFFTYPE_Equal:    //Not used
                break;
            }
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateECProperty (ECN::ECPropertyCR ecProperty, ECN::IECDiffNodeCR propertyDN, ECClassId ecClassId)
    {
    DbResult r = BE_SQLITE_DONE;

    if (!ecProperty.HasId())
        ECDbSchemaManager::GetPropertyIdForECPropertyFromDuplicateECSchema (m_ecdb, ecProperty); //Callers will assume it has a valid Id

    if (!ecProperty.HasId())
        {
        int count = (int)ecProperty.GetClass().GetPropertyCount(false);
        r = ImportECProperty (ecProperty, ecClassId, count);
        if (r != BE_SQLITE_DONE)
            {
            LOG.errorv(L"Failed while updating property %ls.%ls", ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
            return r;
            }
        return r;
        }

    if (propertyDN.GetChildById(DiffNodeId::IsOverriden))
        {
        if (ecProperty.GetBaseProperty() != nullptr)
            {
            LOG.errorv(L"Property '%ls.%ls' cannot be un-overridden", ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
            return BE_SQLITE_ERROR;
            }
        }

    if (propertyDN.GetChildById(DiffNodeId::IsArray))
        {
        LOG.errorv(L"Updating 'IsArray' attribute of ECProperty '%ls.%ls' is not supported", ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
        return BE_SQLITE_ERROR;
        }
    if (propertyDN.GetChildById(DiffNodeId::TypeName))
        {
        LOG.errorv(L"Updating 'TypeName' attribute of ECProperty '%ls.%ls' is not supported", ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
        return BE_SQLITE_ERROR;
        }
    if (propertyDN.GetChildById(DiffNodeId::IsPrimitive))
        {
        LOG.errorv(L"Updating 'IsPrimitive' attribute of ECProperty '%ls.%ls' is not supported", ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
        return BE_SQLITE_ERROR;
        }
    if (propertyDN.GetChildById(DiffNodeId::IsReadOnly))
        {
        LOG.errorv(L"Updating 'IsReadOnly' attribute of ECProperty '%ls.%ls' is not supported", ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
        return BE_SQLITE_ERROR;
        }

    //Following are changes that we do support in a property
    DbECPropertyInfo info;
    info.ColsWhere = DbECPropertyInfo::COL_ECPropertyId;
    info.m_ecPropertyId = ecProperty.GetId();
 
    if (auto n = propertyDN.GetChildById(DiffNodeId::DisplayLabel))
        {
        info.ColsUpdate |= DbECPropertyInfo::COL_DisplayLabel;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECPropertyInfo::COL_DisplayLabel;
        else
            info.m_displayLabel = Utf8String(n->GetValueRight().GetString());
        }
    if (auto n = propertyDN.GetChildById(DiffNodeId::Description))
        {
        info.ColsUpdate |= DbECPropertyInfo::COL_Description;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECPropertyInfo::COL_Description;
        else
            info.m_description = Utf8String(n->GetValueRight().GetString());
        }
    if (info.ColsUpdate != 0)
        {
        if ((r = ECDbSchemaPersistence::UpdateECPropertyInfo(m_ecdb, info))!= BE_SQLITE_DONE)
            {
            LOG.errorv(L"Failed to update ECProperty '%ls.%ls' attributes", ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
            return r;
            }
        }
    // Update/Add CustomAttributes (if any)
    if (auto customAttributesDN = propertyDN.GetChildById (DiffNodeId::CustomAttributes))
        if ((r = UpdateCustomAttributes(ecProperty, *customAttributesDN, ecProperty.GetId(), ECONTAINERTYPE_Property)) != BE_SQLITE_DONE)
            return r;

    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateECProperties(ECN::ECClassCR ecClass, ECN::IECDiffNodeCR propertiesDN)
    {
    DbResult r = BE_SQLITE_DONE;
    for (auto propertyDN :propertiesDN.GetChildren())
        {
        auto& name = propertyDN->GetName();
        auto  type = propertyDN->GetDiffType(true);
        switch(type)
            {
            case DIFFTYPE_Conflict: //Property is conflicting 
            case DIFFTYPE_Right:    //Property added in new schema
            case DIFFTYPE_Left:
            case DIFFTYPE_Empty:    //Not used
                {
                auto propertyP = ecClass.GetPropertyP(name.c_str(), false);
                if (propertyP == nullptr)
                    {
                    //Delete overriden property.
                    auto schemaName = Utf8String(ecClass.GetSchema().GetName().c_str());
                    auto className = Utf8String(ecClass.GetName().c_str());       
                    auto propertyName = Utf8String (name.c_str());
                    auto propertyId = ECDbSchemaPersistence::GetECPropertyId (m_ecdb, schemaName.c_str(), className.c_str(), propertyName.c_str());
                    BeAssert(propertyId != 0);
                      
                    r = ECDbSchemaPersistence::DeleteECProperty(propertyId, m_ecdb);
                    if (r != DbResult::BE_SQLITE_DONE)
                        {
                        LOG.errorv(L"Failed un-override property %ls.%ls", ecClass.GetFullName(), name.c_str());
                        return r;
                        }  
                    LOG.infov("ECSchemaUpgrade: Deleted property %ls.%ls", ecClass.GetFullName(), name.c_str());
                   }
                else
                    {
                    r = UpdateECProperty (*propertyP, *propertyDN, ecClass.GetId());
                    if (r != BE_SQLITE_DONE)
                        {
                        LOG.errorv(L"Failed while updating property %ls.%ls", ecClass.GetFullName(), name.c_str());
                        return r;
                        }
                    }
                break;
                }
            case DIFFTYPE_Equal:    //Not used
                break;
            }
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateECClass (ECN::ECClassCR ecClass, ECN::IECDiffNodeCR classDN)
    {
    DbResult r = BE_SQLITE_ERROR;
    if (!ecClass.HasId())
        ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (m_ecdb, ecClass); //Callers will assume it has a valid Id

    if(!ecClass.HasId())
        {
        r = ImportECClass (ecClass);
        if (r != BE_SQLITE_DONE)
            {
            LOG.errorv(L"Failed while importing %ls", ecClass.GetFullName());
            BeDataAssert(false);
            return r;
            }
        return r;
        }

    m_updateContext->MarkAsUpdated(&classDN);

    // PRECONDITIONS Following changes in schema is not supported at this time   
    if (auto isDomain = classDN.GetChildById(DiffNodeId::IsDomainClass))
        {
        //If existing class is custom attribute and its domain property is set to true in upgrade schema
        //we accept it. This will cause a table to be created for custom attribute class.
        if (!(isDomain->GetValueRight ().GetBoolean () && ecClass.GetIsCustomAttributeClass ()))
            {
            LOG.errorv (L"Updating 'IsDomain' property of ECClass %ls is not supported", ecClass.GetFullName ());
            return BE_SQLITE_ERROR;
            }
        }

    if (classDN.GetChildById (DiffNodeId::IsCustomAttributeClass))
        {
        LOG.errorv (L"Updating 'IsStruct' property of ECClass %ls is not supported", ecClass.GetFullName ());
        return BE_SQLITE_ERROR;
        }

    if (classDN.GetChildById(DiffNodeId::IsStruct))
        {
        LOG.errorv(L"Updating 'IsStruct' property of ECClass %ls is not supported", ecClass.GetFullName());
        return BE_SQLITE_ERROR;
        }


    if (classDN.GetChildById(DiffNodeId::IsRelationshipClass))
        {
        LOG.errorv(L"Updating 'IsRelationshipClass' property of ECClass %ls is not supported", ecClass.GetFullName());
        return BE_SQLITE_ERROR;
        }

    if (classDN.GetChildById(DiffNodeId::BaseClasses))
        {
        LOG.errorv(L"Updating 'BaseClasses' of ECClass %ls is not supported", ecClass.GetFullName());
        return BE_SQLITE_ERROR;
        }

    if (auto n = classDN.GetChildById(DiffNodeId::RelationshipInfo))
        {
        auto relationshipClass = ecClass.GetRelationshipClassCP();
        BeAssert (relationshipClass != nullptr);
        if (relationshipClass)
            {
            r = UpdateECRelationshipClass(*ecClass.GetRelationshipClassCP(), *n);
            if (r != BE_SQLITE_DONE)
                return r;
            }
        else
            {
            LOG.error(L"Updating 'ECRelationshipClass' has limited support");
            return BE_SQLITE_ERROR;
            }
        }

    //Following are changes that we do support in a class
    DbECClassInfo info;
    info.ColsWhere = DbECClassInfo::COL_ECClassId;
    info.m_ecClassId = ecClass.GetId();
    if (auto n = classDN.GetChildById(DiffNodeId::DisplayLabel))
        {
        info.ColsUpdate |= DbECClassInfo::COL_DisplayLabel;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECClassInfo::COL_DisplayLabel;
        else
            info.m_displayLabel = Utf8String(n->GetValueRight().GetString());
        }

    if (auto n = classDN.GetChildById(DiffNodeId::Description))
        {
        info.ColsUpdate |= DbECClassInfo::COL_Description;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECClassInfo::COL_Description;
        else
            info.m_description = Utf8String(n->GetValueRight().GetString());
        }

    if (auto n = classDN.GetChildById (DiffNodeId::IsDomainClass))
        {
        info.ColsUpdate |= DbECClassInfo::COL_IsDomainClass;
        if (n->GetValueRight ().IsNull ())
            info.ColsNull |= DbECClassInfo::COL_IsDomainClass;
        else
            info.m_isDomainClass = n->GetValueRight ().GetBoolean ();
        }

    if (info.ColsUpdate != 0)
        {
        if ((r = ECDbSchemaPersistence::UpdateECClassInfo(m_ecdb, info))!= BE_SQLITE_DONE)
            {
            LOG.errorv(L"Failed to update ECClass %ls class attributes", ecClass.GetFullName());
            return BE_SQLITE_ERROR;
            }
        }

    // Update/Add ECProperties (if any)
    if (auto propertiesDN = classDN.GetChildById (DiffNodeId::Properties))
        if ((r = UpdateECProperties(ecClass, *propertiesDN)) != BE_SQLITE_DONE)
            return r;

    // Update/Add CustomAttributes (if any)
    if (auto customAttributesDN = classDN.GetChildById (DiffNodeId::CustomAttributes))
        if ((r = UpdateCustomAttributes(ecClass, *customAttributesDN, ecClass.GetId(), ECONTAINERTYPE_Class)) != BE_SQLITE_DONE)
            return r;
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId ECDbSchemaWriter::ResolveClassIdFromUpdateContext(WCharCP schemaName, WCharCP className)
    {
    PRECONDITION(m_updateContext.get() != nullptr && "This funtion must be classed from ECDbSchemaWriter::Update()", 0);
    auto& modifiedSchema = m_updateContext->GetDiff().GetRightSchema();
    if (modifiedSchema.GetName() == schemaName)
        {
        auto classDiff = m_updateContext->GetClassDiff(className);
        if (classDiff && !m_updateContext->IsUpdated(classDiff))
            {
            auto classP = modifiedSchema.GetClassCP(className);
            PRECONDITION (classP != nullptr && "Expected to find class", 0);
            auto r = UpdateECClass(*classP, *classDiff);
            if (r != BE_SQLITE_DONE)
                {
                LOG.errorv(L"Failed to update ECClass %ls.%ls class attributes", schemaName, className);
                return 0;
                }
            m_updateContext->MarkAsUpdated(classDiff);
            }
        }
    auto classId = ECDbSchemaPersistence::GetECClassIdBySchemaName(m_ecdb, Utf8String(schemaName).c_str(), Utf8String(className).c_str());
    BeAssert (classId != 0);
    return classId;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateECRelationshipConstraint (ECN::ECRelationshipClassCR updateRelationshipClass, ECRelationshipEnd relationshipEnd,  ECN::IECDiffNodeCR relationshipConstraint)
    {
    DbResult r;
    ECN::ECRelationshipConstraintCR constraint = 
        relationshipEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? updateRelationshipClass.GetSource() : updateRelationshipClass.GetTarget();
  
    auto constraintDiff =  relationshipConstraint.GetChildById (relationshipEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? DiffNodeId::Source : DiffNodeId::Target);
    if (constraintDiff == nullptr)
        return BE_SQLITE_DONE;


    if (constraintDiff->GetChildById(DiffNodeId::IsPolymorphic))
        {
        LOG.error(L"Updating 'ECRelationshipConstraint' IsPolymorphic attribute is not supported");
        return BE_SQLITE_ERROR;
        }

    if (constraintDiff->GetChildById(DiffNodeId::Cardinality))
        {
        LOG.error(L"Updating 'ECRelationshipConstraint' Cardinality is not supported");
        return BE_SQLITE_ERROR;
        }

    if (auto classesDiff = constraintDiff->GetChildById(DiffNodeId::Classes))
        {
        for (auto classDiff : classesDiff->GetChildren())
            {
            if (classDiff->GetDiffType(true) == DiffType::DIFFTYPE_Left)
                {
                LOG.errorv(L"Relationship constraint class '%ls' cannot be deleted  from relationship '%ls'. Operation not supported.", 
                    updateRelationshipClass.GetName().c_str(), classDiff->GetValueLeft().GetString());

                return BE_SQLITE_ERROR;
                }

            //add relationship constraint.
            WString constraintClassSchemaName, constraintClassName;
            ECDiffValueHelper::TryParseClassKey(constraintClassSchemaName, constraintClassName, classDiff->GetValueRight().GetString());
            auto constraintClassId = ResolveClassIdFromUpdateContext(constraintClassSchemaName.c_str(), constraintClassName.c_str());
            if (constraintClassId == 0)
                {
                BeDataAssert(constraintClassId != 0 && "Failed to add class to relationship constraint");
                LOG.errorv(L"Failed to add class to relationship constraint '%ls'", classDiff->GetValueRight().GetString());
                return BE_SQLITE_ERROR;
                }

            auto r = CreateECRelationshipConstraintClassEntry (updateRelationshipClass.GetId(), constraintClassId, relationshipEnd);
            if (r != BE_SQLITE_DONE)
                return r;
            }
        }

    DbECRelationshipConstraintInfo info;  
    info.m_ecClassId = updateRelationshipClass.GetId();
    info.m_ecRelationshipEnd = relationshipEnd;
    info.ColsWhere = DbECRelationshipConstraintInfo::COL_ECClassId | DbECRelationshipConstraintInfo::COL_ECRelationshipEnd;

    if (auto n = constraintDiff->GetChildById(DiffNodeId::RoleLabel))
        {
        info.ColsUpdate |= DbECRelationshipConstraintInfo::COL_RoleLabel;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECRelationshipConstraintInfo::COL_RoleLabel;
        else
            BeStringUtilities::WCharToUtf8 (info.m_roleLabel, n->GetValueRight().GetString());
        }

    if (info.ColsUpdate != 0)
        {
        DbResult r = ECDbSchemaPersistence::UpdateECRelationConstraintInfo(m_ecdb, info);
        if (r != BE_SQLITE_DONE)
            {
            LOG.errorv(L"Failed to update ECRelationshipConstraint %ls class attributes", updateRelationshipClass.GetFullName());
            return BE_SQLITE_ERROR;
            }
        }

    ECContainerType containerType = relationshipEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? ECONTAINERTYPE_RelationshipConstraintSource : ECONTAINERTYPE_RelationshipConstraintTarget;
    if (auto customAttributesDN = relationshipConstraint.GetChildById (DiffNodeId::CustomAttributes))
        if ((r = UpdateCustomAttributes(constraint, *customAttributesDN, updateRelationshipClass.GetId(), containerType)) != BE_SQLITE_DONE)
            return r;

    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateECRelationshipClass (ECN::ECRelationshipClassCR updateRelationshipClass, ECN::IECDiffNodeCR relationshipInfo)
    {
    if (relationshipInfo.GetChildById(DiffNodeId::Source))
        {
        auto r = UpdateECRelationshipConstraint(updateRelationshipClass, ECRelationshipEnd_Source, relationshipInfo);
        if (r != BE_SQLITE_DONE)
            return r;
        }

    if (relationshipInfo.GetChildById(DiffNodeId::Target))
        {
        auto r = UpdateECRelationshipConstraint(updateRelationshipClass, ECRelationshipEnd_Target, relationshipInfo);
        if (r != BE_SQLITE_DONE)
            return r;
        }

    DbECClassInfo info;
    info.ColsWhere = DbECClassInfo::COL_ECClassId;
    info.m_ecClassId = updateRelationshipClass.GetId();
    if (auto n = relationshipInfo.GetChildById(DiffNodeId::Strength))
        {
        info.ColsUpdate |= DbECClassInfo::COL_RelationStrength;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECClassInfo::COL_RelationStrength;
        else
            {
            StrengthType strengthType;
            if (!ECDiffValueHelper::TryParseRelationshipStrengthType(strengthType, n->GetValueRight().GetString()))
                {
                LOG.errorv(L"Failed to parse value of strength type %ls",  n->GetValueRight().GetString());
                return BE_SQLITE_ERROR;
                }
            info.ColsUpdate |= DbECClassInfo::COL_RelationStrength;
            info.m_relationStrength = strengthType; 
            }
        }

    if (auto n = relationshipInfo.GetChildById(DiffNodeId::StrengthDirection))
        {
        info.ColsUpdate |= DbECClassInfo::COL_RelationStrengthDirection;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECClassInfo::COL_RelationStrengthDirection;
        else
            {
            ECRelatedInstanceDirection strengthDirection;
            if (!ECDiffValueHelper::TryParseRelatedStrengthDirection(strengthDirection, n->GetValueRight().GetString()))
                {
                LOG.errorv(L"Failed to parse value of strength direction %ls",  n->GetValueRight().GetString());
                return BE_SQLITE_ERROR;
                }
            info.ColsUpdate |= DbECClassInfo::COL_RelationStrengthDirection;
            info.m_relationStrengthDirection = strengthDirection; 
            }
        }

    if (info.ColsUpdate != 0)
        {
        DbResult r = ECDbSchemaPersistence::UpdateECClassInfo(m_ecdb, info);
        if (r != BE_SQLITE_DONE)
            {
            LOG.errorv(L"Failed to update ECClass %ls class attributes", updateRelationshipClass.GetFullName());
            return BE_SQLITE_ERROR;
            }
        }

    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateECClasses (ECN::ECSchemaCR updatedSchema, ECN::IECDiffNodeCR classesDN)
    {
    DbResult r = BE_SQLITE_DONE;
    for(auto classDN :classesDN.GetChildren())
        {
        if (m_updateContext->IsUpdated(classDN))
            continue;

        auto& name = classDN->GetName();
        auto  type = classDN->GetDiffType (true);
        ///////////////////////////////////////////////////////////////////////////////////////////////
        //Defect 120714:Cannot upgrade Graphite02 files. We do not support class deletetion for now
        if (name == L"NamedGroup" || name == L"NamedGroupHasElements" || name == L"NamedGroupHasGroups")
            continue;
        ///////////////////////////////////////////////////////////////////////////////////////////////
        switch(type)
            {
            case DIFFTYPE_Conflict: //Class has conflicting 
            case DIFFTYPE_Right:    //Class added in new schema
            case DIFFTYPE_Left:
            case DIFFTYPE_Empty:    //Not used
                {
                auto classP = updatedSchema.GetClassCP(name.c_str());
                if (classP == nullptr)
                    {

                    LOG.errorv(L"Existing class '%ls' deleted in new schema", name.c_str());
                    BeDataAssert(false);
                    return BE_SQLITE_ERROR;
                    }
                r = UpdateECClass (*classP, *classDN);
                if (r != BE_SQLITE_DONE)
                    {
                    LOG.errorv(L"Failed while updating %ls", classP->GetFullName());
                    BeDataAssert(false);
                    return r;
                    }
                break;
                }
            case DIFFTYPE_Equal:    //Not used
                break;
            }
        }
    return r;
}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::UpdateReferences(ECN::ECSchemaCR updatedECSchema, IECDiffNodeCR referencesDN)
    {
    DbResult r = BE_SQLITE_DONE;
    for(auto referenceDN :referencesDN.GetChildren())
        {
        auto& refNamespacePrefix = referenceDN->GetName();
        auto refSchemaFullNameNew = referenceDN->GetValueRight();
        auto refSchemaFullNameOld = referenceDN->GetValueLeft();

        auto  type = referenceDN->GetDiffType(true);
        switch(type)
            {
            case DIFFTYPE_Conflict: //Class has conflicting 
            case DIFFTYPE_Right:
            case DIFFTYPE_Left:
                {     
                if (!refSchemaFullNameOld.IsNull())
                    {
                    SchemaKey newKey,oldKey;
                    SchemaKey::ParseSchemaFullName(newKey,refSchemaFullNameNew.GetString());
                    SchemaKey::ParseSchemaFullName(oldKey,refSchemaFullNameOld.GetString());

                    if (newKey < oldKey)
                        {
                        return r;
                        }
                    }
                //Let reference schemas name updates
                auto refSchema = updatedECSchema.GetSchemaByNamespacePrefixP(refNamespacePrefix);
                if (refSchema == nullptr)
                    {
                    LOG.errorv(L"Failed to find reference schema %ls with namespacePrefix %ls", 
                        refSchemaFullNameOld.GetString(), refNamespacePrefix.c_str());
                    BeDataAssert(false);
                    return BE_SQLITE_ERROR;
                    }

                if (!refSchema->HasId())
                    {
                    ECSchemaId referenceId = ECDbSchemaPersistence::GetECSchemaId(m_ecdb, *refSchema);
                    if (referenceId == 0)
                        {
                        r = ImportECSchema(*refSchema);
                        if (r != BE_SQLITE_DONE)
                            {
                            LOG.errorv(L"Failed while importing ECSchema Reference %ls", refSchema->GetFullSchemaName().c_str());
                            BeDataAssert(false);
                            return r;
                            }
                        }
                    else
                        const_cast<ECSchemaP>( refSchema)->SetId(referenceId);
                    }

                if (!ECDbSchemaPersistence::ContainsECSchemaReference (m_ecdb, updatedECSchema.GetId(), refSchema->GetId()))
                    {
                    r = CreateECSchemaReferenceEntry (updatedECSchema.GetId(), refSchema->GetId());
                    POSTCONDITION (r == BE_SQLITE_DONE, r);
                    }
                break;
                }

            case DIFFTYPE_Equal:    //Not used
            case DIFFTYPE_Empty:    //Not used
                break;
            }
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbSchemaWriter::Update (ECDiffR diff, ECDbSchemaReaderR schemaReader, ECDbMapR ecdbMap, CustomAttributeTrackerP tracker)
    {
    BeCriticalSectionHolder aGuard (m_aCriticalSection);

    auto& existingSchema = diff.GetLeftSchema();
    auto& updatedSchema = diff.GetRightSchema();
    auto diffTree = diff.GetRootNode();

    if (!updatedSchema.HasId())
        ECDbSchemaManager::GetSchemaIdForECSchemaFromDuplicateECSchema(m_ecdb, updatedSchema);

    if (diff.GetStatus() != DIFFSTATUS_Success)
        {
        LOG.error (L"Provided ECDiff has invalid state");
        return BE_SQLITE_ERROR;
        }

    if (diff.IsEmpty())
        return BE_SQLITE_DONE; //Success
    
    if (diffTree->GetChildById(DiffNodeId::Name))
        {
        LOG.errorv (L"Updating 'Name' of ECSchema %ls to %ls is not supported", existingSchema.GetFullSchemaName().c_str(), updatedSchema.GetFullSchemaName().c_str());
        return BE_SQLITE_ERROR;
        }
    if (diffTree->GetChildById(DiffNodeId::VersionMajor))
        {
        LOG.error (L"Updating 'Major Version Number' of ECSchema is not supported");
        return BE_SQLITE_ERROR;
        }
    if (diffTree->GetChildById(DiffNodeId::NamespacePrefix))
        {
        LOG.errorv (L"Updating 'NamespacePrefix' of ECSchema is not supported");
        return BE_SQLITE_ERROR;
        }

    StopWatch timer ("", true);
    ECSchemaId ecSchemaId = ECDbSchemaPersistence::GetECSchemaId(m_ecdb, updatedSchema);
    if (0 == ecSchemaId)
        {
        LOG.warningv (L"Did not update ECSchema %ls (it does not exists in the ECDb).", updatedSchema.GetFullSchemaName().c_str());
        return BE_SQLITE_ERROR;
        }

    m_customAttributeTracker = tracker;
    DbResult r = BE_SQLITE_ERROR;

    if (auto n = diffTree->GetChildById (DiffNodeId::References))
        r = UpdateReferences(updatedSchema, *n);

    //Following are changes that we do support in a class
    DbECSchemaInfo info;
    info.ColsWhere = DbECSchemaInfo::COL_ECSchemaId;
    info.m_ecSchemaId = updatedSchema.GetId();
    if (auto n = diffTree->GetChildById(DiffNodeId::DisplayLabel))
        {
        info.ColsUpdate |= DbECSchemaInfo::COL_DisplayLabel;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECSchemaInfo::COL_DisplayLabel;
        else
            info.m_displayLabel = Utf8String(n->GetValueRight().GetString());
        }
    if (auto n = diffTree->GetChildById(DiffNodeId::Description))
        {
        info.ColsUpdate |= DbECSchemaInfo::COL_Description;
        if (n->GetValueRight().IsNull())
            info.ColsNull |= DbECSchemaInfo::COL_Description;
        else
            info.m_description = Utf8String(n->GetValueRight().GetString());
        }
    if (auto n = diffTree->GetChildById(DiffNodeId::VersionMinor))
        {
        info.ColsUpdate |= DbECSchemaInfo::COL_VersionMinor;
        if (n->GetValueRight().IsNull())
            {
            LOG.error (L"MinorVersion cannot be null");
            return BE_SQLITE_ERROR;
            }
         info.m_versionMinor = n->GetValueRight().GetInteger();
        }

    if (info.ColsUpdate != 0)
        {
        if ((r = ECDbSchemaPersistence::UpdateECSchemaInfo(m_ecdb, info))!= BE_SQLITE_DONE)
            {
            LOG.errorv (L"Failed to update ECSchema %ls attributes", existingSchema.GetFullSchemaName().c_str());
            return BE_SQLITE_ERROR;
            }
        }

    //Clean up following context before leaving this funtion
    m_updateContext =  std::unique_ptr<SchemaUpdateContext>(new SchemaUpdateContext(diff,schemaReader, ecdbMap));

    if (auto n = diffTree->GetChildById (DiffNodeId::Classes))
        r = UpdateECClasses(updatedSchema, *n);

    if (BE_SQLITE_ERROR != r)
        {

        if (auto n = diffTree->GetChildById(DiffNodeId::CustomAttributes))
            {
            r = UpdateCustomAttributes(updatedSchema, *n, ecSchemaId, ECONTAINERTYPE_Schema);
            }
        }
    if (r != BE_SQLITE_DONE)
        {
        Utf8CP msg = m_ecdb.GetLastError();
        LOG.errorv ("ECSchema update failed for %s (Error: %s)", Utf8String (existingSchema.GetFullSchemaName().c_str()).c_str (), msg);
        BeAssert(false);
        m_updateContext = nullptr;
        return r;
        }

    timer.Stop();
    LOG.debugv ("Updated ECSchema %s in %s in %.2f seconds", Utf8String (existingSchema.GetFullSchemaName().c_str()).c_str (), m_ecdb.GetDbFileName(), timer.GetElapsedSeconds());
    m_updateContext = nullptr;
    return BE_SQLITE_OK;
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
    DbResult status = m_ecdb.GetImplR ().GetECClassIdSequence().GetNextValue (nextId);
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
    r = m_ecdb.GetImplR ().GetECPropertyIdSequence().GetNextValue (nextId);
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

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaUpdateContext::SchemaUpdateContext(ECDiffR diff, ECDbSchemaReaderR schemaReader, ECDbMapR ecdbMap) 
    : m_diff(diff), m_schemaReader(schemaReader), m_ecdbMap(ecdbMap)
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffR SchemaUpdateContext::GetDiff() const {return m_diff;}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECDiffNodeCP SchemaUpdateContext::GetClassDiff(WCharCP className) const
    {
    if (auto classesNode = GetDiff().GetRootNode()->GetChildById(DiffNodeId::Classes))        
        {
        return classesNode->GetChildByAccessString(className);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP SchemaUpdateContext::FindExistingClass(ECClassId existingClassId)
    {
    return m_schemaReader.GetECClass(existingClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IClassMap const* SchemaUpdateContext::FindExistingClassMap(ECClassId existingClassId)
    {
    auto existingClass = FindExistingClass(existingClassId);
    BeAssert(existingClass != nullptr);
    return m_ecdbMap.GetClassMap(*existingClass);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaUpdateContext::IsUpdated(IECDiffNodeCP classDiffNode)
    {
    return m_classUpdated.find(classDiffNode) != m_classUpdated.end();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaUpdateContext::MarkAsUpdated(IECDiffNodeCP classDiffNode)
    {
    m_classUpdated.insert(classDiffNode);
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
