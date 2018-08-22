/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/BeUri.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/BeUri.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <regex>

USING_NAMESPACE_BENTLEY_HTTP

std::basic_regex<Utf8String::value_type> s_uriPattern
(
R"(^([a-z][a-z0-9+\-.]*):(?:\/\/((?:([^@\/?#]*)@)?((?:[^:\/?#\[\]]|\[[^\]\/]+\])*)(?::(\d+))?))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?$)",
std::regex_constants::icase
);

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeUri::MatchResults
    {
    std::match_results<Utf8String::const_iterator> matches;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BeUri::BeUri(Utf8StringCR uri)
    {
    if (uri.empty())
        return;

    MatchResults results;
    if (!std::regex_search(uri.c_str(), results.matches, s_uriPattern))
        return;

    if (!results.matches[0].matched)
        return;

    bool authorityIsPresent = results.matches[static_cast<size_t>(UriPatternGroup::Authority)].matched;
    Utf8String path = EscapeUriComponent(GetUriComponent(UriPatternGroup::Path, results));
    if (authorityIsPresent && !path.empty() && '/' != path[0])
        return;

    m_scheme = GetUriComponent(UriPatternGroup::Scheme, results);
    m_userInfo = GetUriComponent(UriPatternGroup::Userinfo, results);
    m_host = GetUriComponent(UriPatternGroup::Host, results);
    m_path = path;
    m_query = EscapeUriComponent(GetUriComponent(UriPatternGroup::Query, results));
    m_fragment = EscapeUriComponent(GetUriComponent(UriPatternGroup::Fragment, results));

    Utf8String portComponent = GetUriComponent(UriPatternGroup::Port, results);
    if (!portComponent.empty())
        m_port = atoi(portComponent.c_str());

    m_authorityIsPresent = authorityIsPresent;
    m_valid = true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Robert.Lukasonok   07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::GetUriComponent(UriPatternGroup group, const MatchResults& results)
    {
    std::sub_match<Utf8String::const_iterator> subMatch = results.matches[static_cast<size_t>(group)];

    if (!subMatch.matched)
        return "";

    return Utf8String(subMatch.first, subMatch.second);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::EscapeUriComponent(Utf8String component)
    {
    // More information:
    // http://tools.ietf.org/html/rfc1738#section-2.2
    // http://tools.ietf.org/html/rfc2396

    component.ReplaceAll(R"(<)", "%3C");
    component.ReplaceAll(R"(>)", "%3E");
    component.ReplaceAll(R"(")", "%22");
    component.ReplaceAll(R"(#)", "%23");
    // "%" should be encoded by client
    component.ReplaceAll(R"({)", "%7B");
    component.ReplaceAll(R"(})", "%7D");
    component.ReplaceAll(R"(|)", "%7C");
    component.ReplaceAll(R"(\)", "%5C");
    component.ReplaceAll(R"(^)", "%5E");
    // "~" is considered safe
    component.ReplaceAll(R"([)", "%5B");
    component.ReplaceAll(R"(])", "%5D");
    component.ReplaceAll(R"(`)", "%60");
    // Whitespace character?

    return component;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String& BeUri::EscapeUnsafeCharactersInUrl(Utf8String& url)
    {
    BeUri uri(url);
    url = uri.ToString();
    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::EscapeString(Utf8StringCR str)
    {
    Utf8P escapedStr = curl_escape(str.c_str(), (int) str.length());
    Utf8String outStr = escapedStr;
    curl_free(escapedStr);
    return outStr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::UnescapeString(Utf8StringCR str)
    {
    Utf8P unescapedStr = curl_unescape(str.c_str(), (int) str.length());
    Utf8String outStr = unescapedStr;
    curl_free(unescapedStr);
    return outStr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::ToString() const
    {
    Utf8String uri;
    if (!m_scheme.empty())
        uri += GetScheme() + ":";

    if (m_authorityIsPresent)
        uri += "//" + GetAuthority();

    uri += m_path;
    Utf8String query = GetQuery();
    if (!query.empty())
        uri += "?" + query;

    Utf8String fragment = GetFragment();
    if (!fragment.empty())
        uri += "#" + fragment;

    return uri;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::GetAuthority() const
    {
    Utf8String authority = m_userInfo;
    if (!authority.empty())
        authority += '@';

    authority += GetHost();
    if (m_port >= 0)
        authority += Utf8PrintfString(":%d", m_port);

    return authority;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::GetPath() const
    {
    if (!m_valid)
        return "";

    if (m_path.empty())
        return "/";

    return m_path;
    }
