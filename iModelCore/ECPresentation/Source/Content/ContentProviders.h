/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Shared/Queries/QueryExecutor.h"
#include "../Shared/RulesDrivenProviderContext.h"
#include "ContentQueryBuilder.h"
#include "ContentQueryResultsReader.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentProviderContext : RulesDrivenProviderContext
{
private:
    // General
    Utf8String m_preferredDisplayType;
    int m_contentFlags;
    std::shared_ptr<INavNodeLocater> m_nodesLocater;
    IPropertyCategorySupplierCR m_categorySupplier;
    INavNodeKeysContainerCPtr m_inputNodeKeys;

    // Selection info context
    bool m_isSelectionContext;
    SelectionInfoCPtr m_selectionInfo;

private:
    void Init();
    ECPRESENTATION_EXPORT ContentProviderContext(PresentationRuleSetCR, Utf8String, int, INavNodeKeysContainerCR, std::shared_ptr<INavNodeLocater>, IPropertyCategorySupplierCR, std::unique_ptr<RulesetVariables>,
        ECExpressionsCache&, RelatedPathsCache&, NavNodesFactory const&, IJsonLocalState const*);
    ECPRESENTATION_EXPORT ContentProviderContext(ContentProviderContextCR other);

public:
    static ContentProviderContextPtr Create(PresentationRuleSetCR ruleset, Utf8String preferredDisplayType, int contentFlags, INavNodeKeysContainerCR inputKeys,
        std::shared_ptr<INavNodeLocater> nodesLocater, IPropertyCategorySupplierCR categorySupplier, std::unique_ptr<RulesetVariables> rulesetVariables, ECExpressionsCache& ecexpressionsCache,
        RelatedPathsCache& relatedPathsCache, NavNodesFactory const& nodesFactory, IJsonLocalState const* localState)
        {
        return new ContentProviderContext(ruleset, preferredDisplayType, contentFlags, inputKeys, nodesLocater,
            categorySupplier, std::move(rulesetVariables), ecexpressionsCache, relatedPathsCache, nodesFactory, localState);
        }
    static ContentProviderContextPtr Create(ContentProviderContextCR other) {return new ContentProviderContext(other);}
    ~ContentProviderContext();

    // General
    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    int GetContentFlags() const {return m_contentFlags;}
    INavNodeLocaterCR GetNodesLocater() const {return *m_nodesLocater;}
    IPropertyCategorySupplierCR GetCategorySupplier() const {return m_categorySupplier;}
    INavNodeKeysContainerCR GetInputKeys() const {return *m_inputNodeKeys;}
    void SetInputKeys(INavNodeKeysContainerCR inputNodeKeys) {m_inputNodeKeys = &inputNodeKeys;}
    ECPRESENTATION_EXPORT bvector<RulesetVariableEntry> GetRelatedRulesetVariables() const;

    // Selection info context
    ECPRESENTATION_EXPORT void SetSelectionInfo(SelectionInfoCR selectionInfo);
    ECPRESENTATION_EXPORT void SetSelectionInfo(ContentProviderContextCR);
    bool IsSelectionContext() const {return m_isSelectionContext;}
    SelectionInfo const* GetSelectionInfo() const {return m_selectionInfo.get();}

    // ECDb context
    void SetQueryContext(IConnectionManagerCR connections, IConnectionCR connection)
        {
        RulesDrivenProviderContext::SetQueryContext(connections, connection);
        }
    void SetQueryContext(ContentProviderContextCR other) {RulesDrivenProviderContext::SetQueryContext(other);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentProvider : RefCountedBase
{
private:
    ContentProviderContextPtr m_context;
    PageOptions m_pageOptions;
    std::unique_ptr<bvector<ContentSetItemPtr>> m_records;
    mutable std::unique_ptr<size_t> m_fullContentSetSize;
    mutable bmap<ContentDescriptor::NestedContentField const*, NestedContentProviderPtr> m_nestedContentProviders;
    mutable BeMutex m_mutex;

private:
    NestedContentProviderPtr GetNestedContentProvider(ContentDescriptor::NestedContentField const&, bool) const;
    void LoadNestedContent(ContentSetItemR, bvector<ContentDescriptor::Field*> const&) const;
    void LoadNestedContent(ContentSetItemR) const;
    void LoadNestedContentFieldValue(ContentSetItemR, ContentDescriptor::NestedContentField const&, bool) const;
    void LoadCompositePropertiesFieldValue(ContentSetItemR, ContentDescriptor::ECPropertiesField const&) const;

protected:
    ECPRESENTATION_EXPORT ContentProvider(ContentProviderContextR);
    ECPRESENTATION_EXPORT ContentProvider(ContentProviderCR);
    BeMutex& GetMutex() const {return m_mutex;}
    void InvalidateRecords();
    void InvalidateFullContentSetSize();
    void InvalidateNestedContentProviders();
    virtual ContentDescriptorCP _GetContentDescriptor() const = 0;
    virtual ContentQuerySet const& _GetContentQuerySet() const = 0;
    virtual GenericQuerySet _GetCountQuerySet() const = 0;
    virtual ContentProviderPtr _Clone() const = 0;
    virtual void _OnDescriptorChanged();
    virtual void _OnPageOptionsChanged();

public:
    void Initialize();
    ContentProviderPtr Clone() const {return _Clone();}
    ECPRESENTATION_EXPORT void Adopt(IConnectionCR, ICancelationTokenCP);

    ContentProviderContextR GetContextR() const {return *m_context;}
    ContentProviderContextCR GetContext() const {return GetContextR();}

    ContentDescriptorCP GetContentDescriptor() const {return _GetContentDescriptor();}

    PageOptionsCR GetPageOptions() const {return m_pageOptions;}
    ECPRESENTATION_EXPORT void SetPageOptions(PageOptions options);

    ECPRESENTATION_EXPORT bool GetContentSetItem(ContentSetItemPtr& item, size_t index) const;
    ECPRESENTATION_EXPORT size_t GetContentSetSize() const;
    ECPRESENTATION_EXPORT size_t GetFullContentSetSize() const;

    void InvalidateContent() {_OnDescriptorChanged();}
};

/*=================================================================================**//**
* Content provider used to get content based on presentation rule specifications.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SpecificationContentProvider : ContentProvider
{
private:
    ContentRuleInstanceKeysContainer m_rules;
    mutable ContentDescriptorCPtr m_descriptor;
    mutable std::unique_ptr<ContentQuerySet> m_queries;
    mutable bmap<ContentRuleCP, IParsedInput const*> m_inputCache;
    mutable bmap<Utf8String, bvector<DisplayValueGroupCPtr>> m_distinctValuesCache;
private:
    ECPRESENTATION_EXPORT SpecificationContentProvider(ContentProviderContextR, ContentRuleInstanceKeysContainer);
    ECPRESENTATION_EXPORT SpecificationContentProvider(SpecificationContentProviderCR);
    bvector<DisplayValueGroupCPtr> CreateDistinctValues(ContentDescriptor::Field const&) const;
protected:
    ContentDescriptorCP _GetContentDescriptor() const override;
    ContentQuerySet const& _GetContentQuerySet() const override;
    GenericQuerySet _GetCountQuerySet() const override;
    ContentProviderPtr _Clone() const override {return new SpecificationContentProvider(*this);}
    void _OnDescriptorChanged() override;
public:
    static SpecificationContentProviderPtr Create(ContentProviderContextR context, ContentRuleInstanceKeysContainer const& specs) {return new SpecificationContentProvider(context, specs);}
    static SpecificationContentProviderPtr Create(ContentProviderContextR context, ContentRuleInstanceKeys const& spec)
        {
        ContentRuleInstanceKeysContainer specs;
        specs.push_back(spec);
        return SpecificationContentProvider::Create(context, specs);
        }
    ~SpecificationContentProvider();
    SpecificationContentProviderPtr Clone() const {BeMutexHolder lock(GetMutex()); return new SpecificationContentProvider(*this);}
    ECPRESENTATION_EXPORT void SetContentDescriptor(ContentDescriptorCR descriptor);
    void InvalidateDescriptor() {m_descriptor = nullptr;}

    ECPRESENTATION_EXPORT IDataSourceCPtr<DisplayValueGroupCPtr> GetDistinctValues(ContentDescriptor::Field const&) const;
};

/*=================================================================================**//**
* Content provider used to get nested content based on NestedContentField.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NestedContentProvider : ContentProvider
{
private:
    ContentDescriptor::NestedContentField const& m_field;
    bvector<ECClassInstanceKey> m_primaryInstanceKeys;
    mutable std::unique_ptr<ContentQuerySet> m_unfilteredQueries;
    mutable std::unique_ptr<ContentQuerySet> m_queries;
    bool m_mergedResults;
private:
    ECPRESENTATION_EXPORT NestedContentProvider(ContentProviderContextR, ContentDescriptor::NestedContentField const&);
    ECPRESENTATION_EXPORT NestedContentProvider(NestedContentProviderCR);
protected:
    ContentDescriptorCP _GetContentDescriptor() const override;
    ContentQuerySet const& _GetContentQuerySet() const override;
    GenericQuerySet _GetCountQuerySet() const override;
    ContentProviderPtr _Clone() const override {return new NestedContentProvider(*this);}
    void _OnDescriptorChanged() override;
public:
    static NestedContentProviderPtr Create(ContentProviderContextR context, ContentDescriptor::NestedContentField const& nestedContentField)
        {
        return new NestedContentProvider(context, nestedContentField);
        }
    ContentDescriptor::NestedContentField const& GetContentField() const {return m_field;}
    void SetPrimaryInstanceKeys(bvector<ECClassInstanceKey> const&);
    void SetPrimaryInstanceKey(ECClassInstanceKeyCR);
    void SetIsResultsMerged(bool mergedResults);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ContentSetDataSource : IDataSource<ContentSetItemCPtr>
{
private:
    ContentProviderCPtr m_contentProvider;

private:
    ContentSetDataSource(ContentProviderCR contentProvider) : m_contentProvider(&contentProvider) {}

protected:
    size_t _GetSize() const override {return m_contentProvider->GetContentSetSize();}
    ContentSetItemCPtr _Get(size_t index) const override
        {
        ContentSetItemPtr item;
        if (!m_contentProvider->GetContentSetItem(item, index))
            return nullptr;
        return item;
        }
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<RandomAccessIteratorImpl<ContentSetDataSource, ContentSetItemCPtr>>(*this));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<RandomAccessIteratorImpl<ContentSetDataSource, ContentSetItemCPtr>>(*this, GetSize()));}

public:
    static ContentSetDataSourcePtr Create(ContentProviderCR contentProvider) {return new ContentSetDataSource(contentProvider);}
    ContentSetItemCPtr operator[](size_t index) const {return Get(index);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
