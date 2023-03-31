/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <set>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct IntegrityChecker final {
    constexpr static auto check_profile_tables_and_indexes = "check_profile_tables_and_indexes";
	constexpr static auto check_data_tables_and_indexes = "check_data_tables_and_indexes";
	constexpr static auto check_data_table_columns = "check_data_table_columns";
	constexpr static auto check_nav_class_ids = "check_nav_class_ids";
	constexpr static auto check_nav_ids = "check_nav_ids";
	constexpr static auto check_linktable_source_and_target_class_ids = "check_linktable_source_and_target_class_ids";
	constexpr static auto check_linktable_source_and_target_ids = "check_linktable_source_and_target_ids";
	constexpr static auto check_entity_and_rel_class_ids = "check_entity_and_rel_class_ids";
	constexpr static auto check_schema_load = "check_schema_load";



    enum class Checks {
		None = 0x0,
		CheckProfileTablesAndIndexes = 0x1,
		CheckDataTablesAndIndexes = 0x2,
		CheckDataTableColumns = 0x4,
		CheckNavClassIds = 0x8,
		CheckNavIds = 0x10,
		CheckLinkTableSourceAndTargetClassIds = 0x20,
		CheckLinkTableSourceAndTargetIds = 0x40,
		CheckEntityAndRelClassIds = 0x80,
		CheckSchemaLoad = 0x100,
		OnlyMetaChecks = CheckProfileTablesAndIndexes | CheckDataTablesAndIndexes | CheckDataTableColumns | CheckSchemaLoad,
		OnlyDataChecks =CheckNavClassIds | CheckNavIds | CheckLinkTableSourceAndTargetClassIds | CheckLinkTableSourceAndTargetIds | CheckEntityAndRelClassIds,
		All = OnlyMetaChecks | OnlyDataChecks,
	};
private:
    ECDbCR m_conn;
    std::string m_lastError;
    bmap<Utf8CP, Checks, CompareIUtf8Ascii> m_nameToCheckId;
    bmap<Checks, Utf8CP> m_checkIdToName;

    DbResult GetTablePerHierarchyClasses(std::vector<ECClassId>&);
    DbResult GetRootLinkTableRelationships(std::vector<ECClassId>&);
    DbResult GetNavigationProperties(std::map<ECN::ECClassId, std::vector<std::string>>&);
    DbResult GetMappedClasses(std::set<ECN::ECClassId>&);

    DbResult CheckProfileTablesAndIndexes4002AndLater(std::function<bool(std::string, std::string, std::string)>);
    DbResult CheckProfileTablesAndIndexes4001AndOlder(std::function<bool(std::string, std::string, std::string)>);
    DbResult CheckProfileTablesAndIndexes(
		std::map<std::string, std::string> const&,
		std::map<std::string, std::string> const&,
		std::map<std::string, std::string> const&,
		std::function<bool(std::string, std::string, std::string)>
	);
    //! Callback(table)
    DbResult CheckDataTableExists(std::function<bool(std::string)>);
    //! Callback(index)
    DbResult CheckDataIndexExists(std::function<bool(std::string)>);

public:
    IntegrityChecker(ECDbCR conn):m_conn(conn){}
    static Utf8CP GetCheckName(Checks);
    static Checks GetCheckId(Utf8CP);
    static std::vector<Checks> GetChecks();
    std::string const& GetLastError() const { return m_lastError;  }
    //! Callback(table,column)
    DbResult CheckDataTableColumns(std::function<bool(std::string, std::string)>);
	//! Callback(name, type)
	DbResult CheckDataTablesAndIndexes(std::function<bool(std::string, std::string)>);
    //! Callback(type, name, issue)
    DbResult CheckProfileTablesAndIndexes(std::function<bool(std::string, std::string, std::string)>);
    // Callback(InstanceId, className, propertyName, id, primaryClassName)
    DbResult CheckNavIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP)>);
	// Callback(InstanceId,relName, propertyName, id, primaryClassName)
	DbResult CheckLinkTableSourceAndTargetIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP)>);
	// Callback(Utf8CP, InstanceId, classId)
    DbResult CheckEntityAndRelClassIds(std::function<bool(Utf8CP, ECInstanceId, ECN::ECClassId, Utf8CP)>);
	// Callback(InstanceId, relName, propertyName, Id, ClassId)
    DbResult CheckLinkTableSourceAndTargetClassIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId)>);
	// Callback(InstanceId, className, propertyName, navId, navClassId)
	DbResult CheckNavClassIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId)>);
	// Callback(schema)
    DbResult CheckSchemaLoad(std::function<bool(Utf8CP)>);
	// Callback(check-name, status)
    DbResult QuickCheck(Checks, std::function<void(Utf8CP, bool, BeDuration)>);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
