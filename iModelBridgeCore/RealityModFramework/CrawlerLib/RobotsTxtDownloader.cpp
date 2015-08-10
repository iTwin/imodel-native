/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/RobotsTxtDownloader.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/RobotsTxtDownloader.h>
#include <CrawlerLib/RobotsTxtParser.h>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
RobotsTxtDownloader::RobotsTxtDownloader()
    {
    m_CurlHandle = curl_easy_init();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
RobotsTxtDownloader::~RobotsTxtDownloader()
    {
    curl_easy_cleanup(m_CurlHandle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
RobotsTxtContentPtr RobotsTxtDownloader::DownloadRobotsTxt(UrlPtr const& pi_Url)
    {
    UrlPtr robotsTxtUrl = new Url(L"/robots.txt", pi_Url);
    SetResourceUrl(robotsTxtUrl->GetUrlWString());

    CURLcode response;
    WString buffer;
    SetDataWritingSettings(&buffer, CurlWriteCallback);
    response = curl_easy_perform(m_CurlHandle);

    if(response == CURLE_OK)
        {
        return m_Parser.ParseRobotsTxt(buffer, robotsTxtUrl);
        }
    else
        {
        return m_Parser.GetEmptyRobotTxt(pi_Url);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtDownloader::SetResourceUrl(WString const& url)
    {
    Utf8String temp;
    BeStringUtilities::WCharToUtf8(temp, url.c_str());
    curl_easy_setopt(m_CurlHandle, CURLOPT_URL, temp.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void RobotsTxtDownloader::SetDataWritingSettings(WString* buffer, void* writeFunction)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEDATA, buffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
size_t RobotsTxtDownloader::CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
    Utf8String s((char *) contents, size * nmemb);
    ((WString*) userp)->AppendUtf8(s.c_str());
    return size * nmemb;
    }
