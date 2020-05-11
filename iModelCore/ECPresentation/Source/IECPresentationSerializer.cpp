/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/IECPresentationSerializer.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IECPresentationSerializer::_SystemFieldAsJson(ContentDescriptor::SystemField const& systemField, RapidJsonDocumentR fieldBaseJson) const
    {
    _FieldAsJson(systemField, fieldBaseJson);
    _AsJson(systemField, fieldBaseJson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::DisplayLabelField const& displayLabelField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(displayLabelField, json);
    _AsJson(displayLabelField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::ECPropertiesField const& ecPropertiesField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(ecPropertiesField, json);
    _AsJson(ecPropertiesField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::CalculatedPropertyField const& calculatedPropertyField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(calculatedPropertyField, json);
    _AsJson(calculatedPropertyField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::CompositeContentField const& compositeContentField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(compositeContentField, json);
    _NestedContentFieldAsJson(compositeContentField, json);
    _AsJson(compositeContentField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::RelatedContentField const& relatedContentField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _FieldAsJson(relatedContentField, json);
    _NestedContentFieldAsJson(relatedContentField, json);
    _AsJson(relatedContentField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::ECInstanceKeyField const& ecInstanceKeyField,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _SystemFieldAsJson(ecInstanceKeyField, json);
    _AsJson(ecInstanceKeyField, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(FieldEditorJsonParams const& jsonParams,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _ParamsAsJson(jsonParams, json);
    _AsJson(jsonParams, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(FieldEditorMultilineParams const& multilineParams,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _ParamsAsJson(multilineParams, json);
    _AsJson(multilineParams, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(FieldEditorRangeParams const& rangeParams,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _ParamsAsJson(rangeParams, json);
    _AsJson(rangeParams, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(FieldEditorSliderParams const& sliderParams,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _ParamsAsJson(sliderParams, json);
    _AsJson(sliderParams, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::Field::TypeDescription const& typeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(typeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::Field::PrimitiveTypeDescription const& primitiveTypeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(primitiveTypeDescription, json);
    _AsJson(primitiveTypeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(arrayTypeDescription, json);
    _AsJson(arrayTypeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::Field::StructTypeDescription const& structTypeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(structTypeDescription, json);
    _AsJson(structTypeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _TypeDescriptionAsJson(nestedContentTypeDescription, json);
    _AsJson(nestedContentTypeDescription, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(NavNodeKey const& navNodeKey, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(navNodeKey, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ECInstancesNodeKey const& ecInstanceNodeKey, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(ecInstanceNodeKey, json);
    _AsJson(ecInstanceNodeKey, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ECClassGroupingNodeKey const& groupingNodeKey,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(groupingNodeKey, json);
    _AsJson(groupingNodeKey, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(ECPropertyGroupingNodeKey const& propertyGroupingNodeKey,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(propertyGroupingNodeKey, json);
    _AsJson(propertyGroupingNodeKey, json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IECPresentationSerializer::AsJson(LabelGroupingNodeKey const& labelGroupingNodeKey,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    _NavNodeKeyAsJson(labelGroupingNodeKey, json);
    _AsJson(labelGroupingNodeKey, json);
    return json;
    }
