/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <pugixml/src/pugixml.hpp>
#include <Bentley/Bentley.h>
#include <Bentley/BeStringUtilities.h>

BEGIN_BENTLEY_NAMESPACE

//pugixml has no native way for getting the inner xml of an xml_node.
//the following solution was suggested here: https://stackoverflow.com/a/60337372
struct pugi_xml_string_writer: pugi::xml_writer
{
    std::string result;

    virtual void write(const void* data, size_t size)
    {
        result.append(static_cast<const char*>(data), size);
    };
};

enum class BePugiXmlValueResult
    {
    Success,
    Null,
    TypeMismatch,
    };

struct BePugiXmlHelper
{
public:
static std::string InnerXml(pugi::xml_node target)
    {
    pugi_xml_string_writer writer;
    for (pugi::xml_node child = target.first_child(); child; child = child.next_sibling())
        child.print(writer, "");
    return writer.result;
    };

static BePugiXmlValueResult ContentBooleanValue (pugi::xml_node node, bool& value)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    if(node.empty())
        return BePugiXmlValueResult::Null;
    auto textValue = node.text();
    if(textValue.empty())
        return BePugiXmlValueResult::Null;
    auto strValue = textValue.as_string();
    
    value = (0 == BeStringUtilities::Stricmp (strValue, "true")) || (0 == BeStringUtilities::Stricmp (strValue, "1"));
    return BePugiXmlValueResult::Success;
    };

static BePugiXmlValueResult GetContentInt64Value (pugi::xml_node node, int64_t& value)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    if(node.empty())
        return BePugiXmlValueResult::Null;
    value = 0;

    auto textValue = node.text();
    if(textValue.empty())
        return BePugiXmlValueResult::Null;
    auto strValue = textValue.as_string();

    if (1 != Utf8String::Sscanf_safe (strValue, "%" PRId64, &value))
        return BePugiXmlValueResult::TypeMismatch;

    return BePugiXmlValueResult::Success;
    };

static BePugiXmlValueResult GetContentDoubleValue (pugi::xml_node node, double& value)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    if(node.empty())
        return BePugiXmlValueResult::Null;
    value = 0.0;

    auto textValue = node.text();
    if(textValue.empty())
        return BePugiXmlValueResult::Null;
    auto strValue = textValue.as_string();

    if (1 != Utf8String::Sscanf_safe (strValue, "%lg", &value))
        return BePugiXmlValueResult::TypeMismatch;

    return BePugiXmlValueResult::Success;
    };

static BePugiXmlValueResult GetAttributeDoubleValue (pugi::xml_node node, double& value, const pugi::char_t* name)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    auto attr = node.attribute(name);
    if(!attr)
        return BePugiXmlValueResult::Null;

    value = 0.0;

    auto strValue = attr.value();

    if (1 != Utf8String::Sscanf_safe (strValue, "%lg", &value))
        return BePugiXmlValueResult::TypeMismatch;

    return BePugiXmlValueResult::Success;
    };

static BePugiXmlValueResult GetAttributeUInt32Value (pugi::xml_node node, uint32_t& value, const pugi::char_t* name)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    auto attr = node.attribute(name);
    if(!attr)
        return BePugiXmlValueResult::Null;

    value = 0;

    auto strValue = attr.value();

    if (1 != Utf8String::Sscanf_safe (strValue, "%u", &value))
        return BePugiXmlValueResult::TypeMismatch;

    return BePugiXmlValueResult::Success;
    };

static BePugiXmlValueResult GetAttributeInt32Value (pugi::xml_node node, int32_t& value, const pugi::char_t* name)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    auto attr = node.attribute(name);
    if(!attr)
        return BePugiXmlValueResult::Null;

    value = 0;

    auto strValue = attr.value();

    if (1 != Utf8String::Sscanf_safe (strValue, "%d", &value))
        return BePugiXmlValueResult::TypeMismatch;

    return BePugiXmlValueResult::Success;
    };


static BePugiXmlValueResult GetContentInt32Value (pugi::xml_node node, int32_t& value)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    if(node.empty())
        return BePugiXmlValueResult::Null;
    value = 0;

    auto textValue = node.text();
    if(textValue.empty())
        return BePugiXmlValueResult::Null;
    auto strValue = textValue.as_string();

    int ivalue = value;
    if (1 != Utf8String::Sscanf_safe (strValue, "%d", &ivalue))
        return BePugiXmlValueResult::TypeMismatch;

    value = ivalue;

    return BePugiXmlValueResult::Success;
    };

static BePugiXmlValueResult GetContentDPoint2dValue (pugi::xml_node node, double& x, double& y)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    if(node.empty())
        return BePugiXmlValueResult::Null;
    
    x = 0;
    y = 0;

    auto textValue = node.text();
    if(textValue.empty())
        return BePugiXmlValueResult::Null;
    auto strValue = textValue.as_string();

    if (2 != Utf8String::Sscanf_safe (strValue, "%lg,%lg", &x, &y))
        return BePugiXmlValueResult::TypeMismatch;

    return BePugiXmlValueResult::Success;
    };

static BePugiXmlValueResult GetContentDPoint3dValue (pugi::xml_node node, double& x, double& y, double& z)
    {
    if(!node)
        return BePugiXmlValueResult::Null;
    if(node.empty())
        return BePugiXmlValueResult::Null;
    
    x = 0;
    y = 0;
    z = 0;

    auto textValue = node.text();
    if(textValue.empty())
        return BePugiXmlValueResult::Null;
    auto strValue = textValue.as_string();

    if (3 != Utf8String::Sscanf_safe (strValue, "%lg,%lg,%lg", &x, &y, &z))
        return BePugiXmlValueResult::TypeMismatch;

    return BePugiXmlValueResult::Success;
    };

};

END_BENTLEY_NAMESPACE
/*__PUBLISH_SECTION_END__*/