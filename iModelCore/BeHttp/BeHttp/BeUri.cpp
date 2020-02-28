/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <BeHttp/BeUri.h>
#include <curl/curl.h>

#include <cstdint>
#include <cstdlib>
#include <regex>

USING_NAMESPACE_BENTLEY_HTTP

const std::basic_regex<Utf8String::value_type> s_uriSchemeAndAuthorityPattern
(
R"(^([a-zA-Z][a-zA-Z0-9+\-.]*):(?:\/\/((?:([^@\/?#]*)@)?((?:[^:\/?#\[\]]+|\[[^\]\/]+\]))?(?::(\d+))?))?)",
std::regex_constants::optimize
);
const size_t CAPTURE_GROUP_SCHEME = 1;
const size_t CAPTURE_GROUP_AUTHORITY = 2;
const size_t CAPTURE_GROUP_USER_INFO = 3;
const size_t CAPTURE_GROUP_HOST = 4;
const size_t CAPTURE_GROUP_PORT = 5;

const std::basic_regex<Utf8String::value_type> s_uriPathQueryAndFragmentPattern
(
R"(([^?#]*)(?:\?([^#]*))?(?:#(.*))?)",
std::regex_constants::optimize
);
const size_t CAPTURE_GROUP_PATH = 1;
const size_t CAPTURE_GROUP_QUERY = 2;
const size_t CAPTURE_GROUP_FRAGMENT = 3;

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

    MatchResults schemeAndAuthority;
    if (!std::regex_search(uri, schemeAndAuthority.matches, s_uriSchemeAndAuthorityPattern))
        return;

    bool authorityIsPresent = schemeAndAuthority.matches[static_cast<size_t>(CAPTURE_GROUP_AUTHORITY)].matched;

    Utf8String unmatchedPart = uri.substr(schemeAndAuthority.matches[0].length());
    MatchResults pathQueryAndFragment;
    if (!std::regex_match(unmatchedPart, pathQueryAndFragment.matches, s_uriPathQueryAndFragmentPattern))
        return;

    Utf8String path = EscapeUriComponent(GetUriComponent(CAPTURE_GROUP_PATH, pathQueryAndFragment));
    if (authorityIsPresent && !path.empty() && '/' != path[0])
        return;

    Utf8String portComponent = GetUriComponent(CAPTURE_GROUP_PORT, schemeAndAuthority);
    if (!portComponent.empty())
        {
        // std::atol return value is undefined if the converted value falls out of range of the return type.
        int32_t port = std::atol(portComponent.c_str());
        if (portComponent.length() > 5 || port > UINT16_MAX)
            {
            BeAssert(false && "BeUri: port number cannot exceed 65535.");
            return;
            }

        m_port = port;
        }

    m_scheme = GetUriComponent(CAPTURE_GROUP_SCHEME, schemeAndAuthority);
    m_userInfo = GetUriComponent(CAPTURE_GROUP_USER_INFO, schemeAndAuthority);
    m_host = GetUriComponent(CAPTURE_GROUP_HOST, schemeAndAuthority);
    m_path = path;
    m_query = EscapeUriComponent(GetUriComponent(CAPTURE_GROUP_QUERY, pathQueryAndFragment));
    m_fragment = EscapeUriComponent(GetUriComponent(CAPTURE_GROUP_FRAGMENT, pathQueryAndFragment));

    m_authorityIsPresent = authorityIsPresent;
    m_valid = true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Robert.Lukasonok   07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::GetUriComponent(size_t group, const MatchResults& results)
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
