/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************************************************************************
//DbTableSpace
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle       11/2017
//---------------------------------------------------------------------------------------
// Bentley coding guideline: don't free static non-POD members
//static
DbTableSpace const* DbTableSpace::s_main = new DbTableSpace(TABLESPACE_Main);

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle       11/2017
//---------------------------------------------------------------------------------------
DbTableSpace::DbTableSpace(Utf8CP name, Utf8CP filePath) : m_name(name), m_filePath(filePath)
    {
    BeAssert(!IsAny(name) && "Must not pass empty string to DbTableSpace");

    if (IsMain(name))
        {
        m_type = Type::Main;
        BeAssert(m_filePath.empty());
        }
    else if (IsTemp())
        {
        m_type = Type::Temp;
        BeAssert(m_filePath.empty());
        }
    else
        m_type = Type::Attached;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle       11/2018
//---------------------------------------------------------------------------------------
//static
bool DbTableSpace::IsMain(Utf8CP tableSpace) { return BeStringUtilities::StricmpAscii(tableSpace, TABLESPACE_Main) == 0; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle       11/2017
//---------------------------------------------------------------------------------------
//static
bool DbTableSpace::IsTemp(Utf8CP tableSpace) { return BeStringUtilities::StricmpAscii(tableSpace, TABLESPACE_Temp) == 0; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle       11/2017
//---------------------------------------------------------------------------------------
//static
bool DbTableSpace::Exists(ECDbCR ecdb, Utf8CP tableSpace)
    {
    if (IsAny(tableSpace))
        return false;

    return IsMain(tableSpace) || IsTemp(tableSpace) || DbUtilities::TableSpaceExists(ecdb, tableSpace);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle       12/2017
//---------------------------------------------------------------------------------------
//static
bool DbTableSpace::IsAttachedECDbFile(ECDbCR ecdb, Utf8CP tableSpace) 
    {
    if (IsMain(tableSpace) || IsTemp(tableSpace))
        return false;

    return DbUtilities::TableExists(ecdb, TABLE_Schema, tableSpace);
    }

//****************************************************************************************
// DbUtilities
//****************************************************************************************


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbUtilities::GetTableSpaces(std::vector<Utf8String>& tableSpaces, ECDbCR ecdb, bool onlyAttachedTableSpaces)
    {
    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement("pragma database_list");
    if (stmt == nullptr)
        return ERROR;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8CP tableSpace = stmt->GetValueText(1);
        if (onlyAttachedTableSpaces && DbTableSpace::IsMain(tableSpace) && DbTableSpace::IsTemp(tableSpace))
            continue;

        tableSpaces.push_back(tableSpace);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
//static
bool DbUtilities::TableSpaceExists(ECDbCR ecdb, Utf8CP tableSpace)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(tableSpace));
    if (DbTableSpace::IsMain(tableSpace) || DbTableSpace::IsTemp(tableSpace))
        return true;

    //cannot use cached statement here as TryPrepare is not available there
    Statement stmt;
    return BE_SQLITE_OK == stmt.TryPrepare(ecdb, Utf8PrintfString("SELECT 1 FROM [%s].sqlite_master", tableSpace).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   12/2017
//---------------------------------------------------------------------------------------
//static
Utf8String DbUtilities::GetAttachedFilePath(ECDbCR ecdb, Utf8CP tableSpace)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(tableSpace));
    if (DbTableSpace::IsMain(tableSpace) || DbTableSpace::IsTemp(tableSpace))
        return Utf8String();

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement("pragma database_list");
    if (stmt == nullptr)
        return Utf8String();

    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8CP actualTableSpace = stmt->GetValueText(1);
        if (BeStringUtilities::StricmpAscii(tableSpace, actualTableSpace) == 0)
            return Utf8String(stmt->GetValueText(2));
        }

    return Utf8String();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle         11/2017
//---------------------------------------------------------------------------------------
//static
 BentleyStatus DbUtilities::QueryRowClassId(ECN::ECClassId& classId, ECDbCR ecdb, Utf8StringCR tableName, Utf8StringCR classIdColName, Utf8StringCR pkColName, ECInstanceId id)
    {
    CachedStatementPtr statement = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT %s FROM %s WHERE %s=?", classIdColName.c_str(), tableName.c_str(), pkColName.c_str()).c_str());
    if (statement == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != statement->BindId(1, id))
        return ERROR;

    if (BE_SQLITE_ROW != statement->Step())
        classId = ECN::ECClassId();
    else
        classId = statement->GetValueId<ECN::ECClassId>(0);

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE