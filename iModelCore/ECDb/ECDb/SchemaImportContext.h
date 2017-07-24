/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ClassMap.h"
#include "DbMappingManager.h"
#include "DbSchemaPersistenceManager.h"
#include <ECObjects/SchemaComparer.h>
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct SchemaPolicy : NonCopyableClass
    {
    enum class Type
        {
        NoAdditionalRootEntityClasses = 1,
        NoAdditionalLinkTables = 2,
        NoAdditionalForeignKeyConstraints = 4
        };

    protected:
        Type m_type;
        ECN::ECSchemaId m_optingInSchemaId;
        bset<ECN::ECSchemaId> m_schemaExceptions;
        bset<ECN::ECClassId> m_classExceptions;

        SchemaPolicy(Type type, ECN::ECSchemaId optingInSchemaId) : m_type(type), m_optingInSchemaId(optingInSchemaId) {}

        bool IsException(ECN::ECClassCR candidateClass) const;

        static BentleyStatus RetrieveExceptions(bvector<bvector<Utf8String>>&, ECN::IECInstanceCR policyCA, Utf8CP exceptionsPropName);

    public:
        virtual ~SchemaPolicy() {}

        template<typename TPolicy>
        TPolicy const& GetAs() const { BeAssert(dynamic_cast<TPolicy const*> (this) != nullptr);  return static_cast<TPolicy const&> (*this); }

        Type GetType() const { return m_type; }
        ECN::ECSchemaId GetOptingInSchemaId() const { return m_optingInSchemaId; }

        static Utf8CP TypeToString(Type);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct NoAdditionalRootEntityClassesPolicy final : SchemaPolicy
    {
    private:
        explicit NoAdditionalRootEntityClassesPolicy(ECN::ECSchemaId optingInSchemaId) : SchemaPolicy(Type::NoAdditionalRootEntityClasses, optingInSchemaId) {}

    public:
        static std::unique_ptr<SchemaPolicy> Create(ECDbCR, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions);
        ~NoAdditionalRootEntityClassesPolicy() {}

        BentleyStatus Evaluate(ECDbCR, ECN::ECClassCR) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct NoAdditionalLinkTablesPolicy final : SchemaPolicy
    {
    private:
        explicit NoAdditionalLinkTablesPolicy(ECN::ECSchemaId optingInSchemaId) : SchemaPolicy(Type::NoAdditionalLinkTables, optingInSchemaId) {}

    public:
        static std::unique_ptr<SchemaPolicy> Create(ECDbCR, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions);
        ~NoAdditionalLinkTablesPolicy() {}

        BentleyStatus Evaluate(ECDbCR, ECN::ECRelationshipClassCR) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct NoAdditionalForeignKeyConstraintsPolicy final : SchemaPolicy
    {
    private:
        bset<ECN::ECPropertyId> m_propertyExceptions;

        explicit NoAdditionalForeignKeyConstraintsPolicy(ECN::ECSchemaId optingInSchemaId) : SchemaPolicy(Type::NoAdditionalForeignKeyConstraints, optingInSchemaId) {}

        using SchemaPolicy::IsException;
        bool IsException(ECN::NavigationECPropertyCR navProp) const;

        BentleyStatus ReadExceptionsFromCA(ECDbCR ecdb, ECN::IECInstanceCR ca);

    public:
        static std::unique_ptr<SchemaPolicy> Create(ECDbCR, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions);
        ~NoAdditionalForeignKeyConstraintsPolicy() {}

        BentleyStatus Evaluate(ECDbCR, ECN::NavigationECPropertyCR) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct SchemaPolicies final
    {
    private:
        std::map<SchemaPolicy::Type, std::unique_ptr<SchemaPolicy>> m_optedInPolicies;

        BentleyStatus ReadPolicy(ECDbCR, ECN::ECSchemaCR, SchemaPolicy::Type, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions);

        static std::vector<ECN::ECSchemaId> GetSystemSchemaExceptions(ECDbCR ecdb);

    public:
        SchemaPolicies() {}

        BentleyStatus ReadPolicies(ECDbCR);
        bool IsOptedIn(SchemaPolicy const*&, SchemaPolicy::Type) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext final
    {
    enum class Phase
        {
        ImportingSchemas,
        MappingSchemas,
        MappingMixins,
        MappingEntities,
        MappingRelationships
        };

    private:
        ECDbCR m_ecdb;
        SchemaManager::SchemaImportOptions m_options;
        Phase m_phase = Phase::ImportingSchemas;

        ClassMapLoadContext m_loadContext;
        bset<ECN::ECClassId> m_classMapsToSave;
        FkRelationshipMappingInfo::Collection m_fkRelMappingInfos;
        SchemaPolicies m_schemaPolicies;

    public:
        SchemaImportContext(ECDbCR ecdb, SchemaManager::SchemaImportOptions options) : m_ecdb(ecdb), m_options(options) {}
        SchemaManager::SchemaImportOptions GetOptions() const { return m_options; }
        Phase GetPhase() const { return m_phase; }
        void SetPhase(Phase phase) { BeAssert(Enum::ToInt(m_phase) < Enum::ToInt(phase)); m_phase = phase; }

        FkRelationshipMappingInfo::Collection& GetFkRelationshipMappingInfos() { return m_fkRelMappingInfos; }

        ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
        void AddClassMapForSaving(ECN::ECClassId classId) { m_classMapsToSave.insert(classId); }
        bool ClassMapNeedsSaving(ECN::ECClassId classId) const { return m_classMapsToSave.find(classId) != m_classMapsToSave.end(); }

        SchemaPolicies const& GetSchemaPolicies() const { return m_schemaPolicies; }
        SchemaPolicies& GetSchemaPoliciesR() { return m_schemaPolicies; }

        DbMap const& GetDbMap() const { return m_ecdb.Schemas().GetDbMap(); }
        ECDbCR GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const { return m_ecdb.GetImpl().Issues(); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaCompareContext final
    {
    private:
        bvector<ECN::ECSchemaCP> m_existingSchemas;
        bvector<ECN::ECSchemaCP> m_schemasToImport;
        ECN::SchemaChanges m_changes;

    public:
        SchemaCompareContext() {}
        ~SchemaCompareContext() {}

        BentleyStatus Prepare(SchemaManager const&, bvector<ECN::ECSchemaCP> const& dependencyOrderedPrimarySchemas);
        bvector<ECN::ECSchemaCP>  const& GetSchemasToImport() const { return m_schemasToImport; }
        ECN::SchemaChanges& GetChanges() { return m_changes; }

        ECN::ECSchemaCP FindExistingSchema(Utf8CP schemaName) const;

        BentleyStatus ReloadContextECSchemas(SchemaManager const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE