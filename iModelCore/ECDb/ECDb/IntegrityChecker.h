/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <set>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct IntegrityChecker final {
private:
    ECDbCR m_conn;
    std::string m_lastError;

    DbResult GetTablePerHierarchyClasses(std::vector<ECClassId>&);
    DbResult GetRootLinkTableRelationships(std::vector<ECClassId>&);
    DbResult GetNavigationProperties(std::map<ECN::ECClassId, std::vector<std::string>>&);
    DbResult GetMappedClasses(std::set<ECN::ECClassId>&);

    DbResult CheckProfileTablesAndIndexes4002AndLater(std::function<bool(std::string, std::string, std::string)>);
    DbResult CheckProfileTablesAndIndexes4001AndOlder(std::function<bool(std::string, std::string, std::string)>);
    DbResult CheckProfileTablesAndIndexes(
		std::map<std::string, std::string> const&,
		std::map<std::string, std::string> const&,
		std::function<bool(std::string, std::string, std::string)>
	);


public:
    IntegrityChecker(ECDbCR conn) : m_conn(conn) {}
    //! Callback(table,column)
    DbResult CheckDataTableColumnExists(std::function<bool(std::string, std::string)>);
    //! Callback(table)
    DbResult CheckDataTableExists(std::function<bool(std::string)>);
    //! Callback(index)
    DbResult CheckDataIndexExists(std::function<bool(std::string)>);
    //! Callback(type, name, issue)
    DbResult CheckProfileTablesAndIndexes(std::function<bool(std::string, std::string, std::string)>);
    // Callback(InstanceId, className, propertyName, id, primaryClassName)
    DbResult CheckSoftNavForeignKeyConstraint(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP)>);
	// Callback(InstanceId,relName, propertyName, id, primaryClassName)
	DbResult CheckSoftLinkTableForeignKeyConstraint(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP)>);
	// Callback(Utf8CP, InstanceId, classId)
    DbResult CheckClassIdForEntityClasses(std::function<bool(Utf8CP, ECInstanceId, ECN::ECClassId)>);
	// Callback(InstanceId, realName, propertyName, Id, ClassId)
    DbResult CheckClassIdForLinkTableRelationships(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId)>);
	// Callback(InstanceId, className, propertyName, navId, navClassId)
	DbResult CheckClassIdForNavProperties(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId)>);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
