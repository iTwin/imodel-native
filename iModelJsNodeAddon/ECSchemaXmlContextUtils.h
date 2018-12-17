/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSchemaXmlContextUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <ECObjects/ECObjectsAPI.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Napi/napi.h>

USING_NAMESPACE_BENTLEY

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
//! @bsiclass                                    Bill.Goehrig                   03/2018
//=======================================================================================
struct ECSchemaXmlContextUtils
    {
    struct LocaterCallback : IECSchemaLocater
        {
        private:
            Napi::FunctionReference m_callback;
            ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override;

        public:
            LocaterCallback(Napi::FunctionReference&& cb);
            virtual ~LocaterCallback() {}
        };

    enum class SchemaConversionStatus : std::underlying_type<SchemaReadStatus>::type
        {
        Success = std::underlying_type<SchemaReadStatus>::type(SchemaReadStatus::Success),
        FailedToParseXml = std::underlying_type<SchemaReadStatus>::type(SchemaReadStatus::FailedToParseXml),
        InvalidECSchemaXml = std::underlying_type<SchemaReadStatus>::type(SchemaReadStatus::InvalidECSchemaXml),
        ReferencedSchemaNotFound = std::underlying_type<SchemaReadStatus>::type(SchemaReadStatus::ReferencedSchemaNotFound),
        DuplicateSchema = std::underlying_type<SchemaReadStatus>::type(SchemaReadStatus::DuplicateSchema),
        DuplicateTypeName = std::underlying_type<SchemaReadStatus>::type(SchemaReadStatus::DuplicateTypeName),
        InvalidPrimitiveType = std::underlying_type<SchemaReadStatus>::type(SchemaReadStatus::InvalidPrimitiveType),
        HasReferenceCycle = std::underlying_type<SchemaReadStatus>::type(SchemaReadStatus::HasReferenceCycle),
        FailedToCreateJson
        };

    using LocaterCallbackUPtr = std::unique_ptr<LocaterCallback>;

    static ECSchemaReadContextPtr CreateSchemaReadContext(Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin&);
    static void AddSchemaPath(ECSchemaReadContextR context, Utf8StringCR schemaPath);
    static void SetSchemaLocater(ECSchemaReadContextR context, LocaterCallbackUPtr& currentLocater, Napi::FunctionReference&& callback);
    static SchemaConversionStatus ConvertECSchemaXmlToJson(JsonValueR results, ECSchemaReadContextR context, Utf8StringCR ecSchemaXmlFile);
    static Utf8CP SchemaConversionStatusToString(SchemaConversionStatus status);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
