#include "BentleyCrawlerLibNet.h"
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;
using namespace BentleyCrawlerLib;

//=======================================================================================
// BentleyCrawlerLibNet
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyCrawlerLibNet^ BentleyCrawlerLibNet::Create(size_t maxNumberOfSimultaneousDownloads)
{	
	return gcnew BentleyCrawlerLibNet(maxNumberOfSimultaneousDownloads);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyCrawlerLibNet::BentleyCrawlerLibNet(size_t maxNumberOfSimultaneousDownloads)
{
    m_pCrawler = BentleyApi::CrawlerLib::Crawler::Create(maxNumberOfSimultaneousDownloads).get();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt^ BentleyCrawlerLibNet::Crawl(Uri^ url)
{
    marshal_context ctx;
    UrlPtr seedNet = new Seed(ctx.marshal_as<const wchar_t*>(url->AbsoluteUri));

	return gcnew StatusInt(m_pCrawler->Crawl(seedNet));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyCrawlerLibNet::~BentleyCrawlerLibNet()
{
    free(m_pCrawler);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
void BentleyCrawlerLibNet::SetUserAgent(System::String^ agent)
{
    marshal_context ctx;
    m_pCrawler->SetUserAgent(ctx.marshal_as<const wchar_t*>(agent));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
void BentleyCrawlerLibNet::SetListOfValidContentType(List<System::String^>^ contentTypes)
{
    if (0 != contentTypes->Count)
    {
        marshal_context ctx;
        bvector<WString> contentTypesList;
        for each (System::String^ type in contentTypes)
        {
            contentTypesList.push_back(ctx.marshal_as<const wchar_t*>(type));
        }
        m_pCrawler->SetListOfValidContentType(contentTypesList);
    }    
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
void BentleyCrawlerLibNet::SetRobotsTxtUserAgent(System::String^ userAgent)
{
    marshal_context ctx;
    m_pCrawler->SetRobotsTxtUserAgent(ctx.marshal_as<const wchar_t*>(userAgent));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
void BentleyCrawlerLibNet::SetObserver(ICrawlerObserverNet ^crawlerObserver)
{
    CrawlerObserverWrapper *pCrawlerObserverWrapper = new CrawlerObserverWrapper(crawlerObserver);

    m_pCrawler->SetObserver(pCrawlerObserverWrapper);
}

//=======================================================================================
// UrlNet
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
Uri^ UrlNet::GetUrl()
{
    marshal_context ctx;
    return gcnew Uri(ctx.marshal_as<System::String^>(m_Url->GetUrlWString().GetWCharCP()));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
System::String^ UrlNet::GetDomainName()
{
    marshal_context ctx;
    return ctx.marshal_as<System::String^>(m_Url->GetDomainName().GetWString().GetWCharCP());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
UrlNet::~UrlNet()
{
    free(m_Url);
}

//=======================================================================================
// PageContentNet
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
UrlNet^ PageContentNet::GetUrl()
{
    return gcnew UrlNet(&(const_cast<Url&>(m_PageContent->GetUrl())));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
System::String^ PageContentNet::GetText()
{
    marshal_context ctx;
    return ctx.marshal_as<System::String^>(m_PageContent->GetText().GetWCharCP());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
List<UrlNet^>^ PageContentNet::GetLinks()
{
    bvector<UrlPtr> const *links = &(m_PageContent->GetLinks());
    List<UrlNet^>^ linksList = gcnew List<UrlNet^>((UInt32)links->size());
    for (size_t i = 0; i < links->size(); i++)
    {
        UrlNet^ linkUrl = gcnew UrlNet(links->at(i).get());
        linksList->Add(linkUrl);
    }
    return linksList;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
PageContentNet::~PageContentNet()
{
    free(m_PageContent);
}
