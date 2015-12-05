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

    explicit ECDbSchemaWriter(ECDbR ecdb) : m_ecdb(ecdb) {}

    DbResult CreateECSchemaEntry(ECSchemaCR, ECSchemaId);
    BentleyStatus CreateBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int index);
    BentleyStatus CreateECPropertyEntry(ECPropertyCR ecProperty, ECPropertyId ecPropertyId, ECClassId ecClassId, int32_t index);
    BentleyStatus CreateECRelationshipConstraintEntry(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint);
    BentleyStatus CreateECRelationshipConstraintClassEntry(ECClassId relationshipClassId, ECClassId constraintClassId, ECRelationshipEnd endpoint);
    BentleyStatus InsertCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, ECContainerId overridenContainerId, int index);
    BentleyStatus CreateECSchemaReferenceEntry(ECSchemaId ecSchemaId, ECSchemaId ecReferencedSchemaId);

    BentleyStatus ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, Utf8CP onlyImportCAWithClassName = nullptr);

    BentleyStatus ImportECClass(ECN::ECClassCR);

    BentleyStatus ImportECProperty(ECN::ECPropertyCR, ECClassId ecClassId, int32_t index);
    BentleyStatus ImportECRelationshipClass(ECN::ECRelationshipClassCP, ECClassId relationshipClassId);
    BentleyStatus ImportECRelationshipConstraint(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECRelationshipEnd);
    BentleyStatus ImportECCustomAttributeECClass(ECN::IECCustomAttributeContainerCR);
    BentleyStatus EnsureECSchemaExists(ECClassCR);


public:
    BentleyStatus Import(ECSchemaCR ecSchema);

    static ECDbSchemaWriterPtr Create(ECDbR ecdb);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
