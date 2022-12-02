/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <set>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include "../Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "../Shared/CustomizationHelper.h"
#include "ContentProviders.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::ContentProviderContext(PresentationRuleSetCR ruleset, Utf8String preferredDisplayType, int contentFlags,
    INavNodeKeysContainerCR inputKeys, std::shared_ptr<INavNodeLocater> nodesLocater, IPropertyCategorySupplierCR categorySupplier,
    std::unique_ptr<RulesetVariables> rulesetVariables, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache,
    NavNodesFactory const& nodesFactory, IJsonLocalState const* localState)
    : RulesDrivenProviderContext(ruleset, std::move(rulesetVariables), ecexpressionsCache, relatedPathsCache, nodesFactory, localState),
    m_preferredDisplayType(preferredDisplayType), m_contentFlags(contentFlags), m_nodesLocater(nodesLocater), m_categorySupplier(categorySupplier), m_inputNodeKeys(&inputKeys)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::ContentProviderContext(ContentProviderContextCR other)
    : RulesDrivenProviderContext(other), m_preferredDisplayType(other.m_preferredDisplayType), m_nodesLocater(other.m_nodesLocater),
    m_categorySupplier(other.m_categorySupplier), m_inputNodeKeys(other.m_inputNodeKeys), m_contentFlags(other.m_contentFlags)
    {
    Init();

    if (other.IsSelectionContext())
        SetSelectionInfo(other);

    if (other.IsQueryContext())
        SetQueryContext(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::Init()
    {
    m_isSelectionContext = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::~ContentProviderContext()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetSelectionInfo(SelectionInfoCR selectionInfo)
    {
    m_isSelectionContext = true;
    m_selectionInfo = &selectionInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetSelectionInfo(ContentProviderContextCR other)
    {
    m_isSelectionContext = true;
    m_selectionInfo = other.m_selectionInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RulesetVariableEntry> ContentProviderContext::GetRelatedRulesetVariables() const
    {
    bset<Utf8String> ids = RulesDrivenProviderContext::GetRelatedVariablesIds();
    bvector<RulesetVariableEntry> idsWithValues;
    for (Utf8StringCR id : ids)
        idsWithValues.push_back(RulesetVariableEntry(id, GetRulesetVariables().GetJsonValue(id.c_str())));

    return idsWithValues;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProvider::SpecificationContentProvider(ContentProviderContextR context, ContentRuleInstanceKeysContainer ruleSpecs)
    : ContentProvider(context), m_rules(ruleSpecs)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProvider::SpecificationContentProvider(SpecificationContentProviderCR other)
    : ContentProvider(other), m_rules(other.m_rules)
    {
    m_descriptor = other.m_descriptor;
    if (other.m_queries)
        m_queries = std::make_unique<ContentQuerySet>(*other.m_queries);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SpecificationContentProvider::_OnDescriptorChanged()
    {
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Descriptor changed - invalidate queries and caches");
    m_queries = nullptr;
    m_distinctValuesCache.clear();
    ContentProvider::_OnDescriptorChanged();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class ContentRequest
    {
    Values,
    DisplayValues
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int GetSerializationFlags(bool isRelated, bool isMerged, ContentRequest req)
    {
    if (!isRelated)
        {
        switch (req)
            {
            case ContentRequest::Values: return ContentSetItem::SERIALIZE_Values;
            case ContentRequest::DisplayValues: return ContentSetItem::SERIALIZE_DisplayValues;
            default: DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Unrecognized type of request for content serialization flags: %d", (int)req));
            }
        }

    if (ContentRequest::DisplayValues == req)
        return ContentSetItem::SERIALIZE_DisplayValues;

    return ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values | ContentSetItem::SERIALIZE_DisplayValues | ContentSetItem::SERIALIZE_MergedFieldNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document GetNestedContent(NestedContentProviderCR provider, ContentRequest req,
    bool isMergedContent,bool isRelatedContent, rapidjson::Document::AllocatorType* allocator = nullptr)
    {
    int serializationFlags = GetSerializationFlags(isRelatedContent, isMergedContent, req);

    rapidjson::Document json(allocator);
    json.SetArray();

    size_t index = 0;
    ContentSetItemPtr item;
    while (provider.GetContentSetItem(item, index++))
        {
        if (!isRelatedContent && ContentRequest::Values == req)
            json.CopyFrom(item->GetValues()[provider.GetContentField().GetUniqueName().c_str()], json.GetAllocator());
        else if (!isRelatedContent && ContentRequest::DisplayValues == req)
            json.CopyFrom(item->GetDisplayValues()[provider.GetContentField().GetUniqueName().c_str()], json.GetAllocator());
        else
            json.PushBack(item->AsJson(serializationFlags, &json.GetAllocator()), json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ContentSetItemPtr> GetNestedContentSetItems(NestedContentProviderCR provider)
    {
    bvector<ContentSetItemPtr> contentSetItems;

    size_t index = 0;
    ContentSetItemPtr item;
    while (provider.GetContentSetItem(item, index++))
        contentSetItems.push_back(item);

    return contentSetItems;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergePrimaryKeys(bvector<ContentSetItemPtr> const& targetSetItems, bvector<ContentSetItemPtr> const& sourceSetItems)
    {
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Content, targetSetItems.size() == sourceSetItems.size(),
        Utf8PrintfString("Expecting target and source sets to be of equal size. Actual %" PRIu64 " != %" PRIu64, targetSetItems.size(), sourceSetItems.size()));

    for (size_t i = 0; i < sourceSetItems.size() && i < targetSetItems.size(); ++i)
        {
        bvector<ECClassInstanceKey>& sourceKeys = sourceSetItems[i]->GetKeys();
        bvector<ECClassInstanceKey>& targetKeys = targetSetItems[i]->GetKeys();
        for (size_t j = 0; j < sourceKeys.size(); ++j)
            targetKeys.push_back(sourceKeys[j]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergeField(RapidJsonValueR targetValue, RapidJsonValueR targetDisplayValue,
    rapidjson::Document::AllocatorType& targetDisplayValueAllocator)
    {
    targetValue.SetNull();
    static Utf8String const variesLabel = Utf8PrintfString(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES).c_str();
    targetDisplayValue.SetString(variesLabel.c_str(), targetDisplayValueAllocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MergeContent(RapidJsonValueR targetValues, RapidJsonValueR targetDisplayValues,
    rapidjson::Document::AllocatorType& targetDisplayValuesAllocator, RapidJsonValueCR source)
    {
    if (targetValues != source)
        {
        // values are different - set the "varies" string
        MergeField(targetValues, targetDisplayValues, targetDisplayValuesAllocator);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MergeContent(rapidjson::Document& targetValues, rapidjson::Document& targetDisplayValues, RapidJsonValueCR source)
    {
    return MergeContent(targetValues, targetDisplayValues, targetDisplayValues.GetAllocator(), source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MergeContentSetItems(bvector<ContentSetItemPtr> const& targetSetItems, bvector<ContentSetItemPtr> const& sourceSetItems)
    {
    if (targetSetItems.size() != sourceSetItems.size())
        {
        // values are different - set the "varies" string
        return true;
        }

    for (size_t i = 0; i < targetSetItems.size(); ++i)
        {
        RapidJsonValueR lhsValues = targetSetItems[i]->GetValues();
        RapidJsonValueCR rhsValues = sourceSetItems[i]->GetValues();
        if (lhsValues != rhsValues)
            {
            if (1 < targetSetItems.size())
                {
                // values are only merged if there is one item in contentSetItems vector
                return true;
                }

            if (!lhsValues.IsObject() || !rhsValues.IsObject())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Both LHS and RHS should be JSON objects but are not");

            RapidJsonDocumentR lhsDisplayValues = targetSetItems[i]->GetDisplayValues();
            for (rapidjson::Value::ConstMemberIterator iterator = lhsValues.MemberBegin(); iterator != lhsValues.MemberEnd(); ++iterator)
                {
                Utf8CP fieldName = iterator->name.GetString();
                if (!lhsValues.HasMember(fieldName) || !rhsValues.HasMember(fieldName))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Either LHS or RHS doesn't have a value for: '%s'", fieldName));

                if (!targetSetItems[i]->IsMerged(fieldName) && lhsValues[fieldName] != rhsValues[fieldName])
                    {
                    targetSetItems[i]->GetMergedFieldNames().push_back(fieldName);
                    MergeField(lhsValues[fieldName], lhsDisplayValues[fieldName], lhsDisplayValues.GetAllocator());
                    }
                }
            }
        }

    MergePrimaryKeys(targetSetItems, sourceSetItems);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NestedContentProviderPtr ContentProvider::GetNestedContentProvider(ContentDescriptor::NestedContentField const& field, bool cacheable) const
    {
    auto scope = Diagnostics::Scope::Create("Get nested content provider");
    if (cacheable)
        {
        auto iter = m_nestedContentProviders.find(&field);
        if (m_nestedContentProviders.end() != iter)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Return provider from cache.");
            return iter->second;
            }
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Provider not found in cache.");
        }

    ContentProviderContextPtr context = ContentProviderContext::Create(GetContext());
    NestedContentProviderPtr provider = NestedContentProvider::Create(*context, field);
    if (cacheable)
        {
        m_nestedContentProviders.Insert(&field, provider);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Put provider to cache.");
        }
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::LoadNestedContentFieldValue(ContentSetItemR item, ContentDescriptor::NestedContentField const& field, bool cacheable) const
    {
    ContentDescriptorCR descriptor = *GetContentDescriptor();
    Utf8CP fieldName = field.GetUniqueName().c_str();
    bool isRelatedContent = (nullptr != field.AsRelatedContentField());

    NestedContentProviderPtr provider = GetNestedContentProvider(field, cacheable);
    provider->SetIsResultsMerged(descriptor.MergeResults());

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Want results merging: %s", descriptor.MergeResults() ? "TRUE" : "FALSE"));
    if (descriptor.MergeResults())
        {
        // if records are merged, have to query nested content for each merged record individually
        // and merge them one after one
        rapidjson::Document contentValues, contentDisplayValues;
        bvector<ContentSetItemPtr> targetSetitems;
        bool mergeAllField = false;
        bool firstPass = true;
        for (ECClassInstanceKeyCR key : item.GetKeys())
            {
            if (!isRelatedContent && &field.GetContentClass() != key.GetClass())
                continue;

            auto keyScope = Diagnostics::Scope::Create(Utf8PrintfString("Load content for item key: %s", DiagnosticsHelpers::CreateECInstanceKeyStr(key).c_str()));

            provider->SetPrimaryInstanceKey(key);
            if (!isRelatedContent)
                {
                rapidjson::Document instanceValues = GetNestedContent(*provider, ContentRequest::Values, true, isRelatedContent);
                rapidjson::Document instanceDisplayValues = GetNestedContent(*provider, ContentRequest::DisplayValues, true, isRelatedContent);
                if (firstPass)
                    {
                    // first pass - save the instance content
                    contentValues = std::move(instanceValues);
                    contentDisplayValues = std::move(instanceDisplayValues);
                    firstPass = false;
                    }
                else if (MergeContent(contentValues, contentDisplayValues, instanceValues))
                    {
                    // if detected different values during merge, stop
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Detected different composite content values.");
                    if (item.GetValues().HasMember(fieldName))
                        item.GetValues()[fieldName].CopyFrom(contentValues, item.GetValues().GetAllocator());
                    if (item.GetDisplayValues().HasMember(fieldName))
                        item.GetDisplayValues()[fieldName].CopyFrom(contentDisplayValues, item.GetDisplayValues().GetAllocator());
                    item.GetMergedFieldNames().push_back(fieldName);
                    break;
                    }
                }
            else
                {
                bvector<ContentSetItemPtr> sourceSetItems = GetNestedContentSetItems(*provider);
                if (firstPass)
                    {
                    // first pass
                    targetSetitems = sourceSetItems;
                    firstPass = false;
                    }
                else if (MergeContentSetItems(targetSetitems, sourceSetItems))
                    {
                    // if detected different values during merge, stop
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Detected different related content values. Going to merge the whole field.");
                    mergeAllField = true;
                    item.GetMergedFieldNames().push_back(fieldName);
                    break;
                    }
                }
            }

        if (isRelatedContent)
            {
            if (mergeAllField)
                {
                MergeField(contentValues, contentDisplayValues, contentDisplayValues.GetAllocator());
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Merged the whole related content field value.");
                }
            else
                {
                int serializationFlags = GetSerializationFlags(isRelatedContent, true, ContentRequest::Values);

                contentValues.SetArray();
                for (size_t i = 0; i < targetSetitems.size(); ++i)
                    contentValues.PushBack(targetSetitems[i]->AsJson(serializationFlags, &contentValues.GetAllocator()), contentValues.GetAllocator());

                contentDisplayValues.CopyFrom(contentValues, contentDisplayValues.GetAllocator());
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Loaded related content values.");
                }
            }

        if (item.GetDisplayValues().HasMember(fieldName) || item.GetValues().HasMember(fieldName))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Successfully loaded nested content.");
            if (item.GetMergedFieldNames().end() == std::find(item.GetMergedFieldNames().begin(), item.GetMergedFieldNames().end(), fieldName))
                {
                // note: only need to do this if the field is not yet set as merged
                RapidJsonValueR values = item.GetValues()[fieldName];
                RapidJsonValueR displayValues = item.GetDisplayValues()[fieldName];
                bool areValuesDifferent = MergeContent(values, displayValues, item.GetDisplayValues().GetAllocator(), contentValues);
                if (areValuesDifferent)
                    item.GetMergedFieldNames().push_back(fieldName);
                }
            }
        else
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Nested content is not loaded - add NULL values");
            item.GetDisplayValues().AddMember(rapidjson::Value(fieldName, item.GetDisplayValues().GetAllocator()),
                rapidjson::Value(contentDisplayValues, item.GetDisplayValues().GetAllocator()), item.GetDisplayValues().GetAllocator());
            item.GetValues().AddMember(rapidjson::Value(fieldName, item.GetValues().GetAllocator()),
                rapidjson::Value(contentValues, item.GetValues().GetAllocator()), item.GetValues().GetAllocator());
            }
        }
    else
        {
        // if not merging, can query nested content without any additional work afterwards
        provider->SetPrimaryInstanceKeys(item.GetKeys());
        item.GetValues().AddMember(rapidjson::Value(fieldName, item.GetValues().GetAllocator()),
            GetNestedContent(*provider, ContentRequest::Values, false, isRelatedContent, &item.GetValues().GetAllocator()),
            item.GetValues().GetAllocator());
        item.GetDisplayValues().AddMember(rapidjson::Value(fieldName, item.GetDisplayValues().GetAllocator()),
            GetNestedContent(*provider, ContentRequest::DisplayValues, false, isRelatedContent, &item.GetDisplayValues().GetAllocator()),
            item.GetDisplayValues().GetAllocator());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Loaded non-merged nested content for item: %s", DiagnosticsHelpers::CreateContentSetItemStr(item).c_str()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::LoadCompositePropertiesFieldValue(ContentSetItemR item, ContentDescriptor::ECPropertiesField const& field) const
    {
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Content, field.IsCompositePropertiesField(), Utf8PrintfString("Expecting field to be composite properties field, but it's not. Field name: '%s'", field.GetUniqueName().c_str()));

    // find field properties that're appropriate for this item
    bmap<ECClassCP, ContentDescriptor::Property const*> propertiesPerItemClass;
    if (nullptr == item.GetClass())
        {
        // item may have no class if it's created from multiple different classes (merged rows case)
        auto itemScope = Diagnostics::Scope::Create(Utf8PrintfString("Detecting merged composite content for item with %" PRIu64 " keys", (uint64_t)item.GetKeys().size()));

        bset<ECClassCP> handledClasses;
        for (ECClassInstanceKeyCR key : item.GetKeys())
            {
            if (handledClasses.end() != handledClasses.find(key.GetClass()))
                continue;

            auto classScope = Diagnostics::Scope::Create(Utf8PrintfString("Handling class `%s`", key.GetClass()->GetFullName()));

            bvector<ContentDescriptor::Property const*> const& matchingProperties = field.FindMatchingProperties(key.GetClass());
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Content, matchingProperties.size() <= 1, Utf8PrintfString("Expecting count of matching properties in field to be less or equal than 1, but got more. "
                "Actual count: %" PRIu64 ", class: '%s'", (uint64_t)matchingProperties.size(), key.GetClass() ? key.GetClass()->GetFullName() : "NULL"));

            if (matchingProperties.size() == 0)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Found no matching properties - add NULL value.");
                item.GetValues().AddMember(rapidjson::Value(field.GetUniqueName().c_str(), item.GetValues().GetAllocator()), rapidjson::Value(), item.GetValues().GetAllocator());
                item.GetDisplayValues().AddMember(rapidjson::Value(field.GetUniqueName().c_str(), item.GetDisplayValues().GetAllocator()), rapidjson::Value(), item.GetDisplayValues().GetAllocator());
                }
            else
                {
                ContentDescriptor::Property const* matchingProperty = matchingProperties.front();
                if (nullptr == matchingProperty)
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Got non-empty matching properties list, but the first item is NULL");

                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Found matching property `%s`", matchingProperty->GetProperty().GetName().c_str()));
                propertiesPerItemClass[key.GetClass()] = matchingProperty;
                }
            handledClasses.insert(key.GetClass());
            }
        }
    else
        {
        auto itemScope = Diagnostics::Scope::Create(Utf8PrintfString("Detecting composite content for item with %" PRIu64 " keys, all of `%s` class",
            (uint64_t)item.GetKeys().size(), item.GetClass()->GetFullName()));

        uint64_t contractId = ContentSetItemExtendedData(item).GetContractId();
        ContentQueryContract const* contract = _GetContentQuerySet().GetContract(contractId);
        if (nullptr == contract)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Contract is NULL");

        ContentDescriptor::Property const* matchingProperty = contract->FindMatchingProperty(field, item.GetClass());
        if (nullptr == matchingProperty)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Found no matching properties - add NULL value.");
            item.GetValues().AddMember(rapidjson::Value(field.GetUniqueName().c_str(), item.GetValues().GetAllocator()), rapidjson::Value(), item.GetValues().GetAllocator());
            item.GetDisplayValues().AddMember(rapidjson::Value(field.GetUniqueName().c_str(), item.GetDisplayValues().GetAllocator()), rapidjson::Value(), item.GetDisplayValues().GetAllocator());
            return;
            }

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Found matching property `%s`", matchingProperty->GetProperty().GetName().c_str()));
        propertiesPerItemClass[item.GetClass()] = matchingProperty;
        }

    for (auto pair : propertiesPerItemClass)
        {
        ECClassCP itemClass = pair.first;
        ContentDescriptor::Property const* matchingProperty = pair.second;
        auto loadValueScope = Diagnostics::Scope::Create(Utf8PrintfString("Loading composite content `%s.%s`", itemClass->GetFullName(), matchingProperty->GetProperty().GetName().c_str()));

        // create a nested content provider for it
        ContentDescriptor::ECPropertiesField* nestedField = new ContentDescriptor::ECPropertiesField(nullptr, *matchingProperty);
        nestedField->SetUniqueName(field.GetUniqueName());
        ContentDescriptor::CompositeContentField nestingField(field.GetCategory(), field.GetLabel(),
            *itemClass, matchingProperty->GetPrefix(), {nestedField}, false, field.GetPriority());
        nestingField.SetUniqueName(field.GetUniqueName());

        // get the nested content
        LoadNestedContentFieldValue(item, nestingField, false);

        // don't repeat if detected different (merged) values
        if (item.IsMerged(field.GetUniqueName()))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Detected different values - stop loading.");
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::LoadNestedContent(ContentSetItemR item, bvector<ContentDescriptor::Field*> const& fields) const
    {
    for (ContentDescriptor::Field const* field : fields)
        {
        if (field->IsNestedContentField())
            {
            auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle nested content field `%s`", field->GetUniqueName().c_str()));
            ContentDescriptor::RelatedContentField const* relatedContentField = field->AsNestedContentField()->AsRelatedContentField();
            if (relatedContentField && item.GetClass() && !item.GetClass()->Is(relatedContentField->GetPathFromSelectToContentClass().front().GetSourceClass()))
                {
                // do not attempt to load related content for related content fields that don't match current item
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("The field targets class `%s` and content item targets `%s` - skip.",
                    relatedContentField->GetPathFromSelectToContentClass().front().GetSourceClass()->GetFullName(), item.GetClass()->GetFullName()));
                continue;
                }
            if (item.GetValues().HasMember(field->GetUniqueName().c_str()))
                {
                // already loaded as json
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Field value already loaded - skip");
                continue;
                }
            auto nestedContentFieldIter = item.GetNestedContent().find(field->GetUniqueName().c_str());
            if (item.GetNestedContent().end() == nestedContentFieldIter)
                {
                // nested content is not loaded at all - do that
                auto loadScope = Diagnostics::Scope::Create("Load nested content");
                LoadNestedContentFieldValue(item, *field->AsNestedContentField(), true);
                }
            else
                {
                // nested content is loaded - need to ensure that all deeply nested content is loaded fully as well
                auto nestedScope = Diagnostics::Scope::Create("Ensure deeply nested content is loaded");
                for (auto const& nestedContentItem : nestedContentFieldIter->second)
                    LoadNestedContent(*nestedContentItem, field->AsNestedContentField()->GetFields());
                }
            continue;
            }
        if (field->IsPropertiesField() && field->AsPropertiesField()->IsCompositePropertiesField() && !item.GetValues().HasMember(field->GetUniqueName().c_str()))
            {
            auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle composite properties field `%s`", field->GetUniqueName().c_str()));
            LoadCompositePropertiesFieldValue(item, *field->AsPropertiesField());
            continue;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::LoadNestedContent(ContentSetItemR item) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Load nested content for %s", DiagnosticsHelpers::CreateContentSetItemStr(item).c_str()));

    ContentDescriptorCP descriptor = GetContentDescriptor();
    if (nullptr == descriptor)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "No descriptor - nothing to load.");
        return;
        }

    if (descriptor->HasContentFlag(ContentFlags::KeysOnly) || descriptor->HasContentFlag(ContentFlags::NoFields))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Descriptor's flags set to omit fields - nothing to load.");
        return;
        }

    LoadNestedContent(item, descriptor->GetAllFields());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DescriptorBuilder : ContentSpecificationsVisitor
{
private:
    std::unique_ptr<ContentDescriptorBuilder::Context> m_context;
    std::unique_ptr<ContentDescriptorBuilder> m_descriptorBuilder;
    std::unique_ptr<CustomFunctionsContext> m_functionsContext;
    ContentDescriptorPtr m_descriptor;

protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool _VisitImplementation(SelectedNodeInstancesSpecificationCR specification) override
        {
        if (nullptr == GetInput())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Specification input keys are not set");

        ContentDescriptorPtr specificationDescriptor = m_descriptorBuilder->CreateDescriptor(specification, *GetInput());
        if (specificationDescriptor.IsValid())
            {
            QueryBuilderHelpers::Aggregate(m_descriptor, *specificationDescriptor);
            return true;
            }

        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, "SelectedNodeInstances content specification did not result in any query");
        return false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool _VisitImplementation(ContentInstancesOfSpecificClassesSpecificationCR specification) override
        {
        ContentDescriptorPtr specificationDescriptor = m_descriptorBuilder->CreateDescriptor(specification);
        if (specificationDescriptor.IsValid())
            {
            QueryBuilderHelpers::Aggregate(m_descriptor, *specificationDescriptor);
            return true;
            }

        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, "ContentInstancesOfSpecificClasses content specification did not result in any query");
        return false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool _VisitImplementation(ContentRelatedInstancesSpecificationCR specification) override
        {
        if (nullptr == GetInput())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Specification input keys are not set");

        ContentDescriptorPtr specificationDescriptor = m_descriptorBuilder->CreateDescriptor(specification, *GetInput());
        if (specificationDescriptor.IsValid())
            {
            QueryBuilderHelpers::Aggregate(m_descriptor, *specificationDescriptor);
            return true;
            }

        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, "ContentRelatedInstances content specification did not result in any query");
        return false;
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    DescriptorBuilder(ContentProviderContextCR context)
        : ContentSpecificationsVisitor()
        {
        IECPropertyFormatter const* formatter = context.IsPropertyFormattingContext() ? &context.GetECPropertyFormatter() : nullptr;
        ECPresentation::UnitSystem unitSystem = context.IsPropertyFormattingContext() ? context.GetUnitSystem() : ECPresentation::UnitSystem::Undefined;

        m_context = std::make_unique<ContentDescriptorBuilder::Context>(context.GetSchemaHelper(), context.GetConnections(), context.GetConnection(), &context.GetCancelationToken(),
            context.GetRulesPreprocessor(), context.GetRuleset(), context.GetPreferredDisplayType().c_str(), context.GetRulesetVariables(), context.GetCategorySupplier(), formatter, unitSystem,
            context.GetInputKeys(), context.GetSelectionInfo());
        m_context->SetContentFlagsCalculator([flags = context.GetContentFlags()](int defaultFlags){return flags | defaultFlags;});
        m_descriptorBuilder = std::make_unique<ContentDescriptorBuilder>(*m_context);
        m_functionsContext = std::make_unique<CustomFunctionsContext>(context.GetSchemaHelper(), context.GetConnections(), context.GetConnection(),
            context.GetRuleset().GetRuleSetId(), context.GetRulesPreprocessor(), context.GetRulesetVariables(), &context.GetUsedVariablesListener(),
            context.GetECExpressionsCache(), context.GetNodesFactory(), nullptr, nullptr, nullptr, formatter, unitSystem);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    ContentDescriptorCPtr GetDescriptor() {return m_descriptor;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBuilder : ContentSpecificationsVisitor
{
private:
    std::unique_ptr<MultiContentQueryBuilder> m_queryBuilder;

private:
    static void CollectMatchingNavigationPropertyClassAliases(bset<Utf8String>& aliases, bvector<RelatedClass> const& allClasses, bvector<ContentDescriptor::Property> const& properties)
        {
        for (RelatedClassCR rc : allClasses)
            {
            if (ContainerHelpers::Contains(properties, [&rc](auto const& prop){return prop.GetProperty().GetIsNavigation() && prop.GetPropertyClass().Is(&rc.GetTargetClass().GetClass());}))
                aliases.insert(rc.GetTargetClass().GetAlias());
            }
        }
    static void CollectRelatedContentFieldClassAliases(bset<Utf8String>& aliases, ContentDescriptor::RelatedContentField const& field)
        {
        QueryBuilderHelpers::CollectRelatedClassPathAliases(aliases, field.GetPathFromSelectToContentClass());
        for (auto const& nestedField : field.GetFields())
            {
            if (nestedField->IsNestedContentField() && nestedField->AsNestedContentField()->AsRelatedContentField())
                CollectRelatedContentFieldClassAliases(aliases, *nestedField->AsNestedContentField()->AsRelatedContentField());
            }
        }
    static bool AnySelectClassExceedsRelatedClassesThreshold(bvector<SelectClassInfo> const& selectClasses, bvector<ContentDescriptor::Field*> const& fields)
        {
        for (SelectClassInfo const& selectClass : selectClasses)
            {
            bset<Utf8String> uniqueAliases;
            QueryBuilderHelpers::CollectRelatedClassPathAliases(uniqueAliases, selectClass.GetPathFromInputToSelectClass());
            QueryBuilderHelpers::CollectRelatedClassPathAliases(uniqueAliases, selectClass.GetRelatedInstancePaths());

            for (auto const& field : fields)
                {
                if (field->IsPropertiesField())
                    CollectMatchingNavigationPropertyClassAliases(uniqueAliases, selectClass.GetNavigationPropertyClasses(), field->AsPropertiesField()->GetProperties());
                else if (field->IsNestedContentField() && field->AsNestedContentField()->AsRelatedContentField())
                    CollectRelatedContentFieldClassAliases(uniqueAliases, *field->AsNestedContentField()->AsRelatedContentField());
                }

            if (uniqueAliases.size() > RELATED_CLASS_PATH_ALIASES_THRESHOLD)
                return true;
            }
        return false;
        }

protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool _VisitImplementation(SelectedNodeInstancesSpecificationCR specification) override
        {
        if (nullptr == GetInput())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Specification input keys are not set");

        return m_queryBuilder->Accept(specification, *GetInput());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool _VisitImplementation(ContentInstancesOfSpecificClassesSpecificationCR specification) override
        {
        return m_queryBuilder->Accept(specification);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool _VisitImplementation(ContentRelatedInstancesSpecificationCR specification) override
        {
        if (nullptr == GetInput())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Specification input keys are not set");

        return m_queryBuilder->Accept(specification, *GetInput());
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    QueryBuilder(ContentProviderContextCR context, ContentDescriptorCR descriptor, bool forceDirectReadOfAllProperties = false, bool isCountQuery = false)
        : ContentSpecificationsVisitor()
        {
        IECPropertyFormatter const* formatter = context.IsPropertyFormattingContext() ? &context.GetECPropertyFormatter() : nullptr;
        bool exceedsFieldsOrRelatedClassesLimit = descriptor.GetTotalFieldsCount() > 1000 || AnySelectClassExceedsRelatedClassesThreshold(descriptor.GetSelectClasses(), descriptor.GetAllFields());
        bool canDirectlyReadXToManyRelatedContent = (forceDirectReadOfAllProperties || !exceedsFieldsOrRelatedClassesLimit) && !isCountQuery;
        ContentQueryBuilderParameters params(context.GetSchemaHelper(), context.GetConnections(), context.GetNodesLocater(), context.GetConnection(), &context.GetCancelationToken(),
            context.GetRulesPreprocessor(), context.GetRuleset(), context.GetRulesetVariables(), context.GetECExpressionsCache(), &context.GetUsedVariablesListener(),
            context.GetCategorySupplier(), true, !canDirectlyReadXToManyRelatedContent, formatter, context.GetLocalState());
        m_queryBuilder = std::make_unique<MultiContentQueryBuilder>(params, descriptor);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    ContentQuerySet const& GetQuerySet() {return m_queryBuilder->GetQuerySet();}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProvider::ContentProvider(ContentProviderContextR context)
    : m_context(&context)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProvider::ContentProvider(ContentProviderCR other)
    : m_pageOptions(other.m_pageOptions)
    {
    m_context = ContentProviderContext::Create(*other.m_context);
    if (other.m_fullContentSetSize)
        m_fullContentSetSize = std::make_unique<size_t>(*other.m_fullContentSetSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::InvalidateRecords()
    {
    BeMutexHolder lock(GetMutex());
    m_records = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::InvalidateFullContentSetSize()
    {
    BeMutexHolder lock(GetMutex());
    m_fullContentSetSize = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::InvalidateNestedContentProviders()
    {
    BeMutexHolder lock(GetMutex());
    m_nestedContentProviders.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::Adopt(IConnectionCR connection, ICancelationTokenCP cancellationToken)
    {
    GetContextR().ShallowAdopt(connection, cancellationToken);
    for (auto& entry : m_nestedContentProviders)
        entry.second->Adopt(connection, cancellationToken);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProvider::~SpecificationContentProvider()
    {
    for (auto iter : m_inputCache)
        delete iter.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void VisitRuleSpecifications(ContentSpecificationsVisitor& visitor, bmap<ContentRuleCP, IParsedInput const*>& inputCache,
    ContentProviderContextCR context, ContentRuleInstanceKeysContainer const& rules)
    {
    for (ContentRuleInstanceKeys const& rule : rules)
        {
        auto ruleScope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(rule.GetRule()).c_str()));
        DiagnosticsHelpers::ReportRule(rule.GetRule());

        IParsedInput const* input = nullptr;
        auto inputIter = inputCache.find(&rule.GetRule());
        if (inputCache.end() == inputIter)
            {
            // note: each content rule may be based on different selected nodes, so we create a different selection context for each of them
            inputIter = inputCache.Insert(&rule.GetRule(),
                new ParsedInput(rule.GetInstanceKeys(), context.GetNodesLocater(), context.GetConnection(), context.GetSchemaHelper())).first;
            }
        input = inputIter->second;

        visitor.SetCurrentInput(input);

        for (ContentSpecificationCP spec : rule.GetRule().GetSpecifications())
            spec->Accept(visitor);

        visitor.SetCurrentInput(nullptr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCP SpecificationContentProvider::_GetContentDescriptor() const
    {
    BeMutexHolder lock(GetMutex());
    if (m_descriptor.IsNull())
        {
        auto scope = Diagnostics::Scope::Create("Create content descriptor");
        DescriptorBuilder builder(GetContext());
        VisitRuleSpecifications(builder, m_inputCache, GetContext(), m_rules);
        m_descriptor = builder.GetDescriptor();
        }
    return m_descriptor.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQuerySet const& SpecificationContentProvider::_GetContentQuerySet() const
    {
    BeMutexHolder lock(GetMutex());
    if (m_queries == nullptr)
        {
        auto scope = Diagnostics::Scope::Create("Create content queries");
        m_queries = std::make_unique<ContentQuerySet>();
        ContentDescriptorCP descriptor = GetContentDescriptor();
        if (nullptr == descriptor)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Query set is empty due to NULL descriptor");
            return *m_queries;
            }

        QueryBuilder builder(GetContext(), *descriptor);
        VisitRuleSpecifications(builder, m_inputCache, GetContext(), m_rules);
        m_queries->GetQueries() = builder.GetQuerySet().GetQueries();
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Created %" PRIu64 " queries", (uint64_t)m_queries->GetQueries().size()));
        }
    return *m_queries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GenericQuerySet SpecificationContentProvider::_GetCountQuerySet() const
    {
    auto scope = Diagnostics::Scope::Create("Create content set size queries");

    ContentDescriptorPtr countDescriptor = ContentDescriptor::Create(*GetContentDescriptor());
    if (countDescriptor->GetFieldsFilterExpression().empty())
        {
        // note: can't add `KeysOnly` if there's a filter expression - we need to select properties to filter by them
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        if (!countDescriptor->OnlyDistinctValues())
#endif
            countDescriptor->AddContentFlag(ContentFlags::KeysOnly);
        }

    QueryBuilder builder(GetContext(), *countDescriptor, false, true);
    VisitRuleSpecifications(builder, m_inputCache, GetContext(), m_rules);
    GenericQuerySet querySet(ContainerHelpers::TransformContainer<bvector<GenericQueryPtr>>(builder.GetQuerySet().GetQueries(), [&](auto const& keysQuery)
        {
        Utf8CP groupingFieldName = ContentQueryContract::ECInstanceKeysFieldName;
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        if (countDescriptor->OnlyDistinctValues())
            groupingFieldName = countDescriptor->GetDistinctField()->GetUniqueName().c_str();
#endif
        RefCountedPtr<CountQueryContract> contract = CountQueryContract::Create(groupingFieldName);
        ComplexGenericQueryPtr countQuery = ComplexGenericQuery::Create();
        countQuery->SelectContract(*contract);
        countQuery->GroupByContract(*contract);
        countQuery->From(*StringGenericQuery::Create(keysQuery->ToString(), keysQuery->GetBoundValues()));
        return countQuery;
        }));
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Created %" PRIu64 " queries", (uint64_t)querySet.GetQueries().size()));
    return querySet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SpecificationContentProvider::SetContentDescriptor(ContentDescriptorCR descriptor)
    {
    BeMutexHolder lock(GetMutex());
    auto scope = Diagnostics::Scope::Create("Set content descriptor");
    if (m_descriptor.IsValid() && descriptor.Equals(*m_descriptor))
        return;

    m_descriptor = ContentDescriptor::Create(descriptor);
    _OnDescriptorChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DisplayValueGroupCPtr> SpecificationContentProvider::CreateDistinctValues(ContentDescriptor::Field const& field) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create distinct values for `%s`", field.GetUniqueName().c_str()));

    if (!GetContext().IsQueryContext())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Not a query context - return empty result.");
        return bvector<DisplayValueGroupCPtr>();
        }

    if (!field.IsPropertiesField() && !field.IsDisplayLabelField() && !field.IsCalculatedPropertyField())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Not a properties, display label or calculated field - return empty result.");
        return bvector<DisplayValueGroupCPtr>();
        }

    ContentDescriptorCP contentDescriptor = GetContentDescriptor();
    if (!contentDescriptor)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "No content descriptor - return empty result.");
        return bvector<DisplayValueGroupCPtr>();
        }

    ContentDescriptorPtr distinctValuesDescriptor = ContentDescriptor::Create(*contentDescriptor);
    distinctValuesDescriptor->ExclusivelyIncludeFields({ &field });
    distinctValuesDescriptor->IterateFields([](auto& field)
        {
        if (field.IsNestedContentField() && field.AsNestedContentField()->AsRelatedContentField())
            field.AsNestedContentField()->AsRelatedContentField()->GetPathFromSelectToContentClass().SetTargetsCount(nullptr);
        return true;
        });
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Created a new descriptor with a single field.");

    IECPropertyFormatter const* formatter = GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr;
    ECPresentation::UnitSystem unitSystem = GetContext().IsPropertyFormattingContext() ? GetContext().GetUnitSystem() : ECPresentation::UnitSystem::Undefined;
    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(),
        GetContext().GetRuleset().GetRuleSetId(), GetContext().GetRulesPreprocessor(), GetContext().GetRulesetVariables(), &GetContext().GetUsedVariablesListener(),
        GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), nullptr, nullptr, nullptr, formatter, unitSystem);

    DistinctValuesAccumulator distinctValuesAccumulator(field, GetContext().GetConnection().GetECDb().Schemas());
    if (GetContext().IsPropertyFormattingContext())
        {
        distinctValuesAccumulator.SetPropertyFormatter(GetContext().GetECPropertyFormatter());
        distinctValuesAccumulator.SetUnitSystem(GetContext().GetUnitSystem());
        }

    QueryBuilder builder(GetContext(), *distinctValuesDescriptor, true);
    VisitRuleSpecifications(builder, m_inputCache, GetContext(), m_rules);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Content is built from %" PRIu64 " queries", (uint64_t)builder.GetQuerySet().GetQueries().size()));
    for (auto const& contentQuery : builder.GetQuerySet().GetQueries())
        {
        ThrowIfCancelled(GetContext().GetCancelationToken());

        if (contentQuery.IsNull())
            continue;

        RefCountedPtr<SimpleQueryContract> contract = SimpleQueryContract::Create(
            {
            PresentationQueryContractSimpleField::Create(field.GetUniqueName().c_str(), field.GetUniqueName().c_str())
            });
        ComplexGenericQueryPtr distinctValuesQuery = ComplexGenericQuery::Create();
        distinctValuesQuery->SelectContract(*contract, "values");
        distinctValuesQuery->From(*StringGenericQuery::Create(contentQuery->ToString(), contentQuery->GetBoundValues()), "values");
        distinctValuesQuery->GroupByContract(*contract);

        auto queryScope = Diagnostics::Scope::Create("Accumulate query results");
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Query: `%s`", distinctValuesQuery->ToString().c_str()));

        QueryExecutorHelper::ExecuteQuery(GetContext().GetConnection(), *distinctValuesQuery,
            distinctValuesAccumulator, GetContext().GetCancelationToken());
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Total distinct values: %" PRIu64, (uint64_t)distinctValuesAccumulator.GetValues().size()));

    bvector<DisplayValueGroupCPtr> values;
    values.reserve(distinctValuesAccumulator.GetValues().size());
    return ContainerHelpers::TransformContainer(values, distinctValuesAccumulator.GetValues(), [](auto pair){return pair.second;});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IDataSourceCPtr<DisplayValueGroupCPtr> SpecificationContentProvider::GetDistinctValues(ContentDescriptor::Field const& field) const
    {
    BeMutexHolder lock(GetMutex());
    auto cacheIter = m_distinctValuesCache.find(field.GetUniqueName());
    if (m_distinctValuesCache.end() == cacheIter)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Distinct values not found in cache");
        bvector<DisplayValueGroupCPtr> values = CreateDistinctValues(field);
        cacheIter = m_distinctValuesCache.Insert(field.GetUniqueName(), values).first;
        }
    else
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Distinct values found in cache");
        }
    return VectorDataSource<DisplayValueGroupCPtr>::Create(cacheIter->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::SetPageOptions(PageOptions options)
    {
    BeMutexHolder lock(GetMutex());
    if (m_pageOptions.Equals(options))
        return;

    m_pageOptions = options;
    _OnPageOptionsChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::_OnDescriptorChanged()
    {
    BeMutexHolder lock(GetMutex());
    InvalidateRecords();
    InvalidateFullContentSetSize();
    InvalidateNestedContentProviders();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::_OnPageOptionsChanged()
    {
    BeMutexHolder lock(GetMutex());
    m_records = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::Initialize()
    {
    BeMutexHolder lock(GetMutex());
    if (nullptr != m_records)
        return;

    auto scope = Diagnostics::Scope::Create("Initialize content provider");

    if (!GetContext().IsQueryContext())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Not a query context - return.");
        return;
        }

    IECPropertyFormatter const* formatter = GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr;
    ECPresentation::UnitSystem unitSystem = GetContext().IsPropertyFormattingContext() ? GetContext().GetUnitSystem() : ECPresentation::UnitSystem::Undefined;
    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(),
        GetContext().GetRuleset().GetRuleSetId(), GetContext().GetRulesPreprocessor(), GetContext().GetRulesetVariables(), &GetContext().GetUsedVariablesListener(),
        GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), nullptr, nullptr, nullptr, formatter, unitSystem);

    auto const& querySet = _GetContentQuerySet();
    if (querySet.GetQueries().empty())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Empty query set - return.");
        return;
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Content is built from %" PRIu64 " queries", (uint64_t)querySet.GetQueries().size()));

    ContentDescriptorCR descriptor = querySet.GetContract()->GetDescriptor();
    ContentReader contentReader(GetContext().GetConnection().GetECDb().Schemas(), querySet);
    if (GetContext().IsPropertyFormattingContext())
        {
        contentReader.SetPropertyFormatter(GetContext().GetECPropertyFormatter());
        contentReader.SetUnitSystem(GetContext().GetUnitSystem());
        }

    size_t skipRecords = GetPageOptions().GetPageStart();
    if (skipRecords > 1)
        {
        // note: the reader lags returning the items by 1 record - need to start
        // reading 1 record before it reaches `skipRecords == 0`
        contentReader.SetMode(ContentReader::Mode::Skip);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Starting content reader in `Skip` mode.");
        }

    bvector<ContentSetItemPtr> records;
    QueryExecutor executor(GetContext().GetConnection());
    for (auto const& query : querySet.GetQueries())
        {
        auto queryScope = Diagnostics::Scope::Create("Execute query");
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Query: `%s`", query->ToString().c_str()));

        executor.SetQuery(*query);
        ContentSetItemPtr item;
        while (QueryExecutorStatus::Row == executor.ReadNext(item, contentReader) && (GetPageOptions().GetPageSize() == 0 || records.size() < GetPageOptions().GetPageSize()))
            {
            ThrowIfCancelled(GetContext().GetCancelationToken());

            if (skipRecords > 0)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Skip record. Remaining: %" PRIu64, (uint64_t)skipRecords));
                if (--skipRecords == 1)
                    {
                    contentReader.SetMode(ContentReader::Mode::Read);
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Set content reader mode to `Read`.");
                    }
                continue;
                }

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
            if (descriptor.GetDistinctField() != nullptr)
                {
                bool skipItem = false;
                for (size_t i = 0; i < records.size(); i++)
                    {
                    if (item->GetDisplayValues() == records[i]->GetDisplayValues() && item->GetDisplayLabelDefinition() == records[i]->GetDisplayLabelDefinition())
                        {
                        skipItem = true;
                        break;
                        }
                    }
                if (skipItem)
                    continue;
                }
#endif

            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Read content item: %s", DiagnosticsHelpers::CreateContentSetItemStr(*item).c_str()));
            LoadNestedContent(*item);
            records.push_back(item);
            }
        }
    m_records = std::make_unique<bvector<ContentSetItemPtr>>(std::move(records));
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Read content items: %" PRIu64, (uint64_t)m_records->size()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentProvider::GetFullContentSetSize() const
    {
    BeMutexHolder lock(GetMutex());
    if (nullptr == m_fullContentSetSize)
        {
        auto scope = Diagnostics::Scope::Create("Initialize content set size");
        if (GetContentDescriptor() && GetContentDescriptor()->MergeResults())
            {
            m_fullContentSetSize = std::make_unique<size_t>(1);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Descriptor is valid and requests merged rows. Set size to 1.");
            }
        else if (GetContext().IsQueryContext())
            {
            IECPropertyFormatter const* formatter = GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr;
            ECPresentation::UnitSystem unitSystem = GetContext().IsPropertyFormattingContext() ? GetContext().GetUnitSystem() : ECPresentation::UnitSystem::Undefined;
            CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(),
                GetContext().GetRuleset().GetRuleSetId(), GetContext().GetRulesPreprocessor(), GetContext().GetRulesetVariables(), &GetContext().GetUsedVariablesListener(),
                GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), nullptr, nullptr, nullptr, formatter, unitSystem);

            auto countQuerySet = _GetCountQuerySet();
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Content is built from %" PRIu64 " queries", (uint64_t)countQuerySet.GetQueries().size()));
            size_t fullSize = 0;
            for (auto const& query : countQuerySet.GetQueries())
                {
                if (query.IsNull())
                    continue;

                auto queryScope = Diagnostics::Scope::Create("Execute count query");
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Query: `%s`", query->ToString().c_str()));
                fullSize += (size_t)QueryExecutorHelper::ReadUInt64(GetContext().GetConnection(), *query);
                }
            m_fullContentSetSize = std::make_unique<size_t>(fullSize);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Total content set size: %" PRIu64, (uint64_t)*m_fullContentSetSize));
            }
        }
    return m_fullContentSetSize ? *m_fullContentSetSize : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentProvider::GetContentSetSize() const
    {
    BeMutexHolder lock(GetMutex());
    const_cast<ContentProviderP>(this)->Initialize();
    return m_records ? m_records->size() : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentProvider::GetContentSetItem(ContentSetItemPtr& item, size_t index) const
    {
    BeMutexHolder lock(GetMutex());
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Get content set item at %" PRIu64, (uint64_t)index));
    const_cast<ContentProviderP>(this)->Initialize();

    if (!GetContentDescriptor())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Descriptor is NULL. No item returned.");
        return false;
        }

    if (!m_records || index >= m_records->size())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Supplied index out of bounds. No item returned.");
        return false;
        }

    item = m_records->at(index);
    CustomizationHelper::Customize(GetContext(), *GetContentDescriptor(), *item);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Returning item: %s", DiagnosticsHelpers::CreateContentSetItemStr(*item).c_str()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NestedContentProvider::NestedContentProvider(ContentProviderContextR context, ContentDescriptor::NestedContentField const& field)
    : ContentProvider(context), m_field(field), m_mergedResults(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NestedContentProvider::NestedContentProvider(NestedContentProviderCR other)
    : ContentProvider(other), m_field(other.m_field)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::_OnDescriptorChanged()
    {
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Descriptor changed - invalidate queries and caches");
    m_unfilteredQueries = nullptr;
    m_queries = nullptr;
    ContentProvider::_OnDescriptorChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCP NestedContentProvider::_GetContentDescriptor() const
    {
    auto const& querySet = _GetContentQuerySet();
    auto const& contract = querySet.GetContract();
    return contract ? &contract->GetDescriptor() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQuerySet const& NestedContentProvider::_GetContentQuerySet() const
    {
    BeMutexHolder lock(GetMutex());
    auto scope = Diagnostics::Scope::Create("Create content queries");
    if (m_unfilteredQueries == nullptr)
        {
        IECPropertyFormatter const* formatter = GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr;
        ContentQueryBuilderParameters params(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetNodesLocater(), GetContext().GetConnection(), &GetContext().GetCancelationToken(),
            GetContext().GetRulesPreprocessor(), GetContext().GetRuleset(), GetContext().GetRulesetVariables(), GetContext().GetECExpressionsCache(), &GetContext().GetUsedVariablesListener(),
            GetContext().GetCategorySupplier(), false, false, formatter, GetContext().GetLocalState());
        ContentQueryBuilder queryBuilder(params);
        m_unfilteredQueries = std::make_unique<ContentQuerySet>(queryBuilder.CreateQuerySet(m_field));
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Created %" PRIu64 " unfiltered queries", (uint64_t)m_unfilteredQueries->GetQueries().size()));
        }
    if (m_queries == nullptr)
        {
        if (!m_mergedResults)
            {
            Utf8CP idFieldAlias = (nullptr != m_field.AsRelatedContentField()) ? m_field.AsRelatedContentField()->GetSelectClassAlias() : m_field.GetContentClassAlias().c_str();
            Utf8String idSelector = Utf8String("[").append(idFieldAlias).append("].[ECInstanceId]");
            bvector<ECInstanceId> ids = ContainerHelpers::TransformContainer<bvector<ECInstanceId>>(m_primaryInstanceKeys, [](ECClassInstanceKeyCR key){return key.GetId();});
            ValuesFilteringHelper idsFilteringHelper(ids);
            m_queries = std::make_unique<ContentQuerySet>(ContainerHelpers::TransformContainer<bvector<ContentQueryPtr>>(m_unfilteredQueries->GetQueries(), [&](auto const& unfilteredQuery)
                {
                ContentQueryPtr query = unfilteredQuery->Clone();
                QueryBuilderHelpers::Where(query, idsFilteringHelper.CreateWhereClause(idSelector.c_str()).c_str(), idsFilteringHelper.CreateBoundValues());
                return query;
                }));
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Created %" PRIu64 " id-filtered queries", (uint64_t)m_queries->GetQueries().size()));
            }
        else
            {
            BoundQueryValuesList bindings({ std::make_shared<BoundQueryId>(m_primaryInstanceKeys[0].GetId()) });
            Utf8CP idFieldAlias = (nullptr != m_field.AsRelatedContentField()) ? m_field.AsRelatedContentField()->GetSelectClassAlias() : m_field.GetContentClassAlias().c_str();
            Utf8String whereClause = Utf8String("[").append(idFieldAlias).append("].[ECInstanceId] = ? ");
            m_queries = std::make_unique<ContentQuerySet>(ContainerHelpers::TransformContainer<bvector<ContentQueryPtr>>(m_unfilteredQueries->GetQueries(), [&](auto const& unfilteredQuery)
                {
                ContentQueryPtr query = unfilteredQuery->Clone();
                QueryBuilderHelpers::Where(query, whereClause.c_str(), bindings);
                return query;
                }));
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Created %" PRIu64 " id-filtered merging queries", (uint64_t)m_queries->GetQueries().size()));
            }
        }
    return *m_queries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GenericQuerySet NestedContentProvider::_GetCountQuerySet() const
    {
    auto scope = Diagnostics::Scope::Create("Create content set size queries");
    auto const& contentQueries = _GetContentQuerySet();
    GenericQuerySet querySet(ContainerHelpers::TransformContainer<bvector<GenericQueryPtr>>(contentQueries.GetQueries(), [](auto const& contentQuery)
        {
        RefCountedPtr<CountQueryContract> contract = CountQueryContract::Create();
        ComplexGenericQueryPtr countQuery = ComplexGenericQuery::Create();
        countQuery->SelectContract(*contract);
        countQuery->GroupByContract(*contract);
        countQuery->From(*StringGenericQuery::Create(contentQuery->ToString(), contentQuery->GetBoundValues()));
        return countQuery;
        }));
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Created %" PRIu64 " queries", (uint64_t)querySet.GetQueries().size()));
    return querySet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::SetPrimaryInstanceKeys(bvector<ECClassInstanceKey> const& keys)
    {
    BeMutexHolder lock(GetMutex());
    InvalidateRecords();
    InvalidateFullContentSetSize();
    m_primaryInstanceKeys = keys;
    m_queries = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::SetPrimaryInstanceKey(ECClassInstanceKeyCR key)
    {
    SetPrimaryInstanceKeys({key});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::SetIsResultsMerged(bool mergedResults)
    {
    BeMutexHolder lock(GetMutex());
    InvalidateRecords();
    InvalidateFullContentSetSize();
    m_mergedResults = mergedResults;
    m_queries = nullptr;
    }
