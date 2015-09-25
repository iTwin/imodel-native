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

//=======================================================================================
//! @bsiclass
// A DomainName is a string that identifies a Web domain.
//=======================================================================================
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

//=======================================================================================
//! @bsiclass
// This class represent an URL. Here the concept of URL is defined in a context of 
// crawling and recognises the relation of possible existence of a parent URL,
// a depth from the origin of crawl, indication the URL references an external page,
// and even the fact a URL is a sub-url from some parent.
//
// In order to support ordered containers equal and an arbitrary less than operator
// are defined.
//
// The class defines regex match patterns used in the classification of URLs
//
//=======================================================================================
struct Url : public RefCountedBase
    {
    public:
    
    //---------------------------------------------------------------------------------------
    // This constructor creates a URL as defined in the context of a crawling library only.
    // The object is created by providing a URL and a parent URL. Normally in the context
    // of crawling there always a parent except for a Seed URL (See class Seed).
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    CRAWLERLIB_EXPORT Url(WString const& url, UrlPtr const& parent);
    CRAWLERLIB_EXPORT virtual ~Url() {}

    CRAWLERLIB_EXPORT inline DomainName const& GetDomainName() const {return m_DomainName;}
    CRAWLERLIB_EXPORT inline WString const& GetUrlWString() const {return m_Url;}

    
    //---------------------------------------------------------------------------------------
    // Returns pointer to parent Url. This parent can be null for seeds.
    //---------------------------------------------------------------------------------------
    CRAWLERLIB_EXPORT inline UrlPtr const& GetParent() const {return m_Parent;}

    
    //---------------------------------------------------------------------------------------
    // Returns depth of Url path. Seeds have a depth of 0.
    //---------------------------------------------------------------------------------------
    CRAWLERLIB_EXPORT inline uint32_t GetDepth() const {return m_Depth;}
    CRAWLERLIB_EXPORT inline bool IsExternalPage() const {return m_IsExternalPage;}
    CRAWLERLIB_EXPORT bool IsSubUrlOf(Url const& parent);

    CRAWLERLIB_EXPORT inline bool operator==(Url const& other) const;
    CRAWLERLIB_EXPORT inline bool operator<(Url const& other) const;


    protected:
    //---------------------------------------------------------------------------------------
    // The default constructor is only provided for subclasses (such as Seed).
    // It is the responsibility of the subclass that the Url be valid and that all members 
    // are properly initialized.
    //
    // @bsimethod                                                 Alexandre.Gariepy   08/15
    //+---------------+---------------+---------------+---------------+---------------+------
    Url() {} 

    void RemoveTrailingSlash(WString& urlString) const;

    UrlPtr m_Parent; // Parent URL. Usually non-null in a plain URL but subclass can decide otherwise.
    WString m_Url;
    DomainName m_DomainName;
    bool m_IsExternalPage;
    uint32_t m_Depth;

    static const std::wregex s_UrlRegex;
    static const std::wregex s_RelativeUrlRegex;
    static const std::wregex s_DomainNameRegex;
    static const std::wregex s_RelativeUrlWithDotRegex;
    };

//=======================================================================================
//! @bsiclass
// A seed is a simple classifying overload of a URL. It serves the purpose of 
// explicetely declaring the intent of the specific URL to be used as a seed.
// It also allows a URL that has no parent which cannot be for a Url object.
//=======================================================================================
struct Seed : public Url
    {
    public:
    CRAWLERLIB_EXPORT Seed(WString const& url);
    };

//=======================================================================================
//! @bsiclass
// Exception class used when an invalid exception in encountered.
//=======================================================================================
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

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct UrlPtrCompare
    {
    bool operator() (UrlPtr const& lhs, UrlPtr const& rhs) const {return *lhs < *rhs;}
    };

typedef std::set<UrlPtr, UrlPtrCompare> UrlPtrSet;

END_BENTLEY_CRAWLERLIB_NAMESPACE
