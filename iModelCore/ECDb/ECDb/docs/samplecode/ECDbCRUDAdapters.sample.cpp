/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

void ShowPageInECDatagrid(bvector<IECInstancePtr> const& ecinstanceList);

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECInstanceECSqlSelectAdapter()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECInstanceECSqlSelectAdapter.sampleCode

    // Prepare statement
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, "SELECT FirstName, LastName, Birthday, Address FROM stco.Employee WHERE LastName LIKE 'S%' ORDER BY LastName, FirstName LIMIT 50");
    if (!stat.IsSuccess())
        {
        // do error handling here...
        return ERROR;
        }

    bvector<IECInstancePtr> pageOfECInstances;

    ECInstanceECSqlSelectAdapter adapter(statement);
    // Execute statement and step over each row of the result set
    while (BE_SQLITE_ROW == statement.Step())
        {
        pageOfECInstances.push_back(adapter.GetInstance());
        }

    // do something with the retrieved bunch of ECInstances
    ShowPageInECDatagrid(pageOfECInstances);

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_JsonECSqlBinder()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_JsonECSqlBinder.sampleCode

    // omit any error handling to focus on the JsonECSqlBinder usage

    ECSqlStatement statement;
    statement.Prepare(ecdb, "UPDATE stco.Employee SET Birthday=?, Address=? WHERE ECInstanceId=?");

    Json::Value birthdayJson("1971-04-30");
    JsonECSqlBinder::BindPrimitiveValue(statement.GetBinder(1), birthdayJson, PRIMITIVETYPE_DateTime);

    Json::Value addressJson;
    Json::Reader::Parse(R"json( { "street " : "2000 Main Street",
                                  "zip" : 94300,
                                  "city" : "MyTown" 
                                }
                               )json", addressJson);
    ECStructClassCP locationStruct = ecdb.Schemas().GetClass("StartupCompany", "Location")->GetStructClassCP();
    JsonECSqlBinder::BindStructValue(statement.GetBinder(2), addressJson, *locationStruct);

    statement.Step();

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

void SendToClient(Json::Value const& jsonPage);

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_JsonECSqlSelectAdapter()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_JsonECSqlSelectAdapter.sampleCode

    // Prepare statement
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, "SELECT FirstName, LastName, Birthday, Address FROM stco.Employee WHERE LastName LIKE 'S%' ORDER BY LastName, FirstName LIMIT 50");
    if (!stat.IsSuccess())
        {
        // do error handling here...
        return ERROR;
        }


    Json::Value jsonPage;

    JsonECSqlSelectAdapter adapter(statement);
    // Execute statement and step over each row of the result set
    while (BE_SQLITE_ROW == statement.Step())
        {
        Json::Value currentRow;
        if (SUCCESS != adapter.GetRow(currentRow))
            return ERROR;

        jsonPage.append(currentRow);
        }

    // do something with the page of JSON data
    SendToClient(jsonPage);

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

void GetPageOfECInstancesFromSomeOtherWorkflow(ECN::ECClassCP&, bvector<ECN::IECInstanceCP>&);
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECInstanceInserter()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECInstanceInserter.sampleCode

    ECN::ECClassCP ecClass = nullptr;
    bvector<ECN::IECInstanceCP> instanceList;
    GetPageOfECInstancesFromSomeOtherWorkflow(ecClass, instanceList);

    ECInstanceInserter inserter(ecdb, *ecClass, nullptr);
    if (!inserter.IsValid())
        //the inserter is not valid, if for example the ECClass passed is not instantiable (abstract class) or
        //if it is generally not usable in ECDb because it not mapped to a table (e.g. custom attributes)
        return ERROR;

    //now insert each instance from the list
    for (IECInstanceCP instance : instanceList)
        {
        ECInstanceKey generatedKey;
        const DbResult stat = inserter.Insert(generatedKey, *instance);
        if (BE_SQLITE_OK != stat)
            return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

void GetPageOfJsonInstancesFromSomeOtherWorkflow(ECN::ECClassCP&, bvector<Json::Value const*>&);
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_JsonInserter()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_JsonInserter.sampleCode

    ECN::ECClassCP ecClass = nullptr;
    bvector<Json::Value const*> jsonInstanceList;
    GetPageOfJsonInstancesFromSomeOtherWorkflow(ecClass, jsonInstanceList);

    JsonInserter inserter(ecdb, *ecClass, nullptr);
    if (!inserter.IsValid())
        //the inserter is not valid, if for example the ECClass passed is not instantiable (abstract class) or
        //if it is generally not usable in ECDb because it not mapped to a table (e.g. custom attributes)
        return ERROR;

    //now insert each JSON instance from the list
    for (Json::Value const* jsonInstance : jsonInstanceList)
        {
        ECInstanceKey generatedKey;
        const DbResult stat = inserter.Insert(generatedKey, *jsonInstance);
        if (BE_SQLITE_OK != stat)
            return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
