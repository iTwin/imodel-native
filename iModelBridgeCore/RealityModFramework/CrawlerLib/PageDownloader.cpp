/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PageDownloader.h"

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
IPageDownloader::~IPageDownloader() {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
PageDownloader::PageDownloader(CrawlDelaySleeperPtr sleeper)
    : m_AbortDownload(false)
    {
    m_pSleeper = sleeper;
    m_CurlHandle = curl_easy_init();
    SetDefaultSettings();

    m_pParser = PageParser::Create();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
PageDownloader::~PageDownloader()
    {
    curl_easy_cleanup(m_CurlHandle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
PageContentPtr PageDownloader::DownloadPage(DownloadJobPtr p_DownloadJob)
    {
    WaitToRespectCrawlDelay(*p_DownloadJob);

    PageContentPtr content;
    SetResourceUrl(p_DownloadJob->GetUrlToDownload()->GetUrlWString());
    if(IsValidContentType())
        {
        WString buffer;
        CURLcode response;
        SetDataWritingSettings(&buffer, CurlWriteCallback);
        response = curl_easy_perform(m_CurlHandle);
        if(response == CURLE_OK)
            content = m_pParser->ParsePage(buffer, *p_DownloadJob->GetUrlToDownload());
        else
            content = m_pParser->GetEmptyPageContent(*p_DownloadJob->GetUrlToDownload());
        }
    else
        content = m_pParser->GetEmptyPageContent(*p_DownloadJob->GetUrlToDownload());

    return content;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool PageDownloader::IsValidContentType()
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
void PageDownloader::DownloadContentType(WString& outputContentType)
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
void PageDownloader::SetUserAgent(WString const& agent)
    {
    Utf8String temp;
    BeStringUtilities::WCharToUtf8(temp, agent.c_str());
    curl_easy_setopt(m_CurlHandle, CURLOPT_USERAGENT, temp.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetRequestTimeoutInSeconds(long timeout)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_TIMEOUT, timeout);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetDefaultSettings()
    {
    ValidateSslCertificates(false);
    m_ValidateContentType = true;
    m_ListOfValidContentTypes.push_back(wregex(L"text/html", regex_constants::icase));
    m_ListOfValidContentTypes.push_back(wregex(L"text/plain", regex_constants::icase));
    curl_easy_setopt(m_CurlHandle, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(m_CurlHandle, CURLOPT_PROGRESSFUNCTION, CurlProgressCallback);
    curl_easy_setopt(m_CurlHandle, CURLOPT_PROGRESSDATA, &m_AbortDownload);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetResourceUrl(WString const& url)
    {
    Utf8String temp;
    BeStringUtilities::WCharToUtf8(temp, url.c_str());
    curl_easy_setopt(m_CurlHandle, CURLOPT_URL, temp.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetDataWritingSettings(WString* buffer, void* writeFunction)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(m_CurlHandle, CURLOPT_WRITEDATA, buffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
size_t PageDownloader::CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
    Utf8String s((char *) contents, size * nmemb);
    ((WString*) userp)->AppendUtf8(s.c_str());
    return size * nmemb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
size_t PageDownloader::CurlDiscardDataCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
    return size * nmemb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
int PageDownloader::CurlProgressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
    atomic<bool>* abortFlag = (atomic<bool>*) clientp;
    bool aborted = abortFlag->load(memory_order_relaxed);

    if(aborted)
        return -1;

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetFollowAutoRedirects(bool follow)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_FOLLOWLOCATION, follow);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetMaxAutoRedirectCount(long count)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_MAXREDIRS, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::ValidateSslCertificates(bool validate)
    {
    curl_easy_setopt(m_CurlHandle, CURLOPT_SSL_VERIFYPEER, validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::ValidateContentType(bool validate)
    {
    m_ValidateContentType = validate;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetListOfValidContentType(bvector<WString> const& types)
    {
    m_ListOfValidContentTypes.clear();
    for (auto type : types)
        {
        wregex typeRegex = wregex(type.c_str(), regex_constants::icase);
        m_ListOfValidContentTypes.push_back(typeRegex);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetParseLinksRelNoFollow(bool parse)
    {
    m_pParser->SetParseLinksRelNoFollow(parse);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::SetParsePagesWithNoFollowMetaTag(bool parse)
    {
    m_pParser->SetParsePagesWithNoFollowMetaTag(parse);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::WaitToRespectCrawlDelay(DownloadJobCR p_DownloadJob)
    {
    m_pSleeper->Sleep(p_DownloadJob.GetCrawlDelay(), p_DownloadJob.GetUrlToDownload()->GetDomainName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageDownloader::AbortDownload()
    {
    m_AbortDownload.store(true);
    }
