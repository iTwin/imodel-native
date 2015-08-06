/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/Downloader.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/Downloader.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
SingleThreadedDownloader::SingleThreadedDownloader()
    {
    m_CurlHandle = curl_easy_init();
    SetDefaultSettings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
SingleThreadedDownloader::~SingleThreadedDownloader()
    {
    curl_easy_cleanup(m_CurlHandle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt SingleThreadedDownloader::DownloadPage(WString& buffer, UrlPtr const& p_Url)
    {
    SetResourceUrl(p_Url->GetUrlWString());

    if(IsValidContentType())
        {
        CURLcode response;
        SetDataWritingSettings(&buffer, CurlWriteCallback);
        response = curl_easy_perform(m_CurlHandle);
        if(response == CURLE_OK)
            return SUCCESS;
        else
            return ERROR;
        }
    else
        return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool SingleThreadedDownloader::IsValidContentType()
    {
    if(m_ValidateContentType)
        {
        WString contentType;
        DownloadContentType(contentType);
        for (auto validContentType : m_ListOfValidContentTypes) 
            {
            if(regex_search(contentType.c_str(), validContentType))
                return true;
            }
        return false;
        }
    else
        {
        return true; //No validation, always a valid content type;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::DownloadContentType(WString& outputContentType)
    {
    //Get only the HTTP header
    curl_easy_setopt(m_CurlHandle, CURLOPT_HEADER, true);
    curl_easy_setopt(m_CurlHandle, CURLOPT_NOBODY, true);
    SetDataWritingSettings(NULL, CurlDiscardDataCallback);

    char* contentType;
    curl_easy_perform(m_CurlHandle);
    curl_easy_getinfo(m_CurlHandle, CURLINFO_CONTENT_TYPE, &contentType);

    outputContentType = WString(contentType, true/*is Utf8*/);

    //Reset to getting only the body
    curl_easy_setopt(m_CurlHandle, CURLOPT_HEADER, false);
    curl_easy_setopt(m_CurlHandle, CURLOPT_NOBODY, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetUserAgent(WString const& agent)
    {
    Utf8String temp;
    BeStringUtilities::WCharToUtf8(temp, agent.c_str());
    curl_easy_setopt(m_CurlHandle, CURLOPT_USERAGENT, temp.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetRequestTimeoutInSeconds(long timeout)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_TIMEOUT, timeout);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetDefaultSettings()
    {
    ValidateSslCertificates(false);
    m_ValidateContentType = true;
    m_ListOfValidContentTypes.push_back(wregex(L"text/html", regex_constants::icase));
    m_ListOfValidContentTypes.push_back(wregex(L"text/plain", regex_constants::icase));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetResourceUrl(WString const& url)
    {
    Utf8String temp;
    BeStringUtilities::WCharToUtf8(temp, url.c_str());
    curl_easy_setopt(m_CurlHandle, CURLOPT_URL, temp.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetDataWritingSettings(WString* buffer, void* writeFunction)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEDATA, buffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
size_t SingleThreadedDownloader::CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
    Utf8String s((char *) contents, size * nmemb);
    ((WString*) userp)->AppendUtf8(s.c_str());
    return size * nmemb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
size_t SingleThreadedDownloader::CurlDiscardDataCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
    return size * nmemb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetFollowAutoRedirects(bool follow)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_FOLLOWLOCATION, follow);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetMaxAutoRedirectCount(long count)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_MAXREDIRS, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetMaxHttpConnectionCount(long count)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_MAXCONNECTS, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::ValidateSslCertificates(bool validate)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_SSL_VERIFYPEER, validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::ValidateContentType(bool validate)
    {
    m_ValidateContentType = validate;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SingleThreadedDownloader::SetListOfValidContentType(bvector<WString> const& types)
    {
    m_ListOfValidContentTypes.clear();
    for (auto type : types)
        {
        wregex typeRegex = wregex(type.c_str(), regex_constants::icase);
        m_ListOfValidContentTypes.push_back(typeRegex);
        }
    }
