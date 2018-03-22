/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaWriter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ECSql/NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaWriter final
    {
    friend struct ProfileUpgrader_4002;
    public:
        struct Context final
            {
        private:
            ECDbCR m_ecdb;
            SchemaImportContext& m_importCtx;

            ECN::SchemaChanges m_changes;
            bvector<ECN::ECSchemaCP> m_existingSchemas;
            bvector<ECN::ECSchemaCP> m_schemasToImport;
            bset<ECN::ECSchemaId> m_schemasWithMajorVersionChange;
            ECN::CustomAttributeValidator m_schemaUpgradeCustomAttributeValidator;

        public:
            Context(ECDbCR ecdb, SchemaImportContext& ctx) : m_ecdb(ecdb), m_importCtx(ctx)
                {
                m_schemaUpgradeCustomAttributeValidator.Reject("CoreCustomAttributes:IsMixin.*");
                m_schemaUpgradeCustomAttributeValidator.Reject("ECDbMap:*");
                }

            void ClearCache() { m_schemasToImport.clear(); m_existingSchemas.clear(); m_ecdb.ClearECDbCache(); }
            SchemaImportContext& ImportCtx() const { return m_importCtx; }
            bvector<ECN::ECSchemaCP> const& GetSchemasToImport() const { return m_schemasToImport; }
            bvector<ECN::ECSchemaCP>& GetSchemasToImportR() { return m_schemasToImport; }
            void AddSchemaToImport(ECN::ECSchemaCR schema) { m_schemasToImport.push_back(&schema); }
            bvector<ECN::ECSchemaCP> const& GetExistingSchemas() const { return m_existingSchemas; }
            bvector<ECN::ECSchemaCP>& GetExistingSchemasR() { return m_existingSchemas; }
            void AddExistingSchema(ECN::ECSchemaCR schema) { m_existingSchemas.push_back(&schema); }
            ECN::SchemaChanges& Changes() { return m_changes; }

            bool AreMajorSchemaVersionChangesAllowed() const { return !Enum::Contains(m_importCtx.GetOptions(), SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade); }
            bool IsMajorSchemaVersionChange(ECN::ECSchemaId schemaId) const { return m_schemasWithMajorVersionChange.find(schemaId) != m_schemasWithMajorVersionChange.end(); }
            void AddSchemaWithMajorVersionChange(ECN::ECSchemaId schemaId) { m_schemasWithMajorVersionChange.insert(schemaId); }
            CachedStatementPtr GetCachedStatement(Utf8CP sql) { return m_ecdb.GetImpl().GetCachedSqliteStatement(sql); }
            ECDbCR GetECDb() const { return m_ecdb; }
            MainSchemaManager const& GetSchemaManager() const { return m_importCtx.GetSchemaManager(); }
            ECN::CustomAttributeValidator const& GetSchemaUpgradeCustomAttributeValidator() const { return m_schemaUpgradeCustomAttributeValidator; }
            IssueReporter const& Issues() const { return m_ecdb.GetImpl().Issues(); }
            };

    private:
        SchemaWriter() = delete;
        ~SchemaWriter() = delete;

        static BentleyStatus ImportSchema(Context&, ECN::ECSchemaCR);
        static BentleyStatus ImportClass(Context&, ECN::ECClassCR);
        static BentleyStatus ImportEnumeration(Context&, ECN::ECEnumerationCR);
        static BentleyStatus ImportUnitSystem(Context&, ECN::UnitSystemCR);
        static BentleyStatus ImportPhenomenon(Context&, ECN::PhenomenonCR);
        static BentleyStatus ImportUnit(Context&, ECN::ECUnitCR);
        static BentleyStatus ImportKindOfQuantity(Context&, ECN::KindOfQuantityCR);
        static BentleyStatus ImportPropertyCategory(Context&, ECN::PropertyCategoryCR);
        static BentleyStatus ImportProperty(Context&, ECN::ECPropertyCR, int ordinal);
        static BentleyStatus ImportRelationshipClass(Context&, ECN::ECRelationshipClassCP);
        static BentleyStatus ImportRelationshipConstraint(Context&, ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
        static BentleyStatus ImportCustomAttributes(Context&, ECN::IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

        static BentleyStatus BindPropertyMinMaxValue(Context&, Statement&, int paramIndex, ECN::ECPropertyCR, ECN::ECValueCR);
        static BentleyStatus BindPropertyExtendedTypeName(Statement&, int paramIndex, ECN::ECPropertyCR);
        static BentleyStatus BindPropertyPrimTypeOrEnumeration(Context& ctx, Statement&, int primTypeParamIndex, int enumParamIndex, ECN::ECPropertyCR);
        static BentleyStatus BindPropertyKindOfQuantity(Context&, Statement&, int paramIndex, ECN::ECPropertyCR);
        static BentleyStatus BindPropertyCategory(Context&, Statement&, int paramIndex, ECN::ECPropertyCR);

        static BentleyStatus InsertSchemaEntry(Context& ctx, ECN::ECSchemaCR schema) { return InsertSchemaEntry(ctx.GetECDb(), schema); }
        static BentleyStatus InsertSchemaEntry(ECDbCR, ECN::ECSchemaCR);  //!< Also used by ProfileUpgrader_4002
        static BentleyStatus InsertBaseClassEntry(Context&, ECN::ECClassId, ECN::ECClassCR baseClass, int ordinal);
        static BentleyStatus InsertRelationshipConstraintEntry(Context&, ECRelationshipConstraintId& constraintId, ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
        static BentleyStatus InsertSchemaReferenceEntries(Context&, ECN::ECSchemaCR);
        static BentleyStatus InsertCAEntry(Context&, ECN::IECInstanceR customAttribute, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
        static BentleyStatus ReplaceCAEntry(Context&, ECN::IECInstanceR customAttribute, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
        static BentleyStatus DeleteCAEntry(int& ordinal, Context&, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

        static BentleyStatus UpdateRelationshipConstraint(Context&, ECContainerId, ECN::ECRelationshipConstraintChange&, ECN::ECRelationshipConstraintCR oldConstraint, ECN::ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName);
        static BentleyStatus UpdateCustomAttributes(Context&, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, ECContainerId, ECN::ECInstanceChanges&, ECN::IECCustomAttributeContainerCR oldClass, ECN::IECCustomAttributeContainerCR newClass);
        static BentleyStatus UpdateClass(Context&, ECN::ClassChange&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);
        static BentleyStatus UpdateProperty(Context&, ECN::ECPropertyChange&, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty);
        static BentleyStatus UpdateSchema(Context&, ECN::SchemaChange&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateSchemaReferences(Context&, ECN::ReferenceChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateClasses(Context&, ECN::ClassChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateEnumerations(Context&, ECN::ECEnumerationChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateEnumeration(Context&, ECN::ECEnumerationChange&, ECN::ECEnumerationCR oldEnum, ECN::ECEnumerationCR newEnum);
        static BentleyStatus VerifyEnumeratorChanges(Context&, ECN::ECEnumerationCR oldEnum, ECN::ECEnumeratorChanges&);

        static BentleyStatus UpdateKindOfQuantities(Context&, ECN::KindOfQuantityChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateKindOfQuantity(Context&, ECN::KindOfQuantityChange&, ECN::KindOfQuantityCR oldKoq, ECN::KindOfQuantityCR newKoq);
        static BentleyStatus UpdatePropertyCategories(Context&, ECN::PropertyCategoryChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdatePropertyCategory(Context&, ECN::PropertyCategoryChange&, ECN::PropertyCategoryCR oldCat, ECN::PropertyCategoryCR newCat);
        static BentleyStatus UpdatePhenomena(Context&, ECN::PhenomenonChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdatePhenomenon(Context&, ECN::PhenomenonChange&, ECN::PhenomenonCR oldVal, ECN::PhenomenonCR newVal);
        static BentleyStatus UpdateUnitSystems(Context&, ECN::UnitSystemChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateUnitSystem(Context&, ECN::UnitSystemChange&, ECN::UnitSystemCR oldVal, ECN::UnitSystemCR newVal);
        static BentleyStatus UpdateUnits(Context&, ECN::UnitChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateUnit(Context&, ECN::UnitChange&, ECN::ECUnitCR oldVal, ECN::ECUnitCR newVal);

        static BentleyStatus UpdateProperties(Context&, ECN::ECPropertyChanges&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);

        static BentleyStatus DeleteClass(Context&, ECN::ClassChange&, ECN::ECClassCR);
        static BentleyStatus DeleteProperty(Context&, ECN::ECPropertyChange&, ECN::ECPropertyCR);
        static BentleyStatus DeleteCustomAttributes(Context&, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);
        static BentleyStatus DeleteInstances(Context&, ECN::ECClassCR);
        static BentleyStatus DeleteCustomAttributeClass(Context&, ECN::ECCustomAttributeClassCR);

        static bool IsSpecifiedInRelationshipConstraint(Context&, ECN::ECClassCR);

        static bool IsPropertyTypeChangeSupported(Utf8StringR error, ECN::StringChange& typeChange, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty);

        static BentleyStatus ValidateSchemasPreImport(Context const&, bvector<ECN::ECSchemaCP> const& primarySchemasOrderedByDependencies);

        static BentleyStatus UpdateBaseClasses(Context&, ECN::BaseClassChanges&, ECN::ECClassCR, ECN::ECClassCR);
        static bool IsChangeToBaseClassIsSupported(ECN::ECClassCR baseClass);

        static BentleyStatus CompareSchemas(Context& ctx, bvector<ECN::ECSchemaCP> const& primarySchemasOrderedByDependencies);
        static BentleyStatus ReloadSchemas(Context& ctx);

    public:
        static BentleyStatus ImportSchemas(bvector<ECN::ECSchemaCP>& schemasToMap, ECDbCR, SchemaImportContext&, bvector<ECN::ECSchemaCP> const& primarySchemasOrderedByDependencies);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
