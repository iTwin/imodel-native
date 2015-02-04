/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/Connect/DatabaseHelper.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "DatabaseHelper.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define PROJECT_TIMESTAMP "@_timestamp_"


DbResult DatabaseHelper::PrepareAndBindStatement(Statement& sqlStatement,
        Db& db, Utf8CP sql, JsonValueCR bindings)
    {
    if (!(bindings.isNull() || bindings.isArray()))
        {
        return BE_SQLITE_MISUSE;
        }

    DbResult dbResult = sqlStatement.Prepare (db, sql);
    if (BE_SQLITE_OK != dbResult)
        {
        MOBILEDGN_LOGE ("Failed to prepare query \'%s\', error %d", sql, dbResult);
        return dbResult;
        }

    Json::FastWriter jsonWriter;

    if (bindings.isArray())
        {
        int bindIndex = 1;

        for (auto item : bindings)
            {
            if (item.isNull())
                {
                dbResult = sqlStatement.BindNull (bindIndex);
                }
            else if (item.isString())
                {
                dbResult = sqlStatement.BindText (bindIndex, item.asString(),
                    Statement::MakeCopy::Yes);
                }
            else if (item.isInt())
                {
                dbResult = sqlStatement.BindInt64 (bindIndex, item.asInt64());
                }
            else if (item.isDouble())
                {
                dbResult = sqlStatement.BindDouble (bindIndex, item.asDouble());
                }
            else
                {
                dbResult = sqlStatement.BindText (bindIndex, jsonWriter.write (item).c_str(),
                    Statement::MakeCopy::Yes);
                }

            if (BE_SQLITE_OK != dbResult)
                {
                MOBILEDGN_LOGE ("Failed to bind value %d in query \'%s\', error %d",
                        bindIndex, sql, dbResult);
                return dbResult;
                }

            bindIndex++;
            }
        }

    return BE_SQLITE_DONE;
    }

BeSQLite::DbResult DatabaseHelper::ExecuteStatement (BeSQLite::Db& db, Utf8CP sql)
    {
    Json::Value nullJson(Json::nullValue);
    return ExecuteStatement (db, sql, nullJson);
    }

DbResult DatabaseHelper::ExecuteStatement (Db& db, Utf8CP sql, JsonValueCR bindings)
    {
    Statement sqlStatement;
    DbResult dbResult = PrepareAndBindStatement (sqlStatement, db, sql, bindings);

    if (dbResult != BE_SQLITE_DONE)
        {
        return dbResult;
        }

    return sqlStatement.Step();
    }

DbResult DatabaseHelper::EnsureStorageTableExists (BeSQLite::Db& db, Utf8CP table)
    {
    DbResult dbResult;

    Utf8String persistenceTable;
    persistenceTable.Sprintf ("CREATE TABLE IF NOT EXISTS %s(key TEXT UNIQUE, value TEXT)", table);

    if (BE_SQLITE_DONE != (dbResult = ExecuteStatement (db, persistenceTable.c_str())))
        {
        MOBILEDGN_LOGE ("Failed to create table \'%s\'", table);
        }

    return dbResult;
    }

DbResult DatabaseHelper::Persist (Db& db, Utf8CP key, Utf8CP value,
        bool saveChanges /* = true */, Utf8CP table /* = DEFAULT_PERSISTENCE_TABLE */)
    {
    DbResult dbResult = EnsureStorageTableExists (db, table);
    if (BE_SQLITE_DONE != dbResult)
        {
        return dbResult;
        }

    Utf8String persistQuery;
    persistQuery.Sprintf ("INSERT OR REPLACE INTO %s(key, value) VALUES (?, ?)", table);

    Json::Value persistParams (Json::arrayValue);
    persistParams[0] = key;
    persistParams[1] = value;
    dbResult = ExecuteStatement (db, persistQuery.c_str(), persistParams);

    if (saveChanges)
        {
        if (dbResult == BE_SQLITE_DONE)
            {
            dbResult = db.SaveChanges();
            }
        else
            {
            db.AbandonChanges();
            return dbResult;
            }
        }

    return dbResult;
    }

DbResult DatabaseHelper::Retrieve (Db& db, Utf8CP key, Utf8StringR value, Utf8CP table /* = DEFAULT_PERSISTENCE_TABLE */)
    {
    DbResult dbResult = EnsureStorageTableExists (db, table);
    if (BE_SQLITE_DONE != dbResult)
        {
        return dbResult;
        }

    Utf8String query;
    query.Sprintf ("SELECT value FROM %s WHERE key=?", table);

    Utf8CP sql = query.c_str();

    Statement statement;
    dbResult = statement.Prepare (db, sql);
    if (BE_SQLITE_OK != dbResult)
        {
        MOBILEDGN_LOGE ("Failed to prepare query \'%s\', error %d", sql, dbResult);
        return dbResult;
        }

    dbResult = statement.BindText (1, key, Statement::MakeCopy::No);
    if (BE_SQLITE_OK != dbResult)
        {
        MOBILEDGN_LOGE ("Failed to bind value in query \'%s\', error %d", sql, dbResult);
        return dbResult;
        }

    dbResult = statement.Step();
    if (BE_SQLITE_DONE == dbResult)
        {
        return BE_SQLITE_NOTFOUND;
        }

    if (BE_SQLITE_ROW != dbResult)
        {
        MOBILEDGN_LOGE ("Failed to execute query \'%s\', error %d", sql, dbResult);
        return dbResult;
        }

    value = statement.GetValueText (0);

    return BE_SQLITE_DONE;
    }

BeSQLite::DbResult DatabaseHelper::RetrieveProjectObject (BeSQLite::Db& db, DgnDbR project,
                                                          Json::Value& dbObject)
    {
    dbObject.clear();

    Utf8String creationDate;
    DbResult dbResult = project.QueryProperty (creationDate, Properties::CreationDate());
    if (BE_SQLITE_ROW != dbResult)
        {
        MOBILEDGN_LOGW ("Cannot store value for project: creation date unknown");
        return dbResult;
        }

    dbResult = EnsureStorageTableExists (db, PROJECT_PERSISTENCE_TABLE);
    if (BE_SQLITE_DONE != dbResult)
        {
        return dbResult;
        }

    Utf8String dbValue;
    dbResult = Retrieve (db, project.GetDbFileName(), dbValue, PROJECT_PERSISTENCE_TABLE);

    if (BE_SQLITE_DONE != dbResult && BE_SQLITE_NOTFOUND != dbResult)
        {
        return dbResult;
        }

    if (BE_SQLITE_DONE == dbResult)
        {
        Json::Reader reader;
        if (reader.parse (dbValue.c_str(), dbObject))
            {
            if (dbObject.isMember(PROJECT_TIMESTAMP))
                {
                Json::Value persistedDate = dbObject[PROJECT_TIMESTAMP];
                if (persistedDate.isString() && creationDate.Equals (persistedDate.asString()))
                    {
                    return BE_SQLITE_DONE;
                    }
                }
            }
        }

    dbObject.clear();
    dbObject[PROJECT_TIMESTAMP] = creationDate;

    return BE_SQLITE_DONE;
    }

BeSQLite::DbResult DatabaseHelper::SetProjectValue (BeSQLite::Db& db, DgnDbR project, Utf8CP key,
                                                    const Json::Value& value)
    {
    Json::Value dbObject;
    DbResult dbResult = RetrieveProjectObject (db, project, dbObject);
    if (BE_SQLITE_DONE != dbResult)
        {
        return dbResult;
        }

    dbObject[key] = value;

    Json::FastWriter writer;
    return Persist(db, project.GetDbFileName(), writer.write (dbObject).c_str(), true,
        PROJECT_PERSISTENCE_TABLE);
    }

BeSQLite::DbResult DatabaseHelper::GetProjectValue (BeSQLite::Db& db, DgnDbR project, Utf8CP key,
                                                    Json::Value& value)
    {
    value = Json::nullValue;

    Json::Value dbObject;
    DbResult dbResult = RetrieveProjectObject (db, project, dbObject);
    if (BE_SQLITE_DONE != dbResult)
        {
        return dbResult;
        }

    if (!dbObject.isMember (key))
        {
        return BE_SQLITE_NOTFOUND;
        }

    value = dbObject[key];
    return BE_SQLITE_DONE;
    }
