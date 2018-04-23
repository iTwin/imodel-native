/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/DefaultECPresentationSerializer.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>
#include "ValueHelpers.h"
#include "RulesDriven/RulesEngine/JsonNavNode.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ConnectionEvent const& connectionEvent,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("EventType", (int)connectionEvent.GetEventType(), json.GetAllocator());

    rapidjson::Value connectionJson(rapidjson::kObjectType);
    connectionJson.AddMember("ConnectionId", rapidjson::Value(connectionEvent.GetConnection().GetId().c_str(), json.GetAllocator()), json.GetAllocator());
    connectionJson.AddMember("ConnectionGuid", rapidjson::Value(connectionEvent.GetConnection().GetECDb().GetDbGuid().ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    if (connectionEvent.GetEventType() == ConnectionEventType::Opened)
        connectionJson.AddMember("IsProjectPrimary", connectionEvent.IsPrimaryConnection(), json.GetAllocator());
    else
        connectionJson.AddMember("IsProjectPrimary", false, json.GetAllocator());

    json.AddMember("Connection", connectionJson, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContentFieldEditor const& contentFieldEditor,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Name", rapidjson::Value(contentFieldEditor.GetName().c_str(), json.GetAllocator()), json.GetAllocator());

    rapidjson::Value paramsJson(rapidjson::kObjectType);
    for (ContentFieldEditor::Params const* params : contentFieldEditor.GetParams())
        {
        if (paramsJson.HasMember(params->GetName()))
            {
            BeAssert(false);
            continue;
            }
        paramsJson.AddMember(rapidjson::Value(params->GetName(), json.GetAllocator()), params->AsJson(&json.GetAllocator()), json.GetAllocator());
        }

    json.AddMember("Params", paramsJson, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContentDescriptor::Category const& category,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Name", rapidjson::Value(category.GetName().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("DisplayLabel", rapidjson::Value(category.GetLabel().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Description", rapidjson::Value(category.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Expand", category.ShouldExpand(), json.GetAllocator());
    json.AddMember("Priority", category.GetPriority(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContentDescriptor::Property const& property,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    
    rapidjson::Value propertyJson(rapidjson::kObjectType);
    propertyJson.AddMember("BaseClassInfo", _AsJson(property.GetProperty().GetClass(), &json.GetAllocator()), json.GetAllocator());
    propertyJson.AddMember("ActualClassInfo", _AsJson(property.GetPropertyClass(), &json.GetAllocator()), json.GetAllocator());
    propertyJson.AddMember("Name", rapidjson::Value(property.GetProperty().GetName().c_str(), json.GetAllocator()), json.GetAllocator());

    if (property.GetProperty().GetIsPrimitive() && nullptr != property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration())
        {
        ECEnumerationCP propEnum = property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration();
        propertyJson.AddMember("Type", "enum", json.GetAllocator());
        propertyJson.AddMember("IsStrict", rapidjson::Value(propEnum->GetIsStrict()), json.GetAllocator());
        propertyJson.AddMember("Choices", _AsJson(*propEnum, json.GetAllocator()), json.GetAllocator());
        }
    else
        {
        propertyJson.AddMember("Type", rapidjson::Value(property.GetProperty().GetTypeName().c_str(), json.GetAllocator()), json.GetAllocator());
        }

    if (nullptr != property.GetProperty().GetKindOfQuantity())
        {
        KindOfQuantityCP koq = property.GetProperty().GetKindOfQuantity();
        propertyJson.AddMember("KindOfQuantity", _AsJson(*koq, json.GetAllocator()), json.GetAllocator());
        }

    json.AddMember("Property", propertyJson, json.GetAllocator());
    json.AddMember("RelatedClassPath", _AsJson(property.GetRelatedClassPath(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_FieldAsJson(ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const
    {
    fieldBaseJson.SetObject();
    fieldBaseJson.AddMember("Category", field.GetCategory().AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("Name", rapidjson::Value(field.GetName().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("DisplayLabel", rapidjson::Value(field.GetLabel().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("Type", field.GetTypeDescription().AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("IsReadOnly", field.IsReadOnly(), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("Priority", field.GetPriority(), fieldBaseJson.GetAllocator());

    if (nullptr != field.GetEditor())
        fieldBaseJson.AddMember("Editor", field.GetEditor()->AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const
    {
    rapidjson::Value propertiesJson(rapidjson::kArrayType);
    for (ContentDescriptor::Property const& prop : ecPropertiesField.GetProperties())
        propertiesJson.PushBack(prop.AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("Properties", propertiesJson, fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const
    {
    fieldBaseJson.AddMember("ContentClassInfo", _AsJson(nestedContentField.GetContentClass(), &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("PathToPrimary", _AsJson(nestedContentField.GetRelationshipPath(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    
    rapidjson::Value nestedFieldsJson(rapidjson::kArrayType);
    for (ContentDescriptor::Field const* nestedField : nestedContentField.GetFields())
        nestedFieldsJson.PushBack(nestedField->AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("NestedFields", nestedFieldsJson, fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(UpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    switch (updateRecord.GetChangeType())
        {
        case ChangeType::Delete:
            {
            json.AddMember("Type", "Delete", json.GetAllocator());
            json.AddMember("Node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
            break;
            }
        case ChangeType::Insert:
            {
            json.AddMember("Type", "Insert", json.GetAllocator());
            json.AddMember("Node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
            json.AddMember("Position", (uint64_t)updateRecord.GetPosition(), json.GetAllocator());
            break;
            }
        case ChangeType::Update:
            {
            json.AddMember("Type", "Update", json.GetAllocator());
            json.AddMember("Node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
            json.AddMember("Changes", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
            RapidJsonValueR changesJson = json["Changes"];
            for (JsonChange const& change : updateRecord.GetChanges())
                {
                rapidjson::Value changeJson(rapidjson::kObjectType);
                changeJson.AddMember("Name", rapidjson::Value(change.GetName(), json.GetAllocator()), json.GetAllocator());
                changeJson.AddMember("OldValue", rapidjson::Value(change.GetOldValue(), json.GetAllocator()), json.GetAllocator());
                changeJson.AddMember("NewValue", rapidjson::Value(change.GetNewValue(), json.GetAllocator()), json.GetAllocator());
                changesJson.PushBack(changeJson, json.GetAllocator());
                }
            break;
            }
        }

    RapidJsonValueCR extendedData =  updateRecord.GetNode()->GetExtendedData();
    if (extendedData.HasMember("RulesetId"))
        json.AddMember("RulesetId", rapidjson::Value(extendedData["RulesetId"].GetString(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const
    {
    paramsBaseJson.CopyFrom(jsonParams.GetJson(), paramsBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const
    {
    paramsBaseJson.SetObject();
    paramsBaseJson.AddMember("HeightInRows", multilineParams.GetParameters().GetHeightInRows(), paramsBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const
    {
    paramsBaseJson.SetObject();

    if (nullptr != rangeParams.GetParameters().GetMinimumValue())
        paramsBaseJson.AddMember("Minimum", *rangeParams.GetParameters().GetMinimumValue(), paramsBaseJson.GetAllocator());
    else
        paramsBaseJson.AddMember("Minimum", rapidjson::Value(), paramsBaseJson.GetAllocator());
    
    if (nullptr != rangeParams.GetParameters().GetMaximumValue())
        paramsBaseJson.AddMember("Maximum", *rangeParams.GetParameters().GetMaximumValue(), paramsBaseJson.GetAllocator());
    else
        paramsBaseJson.AddMember("Maximum", rapidjson::Value(), paramsBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const
    {
    paramsBaseJson.SetObject();
    paramsBaseJson.AddMember("Minimum", sliderParams.GetParameters().GetMinimumValue(), paramsBaseJson.GetAllocator());
    paramsBaseJson.AddMember("Maximum", sliderParams.GetParameters().GetMaximumValue(), paramsBaseJson.GetAllocator());
    paramsBaseJson.AddMember("IntervalsCount", sliderParams.GetParameters().GetIntervalsCount(), paramsBaseJson.GetAllocator());
    paramsBaseJson.AddMember("ValueFactor", sliderParams.GetParameters().GetValueFactor(), paramsBaseJson.GetAllocator());
    paramsBaseJson.AddMember("IsVertical", sliderParams.GetParameters().IsVertical(), paramsBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_TypeDescriptionAsJson(ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.SetObject();
    typeDescriptionBaseJson.AddMember("TypeName", rapidjson::Value(typeDescription.GetTypeName().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContentDescriptor::Field::PrimitiveTypeDescription const&,
    RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("ValueFormat", "Primitive", typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription,
    RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("ValueFormat", "Array", typeDescriptionBaseJson.GetAllocator());
    typeDescriptionBaseJson.AddMember("MemberType", arrayTypeDescription.GetMemberType()->AsJson(&typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContentDescriptor::Field::StructTypeDescription const& structTypeDescription,
    RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("ValueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
    rapidjson::Value members(rapidjson::kArrayType);
    for (ECPropertyCP prop : structTypeDescription.GetStruct().GetProperties())
        {
        rapidjson::Value member(rapidjson::kObjectType);
        member.AddMember("Name", rapidjson::StringRef(prop->GetName().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("Label", rapidjson::StringRef(prop->GetDisplayLabel().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("Type", ContentDescriptor::ECPropertiesField::TypeDescription::Create(*prop)->AsJson(&typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
        }
    typeDescriptionBaseJson.AddMember("Members", members, typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription,
    RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("ValueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
    rapidjson::Value members(rapidjson::kArrayType);
    for (ContentDescriptor::Field const* nestedField : nestedContentTypeDescription.GetNestedContentField().GetFields())
        {
        rapidjson::Value member(rapidjson::kObjectType);
        member.AddMember("Name", rapidjson::StringRef(nestedField->GetName().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("Label", rapidjson::StringRef(nestedField->GetLabel().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("Type", nestedField->GetTypeDescription().AsJson(&typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
        }
    typeDescriptionBaseJson.AddMember("Members", members, typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ECClassCR ecClass,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Id", rapidjson::Value(ecClass.GetId().ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Name", rapidjson::StringRef(ecClass.GetFullName()), json.GetAllocator());
    json.AddMember("Label", rapidjson::StringRef(ecClass.GetDisplayLabel().c_str()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContentDescriptor const& contentDescriptor,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("PreferredDisplayType", rapidjson::StringRef(contentDescriptor.GetPreferredDisplayType().c_str()), json.GetAllocator());
    json.AddMember("SelectClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (SelectClassInfo const& selectClass : contentDescriptor.GetSelectClasses())
        {
        rapidjson::Value selectClassJson(rapidjson::kObjectType);
        selectClassJson.AddMember("SelectClassInfo", _AsJson(selectClass.GetSelectClass(), &json.GetAllocator()), json.GetAllocator());
        selectClassJson.AddMember("IsPolymorphic", selectClass.IsSelectPolymorphic(), json.GetAllocator());
        selectClassJson.AddMember("PathToPrimaryClass", _AsJson(selectClass.GetPathToPrimaryClass(), json.GetAllocator()), json.GetAllocator());
        selectClassJson.AddMember("RelatedPropertyPaths", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (RelatedClassPathCR propertyPath : selectClass.GetRelatedPropertyPaths())
            selectClassJson["RelatedPropertyPaths"].PushBack(_AsJson(propertyPath, json.GetAllocator()), json.GetAllocator());

        json["SelectClasses"].PushBack(selectClassJson, json.GetAllocator());
        }

    bvector<ContentDescriptor::Field*> visibleFields = contentDescriptor.GetVisibleFields();
    json.AddMember("Fields", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (ContentDescriptor::Field const* field : visibleFields)
        json["Fields"].PushBack(field->AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("SortingFieldIndex", contentDescriptor.GetSortingFieldIndex(), json.GetAllocator());
    json.AddMember("SortDirection", (int)contentDescriptor.GetSortDirection(), json.GetAllocator());
    json.AddMember("ContentFlags", contentDescriptor.GetContentFlags(), json.GetAllocator());
    json.AddMember("ConnectionId", rapidjson::StringRef(contentDescriptor.GetConnection().GetId().c_str()), json.GetAllocator());
    json.AddMember("FilterExpression", rapidjson::StringRef(contentDescriptor.GetFilterExpression().c_str()), json.GetAllocator());
    json.AddMember("InputKeysHash", rapidjson::Value(contentDescriptor.GetInputNodeKeys().GetHash().c_str(), json.GetAllocator()), json.GetAllocator());
    rapidjson::Document options(&json.GetAllocator());
    options.Parse(contentDescriptor.GetOptions().ToString().c_str());
    json.AddMember("ContentOptions", options, json.GetAllocator());
    if (nullptr != contentDescriptor.GetSelectionInfo())
        json.AddMember("SelectionInfo", _AsJson(*contentDescriptor.GetSelectionInfo(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContentSetItem const& contentSetItem, int flags,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayLabel & flags))
        json.AddMember("DisplayLabel", rapidjson::Value(contentSetItem.GetDisplayLabel().c_str(), json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_ImageId & flags))
        json.AddMember("ImageId", rapidjson::Value(contentSetItem.GetImageId().c_str(), json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_Values & flags))
        json.AddMember("Values", rapidjson::Value(contentSetItem.GetValues(), json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayValues & flags))
        json.AddMember("DisplayValues", rapidjson::Value(contentSetItem.GetDisplayValues(), json.GetAllocator()), json.GetAllocator());

    if (contentSetItem.GetClass() != nullptr && 0 != (ContentSetItem::SerializationFlags::SERIALIZE_ClassInfo & flags))
        json.AddMember("ClassInfo", _AsJson(*contentSetItem.GetClass(), &json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_PrimaryKeys & flags))
        {
        rapidjson::Value primaryKeys(rapidjson::kArrayType);
        for (ECClassInstanceKeyCR key : contentSetItem.GetKeys())
            primaryKeys.PushBack(_AsJson(key, &json.GetAllocator()), json.GetAllocator());
        json.AddMember("PrimaryKeys", primaryKeys, json.GetAllocator());
        }

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_MergedFieldNames & flags))
        {
        rapidjson::Value fieldNamesJson(rapidjson::kArrayType);
        for (Utf8StringCR fieldName : contentSetItem.GetMergedFieldNames())
            fieldNamesJson.PushBack(rapidjson::Value(fieldName.c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("MergedFieldNames", fieldNamesJson, json.GetAllocator());
        }

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_FieldPropertyInstanceKeys & flags))
        {
        rapidjson::Value fieldValueKeys(rapidjson::kObjectType);
        for (auto pair : contentSetItem.GetFieldInstanceKeys())
            {
            Utf8CP fieldName = pair.first.GetField().GetName().c_str();
            if (!fieldValueKeys.HasMember(fieldName))
                fieldValueKeys.AddMember(rapidjson::StringRef(fieldName), rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
            rapidjson::Value& fieldProperties = fieldValueKeys[fieldName];

            rapidjson::Value propertyKeys(rapidjson::kArrayType);
            for (ECClassInstanceKeyCR key : pair.second)
                propertyKeys.PushBack(_AsJson(key, &json.GetAllocator()), json.GetAllocator());

            rapidjson::Value fieldProperty(rapidjson::kObjectType);
            fieldProperty.AddMember("PropertyIndex", (uint64_t)pair.first.GetPropertyIndex(), json.GetAllocator());
            fieldProperty.AddMember("Keys", propertyKeys, json.GetAllocator());
            fieldProperties.PushBack(fieldProperty, json.GetAllocator());
            }
        json.AddMember("FieldValueKeys", fieldValueKeys, json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ECClassInstanceKeyCR key,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("ECClassId", rapidjson::Value(key.GetClass()->GetId().ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ECInstanceId", rapidjson::Value(key.GetId().ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(Content const& content,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Descriptor", content.GetDescriptor().AsJson(&json.GetAllocator()), json.GetAllocator());

    rapidjson::Value set(rapidjson::kArrayType);
    DataContainer<ContentSetItemCPtr> container = content.GetContentSet();
    for (ContentSetItemCPtr item : container)
        {
        if (item.IsValid())
            set.PushBack(item->AsJson(ContentSetItem::SERIALIZE_All, &json.GetAllocator()), json.GetAllocator());
        }

    json.AddMember("ContentSet", set, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ECInstanceChangeResult const& ecInstanceChangeResult,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    if (SUCCESS == ecInstanceChangeResult.GetStatus() && !ecInstanceChangeResult.GetChangedValue().IsUninitialized())
        {
        json.AddMember("Status", 0, json.GetAllocator());
        json.AddMember("Value", ValueHelpers::GetJsonFromECValue(ecInstanceChangeResult.GetChangedValue(), &json.GetAllocator()), json.GetAllocator());
        }
    else if (SUCCESS == ecInstanceChangeResult.GetStatus())
        {
        json.AddMember("Status", 1, json.GetAllocator());
        json.AddMember("IgnoreReason", rapidjson::Value(ecInstanceChangeResult.GetErrorMessage().c_str(), json.GetAllocator()), json.GetAllocator());
        }
    else
        {
        json.AddMember("Status", 2, json.GetAllocator());
        json.AddMember("ErrorMessage", rapidjson::Value(ecInstanceChangeResult.GetErrorMessage().c_str(), json.GetAllocator()), json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_NavNodeKeyAsJson(NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.SetObject();
    navNodeKeyBaseJson.AddMember("Type", rapidjson::Value(navNodeKey.GetType().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

    rapidjson::Value pathJson(rapidjson::kArrayType);
    for (Utf8StringCR pathElement : navNodeKey.GetPathFromRoot())
        pathJson.PushBack(rapidjson::Value(pathElement.c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

    navNodeKeyBaseJson.AddMember("PathFromRoot", pathJson, navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<Utf8String> ParseNodeKeyHashPath(JsonValueCR pathJson)
    {
    bvector<Utf8String> path;
    for (JsonValueCR pathElement : pathJson)
        path.push_back(pathElement.asString());
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<Utf8String> ParseNodeKeyHashPath(RapidJsonValueCR pathJson)
    {
    bvector<Utf8String> path;
    for (RapidJsonValueCR pathElement : pathJson.GetArray())
        path.push_back(pathElement.GetString());
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr DefaultECPresentationSerializer::_GetNavNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const
    {
    if (!json.isObject() || json.isNull())
        {
        BeAssert(false);
        return nullptr;
        }
    Utf8CP type = json["Type"].asCString();
    if (nullptr == type)
        {
        BeAssert(false);
        return nullptr;
        }
    if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type))
        return _GetECInstanceNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type))
        return _GetECClassGroupingNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
        return _GetECPropertyGroupingNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_DisplayLabelGroupingNode, type))
        return _GetLabelGroupingNodeKeyFromJson(json);
    return _GetBaseNavNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr DefaultECPresentationSerializer::_GetNavNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const
    {
    if (!json.IsObject() || json.IsNull())
        {
        BeAssert(false);
        return nullptr;
        }
    Utf8CP type = json["Type"].GetString();
    if (nullptr == type)
        {
        BeAssert(false);
        return nullptr;
        }
    if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type))
        return _GetECInstanceNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type))
        return _GetECClassGroupingNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
        return _GetECPropertyGroupingNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_DisplayLabelGroupingNode, type))
        return _GetLabelGroupingNodeKeyFromJson(json);
    return _GetBaseNavNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr DefaultECPresentationSerializer::_GetBaseNavNodeKeyFromJson(JsonValueCR json) const
    {
    Utf8CP type = json["Type"].asCString();
    return NavNodeKey::Create(type, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr DefaultECPresentationSerializer::_GetBaseNavNodeKeyFromJson(RapidJsonValueCR json) const
    {
    Utf8CP type = json["Type"].GetString();
    return NavNodeKey::Create(type, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ECInstanceNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("ECClassId", rapidjson::Value(ecInstanceNodeKey.GetECClassId().ToString().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("ECInstanceId", rapidjson::Value(ecInstanceNodeKey.GetInstanceId().ToString().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceNodeKeyPtr DefaultECPresentationSerializer::_GetECInstanceNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const
    {
    ECClassId classId(BeJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
    ECInstanceId instanceId(BeJsonUtilities::UInt64FromValue(json["ECInstanceId"]));
    return ECInstanceNodeKey::Create(ECClassInstanceKey(*ecClass, instanceId), ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceNodeKeyPtr DefaultECPresentationSerializer::_GetECInstanceNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const
    {
    ECClassId classId(BeRapidJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
    ECInstanceId instanceId(BeRapidJsonUtilities::UInt64FromValue(json["ECInstanceId"]));
    return ECInstanceNodeKey::Create(ECClassInstanceKey(*ecClass, instanceId), ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("ECClassId", rapidjson::Value(groupingNodeKey.GetECClassId().ToString().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const
    {
    ECClassId classId(BeJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
    return ECClassGroupingNodeKey::Create(*ecClass, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const
    {
    ECClassId classId(BeRapidJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
    return ECClassGroupingNodeKey::Create(*ecClass, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("ECClassId", rapidjson::Value(propertyGroupingNodeKey.GetECClassId().ToString().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("PropertyName", rapidjson::Value(propertyGroupingNodeKey.GetPropertyName().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    if (nullptr != propertyGroupingNodeKey.GetGroupingValue())
        navNodeKeyBaseJson.AddMember("GroupingValue", rapidjson::Value(*propertyGroupingNodeKey.GetGroupingValue(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const
    {
    ECClassId classId(BeJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
    Utf8CP propertyName = json["PropertyName"].asCString();
    if (!json.isMember("GroupingValue"))
        return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, nullptr, ParseNodeKeyHashPath(json["PathFromRoot"]));

    rapidjson::Document groupingValue;
    groupingValue.Parse(Json::FastWriter().write(json["GroupingValue"]).c_str());
    return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, &groupingValue, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const
    {
    ECClassId classId(BeRapidJsonUtilities::UInt64FromValue(json["ECClassId"]));
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
    Utf8CP propertyName = json["PropertyName"].GetString();
    rapidjson::Value const* groupingValue = nullptr;
    if (json.HasMember("GroupingValue"))
        groupingValue = &json["GroupingValue"];
    return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, groupingValue, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("Label", rapidjson::Value(labelGroupingNodeKey.GetLabel().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LabelGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetLabelGroupingNodeKeyFromJson(JsonValueCR json) const
    {
    Utf8CP label = json["Label"].asCString();
    return LabelGroupingNodeKey::Create(label, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LabelGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR json) const
    {
    Utf8CP label = json["Label"].GetString();
    return LabelGroupingNodeKey::Create(label, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(NavNode const& navNode, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    Utf8String nodeId = BeInt64Id(navNode.GetNodeId()).ToString();
    Utf8String parentNodeId = BeInt64Id(navNode.GetParentNodeId()).ToString();
    Utf8String instanceId = BeInt64Id(navNode.GetInstanceId()).ToString();
    json.AddMember("NodeId", rapidjson::Value(nodeId.c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ParentNodeId", rapidjson::Value(parentNodeId.c_str(), json.GetAllocator()), json.GetAllocator());
    if (navNode.GetKey().IsNull())
        {
        BeAssert(false);
        json.AddMember("Key", NavNodeKey::Create("", bvector<Utf8String>())->AsJson(&json.GetAllocator()), json.GetAllocator());
        }
    else
        json.AddMember("Key", navNode.GetKey()->AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("Label", rapidjson::Value(navNode.GetLabel().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Description", rapidjson::Value(navNode.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ExpandedImageId", rapidjson::Value(navNode.GetExpandedImageId().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("CollapsedImageId", rapidjson::Value(navNode.GetCollapsedImageId().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ForeColor", rapidjson::Value(navNode.GetForeColor().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("BackColor", rapidjson::Value(navNode.GetBackColor().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("FontStyle", rapidjson::Value(navNode.GetFontStyle().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Type", rapidjson::Value(navNode.GetType().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("HasChildren", navNode.HasChildren(), json.GetAllocator());
    json.AddMember("IsSelectable", navNode.IsSelectable(), json.GetAllocator());
    json.AddMember("IsEditable", navNode.IsEditable(), json.GetAllocator());
    json.AddMember("IsChecked", navNode.IsChecked(), json.GetAllocator());
    json.AddMember("IsCheckboxVisible", navNode.IsCheckboxVisible(), json.GetAllocator());
    json.AddMember("IsCheckboxEnabled", navNode.IsCheckboxEnabled(), json.GetAllocator());
    json.AddMember("IsExpanded", navNode.IsExpanded(), json.GetAllocator());
    json.AddMember("ECInstanceId", rapidjson::Value(instanceId.c_str(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(NodesPathElement const& navNodesPathElement,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    if (navNodesPathElement.GetNode().IsNull())
        return json;

    json.SetObject();
    json.AddMember("Node", navNodesPathElement.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("Index", (uint64_t)navNodesPathElement.GetIndex(), json.GetAllocator());
    json.AddMember("IsMarked", navNodesPathElement.IsMarked(), json.GetAllocator());

    rapidjson::Value childrenJson;
    childrenJson.SetArray();
    for (NodesPathElement const& child : navNodesPathElement.GetChildren())
        childrenJson.PushBack(child.AsJson(&json.GetAllocator()), json.GetAllocator());

    json.AddMember("Children", childrenJson, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(KeySet const& keySet, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();  
    rapidjson::Value instances(rapidjson::kObjectType);
    for (auto const& pair : keySet.GetInstanceKeys())
        {
        ECClassCP ecClass = pair.first;
        rapidjson::Value instanceIds(rapidjson::kArrayType);
        for (ECInstanceId const& instanceId : pair.second)
            instanceIds.PushBack(instanceId.GetValueUnchecked(), json.GetAllocator());
        instances.AddMember(rapidjson::Value(ecClass->GetId().ToString().c_str(), json.GetAllocator()), instanceIds, json.GetAllocator());
        }

    rapidjson::Value nodeKeys(rapidjson::kArrayType);
    for (NavNodeKeyCPtr const& key : keySet.GetNavNodeKeys())
        nodeKeys.PushBack(key->AsJson(&json.GetAllocator()), json.GetAllocator());

    json.AddMember("InstanceKeys", instances, json.GetAllocator());
    json.AddMember("NodeKeys", nodeKeys, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr DefaultECPresentationSerializer::_GetKeySetFromJson(IConnectionCR connection, JsonValueCR json) const
    {
    InstanceKeyMap instanceKeys;
    JsonValueCR instanceKey = json["InstanceKeys"];
    bvector<Utf8String> classIds = instanceKey.getMemberNames();
    for (Utf8StringCR classIdString : classIds)
        {
        ECClassId ecClassId;
        ECClassId::FromString(ecClassId, classIdString.c_str());
        ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(ecClassId);
        if (nullptr == ecClass)
            {
            BeAssert(false);
            continue;
            }
        bset<ECInstanceId> instanceIdSet;
        for (JsonValueCR instanceIdJson : instanceKey[classIdString.c_str()])
            {
            uint64_t instanceId = BeJsonUtilities::UInt64FromValue(instanceIdJson);
            instanceIdSet.insert(ECInstanceId(instanceId));
            }
        instanceKeys[ecClass] = instanceIdSet;
        }

    NavNodeKeySet nodeKeys;
    for (JsonValueCR nodeKeyJson : json["NodeKeys"])
        nodeKeys.insert(_GetNavNodeKeyFromJson(connection, nodeKeyJson));

    return KeySet::Create(instanceKeys, nodeKeys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(SelectionChangedEvent const& selectionChangedEvent,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    if (selectionChangedEvent.IsValid())
        json.AddMember(rapidjson::StringRef("ConnectionId"), rapidjson::Value(selectionChangedEvent.GetConnection().GetId().c_str(), json.GetAllocator()), json.GetAllocator());

    json.AddMember(rapidjson::StringRef("Source"), rapidjson::StringRef(selectionChangedEvent.GetSourceName().c_str()), json.GetAllocator());
    json.AddMember(rapidjson::StringRef("IsSubSelection"), selectionChangedEvent.IsSubSelection(), json.GetAllocator());
    json.AddMember(rapidjson::StringRef("ChangeType"), (int) selectionChangedEvent.GetChangeType(), json.GetAllocator());
    json.AddMember(rapidjson::StringRef("Keys"), selectionChangedEvent.GetSelectedKeys().AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember(rapidjson::StringRef("Timestamp"), rapidjson::Value(std::to_string(selectionChangedEvent.GetTimestamp()).c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember(rapidjson::StringRef("ExtendedData"), rapidjson::Value(selectionChangedEvent.GetExtendedData(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionChangedEventPtr DefaultECPresentationSerializer::_GetSelectionChangedEventFromJson(IConnectionCacheCR connectionCache, JsonValueCR json) const
    {
    if (!(json.isMember("ConnectionId") && json["ConnectionId"].isString()) ||
        !(json.isMember("Source") && json["Source"].isString()) ||
        !(json.isMember("ChangeType") && json["ChangeType"].isInt()) ||
        !(json.isMember("IsSubSelection") && json["IsSubSelection"].isBool()) ||
        !(json.isMember("Timestamp") && json["Timestamp"].isString()) ||
        !json.isMember("Keys"))
        {
        BeAssert(false);
        return nullptr;
        }

    Utf8CP connectionId = json["ConnectionId"].asCString();
    IConnectionCP connection = connectionCache.GetConnection(connectionId);
    if (nullptr == connection)
        {
        BeAssert(false);
        return nullptr;
        }

    Utf8String sourceName = json["Source"].asCString();
    SelectionChangeType changeType = (SelectionChangeType) json["ChangeType"].asInt();
    bool isSubSelection = json["IsSubSelection"].asBool();
    uint64_t timestamp = BeJsonUtilities::UInt64FromValue(json["Timestamp"]);
    JsonValueCR keysJson = json["Keys"];
    KeySetPtr keys = KeySet::FromJson(*connection, keysJson);

    SelectionChangedEventPtr evt = SelectionChangedEvent::Create(*connection, sourceName, changeType, isSubSelection, *keys, timestamp);

    if (json.isMember("ExtendedData") && json["ExtendedData"].isObject() && !json["ExtendedData"].empty())
        {
        Utf8String serializedExtendedData = Json::FastWriter().write(json["ExtendedData"]);
        rapidjson::Document extendedData;
        extendedData.Parse(serializedExtendedData.c_str());
        evt->GetExtendedDataR().CopyFrom(extendedData, evt->GetExtendedDataAllocator());
        }

    return evt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(ECEnumerationCR enumeration,
    rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (ECEnumeratorCP enumerator : enumeration.GetEnumerators())
        {
        rapidjson::Value choice(rapidjson::kObjectType);
        choice.AddMember("Label", rapidjson::Value(enumerator->GetDisplayLabel().c_str(), allocator), allocator);
        if (enumerator->IsInteger())
            choice.AddMember("Value", rapidjson::Value(enumerator->GetInteger()), allocator);
        else
            choice.AddMember("Value", rapidjson::Value(enumerator->GetString().c_str(), allocator), allocator);

        json.PushBack(choice, allocator);
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(KindOfQuantityCR koq,
    rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("Name", rapidjson::StringRef(koq.GetFullName().c_str()), allocator);
    json.AddMember("DisplayLabel", rapidjson::StringRef(koq.GetDisplayLabel().c_str()), allocator);
    json.AddMember("PersistenceUnit", rapidjson::Value(koq.GetPersistenceUnit()->GetName().c_str(), allocator), allocator);
    json.AddMember("CurrentFusId", rapidjson::Value(koq.GetDefaultPresentationFormat()->GetName().c_str(), allocator), allocator);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(RelatedClassPathCR path,
    rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (RelatedClass const& relatedClass : path)
        {
        if (!relatedClass.IsValid())
            {
            BeAssert(false);
            continue;
            }
        rapidjson::Value relatedClassJson(rapidjson::kObjectType);
        relatedClassJson.AddMember("SourceClassInfo", _AsJson(*relatedClass.GetSourceClass(), &allocator), allocator);
        relatedClassJson.AddMember("TargetClassInfo", _AsJson(*relatedClass.GetTargetClass(), &allocator), allocator);
        relatedClassJson.AddMember("RelationshipInfo", _AsJson(*relatedClass.GetRelationship(), &allocator), allocator);
        relatedClassJson.AddMember("IsForwardRelationship", relatedClass.IsForwardRelationship(), allocator);
        relatedClassJson.AddMember("IsPolymorphicRelationship", relatedClass.IsPolymorphic(), allocator);
        json.PushBack(relatedClassJson, allocator);
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(SelectionInfo const& selectionInfo,
    rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value info(rapidjson::kObjectType);
    info.AddMember("SelectionProvider", rapidjson::StringRef(selectionInfo.GetSelectionProviderName().c_str()), allocator);
    info.AddMember("IsSubSelection", selectionInfo.IsSubSelection(), allocator);
    info.AddMember("Timestamp", rapidjson::Value(std::to_string(selectionInfo.GetTimestamp()).c_str(), allocator), allocator);
    return info;
    }