/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/IECPresentationSerializer.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_EC

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2018
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationSerializer : IECPresentationSerializer
{
protected:
    // Not supported:
    rapidjson::Document _AsJson(ConnectionEvent const&, rapidjson::Document::AllocatorType* allocator) const override;
    
    // Common:
    rapidjson::Document _AsJson(KeySet const& keySet, rapidjson::Document::AllocatorType* allocator) const override;
    KeySetPtr _GetKeySetFromJson(IConnectionCR connection, JsonValueCR json) const override;
    rapidjson::Document _AsJson(ECClassCR ecClass, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ECClassInstanceKeyCR key, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Value _AsJson(ECEnumerationCR enumeration, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Value _AsJson(KindOfQuantityCR koq, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Value _AsJson(RelatedClassCR relatedClass, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Value _AsJson(RelatedClassPathCR path, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Document _AsJson(LabelDefinition const& labelDefinition, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(LabelDefinition::SimpleRawValue const& value, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(LabelDefinition::CompositeRawValue const& value, rapidjson::Document::AllocatorType* allocator) const override;

    // Content:
    rapidjson::Value _AsJson(SelectionInfo const& selectionInfo, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Document _AsJson(Content const& content, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContentSetItem const& contentSetItem, int flags, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType* allocator) const override;
    void _FieldAsJson(ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const override;
    void _AsJson(ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const override;
    void _NestedContentFieldAsJson(ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const override;
    void _AsJson(ContentDescriptor::CompositeContentField const& compositeContentField, RapidJsonDocumentR nestedContentBaseJson) const override;
    void _AsJson(ContentDescriptor::RelatedContentField const& relatedContentField, RapidJsonDocumentR nestedContentBaseJson) const override;
    void _AsJson(ContentDescriptor::DisplayLabelField const&, RapidJsonDocumentR) const override {}
    void _AsJson(ContentDescriptor::CalculatedPropertyField const&, RapidJsonDocumentR) const override {}
    void _AsJson(ContentDescriptor::SystemField const&, RapidJsonDocumentR) const override {}
    void _AsJson(ContentDescriptor::ECInstanceKeyField const&, RapidJsonDocumentR) const override {}
    void _AsJson(ContentDescriptor::ECNavigationInstanceIdField const&, RapidJsonDocumentR) const override {}
    rapidjson::Document _AsJson(ContentFieldEditor const& editor, rapidjson::Document::AllocatorType* allocator) const override;
    void _ParamsAsJson(ContentFieldEditor::Params const&, RapidJsonDocumentR) const override {}
    void _AsJson(FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const override;
    void _AsJson(FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const override;
    void _AsJson(FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const override;
    void _AsJson(FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const override;
    void _TypeDescriptionAsJson(ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    void _AsJson(ContentDescriptor::Field::PrimitiveTypeDescription const&, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    void _AsJson(ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    void _AsJson(ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    void _AsJson(ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;

    // Hierarchies:
    void _NavNodeKeyAsJson(NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override;
    NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override;
    NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(JsonValueCR json) const override;
    NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(RapidJsonValueCR json) const override;
    void _AsJson(ECInstancesNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override;
    ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override;
    void _AsJson(ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override;
    ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override;
    void _AsJson(ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override;
    ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override;
    void _AsJson(LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(JsonValueCR json) const override;
    LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR json) const override;
    rapidjson::Document _AsJson(NavNode const& navNode, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType* allocator) const override;

    // Update:
    rapidjson::Document _AsJson(HierarchyUpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ECInstanceChangeResult const&, rapidjson::Document::AllocatorType* allocator) const override;

public:
    static ECClassCP GetClassFromFullName(IConnectionCR connection, Utf8CP fullClassName);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
