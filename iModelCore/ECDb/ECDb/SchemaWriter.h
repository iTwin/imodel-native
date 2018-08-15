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
        public:
            //! When importing EC3.1 schemas into EC3.1 files, ECObjects deserializes the EC3.2 units and formats schema
            //! as in-memory the schema will be an EC3.2 one. ECDb, however, must not persist these temporarily deserialized units and formats schemas.
            //! This helper helps the respective schema writer code to ignore those schemas
            struct LegacyUnitsHelper final
                {
                private:
                    ECN::ECSchemaCP m_unitsSchema = nullptr;
                    ECN::ECSchemaCP m_formatsSchema = nullptr;

                public:
                    LegacyUnitsHelper() {}
                    void Preprocess(bvector<ECN::ECSchemaCP>& out, bvector<ECN::ECSchemaCP> const& in)
                        {
                        for (ECN::ECSchemaCP schema : in)
                            {
                            if (schema->GetName().EqualsIAscii("Units") && schema->GetVersionRead() == 1 && schema->GetVersionWrite() == 0 && schema->GetVersionMinor() == 0)
                                m_unitsSchema = schema;
                            else if (schema->GetName().EqualsIAscii("Formats") && schema->GetVersionRead() == 1 && schema->GetVersionWrite() == 0 && schema->GetVersionMinor() == 0)
                                m_formatsSchema = schema;
                            else
                                out.push_back(schema);
                            }
                        }

                    void ClearCache() { m_unitsSchema = nullptr; m_formatsSchema = nullptr; }
                    bool IgnoreSchema(ECN::ECSchemaCR schema) const { return (m_unitsSchema != nullptr && &schema == m_unitsSchema) || (m_formatsSchema != nullptr && &schema == m_formatsSchema); }
                    bool IgnoreSchema(ECN::SchemaKeyCR schemaKey) const { return (m_unitsSchema != nullptr && schemaKey.CompareByName(m_unitsSchema->GetName()) == 0 && schemaKey.CompareByVersion(m_unitsSchema->GetSchemaKey()) == 0) || (m_formatsSchema != nullptr && schemaKey.CompareByName(m_formatsSchema->GetName()) == 0 && schemaKey.CompareByVersion(m_formatsSchema->GetSchemaKey()) == 0); }
                };

        private:
            SchemaImportContext& m_importCtx;

            ECN::SchemaDiff m_diff;
            bvector<ECN::ECSchemaCP> m_existingSchemas;
            bvector<ECN::ECSchemaCP> m_schemasToImport;
            bset<ECN::ECSchemaId> m_schemasWithMajorVersionChange;
            ECN::CustomAttributeValidator m_schemaUpgradeCustomAttributeValidator;

            bool m_ec32AvailableInFile = true;
            LegacyUnitsHelper m_legacyUnitsHelper;

            static bvector<ECN::ECSchemaCP> FindAllSchemasInGraph(bvector<ECN::ECSchemaCP> const&);
            static void FindAllSchemasInGraph(bmap<ECN::SchemaKey, ECN::ECSchemaCP, ECN::SchemaKeyLessThan<ECN::SchemaMatchType::Exact>>&, ECN::ECSchemaCP);
            static bmap<ECN::SchemaKey, ECN::ECSchemaCP, ECN::SchemaKeyLessThan<ECN::SchemaMatchType::Exact>> FindAllSchemasInGraph(ECN::ECSchemaCR, bool includeThisSchema);
            static bvector<ECN::ECSchemaCP> Sort(bvector<ECN::ECSchemaCP> const& in);
            static bvector<ECN::ECSchemaCP> GetNextLayer(bvector<ECN::ECSchemaCP> const&, bvector<ECN::ECSchemaCP> const& referencedBy);

        public:
            explicit Context(SchemaImportContext& ctx) : m_importCtx(ctx)
                {
                m_ec32AvailableInFile = FeatureManager::IsEC32Available(GetECDb());
                m_schemaUpgradeCustomAttributeValidator.AddRejectRule("CoreCustomAttributes:IsMixin.*");
                m_schemaUpgradeCustomAttributeValidator.AddRejectRule("ECDbMap:*");
                }

            BentleyStatus PreprocessSchemas(bvector<ECN::ECSchemaCP>& out, bvector<ECN::ECSchemaCP> const& in);

            void ClearCache() { m_schemasToImport.clear(); m_existingSchemas.clear(); m_legacyUnitsHelper.ClearCache(); GetECDb().ClearECDbCache(); }
            SchemaImportContext& ImportCtx() const { return m_importCtx; }
            bvector<ECN::ECSchemaCP> const& GetSchemasToImport() const { return m_schemasToImport; }
            bvector<ECN::ECSchemaCP>& GetSchemasToImportR() { return m_schemasToImport; }
            void AddSchemaToImport(ECN::ECSchemaCR schema) { m_schemasToImport.push_back(&schema); }
            bvector<ECN::ECSchemaCP> const& GetExistingSchemas() const { return m_existingSchemas; }
            bvector<ECN::ECSchemaCP>& GetExistingSchemasR() { return m_existingSchemas; }
            void AddExistingSchema(ECN::ECSchemaCR schema) { m_existingSchemas.push_back(&schema); }
            ECN::SchemaDiff& GetDiff() { return m_diff; }

            bool AreMajorSchemaVersionChangesAllowed() const { return !Enum::Contains(m_importCtx.GetOptions(), SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade); }
            bool IsMajorSchemaVersionChange(ECN::ECSchemaId schemaId) const { return m_schemasWithMajorVersionChange.find(schemaId) != m_schemasWithMajorVersionChange.end(); }
            bool IsEC32AvailableInFile() const { return m_ec32AvailableInFile; }
            void AddSchemaWithMajorVersionChange(ECN::ECSchemaId schemaId) { m_schemasWithMajorVersionChange.insert(schemaId); }
            CachedStatementPtr GetCachedStatement(Utf8CP sql) { return GetECDb().GetImpl().GetCachedSqliteStatement(sql); }
            ECDbCR GetECDb() const { return m_importCtx.GetECDb(); }
            MainSchemaManager const& GetSchemaManager() const { return m_importCtx.GetSchemaManager(); }
            ECN::CustomAttributeValidator const& GetSchemaUpgradeCustomAttributeValidator() const { return m_schemaUpgradeCustomAttributeValidator; }
            IssueReporter const& Issues() const { return GetECDb().GetImpl().Issues(); }

            LegacyUnitsHelper& LegacyUnitsHelper() { return m_legacyUnitsHelper; }
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
        static BentleyStatus ImportFormat(Context&, ECN::ECFormatCR);
        static BentleyStatus ImportFormatComposite(Context&, ECN::ECFormatCR, ECN::FormatId);
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

        static BentleyStatus InsertSchemaEntry(ECDbCR, ECN::ECSchemaCR);  //!< Also used by ProfileUpgrader_4002
        static BentleyStatus InsertBaseClassEntry(Context&, ECN::ECClassId, ECN::ECClassCR baseClass, int ordinal);
        static BentleyStatus InsertRelationshipConstraintEntry(Context&, ECRelationshipConstraintId& constraintId, ECN::ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECN::ECRelationshipEnd);
        static BentleyStatus InsertSchemaReferenceEntries(Context&, ECN::ECSchemaCR);
        static BentleyStatus InsertCAEntry(Context&, ECN::IECInstanceR customAttribute, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
        static BentleyStatus ReplaceCAEntry(Context&, ECN::IECInstanceR customAttribute, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, int ordinal);
        static BentleyStatus DeleteCAEntry(int& ordinal, Context&, ECN::ECClassId, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);

        static BentleyStatus UpdateRelationshipConstraint(Context&, ECContainerId, ECN::RelationshipConstraintChange&, ECN::ECRelationshipConstraintCR oldConstraint, ECN::ECRelationshipConstraintCR newConstraint, bool isSource, Utf8CP relationshipName);
        static BentleyStatus UpdateCustomAttributes(Context&, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType, ECContainerId, ECN::CustomAttributeChanges&, ECN::IECCustomAttributeContainerCR oldClass, ECN::IECCustomAttributeContainerCR newClass);
        static BentleyStatus UpdateClass(Context&, ECN::ClassChange&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);
        static BentleyStatus UpdateProperty(Context&, ECN::PropertyChange&, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty);
        static BentleyStatus UpdateSchema(Context&, ECN::SchemaChange&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateSchemaReferences(Context&, ECN::SchemaReferenceChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateClasses(Context&, ECN::ClassChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateEnumerations(Context&, ECN::EnumerationChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateEnumeration(Context&, ECN::EnumerationChange&, ECN::ECEnumerationCR oldEnum, ECN::ECEnumerationCR newEnum);
        static BentleyStatus VerifyEnumeratorChanges(Context&, ECN::ECEnumerationCR oldEnum, ECN::EnumeratorChanges&);

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
        static BentleyStatus UpdateFormats(Context&, ECN::FormatChanges&, ECN::ECSchemaCR oldSchema, ECN::ECSchemaCR newSchema);
        static BentleyStatus UpdateFormat(Context&, ECN::FormatChange&, ECN::ECFormatCR oldVal, ECN::ECFormatCR newVal);
        static BentleyStatus UpdateFormatCompositeUnitLabel(Context&, ECN::FormatId, ECN::StringChange& unitLabelChange, int ordinal);

        static BentleyStatus UpdateProperties(Context&, ECN::PropertyChanges&, ECN::ECClassCR oldClass, ECN::ECClassCR newClass);

        static BentleyStatus DeleteClass(Context&, ECN::ClassChange&, ECN::ECClassCR);
        static BentleyStatus DeleteProperty(Context&, ECN::PropertyChange&, ECN::ECPropertyCR);
        static BentleyStatus DeleteCustomAttributes(Context&, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType);
        static BentleyStatus DeleteInstances(Context&, ECN::ECClassCR);
        static BentleyStatus DeleteCustomAttributeClass(Context&, ECN::ECCustomAttributeClassCR);

        static bool IsSpecifiedInRelationshipConstraint(Context&, ECN::ECClassCR);

        static bool IsPropertyTypeChangeSupported(Utf8StringR error, ECN::StringChange& typeChange, ECN::ECPropertyCR oldProperty, ECN::ECPropertyCR newProperty);

        static BentleyStatus UpdateBaseClasses(Context&, ECN::BaseClassChanges&, ECN::ECClassCR, ECN::ECClassCR);
        static bool IsChangeToBaseClassIsSupported(ECN::ECClassCR baseClass);

        static BentleyStatus CompareSchemas(Context& ctx, bvector<ECN::ECSchemaCP> const& primarySchemasOrderedByDependencies);
        static BentleyStatus ReloadSchemas(Context& ctx);

    public:
        static BentleyStatus ImportSchemas(bvector<ECN::ECSchemaCP>& schemasToMap, SchemaImportContext&, bvector<ECN::ECSchemaCP> const& primarySchemasOrderedByDependencies);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
