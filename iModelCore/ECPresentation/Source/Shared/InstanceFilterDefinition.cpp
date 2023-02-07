/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
static BeJsConst ToBeJsConst(rapidjson::Value const& value)
    {
    // `BeJsConst` for `rapidjson::Value const&` requires an allocator, although it's completely
    // read-only and doesn't use it. The only reason it requires an allocator is that it uses
    // a read-write `BeJsValue` for all the operations.
    static rapidjson::MemoryPoolAllocator<> s_staticRapidJsonAllocator(8);
    return BeJsConst(value, s_staticRapidJsonAllocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document InstanceFilterDefinition::ToInternalJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    DefaultECPresentationSerializer serializer;
    return serializer.AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<InstanceFilterDefinition> InstanceFilterDefinition::FromInternalJson(RapidJsonValueCR json, IConnectionCR connection)
    {
    DefaultECPresentationSerializer serializer;
    return serializer.GetInstanceFilterFromJson(connection, ToBeJsConst(json));
    }
