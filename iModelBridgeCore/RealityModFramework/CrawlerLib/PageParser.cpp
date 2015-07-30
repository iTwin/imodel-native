/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/PageParser.cpp $
| 
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/PageParser.h>
#include <regex>

USING_NAMESPACE_BENTLEY_CRAWLERLIB
using namespace std;

const wregex PageParser::s_LinkRegex = wregex(L"<\\s*a\\s+"         //The opening of the <a> tag
                                              L"[^<]*href\\s*=\\s*" //The href element
                                              L"\"([^<\"]+)\""      //The actual link to parse
                                              L"[^<]*>");           //The closing '>' of the <a> tag

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
PageContentPtr PageParser::ParsePage(UrlPtr const& url, WString const& text) const
    {
    PageContentPtr content = new PageContent(*url, text);

    AddLinksFromText(url, text, content);

    return content;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PageParser::AddLinksFromText(UrlPtr const& url, WString const& text, PageContentPtr content) const
    {
    wcregex_iterator words_begin(text.begin(), text.end(), PageParser::s_LinkRegex);
    wcregex_iterator words_end;
    for(auto i = words_begin; i != words_end; ++i)
        {
        wcmatch match = *i;
        WString matchString = match[1].str().c_str();
        if(Url::IsValidAbsoluteLink(matchString))
            {
            UrlPtr pNewLink = new Url(matchString);
            content->AddLink(pNewLink);
            }
        else if(Url::IsValidRelativeLink(matchString))
            {
            UrlPtr pNewLink = new Url(*url, matchString);
            content->AddLink(pNewLink);
            }
        }
    }
