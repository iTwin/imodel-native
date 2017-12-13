/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/HttpHeaders.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/HttpHeaders.h>
#include <Bentley/Base64Utilities.h>
#include <regex>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpHeaders::SetValue(Utf8StringCR field, Utf8StringCR value)
    {
    if (value.empty())
        {
        m_headers.erase(field);
        return;
        }
    m_headers[field] = value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpHeaders::AddValue(Utf8StringCR field, Utf8StringCR value)
    {
    if (m_headers.find(field) == m_headers.end())
        {
        m_headers[field] = value;
        }
    else
        {
        m_headers[field] += Utf8String(", ") + value;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpHeaders::GetValue(Utf8StringCR field) const
    {
    auto position = m_headers.find(field);
    if (position == m_headers.end())
        {
        return nullptr;
        }
    return (*position).second.c_str();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetAnsiCFormattedDateTime(DateTimeCR dateTime)
    {
    Utf8CP wkday = "";
    switch (dateTime.GetDayOfWeek())
        {
        case DateTime::DayOfWeek::Sunday:   wkday = "Sun"; break;
        case DateTime::DayOfWeek::Monday:   wkday = "Mon"; break;
        case DateTime::DayOfWeek::Tuesday:  wkday = "Tue"; break;
        case DateTime::DayOfWeek::Wednesday:wkday = "Wed"; break;
        case DateTime::DayOfWeek::Thursday: wkday = "Thu"; break;
        case DateTime::DayOfWeek::Friday:   wkday = "Fri"; break;
        case DateTime::DayOfWeek::Saturday: wkday = "Sat"; break;
        }

    Utf8CP month = "";
    switch (dateTime.GetMonth())
        {
        case 1:  month = "Jan"; break;
        case 2:  month = "Feb"; break;
        case 3:  month = "Mar"; break;
        case 4:  month = "Apr"; break;
        case 5:  month = "May"; break;
        case 6:  month = "Jun"; break;
        case 7:  month = "Jul"; break;
        case 8:  month = "Aug"; break;
        case 9:  month = "Sep"; break;
        case 10: month = "Oct"; break;
        case 11: month = "Nov"; break;
        case 12: month = "Dec"; break;
        }

    return Utf8PrintfString("%s %s %02d %02d:%02d:%02d %d", wkday, month, dateTime.GetDay(), dateTime.GetHour(), dateTime.GetMinute(), dateTime.GetSecond(), dateTime.GetYear());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                     Grigas.Petraitis               07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetIfModifiedSince(DateTimeCR dateTime) {SetIfModifiedSince(GetAnsiCFormattedDateTime(dateTime));}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RangeHeaderValue::Parse(Utf8CP stringValue, RangeHeaderValue& valueOut)
    {
    if (nullptr == stringValue)
        {
        return ERROR;
        }
    if (2 != sscanf(stringValue, "bytes=%" SCNu64 "-%" SCNu64, &valueOut.m_from, &valueOut.m_to))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RangeHeaderValue::ToString() const
    {
    return Utf8PrintfString("bytes=%" PRIu64 "-%" PRIu64, m_from, m_to);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ContentRangeHeaderValue::Parse(Utf8CP stringValue, ContentRangeHeaderValue& valueOut)
    {
    if (nullptr == stringValue)
        {
        return ERROR;
        }
    if (3 == sscanf(stringValue, "bytes %" PRIu64 "-%" PRIu64 "/%" PRIu64, &valueOut.m_from, &valueOut.m_to, &valueOut.m_length))
        {
        valueOut.m_hasRange = true;
        valueOut.m_hasLength = true;
        return SUCCESS;
        }
    if (2 == sscanf(stringValue, "bytes %" PRIu64 "-%" PRIu64 "/*", &valueOut.m_from, &valueOut.m_to))
        {
        valueOut.m_hasRange = true;
        valueOut.m_hasLength = false;
        return SUCCESS;
        }
    if (1 == sscanf(stringValue, "bytes */%" PRIu64, &valueOut.m_length))
        {
        valueOut.m_hasRange = false;
        valueOut.m_hasLength = true;
        return SUCCESS;
        }
    if (0 == strcmp("bytes */*", stringValue))
        {
        valueOut.m_hasRange = false;
        valueOut.m_hasLength = false;
        return SUCCESS;
        }
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentRangeHeaderValue::ToString() const
    {
    Utf8String stringValue;
    if (HasLength() && HasRange())
        {
        stringValue.Sprintf("bytes %" SCNu64 "-%" SCNu64 "/%" SCNu64, m_from, m_to, m_length);
        }
    else if (HasRange())
        {
        stringValue.Sprintf("bytes %" SCNu64 "-%" SCNu64 "/*", m_from, m_to);
        }
    else if (HasLength())
        {
        stringValue.Sprintf("bytes */%" SCNu64, m_length);
        }
    else
        {
        stringValue.Sprintf("bytes */*");
        }
    return stringValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AuthenticationChallengeValue::Parse(Utf8CP stringValue, AuthenticationChallengeValue& valueOut)
    {
    if (stringValue == nullptr)
        return ERROR;

    std::regex regex(R"rx((^[^ ]+)( +realm="([^"]*)")?[\s]*$)rx", std::regex_constants::icase);
    std::cmatch matches;
    std::regex_search(stringValue, matches, regex);

    if (matches.empty())
        return ERROR;

    if (matches.size() != 4)
        return ERROR;

    if (matches[1].matched)
        valueOut.m_type = matches[1].str().c_str();

    if (matches[3].matched)
        valueOut.m_realm = matches[3].str().c_str();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AuthenticationChallengeValue::ToString() const
    {
    if (m_realm.empty())
        return m_type;

    if (m_type.empty())
        return "";

    return Utf8PrintfString("%s realm=\"%s\"", m_type.c_str(), m_realm.c_str());
    }
