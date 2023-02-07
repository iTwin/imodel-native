/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaManager.h>
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSynchronizer {
    using SyncAction = SchemaManager::SyncAction;
    using AliasMap = bmap<Utf8String, Utf8String, CompareIUtf8Ascii>;

private:
    static std::string Join(std::vector<std::string> const& list, std::string delimiter = ",", std::string prefix = "", std::string postfix = "");
    static std::string ToLower(std::string const& val);
    static DbResult TryGetAttachDbs(AliasMap& map, ECDbR conn);
    static DbResult GetColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames);
    static DbResult GetPrimaryKeyColumnNames(DbCR db, Utf8CP dbAlias, Utf8CP tableName, std::vector<std::string>& columnNames);
    static DbResult SyncData(ECDbR conn, std::vector<std::string> const& tables, Utf8CP sourceDbAlias, Utf8CP targetDbAlias = "main");
    static DbResult SyncData(ECDbR conn, Utf8CP tableName, Utf8CP sourceDbAlias, Utf8CP targetDbAlias = "main");
public:
    static DbResult SyncData(ECDbR conn, Utf8CP syncDbUri, SyncAction action, std::function<bool(const Utf8String&)> tableFilter, bool useNestedSafePoint = false);

};


END_BENTLEY_SQLITE_EC_NAMESPACE