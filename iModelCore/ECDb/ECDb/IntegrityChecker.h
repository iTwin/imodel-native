/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <set>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct IntegrityChecker final {
    constexpr static auto check_ec_profile = "check_ec_profile";
    constexpr static auto check_data_schema = "check_data_schema";
    constexpr static auto check_data_columns = "check_data_columns";
    constexpr static auto check_nav_class_ids = "check_nav_class_ids";
    constexpr static auto check_nav_ids = "check_nav_ids";
    constexpr static auto check_linktable_fk_class_ids = "check_linktable_fk_class_ids";
    constexpr static auto check_linktable_fk_ids = "check_linktable_fk_ids";
    constexpr static auto check_class_ids = "check_class_ids";
    constexpr static auto check_schema_load = "check_schema_load";
    constexpr static auto check_missing_child_rows = "check_missing_child_rows";

    enum class Checks {
        None = 0x0,
        CheckEcProfile = 0x1,
        CheckDataSchema = 0x2,
        CheckDataColumns = 0x4,
        CheckNavClassIds = 0x8,
        CheckNavIds = 0x10,
        CheckLinkTableFkClassIds = 0x20,
        CheckLinkTableFkIds = 0x40,
        CheckClassIds = 0x80,
        CheckSchemaLoad = 0x100,
        CheckMissingChildRows = 0x200,
        OnlyMetaChecks = CheckEcProfile | CheckDataSchema | CheckDataColumns | CheckSchemaLoad,
        OnlyDataChecks = CheckNavClassIds | CheckNavIds | CheckLinkTableFkClassIds | CheckLinkTableFkIds | CheckClassIds,
        All = OnlyMetaChecks | OnlyDataChecks,
    };

   private:
    ECDbCR m_conn;
    std::string m_lastError;
    bmap<Utf8CP, Checks, CompareIUtf8Ascii> m_nameToCheckId;
    bmap<Checks, Utf8CP> m_checkIdToName;

    DbResult GetTablePerHierarchyClasses(std::vector<ECClassId>&);
    DbResult GetNavigationProperties(std::map<ECN::ECClassId, std::vector<std::string>>&);
    DbResult GetMappedClasses(std::set<ECN::ECClassId>&);

    DbResult CheckProfileTablesAndIndexes4002AndLater(std::function<bool(std::string, std::string, std::string)>);
    DbResult CheckProfileTablesAndIndexes4001AndOlder(std::function<bool(std::string, std::string, std::string)>);
    DbResult CheckEcProfile(
        std::map<std::string, std::string> const&,
        std::map<std::string, std::string> const&,
        std::map<std::string, std::string> const&,
        std::function<bool(std::string, std::string, std::string)>);
    //! Callback(table)
    DbResult CheckDataTableExists(std::function<bool(std::string)>);
    //! Callback(index)
    DbResult CheckDataIndexExists(std::function<bool(std::string)>);

   public:
    IntegrityChecker(ECDbCR conn) : m_conn(conn) {}
    static Utf8CP GetCheckName(Checks);
    static Checks GetCheckId(Utf8CP);
    static std::vector<Checks> GetChecks();
    std::string const& GetLastError() const { return m_lastError; }
    //! Callback(table,column)
    DbResult CheckDataColumns(std::function<bool(std::string, std::string)>);
    //! Callback(name, type)
    DbResult CheckDataSchema(std::function<bool(std::string, std::string)>);
    //! Callback(type, name, issue)
    DbResult CheckEcProfile(std::function<bool(std::string, std::string, std::string)>);
    // Callback(InstanceId, className, propertyName, id, primaryClassName)
    DbResult CheckNavIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP)>);
    // Callback(InstanceId,relName, propertyName, id, primaryClassName)
    DbResult CheckLinkTableFkIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP)>);
    // Callback(Utf8CP, InstanceId, classId)
    DbResult CheckClassIds(std::function<bool(Utf8CP, ECInstanceId, ECN::ECClassId, Utf8CP)>);
    // Callback(InstanceId, relName, propertyName, Id, ClassId)
    DbResult CheckLinkTableFkClassIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId)>);
    // Callback(InstanceId, className, propertyName, navId, navClassId)
    DbResult CheckNavClassIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId)>);
    // Callback(schema)
    DbResult CheckSchemaLoad(std::function<bool(Utf8CP)>);
    // Callback(Utf8CP, InstanceId, classId)
    DbResult CheckMissingChildRows(std::function<bool(Utf8CP, ECInstanceId, ECN::ECClassId, Utf8CP)>);
    // Callback(check-name, status)
    DbResult QuickCheck(Checks, std::function<void(Utf8CP, bool, BeDuration)>);
    DbResult GetRootLinkTableRelationships(std::vector<ECClassId>&);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
