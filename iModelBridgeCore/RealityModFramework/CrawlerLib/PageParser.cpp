/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/PageParser.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PageParser.h"
#include <regex>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;
using namespace regex_constants;

const wregex PageParser::s_LinkRegex = wregex(L"<\\s*a\\s+"          //The opening of the <a> tag
                                              L"[^<]*href\\s*=\\s*"  //The href element
                                              L"\"([^<\"]+)\""       //The actual link to parse
                                              L"[^<]*>", icase);     //The closing '>' of the <a> tag

const wregex PageParser::s_RelNoFollowRegex = wregex(L"<\\s*a\\s+"
                                                     L"[^<]*rel\\s*=\\s*"
                                                     L"\"nofollow\""
                                                     L"[^<]*>", icase);

const wregex PageParser::s_NoFollowMetaTagRegex = wregex(L"<\\s*meta\\s+"
                                                         L"name=\"robots\"\\s+"
                                                         L"[^<]*content\\s*=\\s*"
                                                         L"\".*nofollow.*\""
                                                         L"[^<]*>", icase);

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
PageParserPtr PageParser::Create()
    {
    return new PageParser();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
PageParser::PageParser()
    {
    m_ParseLinksRelNoFollow = true;
    m_ParsePagesWithNoFollowMetaTag = true;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
void PageParser::SetParseLinksRelNoFollow(bool parse) 
    { 
    m_ParseLinksRelNoFollow = parse; 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
void PageParser::SetParsePagesWithNoFollowMetaTag(bool parse) 
    { 
    m_ParsePagesWithNoFollowMetaTag = parse; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
PageContentPtr PageParser::ParsePage(WString const& text, UrlCR url) const
    {
    PageContentPtr content = PageContent::Create(url, text);

    AddLinksFromText(text, url, *content);

    return content;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageParser::AddLinksFromText(WString const& text, UrlCR url, PageContentR content) const
    {
    if (!m_ParsePagesWithNoFollowMetaTag) 
        return;

    if (regex_search(text.c_str(), PageParser::s_NoFollowMetaTagRegex))
        return;

    //if(!m_ParsePagesWithNoFollowMetaTag && regex_search(text.c_str(), PageParser::s_NoFollowMetaTagRegex))
    //    return;

    wcregex_iterator words_begin(text.begin(), text.end(), PageParser::s_LinkRegex);
    wcregex_iterator words_end;
    for(auto i = words_begin; i != words_end; ++i)
        {
        wcmatch match = *i;
        WString completeHtmlATag = match[0].str().c_str();
        if(m_ParseLinksRelNoFollow || !regex_match(completeHtmlATag.c_str(), PageParser::s_RelNoFollowRegex))
            {
            WString linkString = match[1].str().c_str();
            try
                {
                UrlCPtr pNewLink = Url::Create(linkString, url);
                content.AddLink(*pNewLink);
                }
            catch(InvalidUrlException const&) { }
            }
        }
    }

PageContentPtr PageParser::GetEmptyPageContent(UrlCR url) const
    {
    PageContentPtr content = PageContent::Create(url, L"");

    return content;
    }
