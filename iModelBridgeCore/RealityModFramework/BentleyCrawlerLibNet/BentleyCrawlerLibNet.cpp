/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyCrawlerLibNet/BentleyCrawlerLibNet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BentleyCrawlerLibNet.h"

#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;
using namespace BentleyCrawlerLib;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
CrawlerObserverWrapper::CrawlerObserverWrapper(gcroot<ICrawlerObserverNet ^> managedCrawlerObserver)
    : m_managedCrawlerObserver(managedCrawlerObserver)
    {

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
void CrawlerObserverWrapper::OnPageCrawled(PageContentCR page)
    {
    marshal_context ctx;

    // Url
    String^ urlStr = ctx.marshal_as<String^>(page.GetUrl().GetUrlWString().GetWCharCP());

    //UrlNet::CreateFromUrl(*page.GetUrl().GetParent());
    UrlNet^ url = gcnew UrlNet(urlStr, nullptr);
    if (nullptr == url)
        return;

    // Textual content
    String^ text = ctx.marshal_as<String^>(page.GetText().c_str());

    PageContentNet^ pageNet = gcnew PageContentNet(url, text);

    // Link
    bvector<UrlCPtr> links = page.GetLinks();
    for each (UrlCPtr url in links)
        {
        String^ urlNetStr = ctx.marshal_as<String^>(url->GetUrlWString().GetWCharCP());

        UrlNet^ urlNet = gcnew UrlNet(urlNetStr, nullptr);
        if (urlNet->IsValid())
            {
            pageNet->AddLink(urlNet);
            }            
        }

    m_managedCrawlerObserver->OnPageCrawled(pageNet);
    }

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
    m_pCrawler = new CrawlerPtr(BentleyApi::CrawlerLib::Crawler::Create(maxNumberOfSimultaneousDownloads));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt BentleyCrawlerLibNet::Crawl(Uri^ uri)
    {
    marshal_context ctx;
    SeedPtr seedNet = Seed::Create(ctx.marshal_as<const wchar_t*>(uri->AbsoluteUri));

	return (*m_pCrawler)->Crawl(*seedNet);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyCrawlerLibNet::~BentleyCrawlerLibNet()
    {
    if (0 != m_pCrawler)
        {
        delete m_pCrawler;
        m_pCrawler = 0;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
void BentleyCrawlerLibNet::SetObserver(ICrawlerObserverNet^ crawlerObserver)
    {
    CrawlerObserverWrapper* pCrawlerObserverWrapper = new CrawlerObserverWrapper(crawlerObserver);

    (*m_pCrawler)->SetObserver(pCrawlerObserverWrapper);
    }

//=======================================================================================
// UrlNet
//=======================================================================================

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
UrlNet::UrlNet(System::String^ url, const UrlNet^ parent)
    {
    marshal_context ctx;
    WString urlW = L"";
    m_pUrl = nullptr;
    m_isValid = false;

    // Make sure this is a valid uri.
    Uri^ tempUri;
    if (Uri::TryCreate(url, UriKind::RelativeOrAbsolute, tempUri))
        {
        urlW = ctx.marshal_as<const wchar_t*>(url);

        //&&JFC TODO: Recurse for all the parents
        m_pUrl = new UrlPtr(Url::Create(urlW));

        m_isValid = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
Uri^ UrlNet::GetUrl()
    {
    marshal_context ctx;
    return gcnew Uri(ctx.marshal_as<System::String^>((*m_pUrl)->GetUrlWString().GetWCharCP()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
System::String^ UrlNet::GetDomainName()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pUrl)->GetDomainName().GetWString().GetWCharCP());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
UrlNet^ UrlNet::GetParent()
    {
    UrlCPtr parentUrl = (*m_pUrl)->GetParent();
    
    marshal_context ctx;
    String^ url = ctx.marshal_as<System::String^>((*m_pUrl)->GetUrlWString().GetWCharCP());

    //&&JFC TODO: Recurse on parent.
    return gcnew UrlNet(url, nullptr);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
bool UrlNet::IsSubUrlOf(const UrlNet^ parentUrl)
    { 
    //&&JFC TODO: Necessity to redefine this method in the wrapper?
    //m_pUrl->IsSubUrlOf(parent)
    return true; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
UrlNet::~UrlNet()
    {
    if (0 != m_pUrl)
        {
        delete m_pUrl;
        m_pUrl = 0;
        }
    }

//=======================================================================================
// PageContentNet
//=======================================================================================

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
PageContentNet::PageContentNet(UrlNet^ urlNet, String^ textualContent)
    {
    // Url
    Uri^ uri = urlNet->GetUrl();

    marshal_context ctx;
    WString urlW = ctx.marshal_as<const wchar_t*>(uri->AbsoluteUri);

    UrlCPtr url = Url::Create(urlW);

    // Text
    WString text = ctx.marshal_as<const wchar_t*>(textualContent);

    m_pPageContent = new PageContentPtr(PageContent::Create(*url, text));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
UrlNet^ PageContentNet::GetUrl()
    {
    Url url = (*m_pPageContent)->GetUrl();

    marshal_context ctx;
    String^ urlStr = ctx.marshal_as<System::String^>(url.GetUrlWString().GetWCharCP());

    return gcnew UrlNet(urlStr, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
System::String^ PageContentNet::GetText()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pPageContent)->GetText().GetWCharCP());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
void PageContentNet::AddLink(UrlNet^ urlNet)
    {
    Uri^ uri = urlNet->GetUrl();

    marshal_context ctx;
    WString urlW = ctx.marshal_as<const wchar_t*>(uri->AbsoluteUri);

    UrlCPtr url = Url::Create(urlW);

    (*m_pPageContent)->AddLink(*url);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
List<UrlNet^>^ PageContentNet::GetLinks()
    {
    marshal_context ctx;
    

    bvector<UrlCPtr> links = (*m_pPageContent)->GetLinks();
    List<UrlNet^>^ linksList = gcnew List<UrlNet^>((UInt32)links.size());
    for (size_t i = 0; i < links.size(); i++)
        {
        UrlCPtr urlTemp = links.at(i);
        String^ urlTxt = ctx.marshal_as<System::String^>(urlTemp->GetUrlWString().GetWCharCP());

        //&&JFC TODO: Recurse for parents
        UrlNet^ linkUrl = gcnew UrlNet(urlTxt, nullptr);
        linksList->Add(linkUrl);
        }
    return linksList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Martin-Yanick.Guillemette       1/2016
//+---------------+---------------+---------------+---------------+---------------+------
PageContentNet::~PageContentNet()
    {
    if (0 != m_pPageContent)
        {
        delete m_pPageContent;
        m_pPageContent = 0;
        }
    }
