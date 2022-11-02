/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "ClassMap.h"
#include "PropertyMap.h"
#include "DbMappingManager.h"
#include "RemapManager.h"
#include <ECObjects/SchemaComparer.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
struct SchemaImportContext;

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DataTransformTask {
    private:
        Utf8String m_description;
    public:
        explicit DataTransformTask(Utf8CP description):m_description(description){}
        virtual BentleyStatus Execute(ECDbCR ecdb) const { return ERROR;}
        Utf8StringCR GetDescription() const { return m_description;}
        virtual ~DataTransformTask(){}
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct GroupDataTransformTask final : DataTransformTask {
    private:
        Utf8String m_sql;
        bvector<std::unique_ptr<DataTransformTask>> m_taskGroup;
    public:
        explicit GroupDataTransformTask(Utf8CP description):DataTransformTask(description){}
        virtual BentleyStatus Execute(ECDbCR ecdb) const override {
            if (!m_taskGroup.empty()) {
                LOG.infov("Executing data transform task group: %s", GetDescription().c_str());
                for(auto& task: m_taskGroup){
                    if (task->Execute(ecdb)!= SUCCESS) {
                        ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "TransformTask failed: %s ", task->GetDescription().c_str());
                        return ERROR;
                    }
                }
            }
            return SUCCESS;
        }

        void AddTask(std::unique_ptr<DataTransformTask> task) {
            m_taskGroup.push_back(std::move(task));
        }
        virtual ~GroupDataTransformTask(){}
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SqlDataTransformTask final : DataTransformTask {
    private:
        Utf8String m_sql;
    public:
        explicit SqlDataTransformTask(Utf8CP description, Utf8CP sql):DataTransformTask(description), m_sql(sql){}
        virtual BentleyStatus Execute(ECDbCR ecdb) const override {
            LOG.infov("Executing data transform task: %s", GetDescription().c_str());
            auto rc = ecdb.TryExecuteSql(m_sql.c_str());
            if (rc != BE_SQLITE_OK) {
                Utf8String msg = SqlPrintfString("SqlTransformTask failed: %s - %s: %s", m_sql.c_str(), GetDescription().c_str(), BeSQLiteLib::GetLogError(rc).c_str()).GetUtf8CP();
                ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, msg.c_str());
                LOG.error(msg.c_str());
                BeAssert(false && "transform query failed");
                return ERROR;
            }
            return SUCCESS;
        }
        virtual ~SqlDataTransformTask(){}
};
//=======================================================================================
// @bsiclass
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
        GroupDataTransformTask m_transformTask;
        RemapManager m_remapManager;

    public:
        SchemaImportContext(ECDbCR ecdb, SchemaManager::SchemaImportOptions options) : m_ecdb(ecdb),m_transformTask("Schema import data transform task"), m_options(options), m_builtinSchemaNames(ProfileManager::GetECDbSchemaNames()), m_remapManager(ecdb) {}
        SchemaManager::SchemaImportOptions GetOptions() const { return m_options; }
        RemapManager& RemapManager() { return m_remapManager; }

        FkRelationshipMappingInfo::Collection& GetFkRelationshipMappingInfos() { return m_fkRelMappingInfos; }

        ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
        void AddClassMapForSaving(ECN::ECClassId classId) { m_classMapsToSave.insert(classId); }
        bool ClassMapNeedsSaving(ECN::ECClassId classId) const { return m_classMapsToSave.find(classId) != m_classMapsToSave.end(); }

        SchemaPolicies const& GetSchemaPolicies() const { return m_schemaPolicies; }
        SchemaPolicies& GetSchemaPoliciesR() { return m_schemaPolicies; }

        bset<Utf8CP, CompareIUtf8Ascii> const& GetBuiltinSchemaNames() const { return m_builtinSchemaNames; }

        MainSchemaManager const& GetSchemaManager() const;
        ECDbCR GetECDb() const { return m_ecdb; }
        IssueDataSource const& Issues() const { return m_ecdb.GetImpl().Issues(); }
        GroupDataTransformTask& GetDataTransfrom() {return m_transformTask; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE