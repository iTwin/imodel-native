/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SqlUpdater
    {
    private:
        std::map<Utf8String, ECN::ECValue> m_updateMap;
        std::map<Utf8String, ECN::ECValue> m_whereMap;
        Utf8String m_table;
        BentleyStatus BindSet(Statement& stmt, Utf8StringCR column, int i) const;
        BentleyStatus BindWhere(Statement& stmt, Utf8StringCR column, int i) const;

    public:
        SqlUpdater(Utf8CP table) : m_table(table) {}
        ~SqlUpdater(){}
        void Set(Utf8CP column, Utf8CP value);
        void Set(Utf8CP column, Utf8StringCR value);
        void Set(Utf8CP column, double value);
        void Set(Utf8CP column, bool value);
        void Set(Utf8CP column, uint32_t value);
        void Set(Utf8CP column, uint64_t value);
        void Set(Utf8CP column, int32_t value);
        void Set(Utf8CP column, int64_t value);
        void Where(Utf8CP column, int64_t value);
        BentleyStatus Apply(ECDb const& ecdb) const;
    };
//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECDbSchemaWriter : NonCopyableClass
    {
private:
    ECDbCR m_ecdb;
    bmap<ECN::ECEnumerationCP, uint64_t> m_enumIdCache;
    BeMutex m_mutex;

    private:
    BentleyStatus CreateECSchemaEntry(ECSchemaCR);
    BentleyStatus CreateBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int ordinal);
    BentleyStatus CreateECRelationshipConstraintEntry(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint);
    BentleyStatus InsertCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, int ordinal);
    BentleyStatus ReplaceCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, int ordinal);
    BentleyStatus DeleteCAEntry(ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType);
    BentleyStatus CreateECSchemaReferenceEntry(ECSchemaId ecSchemaId, ECSchemaId ecReferencedSchemaId);
    BentleyStatus ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, Utf8CP onlyImportCAWithClassName = nullptr);
    BentleyStatus ImportECClass(ECN::ECClassCR);
    BentleyStatus ImportECEnumeration(ECN::ECEnumerationCR);
    BentleyStatus ImportECProperty(ECN::ECPropertyCR, int ordinal);
    BentleyStatus ImportECRelationshipClass(ECN::ECRelationshipClassCP);
    BentleyStatus ImportECRelationshipConstraint(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECRelationshipEnd);
    BentleyStatus EnsureECSchemaExists(ECClassCR);
    BentleyStatus UpdateECRelationshipConstraint(ECContainerId containerId, SqlUpdater& sqlUpdater, ECRelationshipConstraintChange& constraintChange, ECRelationshipConstraintCR oldConstraint, ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName);
    BentleyStatus UpdateECCustomAttributes(ECContainerType containerType, ECContainerId containerId, ECInstanceChanges& instanceChanges,IECCustomAttributeContainerCR oldClass, IECCustomAttributeContainerCR newClass);
    BentleyStatus UpdateECClass(ECClassChange& classChange, ECClassCR oldClass, ECClassCR newClass);
    BentleyStatus UpdateECProperty(ECPropertyChange& propertyChange, ECPropertyCR oldProperty, ECPropertyCR newProperty);
    BentleyStatus UpdateECSchema(ECSchemaChange& schemaChange, ECSchemaCR oldSchema, ECSchemaCR newSchema);
    BentleyStatus UpdateECSchemaReferences(ReferenceChanges& referenceChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema);
    BentleyStatus UpdateECClasses(ECClassChanges& classChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema);
    BentleyStatus UpdateECEnumerations(ECEnumerationChanges& enumChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema);

    BentleyStatus Fail(Utf8CP fmt, ...) const;
    void Warn(Utf8CP fmt, ...) const;
    BentleyStatus TryParseId(Utf8StringR schemaName, Utf8StringR className, Utf8StringCR id) const;

public:
    explicit ECDbSchemaWriter(ECDbCR ecdb) : m_ecdb (ecdb) {}
    BentleyStatus Import(ECSchemaCompareContext& ctx, ECSchemaCR ecSchema);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
