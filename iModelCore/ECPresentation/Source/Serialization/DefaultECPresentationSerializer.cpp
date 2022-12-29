/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>
#include "../Shared/ECSchemaHelper.h"
#include "../Shared/Queries/PresentationQuery.h"

// Member names of the serialized NavNode JSON object
#define NAVNODE_NodeId              "NodeId"
#define NAVNODE_ParentNodeId        "ParentNodeId"
#define NAVNODE_Key                 "Key"
#define NAVNODE_InstanceId          "ECInstanceId"
#define NAVNODE_ExpandedImageId     "ExpandedImageId"
#define NAVNODE_CollapsedImageId    "CollapsedImageId"
#define NAVNODE_ForeColor           "ForeColor"
#define NAVNODE_BackColor           "BackColor"
#define NAVNODE_FontStyle           "FontStyle"
#define NAVNODE_Type                "Type"
#define NAVNODE_HasChildren         "HasChildren"
#define NAVNODE_IsChecked           "IsChecked"
#define NAVNODE_IsCheckboxVisible   "IsCheckboxVisible"
#define NAVNODE_IsCheckboxEnabled   "IsCheckboxEnabled"
#define NAVNODE_IsExpanded          "IsExpanded"
#define NAVNODE_Description         "Description"
#define NAVNODE_InternalData        "InternalData"
#define NAVNODE_UsersExtendedData   "ExtendedData"
#define NAVNODE_LabelDefinition     "LabelDefinition"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ConnectionEvent const& connectionEvent,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("EventType", (int)connectionEvent.GetEventType(), json.GetAllocator());

    rapidjson::Value connectionJson(rapidjson::kObjectType);
    connectionJson.AddMember("ConnectionId", rapidjson::Value(connectionEvent.GetConnection().GetId().c_str(), json.GetAllocator()), json.GetAllocator());
    connectionJson.AddMember("ConnectionGuid", rapidjson::Value(connectionEvent.GetConnection().GetECDb().GetDbGuid().ToString().c_str(), json.GetAllocator()), json.GetAllocator());

    json.AddMember("Connection", connectionJson, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentFieldRenderer const& contentFieldRenderer,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Name", rapidjson::Value(contentFieldRenderer.GetName().c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentFieldEditor const& contentFieldEditor,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Name", rapidjson::Value(contentFieldEditor.GetName().c_str(), json.GetAllocator()), json.GetAllocator());

    rapidjson::Value paramsJson(rapidjson::kObjectType);
    for (ContentFieldEditor::Params const* params : contentFieldEditor.GetParams())
        {
        if (paramsJson.HasMember(params->GetName()))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, Utf8PrintfString("Field editor params with name '%s' already exists", params->GetName()));

        paramsJson.AddMember(rapidjson::Value(params->GetName(), json.GetAllocator()), params->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
        }

    json.AddMember("Params", paramsJson, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Category const& category,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Name", rapidjson::Value(category.GetName().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("DisplayLabel", rapidjson::Value(category.GetLabel().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Description", rapidjson::Value(category.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Expand", category.ShouldExpand(), json.GetAllocator());
    json.AddMember("Priority", category.GetPriority(), json.GetAllocator());
    if (!category.GetRendererName().empty())
        {
        rapidjson::Value renderer(rapidjson::kObjectType);
        renderer.AddMember("Name", rapidjson::Value(category.GetRendererName().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("Renderer", renderer, json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Property const& property,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    rapidjson::Value propertyJson(rapidjson::kObjectType);
    propertyJson.AddMember("BaseClassInfo", _AsJson(ctx, property.GetProperty().GetClass(), &json.GetAllocator()), json.GetAllocator());
    propertyJson.AddMember("ActualClassInfo", _AsJson(ctx, property.GetPropertyClass(), &json.GetAllocator()), json.GetAllocator());
    propertyJson.AddMember("Name", rapidjson::Value(property.GetProperty().GetName().c_str(), json.GetAllocator()), json.GetAllocator());

    if (property.GetProperty().GetIsPrimitive() && nullptr != property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration())
        {
        ECEnumerationCP propEnum = property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration();
        propertyJson.AddMember("Type", "enum", json.GetAllocator());
        propertyJson.AddMember("IsStrict", rapidjson::Value(propEnum->GetIsStrict()), json.GetAllocator());
        propertyJson.AddMember("Choices", _AsJson(ctx, *propEnum, json.GetAllocator()), json.GetAllocator());
        }
    else
        {
        propertyJson.AddMember("Type", rapidjson::Value(ECSchemaHelper::GetTypeName(property.GetProperty()).c_str(), json.GetAllocator()), json.GetAllocator());
        }

    if (nullptr != property.GetProperty().GetKindOfQuantity())
        {
        KindOfQuantityCP koq = property.GetProperty().GetKindOfQuantity();
        propertyJson.AddMember("KindOfQuantity", _AsJson(ctx, *koq, json.GetAllocator()), json.GetAllocator());
        }

    json.AddMember("Property", propertyJson, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_FieldAsJson(ContextR ctx, ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const
    {
    fieldBaseJson.SetObject();
    if (field.GetCategory())
        fieldBaseJson.AddMember("Category", field.GetCategory()->AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("Name", rapidjson::Value(field.GetUniqueName().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("DisplayLabel", rapidjson::Value(field.GetLabel().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("Type", field.GetTypeDescription().AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("IsReadOnly", field.IsReadOnly(), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("Priority", field.GetPriority(), fieldBaseJson.GetAllocator());

    if (nullptr != field.GetRenderer())
        fieldBaseJson.AddMember("Renderer", field.GetRenderer()->AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());

    if (nullptr != field.GetEditor())
        fieldBaseJson.AddMember("Editor", field.GetEditor()->AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const
    {
    rapidjson::Value propertiesJson(rapidjson::kArrayType);
    for (ContentDescriptor::Property const& prop : ecPropertiesField.GetProperties())
        propertiesJson.PushBack(prop.AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("Properties", propertiesJson, fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_NestedContentFieldAsJson(ContextR ctx, ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const
    {
    rapidjson::Value nestedFieldsJson(rapidjson::kArrayType);
    for (ContentDescriptor::Field const* nestedField : nestedContentField.GetFields())
        nestedFieldsJson.PushBack(nestedField->AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("NestedFields", nestedFieldsJson, fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::CompositeContentField const& compositeContentField, RapidJsonDocumentR fieldBaseJson) const
    {
    fieldBaseJson.AddMember("ContentClassInfo", _AsJson(ctx, compositeContentField.GetContentClass(), &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::RelatedContentField const& relatedContentField, RapidJsonDocumentR fieldBaseJson) const
    {
    fieldBaseJson.AddMember("PathFromSelectToContentClass", _AsJson(ctx, relatedContentField.GetPathFromSelectToContentClass(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct HierarchyChangeSerializer : HierarchyChangeRecordDiffHandler
{
private:
    rapidjson::Document::AllocatorType& m_allocator;
    rapidjson::Value& m_changes;

private:
    //! Make sure `member` string lifetime is static
    void Add(rapidjson::Document::StringRefType member, rapidjson::Value& value)
        {
        m_changes.AddMember(member, value, m_allocator);
        }

protected:
    void _HandleKey(NavNodeKeyCR newValue) override {Add(NAVNODE_Key, newValue.AsJson(&m_allocator).Move());}
    void _HandleHasChildren(bool newValue) override {Add(NAVNODE_HasChildren, rapidjson::Value(newValue).Move());}
    void _HandleIsChecked(bool newValue) override {Add(NAVNODE_IsChecked, rapidjson::Value(newValue).Move());}
    void _HandleIsCheckboxVisible(bool newValue) override {Add(NAVNODE_IsCheckboxVisible, rapidjson::Value(newValue).Move());}
    void _HandleIsCheckboxEnabled(bool newValue) override {Add(NAVNODE_IsCheckboxEnabled, rapidjson::Value(newValue).Move());}
    void _HandleShouldAutoExpand(bool newValue) override {Add(NAVNODE_IsExpanded, rapidjson::Value(newValue).Move()); }
    void _HandleDescription(Utf8StringCR newValue) override {Add(NAVNODE_Description, rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleImageId(Utf8StringCR newValue) override {Add(NAVNODE_CollapsedImageId, rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleForeColor(Utf8StringCR newValue) override {Add(NAVNODE_ForeColor, rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleBackColor(Utf8StringCR newValue) override {Add(NAVNODE_BackColor, rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleFontStyle(Utf8StringCR newValue) override {Add(NAVNODE_FontStyle, rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleType(Utf8StringCR newValue) override {Add(NAVNODE_Type, rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleLabelDefinition(LabelDefinitionCR newValue) override {Add(NAVNODE_LabelDefinition, newValue.AsJson(&m_allocator).Move());}

public:
    HierarchyChangeSerializer(rapidjson::Value& changes, rapidjson::Document::AllocatorType& allocator): m_changes(changes), m_allocator(allocator) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, HierarchyChangeRecord const& changeRecord, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    switch (changeRecord.GetChangeType())
        {
        case ChangeType::Delete:
            {
            json.AddMember("Type", "Delete", json.GetAllocator());
            json.AddMember("Node", changeRecord.GetNode()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
            if (changeRecord.GetParentNode().IsValid())
                json.AddMember("Parent", changeRecord.GetParentNode()->GetKey()->AsJson(&json.GetAllocator()), json.GetAllocator());
            json.AddMember("Position", (uint64_t)changeRecord.GetPosition(), json.GetAllocator());
            break;
            }
        case ChangeType::Insert:
            {
            json.AddMember("Type", "Insert", json.GetAllocator());
            json.AddMember("Node", changeRecord.GetNode()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
            if (changeRecord.GetParentNode().IsValid())
                json.AddMember("Parent", changeRecord.GetParentNode()->GetKey()->AsJson(&json.GetAllocator()), json.GetAllocator());
            json.AddMember("Position", (uint64_t)changeRecord.GetPosition(), json.GetAllocator());
            break;
            }
        case ChangeType::Update:
            {
            json.AddMember("Type", "Update", json.GetAllocator());
            json.AddMember("Node", changeRecord.GetNode()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
            json.AddMember("Changes", rapidjson::Value(rapidjson::kObjectType), json.GetAllocator());
            RapidJsonValueR changesJson = json["Changes"];
            auto serializer = HierarchyChangeSerializer(changesJson, json.GetAllocator());
            changeRecord.GetNodeChanges().FindChanges(serializer);
            break;
            }
        }
    json.AddMember("RulesetId", rapidjson::Value(changeRecord.GetRulesetId().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ECDbFileName", rapidjson::Value(changeRecord.GetECDbFileName().c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, HierarchyUpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    if (updateRecord.GetParentNode().IsValid())
        json.AddMember("Parent", updateRecord.GetParentNode()->GetKey()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("NodesCount", (uint64_t)updateRecord.GetNodesCount(), json.GetAllocator());

    rapidjson::Value expandedNodes(rapidjson::kArrayType);
    for (HierarchyUpdateRecord::ExpandedNode const& expandedNode : updateRecord.GetExpandedNodes())
        expandedNodes.PushBack(expandedNode.AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    if (!expandedNodes.Empty())
        json.AddMember("ExpandedNodes", expandedNodes.Move(), json.GetAllocator());

    json.AddMember("RulesetId", rapidjson::Value(updateRecord.GetRulesetId().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ECDbFileName", rapidjson::Value(updateRecord.GetECDbFileName().c_str(), json.GetAllocator()), json.GetAllocator());

    if (!updateRecord.GetInstanceFilter().empty())
        json.AddMember("InstanceFilter", rapidjson::Value(updateRecord.GetInstanceFilter().c_str(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, HierarchyUpdateRecord::ExpandedNode const& expandedNode, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Node", expandedNode.GetNode()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("Position", (uint64_t)expandedNode.GetPosition(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const
    {
    paramsBaseJson.CopyFrom(jsonParams.GetJson(), paramsBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const
    {
    paramsBaseJson.SetObject();
    paramsBaseJson.AddMember("HeightInRows", multilineParams.GetParameters().GetHeightInRows(), paramsBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const
    {
    paramsBaseJson.SetObject();

    if (rangeParams.GetParameters().GetMinimumValue().IsValid())
        paramsBaseJson.AddMember("Minimum", rangeParams.GetParameters().GetMinimumValue().Value(), paramsBaseJson.GetAllocator());
    else
        paramsBaseJson.AddMember("Minimum", rapidjson::Value(), paramsBaseJson.GetAllocator());

    if (rangeParams.GetParameters().GetMaximumValue().IsValid())
        paramsBaseJson.AddMember("Maximum", rangeParams.GetParameters().GetMaximumValue().Value(), paramsBaseJson.GetAllocator());
    else
        paramsBaseJson.AddMember("Maximum", rapidjson::Value(), paramsBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const
    {
    paramsBaseJson.SetObject();
    paramsBaseJson.AddMember("Minimum", sliderParams.GetParameters().GetMinimumValue(), paramsBaseJson.GetAllocator());
    paramsBaseJson.AddMember("Maximum", sliderParams.GetParameters().GetMaximumValue(), paramsBaseJson.GetAllocator());
    paramsBaseJson.AddMember("IntervalsCount", sliderParams.GetParameters().GetIntervalsCount(), paramsBaseJson.GetAllocator());
    paramsBaseJson.AddMember("ValueFactor", sliderParams.GetParameters().GetValueFactor(), paramsBaseJson.GetAllocator());
    paramsBaseJson.AddMember("IsVertical", sliderParams.GetParameters().IsVertical(), paramsBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_TypeDescriptionAsJson(ContextR ctx, ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.SetObject();
    typeDescriptionBaseJson.AddMember("TypeName", rapidjson::Value(typeDescription.GetTypeName().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Field::PrimitiveTypeDescription const&,
    RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("ValueFormat", "Primitive", typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription,
    RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("ValueFormat", "Array", typeDescriptionBaseJson.GetAllocator());
    typeDescriptionBaseJson.AddMember("MemberType", arrayTypeDescription.GetMemberType()->AsJson(ctx, &typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Field::StructTypeDescription const& structTypeDescription,
    RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("ValueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
    rapidjson::Value members(rapidjson::kArrayType);
    for (ECPropertyCP prop : structTypeDescription.GetStruct().GetProperties())
        {
        rapidjson::Value member(rapidjson::kObjectType);
        member.AddMember("Name", rapidjson::StringRef(prop->GetName().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("Label", rapidjson::StringRef(prop->GetDisplayLabel().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("Type", ContentDescriptor::ECPropertiesField::TypeDescription::Create(*prop)->AsJson(ctx, &typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
        }
    typeDescriptionBaseJson.AddMember("Members", members, typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription,
    RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("ValueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
    rapidjson::Value members(rapidjson::kArrayType);
    for (ContentDescriptor::Field const* nestedField : nestedContentTypeDescription.GetNestedContentField().GetFields())
        {
        rapidjson::Value member(rapidjson::kObjectType);
        member.AddMember("Name", rapidjson::StringRef(nestedField->GetUniqueName().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("Label", rapidjson::StringRef(nestedField->GetLabel().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("Type", nestedField->GetTypeDescription().AsJson(ctx, &typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
        }
    typeDescriptionBaseJson.AddMember("Members", members, typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ECClassCR ecClass,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Id", AsJson(ctx, ecClass.GetId(), &json.GetAllocator()), json.GetAllocator());
    json.AddMember("Name", rapidjson::StringRef(ecClass.GetFullName()), json.GetAllocator());
    json.AddMember("Label", rapidjson::StringRef(ecClass.GetDisplayLabel().c_str()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, SelectClassInfo const& selectClass,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("SelectClassInfo", _AsJson(ctx, selectClass.GetSelectClass().GetClass(), &json.GetAllocator()), json.GetAllocator());
    json.AddMember("IsPolymorphic", selectClass.GetSelectClass().IsSelectPolymorphic(), json.GetAllocator());
    json.AddMember("PathToSelectClass", _AsJson(ctx, selectClass.GetPathFromInputToSelectClass(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("RelatedPropertyPaths", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (RelatedClassPathCR propertyPath : selectClass.GetRelatedPropertyPaths())
        json["RelatedPropertyPaths"].PushBack(_AsJson(ctx, propertyPath, json.GetAllocator()), json.GetAllocator());
    json.AddMember("NavigationPropertyClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (RelatedClassCR navPropertyClass : selectClass.GetNavigationPropertyClasses())
        json["NavigationPropertyClasses"].PushBack(_AsJson(ctx, navPropertyClass, json.GetAllocator()), json.GetAllocator());
    json.AddMember("RelatedInstanceClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (RelatedClassPathCR relatedInstancePath : selectClass.GetRelatedInstancePaths())
        json["RelatedInstancePaths"].PushBack(_AsJson(ctx, relatedInstancePath, json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor const& contentDescriptor,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("PreferredDisplayType", rapidjson::StringRef(contentDescriptor.GetPreferredDisplayType() != ContentDisplayType::Undefined ? contentDescriptor.GetPreferredDisplayType().c_str() : ""), json.GetAllocator());
    json.AddMember("SelectClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (SelectClassInfo const& selectClass : contentDescriptor.GetSelectClasses())
        json["SelectClasses"].PushBack(AsJson(ctx, selectClass, &json.GetAllocator()), json.GetAllocator());

    bvector<ContentDescriptor::Field*> visibleFields = contentDescriptor.GetVisibleFields();
    json.AddMember("Fields", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (ContentDescriptor::Field const* field : visibleFields)
        json["Fields"].PushBack(field->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("SortingFieldIndex", contentDescriptor.GetSortingFieldIndex(), json.GetAllocator());
    json.AddMember("SortDirection", (int)contentDescriptor.GetSortDirection(), json.GetAllocator());
    json.AddMember("ContentFlags", contentDescriptor.GetContentFlags(), json.GetAllocator());
    json.AddMember("ConnectionId", rapidjson::StringRef(contentDescriptor.GetConnectionId().c_str()), json.GetAllocator());
    json.AddMember("FilterExpression", rapidjson::StringRef(contentDescriptor.GetFieldsFilterExpression().c_str()), json.GetAllocator());
    json.AddMember("InputKeysHash", rapidjson::Value(contentDescriptor.GetInputNodeKeys().GetHash().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("RulesetId", rapidjson::StringRef(contentDescriptor.GetRuleset().GetRuleSetId().c_str()), json.GetAllocator());
    json.AddMember("UnitSystem", (int)contentDescriptor.GetUnitSystem(), json.GetAllocator());
    if (nullptr != contentDescriptor.GetSelectionInfo())
        json.AddMember("SelectionInfo", _AsJson(ctx, *contentDescriptor.GetSelectionInfo(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ContentSetItem const& contentSetItem, int flags,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayLabel & flags))
        json.AddMember("DisplayLabel", _AsJson(ctx, contentSetItem.GetDisplayLabelDefinition(), &json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_ImageId & flags))
        json.AddMember("ImageId", rapidjson::Value(contentSetItem.GetImageId().c_str(), json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_Values & flags))
        {
        json.AddMember("Values", rapidjson::Value(contentSetItem.GetValues(), json.GetAllocator()), json.GetAllocator());
        for (auto const& nestedContentEntry : contentSetItem.GetNestedContent())
            {
            rapidjson::Value nestedValuesJson(rapidjson::kArrayType);
            for (auto const& nestedItem : nestedContentEntry.second)
                {
                static const int valueSerializationFlags = ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values | ContentSetItem::SERIALIZE_DisplayValues | ContentSetItem::SERIALIZE_MergedFieldNames;
                nestedValuesJson.PushBack(nestedItem->AsJson(ctx, valueSerializationFlags, &json.GetAllocator()), json.GetAllocator());
                }
            json["Values"].AddMember(rapidjson::Value(nestedContentEntry.first.c_str(), json.GetAllocator()), nestedValuesJson, json.GetAllocator());
            }
        }

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayValues & flags))
        {
        json.AddMember("DisplayValues", rapidjson::Value(contentSetItem.GetDisplayValues(), json.GetAllocator()), json.GetAllocator());
        for (auto const& nestedContentEntry : contentSetItem.GetNestedContent())
            {
            rapidjson::Value nestedValuesJson(rapidjson::kArrayType);
            for (auto const& nestedItem : nestedContentEntry.second)
                nestedValuesJson.PushBack(nestedItem->AsJson(ctx, (int)ContentSetItem::SERIALIZE_DisplayValues, &json.GetAllocator()), json.GetAllocator());
            json["DisplayValues"].AddMember(rapidjson::Value(nestedContentEntry.first.c_str(), json.GetAllocator()), nestedValuesJson, json.GetAllocator());
            }
        }

    if (contentSetItem.GetClass() != nullptr && 0 != (ContentSetItem::SerializationFlags::SERIALIZE_ClassInfo & flags))
        json.AddMember("ClassInfo", _AsJson(ctx, *contentSetItem.GetClass(), &json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_PrimaryKeys & flags))
        {
        rapidjson::Value primaryKeys(rapidjson::kArrayType);
        for (ECClassInstanceKeyCR key : contentSetItem.GetKeys())
            primaryKeys.PushBack(_AsJson(ctx, key, &json.GetAllocator()), json.GetAllocator());
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
            Utf8CP fieldName = pair.first.GetFieldName().c_str();
            if (!fieldValueKeys.HasMember(fieldName))
                fieldValueKeys.AddMember(rapidjson::Value(fieldName, json.GetAllocator()), rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
            rapidjson::Value& fieldProperties = fieldValueKeys[fieldName];

            rapidjson::Value propertyKeys(rapidjson::kArrayType);
            for (ECClassInstanceKeyCR key : pair.second)
                propertyKeys.PushBack(_AsJson(ctx, key, &json.GetAllocator()), json.GetAllocator());

            rapidjson::Value fieldProperty(rapidjson::kObjectType);
            fieldProperty.AddMember("PropertyIndex", (uint64_t)pair.first.GetPropertyIndex(), json.GetAllocator());
            fieldProperty.AddMember("Keys", propertyKeys, json.GetAllocator());
            fieldProperties.PushBack(fieldProperty, json.GetAllocator());
            }
        json.AddMember("FieldValueKeys", fieldValueKeys, json.GetAllocator());
        }

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_UsersExtendedData & flags) && contentSetItem.GetUsersExtendedData().GetJson().MemberCount() > 0)
        {
        json.AddMember("ExtendedData", rapidjson::Value(contentSetItem.GetUsersExtendedData().GetJson(), json.GetAllocator()), json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, DisplayValueGroupCR value,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Label", rapidjson::Value(value.GetDisplayValue().c_str(), json.GetAllocator()), json.GetAllocator());
    rapidjson::Value rawValuesJson(rapidjson::kArrayType);
    for (RapidJsonValueCR rawValue : value.GetRawValues())
        rawValuesJson.PushBack(rapidjson::Value(rawValue, json.GetAllocator()), json.GetAllocator());
    json.AddMember("RawValues", rawValuesJson, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, BeInt64Id const& id, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetString(id.ToString().c_str(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ECClassInstanceKeyCR key,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("ECClassId", AsJson(ctx, key.GetClass()->GetId(), &json.GetAllocator()), json.GetAllocator());
    json.AddMember("ECInstanceId", AsJson(ctx, key.GetId(), &json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, Content const& content,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Descriptor", content.GetDescriptor().AsJson(ctx, &json.GetAllocator()), json.GetAllocator());

    rapidjson::Value set(rapidjson::kArrayType);
    DataContainer<ContentSetItemCPtr> container = content.GetContentSet();
    for (ContentSetItemCPtr item : container)
        {
        if (item.IsValid())
            set.PushBack(item->AsJson(ctx, ContentSetItem::SERIALIZE_All, &json.GetAllocator()), json.GetAllocator());
        }

    json.AddMember("ContentSet", set, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, ECInstanceChangeResult const& ecInstanceChangeResult,
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, PresentationQueryBase const& presentationQueryBase,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Query", rapidjson::Value(presentationQueryBase.ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Bindings", _AsJson(ctx, presentationQueryBase.GetBoundValues(), &json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, BoundQueryValuesList const& boundQueryValuesList,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetArray();
    auto boundQueryValueSerializer = DefaultBoundQueryValueSerializer();
    for (size_t i = 0; i < boundQueryValuesList.size(); ++i)
        {
        auto const& value = boundQueryValuesList.at(i);
        json.PushBack(value->ToJson(boundQueryValueSerializer, &json.GetAllocator()), json.GetAllocator());
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_NavNodeKeyAsJson(ContextR ctx, NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.SetObject();
    navNodeKeyBaseJson.AddMember("Type", rapidjson::Value(navNodeKey.GetType().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("SpecificationIdentifier", rapidjson::Value(navNodeKey.GetSpecificationIdentifier().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

    rapidjson::Value pathJson(rapidjson::kArrayType);
    for (Utf8StringCR pathElement : navNodeKey.GetHashPath())
        pathJson.PushBack(rapidjson::Value(pathElement.c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

    navNodeKeyBaseJson.AddMember("PathFromRoot", pathJson, navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("InstanceKeysSelectQuery", _AsJson(ctx, *navNodeKey.GetInstanceKeysSelectQuery(), &navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<Utf8String> ParseNodeKeyHashPath(BeJsConst pathJson)
    {
    bvector<Utf8String> path;
    pathJson.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst pathElement)
        {
        path.push_back(pathElement.asString());
        return false;
        });
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr DefaultECPresentationSerializer::_GetNavNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const
    {
    if (!json.isObject() || json.isNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, "Attempting to get NavNode key from invalid JSON");

    Utf8CP type = json["Type"].asCString();
    if (nullptr == type)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, "NavNode key JSON doesn't contain node type information");

    if (0 == strcmp(NAVNODE_TYPE_ECInstancesNode, type))
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr DefaultECPresentationSerializer::_GetBaseNavNodeKeyFromJson(BeJsConst json) const
    {
    Utf8CP type = json["Type"].asCString();
    Utf8CP specificationIdentifier = json["SpecificationIdentifier"].asCString();
    NavNodeKeyPtr key = NavNodeKey::Create(type, specificationIdentifier, ParseNodeKeyHashPath(json["PathFromRoot"]));
    key->SetInstanceKeysSelectQuery(std::unique_ptr<const PresentationQuery>(GetPresentationQueryFromJson(json["InstanceKeysSelectQuery"])));
    return key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PresentationQuery> DefaultECPresentationSerializer::_GetPresentationQueryFromJson(BeJsConst json) const
    {
    Utf8CP queryString = json["Query"].asCString();
    BoundQueryValuesList bindings;
    DefaultBoundQueryValueSerializer serializer;
    bindings.FromJson(serializer, json["Bindings"]);
    return std::make_unique<PresentationQuery>(queryString, bindings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ECInstancesNodeKey const& nodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    rapidjson::Value keysJson(rapidjson::kArrayType);
    for (ECClassInstanceKey const& instanceKey : nodeKey.GetInstanceKeys())
        {
        rapidjson::Value instanceKeyJson(rapidjson::kObjectType);
        instanceKeyJson.AddMember("ECClassId", AsJson(ctx, instanceKey.GetClass()->GetId(), &navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
        instanceKeyJson.AddMember("ECInstanceId", AsJson(ctx, instanceKey.GetId(), &navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
        keysJson.PushBack(instanceKeyJson, navNodeKeyBaseJson.GetAllocator());
        }
    navNodeKeyBaseJson.AddMember("InstanceKeys", keysJson, navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstancesNodeKeyPtr DefaultECPresentationSerializer::_GetECInstanceNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const
    {
    bvector<ECClassInstanceKey> instanceKeys;
    if (json.isMember("InstanceKeys") && json["InstanceKeys"].isArray())
        {
        BeJsConst instanceKeysJson = json["InstanceKeys"];
        for (Json::ArrayIndex i = 0; i < instanceKeysJson.size(); ++i)
            {
            ECClassId classId(instanceKeysJson[i]["ECClassId"].GetUInt64());
            ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
            ECInstanceId instanceId(instanceKeysJson[i]["ECInstanceId"].GetUInt64());
            instanceKeys.push_back(ECClassInstanceKey(ecClass, instanceId));
            }
        }
    Utf8CP specificationIdentifier = json["SpecificationIdentifier"].asCString();
    return ECInstancesNodeKey::Create(instanceKeys, specificationIdentifier, ParseNodeKeyHashPath(json["PathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("GroupedInstancesCount", groupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("ECClassId", rapidjson::Value(groupingNodeKey.GetECClassId().ToString().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const
    {
    uint64_t groupedInstancesCount = json["GroupedInstancesCount"].GetUInt64();
    Utf8CP specificationIdentifier = json["SpecificationIdentifier"].asCString();
    ECClassId classId(json["ECClassId"].GetUInt64());
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
    bool isPolymorphic = json["IsPolymorphic"].asBool(false);
    return ECClassGroupingNodeKey::Create(*ecClass, isPolymorphic, specificationIdentifier, ParseNodeKeyHashPath(json["PathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("GroupedInstancesCount", propertyGroupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("ECClassId", rapidjson::Value(propertyGroupingNodeKey.GetECClassId().ToString().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("PropertyName", rapidjson::Value(propertyGroupingNodeKey.GetPropertyName().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("GroupingValues", rapidjson::Value(propertyGroupingNodeKey.GetGroupingValuesArray(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const
    {
    uint64_t groupedInstancesCount = json["GroupedInstancesCount"].GetUInt64();
    Utf8CP specificationIdentifier = json["SpecificationIdentifier"].asCString();
    ECClassId classId(json["ECClassId"].GetUInt64());
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(classId);
    Utf8CP propertyName = json["PropertyName"].asCString();
    rapidjson::Document groupingValues;
    json["GroupingValues"].SaveTo(BeJsValue(groupingValues, groupingValues.GetAllocator()));
    return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, groupingValues, specificationIdentifier, ParseNodeKeyHashPath(json["PathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultECPresentationSerializer::_AsJson(ContextR ctx, LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("GroupedInstancesCount", labelGroupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("Label", rapidjson::Value(labelGroupingNodeKey.GetLabel().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LabelGroupingNodeKeyPtr DefaultECPresentationSerializer::_GetLabelGroupingNodeKeyFromJson(BeJsConst json) const
    {
    uint64_t groupedInstancesCount = json["GroupedInstancesCount"].GetUInt64();
    Utf8CP specificationIdentifier = json["SpecificationIdentifier"].asCString();
    Utf8CP label = json["Label"].asCString();
    return LabelGroupingNodeKey::Create(label, specificationIdentifier, ParseNodeKeyHashPath(json["PathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, NavNode const& navNode, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    Utf8String nodeId = ValueHelpers::GuidToString(navNode.GetNodeId());
    json.AddMember("NodeId", rapidjson::Value(nodeId.c_str(), json.GetAllocator()), json.GetAllocator());
    if (navNode.GetKey().IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Serialization, LOG_ERROR, "Attempting to serialize a node with NULL key");
        json.AddMember("Key", NavNodeKey::Create("", "", bvector<Utf8String>())->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
        }
    else
        {
        json.AddMember("Key", navNode.GetKey()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
        }
    json.AddMember("Description", rapidjson::Value(navNode.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ImageId", rapidjson::Value(navNode.GetImageId().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("ForeColor", rapidjson::Value(navNode.GetForeColor().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("BackColor", rapidjson::Value(navNode.GetBackColor().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("FontStyle", rapidjson::Value(navNode.GetFontStyle().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Type", rapidjson::Value(navNode.GetType().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("HasChildren", navNode.HasChildren(), json.GetAllocator());
    json.AddMember("IsChecked", navNode.IsChecked(), json.GetAllocator());
    json.AddMember("IsCheckboxVisible", navNode.IsCheckboxVisible(), json.GetAllocator());
    json.AddMember("IsCheckboxEnabled", navNode.IsCheckboxEnabled(), json.GetAllocator());
    json.AddMember("IsExpanded", navNode.ShouldAutoExpand(), json.GetAllocator());
    if (navNode.GetUsersExtendedData().GetJson().MemberCount() > 0)
        json.AddMember("ExtendedData", rapidjson::Value(navNode.GetUsersExtendedData().GetJson(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("LabelDefinition", _AsJson(ctx, navNode.GetLabelDefinition(), &json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, NodesPathElement const& navNodesPathElement,
    rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    if (navNodesPathElement.GetNode().IsNull())
        return json;

    json.SetObject();
    json.AddMember("Node", navNodesPathElement.GetNode()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("Index", (uint64_t)navNodesPathElement.GetIndex(), json.GetAllocator());
    json.AddMember("IsMarked", navNodesPathElement.IsMarked(), json.GetAllocator());

    rapidjson::Value childrenJson;
    childrenJson.SetArray();
    for (NodesPathElement const& child : navNodesPathElement.GetChildren())
        childrenJson.PushBack(child.AsJson(ctx, &json.GetAllocator()), json.GetAllocator());

    json.AddMember("Children", childrenJson, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, LabelDefinition const& labelDefinition, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    if (!labelDefinition.IsDefinitionValid())
        return json;

    json.AddMember("DisplayValue", rapidjson::Value(labelDefinition.GetDisplayValue().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("TypeName", rapidjson::Value(labelDefinition.GetTypeName().c_str(), json.GetAllocator()), json.GetAllocator());
    if (nullptr != labelDefinition.GetRawValue())
        json.AddMember("RawValue", labelDefinition.GetRawValue()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, LabelDefinition::SimpleRawValue const& value, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.CopyFrom(value.GetValue(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, LabelDefinition::CompositeRawValue const& value, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Separator", rapidjson::Value(value.GetSeparator().c_str(), json.GetAllocator()), json.GetAllocator());
    rapidjson::Value values(rapidjson::kArrayType);
    for (LabelDefinitionCPtr labelValue : value.GetValues())
        values.PushBack(labelValue->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());

    json.AddMember("Values", values, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultECPresentationSerializer::_AsJson(ContextR ctx, KeySet const& keySet, rapidjson::Document::AllocatorType* allocator) const
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
        instances.AddMember(AsJson(ctx, ecClass->GetId(), &json.GetAllocator()), instanceIds, json.GetAllocator());
        }

    rapidjson::Value nodeKeys(rapidjson::kArrayType);
    for (NavNodeKeyCPtr const& key : keySet.GetNavNodeKeys())
        nodeKeys.PushBack(key->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());

    json.AddMember("InstanceKeys", instances, json.GetAllocator());
    json.AddMember("NodeKeys", nodeKeys, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr DefaultECPresentationSerializer::_GetKeySetFromJson(IConnectionCR connection, BeJsConst json) const
    {
    InstanceKeyMap instanceKeys;
    json["InstanceKeys"].ForEachProperty([&](Utf8CP classId, BeJsConst instanceIdsJson)
        {
        ECClassId ecClassId;
        ECClassId::FromString(ecClassId, classId);
        ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(ecClassId);
        if (nullptr == ecClass)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Serialization, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Instance key contains an invalid ECClass ID: '%s'", classId));
            return false;
            }
        bset<ECInstanceId> instanceIdSet;
        instanceIdsJson.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst instanceIdJson)
            {
            instanceIdSet.insert(ECInstanceId(instanceIdJson.GetUInt64()));
            return false;
            });
        instanceKeys[ecClass] = instanceIdSet;
        return false;
        });
    
    NavNodeKeySet nodeKeys;
    json["NodeKeys"].ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst nodeKeyJson)
        {
        nodeKeys.insert(_GetNavNodeKeyFromJson(connection, nodeKeyJson));
        return false;
        });

    return KeySet::Create(instanceKeys, nodeKeys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(ContextR ctx, ECEnumerationCR enumeration,
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(ContextR ctx, KindOfQuantityCR koq,
    rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("Name", rapidjson::StringRef(koq.GetFullName().c_str()), allocator);
    json.AddMember("DisplayLabel", rapidjson::StringRef(koq.GetDisplayLabel().c_str()), allocator);
    json.AddMember("PersistenceUnit", rapidjson::Value(koq.GetPersistenceUnit()->GetName().c_str(), allocator), allocator);

    IECPropertyFormatter const* formatter = ctx.GetPropertyFormatter();
    if (formatter == nullptr)
        return json;

    Formatting::Format const* format = formatter->GetActiveFormat(koq, ctx.GetUnitSystem());
    rapidjson::Document formatJson(rapidjson::kObjectType, &allocator);
    if (nullptr != format && format->ToJson(formatJson, false))
        json.AddMember("ActiveFormat", formatJson, allocator);

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(ContextR ctx, RelatedClassCR relatedClass,
    rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("SourceClassInfo", _AsJson(ctx, *relatedClass.GetSourceClass(), &allocator), allocator);
    json.AddMember("TargetClassInfo", _AsJson(ctx, relatedClass.GetTargetClass().GetClass(), &allocator), allocator);
    json.AddMember("IsTargetPolymorphic", relatedClass.GetTargetClass().IsSelectPolymorphic(), allocator);
    json.AddMember("RelationshipInfo", _AsJson(ctx, relatedClass.GetRelationship().GetClass(), &allocator), allocator);
    json.AddMember("IsRelationshipPolymorphic", relatedClass.GetRelationship().IsSelectPolymorphic(), allocator);
    json.AddMember("IsRelationshipForward", relatedClass.IsForwardRelationship(), allocator);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(ContextR ctx, RelatedClassPathCR path,
    rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (RelatedClass const& relatedClass : path)
        {
        if (!relatedClass.IsValid())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, "Attempting to serialize related class path with invalid steps");

        json.PushBack(_AsJson(ctx, relatedClass, allocator), allocator);
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value DefaultECPresentationSerializer::_AsJson(ContextR ctx, SelectionInfo const& selectionInfo,
    rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value info(rapidjson::kObjectType);
    info.AddMember("SelectionProvider", rapidjson::StringRef(selectionInfo.GetSelectionProviderName().c_str()), allocator);
    info.AddMember("IsSubSelection", selectionInfo.IsSubSelection(), allocator);
    info.AddMember("Timestamp", rapidjson::Value(std::to_string(selectionInfo.GetTimestamp()).c_str(), allocator), allocator);
    return info;
    }
