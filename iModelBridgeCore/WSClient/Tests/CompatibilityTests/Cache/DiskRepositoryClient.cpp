/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Cache/DiskRepositoryClient.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DiskRepositoryClient.h"

#include <Bentley/BeFileListIterator.h>
#include <ECObjects/ECSchema.h>

#include "../../UnitTests/Published/Utils/StubInstances.h"
#include "Logging.h"

USING_NAMESPACE_WSCLIENT_UNITTESTS
USING_NAMESPACE_BENTLEY_EC

#define STUB_FILE_ETAG "TestFakeTag"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DiskRepositoryClient::DiskRepositoryClient(BeFileName schemasDir) :
m_schemasDir(schemasDir)
    {
    if (m_schemasDir.empty())
        return;

    // Fix path for BeFileListIterator to work
    auto endChar = m_schemasDir[m_schemasDir.length() - 1];
    if (endChar == L'\\' || endChar == L'/')
        m_schemasDir.resize(m_schemasDir.length() - 1);

    BeFileListIterator iterator(m_schemasDir, true);
    BeFileName path;
    while (SUCCESS == iterator.GetNextFileName(path))
        {
        if (path.IsDirectory())
            continue;

        if (!path.GetExtension().EqualsI(L"xml"))
            continue;

        SchemaKey key;
        if (SUCCESS != SchemaKey::ParseSchemaFullName(key, path.GetFileNameWithoutExtension().c_str()))
            continue;

        m_schemaPaths[key] = path;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSFileResult> DiskRepositoryClient::SendGetFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
Utf8StringCR eTag,
HttpRequest::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    if (eTag == STUB_FILE_ETAG)
        {
        return CreateCompletedAsyncTask(WSFileResult::Success(
            WSFileResponse(BeFileName(), HttpStatus::NotModified, STUB_FILE_ETAG)));
        }

    SchemaKey key;
    SchemaKey::ParseSchemaFullName(key, WString(objectId.remoteId.c_str(), true).c_str());

    auto it = m_schemaPaths.find(key);
    if (it == m_schemaPaths.end())
        {
        BeAssert(false);
        return CreateCompletedAsyncTask(WSFileResult::Error({}));
        }

    LOG.infov("Getting schema: %s\n", Utf8String(it->second).c_str());

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile(it->second.c_str(), filePath.c_str()))
        {
        BeAssert(false);
        return CreateCompletedAsyncTask(WSFileResult::Error({}));
        }

    return CreateCompletedAsyncTask(WSFileResult::Success(
        WSFileResponse(filePath, HttpStatus::OK, STUB_FILE_ETAG)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSObjectsResult> DiskRepositoryClient::SendGetSchemasRequest
(
Utf8StringCR eTag,
ICancellationTokenPtr ct
) const
    {
    StubInstances instances;

    for (auto pair : m_schemaPaths)
        {
        auto key = pair.first;
        Utf8String name(key.m_schemaName);
        Utf8PrintfString fullName("%s.%d.%d", name.c_str(), key.m_versionMajor, key.m_versionMinor);
        instances.Add({"MetaSchema", "ECSchemaDef", fullName}, {
                {"Name", name},
                {"VersionMajor", key.m_versionMajor},
                {"VersionMinor", key.m_versionMinor}});
        }

    // No ETag because WSG tries to parse it if sent
    return CreateCompletedAsyncTask(instances.ToWSObjectsResult(""));
    }