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
    CustomAttributeValidator m_customAttributeValidator;
    std::set<ECSchemaId> m_majorChangesAllowedForSchemas;
    bool IsMajorChangeAllowedForECSchema(ECSchemaId Id) const { return m_majorChangesAllowedForSchemas.find(Id) != m_majorChangesAllowedForSchemas.end(); }
    BentleyStatus CreateECSchemaEntry(ECSchemaCR);
    BentleyStatus CreateBaseClassEntry(ECClassId, ECClassCR baseClass, int ordinal);
    BentleyStatus CreateECRelationshipConstraintEntry(ECRelationshipConstraintId& constraintId, ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECRelationshipEnd);
    BentleyStatus InsertCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
    BentleyStatus ReplaceCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
    BentleyStatus DeleteCAEntry(ECClassId, ECContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);
    BentleyStatus CreateECSchemaReferenceEntry(ECSchemaId, ECSchemaId ecReferencedSchemaId);
    BentleyStatus ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, Utf8CP onlyImportCAWithClassName = nullptr);
    BentleyStatus ImportECClass(ECN::ECClassCR);
    BentleyStatus ImportECEnumeration(ECN::ECEnumerationCR);
    BentleyStatus ImportECProperty(ECN::ECPropertyCR, int ordinal);
    BentleyStatus ImportECRelationshipClass(ECN::ECRelationshipClassCP);
    BentleyStatus ImportECRelationshipConstraint(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECRelationshipEnd);
    BentleyStatus EnsureECSchemaExists(ECClassCR);
    BentleyStatus UpdateECRelationshipConstraint(ECContainerId, SqlUpdater&, ECRelationshipConstraintChange&, ECRelationshipConstraintCR oldConstraint, ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName);
    BentleyStatus UpdateECCustomAttributes(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, ECContainerId, ECInstanceChanges&,IECCustomAttributeContainerCR oldClass, IECCustomAttributeContainerCR newClass);
    BentleyStatus UpdateECClass(ECClassChange&, ECClassCR oldClass, ECClassCR newClass);
    BentleyStatus UpdateECProperty(ECPropertyChange&, ECPropertyCR oldProperty, ECPropertyCR newProperty);
    BentleyStatus UpdateECSchema(ECSchemaChange&, ECSchemaCR oldSchema, ECSchemaCR newSchema);
    BentleyStatus UpdateECSchemaReferences(ReferenceChanges& referenceChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema);
    BentleyStatus UpdateECClasses(ECClassChanges& classChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema);
    BentleyStatus UpdateECEnumerations(ECEnumerationChanges& enumChanges, ECSchemaCR oldSchema, ECSchemaCR newSchema);
    BentleyStatus UpdateECProperties(ECPropertyChanges&, ECClassCR oldClass, ECClassCR newClass);

    BentleyStatus DeleteECClass(ECClassChange& classChange, ECClassCR deletedClass);
    BentleyStatus DeleteECProperty(ECPropertyChange& propertyChange, ECPropertyCR deletedProperty);
    BentleyStatus DeleteECCustomAttributes(ECContainerId id, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType type);
    BentleyStatus DeleteECInstances(ECClassCR deletedClass);
    bool IsSpecifiedInECRelationshipConstraint(ECClassCR deletedClass) const;
    BentleyStatus TryParseId(Utf8StringR schemaName, Utf8StringR className, Utf8StringCR id) const;

    IssueReporter const& GetIssueReporter() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }

public:
    explicit ECDbSchemaWriter(ECDbCR ecdb) : m_ecdb (ecdb)
        {
        m_customAttributeValidator.Accept("ECDbMap:ClassMap.Indexes.Name");
        m_customAttributeValidator.Accept("ECDbMap:ClassMap.MapStrategy.MinimumSharedColumnCount");
        m_customAttributeValidator.Reject("ECDbMap:*");
        }

    BentleyStatus Import(ECSchemaCompareContext& ctx, ECSchemaCR);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
