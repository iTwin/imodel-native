/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentProviders.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentationRules/SpecificationVisitor.h>
#include "ContentProviders.h"
#include "ECExpressionContextsProvider.h"
#include "LocalizationHelper.h"
#include "LoggingHelper.h"
#include "CustomizationHelper.h"
#include "NavNodeProviders.h"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentProviderContext::ContentProviderContext(ECN::PresentationRuleSetCR ruleset, bool holdRuleset, Utf8String preferredDisplayType, INavNodeLocaterCR nodesLocater, IPropertyCategorySupplierR categorySupplier, 
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
static int GetSerializationFlags(bool isMerged, bool isNested, ContentRequest req)
    {
    if (isMerged || !isNested)
        return ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values | ContentSetItem::SERIALIZE_DisplayValues;

    if (ContentRequest::Values == req)
        return ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values;
    if (ContentRequest::DisplayValues == req)
        return ContentSetItem::SERIALIZE_DisplayValues;

    BeAssert(false);
    return ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values | ContentSetItem::SERIALIZE_DisplayValues;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document GetNestedContent(ContentProviderCR provider, int serializationflags, rapidjson::Document::AllocatorType* allocator = nullptr)
    {
    rapidjson::Document json(allocator);
    json.SetArray();
    size_t index = 0;
    ContentSetItemPtr item;
    while (provider.GetContentSetItem(item, index++))
        json.PushBack(item->AsJson(serializationflags, &json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AreContentValuesEqual(RapidJsonValueCR lhs, RapidJsonValueCR rhs)
    {
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
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NestedContentProviderPtr ContentProvider::GetNestedContentProvider(ContentDescriptor::NestedContentField const& field) const
    {
    auto iter = m_nestedContentProviders.find(&field);
    if (m_nestedContentProviders.end() == iter)
        {
        ContentProviderContextPtr context = ContentProviderContext::Create(GetContext());
        context->SetIsNestedContent(true);
        NestedContentProviderPtr provider = NestedContentProvider::Create(*context, field);
        iter = m_nestedContentProviders.Insert(&field, provider).first;
        }
    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentProvider::LoadNestedContentFieldValues(ContentSetItemR item) const
    {
    ContentDescriptorCP descriptor = GetContentDescriptor();
    if (nullptr == descriptor)
        return;

    for (ContentDescriptor::Field const* field : descriptor->GetAllFields())
        {
        if (!field->IsNestedContentField())
            continue;
        
        Utf8CP fieldName = field->GetName().c_str();
        NestedContentProviderPtr provider = GetNestedContentProvider(*field->AsNestedContentField());
        if (descriptor->MergeResults())
            {
            // if records are merged, have to query nested content for each merged record individually 
            // and merge them one after one
            rapidjson::Document content;
            for (ECInstanceKeyCR key : item.GetKeys())
                {
                provider->SetPrimaryInstanceKey(key);
                rapidjson::Document instanceContent = GetNestedContent(*provider, 
                    GetSerializationFlags(true, GetContext().IsNestedContent(), ContentRequest::Values));
                if (content.IsNull())
                    {
                    // first pass - save the instance content
                    content = std::move(instanceContent);
                    }
                else if (AreContentValuesEqual(content, instanceContent))
                    {
                    // values are equal - merge PrimaryKeys arrays
                    MergePrimaryKeys(content, instanceContent, content.GetAllocator());
                    }
                else
                    {
                    // values are different - set the "varies" string
                    Utf8String localizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
                    Utf8String prelocalizedLabel = Utf8PrintfString(CONTENTRECORD_MERGED_VALUE_FORMAT, localizationId.c_str());
                    Utf8String localizedLabel = prelocalizedLabel;
                    LocalizationHelper(GetContext().GetLocalizationProvider()).LocalizeString(localizedLabel);
                    content.Clear();
                    content.SetString(localizedLabel.c_str(), content.GetAllocator());
                    item.GetMergedFieldNames().push_back(fieldName);
                    break;
                    }
                }
            item.GetDisplayValues().AddMember(rapidjson::Value(fieldName, item.GetDisplayValues().GetAllocator()), 
                rapidjson::Value(content, item.GetDisplayValues().GetAllocator()), item.GetDisplayValues().GetAllocator());
            item.GetValues().AddMember(rapidjson::Value(fieldName, item.GetValues().GetAllocator()), 
                rapidjson::Value(content, item.GetValues().GetAllocator()), item.GetValues().GetAllocator());
            }
        else
            {
            // if not merging, can query nested content without any additional work afterwards
            provider->SetPrimaryInstanceKeys(item.GetKeys());
            item.GetValues().AddMember(rapidjson::Value(fieldName, item.GetValues().GetAllocator()), 
                GetNestedContent(*provider, GetSerializationFlags(false, GetContext().IsNestedContent(), ContentRequest::Values), &item.GetValues().GetAllocator()), 
                item.GetValues().GetAllocator());
            item.GetDisplayValues().AddMember(rapidjson::Value(fieldName, item.GetDisplayValues().GetAllocator()), 
                GetNestedContent(*provider, GetSerializationFlags(false, GetContext().IsNestedContent(), ContentRequest::DisplayValues), &item.GetDisplayValues().GetAllocator()), 
                item.GetDisplayValues().GetAllocator());
            }
        }
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ContentSpecificationsVisitor : PresentationRuleSpecificationVisitor
{
private:
    ContentQueryBuilder* m_queryBuilder;
    IParsedSelectionInfo const* m_selectionInfo;    
protected:
    ContentQueryBuilder& GetQueryBuilder() {return *m_queryBuilder;}
    IParsedSelectionInfo const* GetSelectionInfo() {return m_selectionInfo;}
public:
    ContentSpecificationsVisitor(ContentProviderContext const& context)
        {
        IECPropertyFormatter const* formatter = context.IsPropertyFormattingContext() ? &context.GetECPropertyFormatter() : nullptr;
        ContentQueryBuilderParameters params(context.GetSchemaHelper(), context.GetNodesLocater(), 
            context.GetRuleset(), context.GetPreferredDisplayType().c_str(), context.GetUserSettings(), context.GetECExpressionsCache(), 
            context.GetCategorySupplier(), formatter, context.GetLocalState());
        
        if (context.IsLocalizationContext())
            params.SetLoacalizationProvider(&context.GetLocalizationProvider());

        m_queryBuilder = new ContentQueryBuilder(params);
        }
    virtual ~ContentSpecificationsVisitor() {}
    void SetCurrentSelection(IParsedSelectionInfo const* selection) {m_selectionInfo = selection;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct DescriptorBuilder : ContentSpecificationsVisitor
{
private:
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

        ContentDescriptorPtr specificationDescriptor = GetQueryBuilder().CreateDescriptor(specification, *GetSelectionInfo());
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
        ContentDescriptorPtr specificationDescriptor = GetQueryBuilder().CreateDescriptor(specification);
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

        ContentDescriptorPtr specificationDescriptor = GetQueryBuilder().CreateDescriptor(specification, *GetSelectionInfo());
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
        {}

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

        ContentQueryPtr query = GetQueryBuilder().CreateQuery(specification, *m_descriptor, *GetSelectionInfo());
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
        ContentQueryPtr query = GetQueryBuilder().CreateQuery(specification, *m_descriptor);
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

        ContentQueryPtr query = GetQueryBuilder().CreateQuery(specification, *m_descriptor, *GetSelectionInfo());
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
        m_descriptor = ContentDescriptor::Create(descriptor);
        }

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
static void VisitRuleSpecifications(ContentSpecificationsVisitor& visitor, bmap<ECN::ContentRuleCP, IParsedSelectionInfo const*> selectionsCache,
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
        
    CustomFunctionsContext fnContext(GetContext().GetDb(), GetContext().GetRuleset(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(), 
        GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), nullptr, nullptr, nullptr);
    if (GetContext().IsLocalizationContext())
        fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());

    size_t index = 0;
    ContentSetItemPtr item;
    while ((item = m_executor.GetRecord(index++)).IsValid())
        {
        LoadNestedContentFieldValues(*item);
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
            CustomFunctionsContext fnContext(GetContext().GetDb(), GetContext().GetRuleset(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(), 
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
            GetContext().GetRuleset(), GetContext().GetPreferredDisplayType().c_str(), GetContext().GetUserSettings(), GetContext().GetECExpressionsCache(),
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
        ContentQueryPtr query = m_query->Clone();
        Utf8StringCR primaryInstanceClassAlias = m_field.GetRelationshipPath().front().GetTargetClassAlias();
        if (m_primaryInstanceKeys.size() > 100)
            {
            Utf8String whereClause("InVirtualSet(?, [");
            whereClause.append(primaryInstanceClassAlias).append("].[ECInstanceId])");
            QueryBuilderHelpers::Where(query, whereClause.c_str(), {new BoundQueryIdSet(m_primaryInstanceKeys)});
            }
        else
            {
            Utf8String idsArg(m_primaryInstanceKeys.size() * 2 - 1, '?');
            for (size_t i = 1; i < m_primaryInstanceKeys.size(); i += 2)
                idsArg[i] = ',';
            Utf8String whereClause;
            whereClause.append("[").append(primaryInstanceClassAlias).append("].[ECInstanceId] IN (");
            whereClause.append(idsArg).append(")");
            bvector<BoundQueryValue const*> boundIds;
            for (ECInstanceKeyCR key : m_primaryInstanceKeys)
                boundIds.push_back(new BoundQueryId(key.GetInstanceId()));
            QueryBuilderHelpers::Where(query, whereClause.c_str(), boundIds);
            }
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