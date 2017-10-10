/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentProviders.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
ContentProviderContext::ContentProviderContext(PresentationRuleSetCR ruleset, bool holdRuleset, Utf8String preferredDisplayType, INavNodeLocaterCR nodesLocater, IPropertyCategorySupplierR categorySupplier, 
    IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache, JsonNavNodesFactory const& nodesFactory, IJsonLocalState const* localState) 
    : RulesDrivenProviderContext(ruleset, holdRuleset, userSettings, ecexpressionsCache, relatedPathsCache, nodesFactory, localState), m_preferredDisplayType(preferredDisplayType), 
    m_nodesLocater(nodesLocater), m_categorySupplier(categorySupplier)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::ContentProviderContext(ContentProviderContextCR other) 
    : RulesDrivenProviderContext(other), m_preferredDisplayType(other.m_preferredDisplayType), m_nodesLocater(other.m_nodesLocater), 
    m_categorySupplier(other.m_categorySupplier)
    {
    Init();

    if (other.IsSelectionContext())
        SetSelectionContext(other);

    if (other.IsPropertyFormattingContext())
        SetPropertyFormattingContext(other);
    
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
    m_selectionInfo = nullptr;
    m_ownsSelectionInfo = false;
    m_propertyFormatter = nullptr;
    m_isNestedContent = false;
    m_createFields = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::~ContentProviderContext()
    {
    if (m_ownsSelectionInfo)
        DELETE_AND_CLEAR(m_selectionInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetSelectionContext(SelectionInfo const& selectionInfo)
    {
    BeAssert(!IsSelectionContext());
    m_isSelectionContext = true;
    m_selectionInfo = new SelectionInfo(selectionInfo);
    m_ownsSelectionInfo = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetSelectionContext(SelectionInfo&& selectionInfo)
    {
    BeAssert(!IsSelectionContext());
    m_isSelectionContext = true;
    m_selectionInfo = new SelectionInfo(std::move(selectionInfo));
    m_ownsSelectionInfo = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetSelectionContext(ContentProviderContextCR other)
    {
    BeAssert(!IsSelectionContext());
    m_isSelectionContext = true;
    m_selectionInfo = other.m_selectionInfo;
    m_ownsSelectionInfo = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetPropertyFormattingContext(IECPropertyFormatter const& formatter)
    {
    BeAssert(!IsPropertyFormattingContext());
    m_propertyFormatter = &formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProviderContext::SetPropertyFormattingContext(ContentProviderContextCR other)
    {
    BeAssert(!IsPropertyFormattingContext());
    m_propertyFormatter = other.m_propertyFormatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProvider::SpecificationContentProvider(ContentProviderContextR context, ContentRuleSpecificationsList ruleSpecs)
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

    if (isMerged || !isNested)
        return ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values | ContentSetItem::SERIALIZE_DisplayValues;
    if (ContentRequest::Values == req)
        return ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values;
    if (ContentRequest::DisplayValues == req)
        return ContentSetItem::SERIALIZE_DisplayValues;
    
    BeAssert(false);
    return ContentSetItem::SERIALIZE_All;
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
        rapidjson::Document itemJson = item->AsJson(serializationFlags, &json.GetAllocator());
        if (!isRelatedContent && ContentRequest::Values == req)
            json.CopyFrom(itemJson["Values"][provider.GetContentField().GetName().c_str()], json.GetAllocator());
        else if (!isRelatedContent && ContentRequest::DisplayValues == req)
            json.CopyFrom(itemJson["DisplayValues"][provider.GetContentField().GetName().c_str()], json.GetAllocator());
        else
            json.PushBack(itemJson, json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AreContentValuesEqual(RapidJsonValueCR lhs, RapidJsonValueCR rhs, bool isRelatedContent)
    {
    if (!isRelatedContent)
        return lhs == rhs;

    BeAssert(lhs.IsArray() && rhs.IsArray());
    if (lhs.Size() != rhs.Size())
        return false;

    for (rapidjson::SizeType i = 0; i < lhs.Size(); ++i)
        {
        BeAssert(lhs[i].HasMember("Values") && rhs[i].HasMember("Values"));
        RapidJsonValueCR lhsValues = lhs[i]["Values"];
        RapidJsonValueCR rhsValues = rhs[i]["Values"];
        if (lhsValues != rhsValues)
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergePrimaryKeys(RapidJsonValueR target, RapidJsonValueCR source, rapidjson::Document::AllocatorType& allocator)
    {
    BeAssert(source.IsArray() && target.IsArray());
    BeAssert(source.Size() == target.Size());    
    for (rapidjson::SizeType i = 0; i < source.Size(); ++i)
        {
        BeAssert(source[i].HasMember("PrimaryKeys") && target[i]["PrimaryKeys"].IsArray());
        BeAssert(target[i].HasMember("PrimaryKeys") && target[i]["PrimaryKeys"].IsArray());
        RapidJsonValueCR sourceKeys = source[i]["PrimaryKeys"];
        RapidJsonValueR targetKeys = target[i]["PrimaryKeys"];
        for (rapidjson::SizeType j = 0; j < sourceKeys.Size(); ++j)
            targetKeys.PushBack(rapidjson::Value(sourceKeys[j], allocator), allocator);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetLocalizedVariesString(ILocalizationProvider const& localizationProvider)
    {
    Utf8String localizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
    Utf8String prelocalizedLabel = Utf8PrintfString(CONTENTRECORD_MERGED_VALUE_FORMAT, localizationId.c_str());
    Utf8String localizedLabel = prelocalizedLabel;
    LocalizationHelper(localizationProvider).LocalizeString(localizedLabel);
    return localizedLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MergeContent(RapidJsonValueR targetValues, rapidjson::Document::AllocatorType& targetValuesAllocator, 
    RapidJsonValueR targetDisplayValues, rapidjson::Document::AllocatorType& targetDisplayValuesAllocator, 
    RapidJsonValueCR source, bool isRelatedContent, ILocalizationProvider const& localizationProvider)
    {
    if (AreContentValuesEqual(targetValues, source, isRelatedContent))
        {
        if (isRelatedContent)
            {
            // values are equal - merge PrimaryKeys arrays (only for related fields)
            MergePrimaryKeys(targetValues, source, targetValuesAllocator);
            }
        return false;
        }
    
    // values are different - set the "varies" string
    targetValues.SetNull();
    targetDisplayValues.SetString(GetLocalizedVariesString(localizationProvider).c_str(), targetDisplayValuesAllocator);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MergeContent(rapidjson::Document& targetValues, rapidjson::Document& targetDisplayValues, RapidJsonValueCR source,
    bool isRelatedContent, ILocalizationProvider const& localizationProvider)
    {
    return MergeContent(targetValues, targetValues.GetAllocator(), targetDisplayValues, targetDisplayValues.GetAllocator(),
        source, isRelatedContent, localizationProvider);
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
    ContentDescriptorCR descriptor = *GetContentDescriptor();
    Utf8CP fieldName = field.GetName().c_str();
    bool isRelatedContent = !field.GetRelationshipPath().empty();
    NestedContentProviderPtr provider = GetNestedContentProvider(field, cacheable);
    if (descriptor.MergeResults())
        {
        // if records are merged, have to query nested content for each merged record individually 
        // and merge them one after one
        rapidjson::Document content;
        for (ECInstanceKeyCR key : item.GetKeys())
            {
            if (!isRelatedContent && field.GetContentClass().GetId() != key.GetClassId())
                continue;

            provider->SetPrimaryInstanceKey(key);
            rapidjson::Document instanceContent = GetNestedContent(*provider, ContentRequest::Values, true, GetContext().IsNestedContent(), isRelatedContent);
            if (content.IsNull())
                {
                // first pass - save the instance content
                content = std::move(instanceContent);
                }
            else if (MergeContent(content, content, instanceContent, isRelatedContent, GetContext().GetLocalizationProvider()))
                {
                // if detected different values during merge, stop
                item.GetMergedFieldNames().push_back(fieldName);
                break;
                }
            }
        if (item.GetDisplayValues().HasMember(fieldName) || item.GetValues().HasMember(fieldName))
            {
            RapidJsonValueR values = item.GetValues()[fieldName];
            RapidJsonValueR displayValues = item.GetDisplayValues()[fieldName];
            bool areValuesDifferent = MergeContent(values, item.GetValues().GetAllocator(), displayValues, item.GetDisplayValues().GetAllocator(),
                content, isRelatedContent, GetContext().GetLocalizationProvider());
            if (areValuesDifferent)
                item.GetMergedFieldNames().push_back(fieldName);
            }
        else
            {
            item.GetDisplayValues().AddMember(rapidjson::Value(fieldName, item.GetDisplayValues().GetAllocator()),
                rapidjson::Value(content, item.GetDisplayValues().GetAllocator()), item.GetDisplayValues().GetAllocator());
            item.GetValues().AddMember(rapidjson::Value(fieldName, item.GetValues().GetAllocator()),
                rapidjson::Value(content, item.GetValues().GetAllocator()), item.GetValues().GetAllocator());
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
        bset<ECClassId> handledClassIds;
        for (ECInstanceKeyCR key : item.GetKeys())
            {
            if (handledClassIds.end() != handledClassIds.find(key.GetClassId()))
                continue;

            ECClassCP keyClass = GetContext().GetDb().Schemas().GetClass(key.GetClassId());
            bvector<ContentDescriptor::Property const*> matchingProperties = field.FindMatchingProperties(keyClass);
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
                propertiesPerItemClass[keyClass] = matchingProperty;
                }
            handledClassIds.insert(key.GetClassId());
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
            BeAssert(false);
            return;
            }
        propertiesPerItemClass[item.GetClass()] = matchingProperty;
        }

    for (auto pair : propertiesPerItemClass)
        {
        ECClassCP itemClass = pair.first;
        ContentDescriptor::Property const* matchingProperty = pair.second;

        // create a nested content provider for it
        ContentDescriptor::ECPropertiesField* nestedField = new ContentDescriptor::ECPropertiesField(*itemClass, *matchingProperty);
        nestedField->SetName(field.GetName());
        ContentDescriptor::NestedContentField nestingField(field.GetCategory(), field.GetName(), field.GetLabel(),
            *itemClass, "this", RelatedClassPath(), {nestedField}, field.GetPriority());

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
    IParsedSelectionInfo const* m_selectionInfo;    
protected:
    IParsedSelectionInfo const* GetSelectionInfo() {return m_selectionInfo;}
public:
    ContentSpecificationsVisitor(ContentProviderContext const& context) : m_selectionInfo(nullptr) {}
    virtual ~ContentSpecificationsVisitor() {}
    void SetCurrentSelection(IParsedSelectionInfo const* selection) {m_selectionInfo = selection;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct DescriptorBuilder : ContentSpecificationsVisitor
{
private:
    ContentDescriptorBuilder::Context* m_context;
    ContentDescriptorBuilder* m_descriptorBuilder;
    ContentDescriptorPtr m_descriptor;
    
protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            07/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(SelectedNodeInstancesSpecificationCR specification) override
        {
        if (nullptr == GetSelectionInfo())
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

        ContentDescriptorPtr specificationDescriptor = m_descriptorBuilder->CreateDescriptor(specification, *GetSelectionInfo());
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
        if (nullptr == GetSelectionInfo())
            {
            BeAssert(false);
            return;
            }

        ContentDescriptorPtr specificationDescriptor = m_descriptorBuilder->CreateDescriptor(specification, *GetSelectionInfo());
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
        m_context = new ContentDescriptorBuilder::Context(context.GetSchemaHelper(), context.GetRuleset(),
            context.GetPreferredDisplayType().c_str(), context.GetCategorySupplier(), formatter, localizationProvider);
        m_descriptorBuilder = new ContentDescriptorBuilder(*m_context);
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    ~DescriptorBuilder() {DELETE_AND_CLEAR(m_descriptorBuilder); DELETE_AND_CLEAR(m_context);}

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
        if (nullptr == GetSelectionInfo())
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

        ContentQueryPtr query = m_queryBuilder->CreateQuery(specification, *m_descriptor, *GetSelectionInfo());
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
        if (nullptr == GetSelectionInfo())
            {
            BeAssert(false);
            return;
            }

        ContentQueryPtr query = m_queryBuilder->CreateQuery(specification, *m_descriptor, *GetSelectionInfo());
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

        ContentQueryBuilderParameters params(context.GetSchemaHelper(), context.GetNodesLocater(), 
            context.GetRuleset(), context.GetUserSettings(), context.GetECExpressionsCache(), 
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
    : m_context(&context), m_executor(context.GetDb(), context.GetStatementCache()), m_initialized(false), 
    m_contentSetSize(0), m_fullContentSetSizeDetermined(false)
    {
    if (GetContext().IsPropertyFormattingContext())
        m_executor.SetPropertyFormatter(GetContext().GetECPropertyFormatter());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProvider::ContentProvider(ContentProviderCR other)
    : m_context(other.m_context), m_executor(m_context->GetDb(), m_context->GetStatementCache()), m_initialized(false), 
    m_contentSetSize(0), m_fullContentSetSizeDetermined(false)
    {
    if (GetContext().IsPropertyFormattingContext())
        m_executor.SetPropertyFormatter(GetContext().GetECPropertyFormatter());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SpecificationContentProvider::~SpecificationContentProvider()
    {
    for (auto iter : m_parsedSelectionsCache)
        delete iter.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void VisitRuleSpecifications(ContentSpecificationsVisitor& visitor, bmap<ContentRuleCP, IParsedSelectionInfo const*> selectionsCache,
    ContentProviderContextCR context, ContentRuleSpecificationsList const& rules)
    {
    for (ContentRuleSpecification const& rule : rules)
        {
        IParsedSelectionInfo const* selectionInfo = nullptr;
        if (context.IsSelectionContext())
            {
            auto selectionIter = selectionsCache.find(&rule.GetRule());
            if (selectionsCache.end() == selectionIter)
                {
                // note: each content rule may be based on different selected nodes, so we create a different selection context for each of them
                selectionIter = selectionsCache.Insert(&rule.GetRule(),
                    new ParsedSelectionInfo(rule.GetMatchingSelectedNodeKeys(), context.GetNodesLocater(), context.GetSchemaHelper())).first;
                }
            selectionInfo = selectionIter->second;
            }
        visitor.SetCurrentSelection(selectionInfo);

        for (ContentSpecificationCP spec : rule.GetRule().GetSpecifications())
            spec->Accept(visitor);

        visitor.SetCurrentSelection(nullptr);
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
        VisitRuleSpecifications(builder, m_parsedSelectionsCache, GetContext(), m_rules);
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
        VisitRuleSpecifications(builder, m_parsedSelectionsCache, GetContext(), m_rules);
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

    m_initialized = true;
    m_executor.SetQuery(*_GetQuery());
        
    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetRuleset(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(), 
        GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), nullptr, nullptr, nullptr);
    if (GetContext().IsLocalizationContext())
        fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());

    size_t index = 0;
    ContentSetItemPtr item;
    while ((item = m_executor.GetRecord(index++)).IsValid())
        {
        LoadNestedContent(*item);
        m_records.push_back(item);
        }
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
        else
            {            
            CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetRuleset(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(), 
                GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), nullptr, nullptr, nullptr);
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
            CountQueryExecutor executor(GetContext().GetDb(), GetContext().GetStatementCache(), *countQuery);
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

    if (GetContentDescriptor()->ShowImages())
        CustomizationHelper::Customize(GetContext(), *item);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::InvalidateContent()
    {
    _Reset();
    m_executor.ClearCache();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NestedContentProvider::NestedContentProvider(ContentProviderContextR context, ContentDescriptor::NestedContentField const& field)
    : ContentProvider(context), m_field(field)
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
        ContentQueryBuilderParameters params(GetContext().GetSchemaHelper(), GetContext().GetNodesLocater(),
            GetContext().GetRuleset(), GetContext().GetUserSettings(), GetContext().GetECExpressionsCache(),
            GetContext().GetCategorySupplier(), formatter, GetContext().GetLocalState());
        if (GetContext().IsLocalizationContext())
            params.SetLoacalizationProvider(&GetContext().GetLocalizationProvider());
        ContentQueryBuilder queryBuilder(params);
        m_query = queryBuilder.CreateQuery(m_field);
        }
    
    if (m_query.IsNull())
        return nullptr;

    if (m_adjustedQuery.IsNull())
        {
        Utf8StringCR idFieldAlias = m_field.GetRelationshipPath().empty() ? m_field.GetContentClassAlias() : m_field.GetRelationshipPath().front().GetTargetClassAlias();
        ContentQueryPtr query = m_query->Clone();

        Utf8String whereClause;
        BoundQueryValuesList bindings;
        if (IdSetHelper::BIND_VirtualSet == IdSetHelper::CreateInVirtualSetClause(whereClause, m_primaryInstanceKeys, Utf8String("[").append(idFieldAlias).append("].[ECInstanceId]")))
            {
            bindings = {new BoundQueryIdSet(m_primaryInstanceKeys)};
            }
        else
            {
            for (ECInstanceKeyCR key : m_primaryInstanceKeys)
                bindings.push_back(new BoundQueryId(key.GetInstanceId()));
            }
        QueryBuilderHelpers::Where(query, whereClause.c_str(), bindings);
        m_adjustedQuery = query;
        }
        
    return m_adjustedQuery;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::SetPrimaryInstanceKeys(bvector<ECInstanceKey> const& keys)
    {
    m_primaryInstanceKeys = keys;
    _Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NestedContentProvider::SetPrimaryInstanceKey(ECInstanceKeyCR key)
    {
    SetPrimaryInstanceKeys({key});
    }
