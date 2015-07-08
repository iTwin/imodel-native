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
    BeMutex m_mutex;
private:
    explicit ECDbSchemaWriter(ECDbR ecdb) : m_ecdb(ecdb) {}

    bool EnsureNamespacePrefixIsUnique(ECSchemaCR);
    BeSQLite::DbResult CreateECSchemaEntry(ECSchemaCR, ECSchemaId);
    BeSQLite::DbResult CreateECClassEntry(ECClassCR, ECClassId);
    BeSQLite::DbResult CreateBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int index);
    BeSQLite::DbResult CreateECPropertyEntry(ECPropertyCR ecProperty, ECPropertyId ecPropertyId, ECClassId ecClassId, int32_t index);
    BeSQLite::DbResult CreateECRelationshipConstraintEntry(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint);
    BeSQLite::DbResult CreateECRelationshipConstraintClassEntry(ECClassId relationshipClassId, ECClassId constraintClassId, ECRelationshipEnd endpoint);
    BeSQLite::DbResult InsertCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, ECContainerId overridenContainerId, int index);
    BeSQLite::DbResult CreateECSchemaReferenceEntry(ECSchemaId ecSchemaId, ECSchemaId ecReferencedSchemaId);

    BeSQLite::DbResult ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, WCharCP onlyImportCAWithClassName = nullptr);

    BeSQLite::DbResult ImportECSchema(ECN::ECSchemaCR);
    BeSQLite::DbResult ImportECClass(ECN::ECClassCR);

    BeSQLite::DbResult ImportECProperty(ECN::ECPropertyCR, ECClassId ecClassId, int32_t index);
    BeSQLite::DbResult ImportECRelationshipClass(ECN::ECRelationshipClassCP, ECClassId relationshipClassId);
    BeSQLite::DbResult ImportECRelationshipConstraint(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECRelationshipEnd);
    BeSQLite::DbResult ImportECCustomAttributeECClass(ECN::IECCustomAttributeContainerCR);
    BeSQLite::DbResult EnsureECSchemaExists(ECClassCR);


public:
    BeSQLite::DbResult Import(ECSchemaCR ecSchema);

    static ECDbSchemaWriterPtr Create(ECDbR ecdb);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
