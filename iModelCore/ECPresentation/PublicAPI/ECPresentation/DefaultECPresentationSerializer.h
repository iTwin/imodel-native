/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/DefaultECPresentationSerializer.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/IECPresentationSerializer.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Mantas.Kontrimas                03/2018
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE DefaultECPresentationSerializer : IECPresentationSerializer
{
protected:
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ConnectionEvent const& connectionEvent, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContentFieldEditor const& contentFieldEditor, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual void _FieldAsJson(ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const override;
    virtual void _AsJson(ContentDescriptor::DisplayLabelField const&, RapidJsonDocumentR) const override {}
    ECPRESENTATION_EXPORT virtual void _AsJson(ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const override;
    virtual void _AsJson(ContentDescriptor::CalculatedPropertyField const&, RapidJsonDocumentR) const override {}
    ECPRESENTATION_EXPORT virtual void _AsJson(ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const override;
    virtual void _AsJson(ContentDescriptor::SystemField const&, RapidJsonDocumentR) const override {}
    virtual void _AsJson(ContentDescriptor::ECInstanceKeyField const&, RapidJsonDocumentR) const override {}
    virtual void _AsJson(ContentDescriptor::ECNavigationInstanceIdField const&, RapidJsonDocumentR) const override {}

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(UpdateRecord const& updateRecord, rapidjson::Document::AllocatorType*) const override;

    virtual void _ParamsAsJson(ContentFieldEditor::Params const&, RapidJsonDocumentR) const override {}
    ECPRESENTATION_EXPORT virtual void _AsJson(FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const override;

    ECPRESENTATION_EXPORT virtual void _TypeDescriptionAsJson(ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContentDescriptor::Field::PrimitiveTypeDescription const& primitiveTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECClassCR ecClass, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ContentSetItem const& contentSetItem, int flags, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECClassInstanceKeyCR key, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(Content const& content, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECInstanceChangeResult const& ecInstanceChangeResult, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual void _NavNodeKeyAsJson(NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR, JsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR, RapidJsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(JsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(RapidJsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ECInstanceNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual ECInstanceNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR, JsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual ECInstanceNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR, RapidJsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR, JsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR, RapidJsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR, JsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR, RapidJsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual void _AsJson(LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override;
    ECPRESENTATION_EXPORT virtual LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(JsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR json) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(NavNode const& navNode, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType*) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(KeySet const&, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual KeySetPtr _GetKeySetFromJson(IConnectionCR, JsonValueCR json) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(SelectionChangedEvent const& selectionChangedEvent, rapidjson::Document::AllocatorType*) const override;
    ECPRESENTATION_EXPORT virtual SelectionChangedEventPtr _GetSelectionChangedEventFromJson(IConnectionCacheCR connectionCache, JsonValueCR json) const override;
    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(ECEnumerationCR enumeration, rapidjson::Document::AllocatorType&) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(KindOfQuantityCR koq, rapidjson::Document::AllocatorType&) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(RelatedClassPathCR path, rapidjson::Document::AllocatorType&) const override;

    ECPRESENTATION_EXPORT virtual rapidjson::Value _AsJson(SelectionInfo const& selectionInfo, rapidjson::Document::AllocatorType&) const override;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
