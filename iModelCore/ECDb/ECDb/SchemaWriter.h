/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaWriter.h $
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
struct SchemaWriter final : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        CustomAttributeValidator m_customAttributeValidator;
        std::set<ECN::ECSchemaId> m_majorChangesAllowedForSchemas;

        BentleyStatus ImportClass(ECN::ECClassCR);
        BentleyStatus ImportEnumeration(ECN::ECEnumerationCR);
        BentleyStatus ImportKindOfQuantity(ECN::KindOfQuantityCR);
        BentleyStatus ImportProperty(ECN::ECPropertyCR, int ordinal);
        BentleyStatus ImportRelationshipClass(ECN::ECRelationshipClassCP);
        BentleyStatus ImportRelationshipConstraint(ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
        BentleyStatus ImportCustomAttributes(ECN::IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

        BentleyStatus BindPropertyExtendedTypeName(Statement&, int paramIndex, ECN::PrimitiveECPropertyCR);
        BentleyStatus BindPropertyExtendedTypeName(Statement&, int paramIndex, ECN::PrimitiveArrayECPropertyCR);
        BentleyStatus BindPropertyKindOfQuantityId(Statement&, int paramIndex, ECN::PrimitiveECPropertyCR);
        BentleyStatus BindPropertyKindOfQuantityId(Statement&, int paramIndex, ECN::PrimitiveArrayECPropertyCR);

        BentleyStatus InsertSchemaEntry(ECN::ECSchemaCR);
        BentleyStatus InsertBaseClassEntry(ECN::ECClassId, ECN::ECClassCR baseClass, int ordinal);
        BentleyStatus InsertRelationshipConstraintEntry(ECRelationshipConstraintId& constraintId, ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
        BentleyStatus InsertSchemaReferenceEntries(ECN::ECSchemaCR);
        BentleyStatus InsertCAEntry(ECN::IECInstanceP customAttribute, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
        BentleyStatus ReplaceCAEntry(ECN::IECInstanceP customAttribute, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
        BentleyStatus DeleteCAEntry(int& ordinal, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

        BentleyStatus UpdateRelationshipConstraint(ECContainerId, ECRelationshipConstraintChange&, ECN::ECRelationshipConstraintCR oldConstraint, ECN::ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName);
        BentleyStatus UpdateCustomAttributes(SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, ECContainerId, ECInstanceChanges&, ECN::IECCustomAttributeContainerCR oldClass, ECN::IECCustomAttributeContainerCR newClass);
        BentleyStatus UpdateClass(ClassChange&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);
        BentleyStatus UpdateProperty(ECPropertyChange&, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty);
        BentleyStatus UpdateSchema(SchemaChange&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdateSchemaReferences(ReferenceChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdateClasses(ClassChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdateEnumerations(ECEnumerationChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdateEnumeration(ECEnumerationChange& enumChanges, ECN::ECEnumerationCR oldEnum, ECN::ECEnumerationCR newEnum);

        BentleyStatus UpdateKindOfQuanitites(ECKindOfQuantityChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);

        BentleyStatus UpdateProperties(ECPropertyChanges&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);

        BentleyStatus DeleteClass(ClassChange&, ECN::ECClassCR);
        BentleyStatus DeleteProperty(ECPropertyChange&, ECN::ECPropertyCR);
        BentleyStatus DeleteCustomAttributes(ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);
        BentleyStatus DeleteInstances(ECN::ECClassCR);

        bool IsSpecifiedInRelationshipConstraint(ECN::ECClassCR) const;
        BentleyStatus TryParseId(Utf8StringR schemaName, Utf8StringR className, Utf8StringCR id) const;

        bool IsMajorChangeAllowedForSchema(ECN::ECSchemaId id) const { return m_majorChangesAllowedForSchemas.find(id) != m_majorChangesAllowedForSchemas.end(); }
        bool IsPropertyTypeChangeSupported(Utf8StringR error, StringChange& typeChange, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty) const;

        IssueReporter const& Issues() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }

    public:
        explicit SchemaWriter(ECDbCR ecdb) : m_ecdb(ecdb)
            {
            m_customAttributeValidator.Accept("ECDbMap:DbIndexList.Indexes.Name");
            m_customAttributeValidator.Reject("ECDbMap:*");
            }

        BentleyStatus Import(SchemaCompareContext&, ECN::ECSchemaCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
