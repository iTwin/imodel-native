/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include <assert.h>
#include <algorithm>
#include <sstream>
#include <curl/curl.h>
#include <openssl/crypto.h>
#include <locale>
#include <codecvt>
#include "include/DataSourceAccountWSG.h"
#include "include/DataSourceWSG.h"

DataSourceAccountWSG::DataSourceAccountWSG(const ServiceName & name, const AccountIdentifier & identifier, const AccountKey & key)
    : Super(name, identifier, key)
    {
    setAccount(name, identifier, key);
    }

DataSource * DataSourceAccountWSG::createDataSource(const SessionName &session)
{
                                                            // NOTE: This method is for internal use only, don't call this directly.
    DataSourceWSG *   dataSourceWSG;
                                                            // Create a new DataSourceAzure
    dataSourceWSG = new DataSourceWSG(this, session);
    if (dataSourceWSG == nullptr)
        return nullptr;
                                                            // Set the timeout from the account's default (which comes from the Service's default)
    dataSourceWSG->setTimeout(this->getDefaultTimeout());
                                                            // Set the segment size from the account's default (which comes from the Service's default)
    dataSourceWSG->setSegmentSize(this->getDefaultSegmentSize());

    return dataSourceWSG;
}


DataSourceStatus DataSourceAccountWSG::destroyDataSource(DataSource *dataSource)
{
    if (dataSource)
    {
        delete dataSource;

        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}


DataSourceAccountWSG::~DataSourceAccountWSG(void)
    {
    m_isValid = false;
    }

DataSourceStatus DataSourceAccountWSG::setAccount(const AccountName & account, const AccountIdentifier & identifier, const AccountKey & key)
{
    if (account.length() == 0 )
        return DataSourceStatus(DataSourceStatus::Status_Error_Bad_Parameters);
                                                            // Set details in base class
    DataSourceAccount::setAccount(ServiceName(L"DataSourceServiceWSG"), account, identifier, key);

    return DataSourceStatus();
}

void DataSourceAccountWSG::setPrefixPath(const DataSourceURL &prefix)
    {
    DataSourceURL url(this->wsgProtocol + L"//");
    url.append(this->getAccountIdentifier());
    //if (!this->wsgPort.empty())
    //    {
    //    // using the += operator prevents appending an unwanted separator
    //    url += (L":" + this->wsgPort);
    //    }
    url.append(this->wsgVersion);
    url.append(this->wsgAPIID);
    url.append(this->wsgRepository);
    this->wsgOrganizationID = this->getOrganizationID(url);
    url.append(this->wsgSchema);
    url.append(this->wsgClassName);
    url.append(this->wsgOrganizationID);

    // From this point forward, this separator must be used to be understood by WSG
    url.setSeparator(L"~2F");
    url.append(prefix);


    DataSourceAccount::setPrefixPath(url);
    }


DataSourceStatus DataSourceAccountWSG::downloadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize & readSize)
{
    DataSourceURL    url;
    url.setSeparator(L"~2F");

    dataSource.getURL(url);

    return downloadBlobSync(url, dest, readSize, destSize, dataSource.getSessionName());
}

DataSourceStatus DataSourceAccountWSG::downloadBlobSync(DataSourceURL &url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session)
{
    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();

    CURL* curl = curl_handle->get();
    if (m_useDirectAzureCalls)
        {
        this->getWSGToken(url).c_str();
        auto nameOfFileToDownload = url.substr(url.find(L"~2F"));
        
        // replace all ~2F --> /
        size_t pos = nameOfFileToDownload.find(L"~2F");
        do
            {
            nameOfFileToDownload.replace(pos, 3, L"/");
            pos = nameOfFileToDownload.find(L"~2F");
            } while (pos != std::wstring::npos);
        
        // Use Azure directly by using the information provided when getting a valid token
        url = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(m_AzureDirectPrefix);
        url += nameOfFileToDownload + L"?";
        url += std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(m_AzureDirectSuffix);
        }
    else
        {
        curl_handle->add_item_to_header(this->getWSGToken(url).c_str());

        // indicate that we want to download the data (instead of just information about the data)
        url += L"/$file";
        }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_handle->get_headers());
    curl_easy_setopt(curl, CURLOPT_CAINFO,     this->getAccountSSLCertificatePath().c_str());

    return Super::downloadBlobSync(url, dest, readSize, size, session);
}

DataSourceStatus DataSourceAccountWSG::uploadBlobSync(DataSourceURL &url, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    // indicate that we want to upload the data (instead of just information about the data)
    url += L"/$file";

#if 0
    WSGEtag etag = getWSGHandshake(url, filename, size);

    return uploadBlobSync(url, etag, source, size);
#else
    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();

    CURL* curl = curl_handle->get();
    curl_handle->add_item_to_header(this->getWSGToken(url).c_str());

    curl_easy_setopt(curl, CURLOPT_CAINFO, this->getAccountSSLCertificatePath().c_str());

    return Super::uploadBlobSync(url, filename, source, size);
#endif
    }

DataSourceStatus DataSourceAccountWSG::uploadBlobSync(DataSource &dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
    {
    DataSourceURL    url;
    dataSource.getURL(url);

    return uploadBlobSync(url, dataSource.getSubPath().c_str(), source, size);
    }


#if 0
DataSourceStatus DataSourceAccountWSG::uploadBlobSync(const DataSourceURL &url, const WSGEtag &etag, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size)
{
(void)url;
(void)etag;
(void)source;
(void)size;
    //try
    //{
    //CURL *curl_handle;
    //curl_handle = curl_easy_init();
    //
    //struct CURLDataMemoryBuffer buffer;
    //buffer.data = source;
    //buffer.size = size;
    //
    //struct CURLDataResponseHeader header;
    //
    //std::string utf8URL = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(url);
    //
    //std::string ETag = "If-Match: " + etag;
    //std::string contentRange = "Content-Range: bytes 0-";
    //contentRange += std::to_string(size - 1) + "/" + std::to_string(size);
    //std::string contentLength = "Content-Length " + std::to_string(size);
    //
    //
    //struct curl_slist *headers = NULL;
    //headers = curl_slist_append(headers, this->getWSGToken().c_str());
    //headers = curl_slist_append(headers, "Content-Type: text/plain");
    //headers = curl_slist_append(headers, ETag.c_str());
    //headers = curl_slist_append(headers, contentRange.c_str());
    //headers = curl_slist_append(headers, contentLength.c_str());
    //
    //curl_easy_setopt(curl_handle, CURLOPT_URL, utf8URL.c_str());
    //curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    //curl_easy_setopt(curl_handle, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
    //curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0/*1*/);        // &&RB TODO : Ask Francis.Boily about his server certificate
    //curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0/*1*/);
    //curl_easy_setopt(curl_handle, CURLOPT_CAINFO, this->getAccountSSLCertificatePath().c_str());
    //curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
    //curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, CURLReadDataCallback);
    //curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, CURLWriteHeaderCallback);
    //curl_easy_setopt(curl_handle, CURLOPT_READDATA, buffer);
    //curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &header);
    //curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, size);
    //
    ///* put it! */
    //CURLcode res = curl_easy_perform(curl_handle);
    //
    ///* check for errors */
    //if (CURLE_OK != res)
    //    {
    //    //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    //    throw;
    //    }
    //
    //curl_easy_cleanup(curl_handle);
    //    }
    //catch (...)
    //{
    //    return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
    //}
    ////DataSourceBuffer::BufferData * download_buffer = new DataSourceBuffer::BufferData[size];
    ////DataSourceBuffer::BufferSize readSize;
    ////downloadBlobSync(url, download_buffer, readSize, size);

    return DataSourceStatus();
}
#endif

void DataSourceAccountWSG::setWSGTokenGetterCallback(const std::function<std::string(void)>& tokenGetter)
    {
    m_getWSGToken = tokenGetter;
    }

void DataSourceAccountWSG::setUseDirectAzureCalls(const bool& isDirect)
    {
    m_useDirectAzureCalls = isDirect;
    }

void DataSourceAccountWSG::setOrganizationID(const WSGServer::organizationID & orgID)
    {
    wsgOrganizationID = orgID;
    }

DataSourceAccountWSG::WSGToken DataSourceAccountWSG::getWSGToken(DataSourceURL &url)
    {
    auto token = this->m_getWSGToken();
    if (this->needsUpdateToken(token))
        {
        this->updateToken(token, url);
        }
    return DataSourceAccountWSG::WSGToken("Authorization: Token " + token);
    }

DataSourceAccountWSG::WSGEtag DataSourceAccountWSG::getWSGHandshake(const DataSourceURL & url, const DataSourceURL & filename, DataSourceBuffer::BufferSize size)
    {
    (void)url;
    (void)filename;
    (void)size;
    //CURL *curl_handle;
    //curl_handle = curl_easy_init();
    //
    //struct CURLDataResponseHeader response_header;
    //
    //std::string utf8URL = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(url);
    //
    //std::string contentDisposition = "Content-Disposition: attachment; filename=\"";
    //contentDisposition += std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(filename);
    //contentDisposition += "\"";
    //
    //std::string contentRange = "Content-Range: bytes */" + std::to_string(size);
    //
    //
    //struct curl_slist *headers = NULL;
    //headers = curl_slist_append(headers, this->getWSGToken().c_str());
    //headers = curl_slist_append(headers, contentDisposition.c_str());
    //headers = curl_slist_append(headers, contentRange.c_str());
    //headers = curl_slist_append(headers, "Content-Length: 0");
    //
    //curl_easy_setopt(curl_handle, CURLOPT_URL, utf8URL.c_str());
    //curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    //curl_easy_setopt(curl_handle, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
    //curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0/*1*/);        // &&RB TODO : Ask Francis.Boily about his server certificate
    //curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0/*1*/);
    //curl_easy_setopt(curl_handle, CURLOPT_CAINFO, this->getAccountSSLCertificatePath().c_str());
    //curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
    //curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, CURLWriteHeaderCallback);
    //curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &response_header);
    //
    ///* perform handshake */
    //CURLcode res = curl_easy_perform(curl_handle);
    //
    ///* check for errors */
    //if (CURLE_OK != res)
    //    {
    //    //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    //    throw;
    //    }
    //
    //assert(1 == response_header.data.count("ETag")); // ETag not returned by handshake
    //
    //curl_easy_cleanup(curl_handle);
    //
    //return DataSourceAccountWSG::WSGEtag(response_header.data["ETag"]);
    return DataSourceAccountWSG::WSGEtag();
    }

bool DataSourceAccountWSG::needsUpdateToken(const WSGToken & token)
    {
    return m_wsgToken != token;
    }

void DataSourceAccountWSG::updateToken(const WSGToken & newToken, DataSourceURL url)
    {
    m_wsgToken = newToken;
    // TODO: must be called separately...
    if (!m_useDirectAzureCalls) return;
    
    url += L"/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true";
    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();
    
    CURL* curl = curl_handle->get();
    
    curl_handle->add_item_to_header(("Authorization: Token " + m_wsgToken).c_str());
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_handle->get_headers());
    curl_easy_setopt(curl, CURLOPT_CAINFO, this->getAccountSSLCertificatePath().c_str());
    
    DataSourceBuffer::BufferData* dest = new DataSourceBuffer::BufferData[10000];
    DataSourceBuffer::BufferSize readSize = 0;
    DataSourceBuffer::BufferSize size = 10000;

    DataSource::SessionName session;
    DataSourceStatus result = Super::downloadBlobSync(url, dest, readSize, size, session);
    assert(result.isOK());
    Json::Reader    reader;
    Json::Value     result_json;
    reader.parse(reinterpret_cast<char *>(dest), reinterpret_cast<char *>(dest + readSize), result_json);
    auto azure_direct_link_url = result_json["instances"][0]["properties"]["Url"].asString();
    auto delimiterPosition = azure_direct_link_url.find("?");
    m_AzureDirectPrefix = azure_direct_link_url.substr(0, delimiterPosition).c_str();
    m_AzureDirectSuffix = azure_direct_link_url.substr(delimiterPosition + 1).c_str();
    }

WSGServer::organizationID DataSourceAccountWSG::getOrganizationID(const DataSourceURL& url)
    {
    CURLHandle* curl_handle = m_CURLManager.getOrCreateThreadCURLHandle();

    CURL* curl = curl_handle->get();

    DataSourceURL navNodesURL = url + L"/Navigation/NavNode/";
    curl_handle->add_item_to_header(this->getWSGToken(navNodesURL).c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_handle->get_headers());
    curl_easy_setopt(curl, CURLOPT_CAINFO, this->getAccountSSLCertificatePath().c_str());
    
    DataSourceBuffer::BufferData* dest = new DataSourceBuffer::BufferData[10000];
    DataSourceBuffer::BufferSize readSize = 0;
    DataSourceBuffer::BufferSize size = 10000;

    DataSource::SessionName session;
    DataSourceStatus result = Super::downloadBlobSync(navNodesURL, dest, readSize, size, session);
    assert(result.isOK());
    Json::Reader    reader;
    Json::Value     result_json;
    reader.parse(reinterpret_cast<char *>(dest), reinterpret_cast<char *>(dest + readSize), result_json);
    std::string utf8OrgID = result_json["instances"][2]["properties"]["Key_InstanceId"].asString().c_str();
    std::wstring utf16OrgID = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(utf8OrgID);

    return WSGServer::organizationID(utf16OrgID);
    }

bool DataSourceAccountWSG::isValid(void)
    {
    return m_isValid;
    }
