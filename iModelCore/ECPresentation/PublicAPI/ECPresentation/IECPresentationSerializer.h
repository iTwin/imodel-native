/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/ContentFieldEditors.h>
#include <ECPresentation/Update.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Mantas.Kontrimas                03/2018
+===============+===============+===============+===============+===============+======*/
struct IECPresentationSerializer
{
protected:
    virtual rapidjson::Document _AsJson(ConnectionEvent const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContentFieldEditor const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContentDescriptor::Category const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContentDescriptor::Property const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual void _FieldAsJson(ContentDescriptor::Field const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::DisplayLabelField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::ECPropertiesField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::CalculatedPropertyField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::NestedContentField const&, RapidJsonDocumentR) const = 0;
    void _SystemFieldAsJson(ContentDescriptor::SystemField const&, RapidJsonDocumentR) const;
    virtual void _AsJson(ContentDescriptor::SystemField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::ECInstanceKeyField const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::ECNavigationInstanceIdField const&, RapidJsonDocumentR) const = 0;

    virtual rapidjson::Document _AsJson(UpdateRecord const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual void _ParamsAsJson(ContentFieldEditor::Params const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(FieldEditorJsonParams const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(FieldEditorMultilineParams const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(FieldEditorRangeParams const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(FieldEditorSliderParams const&, RapidJsonDocumentR) const = 0;

    virtual void _TypeDescriptionAsJson(ContentDescriptor::Field::TypeDescription const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::Field::PrimitiveTypeDescription const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::Field::ArrayTypeDescription const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::Field::StructTypeDescription const&, RapidJsonDocumentR) const = 0;
    virtual void _AsJson(ContentDescriptor::Field::NestedContentTypeDescription const&, RapidJsonDocumentR) const = 0;

    virtual rapidjson::Document _AsJson(ECClassCR, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContentDescriptor const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ContentSetItem const&, int, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ECClassInstanceKeyCR, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(Content const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(ECInstanceChangeResult const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual void _NavNodeKeyAsJson(NavNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR, JsonValueCR) const = 0;
    virtual NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR, RapidJsonValueCR) const = 0;
    virtual NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(JsonValueCR) const = 0;
    virtual NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(RapidJsonValueCR) const = 0;
    virtual void _AsJson(ECInstancesNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR, JsonValueCR) const = 0;
    virtual ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR, RapidJsonValueCR) const = 0;
    virtual void _AsJson(ECClassGroupingNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR, JsonValueCR) const = 0;
    virtual ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR, RapidJsonValueCR) const = 0;
    virtual void _AsJson(ECPropertyGroupingNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR, JsonValueCR) const = 0;
    virtual ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR, RapidJsonValueCR) const = 0;
    virtual void _AsJson(LabelGroupingNodeKey const&, RapidJsonDocumentR) const = 0;
    virtual LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(JsonValueCR) const = 0;
    virtual LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR) const = 0;

    virtual rapidjson::Document _AsJson(NavNode const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _AsJson(NodesPathElement const&, rapidjson::Document::AllocatorType*) const = 0;

    virtual rapidjson::Document _AsJson(KeySet const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual KeySetPtr _GetKeySetFromJson(IConnectionCR, JsonValueCR) const = 0;

    virtual rapidjson::Value _AsJson(ECEnumerationCR, rapidjson::Document::AllocatorType&) const = 0;

    virtual rapidjson::Value _AsJson(KindOfQuantityCR, rapidjson::Document::AllocatorType&) const = 0;

    virtual rapidjson::Value _AsJson(RelatedClassCR, rapidjson::Document::AllocatorType&) const = 0;
    virtual rapidjson::Value _AsJson(RelatedClassPathCR, rapidjson::Document::AllocatorType&) const = 0;

    virtual rapidjson::Value _AsJson(SelectionInfo const&, rapidjson::Document::AllocatorType&) const = 0;

public:
    virtual ~IECPresentationSerializer() {}

    rapidjson::Document AsJson(UpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(updateRecord, allocator);}

    rapidjson::Document AsJson(ConnectionEvent const& connectionEvent, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(connectionEvent, allocator);}

    rapidjson::Document AsJson(NavNode const& navNode, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(navNode, allocator);}

    rapidjson::Document AsJson(NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(navNodesPathElement, allocator);}

    rapidjson::Document AsJson(ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(contentDescriptor, allocator);}

    rapidjson::Document AsJson(Content const& content, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(content, allocator);}

    rapidjson::Document AsJson(ECInstanceChangeResult const& ecInstanceChangeResult, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ecInstanceChangeResult, allocator);}

    rapidjson::Document AsJson(ContentFieldEditor const& contentFieldEditor, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(contentFieldEditor, allocator);}

    rapidjson::Document AsJson(ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(category, allocator);}

    rapidjson::Document AsJson(ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(property, allocator);}

    rapidjson::Document AsJson(ContentDescriptor::DisplayLabelField const& displayLabelField, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::ECPropertiesField const& ecPropertiesField, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::CalculatedPropertyField const& calculatedPropertyField, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::NestedContentField const& nestedContentField, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::ECInstanceKeyField const& ecInstanceKeyField, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::ECNavigationInstanceIdField const& ecNavigationInstanceIdField, rapidjson::Document::AllocatorType* allocator = nullptr) const;

    rapidjson::Document AsJson(FieldEditorJsonParams const& jsonParams, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(FieldEditorMultilineParams const& multilineParams, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(FieldEditorRangeParams const& rangeParams, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(FieldEditorSliderParams const& sliderParams, rapidjson::Document::AllocatorType* allocator = nullptr) const;

    rapidjson::Document AsJson(ContentDescriptor::Field::TypeDescription const& typeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::Field::PrimitiveTypeDescription const& primitiveTypeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    rapidjson::Document AsJson(ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, rapidjson::Document::AllocatorType* allocator = nullptr) const;

    rapidjson::Document AsJson(ContentSetItem const& contentSetItem, int flags = ContentSetItem::SERIALIZE_All, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(contentSetItem, flags, allocator);}

    rapidjson::Document AsJson(NavNodeKey const& navNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    NavNodeKeyPtr GetNavNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const {return _GetNavNodeKeyFromJson(connection, json);}
    NavNodeKeyPtr GetNavNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const {return _GetNavNodeKeyFromJson(connection, json);}
    NavNodeKeyPtr GetBaseNavNodeKeyFromJson(JsonValueCR json) const {return _GetBaseNavNodeKeyFromJson(json);}
    NavNodeKeyPtr GetBaseNavNodeKeyFromJson(RapidJsonValueCR json) const {return _GetBaseNavNodeKeyFromJson(json);}
    rapidjson::Document AsJson(ECInstancesNodeKey const& ecInstanceNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECInstancesNodeKeyPtr GetECInstanceNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const {return _GetECInstanceNodeKeyFromJson(connection, json);}
    ECInstancesNodeKeyPtr GetECInstanceNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const {return _GetECInstanceNodeKeyFromJson(connection, json);}
    rapidjson::Document AsJson(ECClassGroupingNodeKey const& groupingNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECClassGroupingNodeKeyPtr GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const {return _GetECClassGroupingNodeKeyFromJson(connection, json);}
    ECClassGroupingNodeKeyPtr GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const {return _GetECClassGroupingNodeKeyFromJson(connection, json);}
    rapidjson::Document AsJson(ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPropertyGroupingNodeKeyPtr GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const {return _GetECPropertyGroupingNodeKeyFromJson(connection, json);}
    ECPropertyGroupingNodeKeyPtr GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const {return _GetECPropertyGroupingNodeKeyFromJson(connection, json);}
    rapidjson::Document AsJson(LabelGroupingNodeKey const& labelGroupingNodeKey, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    LabelGroupingNodeKeyPtr GetLabelGroupingNodeKeyFromJson(JsonValueCR json) const {return _GetLabelGroupingNodeKeyFromJson(json);}
    LabelGroupingNodeKeyPtr GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR json) const {return _GetLabelGroupingNodeKeyFromJson(json);}

    rapidjson::Document AsJson(KeySet const& keySet, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(keySet, allocator);}
    KeySetPtr GetKeySetFromJson(IConnectionCR connection, JsonValueCR json) const {return _GetKeySetFromJson(connection, json);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
