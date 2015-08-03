/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Examples/CachingExample.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "CachingExample.h"

#include <Bentley/BeDebugLog.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define SEPERATOR "\n-------------------------- CachingExample --------------------------\n"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachingExample::CachingExample(BeFileName workDir) :
m_workDir(workDir.AppendToPath(L"CachingExample/")),
m_cancellationToken(SimpleCancellationToken::Create())
    {
    Cleanup();
    BeFileName::CreateNewDirectory(m_workDir);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachingExample::~CachingExample()
    {
    Cleanup();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::Cleanup()
    {
    // Delete any sample data from disk
    BeFileName::EmptyAndRemoveDirectory(m_workDir);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClientInfoPtr CachingExample::GetClientInfo()
    {
    // ClientInfo should be defined by application that executes requests.
    static auto clientInfo = ClientInfo::Create("Bentley-CachingExample", BeVersion(1, 0), "TestAppId");
    return clientInfo;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheEnvironment CachingExample::GetCacheEnvironment()
    {
    CacheEnvironment environment;

    environment.persistentFileCacheDir.AppendToPath(m_workDir).AppendToPath(L"safeLocation").AppendSeparator();       // Folder that will not be managed any other application
    environment.temporaryFileCacheDir.AppendToPath(m_workDir).AppendToPath(L"temporaryLocation").AppendSeparator();   // Usually inside platform spectific "temp" folder

    return environment;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::HandleGetRepositoriesSuccessResponse(const bvector<WSRepository>& repositories)
    {
    Utf8String message;

    message += SEPERATOR;
    message += Utf8PrintfString("Server returned %d repositories:\n", repositories.size());

    for (const WSRepository& repository : repositories)
        {
        message += Utf8PrintfString(
            "Repository:\n"
            "    Id:                        '%s'\n"
            "    PluginId:                  '%s'\n"
            "    Location:                  '%s'\n"
            "    User-friendly label:       '%s'\n"
            "    User-friendly description: '%s'\n",
            repository.GetId().c_str(),
            repository.GetPluginId().c_str(),
            repository.GetLocation().c_str(),
            repository.GetLabel().c_str(),
            repository.GetDescription().c_str());
        }

    message += SEPERATOR;
    BeDebugLog(message.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::HandleObjects(JsonValueCR data)
    {
    Utf8String message;
    message += SEPERATOR;

    if (data.isArray())
        {
        message += Utf8PrintfString("Got %d object instances:\n", data.size());
        }
    else
        {
        message += "Got 1 object instance:\n";
        }

    message += Json::StyledWriter().write(data);

    message += SEPERATOR;
    BeDebugLog(message.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CachingExample::GetWSErrorMessage(WSErrorCR error)
    {
    WSError::Id errorId = error.GetId();

    return Utf8PrintfString(
        "WSError:\n"
        "    DisplayMessage:            '%s'\n"
        "    DisplayDescription:        '%s'\n"
        "    ServerErrorId:              %d",
        error.GetDisplayMessage().c_str(),
        error.GetDisplayDescription().c_str(),
        errorId
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CachingExample::GetCachingStatusString(CachingDataSource::ErrorCR error)
    {
    CachingDataSource::Status status = error.GetStatus();
    switch (status)
        {
            case CachingDataSource::Status::Canceled:                return "Canceled";
            case CachingDataSource::Status::DataNotCached:           return "DataNotCached";
            case CachingDataSource::Status::InternalCacheError:      return "InternalCacheError";
            case CachingDataSource::Status::NetworkErrorsOccured:    return "NetworkErrorsOccured";
            case CachingDataSource::Status::Success:                 return "Success";
            default: break;
        }
    return "";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::HandleError(WSErrorCR error)
    {
    Utf8String message;
    message += SEPERATOR;

    message += GetWSErrorMessage(error);

    message += SEPERATOR;
    BeDebugLog(message.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::HandleError(CachingDataSource::ErrorCR error)
    {
    Utf8String message;
    message += SEPERATOR;

    message += Utf8PrintfString(
        "Error:\n"
        "    CachingStatus: '%s'",
        GetCachingStatusString(error).c_str()
        );

    if (CachingDataSource::Status::NetworkErrorsOccured == error.GetStatus())
        {
        message += GetWSErrorMessage(error.GetWSError());
        }

    message += SEPERATOR;
    BeDebugLog(message.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::RunExamples()
    {
    RunExampleGetRepositories();
    RunExampleCreateCache();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::RunExampleGetRepositories()
    {
    IWSClientPtr client = WSClient::Create("https://bsw-construct2.bentley.com/ws", GetClientInfo());

    auto async = client->SendGetRepositoriesRequest()->Then([=] (const WSRepositoriesResult& response)
        {
        if (response.IsSuccess())
            {
            HandleGetRepositoriesSuccessResponse(response.GetValue());
            }
        else
            {
            HandleError(response.GetError());
            }
        });
    async->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::RunExampleCreateCache()
    {
    // WSG 1.3 (tests might not work with it)
    //IWSRepositoryClientPtr client = WSRepositoryClient::Create ("https://bsw-construct2.bentley.com/ws", "pw.PW", GetClientInfo ());

    // WSG 2.0
    IWSRepositoryClientPtr client = WSRepositoryClient::Create("https://bsw-construct.bentley.com/ws", "pw--PW", GetClientInfo());

    client->SetCredentials(Credentials("admin", "admin"));

    BeFileName cachePath;
    cachePath.AppendToPath(m_workDir).AppendToPath(L"exampleCache.db");

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, GetCacheEnvironment())->GetResult();

    if (!result.IsSuccess())
        {
        HandleError(result.GetError());
        return;
        }

    // Test open existing cache
    CachingDataSourcePtr dataSource = CachingDataSource::OpenOrCreate(client, cachePath, GetCacheEnvironment())->GetResult().GetValue();

    RunExamplesUseCache(dataSource);

    dataSource = nullptr; // Destructor will cancel all not finished async tasks and close cache
    DataSourceCache::DeleteCacheFromDisk(cachePath, GetCacheEnvironment());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingExample::RunExamplesUseCache(CachingDataSourcePtr dataSource)
    {
    dataSource->GetCacheAccessThread()->ExecuteAsync([=]
        {
        // Folowing code would need to access cache so they must be executed in cache thread

        RunExampleGetObject(dataSource);
        RunExampleGetFile(dataSource);

        RunExampleGetNavigation(dataSource);
        RunExampleQuery(dataSource);

        RunExampleGetServerInfo(dataSource);
        })
            ->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> CachingExample::RunExampleGetObject(CachingDataSourcePtr dataSource)
    {
    ObjectId objectId {"PW_WSG.document", "0ad36a3e-8d91-4d21-bf1e-b599125df725"};

    // Link object id to cache root to be able to store it. Not needed if object is already in cache.
    auto txn = dataSource->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot("CacheRootToHoldCachedObjects", objectId);
    txn.Commit();

    return
        dataSource->GetObject(objectId, CachingDataSource::DataOrigin::CachedOrRemoteData, DataSourceCache::JsonFormat::Raw)
        ->Then([=] (CachingDataSource::ObjectsResult result)
        {
        if (!result.IsSuccess())
            {
            HandleError(result.GetError());
            return;
            }
        HandleObjects(result.GetValue().GetJson());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> CachingExample::RunExampleGetFile(CachingDataSourcePtr dataSource)
    {
    ObjectId fileId {"PW_WSG.document", "0ad36a3e-8d91-4d21-bf1e-b599125df725"};

    // Link object id to cache root to be able to store it. Not needed if object is already in cache.
    auto txn = dataSource->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot("CacheRootToHoldCachedObjects", fileId);
    txn.Commit();

    return
        // Ensure object is in cache, not needed if known that object is in the cache
        dataSource->GetObject(fileId, CachingDataSource::DataOrigin::CachedOrRemoteData, DataSourceCache::JsonFormat::Raw)
        ->Then([=] (CachingDataSource::ObjectsResult result)
        {
        if (!result.IsSuccess())
            {
            HandleError(result.GetError());
            return;
            }
        // Retrieve file
        dataSource->GetFile(fileId, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)
            ->Then([=] (CachingDataSource::FileResult result)
            {
            if (!result.IsSuccess())
                {
                HandleError(result.GetError());
                return;
                }
            BeDebugLog(result.GetValue().GetFilePath().GetNameUtf8().c_str());
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> CachingExample::RunExampleGetNavigation(CachingDataSourcePtr dataSource)
    {
    // Navigation base object was linked to default cache root ("") when cache was created.
    dataSource->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)
        ->Then([=] (CachingDataSource::ObjectsResult result)
        {
        if (!result.IsSuccess())
            {
            HandleError(result.GetError());
            return;
            }
        HandleObjects(result.GetValue().GetJson());
        });

    // Link object to root to be able to store it. Not needed if object is already in cache.
    ObjectId objectId("Navigation.NavNode", "ECObjects--PW_WSG-project-68219509-2eab-4701-a99f-7504941836e2");
    auto txn = dataSource->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot("CacheRootToHoldCachedObjects", objectId);
    txn.Commit();

    return
        dataSource->GetNavigationChildren(objectId, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)
        ->Then([=] (CachingDataSource::ObjectsResult result)
        {
        if (!result.IsSuccess())
            {
            HandleError(result.GetError());
            return;
            }
        HandleObjects(result.GetValue().GetJson());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> CachingExample::RunExampleQuery(CachingDataSourcePtr dataSource)
    {
    WSQuery query("PW_WSG", "project");
    query.SetFilter("name+eq+'CachingExampleFolder'");
    query.SetSkip(0);
    query.SetTop(10);

    auto txn = dataSource->StartCacheTransaction();
    CachedResponseKey resultsKey(txn.GetCache().FindOrCreateRoot("HoldingRoot"), "MyProjectQuery");
    txn.Commit();

    return
        dataSource->GetObjects(resultsKey, query, ICachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)
        ->Then([=] (CachingDataSource::ObjectsResult result)
        {
        if (!result.IsSuccess())
            {
            HandleError(result.GetError());
            return;
            }
        HandleObjects(result.GetValue().GetJson());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> CachingExample::RunExampleGetServerInfo(CachingDataSourcePtr dataSource)
    {
    return
        dataSource->GetClient()->GetWSClient()->GetServerInfo()->Then([=] (WSInfoResult& response)
        {
        if (!response.IsSuccess())
            {
            HandleError(response.GetError());
            return;
            }
        BeDebugLog(response.GetValue().GetVersion().ToString().c_str());
        });
    }
