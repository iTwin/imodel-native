/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/ContentFieldEditors.h>
#include <ECPresentation/Update.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECPresentationSerializer
{
    typedef ECPresentationSerializerContextR ContextR;

protected:
    virtual rapidjson::Document _AsJson(ContextR, ConnectionEvent const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, ContentFieldRenderer const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, ContentFieldEditor const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, ContentDescriptor::Category const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, ContentDescriptor::Property const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual void _FieldAsJson(ContextR, ContentDescriptor::Field const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::DisplayLabelField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::ECPropertiesField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::CalculatedPropertyField const&, RapidJsonDocumentR) const = 0;
    virtual void _NestedContentFieldAsJson(ContextR, ContentDescriptor::NestedContentField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::CompositeContentField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::RelatedContentField const&, RapidJsonDocumentR) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, HierarchyChangeRecord const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, HierarchyUpdateRecord const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, HierarchyUpdateRecord::ExpandedNode const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual void _ParamsAsJson(ContextR, ContentFieldEditor::Params const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, FieldEditorJsonParams const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, FieldEditorMultilineParams const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, FieldEditorRangeParams const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, FieldEditorSliderParams const&, RapidJsonDocumentR) const = 0;

    virtual void _TypeDescriptionAsJson(ContextR, ContentDescriptor::Field::TypeDescription const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::Field::PrimitiveTypeDescription const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::Field::ArrayTypeDescription const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::Field::StructTypeDescription const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContextR, ContentDescriptor::Field::NestedContentTypeDescription const&, RapidJsonDocumentR) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, ECClassCR, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, SelectClassInfo const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, ContentDescriptor const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, ContentSetItem const&, int, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, DisplayValueGroupCR, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, BeInt64Id const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, ECClassInstanceKeyCR, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, Content const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, ECInstanceChangeResult const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual void _NavNodeKeyAsJson(ContextR, NavNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR, BeJsConst) const = 0;
    virtual NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(BeJsConst) const = 0;
    virtual void _AsJson(ContextR, ECInstancesNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR, BeJsConst) const = 0;
    virtual void _AsJson(ContextR, ECClassGroupingNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR, BeJsConst) const = 0;
    virtual void _AsJson(ContextR, ECPropertyGroupingNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR, BeJsConst) const = 0;
    virtual void _AsJson(ContextR, LabelGroupingNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(BeJsConst) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, NavNode const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, NodesPathElement const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, LabelDefinition const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, LabelDefinition::SimpleRawValue const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(ContextR, LabelDefinition::CompositeRawValue const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContextR, KeySet const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual KeySetPtr _GetKeySetFromJson(IConnectionCR, BeJsConst) const = 0;

    virtual rapidjson::Value _AsJson(ContextR, ECEnumerationCR, rapidjson::Document::AllocatorType&) const = 0;

    virtual rapidjson::Value _AsJson(ContextR, KindOfQuantityCR, rapidjson::Document::AllocatorType&) const = 0;

    virtual rapidjson::Value _AsJson(ContextR, RelatedClassCR, rapidjson::Document::AllocatorType&) const = 0;
    virtual rapidjson::Value _AsJson(ContextR, RelatedClassPathCR, rapidjson::Document::AllocatorType&) const = 0;

    virtual rapidjson::Value _AsJson(ContextR, SelectionInfo const&, rapidjson::Document::AllocatorType&) const = 0;

public:
    virtual ~IECPresentationSerializer() {}

    rapidjson::Document AsJson(ContextR ctx, HierarchyChangeRecord const& changeRecord, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, changeRecord, allocator);}

    rapidjson::Document AsJson(ContextR ctx, HierarchyUpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, updateRecord, allocator);}

    rapidjson::Document AsJson(ContextR ctx, HierarchyUpdateRecord::ExpandedNode const& expandedNode, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, expandedNode, allocator);}

    rapidjson::Document AsJson(ContextR ctx, ConnectionEvent const& connectionEvent, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, connectionEvent, allocator);}

    rapidjson::Document AsJson(ContextR ctx, NavNode const& navNode, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, navNode, allocator);}

    rapidjson::Document AsJson(ContextR ctx, NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, navNodesPathElement, allocator);}

    rapidjson::Document AsJson(ContextR ctx, SelectClassInfo const& classInfo, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, classInfo, allocator);}

    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, contentDescriptor, allocator);}

    rapidjson::Document AsJson(ContextR ctx, Content const& content, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, content, allocator);}

    rapidjson::Document AsJson(ContextR ctx, ECInstanceChangeResult const& ecInstanceChangeResult, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, ecInstanceChangeResult, allocator);}

    rapidjson::Document AsJson(ContextR ctx, ContentFieldRenderer const& contentFieldRenderer, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, contentFieldRenderer, allocator);}

    rapidjson::Document AsJson(ContextR ctx, ContentFieldEditor const& contentFieldEditor, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, contentFieldEditor, allocator);}

    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, category, allocator);}

    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, property, allocator);}

    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::DisplayLabelField const& displayLabelField, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::ECPropertiesField const& ecPropertiesField, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::CalculatedPropertyField const& calculatedPropertyField, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::CompositeContentField const&, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::RelatedContentField const&, rapidjson::Document::AllocatorType* allocator = nullptr) const;

    rapidjson::Document AsJson(ContextR ctx, FieldEditorJsonParams const& jsonParams, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, FieldEditorMultilineParams const& multilineParams, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, FieldEditorRangeParams const& rangeParams, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, FieldEditorSliderParams const& sliderParams, rapidjson::Document::AllocatorType* allocator = nullptr) const;

    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::Field::TypeDescription const& typeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::Field::PrimitiveTypeDescription const& primitiveTypeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContextR ctx, ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;

    rapidjson::Document AsJson(ContextR ctx, ContentSetItem const& contentSetItem, int flags = ContentSetItem::SERIALIZE_All, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, contentSetItem, flags, allocator);}
    rapidjson::Document AsJson(ContextR ctx, DisplayValueGroupCR value, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, value, allocator);}

    rapidjson::Document AsJson(ContextR ctx, NavNodeKey const& navNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    NavNodeKeyPtr GetNavNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const {return _GetNavNodeKeyFromJson(connection, json);}
    NavNodeKeyPtr GetBaseNavNodeKeyFromJson(BeJsConst json) const {return _GetBaseNavNodeKeyFromJson(json);}
    rapidjson::Document AsJson(ContextR ctx, ECInstancesNodeKey const& ecInstanceNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECInstancesNodeKeyPtr GetECInstanceNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const {return _GetECInstanceNodeKeyFromJson(connection, json);}
    rapidjson::Document AsJson(ContextR ctx, ECClassGroupingNodeKey const& groupingNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECClassGroupingNodeKeyPtr GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const {return _GetECClassGroupingNodeKeyFromJson(connection, json);}
    rapidjson::Document AsJson(ContextR ctx, ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPropertyGroupingNodeKeyPtr GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const {return _GetECPropertyGroupingNodeKeyFromJson(connection, json);}
    rapidjson::Document AsJson(ContextR ctx, LabelGroupingNodeKey const& labelGroupingNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    LabelGroupingNodeKeyPtr GetLabelGroupingNodeKeyFromJson(BeJsConst json) const {return _GetLabelGroupingNodeKeyFromJson(json);}

    rapidjson::Document AsJson(ContextR ctx, LabelDefinition const& labelDefinition, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, labelDefinition, allocator);}
    rapidjson::Document AsJson(ContextR ctx, LabelDefinition::SimpleRawValue const& value, rapidjson::Document::AllocatorType* allocator = nullptr) const { return _AsJson(ctx, value, allocator); }
    rapidjson::Document AsJson(ContextR ctx, LabelDefinition::CompositeRawValue const& value, rapidjson::Document::AllocatorType* allocator = nullptr) const { return _AsJson(ctx, value, allocator); }

    rapidjson::Document AsJson(ContextR ctx, BeInt64Id const& id, rapidjson::Document::AllocatorType* allocator = nullptr) const { return _AsJson(ctx, id, allocator); }
    rapidjson::Document AsJson(ContextR ctx, ECClassInstanceKeyCR key, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, key, allocator);}

    rapidjson::Document AsJson(ContextR ctx, KeySet const& keySet, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, keySet, allocator);}
    KeySetPtr GetKeySetFromJson(IConnectionCR connection, BeJsConst json) const {return _GetKeySetFromJson(connection, json);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
