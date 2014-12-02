/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/DgnEC/DgnECInteropStringFormatter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "DgnECInteropStringFormatter.h"
#include "DgnECTypeAdapters.h"
#include <boost/algorithm/string/replace.hpp>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool InteropStringFormatter::Format (WStringR formatted, WCharCP inFormatString, IECValueList const& values) const
    {
    // Barebones formatting logic only. We currently don't want to have to implement full support for .NET formatting options.
    // ###TODO: When Android has regex support, use regex to parse formatString
    // ###TODO: Localization for DateTime and numeric formatting
    formatted.clear();
    if (NULL == inFormatString)
        return true;

    // replace any escaped right brackets now
    WString formatString (inFormatString);
    boost::algorithm::replace_all (formatString, L"}}", L"}");

    WCharCP cur = formatString.c_str(), end = formatString.c_str() + formatString.length(), next;
    while (cur < end && NULL != (next = ::wcschr (cur, L'{')))
        {
        formatted.append (cur, next - cur);
        if (next+1 < end && *(next+1) == '{')
            {
            formatted.append (1, L'{');
            cur = next+2;
            }
        else
            {
            WCharCP tokenEnd = ::wcschr (next, L'}');
            UInt32 argIndex;
            ECValueCP arg;
            if (NULL == tokenEnd || 1 != BE_STRING_UTILITIES_SWSCANF (next+1, L"%ud", &argIndex) || NULL == (arg = values[argIndex]))
                return false;
            else if (!arg->IsNull())    // null value treated as empty string
                {
                // Look for alignment (e.g. "{0,10}", "{0,-5:formatString}")
                next += 2;  // skip "{i" where i is arg index
                Int32 alignment = 0;
                if (',' == *next)
                    BE_STRING_UTILITIES_SWSCANF (++next, L"%d", &alignment);

                // Look for format string
                WString fmt;
                WCharCP fmtStart = ::wcschr (next, L':');
                if (NULL != fmtStart && fmtStart < tokenEnd)
                    fmt = WString (fmtStart+1, tokenEnd - (fmtStart+1));

                // Format the argument
                WString argAsString;
                if (!FormatValue (argAsString, fmt.c_str(), *arg))
                    return false;

                // Apply alignment. Negative alignment = left-aligned, positive = right-aligned
                Int32 nPadding = abs(alignment) - (Int32)argAsString.length();
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
bool InteropStringFormatter::FormatValue (WStringR argAsString, WCharCP fmt, ECValueCR v) const
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
            argAsString = v.GetBoolean() ? DgnHandlersMessage::GetStringW (DgnHandlersMessage::MSGID_ECTYPEADAPTER_True) : 
                DgnHandlersMessage::GetStringW (DgnHandlersMessage::MSGID_ECTYPEADAPTER_False);
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
bool InteropStringFormatter::Parse (ECValueR v, WCharCP rawString, PrimitiveType primType) const
    {
    WString str (rawString ? rawString : L"");
    if (primType == PRIMITIVETYPE_String)
        {
        v.SetString (rawString);
        return true;
        }

    str.Trim();
    switch (primType)
        {
    case PRIMITIVETYPE_Integer:
            {
            Int32 i;
            if (1 != BE_STRING_UTILITIES_SWSCANF (str.c_str(), L"%d", &i))
                return false;

            v.SetInteger (i);
            return true;
            }
    case PRIMITIVETYPE_Long:
            {
            Int64 l;
            if (1 != BE_STRING_UTILITIES_SWSCANF (str.c_str(), L"%lld", &l))
                return false;

            v.SetLong (l);
            return true;
            }
    case PRIMITIVETYPE_Double:
            {
            double d;
            DoubleParserPtr parser = DoubleParser::Create();
            if (SUCCESS != parser->ToValue (d, str.c_str()))
                return false;

            v.SetDouble (d);
            return true;
            }
    case PRIMITIVETYPE_Boolean:
            {
            if (str.EqualsI (DgnHandlersMessage::GetStringW (DgnHandlersMessage::MSGID_ECTYPEADAPTER_True)))
                v.SetBoolean (true);
            else if (str.EqualsI (DgnHandlersMessage::GetStringW (DgnHandlersMessage::MSGID_ECTYPEADAPTER_False)))
                v.SetBoolean (false);
            else
                return false;

            return true;
            }
    case PRIMITIVETYPE_Point2D:
    case PRIMITIVETYPE_Point3D:
            {
            bool is3d = PRIMITIVETYPE_Point3D == primType;
            DPoint3d pt;
            PointParserPtr parser = PointParser::Create();
            parser->SetIs3d (is3d);

            if (SUCCESS != parser->ToValue (pt, str.c_str()))
                return false;

            if (is3d)
                v.SetPoint3D (pt);
            else
                v.SetPoint2D (DPoint2d::From (pt.x, pt.y));

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
        BeAssert (false && L"Unrecognized ECN::PrimitiveType");
        return false;
        }
    }

