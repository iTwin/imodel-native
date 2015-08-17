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
#include <set>
#include <exception>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

struct DomainName
    {
    friend struct Url;
    friend struct Seed;

    public:
    inline bool operator==(DomainName const& other) const {return m_DomainName.Equals(other.m_DomainName);}
    inline bool operator<(DomainName const& other) const {return m_DomainName < other.m_DomainName;}

    CRAWLERLIB_EXPORT WString const& GetWString() const {return m_DomainName;}

    private:
    DomainName() {}
    DomainName(WString const& domainName) {m_DomainName = domainName;}

    WString m_DomainName;
    };

struct Url : public RefCountedBase
    {
    public:
    CRAWLERLIB_EXPORT Url(WString const& url, UrlPtr const& parent);
    CRAWLERLIB_EXPORT virtual ~Url() {}

    CRAWLERLIB_EXPORT inline DomainName const& GetDomainName() const {return m_DomainName;}
    CRAWLERLIB_EXPORT inline WString const& GetUrlWString() const {return m_Url;}
    CRAWLERLIB_EXPORT inline UrlPtr const& GetParent() const {return m_Parent;}
    CRAWLERLIB_EXPORT inline uint32_t GetDepth() const {return m_Depth;}
    CRAWLERLIB_EXPORT inline bool IsExternalPage() const {return m_IsExternalPage;}
    CRAWLERLIB_EXPORT bool IsSubUrlOf(Url const& parent);

    CRAWLERLIB_EXPORT inline bool operator==(Url const& other) const;
    CRAWLERLIB_EXPORT inline bool operator<(Url const& other) const;


    protected:
    Url() {}
    void RemoveTrailingSlash(WString& urlString) const;

    UrlPtr m_Parent;
    WString m_Url;
    DomainName m_DomainName;
    bool m_IsExternalPage;
    uint32_t m_Depth;

    static const std::wregex s_UrlRegex;
    static const std::wregex s_RelativeUrlRegex;
    static const std::wregex s_DomainNameRegex;
    static const std::wregex s_RelativeUrlWithDotRegex;
    };

struct Seed : public Url
    {
    public:
    CRAWLERLIB_EXPORT Seed(WString const& url);
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

typedef std::set<UrlPtr, UrlPtrCompare> UrlPtrSet;
END_BENTLEY_CRAWLERLIB_NAMESPACE
