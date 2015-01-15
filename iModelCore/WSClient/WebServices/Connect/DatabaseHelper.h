/*--------------------------------------------------------------------------------------+
 |
 |     $Source: WebServices/Connect/DatabaseHelper.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Common.h>
#include <MobileDgn/MobileDgnApplication.h>
#include <DgnPlatform/DgnCore/DgnProject.h>

#define DEFAULT_PERSISTENCE_TABLE "storage"
#define PROJECT_PERSISTENCE_TABLE "project_data"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct DatabaseHelper : NonCopyableClass
    {
private:

    static BeSQLite::DbResult EnsureStorageTableExists (BeSQLite::Db& db, Utf8CP table);
    static BeSQLite::DbResult RetrieveProjectObject (BeSQLite::Db& db, DgnProjectR project, Json::Value& dbObject);

public:

    // Prepare SQL statement and bind values provided in Json
    static BeSQLite::DbResult PrepareAndBindStatement(BeSQLite::Statement& sqlStatement,
            BeSQLite::Db& db, Utf8CP sql, JsonValueCR bindings);

    // Prepare and execute SQL statement
    static BeSQLite::DbResult ExecuteStatement (BeSQLite::Db& db, Utf8CP sql);

    // Prepare SQL statement, bind values provided in Json, and then execute
    static BeSQLite::DbResult ExecuteStatement (BeSQLite::Db& db, Utf8CP sql, JsonValueCR bindings);

    // Store key-value pair in a database. Later Retrieve can be used get the value using the key
    static BeSQLite::DbResult Persist (BeSQLite::Db& db, Utf8CP key, Utf8CP value,
            bool saveChanges = true, Utf8CP table = DEFAULT_PERSISTENCE_TABLE);

    // Get the value stored by Persist
    static BeSQLite::DbResult Retrieve (BeSQLite::Db& db, Utf8CP key, Utf8StringR value,
            Utf8CP table = DEFAULT_PERSISTENCE_TABLE);
    
    // Store key-value pair in a database ensuring that the value can only be retrieved
    // for the same project file
    static BeSQLite::DbResult SetProjectValue (BeSQLite::Db& db, DgnProjectR project,
        Utf8CP key, const Json::Value& value);

    // Get the value stored by SetProjectValue
    static BeSQLite::DbResult GetProjectValue (BeSQLite::Db& db, DgnProjectR project,
        Utf8CP key, Json::Value& value);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
