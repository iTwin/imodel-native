/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ECSql/NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECDbSchemaWriter : NonCopyableClass
    {
private:
    ECDbCR m_ecdb;
    CustomAttributeValidator m_customAttributeValidator;
    std::set<ECN::ECSchemaId> m_majorChangesAllowedForSchemas;

    BentleyStatus ImportECClass(ECN::ECClassCR);
    BentleyStatus ImportECEnumeration(ECN::ECEnumerationCR);
    BentleyStatus ImportKindOfQuantity(ECN::KindOfQuantityCR);
    BentleyStatus ImportECProperty(ECN::ECPropertyCR, int ordinal);
    BentleyStatus ImportECRelationshipClass(ECN::ECRelationshipClassCP);
    BentleyStatus ImportECRelationshipConstraint(ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
    BentleyStatus ImportCustomAttributes(ECN::IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

    BentleyStatus BindPropertyExtendedTypeName(Statement&, int paramIndex, ECN::PrimitiveECPropertyCR);
    BentleyStatus BindPropertyExtendedTypeName(Statement&, int paramIndex, ECN::PrimitiveArrayECPropertyCR);
    BentleyStatus BindPropertyKindOfQuantityId(Statement&, int paramIndex, ECN::PrimitiveECPropertyCR);
    BentleyStatus BindPropertyKindOfQuantityId(Statement&, int paramIndex, ECN::PrimitiveArrayECPropertyCR);

    BentleyStatus InsertECSchemaEntry(ECN::ECSchemaCR);
    BentleyStatus InsertBaseClassEntry(ECN::ECClassId, ECN::ECClassCR baseClass, int ordinal);
    BentleyStatus InsertECRelationshipConstraintEntry(ECRelationshipConstraintId& constraintId, ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
    BentleyStatus InsertECSchemaReferenceEntries(ECN::ECSchemaCR);
    BentleyStatus InsertCAEntry(ECN::IECInstanceP customAttribute, ECN::ECClassId, ECContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
    BentleyStatus ReplaceCAEntry(ECN::IECInstanceP customAttribute, ECN::ECClassId, ECContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
    BentleyStatus DeleteCAEntry(int& ordinal, ECN::ECClassId, ECContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

    BentleyStatus UpdateECRelationshipConstraint(ECContainerId, ECRelationshipConstraintChange&, ECN::ECRelationshipConstraintCR oldConstraint, ECN::ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName);
    BentleyStatus UpdateECCustomAttributes(ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, ECContainerId, ECInstanceChanges&, ECN::IECCustomAttributeContainerCR oldClass, ECN::IECCustomAttributeContainerCR newClass);
    BentleyStatus UpdateECClass(ECClassChange&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);
    BentleyStatus UpdateECProperty(ECPropertyChange&, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty);
    BentleyStatus UpdateECSchema(ECSchemaChange&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
    BentleyStatus UpdateECSchemaReferences(ReferenceChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
    BentleyStatus UpdateECClasses(ECClassChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
    BentleyStatus UpdateECEnumerations(ECEnumerationChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
    BentleyStatus UpdateECKindOfQuanitites(ECKindOfQuantityChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);

    BentleyStatus UpdateECProperties(ECPropertyChanges&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);

    BentleyStatus DeleteECClass(ECClassChange&, ECN::ECClassCR);
    BentleyStatus DeleteECProperty(ECPropertyChange&, ECN::ECPropertyCR);
    BentleyStatus DeleteECCustomAttributes(ECContainerId, ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);
    BentleyStatus DeleteECInstances(ECN::ECClassCR);

    bool IsSpecifiedInECRelationshipConstraint(ECN::ECClassCR) const;
    BentleyStatus TryParseId(Utf8StringR schemaName, Utf8StringR className, Utf8StringCR id) const;

    bool IsMajorChangeAllowedForECSchema(ECN::ECSchemaId id) const { return m_majorChangesAllowedForSchemas.find(id) != m_majorChangesAllowedForSchemas.end(); }
    bool IsPropertyTypeChangeSupported(Utf8StringR error, StringChange& typeChange, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty) const;

    IssueReporter const& Issues() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }
    public:
    explicit ECDbSchemaWriter(ECDbCR ecdb) : m_ecdb(ecdb)
        {
        m_customAttributeValidator.Accept("ECDbMap:DbIndexList.Indexes.Name");
        m_customAttributeValidator.Reject("ECDbMap:*");
        }

    BentleyStatus Import(ECSchemaCompareContext&, ECN::ECSchemaCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
