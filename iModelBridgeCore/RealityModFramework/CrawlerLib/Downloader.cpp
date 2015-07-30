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
    CURLcode response;

    Utf8String temp;
    BeStringUtilities::WCharToUtf8(temp, p_Url->GetUrlWString().c_str());

    curl_easy_setopt(m_CurlHandle, CURLOPT_URL, temp.c_str());
    curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(m_CurlHandle, CURLOPT_SSL_VERIFYPEER, false);
    response = curl_easy_perform(m_CurlHandle);

    return SUCCESS;
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
