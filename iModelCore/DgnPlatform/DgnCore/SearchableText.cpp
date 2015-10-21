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

#define FTS_TABLE__Content  "dgn_fts"
#define FTS_TABLE__Index    "dgn_ftsidx"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSearchableText::Query::Query(Utf8StringCR matchExpression, Utf8CP category)
    : m_matchExpression(matchExpression)
    {
    m_matchExpression.Trim();
    if (nullptr != category)
        AddCategory(category);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSearchableText::Query::AddCategory(Utf8CP cat)
    {
    Utf8String category(cat);
    category.Trim();
    BeAssert(!category.empty());
    if (!category.empty())
        {
        BeAssert(m_categories.end() == std::find(m_categories.begin(), m_categories.end(), category));
        if (m_categories.end() == std::find(m_categories.begin(), m_categories.end(), category))
            m_categories.push_back(category);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnSearchableText::Query::ToWhereClause() const
    {
    Utf8String categorySql;
    for (auto const& category : m_categories)
        {
        if (!categorySql.empty())
            categorySql.append(1, ',');

        SqlPrintfString cat("%Q", category.c_str());
        categorySql.append(cat);
        }

    Utf8String sql(SqlPrintfString("WHERE (" FTS_TABLE__Index " MATCH %Q)", m_matchExpression.c_str()));
    switch (m_categories.size())
        {
        case 0:
            break;
        case 1:
            sql.append(" AND Category=").append(categorySql);
            break;
        default:
            sql.append(" AND Category IN (").append(categorySql).append(1, ')');
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
    Utf8PrintfString sql("SELECT count(*) FROM " FTS_TABLE__Index " %s", query.ToWhereClause().c_str());
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
        Utf8PrintfString sql("SELECT Category,Id,Text FROM " FTS_TABLE__Index " %s", m_query.ToWhereClause().c_str());
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
    SqlPrintfString sql("SELECT Category,Id,Text FROM " FTS_TABLE__Index " WHERE Category=%Q AND Id=%llu", key.GetCategory().c_str(), key.GetId().GetValue());
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
DgnSearchableText::Categories DgnSearchableText::QueryCategories() const
    {
    Categories categories;
    Statement stmt;
    if (BE_SQLITE_OK == stmt.Prepare(GetDgnDb(), "SELECT DISTINCT Category FROM " FTS_TABLE__Index))
        {
        while (BE_SQLITE_ROW == stmt.Step())
            categories.push_back(stmt.GetValueText(0));
        }
    else
        {
        BeAssert(false);
        }

    return categories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::Insert(Record const& record)
    {
    BeAssert(record.IsValid());
    SqlPrintfString sql("INSERT INTO " FTS_TABLE__Content " (Category,Id,Text) VALUES (%Q,%llu,%Q)", record.GetCategory().c_str(), record.GetId().GetValue(), record.GetText().c_str());
    return GetDgnDb().ExecuteSql(sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::Update(Record const& record, Key const* pOriginalKey)
    {
    auto const& key = nullptr != pOriginalKey ? *pOriginalKey : record.GetKey();
    SqlPrintfString sql("UPDATE " FTS_TABLE__Content " SET Category=%Q,Id=%llu,Text=%Q WHERE Category=%Q AND Id=%llu",
                        record.GetCategory().c_str(), record.GetId().GetValue(), record.GetText().c_str(),
                        key.GetCategory().c_str(), key.GetId().GetValue());
    return GetDgnDb().ExecuteSql(sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::DropAll()
    {
    return GetDgnDb().ExecuteSql("DELETE FROM " FTS_TABLE__Content);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::DropCategory(Utf8CP cat)
    {
    SqlPrintfString sql("DELETE FROM " FTS_TABLE__Content " WHERE Category=%Q", cat);
    return GetDgnDb().ExecuteSql(sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::DropRecord(Key const& key)
    {
    SqlPrintfString sql("DELETE FROM " FTS_TABLE__Content " WHERE Category=%Q AND Id=%llu", key.GetCategory().c_str(), key.GetId().GetValue());
    return GetDgnDb().ExecuteSql(sql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnSearchableText::CreateTable(DgnDbR db)
    {
    // Create the content table
    DbResult rc = db.CreateTable(FTS_TABLE__Content, "Category NOT NULL,Id INTEGER NOT NULL,Text NOT NULL,PRIMARY KEY (Category,Id)");
    if (BE_SQLITE_OK != rc)
        return rc;

    // Create the virtual table
    rc = db.ExecuteSql("CREATE VIRTUAL TABLE " FTS_TABLE__Index " using fts5(Category UNINDEXED,Id UNINDEXED,Text,"
                       "content=" FTS_TABLE__Content ",content_rowid=rowid)");
    if (BE_SQLITE_OK != rc)
        return rc;

    // Create triggers to keep index in sync with content table
    rc = db.ExecuteSql("CREATE TRIGGER be_fts_ai AFTER INSERT ON " FTS_TABLE__Content
                       " BEGIN INSERT INTO " FTS_TABLE__Index "(rowid,Category,Id,Text) VALUES(new.rowid,new.Category,new.Id,new.Text); END;");
    if (BE_SQLITE_OK != rc)
        return rc;

    rc = db.ExecuteSql("CREATE TRIGGER be_fts_ad AFTER DELETE ON " FTS_TABLE__Content
                       " BEGIN INSERT INTO " FTS_TABLE__Index "(" FTS_TABLE__Index ",rowid,Category,Id,Text) VALUES('delete',old.rowid,old.Category,old.Id,old.Text); END;");
    if (BE_SQLITE_OK != rc)
        return rc;

    rc = db.ExecuteSql("CREATE TRIGGER be_fts_au AFTER UPDATE ON " FTS_TABLE__Content " BEGIN"
                       " INSERT INTO " FTS_TABLE__Index "(" FTS_TABLE__Index ",rowid,Category,Id,Text) VALUES('delete',old.rowid,old.Category,old.Id,old.Text);"
                       " INSERT INTO " FTS_TABLE__Index "(rowid,Category,Id,Text) VALUES(new.rowid,new.Category,new.Id,new.Text);"
                       " END;");

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnSearchableText::Iterator::Entry::GetCategory() const { return m_sql->GetValueText(0); }
BeInt64Id DgnSearchableText::Iterator::Entry::GetId() const { return m_sql->GetValueId<BeInt64Id>(1); }
Utf8CP DgnSearchableText::Iterator::Entry::GetText() const { return m_sql->GetValueText(2); }

