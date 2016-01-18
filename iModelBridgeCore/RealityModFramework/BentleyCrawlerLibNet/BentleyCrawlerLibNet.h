/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyCrawlerLibNet/BentleyCrawlerLibNet.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <CrawlerLib/Crawler.h>
#include <vcclr.h>

using namespace System;
using namespace BentleyApi::CrawlerLib;
using namespace System::Collections::Generic;




namespace BentleyCrawlerLib 
{
    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette       1/2016
    //=====================================================================================
    public ref class UrlNet
    {
    public: 
        UrlNet(Url *p_Url) { m_Url = p_Url; }

        System::String^ GetDomainName();  // No need to define this method since Uri manage all this. 
        Uri^ GetUrl();

        //---------------------------------------------------------------------------------------
        // Returns pointer to parent Url. This parent can be null for seeds.
        //---------------------------------------------------------------------------------------
        UrlNet^ GetParent() { return gcnew UrlNet(m_Url->GetParent().get()); }
         
        UInt32  GetDepth() { return m_Url->GetDepth(); }

        bool IsExternalPage() { return m_Url->IsExternalPage(); }
        
        bool IsSubUrlOf(Url &parent) { return m_Url->IsSubUrlOf(parent); }

        bool operator==(UrlNet^ other) { return m_Url == other->m_Url; }
        bool operator<(UrlNet^ other) { return m_Url < other->m_Url; }        

    private:
        Url *m_Url;
        ~UrlNet();
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette       1/2016
    //=====================================================================================
    public ref class PageContentNet
    {
    public:
        //PageContentNet(Url *p_Url, System::String^ p_Text) { m_PageContent = new PageContent(p_Url, ctx.marshal_as<const wchar_t*>(p_Text))};
        PageContentNet(PageContent *p_PageContent) { m_PageContent = p_PageContent; }

        UrlNet^ GetUrl();

        //void SetUrl(Uri^ url) { m_Url = url; }

        System::String^ GetText();

        //void SetText(System::String^ text) { m_Text = text; }

        //=======================================================================================
        // Adds a link to the page content. There is no validation performed and thus an existing
        // link can be added more than once.
        //=======================================================================================
        //void AddLink(Uri^ link) { m_Links->Add(link); }
        List<UrlNet^>^ GetLinks();

    private:
        //Member attributes
        PageContent *m_PageContent;
        ~PageContentNet();
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette       1/2016
    //=====================================================================================
    public interface class ICrawlerObserverNet //See: http://blogs.microsoft.co.il/alon/2007/05/29/native-callback/
    {
        //void OnPageCrawled(PageContentCR page);
        void OnPageCrawled(PageContentNet^ page);
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette       1/2016
    //=====================================================================================
    class CrawlerObserverWrapper : public ICrawlerObserver
    {
    public:
        CrawlerObserverWrapper(gcroot<ICrawlerObserverNet ^> managedCrawlerObserver)
            : m_managedCrawlerObserver(managedCrawlerObserver)
        {

        }

        virtual void OnPageCrawled(PageContentCR page)
        {
            m_managedCrawlerObserver->OnPageCrawled(gcnew PageContentNet(&const_cast<PageContent&>(page)));
            //m_managedCrawlerObserver->OnPageCrawled(page);
        }

    private:
        gcroot<ICrawlerObserverNet ^> m_managedCrawlerObserver;
    };
	
	//=====================================================================================
	//! @bsiclass                                   Martin-Yanick.Guillemette       12/2015
	//=====================================================================================
    public ref class BentleyCrawlerLibNet
	{
	public:
		static BentleyCrawlerLibNet^ Create(size_t maxNumberOfSimultaneousDownloads);
        //BentleyCrawlerLibNet^ Create(UrlQueue* queue, bvector<IPageDownloader*> const& downloaders); ToDo: Definir UrlQueue et convertir bvector.
		StatusInt^ Crawl(Uri^ url);
        void Pause() { m_pCrawler->Pause(); }
        void Unpause() { m_pCrawler->Unpause(); }
        void Stop() { m_pCrawler->Stop(); }
        void SetObserver(ICrawlerObserverNet ^crawlerObserver);
        void SetMaxNumberOfLinkToCrawl(size_t n) { m_pCrawler->SetMaxNumberOfLinkToCrawl(n); }
        void SetUserAgent(System::String^ agent);
        void SetRequestTimeoutInSeconds(int timeout) { m_pCrawler->SetRequestTimeoutInSeconds(timeout); }
        void SetFollowAutoRedirects(bool follow) { m_pCrawler->SetFollowAutoRedirects(follow); }
        void SetMaxAutoRedirectCount(int count) { m_pCrawler->SetMaxAutoRedirectCount(count); }
        void ValidateSslCertificates(bool validate) { m_pCrawler->ValidateSslCertificates(validate); }
        void ValidateContentType(bool validate) { m_pCrawler->ValidateContentType(validate); }
        void SetListOfValidContentType(List<System::String^>^ contentTypes);             
        void SetMaximumCrawlDepth(uint32_t depth) { m_pCrawler->SetMaximumCrawlDepth(depth); }
        void SetAcceptExternalLinks(bool accept) { m_pCrawler->SetAcceptExternalLinks(accept); }
        void SetAcceptLinksInExternalLinks(bool accept) { m_pCrawler->SetAcceptLinksInExternalLinks(accept); }
        void SetRespectRobotTxt(bool respect) { m_pCrawler->SetRespectRobotTxt(respect); }
        void SetRespectRobotTxtIfDisallowRoot(bool respect) { m_pCrawler->SetRespectRobotTxtIfDisallowRoot(respect); }
        void SetRobotsTxtUserAgent(System::String^ userAgent);
        void SetMaxRobotTxtCrawlDelay(uint32_t delayInSeconds) { m_pCrawler->SetMaxRobotTxtCrawlDelay(delayInSeconds); }
        void SetCrawlLinksWithHtmlTagRelNoFollow(bool crawlLinks) { m_pCrawler->SetCrawlLinksWithHtmlTagRelNoFollow(crawlLinks); }
        void SetCrawlLinksFromPagesWithNoFollowMetaTag(bool crawlLinks) { m_pCrawler->SetCrawlLinksFromPagesWithNoFollowMetaTag(crawlLinks); }

	private:
        Crawler *m_pCrawler;
		BentleyCrawlerLibNet(size_t maxNumberOfSimultaneousDownloads);
		~BentleyCrawlerLibNet();

		//~BentleyCrawlerLibNet() { this->!BentleyCrawlerLibNet(); }
		//!BentleyCrawlerLibNet() { delete m_crawlerLib; }
	};

}

