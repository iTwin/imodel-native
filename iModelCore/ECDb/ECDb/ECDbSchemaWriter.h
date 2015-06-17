/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaWriter : RefCountedBase
    {
private:
    ECDbR m_ecdb;
    BeMutex m_aCriticalSection;
private:
    ECDbSchemaWriter(ECDbR ecdb) : m_ecdb(ecdb) {}

    bool EnsureNamespacePrefixIsUnique                          (ECSchemaCR ecSchema);
    BeSQLite::DbResult CreateECSchemaEntry                      (ECSchemaCR ecSchema, ECSchemaId ecSchemaId);
    BeSQLite::DbResult CreateECClassEntry                       (ECClassCR ecClass, ECClassId ecClassId);
    BeSQLite::DbResult CreateBaseClassEntry                     (ECClassId ecClassId, ECClassCR baseClass, int index);
    BeSQLite::DbResult CreateECPropertyEntry                    (ECPropertyCR ecProperty, ECPropertyId ecPropertyId, ECClassId ecClassId, int32_t index);
    BeSQLite::DbResult CreateECRelationConstraintEntry          (ECClassId ecClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint);
    BeSQLite::DbResult CreateECRelationshipConstraintClassEntry (ECClassId ecClassId, ECClassId constraintClassId, ECRelationshipEnd endpoint);
    BeSQLite::DbResult InsertCAEntry                            (IECInstanceP customAttribute,ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, ECContainerId overridenContainerId, int index);
    BeSQLite::DbResult CreateECSchemaReferenceEntry             (ECSchemaId ecSchemaId, ECSchemaId ecReferencedSchemaId);

    BeSQLite::DbResult ImportCustomAttributes                   (IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, WCharCP onlyImportCAWithClassName = nullptr);

    BeSQLite::DbResult ImportECSchema                           (ECN::ECSchemaCR ecSchema);
    BeSQLite::DbResult ImportECClass                            (ECN::ECClassCR ecClass);

    BeSQLite::DbResult ImportECProperty                         (ECN::ECPropertyCR ecProperty, ECClassId ecClassId, int32_t index);
    BeSQLite::DbResult ImportECRelationshipClass                (ECN::ECRelationshipClassCP relationship, ECClassId ecClassId);
    BeSQLite::DbResult ImportECRelationshipConstraint           (ECClassId ecClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint);
    BeSQLite::DbResult ImportECCustomAttributeECClass           (ECN::IECCustomAttributeContainerCR caContainer);
    BeSQLite::DbResult EnsureECSchemaExists                     (ECClassCR ecClass);


public:  
    BeSQLite::DbResult    Import                                (ECSchemaCR ecSchema);
    //in diff Left schema = existing stored schema and Right schema = newly provided schema by user with same name
    BeSQLite::DbResult    Update                                (ECDiffR diff, ECDbSchemaReaderR schemaReader, ECDbMapR ecdbMap);

    static ECDbSchemaWriterPtr Create                           (ECDbR ecdb);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
