/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
! This class is used to keep state of update. Its important that we isolate updating
! schema state from rest of write code. We also keep ECDbSchemaReader and ECDbMap private
! and only expose very necessary funtions here.
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaUpdateContext
    {
private:
    ECDiffR m_diff;
    std::set<IECDiffNodeCP> m_classUpdated;
    ECDbSchemaReaderR m_schemaReader;
    ECDbMapR m_ecdbMap;
public:
    SchemaUpdateContext(ECDiffR diff, ECDbSchemaReaderR schemaReader, ECDbMapR ecdbMap);
    //Get diff root
    ECDiffR GetDiff() const;
    //! Return class diff for a specified class name.
    IECDiffNodeCP GetClassDiff(WCharCP className) const;
    //! Get exisiting class if any from ECDb.
    ECN::ECClassCP FindExistingClass(ECClassId existingClassId);
    //! Get exisiting classMap if any from ECDb.
    IClassMap const* FindExistingClassMap (ECClassId existingClassId);
    //! See if ECDiff node has been already updated
    bool IsUpdated(IECDiffNodeCP classDiffNode);
    //! Mark a ECDiff node as updated so it doesnt get processed again
    void MarkAsUpdated(IECDiffNodeCP classDiffNode);
    };


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaWriter : RefCountedBase
    {
private:
    ECDbR m_ecdb;
//    ECSchemaCP                                                  m_PrimaryECSchema; -- detected as unused by clang
    CustomAttributeTrackerP                                     m_customAttributeTracker;
    std::unique_ptr<SchemaUpdateContext>                        m_updateContext;
    BeCriticalSection                                           m_aCriticalSection;
private:
                  bool EnsureNamespacePrefixIsUnique            (ECSchemaCR ecSchema);
    BeSQLite::DbResult CreateECSchemaEntry                      (ECSchemaCR ecSchema, ECSchemaId ecSchemaId);
    BeSQLite::DbResult CreateECClassEntry                       (ECClassCR ecClass, ECClassId ecClassId);
    BeSQLite::DbResult CreateBaseClassEntry                     (ECClassId ecClassId, ECClassCR baseClass, int index);
    BeSQLite::DbResult CreateECPropertyEntry                    (ECPropertyCR ecProperty, ECPropertyId ecPropertyId, ECClassId ecClassId, int32_t index);
    BeSQLite::DbResult CreateECRelationConstraintEntry          (ECClassId ecClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint);
    BeSQLite::DbResult CreateECRelationshipConstraintClassEntry (ECClassId ecClassId, ECClassId constraintClassId, ECRelationshipEnd endpoint);
    BeSQLite::DbResult InsertCAEntry                            (IECInstanceP customAttribute,ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, ECContainerId overridenContainerId, int index);
    BeSQLite::DbResult CreateECSchemaReferenceEntry             (ECSchemaId ecSchemaId, ECSchemaId ecReferenceSchemaId);

    BeSQLite::DbResult ImportCustomAttributes                   (IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, WCharCP onlyImportCAWithClassName = nullptr);

    BeSQLite::DbResult ImportECSchema                           (ECN::ECSchemaCR ecSchema);
    BeSQLite::DbResult ImportECClass                            (ECN::ECClassCR ecClass);

    BeSQLite::DbResult ImportECProperty                         (ECN::ECPropertyCR ecProperty, ECClassId ecClassId, int32_t index);
    BeSQLite::DbResult ImportECRelationshipClass                (ECN::ECRelationshipClassCP relationship, ECClassId ecClassId);
    BeSQLite::DbResult ImportECRelationshipConstraint           (ECClassId ecClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint);
    BeSQLite::DbResult ImportECCustomAttributeECClass           (ECN::IECCustomAttributeContainerCR caContainer);
    BeSQLite::DbResult EnsureECSchemaExists                     (ECClassCR ecClass);
                       ECDbSchemaWriter                         (ECDbR ecdb);

    BeSQLite::DbResult UpdateECClasses                          (ECN::ECSchemaCR updatedSchema, ECN::IECDiffNodeCR classesDN);
    BeSQLite::DbResult UpdateECClass                            (ECN::ECClassCR ecClass, ECN::IECDiffNodeCR classDN);

    BeSQLite::DbResult UpdateECProperty                         (ECN::ECPropertyCR ecProperty, ECN::IECDiffNodeCR propertyDN, ECClassId ecClassId);
    BeSQLite::DbResult UpdateECProperties                       (ECN::ECClassCR ecClass, ECN::IECDiffNodeCR propertiesDN);

    BeSQLite::DbResult UpdateCustomAttributes                   (IECCustomAttributeContainerCR sourceContainer, IECDiffNodeCR customAttributesDN,ECContainerId sourceContainerId, ECContainerType containerType);
    BeSQLite::DbResult UpdateCustomAttribute                    (IECInstanceCR ecInstance, IECCustomAttributeContainerCR container, ECContainerId sourceContainerId, ECContainerType containerType);

    BeSQLite::DbResult UpdateReferences                         (ECN::ECSchemaCR updatedECSchema, IECDiffNodeCR referencesDN);
    BeSQLite::DbResult UpdateECRelationshipClass                (ECN::ECRelationshipClassCR updateRelationshipClass, ECN::IECDiffNodeCR relationshipInfo);
    BeSQLite::DbResult UpdateECRelationshipConstraint           (ECN::ECRelationshipClassCR updateRelationshipClass, ECRelationshipEnd relationshipEnd,  ECN::IECDiffNodeCR relationshipConstraint);
    ECClassId          ResolveClassIdFromUpdateContext          (WCharCP schemaName, WCharCP className);
    bool               DiffExist (IECDiffNodeCR diff, WCharCP name);

public:  
    BeSQLite::DbResult    Import                                (ECSchemaCR ecSchema, CustomAttributeTrackerP tracker);
    //in diff Left schema = existing stored schema and Right schema = newly provided schema by user with same name
    BeSQLite::DbResult    Update                                (ECDiffR diff, ECDbSchemaReaderR schemaReader, ECDbMapR ecdbMap, CustomAttributeTrackerP tracker);

    static ECDbSchemaWriterPtr Create                           (ECDbR ecdb);

    };
END_BENTLEY_SQLITE_EC_NAMESPACE
