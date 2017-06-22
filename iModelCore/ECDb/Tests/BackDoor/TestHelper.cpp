/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/TestHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/TestHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE


//************************************************************************************
// TestHelper
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::ImportSchema(SchemaItem const& schema, Utf8CP fileName /*= nullptr*/)
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(ecdb, fileName))
        return ERROR;

    return ImportSchema(ecdb, schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::ImportSchemas(std::vector<SchemaItem> const& schemas, Utf8CP fileName /*= nullptr*/)
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(ecdb, fileName))
        return ERROR;

    return ImportSchemas(ecdb, schemas);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestHelper::ImportSchemas(ECDbR ecdb, std::vector<SchemaItem> const& schemas)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());
    for (SchemaItem const& schema : schemas)
        {
        if (SUCCESS != ECDbTestFixture::ReadECSchema(context, ecdb, schema))
            return ERROR;
        }

    Savepoint sp(ecdb, "ECSchema Import");
    if (SUCCESS == ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        sp.Commit();
        return !HasDataCorruptingMappingIssues(ecdb) ? SUCCESS : ERROR;
        }

    sp.Cancel();
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::ImportSchema(ECDbR ecdb, SchemaItem const& testItem)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());
    if (SUCCESS != ECDbTestFixture::ReadECSchema(context, ecdb, testItem))
        return ERROR;

    Savepoint sp(ecdb, "ECSchema Import");
    if (SUCCESS == ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        sp.Commit();
        return !HasDataCorruptingMappingIssues(ecdb) ? SUCCESS : ERROR;
        }

    sp.Cancel();
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                       02/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestHelper::HasDataCorruptingMappingIssues(ECDbCR ecdb)
    {
    EXPECT_TRUE(ecdb.IsDbOpen());

    if (!ecdb.IsDbOpen())
        return true;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, SchemaManager::GetValidateDbMappingSql()))
        {
        EXPECT_TRUE(false) << ecdb.GetLastError().c_str();
        return true;
        }

    bool hasError = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        hasError = true;
        LOG.errorv("ECClass '%s:%s' with invalid mapping: %s. Table name: %s - %s", stmt.GetValueText(0),
                   stmt.GetValueText(2), stmt.GetValueText(5), stmt.GetValueText(3), stmt.GetValueText(6));
        }

    return hasError;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String TestHelper::RetrieveDdl(ECDbCR ecdb, Utf8CP entityName, Utf8CP entityType)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT sql FROM sqlite_master WHERE name=? COLLATE NOCASE AND type=? COLLATE NOCASE");
    if (stmt == nullptr)
        return Utf8String();

    if (BE_SQLITE_OK != stmt->BindText(1, entityName, Statement::MakeCopy::No))
        return Utf8String();

    if (BE_SQLITE_OK != stmt->BindText(2, entityType, Statement::MakeCopy::No))
        return Utf8String();

    if (BE_SQLITE_ROW != stmt->Step())
        return Utf8String();

    return Utf8String(stmt->GetValueText(0));
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult TestHelper::ExecuteNonSelectECSql(ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
        return BE_SQLITE_ERROR;

    LOG.debugv("ECSQL %s -> SQL %s", ecsql, stmt.GetNativeSql());
    return stmt.Step();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult TestHelper::ExecuteInsertECSql(ECInstanceKey& key, ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
        return BE_SQLITE_ERROR;

    LOG.debugv("ECSQL %s -> SQL %s", ecsql, stmt.GetNativeSql());
    return stmt.Step(key);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestHelper::IndexExists(ECDbCR ecdb, IndexInfo const& expectedIndex)
    {
    return expectedIndex.ToDdl().EqualsIAscii(RetrieveIndexDdl(ecdb, expectedIndex.GetName()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::vector<Utf8String> TestHelper::RetrieveIndexNamesForTable(ECDbCR ecdb, Utf8StringCR tableName)
    {
    std::vector<Utf8String> indexNames;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT name FROM sqlite_master WHERE type='index' AND tbl_name=? ORDER BY name"))
        {
        BeAssert(false && "Preparation failed");
        return indexNames;
        }

    if (BE_SQLITE_OK != stmt.BindText(1, tableName, Statement::MakeCopy::No))
        {
        BeAssert(false && "Preparation failed");
        return indexNames;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        indexNames.push_back(stmt.GetValueText(0));
        }

    return indexNames;
    }

//*************************************************************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 05/17
//+---------------+---------------+---------------+---------------+---------------+------
bool operator==(ColumnInfo::List const& lhs, ColumnInfo::List const& rhs)
    {
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++)
        {
        if (lhs[i] != rhs[i])
            return false;
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 05/17
//+---------------+---------------+---------------+---------------+---------------+------
bool operator!=(ColumnInfo::List const& lhs, ColumnInfo::List const& rhs) { return !(lhs == rhs); }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ColumnInfo::List const& colInfos, std::ostream* os)
    {
    *os << "[";
    bool isFirstItem = true;
    for (ColumnInfo const& colInfo : colInfos)
        {
        if (!isFirstItem)
            *os << ",";

        *os << colInfo.ToString().c_str();
        isFirstItem = false;
        }
    *os << "]";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ColumnInfo const& colInfo, std::ostream* os)
    {
    *os << colInfo.ToString().c_str();
    }

//******************* IndexInfo **********************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String IndexInfo::ToDdl() const
    {
    Utf8String ddl("CREATE ");
    if (m_isUnique)
        ddl.append("UNIQUE ");

    ddl.append("INDEX [").append(m_name).append("] ON [").append(m_table).append("](");
    bool isFirstColumn = true;
    for (Utf8StringCR column : m_indexColumns)
        {
        if (!isFirstColumn)
            ddl.append(", ");

        ddl.append("[").append(column).append("]");
        isFirstColumn = false;
        }

    ddl.append(")");
    if (m_whereClause.IsDefined())
        ddl.append(" WHERE ").append(m_whereClause.ToDdl());

    return ddl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
void IndexInfo::WhereClause::AppendClassIdFilter(std::vector<ECN::ECClassId> const& classIdFilter, bool negatedClassIdFilter /*= false*/)
    {
    if (classIdFilter.empty())
        return;

    bool isFirstClassId = true;
    for (ECN::ECClassId classId : classIdFilter)
        {
        if (!isFirstClassId)
            m_whereClause.append(negatedClassIdFilter ? " AND " : " OR ");

        m_whereClause.append("ECClassId").append(negatedClassIdFilter ? "<>" : "=").append(classId.ToString());
        isFirstClassId = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
void IndexInfo::WhereClause::AppendNotNullFilter(std::vector<Utf8String> const& indexColumns)
    {
    if (indexColumns.empty())
        return;

    bool isFirstCol = true;
    for (Utf8StringCR indexColumn : indexColumns)
        {
        if (!isFirstCol)
            m_whereClause.append(" AND ");

        m_whereClause.append(indexColumn).append(" IS NOT NULL ");
        isFirstCol = false;
        }
    }

END_ECDBUNITTESTS_NAMESPACE