/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/Url.h $
| 
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/RefCounted.h>

#include <regex>
#include <exception>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE
struct Url : public RefCountedBase
    {
    public: 
    CRAWLERLIB_EXPORT Url(WString const& url);
    CRAWLERLIB_EXPORT Url(Url const& baseUrl, WString const& relativePath);
    CRAWLERLIB_EXPORT Url(Url const& other);
    CRAWLERLIB_EXPORT virtual ~Url() {}

    CRAWLERLIB_EXPORT inline WString GetDomainName() const {return m_DomainName;}
    inline WString const& GetUrlWString() const {return m_Url;}

    CRAWLERLIB_EXPORT inline bool operator==(Url const& other) const;
    CRAWLERLIB_EXPORT inline bool operator<(Url const& other) const;
    CRAWLERLIB_EXPORT Url& operator=(Url const& other);

    CRAWLERLIB_EXPORT static bool IsValidAbsoluteLink(WString const& link);
    CRAWLERLIB_EXPORT static bool IsValidRelativeLink(WString const& link);

    protected:
    Url() {} //For testing/mocking purpose

    WString m_Url;
    WString m_DomainName;

    private:
    static const std::wregex s_UrlRegex; 
    static const std::wregex s_RelativeUrlRegex;
    static const std::wregex s_DomainNameRegex;
    static const std::wregex s_RelativeUrlWithDotRegex;
    };

class InvalidUrlException : public std::exception
    {
    public:
    InvalidUrlException(WString url) {m_Url = url;}

    virtual const char* what() const throw()
        {
        WPrintfString message(L"The url %ls is invalid.", m_Url.c_str());
        Utf8String s;
        BeStringUtilities::WCharToUtf8 (s, message.c_str());
        return s.c_str();
        }

    private:
    WString m_Url;
    };

struct UrlPtrCompare
    {
    bool operator() (UrlPtr const& lhs, UrlPtr const& rhs) const {return *lhs < *rhs;}
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
