/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

PUSH_REVIEWED_STATIC_ANALYSIS_WARNING(6313) // Incorrect operator:  zero-valued flag cannot be tested with bitwise-and.  Use an equality test to check for zero-valued flags.
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
POP_REVIEWED_STATIC_ANALYSIS_WARNING

/** @namespace rapidjson rapidjson JSON parser */

BEGIN_BENTLEY_NAMESPACE

typedef rapidjson::Document& RapidJsonDocumentR;
typedef rapidjson::Document const& RapidJsonDocumentCR;

typedef rapidjson::Value& RapidJsonValueR;
typedef rapidjson::Value const& RapidJsonValueCR;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BeRapidJsonUtilities
    {
    //! Returns a UInt64 value from a rapidjson::Value that may be integral or string.
    //! @param[in] value the source rapidjson::Value
    //! @param[in] defaultOnError the UInt64 value to return if it is not possible to convert the source value to UInt64.
    static uint64_t UInt64FromValue(RapidJsonValueCR value, uint64_t defaultOnError = 0)
        {
        if (value.IsNull())
            return defaultOnError;
        if (value.IsNumber())
            return value.GetUint64();
        if (value.IsString())
            {
            uint64_t returnValueInt64 = defaultOnError;
            Utf8String::Sscanf_safe(value.GetString(), "%" PRIu64, &returnValueInt64);
            return returnValueInt64;
            }
        return defaultOnError;
        }

    //! Returns an Int64 value from a rapidjson::Value that may be integral or string.
    //! @param[in] value the source rapidjson::Value
    //! @param[in] defaultOnError the Int64 value to return if it is not possible to convert the source value to Int64.
    static int64_t Int64FromValue(RapidJsonValueCR value, int64_t defaultOnError = 0)
        {
        if (value.IsNull())
            return defaultOnError;
        if (value.IsNumber())
            return value.GetInt64();
        if (value.IsString())
            {
            int64_t returnValueInt64 = defaultOnError;
            Utf8String::Sscanf_safe(value.GetString(), "%" PRId64, &returnValueInt64);
            return returnValueInt64;
            }
        return defaultOnError;
        }

    //! Serializes the supplied JSON value.
    static Utf8String ToString(RapidJsonValueCR json)
        {
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        json.Accept(writer);
        return buf.GetString();
        }

    //! Serializes the supplied JSON value in a pretty formatted way.
    static Utf8String ToPrettyString(RapidJsonValueCR json)
        {
        rapidjson::StringBuffer buf;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
        json.Accept(writer);
        return buf.GetString();
        }
    };

END_BENTLEY_NAMESPACE
