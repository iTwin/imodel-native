/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/LRPJobBackdoorAPI.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "LRPJobBackdoorAPI.h"
#include "../../../iModelHubClient/Utils.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
namespace LRPJobBackdoorAPI
    {
    Json::Value CreateLRPJobJson (Utf8StringCR jobId, Utf8StringCR iModelId)
        {
        Json::Value lrpJobInstanceJson = Json::objectValue;
        JsonValueR instance = lrpJobInstanceJson[ServerSchema::Instance] = Json::objectValue;
        instance[ServerSchema::SchemaName] = ServerSchema::Schema::Project;
        instance[ServerSchema::ClassName] = BackdoorAPISchema::Class::LRPJob;

        JsonValueR properties = instance[ServerSchema::Properties] = Json::objectValue;
        properties[BackdoorAPISchema::Property::LRPJobId] = jobId;
        properties[BackdoorAPISchema::Property::LRPiModelId] = iModelId;
        return lrpJobInstanceJson;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                             Robertas.Maleckas      11/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String ScheduleLRPJob (IWSRepositoryClientPtr projectConnection, Utf8StringCR jobId, Utf8StringCR iModelId)
        {
        Json::Value lrpJobInstanceJson = CreateLRPJobJson(jobId, iModelId);

        auto result = projectConnection->SendCreateObjectRequest(lrpJobInstanceJson, BeFileName (), nullptr, nullptr)->GetResult();

        EXPECT_SUCCESS(result);

        Json::Value resultJson;
        result.GetValue().GetJson(resultJson);

        JsonValueCR createdLrpJobInstance = resultJson[ServerSchema::ChangedInstance][ServerSchema::InstanceAfterChange];
        return createdLrpJobInstance[ServerSchema::InstanceId].asString();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                             Robertas.Maleckas      11/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    int QueryLRPJobState (IWSRepositoryClientPtr projectConnection, Utf8StringCR lrpJobRecordId)
        {
        ObjectId lrpJobRecordObjectId(ServerSchema::Schema::Project, BackdoorAPISchema::Class::LRPJob, lrpJobRecordId);

        auto result = projectConnection->SendGetObjectRequest(lrpJobRecordObjectId, nullptr, nullptr)->GetResult ();
        EXPECT_SUCCESS (result);
        
        RapidJsonValueCR properties = (*result.GetValue().GetInstances().begin()).GetProperties();
        return properties[BackdoorAPISchema::Property::LRPJobState].GetInt ();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                             Robertas.Maleckas      11/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool WaitForLRPJobToFinish (IWSRepositoryClientPtr projectConnection, Utf8StringCR lrpJobRecordId)
        {
        int retries = 300;

        BeDuration timer(BeDuration::FromMilliseconds (1000));
        while (--retries > 0)
            {
            int jobState = QueryLRPJobState (projectConnection, lrpJobRecordId);
            if (Completed == jobState)
                return true;
            if (Failed == jobState)
                return false;
            timer.Sleep ();
            }
        return false;
        }
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
