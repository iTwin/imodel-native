/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "ClassMap.h"
#include "DbMappingManager.h"
#include <ECObjects/SchemaComparer.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct SchemaPolicy
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

    private:
        SchemaPolicy(SchemaPolicy const&) = delete;
        SchemaPolicy& operator=(SchemaPolicy const&) = delete;

    protected:
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

        BentleyStatus Evaluate(ECDbCR, ECN::ECEntityClassCR) const;
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

struct MainSchemaManager;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext final
    {
    private:
        ECDbCR m_ecdb;
        SchemaManager::SchemaImportOptions m_options;
        ClassMapLoadContext m_loadContext;
        bset<ECN::ECClassId> m_classMapsToSave;
        FkRelationshipMappingInfo::Collection m_fkRelMappingInfos;
        SchemaPolicies m_schemaPolicies;
        bset<Utf8CP, CompareIUtf8Ascii> m_builtinSchemaNames;
       
    public:
        SchemaImportContext(ECDbCR ecdb, SchemaManager::SchemaImportOptions options) : m_ecdb(ecdb), m_options(options), m_builtinSchemaNames(ProfileManager::GetECDbSchemaNames()) {}
        SchemaManager::SchemaImportOptions GetOptions() const { return m_options; }

        FkRelationshipMappingInfo::Collection& GetFkRelationshipMappingInfos() { return m_fkRelMappingInfos; }

        ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
        void AddClassMapForSaving(ECN::ECClassId classId) { m_classMapsToSave.insert(classId); }
        bool ClassMapNeedsSaving(ECN::ECClassId classId) const { return m_classMapsToSave.find(classId) != m_classMapsToSave.end(); }

        SchemaPolicies const& GetSchemaPolicies() const { return m_schemaPolicies; }
        SchemaPolicies& GetSchemaPoliciesR() { return m_schemaPolicies; }

        bset<Utf8CP, CompareIUtf8Ascii> const& GetBuiltinSchemaNames() const { return m_builtinSchemaNames; }

        MainSchemaManager const& GetSchemaManager() const;
        ECDbCR GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const { return m_ecdb.GetImpl().Issues(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE