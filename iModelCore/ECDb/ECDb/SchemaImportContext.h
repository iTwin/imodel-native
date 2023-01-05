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
#include <re2/re2.h>
#include <numeric>

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

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct TransformData final: NonCopyableClass {
    struct Task final{
        private:
            Utf8String m_sql;
            Utf8String m_description;
            mutable bool m_executed;

        public:
            Task(Utf8String const& description, Utf8String const& sql) :m_description(description), m_sql(sql), m_executed(false){}
            Task(Task&& rhs) : m_sql(std::move(rhs.m_sql)),m_description(std::move(rhs.m_description)), m_executed(std::move(rhs.m_executed)){}
            Task(Task const& rhs) : m_sql(rhs.m_sql),m_description(rhs.m_description), m_executed(rhs.m_executed){}
            Task& operator=(Task&& rhs);
            Task& operator=(Task& rhs);
            Utf8StringCR GetSql() const { return m_sql; }
            Utf8StringCR GetDescription() const { return m_description; }
            bool IsExecuted() const { return m_executed;  }
            DbResult Execute(ECDbCR conn) const;
            bool Validate(ECDbCR conn) const;
    };
    private:
        std::vector<Task> m_transforms;
    public:
        void Append(Utf8StringCR description, Utf8String sql);
        DbResult Execute(ECDbCR conn) const;
        bool Validate(ECDbCR conn) const;
        bool IsEmpty() const { return m_transforms.empty(); }
        void ForEach(std::function<bool(Task const&)> cb);
};


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SqlTypeDetector {
    private:
        std::unique_ptr<RE2> m_alterTableRegEx;
        std::unique_ptr<RE2> m_createIndexRegEx;
        std::unique_ptr<RE2> m_createTableRegEx;
        std::unique_ptr<RE2> m_createViewRegEx;
        std::unique_ptr<RE2> m_dataDeleteRegEx;
        std::unique_ptr<RE2> m_dataInsertRegEx;
        std::unique_ptr<RE2> m_dataUpdateRegEx;
        std::unique_ptr<RE2> m_deleteRegEx;
        std::unique_ptr<RE2> m_dropIndexRegEx;
        std::unique_ptr<RE2> m_dropTableRegEx;
        std::unique_ptr<RE2> m_insertRegEx;
        std::unique_ptr<RE2> m_pragmaRegEx;
        std::unique_ptr<RE2> m_sysDeleteRegEx;
        std::unique_ptr<RE2> m_sysInsertRegEx;
        std::unique_ptr<RE2> m_sysUpdateRegEx;
        std::unique_ptr<RE2> m_updateRegEx;

        static const std::vector<std::string> GetDataTables (DbCR conn);
        static const std::vector<std::string> GetSystemTables (DbCR conn);
        static std::string Join(std::vector<std::string> const& v, const std::string sep);
        static void Validate(RE2 const& re);
        void SetupRegex(DbCR conn, bool useDataCRUD);
    public:
        explicit SqlTypeDetector(DbCR conn, bool useDataCRUD) { SetupRegex(conn, useDataCRUD); }
        bool IsAlterTable(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_alterTableRegEx); }
        bool IsCreateIndex(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_createIndexRegEx); }
        bool IsCreateTable(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_createTableRegEx); }
        bool IsCreateView(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_createViewRegEx); }
        bool IsDataDelete(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dataDeleteRegEx); }
        bool IsDataDml(Utf8CP sql) const { return IsDataInsert(sql) || IsDataUpdate(sql) || IsDataDelete(sql); }
        bool IsDataInsert(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dataInsertRegEx); }
        bool IsDataUpdate(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dataUpdateRegEx); }
        bool IsDdl(Utf8CP sql) const { return IsAlterTable(sql) || IsDropTable(sql) || IsDropIndex(sql) || IsCreateIndex(sql) || IsCreateTable(sql) || IsCreateView(sql); }
        bool IsDelete(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_deleteRegEx); }
        bool IsDml(Utf8CP sql) const { return IsInsert(sql) || IsUpdate(sql) || IsDelete(sql); }
        bool IsDropIndex(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dropIndexRegEx); }
        bool IsDropTable(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_dropTableRegEx); }
        bool IsInsert(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_insertRegEx); }
        bool IsPragma(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_pragmaRegEx); }
        bool IsSysDelete(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_sysDeleteRegEx); }
        bool IsSysDml(Utf8CP sql) const { return IsSysInsert(sql) || IsSysUpdate(sql) || IsSysDelete(sql); }
        bool IsSysInsert(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_sysInsertRegEx); }
        bool IsSysUpdate(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_sysUpdateRegEx); }
        bool IsUpdate(Utf8CP sql) const { return RE2::PartialMatch(sql, *m_updateRegEx); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DataChangeSqlListener final {

    private:
        ECDbCR m_conn;
        mutable std::vector<Utf8String> m_dataChangeSql;
        mutable std::vector<Utf8String> m_unknownSql;
        mutable cancel_callback_type m_cancelCb;
        SqlTypeDetector m_sqlDetector;
    public:
        DataChangeSqlListener(DataChangeSqlListener const&) = delete;
        DataChangeSqlListener& operator =(DataChangeSqlListener const&) = delete;
        explicit DataChangeSqlListener(ECDbCR conn);
        ~DataChangeSqlListener();
        DbResult Start(bool emptyCaptureBuffer = true) const;
        void Stop() const;
        std::vector<Utf8String> const& GetDataChangeSqlList() const { return  m_dataChangeSql; }
        std::vector<Utf8String> const& GetUnknownSqlList() const { return m_unknownSql; }
};

struct MainSchemaManager;

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext final
    {
    private:
        bset<ECN::ECClassId> m_classMapsToSave;
        bset<Utf8CP, CompareIUtf8Ascii> m_builtinSchemaNames;
        ClassMapLoadContext m_loadContext;
        ECDbCR m_ecdb;
        FkRelationshipMappingInfo::Collection m_fkRelMappingInfos;
        SchemaManager::SchemaImportOptions m_options;
        SchemaPolicies m_schemaPolicies;
        TransformData m_transformData;
        RemapManager m_remapManager;

    public:
        SchemaImportContext(ECDbCR ecdb, SchemaManager::SchemaImportOptions options)
            : m_ecdb(ecdb),
                m_options(options),
                m_builtinSchemaNames(ProfileManager::GetECDbSchemaNames()),
				m_remapManager(ecdb){}
        bool AllowDataTransform();
        bool ClassMapNeedsSaving(ECN::ECClassId classId) const { return m_classMapsToSave.find(classId) != m_classMapsToSave.end(); }
        bset<Utf8CP, CompareIUtf8Ascii> const& GetBuiltinSchemaNames() const { return m_builtinSchemaNames; }
        ClassMapLoadContext& GetClassMapLoadContext() { return m_loadContext; }
        ECDbCR GetECDb() const { return m_ecdb; }
        FkRelationshipMappingInfo::Collection& GetFkRelationshipMappingInfos() { return m_fkRelMappingInfos; }
        IssueDataSource const& Issues() const { return m_ecdb.GetImpl().Issues(); }
        MainSchemaManager const& GetSchemaManager() const;
        RemapManager& RemapManager() { return m_remapManager; }
        SchemaManager::SchemaImportOptions GetOptions() const { return m_options; }
        SchemaPolicies const& GetSchemaPolicies() const { return m_schemaPolicies; }
        SchemaPolicies& GetSchemaPoliciesR() { return m_schemaPolicies; }
        TransformData& GetDataTransform() {return m_transformData; }
        void AddClassMapForSaving(ECN::ECClassId classId) { m_classMapsToSave.insert(classId); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE