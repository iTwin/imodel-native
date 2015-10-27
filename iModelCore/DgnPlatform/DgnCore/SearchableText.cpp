/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SearchableText.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

#define FTS_TABLE_Content  "dgn_fts_content"
#define FTS_TABLE_Index    "dgn_fts_idx"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSearchableText::Query::Query(Utf8StringCR matchExpression, Utf8CP type)
    : m_matchExpression(matchExpression)
    {
    m_matchExpression.Trim();
    if (nullptr != type)
        AddTextType(type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSearchableText::Query::AddTextType(Utf8CP type)
    {
    Utf8String textType(type);
    textType.Trim();
    BeAssert(!textType.empty());
    if (!textType.empty())
        {
        BeAssert(m_types.end() == std::find(m_types.begin(), m_types.end(), textType));
        if (m_types.end() == std::find(m_types.begin(), m_types.end(), textType))
            m_types.push_back(textType);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnSearchableText::Query::ToWhereClause() const
    {
    Utf8String typeSql;
    for (auto const& type : m_types)
        {
        if (!typeSql.empty())
            typeSql.append(1, ',');

        SqlPrintfString quotedType("%Q", type.c_str());
        typeSql.append(quotedType);
        }

    Utf8String sql(SqlPrintfString("WHERE (" FTS_TABLE_Index " MATCH %Q)", m_matchExpression.c_str()));
    switch (m_types.size())
        {
        case 0:
            break;
        case 1:
            sql.append(" AND Type=").append(typeSql);
            break;
        default:
            sql.append(" AND Type IN (").append(typeSql).append(1, ')');
            break;
        }

    return sql;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnSearchableText::QueryCount(Query const& query) const
    {
    Statement stmt;
    Utf8PrintfString sql("SELECT count(*) FROM " FTS_TABLE_Index " %s", query.ToWhereClause().c_str());
    if (BE_SQLITE_OK != stmt.Prepare(GetDgnDb(), sql.c_str()) || BE_SQLITE_ROW != stmt.Step())
        {
        BeAssert(false);
        return 0;
        }

    return stmt.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSearchableText::Iterator::const_iterator DgnSearchableText::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8PrintfString sql("SELECT Type,Id,Text FROM " FTS_TABLE_Index " %s", m_query.ToWhereClause().c_str());
        m_stmt = m_db->GetCachedStatement(sql.c_str());
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), m_stmt.IsValid() && BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSearchableText::Iterator DgnSearchableText::QueryRecords(Query const& query) const
    {
    return Iterator(GetDgnDb(), query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSearchableText::Record DgnSearchableText::QueryRecord(Key const& key) const
    {
    SqlPrintfString sql("SELECT Type,Id,Text FROM " FTS_TABLE_Index " WHERE Type=%Q AND Id=%llu", key.GetTextType().c_str(), key.GetId().GetValue());
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetDgnDb(), sql))
        BeAssert(false);
    else if (BE_SQLITE_ROW == stmt.Step())
        return Record(stmt.GetValueText(0), stmt.GetValueId<BeInt64Id>(1), stmt.GetValueText(2));

    return Record();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSearchableText::TextTypes DgnSearchableText::QueryTextTypes() const
    {
    TextTypes textTypes;
    Statement stmt;
    if (BE_SQLITE_OK == stmt.Prepare(GetDgnDb(), "SELECT DISTINCT Type FROM " FTS_TABLE_Index))
        {
        while (BE_SQLITE_ROW == stmt.Step())
            textTypes.push_back(stmt.GetValueText(0));
        }
    else
        {
        BeAssert(false);
        }

    return textTypes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::Insert(Record const& record)
    {
    BeAssert(record.IsValid());
    SqlPrintfString sql("INSERT INTO " FTS_TABLE_Content " (Type,Id,Text) VALUES (%Q,%llu,%Q)", record.GetTextType().c_str(), record.GetId().GetValue(), record.GetText().c_str());
    return GetDgnDb().ExecuteSql(sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::Update(Record const& record, Key const* pOriginalKey)
    {
    auto const& key = nullptr != pOriginalKey ? *pOriginalKey : record.GetKey();
    SqlPrintfString sql("UPDATE " FTS_TABLE_Content " SET Type=%Q,Id=%llu,Text=%Q WHERE Type=%Q AND Id=%llu",
                        record.GetTextType().c_str(), record.GetId().GetValue(), record.GetText().c_str(),
                        key.GetTextType().c_str(), key.GetId().GetValue());
    return GetDgnDb().ExecuteSql(sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::DropAll()
    {
    return GetDgnDb().ExecuteSql("DELETE FROM " FTS_TABLE_Content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::DropTextType(Utf8CP type)
    {
    SqlPrintfString sql("DELETE FROM " FTS_TABLE_Content " WHERE Type=%Q", type);
    return GetDgnDb().ExecuteSql(sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::DropRecord(Key const& key)
    {
    SqlPrintfString sql("DELETE FROM " FTS_TABLE_Content " WHERE Type=%Q AND Id=%llu", key.GetTextType().c_str(), key.GetId().GetValue());
    return GetDgnDb().ExecuteSql(sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::CreateTable(DgnDbR db)
    {
    // Create the content table
    DbResult rc = db.CreateTable(FTS_TABLE_Content, "Type NOT NULL,Id INTEGER NOT NULL,Text NOT NULL,PRIMARY KEY (Type,Id)");
    if (BE_SQLITE_OK != rc)
        return rc;

    // Create the virtual table
    rc = db.ExecuteSql("CREATE VIRTUAL TABLE " FTS_TABLE_Index " using fts5(Type UNINDEXED,Id UNINDEXED,Text,"
                       "content=" FTS_TABLE_Content ",content_rowid=rowid)");
    if (BE_SQLITE_OK != rc)
        return rc;

    // Create triggers to keep index in sync with content table
    rc = db.ExecuteSql("CREATE TRIGGER dgn_fts_ai AFTER INSERT ON " FTS_TABLE_Content
                       " BEGIN INSERT INTO " FTS_TABLE_Index "(rowid,Type,Id,Text) VALUES(new.rowid,new.Type,new.Id,new.Text); END;");
    if (BE_SQLITE_OK != rc)
        return rc;

    rc = db.ExecuteSql("CREATE TRIGGER dgn_fts_ad AFTER DELETE ON " FTS_TABLE_Content
                       " BEGIN INSERT INTO " FTS_TABLE_Index "(" FTS_TABLE_Index ",rowid,Type,Id,Text) VALUES('delete',old.rowid,old.Type,old.Id,old.Text); END;");
    if (BE_SQLITE_OK != rc)
        return rc;

    rc = db.ExecuteSql("CREATE TRIGGER dgn_fts_au AFTER UPDATE ON " FTS_TABLE_Content " BEGIN"
                       " INSERT INTO " FTS_TABLE_Index "(" FTS_TABLE_Index ",rowid,Type,Id,Text) VALUES('delete',old.rowid,old.Type,old.Id,old.Text);"
                       " INSERT INTO " FTS_TABLE_Index "(rowid,Type,Id,Text) VALUES(new.rowid,new.Type,new.Id,new.Text);"
                       " END;");

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnSearchableText::IsUntrackedFts5Table(Utf8CP tableName)
    {
    // Our virtual table and the additional indexes created by SQLite all start with the same prefix.
    // Our content table has a different prefix.
    // We only want the content table included in changesets.
    // Called by TxnManager::_FilterTable()
    return 0 == strncmp(FTS_TABLE_Index, tableName, sizeof(FTS_TABLE_Index)-1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnSearchableText::Iterator::Entry::GetTextType() const { return m_sql->GetValueText(0); }
BeInt64Id DgnSearchableText::Iterator::Entry::GetId() const { return m_sql->GetValueId<BeInt64Id>(1); }
Utf8CP DgnSearchableText::Iterator::Entry::GetText() const { return m_sql->GetValueText(2); }

