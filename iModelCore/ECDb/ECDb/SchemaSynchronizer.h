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
    static DbResult SyncData(ECDbR conn, Utf8CP syncDbUri, SyncAction action, bool verifySynDb = true, std::vector<std::string> const& additionTables = std::vector<std::string>());
    static DbResult InitSynDb(ECDbR conn, Utf8CP syncDb);


};


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaSyncInfo final {
	private:
		constexpr static char JCreatedOnUtc[] = "created_on_utc";
		constexpr static char JModifiedOnUtc[] = "modified_on_utc";
		constexpr static char JSyncId[] = "id";
		constexpr static char JVersion[] = "version";
        constexpr static char JPropertyName[] = "sync_info";
        constexpr static char JPropertyNamespace[] = "ec_Db";
        constexpr static char JSourceDbGuid[] = "source_db_guid";
        constexpr static char JSourceProjectGuid[] = "source_project_guid";

		BeGuid m_syncId;
		DateTime m_created;
		DateTime m_modified;
		BeInt64Id m_version;
        BeGuid m_sourceProjectGuid;
        BeGuid m_sourceDbGuid;

        SchemaSyncInfo(){}

	public:
        SchemaSyncInfo(SchemaSyncInfo&&) = default;
        SchemaSyncInfo(SchemaSyncInfo const&) = default;
        SchemaSyncInfo& operator =(SchemaSyncInfo&&) = default;
        SchemaSyncInfo& operator =(SchemaSyncInfo const&) = default;
		DbResult SaveTo(DbR syncDb) const;
        BeGuidCR GetSyncId() const { return m_syncId; }
		DateTimeCR GetCreatedOn() const { return m_created; }
		DateTimeCR GetModifiedOn() const {return m_modified; }
		BeInt64Id GetVersion() const { return m_version; }
        BeGuidCR GetSourceProjectGuid() const { return m_sourceProjectGuid; }
        BeGuidCR GetSourceDbGuid() const {return m_sourceDbGuid; }
        bool IsEmpty() const {return !m_syncId.IsValid();}
        void SetSource(DbCR db);
        static SchemaSyncInfo New();
        static SchemaSyncInfo From(DbCR);
        static SchemaSyncInfo const& Empty();

};

END_BENTLEY_SQLITE_EC_NAMESPACE