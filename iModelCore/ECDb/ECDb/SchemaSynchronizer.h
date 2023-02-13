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
    static DbResult GetECTables(DbR conn, std::vector<std::string>& tables, Utf8CP dbAlias);

public:
    static DbResult SyncData(ECDbR conn, Utf8CP syncDbUri, SyncAction action);
    static DbResult InitSynDb(ECDbR conn, Utf8CP syncDb);
};



// struct SyncDbInfo {
// 	private:
// 		constexpr static char* JPropertyName = "sync_info";
// 		constexpr static char* JNamespace = "ec_Db";
// 		constexpr static char* JSyncId = "id";
// 		constexpr static char* JCreatedOn = "created_on";
// 		constexpr static char* JModifiedOn = "modified_on";
// 		constexpr static char* JVer = "ver";

// 		BeGuid m_syncId;
// 		DateTime m_created;
// 		DateTime m_modified;
// 		BeInt64Id m_ver;

// 	public:
// 		static SyncDbInfo New() {
//         	SyncDbInfo info;
//             info.m_syncId.Create();
//             info.m_created = DateTime::GetCurrentTimeUtc();
//             info.m_modified = DateTime::GetCurrentTimeUtc();
//             info.m_ver = BeInt64Id(1u);
//             return info;
//         }
// 		DbResult WriteTo(DbR db) const {
// 			BeJsDocument doc;
// 			doc[JSyncId] = m_syncId.ToString();
// 			doc[JCreatedOn] = m_created.ToTimestampString();
// 			doc[JModifiedOn] = DateTime::GetCurrentTimeUtc().ToTimestampString();
// 			doc[JVer] = m_ver.ToHexStr();
// 			PropertySpec spec(JPropertyName, JNamespace);
// 			return db.SavePropertyString(spec, doc.Stringify());
// 		}
// 		DbResult ReadFrom(DbR db) {
// 			Utf8String strData;
// 			PropertySpec spec(JPropertyName, JNamespace);
// 			auto rc = db.QueryProperty(strData, spec);
// 			if (rc != BE_SQLITE_ROW) {
// 				return rc;
// 			}

// 			BeJsDocument doc;
//             doc.Parse(strData);
// 			if (!doc.isObject()){
//                 return BE_SQLITE_ERROR;
//             }

// 			if (doc.hasMember(JSyncId)) {
//             	m_syncId.FromString(doc[JSyncId].asCString());
// 			} else {
// 				 return BE_SQLITE_ERROR;
// 			}

// 			if (doc.hasMember(JCreatedOn)){
//             	m_created.FromString(doc[JCreatedOn].asCString());
// 			} else {
// 				 return BE_SQLITE_ERROR;
// 			}

// 			if (doc.hasMember(JModifiedOn)){
//             	m_modified.FromString(doc[JModifiedOn].asCString());
// 			} else {
// 				 return BE_SQLITE_ERROR;
// 			}
//             return BE_SQLITE_OK;
//         };

// };

END_BENTLEY_SQLITE_EC_NAMESPACE