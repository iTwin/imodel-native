/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/DgnECInteropStringFormatter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "DgnECInteropStringFormatter.h"
#include "DgnECTypeAdapters.h"

void replaceAll(Utf8StringR str, Utf8CP from, Utf8CP to)
    {
    size_t start_pos = 0;
    size_t fromLen = ::strlen(from);
    size_t toLen = ::strlen(to);
    while ((start_pos = str.find(from, start_pos)) != Utf8String::npos)
        {
        size_t end_pos = start_pos + fromLen;
        str.replace(start_pos, end_pos, to);
        start_pos += toLen; // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool InteropStringFormatter::Format (Utf8StringR formatted, Utf8CP inFormatString, IECValueList const& values) const
    {
    // Barebones formatting logic only. We currently don't want to have to implement full support for .NET formatting options.
    // ###TODO: When Android has regex support, use regex to parse formatString
    // ###TODO: Localization for DateTime and numeric formatting
    formatted.clear();
    if (NULL == inFormatString)
        return true;

    // replace any escaped right brackets now
    Utf8String formatString (inFormatString);
    replaceAll(formatString, "}}", "}");

    Utf8CP cur = formatString.c_str(), end = formatString.c_str() + formatString.length(), next;
    while (cur < end && NULL != (next = ::strchr (cur, '{')))
        {
        formatted.append (cur, next - cur);
        if (next+1 < end && *(next+1) == '{')
            {
            formatted.append (1, L'{');
            cur = next+2;
            }
        else
            {
            Utf8CP tokenEnd = ::strchr (next, L'}');
            uint32_t argIndex;
            ECValueCP arg;
            if (NULL == tokenEnd || 1 != BE_STRING_UTILITIES_UTF8_SSCANF(next + 1, "%ud", &argIndex) || NULL == (arg = values[argIndex]))
                return false;
            else if (!arg->IsNull())    // null value treated as empty string
                {
                // Look for alignment (e.g. "{0,10}", "{0,-5:formatString}")
                next += 2;  // skip "{i" where i is arg index
                int32_t alignment = 0;
                if (',' == *next)
                    BE_STRING_UTILITIES_UTF8_SSCANF(++next, "%d", &alignment);

                // Look for format string
                Utf8String fmt;
                Utf8CP fmtStart = ::strchr (next, L':');
                if (NULL != fmtStart && fmtStart < tokenEnd)
                    fmt = Utf8String (fmtStart+1, tokenEnd - (fmtStart+1));

                // Format the argument
                Utf8String argAsString;
                if (!FormatValue (argAsString, fmt.c_str(), *arg))
                    return false;

                // Apply alignment. Negative alignment = left-aligned, positive = right-aligned
                int32_t nPadding = abs(alignment) - (int32_t)argAsString.length();
                if (nPadding > 0 && alignment > 0)
                    formatted.append ((size_t)nPadding, L' ');

                formatted.append (argAsString);

                if (nPadding > 0 && alignment < 0)
                    formatted.append ((size_t)nPadding, L' ');
                }

            cur = tokenEnd + 1;
            }
        }

    if (cur != NULL && cur < end)
        formatted.append (cur, end - cur);

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool InteropStringFormatter::FormatValue (Utf8StringR argAsString, Utf8CP fmt, ECValueCR v) const
    {
    argAsString.clear();
    if (v.IsNull())
        return true;        // treat as empty string
    else if (NULL != fmt && 0 != *fmt)
        {
        if (v.SupportsDotNetFormatting())
            return v.ApplyDotNetFormatting (argAsString, fmt);
        if (v.IsBoolean())
            {
            argAsString = v.GetBoolean() ? DgnCoreL10N::GetString(DgnCoreL10N::ECTYPEADAPTER_True()) : DgnCoreL10N::GetString(DgnCoreL10N::ECTYPEADAPTER_False());
            return true;
            }
        else if (v.IsDateTime())
            {
            DateTimeTypeAdapter::FormatDateTime (argAsString, fmt, v.GetDateTime());
            return true;
            }
        }

    // Just do default formatting
    argAsString = v.ToString();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool InteropStringFormatter::Parse(ECValueR v, Utf8CP rawString, PrimitiveType primType) const
    {
    Utf8String str(rawString ? rawString : "");
    if (primType == PRIMITIVETYPE_String)
        {
        v.SetUtf8CP(rawString);
        return true;
        }

    str.Trim();
    switch (primType)
        {
        case PRIMITIVETYPE_Integer:
            {
            int32_t i;
            if (1 != BE_STRING_UTILITIES_UTF8_SSCANF(str.c_str(), "%d", &i))
                return false;

            v.SetInteger(i);
            return true;
            }
        case PRIMITIVETYPE_Long:
            {
            int64_t l;
            if (1 != BE_STRING_UTILITIES_UTF8_SSCANF(str.c_str(), "%" SCNd64, &l))
                return false;

            v.SetLong(l);
            return true;
            }
        case PRIMITIVETYPE_Double:
            {
            double d;
            DoubleParserPtr parser = DoubleParser::Create();
            if (SUCCESS != parser->ToValue(d, str.c_str()))
                return false;

            v.SetDouble(d);
            return true;
            }
        case PRIMITIVETYPE_Boolean:
            {
            if (str.Equals(DgnCoreL10N::GetString(DgnCoreL10N::ECTYPEADAPTER_True())))
                v.SetBoolean(true);
            else if (str.Equals(DgnCoreL10N::GetString(DgnCoreL10N::ECTYPEADAPTER_False())))
                v.SetBoolean(false);
            else
                return false;

            return true;
            }
        case PRIMITIVETYPE_Point2d:
        case PRIMITIVETYPE_Point3d:
            {
            bool is3d = PRIMITIVETYPE_Point3d == primType;
            DPoint3d pt;
            PointParserPtr parser = PointParser::Create();
            parser->SetIs3d(is3d);

            if (SUCCESS != parser->ToValue(pt, str.c_str()))
                return false;

            if (is3d)
                v.SetPoint3d(pt);
            else
                v.SetPoint2d(DPoint2d::From(pt.x, pt.y));

            return true;
            }
        case PRIMITIVETYPE_DateTime:
            // ###TODO: native support for parsing datetime
            return false;
        case PRIMITIVETYPE_Binary:
        case PRIMITIVETYPE_IGeometry:
            // No meaningful string representation
            return false;
        default:
            BeAssert(false && "Unrecognized ECN::PrimitiveType");
            return false;
        }
    }

