/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/IECPresentationSerializer.h>
#include "../Shared/ValueHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::DisplayLabelField const& displayLabelField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(ctx, displayLabelField, json);
    _AsJson(ctx, displayLabelField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::ECPropertiesField const& ecPropertiesField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(ctx, ecPropertiesField, json);
    _AsJson(ctx, ecPropertiesField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::CalculatedPropertyField const& calculatedPropertyField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(ctx, calculatedPropertyField, json);
    _AsJson(ctx, calculatedPropertyField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::CompositeContentField const& compositeContentField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(ctx, compositeContentField, json);
    _NestedContentFieldAsJson(ctx, compositeContentField, json);
    _AsJson(ctx, compositeContentField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::RelatedContentField const& relatedContentField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(ctx, relatedContentField, json);
    _NestedContentFieldAsJson(ctx, relatedContentField, json);
    _AsJson(ctx, relatedContentField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, FieldEditorJsonParams const& jsonParams,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _ParamsAsJson(ctx, jsonParams, json);
    _AsJson(ctx, jsonParams, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, FieldEditorMultilineParams const& multilineParams,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _ParamsAsJson(ctx, multilineParams, json);
    _AsJson(ctx, multilineParams, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, FieldEditorRangeParams const& rangeParams,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _ParamsAsJson(ctx, rangeParams, json);
    _AsJson(ctx, rangeParams, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, FieldEditorSliderParams const& sliderParams,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _ParamsAsJson(ctx, sliderParams, json);
    _AsJson(ctx, sliderParams, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::Field::TypeDescription const& typeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(ctx, typeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::Field::PrimitiveTypeDescription const& primitiveTypeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(ctx, primitiveTypeDescription, json);
    _AsJson(ctx, primitiveTypeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(ctx, arrayTypeDescription, json);
    _AsJson(ctx, arrayTypeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::Field::StructTypeDescription const& structTypeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(ctx, structTypeDescription, json);
    _AsJson(ctx, structTypeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(ctx, nestedContentTypeDescription, json);
    _AsJson(ctx, nestedContentTypeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, NavNodeKey const& navNodeKey, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(ctx, navNodeKey, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ECInstancesNodeKey const& ecInstanceNodeKey, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(ctx, ecInstanceNodeKey, json);
    _AsJson(ctx, ecInstanceNodeKey, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ECClassGroupingNodeKey const& groupingNodeKey,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(ctx, groupingNodeKey, json);
    _AsJson(ctx, groupingNodeKey, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, ECPropertyGroupingNodeKey const& propertyGroupingNodeKey,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(ctx, propertyGroupingNodeKey, json);
    _AsJson(ctx, propertyGroupingNodeKey, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContextR ctx, LabelGroupingNodeKey const& labelGroupingNodeKey,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(ctx, labelGroupingNodeKey, json);
    _AsJson(ctx, labelGroupingNodeKey, json);
    return json;
    }
