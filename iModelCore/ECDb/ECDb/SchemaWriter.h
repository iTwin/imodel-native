/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
            //! When importing EC3.1 schemas into EC3.1 files, ECDb has to ignore 
            //!     * EC3.2 units and formats schema which get deserialized automatically by ECObjects
            //!     * ECDb schemas that were accidentally deserialized from disk (and therefore are EC3.2) schemas.
            //! The latter is really a bug in client code and should be fixed in the client. ECDb can safely ignore
            //! them though so helps client code to not fail.
            //! This helper helps the respective schema writer code to ignore those schemas
            struct LegacySchemaImportHelper final
                {
                public:
                    enum class Action
                    {
                    Import,
                    Ignore,
                    UseExisting
                    };

                private:
                    bset<Utf8CP, CompareIUtf8Ascii> m_ignoredSchemaNames;
                    bset<Utf8CP, CompareIUtf8Ascii> m_ecdbSchemaNames;

                public:
                    LegacySchemaImportHelper() : m_ecdbSchemaNames(ProfileManager::GetECDbSchemaNames())
                        {
                        m_ignoredSchemaNames.insert("Units");
                        m_ignoredSchemaNames.insert("Formats");
                        }

                    void RemoveSchemasToSkip(bvector<ECN::ECSchemaCP>& out, bvector<ECN::ECSchemaCP> const& in)
                        {
                        for (ECN::ECSchemaCP schema : in)
                            {
                            if (!schema->OriginalECXmlVersionGreaterThan(ECN::ECVersion::V3_1))
                                {
                                out.push_back(schema);
                                continue;
                                }

                            if (GetImportAction(schema->GetSchemaKey()) == Action::Import)
                                out.push_back(schema);
                            }
                        }

                    Action GetImportAction(ECN::SchemaKeyCR schemaKey) const
                        {
                        if (m_ignoredSchemaNames.find(schemaKey.GetName().c_str()) != m_ignoredSchemaNames.end())
                            return Action::Ignore;

                        if (m_ecdbSchemaNames.find(schemaKey.GetName().c_str()) != m_ecdbSchemaNames.end())
                            return Action::UseExisting;

                        return Action::Import;
                        }
                };
            //! Implement reserved property name policy
            struct ReservedPropertyNamesPolicy
                {
                private:                   
                    typedef std::map<Utf8String, ECN::ECClassCP, CompareIUtf8Ascii> PolicyCacheEntry;
                    typedef std::map<ECN::IECInstanceCP, PolicyCacheEntry> PolicyCacheMap;
                    mutable PolicyCacheMap m_reservedProperties;
                    static ECN::ECClassCP FindCustomAttributeOwner(ECN::ECClassCP entityClass, ECN::IECInstanceCP ca);
                    PolicyCacheMap::iterator RegisterPolicy(IssueReporter const& issues, ECN::ECClassCR entityClass) const;
                    static void FindInhertiedPolicyCustomAttribute(std::vector<std::pair<ECN::ECClassCP, ECN::IECInstanceCP>>& policyAttributes, ECN::ECClassCP entityClass);
                public:
                    //! return true if policy voilation occure else false
                    bool Evaluate(IssueReporter const& issues, ECN::ECClassCR entityClass, ECN::ECPropertyCP property = nullptr) const;
                };

        private:
            SchemaImportContext& m_importCtx;

            ECN::SchemaDiff m_diff;
            bvector<ECN::ECSchemaCP> m_existingSchemas;
            bvector<ECN::ECSchemaCP> m_schemasToImport;
            bset<ECN::ECSchemaId> m_schemasWithMajorVersionChange;
            ECN::CustomAttributeValidator m_schemaUpgradeCustomAttributeValidator;
            ReservedPropertyNamesPolicy m_reservedPropertyNamePolicy;
            bool m_ec32AvailableInFile = true;
            LegacySchemaImportHelper m_legacyHelper;

            static bvector<ECN::ECSchemaCP> FindAllSchemasInGraph(bvector<ECN::ECSchemaCP> const&);
            static void FindAllSchemasInGraph(bmap<ECN::SchemaKey, ECN::ECSchemaCP, ECN::SchemaKeyLessThan<ECN::SchemaMatchType::Exact>>&, ECN::ECSchemaCP);
            static bmap<ECN::SchemaKey, ECN::ECSchemaCP, ECN::SchemaKeyLessThan<ECN::SchemaMatchType::Exact>> FindAllSchemasInGraph(ECN::ECSchemaCR, bool includeThisSchema);
            static bvector<ECN::ECSchemaCP> Sort(bvector<ECN::ECSchemaCP> const& in);
            static bvector<ECN::ECSchemaCP> GetNextLayer(bvector<ECN::ECSchemaCP> const&, bvector<ECN::ECSchemaCP> const& referencedBy);

        public:
            explicit Context(SchemaImportContext& ctx) : m_importCtx(ctx)
                {
                const auto ACCEPT = ECN::CustomAttributeValidator::Policy::Accept;
                const auto REJECT = ECN::CustomAttributeValidator::Policy::Reject;
                const auto ALL = ECN::CustomAttributeValidator::ChangeType::All;
                const auto MODIFIED = ECN::CustomAttributeValidator::ChangeType::Modified;
                const auto NEW = ECN::CustomAttributeValidator::ChangeType::New;
                auto& rules = m_schemaUpgradeCustomAttributeValidator;

                m_ec32AvailableInFile = FeatureManager::IsEC32Available(GetECDb());

                rules
                    .Append(ACCEPT, "ECDbMap", "LinkTableRelationshipMap", MODIFIED) // allow modification
                        .Append(ACCEPT, "AllowDuplicateRelationships", ALL); // only this property can be modified

                rules
                    .Append(ACCEPT, "ECDbMap", "DbIndexList", MODIFIED)
                        .Append(ACCEPT, "Indexes.*", NEW); // allow to add new Indexes

                rules
                    .Append(ACCEPT, "ECDbMap", "DbIndexList", MODIFIED)
                        .Append(ACCEPT, "Indexes.Properties", MODIFIED); // allow to modify Properties of existing Indexes

                rules
                    .Append(ACCEPT, "CoreCustomAttributes", "IsMixin", MODIFIED)
                        .Append(ACCEPT, "AppliesToEntityClass", MODIFIED); // allow to modify AppliesToEntityClass

                rules.Append(REJECT, "ECDbMap", "*", ALL);
                rules.Append(REJECT, "CoreCustomAttributes", "IsMixin", ALL);
                rules.Append(ACCEPT, "*", "*", ALL);
                }

            BentleyStatus PreprocessSchemas(bvector<ECN::ECSchemaCP>& out, bvector<ECN::ECSchemaCP> const& in);
            bool AssertReservedPropertyPolicy(ECN::ECClassCR entityClass, ECN::ECPropertyCP property = nullptr) const;
            void ClearCache() { m_schemasToImport.clear(); m_existingSchemas.clear(); GetECDb().ClearECDbCache(); }
            SchemaImportContext& ImportCtx() const { return m_importCtx; }
            bvector<ECN::ECSchemaCP> const& GetSchemasToImport() const { return m_schemasToImport; }
            bvector<ECN::ECSchemaCP>& GetSchemasToImportR() { return m_schemasToImport; }
            void AddSchemaToImport(ECN::ECSchemaCR schema) { m_schemasToImport.push_back(&schema); }
            bvector<ECN::ECSchemaCP> const& GetExistingSchemas() const { return m_existingSchemas; }
            bvector<ECN::ECSchemaCP>& GetExistingSchemasR() { return m_existingSchemas; }
            void AddExistingSchema(ECN::ECSchemaCR schema) { m_existingSchemas.push_back(&schema); }
            ECN::SchemaDiff& GetDiff() { return m_diff; }

            bool AreMajorSchemaVersionChangesAllowed() const { return !Enum::Contains(m_importCtx.GetOptions(), SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade); }
            bool IgnoreIllegalDeletionsAndModifications() const { return Enum::Contains(m_importCtx.GetOptions(), SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications); }
            bool IsMajorSchemaVersionChange(ECN::ECSchemaId schemaId) const { return m_schemasWithMajorVersionChange.find(schemaId) != m_schemasWithMajorVersionChange.end(); }
            bool IsEC32AvailableInFile() const { return m_ec32AvailableInFile; }
            void AddSchemaWithMajorVersionChange(ECN::ECSchemaId schemaId) { m_schemasWithMajorVersionChange.insert(schemaId); }
            CachedStatementPtr GetCachedStatement(Utf8CP sql) { return GetECDb().GetImpl().GetCachedSqliteStatement(sql); }
            ECDbCR GetECDb() const { return m_importCtx.GetECDb(); }
            MainSchemaManager const& GetSchemaManager() const { return m_importCtx.GetSchemaManager(); }
            ECN::CustomAttributeValidator const& GetSchemaUpgradeCustomAttributeValidator() const { return m_schemaUpgradeCustomAttributeValidator; }
            IssueReporter const& Issues() const { return GetECDb().GetImpl().Issues(); }

            LegacySchemaImportHelper& LegacySchemaImportHelper() { return m_legacyHelper; }
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
