/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/docs/samplecode/ECDbCRUDAdapters.sample.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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


    Json::Value jsonPage(Json::arrayValue);

    JsonECSqlSelectAdapter adapter(statement);
    // Execute statement and step over each row of the result set
    while (BE_SQLITE_ROW == statement.Step())
        {
        Json::Value currentRow;
        bool stat = adapter.GetRow(currentRow);
        if (stat)
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

    ECInstanceInserter inserter(ecdb, *ecClass);
    if (!inserter.IsValid())
        //the inserter is not valid, if for example the ECClass passed is not instantiable (abstract class) or
        //if it is generally not usable in ECDb because it not mapped to a table (e.g. custom attributes)
        return ERROR;

    //now insert each instance from the list
    for (IECInstanceCP instance : instanceList)
        {
        ECInstanceKey generatedKey;
        BentleyStatus stat = inserter.Insert(generatedKey, *instance);
        if (stat != SUCCESS)
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

    JsonInserter inserter(ecdb, *ecClass);
    if (!inserter.IsValid())
        //the inserter is not valid, if for example the ECClass passed is not instantiable (abstract class) or
        //if it is generally not usable in ECDb because it not mapped to a table (e.g. custom attributes)
        return ERROR;

    //now insert each JSON instance from the list
    for (Json::Value const* jsonInstance : jsonInstanceList)
        {
        ECInstanceKey generatedKey;
        BentleyStatus stat = inserter.Insert(generatedKey, *jsonInstance);
        if (stat != SUCCESS)
            return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
