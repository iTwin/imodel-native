/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/HttpHeaders.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/HttpHeaders.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpHeaders::~HttpHeaders ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpHeaderMapCR HttpHeaders::GetMap () const
    {
    return m_headers;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpHeaders::SetValue (Utf8StringCR field, Utf8StringCR value)
    {
    if (value.empty())
        {
        m_headers.erase(field);
        return;
        }
    m_headers[field] = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpHeaders::AddValue (Utf8StringCR field, Utf8StringCR value)
    {
    if (m_headers.find (field) == m_headers.end ())
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
Utf8CP HttpHeaders::GetValue (Utf8StringCR field) const
    {
    auto position = m_headers.find (field);
    if (position == m_headers.end ())
        {
        return nullptr;
        }
    return (*position).second.c_str ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpHeaders::Clear ()
    {
    return m_headers.clear ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetAccept (Utf8StringCR value)
    {
    SetValue ("Accept", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetAccept () const
    {
    return GetValue ("Accept");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetAcceptLanguage (Utf8StringCR value)
    {
    SetValue ("Accept-Language", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetAcceptLanguage () const
    {
    return GetValue ("Accept-Language");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetAuthorization (Utf8StringCR value)
    {
    SetValue ("Authorization", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetAuthorization () const
    {
    return GetValue ("Authorization");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetUserAgent (Utf8StringCR value)
    {
    SetValue ("User-Agent", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetUserAgent () const
    {
    return GetValue ("User-Agent");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetContentDisposition (Utf8StringCR value)
    {
    SetValue ("Content-Disposition", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetContentDisposition () const
    {
    return GetValue ("Content-Disposition");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetContentRange (Utf8StringCR value)
    {
    SetValue ("Content-Range", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetContentRange () const
    {
    return GetValue ("Content-Range");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetContentType (Utf8StringCR value)
    {
    SetValue ("Content-Type", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetContentType () const
    {
    return GetValue ("Content-Type");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetIfNoneMatch (Utf8StringCR value)
    {
    SetValue ("If-None-Match", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetIfNoneMatch () const
    {
    return GetValue ("If-None-Match");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                   Jeremy.Fisher    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetIfMatch (Utf8StringCR value)
    {
    SetValue ("If-Match", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetIfMatch () const
    {
    return GetValue ("If-Match");
    }

/*---------------------------------------------------------------------------------**//**
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetIfModifiedSince(DateTimeCR dateTime) {SetIfModifiedSince(GetAnsiCFormattedDateTime(dateTime));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequestHeaders::SetIfModifiedSince(Utf8StringCR dateTime) {SetValue("If-Modified-Since", dateTime);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpRequestHeaders::GetIfModifiedSince() const {return GetValue("If-Modified-Since");}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpResponseHeaders::SetETag (Utf8StringCR value)
    {
    SetValue ("ETag", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpResponseHeaders::GetETag () const
    {
    return GetValue ("ETag");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpResponseHeaders::SetRange (Utf8StringCR value)
    {
    SetValue ("Range", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpResponseHeaders::GetRange () const
    {
    return GetValue ("Range");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpResponseHeaders::SetContentRange (Utf8StringCR value)
    {
    SetValue ("Content-Range", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpResponseHeaders::GetContentRange () const
    {
    return GetValue ("Content-Range");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpResponseHeaders::SetContentType (Utf8StringCR value)
    {
    SetValue ("Content-Type", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpResponseHeaders::GetContentType () const
    {
    return GetValue ("Content-Type");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpResponseHeaders::SetLocation (Utf8StringCR value)
    {
    SetValue ("Location", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpResponseHeaders::GetLocation () const
    {
    return GetValue ("Location");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpResponseHeaders::SetServer (Utf8StringCR value)
    {
    SetValue ("Server", value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpResponseHeaders::GetServer () const
    {
    return GetValue ("Server");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpResponseHeaders::SetCacheControl(Utf8StringCR cacheControl) {SetValue("Cache-Control", cacheControl);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP HttpResponseHeaders::GetCacheControl() const {return GetValue("Cache-Control");}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RangeHeaderValue::RangeHeaderValue ()
: from (0), to (0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RangeHeaderValue::RangeHeaderValue (uint64_t from, uint64_t to)
: from (from), to (to)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
#define RANGE_HEADER_FORMAT "bytes=%llu-%llu"

BentleyStatus RangeHeaderValue::Parse (Utf8CP stringValue, RangeHeaderValue& valueOut)
    {
    if (nullptr == stringValue)
        {
        return ERROR;
        }
    if (2 != sscanf (stringValue, RANGE_HEADER_FORMAT, &valueOut.from, &valueOut.to))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RangeHeaderValue::ToString () const
    {
    return Utf8PrintfString (RANGE_HEADER_FORMAT, from, to);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRangeHeaderValue::ContentRangeHeaderValue ()
: m_hasRange (false), m_hasLength (false), from (0), to (0), length (0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRangeHeaderValue::ContentRangeHeaderValue (uint64_t length)
: m_hasRange (false), m_hasLength (true), from (0), to (0), length (length)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRangeHeaderValue::ContentRangeHeaderValue (uint64_t from, uint64_t to)
: m_hasRange (true), m_hasLength (false), from (from), to (to), length (0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRangeHeaderValue::ContentRangeHeaderValue (uint64_t from, uint64_t to, uint64_t length)
: m_hasRange (true), m_hasLength (true), from (from), to (to), length (length)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
#define CONTENT_RANGE_HEADER_FORMAT_FULL    "bytes %llu-%llu/%llu"
#define CONTENT_RANGE_HEADER_FORMAT_RANGE   "bytes %llu-%llu/*"
#define CONTENT_RANGE_HEADER_FORMAT_LENGTH  "bytes */%llu"
#define CONTENT_RANGE_HEADER_FORMAT_NONE    "bytes */*"

BentleyStatus ContentRangeHeaderValue::Parse (Utf8CP stringValue, ContentRangeHeaderValue& valueOut)
    {
    if (nullptr == stringValue)
        {
        return ERROR;
        }
    if (3 == sscanf (stringValue, CONTENT_RANGE_HEADER_FORMAT_FULL, &valueOut.from, &valueOut.to, &valueOut.length))
        {
        valueOut.m_hasRange = true;
        valueOut.m_hasLength = true;
        return SUCCESS;
        }
    if (2 == sscanf (stringValue, CONTENT_RANGE_HEADER_FORMAT_RANGE, &valueOut.from, &valueOut.to))
        {
        valueOut.m_hasRange = true;
        valueOut.m_hasLength = false;
        return SUCCESS;
        }
    if (1 == sscanf (stringValue, CONTENT_RANGE_HEADER_FORMAT_LENGTH, &valueOut.length))
        {
        valueOut.m_hasRange = false;
        valueOut.m_hasLength = true;
        return SUCCESS;
        }
    if (0 == strcmp (CONTENT_RANGE_HEADER_FORMAT_NONE, stringValue))
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
Utf8String ContentRangeHeaderValue::ToString () const
    {
    Utf8String stringValue;
    if (HasLength () && HasRange ())
        {
        stringValue.Sprintf (CONTENT_RANGE_HEADER_FORMAT_FULL, from, to, length);
        }
    else if (HasRange ())
        {
        stringValue.Sprintf (CONTENT_RANGE_HEADER_FORMAT_RANGE, from, to);
        }
    else if (HasLength ())
        {
        stringValue.Sprintf (CONTENT_RANGE_HEADER_FORMAT_LENGTH, length);
        }
    else
        {
        stringValue.Sprintf (CONTENT_RANGE_HEADER_FORMAT_NONE);
        }
    return stringValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRangeHeaderValue::HasRange () const
    {
    return m_hasRange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRangeHeaderValue::HasLength () const
    {
    return m_hasLength;
    }
