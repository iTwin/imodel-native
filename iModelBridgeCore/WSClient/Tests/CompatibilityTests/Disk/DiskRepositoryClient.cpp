/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Disk/DiskRepositoryClient.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DiskRepositoryClient.h"

#include <Bentley/BeFileListIterator.h>
#include <ECObjects/ECSchema.h>

#include "../../UnitTests/Published/Utils/StubInstances.h"

#include "../Logging/Logging.h"

USING_NAMESPACE_WSCLIENT_UNITTESTS
USING_NAMESPACE_BENTLEY_EC

#include <openssl/evp.h>
#include <Bentley/Base64Utilities.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool Sha1Calc(const bvector<Byte>& input, unsigned char *binaryHash, unsigned int * hashLen)
    {
    EVP_MD_CTX *mdctx;
    if ((mdctx = EVP_MD_CTX_create()) == NULL)
        return false;
    
    if (!EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL))
        return false;

    if (!EVP_DigestUpdate(mdctx, &input[0], input.size()))
        return false;

    if (!EVP_DigestFinal_ex(mdctx, binaryHash, hashLen))
        return false;

    EVP_MD_CTX_free(mdctx);
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool Sha1CalcToBase64(const bvector<Byte>& input, Utf8StringR hashHexStringOut)
    {
    unsigned int hashLen;
    unsigned char binaryHash[EVP_MAX_MD_SIZE];

    if (!Sha1Calc(input, binaryHash, &hashLen))
        return false;

    hashHexStringOut = Base64Utilities::Encode((Utf8CP) binaryHash, hashLen);
    if (hashHexStringOut.empty())
        return false;

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CalcFileHash(BeFileNameCR filePath, Utf8StringR hashHexStringOut)
    {
    bvector<Byte> contents;

    BeFile file;
    file.Open(filePath, BeFileAccess::Read);
    if (BeFileStatus::Success != file.ReadEntireFile(contents))
        return false;
    file.Close();

    if (!Sha1CalcToBase64(contents, hashHexStringOut))
        return false;

    return true;
    }

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

        if (!path.GetFileNameAndExtension().ContainsI(L".ecschema.xml"))
            continue;

        SchemaKey key;
        Utf8String fName (path.GetFileNameWithoutExtension ());
        if (ECN::ECObjectsStatus::Success != SchemaKey::ParseSchemaFullName(key, fName.c_str()))
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
Utf8StringCR suppliedETag,
Request::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr ct
) const
    {
    SchemaKey key;
    SchemaKey::ParseSchemaFullName(key, objectId.remoteId.c_str());

    auto it = m_schemaPaths.find(key);
    if (it == m_schemaPaths.end())
        {
        BeAssert(false);
        return CreateCompletedAsyncTask(WSFileResult::Error({}));
        }

    BeFileName schemaPath = it->second;

    Utf8String hash;
    if (!CalcFileHash(schemaPath, hash))
        {
        BeAssert(false);
        return CreateCompletedAsyncTask(WSFileResult::Error({}));
        }
    Utf8String newETag = "DiskRepositoryClient." + hash;

    if (suppliedETag == newETag)
        {
        return CreateCompletedAsyncTask(WSFileResult::Success(
            WSFileResponse(BeFileName(), HttpStatus::NotModified, newETag)));
        }

    LOG.infov("Getting schema: %s\n", Utf8String(it->second).c_str());

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile(it->second.c_str(), filePath.c_str()))
        {
        BeAssert(false);
        return CreateCompletedAsyncTask(WSFileResult::Error({}));
        }

    return CreateCompletedAsyncTask(WSFileResult::Success(
        WSFileResponse(filePath, HttpStatus::OK, newETag)));
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
        Utf8String fullName = key.GetFullSchemaName();
        instances.Add({"MetaSchema", "ECSchemaDef", fullName}, {
                {"Name", name},
                {"VersionMajor", key.m_versionRead},
                {"VersionMinor", key.m_versionMinor}});
        }

    // No ETag because WSG tries to parse it if sent
    return CreateCompletedAsyncTask(instances.ToWSObjectsResult(""));
    }
