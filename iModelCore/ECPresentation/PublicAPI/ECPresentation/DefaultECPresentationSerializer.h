/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/IECPresentationSerializer.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE DefaultECPresentationSerializer : IECPresentationSerializer
{
protected:
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ConnectionEvent const& connectionEvent, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ContentFieldRenderer const& contentFieldRenderer, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ContentFieldEditor const& contentFieldEditor, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual void _FieldAsJson(ContextR, ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const override;
    virtual void _AsJson(ContextR, ContentDescriptor::DisplayLabelField const&, RapidJsonDocumentR) const override {}
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const override;
    virtual void _AsJson(ContextR, ContentDescriptor::CalculatedPropertyField const&, RapidJsonDocumentR) const override {}
    ECPRESENTATION_EXPORT virtual void _NestedContentFieldAsJson(ContextR, ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ContentDescriptor::CompositeContentField const& compositeContentField, RapidJsonDocumentR nestedContentFieldJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ContentDescriptor::RelatedContentField const& relatedContentField, RapidJsonDocumentR nestedContentFieldJson) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, HierarchyChangeRecord const&, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, HierarchyUpdateRecord const&, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, HierarchyUpdateRecord::ExpandedNode const&, rapidjson::Document::AllocatorType*) const override;

    virtual void _ParamsAsJson(ContextR, ContentFieldEditor::Params const&, RapidJsonDocumentR) const override {}
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const override;

    ECPRESENTATION_EXPORT virtual void _TypeDescriptionAsJson(ContextR, ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ContentDescriptor::Field::PrimitiveTypeDescription const& primitiveTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ECClassCR ecClass, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, SelectClassInfo const&, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ContentSetItem const& contentSetItem, int flags, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, DisplayValueGroupCR, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, BeInt64Id const&, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ECClassInstanceKeyCR key, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, Content const& content, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, ECInstanceChangeResult const& ecInstanceChangeResult, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual void _NavNodeKeyAsJson(ContextR, NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR, BeJsConst) const override;
    ECPRESENTATION_EXPORT virtual NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(BeJsConst) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ECInstancesNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR, BeJsConst) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR, BeJsConst) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR, BeJsConst) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContextR, LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(BeJsConst) const override;
    
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, PresentationQuery const&, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual std::unique_ptr<PresentationQuery> _GetPresentationQueryFromJson(BeJsConst) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, BoundQueryValuesList const&, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, NavNode const& navNode, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, LabelDefinition const& labelDefinition, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, LabelDefinition::SimpleRawValue const& value, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, LabelDefinition::CompositeRawValue const& value, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContextR, KeySet const&, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual KeySetPtr _GetKeySetFromJson(IConnectionCR, BeJsConst) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(ContextR, ECEnumerationCR enumeration, rapidjson::Document::AllocatorType&) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(ContextR, KindOfQuantityCR koq, rapidjson::Document::AllocatorType&) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(ContextR, RelatedClassCR path, rapidjson::Document::AllocatorType&) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(ContextR, RelatedClassPathCR path, rapidjson::Document::AllocatorType&) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(ContextR, SelectionInfo const& selectionInfo, rapidjson::Document::AllocatorType&) const override;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
