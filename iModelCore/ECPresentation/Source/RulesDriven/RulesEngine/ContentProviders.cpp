/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "ContentProviders.h"
#include "ECExpressionContextsProvider.h"
#include "LocalizationHelper.h"
#include "LoggingHelper.h"
#include "CustomizationHelper.h"
#include "NavNodeProviders.h"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::ContentProviderContext(PresentationRuleSetCR ruleset, Utf8String locale, Utf8String preferredDisplayType, int contentFlags,
    INavNodeKeysContainerCR inputKeys, INavNodeLocaterCR nodesLocater, IPropertyCategorySupplierCR categorySupplier,
    IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache, PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache, 
    JsonNavNodesFactory const& nodesFactory, IJsonLocalState const* localState) 
    : RulesDrivenProviderContext(ruleset, locale, userSettings, ecexpressionsCache, relatedPathsCache, polymorphicallyRelatedClassesCache, nodesFactory, localState), 
    m_preferredDisplayType(preferredDisplayType), m_contentFlags(contentFlags), m_nodesLocater(nodesLocater), m_categorySupplier(categorySupplier), m_inputNodeKeys(&inputKeys)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::ContentProviderContext(ContentProviderContextCR other) 
    : RulesDrivenProviderContext(other), m_preferredDisplayType(other.m_preferredDisplayType), m_nodesLocater(other.m_nodesLocater), 
    m_categorySupplier(other.m_categorySupplier), m_inputNodeKeys(other.m_inputNodeKeys), m_contentFlags(other.m_contentFlags)
    {
    Init();
    SetUsedSettingsListener(other);

    if (other.IsSelectionContext())
        SetSelectionInfo(other);
    
    if (other.IsQueryContext())
        SetQueryContext(other);    

    SetIsNestedContent(other.IsNestedContent());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::Init()
    {
    m_isSelectionContext = false;
    m_isNestedContent = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::~ContentProviderContext()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetSelectionInfo(SelectionInfoCR selectionInfo)
    {
    BeAssert(!IsSelectionContext());
    m_isSelectionContext = true;
    m_selectionInfo = &selectionInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetSelectionInfo(ContentProviderContextCR other)
    {
    BeAssert(!IsSelectionContext());
    m_isSelectionContext = true;
    m_selectionInfo = other.m_selectionInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProvider::SpecificationContentProvider(ContentProviderContextR context, ContentRuleInstanceKeysList ruleSpecs)
    : ContentProvider(context), m_rules(ruleSpecs)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProvider::SpecificationContentProvider(SpecificationContentProviderCR other)
    : ContentProvider(other), m_rules(other.m_rules)
    {
    m_descriptor = other.m_descriptor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SpecificationContentProvider::_Reset()
    {
    m_query = nullptr;
    ContentProvider::_Reset();
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2017
+===============+===============+===============+===============+===============+======*/
enum class ContentRequest
    {
    Values,
    DisplayValues
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static int GetSerializationFlags(bool isRelated, bool isMerged, bool isNested, ContentRequest req)
    {
    if (!isRelated)
        {
        switch (req)
            {
            case ContentRequest::Values: return ContentSetItem::SERIALIZE_Values;
            case ContentRequest::DisplayValues: return ContentSetItem::SERIALIZE_DisplayValues;
            default: BeAssert(false); return ContentSetItem::SERIALIZE_All;
            }
        }
    
    if (ContentRequest::DisplayValues == req)
        return ContentSetItem::SERIALIZE_DisplayValues;

    return ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values | ContentSetItem::SERIALIZE_DisplayValues | ContentSetItem::SERIALIZE_MergedFieldNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document GetNestedContent(NestedContentProviderCR provider, ContentRequest req, 
    bool isMergedContent, bool isNestedContent, bool isRelatedContent, rapidjson::Document::AllocatorType* allocator = nullptr)
    {
    int serializationFlags = GetSerializationFlags(isRelatedContent, isMergedContent, isNestedContent, req);

    rapidjson::Document json(allocator);
    json.SetArray();

    size_t index = 0;
    ContentSetItemPtr item;
    while (provider.GetContentSetItem(item, index++))
        {
        if (!isRelatedContent && ContentRequest::Values == req)
            json.CopyFrom(item->GetValues()[provider.GetContentField().GetName().c_str()], json.GetAllocator());
        else if (!isRelatedContent && ContentRequest::DisplayValues == req)
            json.CopyFrom(item->GetDisplayValues()[provider.GetContentField().GetName().c_str()], json.GetAllocator());
        else
            json.PushBack(item->AsJson(serializationFlags, &json.GetAllocator()), json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
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
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergePrimaryKeys(bvector<ContentSetItemPtr> const& targetSetItems, bvector<ContentSetItemPtr> const& sourceSetItems)
    {
    BeAssert(targetSetItems.size() == sourceSetItems.size());
    for (size_t i = 0; i < sourceSetItems.size(); ++i)
        {
        bvector<ECClassInstanceKey>& sourceKeys = sourceSetItems[i]->GetKeys();
        bvector<ECClassInstanceKey>& targetKeys = targetSetItems[i]->GetKeys();
        for (size_t j = 0; j < sourceKeys.size(); ++j)
            targetKeys.push_back(sourceKeys[j]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetLocalizedVariesString(ILocalizationProvider const* localizationProvider, Utf8StringCR locale)
    {
    if (!localizationProvider)
        return RulesEngineL10N::LABEL_General_Varies().m_str;

    Utf8String localizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
    Utf8String prelocalizedLabel = Utf8PrintfString(CONTENTRECORD_MERGED_VALUE_FORMAT, localizationId.c_str());
    Utf8String localizedLabel = prelocalizedLabel;
    LocalizationHelper(*localizationProvider, locale).LocalizeString(localizedLabel);
    return localizedLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergeField (RapidJsonValueR targetValue, RapidJsonValueR targetDisplayValue,
    rapidjson::Document::AllocatorType& targetDisplayValueAllocator, 
    ILocalizationProvider const* localizationProvider, Utf8StringCR locale)
    {
    targetValue.SetNull();
    targetDisplayValue.SetString(GetLocalizedVariesString(localizationProvider, locale).c_str(), targetDisplayValueAllocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MergeContent(RapidJsonValueR targetValues, RapidJsonValueR targetDisplayValues,
    rapidjson::Document::AllocatorType& targetDisplayValuesAllocator, RapidJsonValueCR source,
    ILocalizationProvider const* localizationProvider, Utf8StringCR locale)
    {
    if (targetValues != source)
        {
        // values are different - set the "varies" string
        MergeField(targetValues, targetDisplayValues, targetDisplayValuesAllocator, localizationProvider, locale);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MergeContent(rapidjson::Document& targetValues, rapidjson::Document& targetDisplayValues, RapidJsonValueCR source,
    ILocalizationProvider const* localizationProvider, Utf8StringCR locale)
    {
    return MergeContent(targetValues, targetDisplayValues, targetDisplayValues.GetAllocator(), source, localizationProvider, locale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MergeContentSetItems(bvector<ContentSetItemPtr> const& targetSetItems, bvector<ContentSetItemPtr> const& sourceSetItems, 
    ILocalizationProvider const* localizationProvider, Utf8StringCR locale)
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

            BeAssert(lhsValues.IsObject() && rhsValues.IsObject());
            RapidJsonDocumentR lhsDisplayValues = targetSetItems[i]->GetDisplayValues();
            for (rapidjson::Value::ConstMemberIterator iterator = lhsValues.MemberBegin(); iterator != lhsValues.MemberEnd(); ++iterator)
                {
                Utf8CP fieldName = iterator->name.GetString();
                BeAssert(lhsValues.HasMember(fieldName) && rhsValues.HasMember(fieldName));
                if (!targetSetItems[i]->IsMerged(fieldName) && lhsValues[fieldName] != rhsValues[fieldName])
                    {
                    targetSetItems[i]->GetMergedFieldNames().push_back(fieldName);
                    MergeField(lhsValues[fieldName], lhsDisplayValues[fieldName], lhsDisplayValues.GetAllocator(), localizationProvider, locale);
                    }
                }
            }
        }

    MergePrimaryKeys(targetSetItems, sourceSetItems);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NestedContentProviderPtr ContentProvider::GetNestedContentProvider(ContentDescriptor::NestedContentField const& field, bool cacheable) const
    {
    if (cacheable)
        {
        auto iter = m_nestedContentProviders.find(&field);
        if (m_nestedContentProviders.end() != iter)
            return iter->second;
        }

    ContentProviderContextPtr context = ContentProviderContext::Create(GetContext());
    context->SetIsNestedContent(true);
    NestedContentProviderPtr provider = NestedContentProvider::Create(*context, field);
    if (cacheable)
        m_nestedContentProviders.Insert(&field, provider);
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::LoadNestedContentFieldValue(ContentSetItemR item, ContentDescriptor::NestedContentField const& field, bool cacheable) const
    {
    ILocalizationProvider const* localizationProvider = GetContext().IsLocalizationContext() ? &GetContext().GetLocalizationProvider() : nullptr;
    ContentDescriptorCR descriptor = *GetContentDescriptor();
    Utf8CP fieldName = field.GetName().c_str();
    bool isRelatedContent = !field.GetRelationshipPath().empty();
    NestedContentProviderPtr provider = GetNestedContentProvider(field, cacheable);
    provider->SetIsResultsMerged(descriptor.MergeResults());
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

            provider->SetPrimaryInstanceKey(key);
            if (!isRelatedContent)
                {
                rapidjson::Document instanceValues = GetNestedContent(*provider, ContentRequest::Values, true, GetContext().IsNestedContent(), isRelatedContent);
                rapidjson::Document instanceDisplayValues = GetNestedContent(*provider, ContentRequest::DisplayValues, true, GetContext().IsNestedContent(), isRelatedContent);
                if (firstPass)
                    {
                    // first pass - save the instance content
                    contentValues = std::move(instanceValues);
                    contentDisplayValues = std::move(instanceDisplayValues);
                    firstPass = false;
                    }
                else if (MergeContent(contentValues, contentDisplayValues, instanceValues, localizationProvider, GetContext().GetLocale()))
                    {
                    // if detected different values during merge, stop
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
                else if (MergeContentSetItems(targetSetitems, sourceSetItems, localizationProvider, GetContext().GetLocale()))
                    {
                    // if detected different values during merge, stop
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
                MergeField(contentValues, contentDisplayValues, contentDisplayValues.GetAllocator(), 
                    localizationProvider, GetContext().GetLocale());
                }
            else
                {
                int serializationFlags = GetSerializationFlags(isRelatedContent, true, GetContext().IsNestedContent(), ContentRequest::Values);

                contentValues.SetArray();
                for (size_t i = 0; i < targetSetitems.size(); ++i)
                    contentValues.PushBack(targetSetitems[i]->AsJson(serializationFlags, &contentValues.GetAllocator()), contentValues.GetAllocator());

                contentDisplayValues.CopyFrom(contentValues, contentDisplayValues.GetAllocator());
                }
            }

        if (item.GetDisplayValues().HasMember(fieldName) || item.GetValues().HasMember(fieldName))
            {
            if (item.GetMergedFieldNames().end() == std::find(item.GetMergedFieldNames().begin(), item.GetMergedFieldNames().end(), fieldName))
                {
                // note: only need to do this if the field is not yet set as merged
                RapidJsonValueR values = item.GetValues()[fieldName];
                RapidJsonValueR displayValues = item.GetDisplayValues()[fieldName];
                bool areValuesDifferent = MergeContent(values, displayValues, item.GetDisplayValues().GetAllocator(), contentValues,
                    localizationProvider, GetContext().GetLocale());
                if (areValuesDifferent)
                    item.GetMergedFieldNames().push_back(fieldName);
                }
            }
        else
            {
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
            GetNestedContent(*provider, ContentRequest::Values, false, GetContext().IsNestedContent(), isRelatedContent, &item.GetValues().GetAllocator()), 
            item.GetValues().GetAllocator());
        item.GetDisplayValues().AddMember(rapidjson::Value(fieldName, item.GetDisplayValues().GetAllocator()), 
            GetNestedContent(*provider, ContentRequest::DisplayValues, false, GetContext().IsNestedContent(), isRelatedContent, &item.GetDisplayValues().GetAllocator()), 
            item.GetDisplayValues().GetAllocator());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::LoadCompositePropertiesFieldValue(ContentSetItemR item, ContentDescriptor::ECPropertiesField const& field) const
    {
    BeAssert(field.IsCompositePropertiesField());

    // find field properties that're appropriate for this item
    bmap<ECClassCP, ContentDescriptor::Property const*> propertiesPerItemClass;
    if (nullptr == item.GetClass())
        {
        // item may have no class if it's created from multiple different classes (merged rows case)
        bset<ECClassCP> handledClasses;
        for (ECClassInstanceKeyCR key : item.GetKeys())
            {
            if (handledClasses.end() != handledClasses.find(key.GetClass()))
                continue;

            bvector<ContentDescriptor::Property const*> const& matchingProperties = field.FindMatchingProperties(key.GetClass());
            BeAssert(matchingProperties.size() <= 1);
            if (matchingProperties.size() == 0)
                {
                item.GetValues().AddMember(rapidjson::Value(field.GetName().c_str(), item.GetValues().GetAllocator()), rapidjson::Value(), item.GetValues().GetAllocator());
                item.GetDisplayValues().AddMember(rapidjson::Value(field.GetName().c_str(), item.GetDisplayValues().GetAllocator()), rapidjson::Value(), item.GetDisplayValues().GetAllocator());
                }
            else
                {
                ContentDescriptor::Property const* matchingProperty = matchingProperties.front();
                if (nullptr == matchingProperty)
                    {
                    BeAssert(false);
                    continue;
                    }
                propertiesPerItemClass[key.GetClass()] = matchingProperty;
                }
            handledClasses.insert(key.GetClass());
            }
        }
    else
        {
        uint64_t contractId = ContentSetItemExtendedData(item).GetContractId();
        ContentQueryContract const* contract = _GetQuery()->GetContract(contractId);
        if (nullptr == contract)
            {
            BeAssert(false);
            return;
            }
        ContentDescriptor::Property const* matchingProperty = contract->FindMatchingProperty(field, item.GetClass());
        if (nullptr == matchingProperty)
            {
            item.GetValues().AddMember(rapidjson::Value(field.GetName().c_str(), item.GetValues().GetAllocator()), rapidjson::Value(), item.GetValues().GetAllocator()); 
            item.GetDisplayValues().AddMember(rapidjson::Value(field.GetName().c_str(), item.GetDisplayValues().GetAllocator()), rapidjson::Value(), item.GetDisplayValues().GetAllocator());
            return;
            }
        propertiesPerItemClass[item.GetClass()] = matchingProperty;
        }

    for (auto pair : propertiesPerItemClass)
        {
        ECClassCP itemClass = pair.first;
        ContentDescriptor::Property const* matchingProperty = pair.second;

        // create a nested content provider for it
        ContentDescriptor::ECPropertiesField* nestedField = new ContentDescriptor::ECPropertiesField(ContentDescriptor::Category(), *matchingProperty);
        nestedField->SetName(field.GetName());
        ContentDescriptor::NestedContentField nestingField(field.GetCategory(), field.GetName(), field.GetLabel(),
            *itemClass, "this", RelatedClassPath(), {nestedField}, false, field.GetPriority());

        // get the nested content
        LoadNestedContentFieldValue(item, nestingField, false);

        // don't repeat if detected different (merged) values
        if (item.IsMerged(field.GetName()))
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::LoadNestedContent(ContentSetItemR item) const
    {
    ContentDescriptorCP descriptor = GetContentDescriptor();
    if (nullptr == descriptor)
        return;

    if (descriptor->HasContentFlag(ContentFlags::KeysOnly))
        return;

    for (ContentDescriptor::Field const* field : descriptor->GetAllFields())
        {
        if (field->IsNestedContentField())
            LoadNestedContentFieldValue(item, *field->AsNestedContentField(), true);
        else if (field->IsPropertiesField() && field->AsPropertiesField()->IsCompositePropertiesField() && !item.GetValues().HasMember(field->GetName().c_str()))
            LoadCompositePropertiesFieldValue(item, *field->AsPropertiesField());
        }
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsVisitor : PresentationRuleSpecificationVisitor
{
private:
    IParsedInput const* m_input;    
protected:
    IParsedInput const* GetInput() {return m_input;}
public:
    ContentSpecificationsVisitor(ContentProviderContext const& context) : m_input(nullptr) {}
    virtual ~ContentSpecificationsVisitor() {}
    void SetCurrentInput(IParsedInput const* input) {m_input = input;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct DescriptorBuilder : ContentSpecificationsVisitor
{
private:
    ContentDescriptorBuilder::Context* m_context;
    ContentDescriptorBuilder* m_descriptorBuilder;
    CustomFunctionsContext* m_functionsContext;
    ContentDescriptorPtr m_descriptor;
    
protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            07/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(SelectedNodeInstancesSpecificationCR specification) override
        {
        if (nullptr == GetInput())
            {
            BeAssert(false);
            return;
            }

        if (m_descriptor.IsValid() && specification.GetOnlyIfNotHandled())
            {
            LoggingHelper::LogMessage(Log::Content, "SelectedNodeInstances content specification was ignored because the rule is already handled", 
                NativeLogging::LOG_INFO);
            return;
            }

        ContentDescriptorPtr specificationDescriptor = m_descriptorBuilder->CreateDescriptor(specification, *GetInput());
        if (specificationDescriptor.IsValid())
            {
            QueryBuilderHelpers::Aggregate(m_descriptor, *specificationDescriptor);
            return;
            }

        LoggingHelper::LogMessage(Log::Content, "SelectedNodeInstances content specification did not result in any query", NativeLogging::LOG_DEBUG);
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            07/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(ContentInstancesOfSpecificClassesSpecificationCR specification) override
        {
        ContentDescriptorPtr specificationDescriptor = m_descriptorBuilder->CreateDescriptor(specification);
        if (specificationDescriptor.IsValid())
            {
            QueryBuilderHelpers::Aggregate(m_descriptor, *specificationDescriptor);
            return;
            }

        LoggingHelper::LogMessage(Log::Content, "ContentInstancesOfSpecificClasses content specification did not result in any query", NativeLogging::LOG_DEBUG);
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            07/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(ContentRelatedInstancesSpecificationCR specification) override 
        {
        if (nullptr == GetInput())
            {
            BeAssert(false);
            return;
            }

        ContentDescriptorPtr specificationDescriptor = m_descriptorBuilder->CreateDescriptor(specification, *GetInput());
        if (specificationDescriptor.IsValid())
            {
            QueryBuilderHelpers::Aggregate(m_descriptor, *specificationDescriptor);
            return;
            }

        LoggingHelper::LogMessage(Log::Content, "ContentRelatedInstances content specification did not result in any query", NativeLogging::LOG_DEBUG);
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            07/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    DescriptorBuilder(ContentProviderContextCR context)
        : ContentSpecificationsVisitor(context)
        {
        IECPropertyFormatter const* formatter = context.IsPropertyFormattingContext() ? &context.GetECPropertyFormatter() : nullptr;
        ILocalizationProvider const* localizationProvider = context.IsLocalizationContext() ? &context.GetLocalizationProvider() : nullptr;
        m_context = new ContentDescriptorBuilder::Context(context.GetSchemaHelper(), context.GetConnections(), context.GetConnection(), context.GetRuleset(),
            context.GetPreferredDisplayType().c_str(), context.GetContentFlags(), context.GetCategorySupplier(), formatter, localizationProvider, context.GetLocale(), 
            context.GetInputKeys(), context.GetSelectionInfo());
        m_descriptorBuilder = new ContentDescriptorBuilder(*m_context);

        m_functionsContext = new CustomFunctionsContext(context.GetSchemaHelper(), context.GetConnections(), context.GetConnection(), 
            context.GetRuleset(), context.GetLocale(), context.GetUserSettings(), &context.GetUsedSettingsListener(), 
            context.GetECExpressionsCache(), context.GetNodesFactory(), nullptr, nullptr, nullptr, 
            context.IsPropertyFormattingContext() ? &context.GetECPropertyFormatter() : nullptr);
        if (context.IsLocalizationContext())
            m_functionsContext->SetLocalizationProvider(*localizationProvider);
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    ~DescriptorBuilder() {DELETE_AND_CLEAR(m_descriptorBuilder); DELETE_AND_CLEAR(m_context); DELETE_AND_CLEAR(m_functionsContext);}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            07/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    ContentDescriptorCPtr GetDescriptor() {return m_descriptor;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct QueryBuilder : ContentSpecificationsVisitor
{
private:
    ContentQueryBuilder* m_queryBuilder;
    ContentDescriptorPtr m_descriptor;
    ContentQueryPtr m_union;
    
protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(SelectedNodeInstancesSpecificationCR specification) override
        {
        if (nullptr == GetInput())
            {
            BeAssert(false);
            return;
            }

        if (m_union.IsValid() && specification.GetOnlyIfNotHandled())
            {
            LoggingHelper::LogMessage(Log::Content, "SelectedNodeInstances content specification was ignored because the rule is already handled", 
                NativeLogging::LOG_INFO);
            return;
            }

        ContentQueryPtr query = m_queryBuilder->CreateQuery(specification, *m_descriptor, *GetInput());
        if (query.IsValid())
            {
            QueryBuilderHelpers::SetOrUnion(m_union, *query);
            return;
            }

        LoggingHelper::LogMessage(Log::Content, "SelectedNodeInstances content specification did not result in any query", NativeLogging::LOG_DEBUG);
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(ContentInstancesOfSpecificClassesSpecificationCR specification) override
        {
        ContentQueryPtr query = m_queryBuilder->CreateQuery(specification, *m_descriptor);
        if (query.IsValid())
            {
            QueryBuilderHelpers::SetOrUnion(m_union, *query);
            return;
            }

        LoggingHelper::LogMessage(Log::Content, "ContentInstancesOfSpecificClasses content specification did not result in any query", NativeLogging::LOG_DEBUG);
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(ContentRelatedInstancesSpecificationCR specification) override 
        {
        if (nullptr == GetInput())
            {
            BeAssert(false);
            return;
            }

        ContentQueryPtr query = m_queryBuilder->CreateQuery(specification, *m_descriptor, *GetInput());
        if (query.IsValid())
            {
            QueryBuilderHelpers::SetOrUnion(m_union, *query);
            return;
            }

        LoggingHelper::LogMessage(Log::Content, "ContentRelatedInstances content specification did not result in any query", NativeLogging::LOG_DEBUG);
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    QueryBuilder(ContentProviderContextCR context, ContentDescriptorCR descriptor)
        : ContentSpecificationsVisitor(context)
        {
        IECPropertyFormatter const* formatter = context.IsPropertyFormattingContext() ? &context.GetECPropertyFormatter() : nullptr;
        ILocalizationProvider const* localizationProvider = context.IsLocalizationContext() ? &context.GetLocalizationProvider() : nullptr;

        ContentQueryBuilderParameters params(context.GetSchemaHelper(), context.GetConnections(), context.GetNodesLocater(), context.GetConnection(), 
            context.GetRuleset(), context.GetLocale(), context.GetUserSettings(), context.GetECExpressionsCache(), 
            context.GetCategorySupplier(), formatter, context.GetLocalState(), localizationProvider);
        
        m_queryBuilder = new ContentQueryBuilder(params);
        m_descriptor = ContentDescriptor::Create(descriptor);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    ~QueryBuilder() {DELETE_AND_CLEAR(m_queryBuilder);}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    ContentQueryPtr GetQuery()
        {
        m_union = QueryBuilderHelpers::CreateMergedResultsQuery(*m_union, *m_descriptor);
        return m_union;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProvider::ContentProvider(ContentProviderContextR context)
    : m_context(&context), m_executor(nullptr), m_initialized(false), 
    m_contentSetSize(0), m_fullContentSetSizeDetermined(false)
    {
    if (GetContext().IsQueryContext())
        m_executor = new ContentQueryExecutor(context.GetConnection());
    if (nullptr != m_executor && GetContext().IsPropertyFormattingContext())
        m_executor->SetPropertyFormatter(GetContext().GetECPropertyFormatter());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProvider::ContentProvider(ContentProviderCR other)
    : m_executor(nullptr), m_initialized(false), 
    m_contentSetSize(0), m_fullContentSetSizeDetermined(false)
    {
    m_context = ContentProviderContext::Create(*other.m_context);
    m_context->SetCancelationToken(&other.GetContext().GetCancelationToken());
    if (GetContext().IsQueryContext())
        m_executor = new ContentQueryExecutor(m_context->GetConnection());
    if (nullptr != m_executor && GetContext().IsPropertyFormattingContext())
        m_executor->SetPropertyFormatter(GetContext().GetECPropertyFormatter());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProvider::~ContentProvider()
    {
    DELETE_AND_CLEAR(m_executor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProvider::~SpecificationContentProvider()
    {
    for (auto iter : m_inputCache)
        delete iter.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void VisitRuleSpecifications(ContentSpecificationsVisitor& visitor, bmap<ContentRuleCP, IParsedInput const*> inputCache,
    ContentProviderContextCR context, ContentRuleInstanceKeysList const& rules)
    {
    for (ContentRuleInstanceKeys const& rule : rules)
        {
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
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCP SpecificationContentProvider::_GetContentDescriptor() const
    {
    if (m_descriptor.IsNull())
        {
        DescriptorBuilder builder(GetContext());
        VisitRuleSpecifications(builder, m_inputCache, GetContext(), m_rules);
        m_descriptor = builder.GetDescriptor();
        }    
    return m_descriptor.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryCPtr SpecificationContentProvider::_GetQuery() const
    {
    if (m_query.IsNull())
        {
        ContentDescriptorCP descriptor = GetContentDescriptor();
        if (nullptr == descriptor)
            return nullptr;

        QueryBuilder builder(GetContext(), *descriptor);
        VisitRuleSpecifications(builder, m_inputCache, GetContext(), m_rules);
        m_query = builder.GetQuery();
        QueryBuilderHelpers::ApplyPagingOptions(m_query, GetPageOptions());
        }
    return m_query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SpecificationContentProvider::SetContentDescriptor(ContentDescriptorCR descriptor)
    {
    if (m_descriptor.IsValid() && descriptor.Equals(*m_descriptor))
        return;

    m_descriptor = &descriptor;
    _Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::SetPageOptions(PageOptions options)
    {
    if (m_pageOptions.Equals(options))
        return;

    m_pageOptions = options;
    _Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::_Reset()
    {
    m_records.clear();
    m_contentSetSize = 0;
    m_fullContentSetSizeDetermined = false;
    m_initialized = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::Initialize()
    {
    if (m_initialized)
        return;

    if (nullptr == m_executor)
        return;

    m_initialized = true;

    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(), 
        GetContext().GetRuleset(), GetContext().GetLocale(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(), 
        GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), nullptr, nullptr, nullptr, 
        GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr);
    if (GetContext().IsLocalizationContext())
        fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());
    
    m_executor->SetQuery(*_GetQuery());
    m_executor->ReadRecords(&GetContext().GetCancelationToken());

    if (GetContext().GetCancelationToken().IsCanceled())
        return;

    size_t index = 0;
    ContentSetItemPtr item;
    while ((item = m_executor->GetRecord(index++)).IsValid())
        {
        LoadNestedContent(*item);
        m_records.push_back(item);
        }

    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentProvider::GetFullContentSetSize() const
    {
    if (!m_fullContentSetSizeDetermined)
        {
        if (GetContentDescriptor()->MergeResults())
            {
            m_contentSetSize = 1;
            }
        else if (GetContext().IsQueryContext())
            {
            CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(), 
                GetContext().GetRuleset(), GetContext().GetLocale(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(), 
                GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), nullptr, nullptr, nullptr, 
                GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr);
            if (GetContext().IsLocalizationContext())
                fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());

            // remove any ordering
            ContentQueryPtr queryNotSorted = _GetQuery()->Clone();
            QueryBuilderHelpers::Order(*queryNotSorted, "");

            // create a count query
            RefCountedPtr<CountQueryContract> contract = CountQueryContract::Create();
            ComplexGenericQueryPtr countQuery = ComplexGenericQuery::Create();
            countQuery->SelectContract(*contract);
            countQuery->GroupByContract(*contract);
            countQuery->From(*StringGenericQuery::Create(queryNotSorted->ToString(), queryNotSorted->GetBoundValues()));

            // execute
            CountQueryExecutor executor(GetContext().GetConnection(), *countQuery);
            executor.ReadRecords(&GetContext().GetCancelationToken());
            if (!GetContext().GetCancelationToken().IsCanceled())
                m_contentSetSize = executor.GetResult();
            }

        m_fullContentSetSizeDetermined = true;
        }
    return m_contentSetSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentProvider::GetContentSetSize() const
    {
    const_cast<ContentProviderP>(this)->Initialize();
    return m_records.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentProvider::GetContentSetItem(ContentSetItemPtr& item, size_t index) const
    {
    const_cast<ContentProviderP>(this)->Initialize();
    if (index >= m_records.size())
        return false;

    item = m_records[index];
    CustomizationHelper::Customize(GetContext(), *GetContentDescriptor(), *item);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::InvalidateContent()
    {
    _Reset();
    m_executor->Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NestedContentProvider::NestedContentProvider(ContentProviderContextR context, ContentDescriptor::NestedContentField const& field)
    : ContentProvider(context), m_field(field), m_mergedResults(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NestedContentProvider::NestedContentProvider(NestedContentProviderCR other)
    : ContentProvider(other), m_field(other.m_field)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::_Reset()
    {
    if (!m_mergedResults)
        m_adjustedQuery = nullptr;
    ContentProvider::_Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorCP NestedContentProvider::_GetContentDescriptor() const
    {
    ContentQueryCPtr query = _GetQuery();
    return query.IsValid() ? &query->GetContract()->GetDescriptor() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryCPtr NestedContentProvider::_GetQuery() const
    {
    if (m_query.IsNull())
        {
        IECPropertyFormatter const* formatter = GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr;
        ContentQueryBuilderParameters params(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetNodesLocater(), 
            GetContext().GetConnection(), GetContext().GetRuleset(), GetContext().GetLocale(), GetContext().GetUserSettings(), GetContext().GetECExpressionsCache(),
            GetContext().GetCategorySupplier(), formatter, GetContext().GetLocalState());
        if (GetContext().IsLocalizationContext())
            params.SetLoacalizationProvider(&GetContext().GetLocalizationProvider());
        ContentQueryBuilder queryBuilder(params);
        m_query = queryBuilder.CreateQuery(m_field);
        }
    
    if (m_query.IsNull())
        return nullptr;

    if (m_adjustedQuery.IsNull() && !m_mergedResults)
        {
        Utf8StringCR idFieldAlias = m_field.GetRelationshipPath().empty() ? m_field.GetContentClassAlias() : m_field.GetRelationshipPath().front().GetTargetClassAlias();
        Utf8String idSelector = Utf8String("[").append(idFieldAlias).append("].[ECInstanceId]");
        bvector<ECInstanceId> ids;
        std::transform(m_primaryInstanceKeys.begin(), m_primaryInstanceKeys.end(), std::back_inserter(ids), [](ECClassInstanceKeyCR key){return key.GetId();});
        IdsFilteringHelper<bvector<ECInstanceId>> idsFilteringHelper(ids);
        ContentQueryPtr query = m_query->Clone();
        QueryBuilderHelpers::Where(query, idsFilteringHelper.CreateWhereClause(idSelector.c_str()).c_str(), idsFilteringHelper.CreateBoundValues());
        m_adjustedQuery = query;
        }

    if (m_mergedResults)
        {
        if (m_adjustedQuery.IsNull())
            {
            BoundQueryValuesList bindings = {new BoundQueryId(m_primaryInstanceKeys[0].GetId())};
            ContentQueryPtr query = m_query->Clone();
            Utf8StringCR idFieldAlias = m_field.GetRelationshipPath().empty() ? m_field.GetContentClassAlias() : m_field.GetRelationshipPath().front().GetTargetClassAlias();
            Utf8String whereClause;
            whereClause.append("[").append(idFieldAlias).append("].[ECInstanceId] = ? ");
            QueryBuilderHelpers::Where(query, whereClause.c_str(), bindings);
            m_adjustedQuery = query;
            }
        else
            {
            BoundQueryValuesList bindings = {new BoundQueryId(m_primaryInstanceKeys[0].GetId())};
            const_cast<ContentQuery*>(m_adjustedQuery.get())->AsComplexQuery()->SetBoundValues(bindings);
            }
        }
        
    return m_adjustedQuery;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::SetPrimaryInstanceKeys(bvector<ECClassInstanceKey> const& keys)
    {
    m_primaryInstanceKeys = keys;
    _Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::SetPrimaryInstanceKey(ECClassInstanceKeyCR key)
    {
    SetPrimaryInstanceKeys({key});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::SetIsResultsMerged(bool mergedResults)
    {
    m_mergedResults = mergedResults;
    }
