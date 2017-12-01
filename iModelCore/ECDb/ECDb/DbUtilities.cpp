/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbUtilities.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
//static
DbTableSpace DbTableSpace::s_main(TABLESPACE_Main);

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle       11/2017
//---------------------------------------------------------------------------------------
DbTableSpace::DbTableSpace(Utf8CP name) : m_name(name)
    {
    BeAssert(!IsAny(name) && "Must not pass empty string to DbTableSpace");

    if (IsMain(name))
        m_type = Type::Main;
    else if (IsTemp())
        m_type = Type::Temp;
    else
        m_type = Type::Attached;
    }

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
bool DbTableSpace::IsAttachedECDb(ECDbCR ecdb) const { return IsAttached() && DbUtilities::TableExists(ecdb, TABLE_Schema, m_name.c_str()); }

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
// @bsimethod                                              Krischan.Eberle         11/2017
//---------------------------------------------------------------------------------------
//static
ECN::ECClassId DbUtilities::QueryRowClassId(ECDbCR ecdb, Utf8StringCR tableName, Utf8StringCR classIdColName, Utf8StringCR pkColName, ECInstanceId id)
    {
    CachedStatementPtr statement = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT %s FROM %s WHERE %s=?", classIdColName.c_str(), tableName.c_str(), pkColName.c_str()).c_str());
    BeAssert(statement.IsValid());

    if (BE_SQLITE_OK != statement->BindId(1, id))
        return ECN::ECClassId();

    if (BE_SQLITE_ROW != statement->Step())
        return ECN::ECClassId();

    return statement->GetValueId<ECN::ECClassId>(0);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE