/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/IECPresentationSerializer.h>
#include "ECPresentationUtils.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_EC

#define PUSH_JSON_IF_VALID(jsonArray, jsonAllocator, objPtr) \
    if (objPtr != nullptr) \
        jsonArray.PushBack(objPtr->AsJson(&jsonAllocator), jsonAllocator); \
    else \
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, NativeLogging::LOG_ERROR, "Attempted to serialize NULL object");

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationSerializer : IECPresentationSerializer
{
private:
    static ECClassCP GetClassFromFullName(IConnectionCR connection, JsonValueCR);

protected:
    // Not supported:
    rapidjson::Document _AsJson(ContextR, ConnectionEvent const&, rapidjson::Document::AllocatorType* allocator) const override;

    // Common:
    rapidjson::Document _AsJson(ContextR, KeySet const& keySet, rapidjson::Document::AllocatorType* allocator) const override;
    KeySetPtr _GetKeySetFromJson(IConnectionCR connection, JsonValueCR json) const override;
	rapidjson::Document _AsJson(ContextR, ECClassCR ecClass, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, BeInt64Id const&, rapidjson::Document::AllocatorType*) const override;
    rapidjson::Document _AsJson(ContextR, ECClassInstanceKeyCR key, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Value _AsJson(ContextR, ECEnumerationCR enumeration, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Value _AsJson(ContextR, KindOfQuantityCR koq, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Value _AsJson(ContextR, RelatedClassCR relatedClass, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Value _AsJson(ContextR, RelatedClassPathCR path, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Document _AsJson(ContextR, LabelDefinition const& labelDefinition, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, LabelDefinition::SimpleRawValue const& value, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, LabelDefinition::CompositeRawValue const& value, rapidjson::Document::AllocatorType* allocator) const override;

    // Content:
    rapidjson::Value _AsJson(ContextR, SelectionInfo const&, rapidjson::Document::AllocatorType& allocator) const override;
    rapidjson::Document _AsJson(ContextR, SelectClassInfo const&, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, Content const& content, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, ContentSetItem const& contentSetItem, int flags, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, DisplayValueGroupCR, rapidjson::Document::AllocatorType*) const override;
    rapidjson::Document _AsJson(ContextR, ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType* allocator) const override;
    void _FieldAsJson(ContextR, ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const override;
    void _AsJson(ContextR, ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const override;
    void _NestedContentFieldAsJson(ContextR, ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const override;
    void _AsJson(ContextR, ContentDescriptor::CompositeContentField const& compositeContentField, RapidJsonDocumentR nestedContentBaseJson) const override;
    void _AsJson(ContextR, ContentDescriptor::RelatedContentField const& relatedContentField, RapidJsonDocumentR nestedContentBaseJson) const override;
    void _AsJson(ContextR, ContentDescriptor::DisplayLabelField const&, RapidJsonDocumentR) const override {}
    void _AsJson(ContextR, ContentDescriptor::CalculatedPropertyField const&, RapidJsonDocumentR) const override {}
    rapidjson::Document _AsJson(ContextR, ContentFieldRenderer const& renderer, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, ContentFieldEditor const& editor, rapidjson::Document::AllocatorType* allocator) const override;
    void _ParamsAsJson(ContextR, ContentFieldEditor::Params const&, RapidJsonDocumentR) const override {}
    void _AsJson(ContextR, FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const override;
    void _AsJson(ContextR, FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const override;
    void _AsJson(ContextR, FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const override;
    void _AsJson(ContextR, FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const override;
    void _TypeDescriptionAsJson(ContextR, ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    void _AsJson(ContextR, ContentDescriptor::Field::PrimitiveTypeDescription const&, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    void _AsJson(ContextR, ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    void _AsJson(ContextR, ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    void _AsJson(ContextR, ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;

    // Hierarchies:
    void _NavNodeKeyAsJson(ContextR, NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override;
    NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override;
    NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(JsonValueCR json) const override;
    NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(RapidJsonValueCR json) const override;
    void _AsJson(ContextR, ECInstancesNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override;
    ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override;
    void _AsJson(ContextR, ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override;
    ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override;
    void _AsJson(ContextR, ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override;
    ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override;
    void _AsJson(ContextR, LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(JsonValueCR json) const override;
    LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR json) const override;
    rapidjson::Document _AsJson(ContextR, NavNode const& navNode, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType* allocator) const override;

    // Update:
    rapidjson::Document _AsJson(ContextR, HierarchyChangeRecord const& changeRecord, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, HierarchyUpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, HierarchyUpdateRecord::ExpandedNode const& expandedNode, rapidjson::Document::AllocatorType* allocator) const override;
    rapidjson::Document _AsJson(ContextR, ECInstanceChangeResult const&, rapidjson::Document::AllocatorType* allocator) const override;

public:
    using IECPresentationSerializer::AsJson;

    static ECClassCP GetClassFromFullName(IConnectionCR connection, RapidJsonValueCR);
    static bvector<NavNodeKeyCPtr> GetNavNodeKeysFromSerializedJson(IConnectionCR, Utf8CP json);

    static KeySetPtr GetKeySetFromJson(IConnectionCR, RapidJsonValueCR);
    static RulesetVariables GetRulesetVariablesFromJson(RapidJsonValueCR);
};

/*=================================================================================**//**
* Serializes a class as a hex ID ant puts it into a set. The set may be serialized as a
* map id => class information to avoid repeating class information.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CompressedClassSerializer : IECClassSerializer
{
private:
	bset<ECClassCP> m_ecClasses;
protected:
    virtual rapidjson::Document _SerializeECClass(ECClassCR, rapidjson::Document::AllocatorType&) override;
public:
    bset<ECClassCP> const& GetClasses() const {return m_ecClasses;};
    rapidjson::Document CreateAccumulatedClassesMap(rapidjson::Document::AllocatorType*) const;
};

/*=================================================================================**//**
* Serializes class information as an object with hex ID, name and label.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DefaultClassSerializer : IECClassSerializer
{
protected:
	virtual rapidjson::Document _SerializeECClass(ECClassCR, rapidjson::Document::AllocatorType&) override;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
