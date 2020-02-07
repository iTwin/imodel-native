/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECSchemaXmlContextUtils.h"

USING_NAMESPACE_BENTLEY_EC

using SchemaConversionStatus = ECSchemaXmlContextUtils::SchemaConversionStatus;
using LocaterCallback = ECSchemaXmlContextUtils::LocaterCallback;
using LocaterCallbackUPtr = ECSchemaXmlContextUtils::LocaterCallbackUPtr;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
LocaterCallback::LocaterCallback(Napi::FunctionReference&& cb) : m_callback(std::forward<Napi::FunctionReference&&>(cb))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
ECSchemaPtr LocaterCallback::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    Napi::Env env = m_callback.Env();

    Napi::Object jsKey = Napi::Object::New(env);
    jsKey.Set(Napi::String::New(env, "name"), Napi::String::New(env, key.GetName().c_str()));
    jsKey.Set(Napi::String::New(env, "readVersion"), Napi::Number::New(env, (int) key.GetVersionRead()));
    jsKey.Set(Napi::String::New(env, "writeVersion"), Napi::Number::New(env, (int) key.GetVersionWrite()));
    jsKey.Set(Napi::String::New(env, "minorVersion"), Napi::Number::New(env, (int) key.GetVersionMinor()));

    auto result = m_callback.MakeCallback(env.Global(), { jsKey, Napi::Number::New(env, (int) matchType) });

    if (result.IsUndefined())
        return nullptr;

    if (!result.IsString())
        Napi::TypeError::New(env, "Schema locater callback must return a string or undefined.").ThrowAsJavaScriptException();

    BeFileName locatedPath(result.As<Napi::String>().Utf8Value().c_str(), BentleyCharEncoding::Utf8);

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlFile(schema, locatedPath, schemaContext);
    return schema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
ECSchemaReadContextPtr ECSchemaXmlContextUtils::CreateSchemaReadContext(Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin& locations)
    {
    auto context = ECSchemaReadContext::CreateContext(false, true);

    // Always include ECDb as a reference path
    BeFileName ecdbSchemaPath = locations.GetDgnPlatformAssetsDirectory().Combine({ L"ECSchemas", L"ECDb" });
    ecdbSchemaPath.AppendSeparator();
    context->AddSchemaPath(ecdbSchemaPath.GetName());
    return context;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
void ECSchemaXmlContextUtils::SetSchemaLocater(ECSchemaReadContextR context, LocaterCallbackUPtr& currentLocater, Napi::FunctionReference&& callback)
    {
    if (nullptr != currentLocater)
        context.RemoveSchemaLocater(*currentLocater);

    currentLocater = std::make_unique<LocaterCallback>(std::forward<Napi::FunctionReference&&>(callback));
    context.SetFinalSchemaLocater(*currentLocater);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Chris.Lawson     02/2020
//---------------------------------------------------------------------------------------
void ECSchemaXmlContextUtils::SetFirstSchemaLocater(ECSchemaReadContextR context, LocaterCallbackUPtr& currentLocater, Napi::FunctionReference&& callback)
	{
	if (nullptr != currentLocater)
		context.RemoveSchemaLocater(*currentLocater);

	currentLocater = std::make_unique<LocaterCallback>(std::forward<Napi::FunctionReference&&>(callback));
	context.AddFirstSchemaLocater(*currentLocater);
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
void ECSchemaXmlContextUtils::AddSchemaPath(ECSchemaReadContextR context, Utf8StringCR schemaPath)
    {
    BeFileName schemaDir(schemaPath);
    context.AddSchemaPath(schemaDir);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
Utf8CP ECSchemaXmlContextUtils::SchemaConversionStatusToString(SchemaConversionStatus status)
    {
    switch (status)
        {
        case SchemaConversionStatus::FailedToParseXml: return "FailedToParseXml";
        case SchemaConversionStatus::InvalidECSchemaXml: return "InvalidECSchemaXml";
        case SchemaConversionStatus::ReferencedSchemaNotFound: return "ReferencedSchemaNotFound";
        case SchemaConversionStatus::DuplicateSchema: return "DuplicateSchema";
        case SchemaConversionStatus::DuplicateTypeName: return "DuplicateTypeName";
        case SchemaConversionStatus::InvalidPrimitiveType: return "InvalidPrimitiveType";
        case SchemaConversionStatus::HasReferenceCycle: return "HasReferenceCycle";
        case SchemaConversionStatus::FailedToCreateJson: return "FailedToCreateJson";
        }

    return "Unknown Error";
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     03/2018
//---------------------------------------------------------------------------------------
SchemaConversionStatus ECSchemaXmlContextUtils::ConvertECSchemaXmlToJson(JsonValueR results, ECSchemaReadContextR context, Utf8StringCR ecSchemaXmlFile)
    {
    BeFileName xmlFileName(ecSchemaXmlFile);
    ECSchemaPtr schema;
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlFile(schema, xmlFileName, context);

    if (SchemaReadStatus::Success != readStatus)
        return static_cast<SchemaConversionStatus>(readStatus);

    if (!schema.IsValid() || true != schema->WriteToJsonValue(results))
        return SchemaConversionStatus::FailedToCreateJson;

    return SchemaConversionStatus::Success;
    }
