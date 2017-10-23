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
        SchemaImportContext& m_ctx;
        ECN::CustomAttributeValidator m_schemaUpgradeCustomAttributeValidator;
        std::set<ECN::ECSchemaId> m_majorChangesAllowedForSchemas;

        BentleyStatus ImportSchema(SchemaCompareContext&, ECN::ECSchemaCR);
        BentleyStatus ImportClass(ECN::ECClassCR);
        BentleyStatus ImportEnumeration(ECN::ECEnumerationCR);
        BentleyStatus ImportKindOfQuantity(ECN::KindOfQuantityCR);
        BentleyStatus ImportPropertyCategory(ECN::PropertyCategoryCR);
        BentleyStatus ImportProperty(ECN::ECPropertyCR, int ordinal);
        BentleyStatus ImportRelationshipClass(ECN::ECRelationshipClassCP);
        BentleyStatus ImportRelationshipConstraint(ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
        BentleyStatus ImportCustomAttributes(ECN::IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

        BentleyStatus BindPropertyMinMaxValue(Statement&, int paramIndex, ECN::ECPropertyCR, ECN::ECValueCR);
        BentleyStatus BindPropertyExtendedTypeName(Statement&, int paramIndex, ECN::ECPropertyCR);
        BentleyStatus BindPropertyPrimTypeOrEnumeration(Statement&, int primTypeParamIndex, int enumParamIndex, ECN::ECPropertyCR);
        BentleyStatus BindPropertyKindOfQuantity(Statement&, int paramIndex, ECN::ECPropertyCR);
        BentleyStatus BindPropertyCategory(Statement&, int paramIndex, ECN::ECPropertyCR);

        BentleyStatus InsertSchemaEntry(ECN::ECSchemaCR);
        BentleyStatus InsertBaseClassEntry(ECN::ECClassId, ECN::ECClassCR baseClass, int ordinal);
        BentleyStatus InsertRelationshipConstraintEntry(ECRelationshipConstraintId& constraintId, ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
        BentleyStatus InsertSchemaReferenceEntries(ECN::ECSchemaCR);
        BentleyStatus InsertCAEntry(ECN::IECInstanceR customAttribute, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
        BentleyStatus ReplaceCAEntry(ECN::IECInstanceR customAttribute, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
        BentleyStatus DeleteCAEntry(int& ordinal, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

        BentleyStatus UpdateRelationshipConstraint(ECContainerId, ECN::ECRelationshipConstraintChange&, ECN::ECRelationshipConstraintCR oldConstraint, ECN::ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName);
        BentleyStatus UpdateCustomAttributes(SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, ECContainerId, ECN::ECInstanceChanges&, ECN::IECCustomAttributeContainerCR oldClass, ECN::IECCustomAttributeContainerCR newClass);
        BentleyStatus UpdateClass(ECN::ClassChange&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);
        BentleyStatus UpdateProperty(ECN::ECPropertyChange&, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty);
        BentleyStatus UpdateSchema(ECN::SchemaChange&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdateSchemaReferences(ECN::ReferenceChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdateClasses(ECN::ClassChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdateEnumerations(ECN::ECEnumerationChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdateEnumeration(ECN::ECEnumerationChange& enumChanges, ECN::ECEnumerationCR oldEnum, ECN::ECEnumerationCR newEnum);

        BentleyStatus UpdateKindOfQuantities(ECN::KindOfQuantityChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        BentleyStatus UpdatePropertyCategories(ECN::PropertyCategoryChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);

        BentleyStatus UpdateProperties(ECN::ECPropertyChanges&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);

        BentleyStatus DeleteClass(ECN::ClassChange&, ECN::ECClassCR);
        BentleyStatus DeleteProperty(ECN::ECPropertyChange&, ECN::ECPropertyCR);
        BentleyStatus DeleteCustomAttributes(ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);
        BentleyStatus DeleteInstances(ECN::ECClassCR);
        BentleyStatus DeleteCustomAttributeClass(ECN::ECClassCR deletedClass);

        bool IsSpecifiedInRelationshipConstraint(ECN::ECClassCR) const;

        bool IsMajorChangeAllowedForSchema(ECN::ECSchemaId id) const { return m_majorChangesAllowedForSchemas.find(id) != m_majorChangesAllowedForSchemas.end(); }
        bool IsPropertyTypeChangeSupported(Utf8StringR error, ECN::StringChange& typeChange, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty) const;

        BentleyStatus ValidateSchemasPreImport(bvector<ECN::ECSchemaCP> const& primarySchemasOrderedByDependencies) const;

        BentleyStatus UpdateBaseClasses(ECN::BaseClassChanges&, ECN::ECClassCR, ECN::ECClassCR);
        IssueReporter const& Issues() const { return m_ecdb.GetImpl().Issues(); }
        static bool IsChangeToBaseClassIsSupported(ECN::ECClassCR baseClass);
    public:
        explicit SchemaWriter(ECDbCR ecdb, SchemaImportContext& ctx) : m_ecdb(ecdb), m_ctx(ctx)
            {
            m_schemaUpgradeCustomAttributeValidator.Reject("CoreCustomAttributes:IsMixin.*");
            m_schemaUpgradeCustomAttributeValidator.Reject("ECDbMap:*");
            }

        BentleyStatus ImportSchemas(bvector<ECN::ECSchemaCP>& schemasToMap, bvector<ECN::ECSchemaCP> const& primarySchemasOrderedByDependencies);

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
