/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once 
#include "RulesDrivenProviderContext.h"
#include "RulesPreprocessor.h"
#include "QueryExecutor.h"
#include "QueryBuilder.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ContentProviderContext : RulesDrivenProviderContext
{
private:
    // General
    Utf8String m_preferredDisplayType;
    int m_contentFlags;
    INavNodeLocaterCR m_nodesLocater;
    IPropertyCategorySupplierCR m_categorySupplier;
    bool m_isNestedContent;
    INavNodeKeysContainerCPtr m_inputNodeKeys;

    // Selection info context
    bool m_isSelectionContext;
    SelectionInfoCPtr m_selectionInfo;

private:
    void Init();
    ECPRESENTATION_EXPORT ContentProviderContext(PresentationRuleSetCR, Utf8String, Utf8String, int, INavNodeKeysContainerCR, INavNodeLocaterCR, IPropertyCategorySupplierCR, IUserSettings const&,
        ECExpressionsCache&, RelatedPathsCache&, PolymorphicallyRelatedClassesCache&, JsonNavNodesFactory const&, IJsonLocalState const*);
    ECPRESENTATION_EXPORT ContentProviderContext(ContentProviderContextCR other);
    
public:
    static ContentProviderContextPtr Create(PresentationRuleSetCR ruleset, Utf8String locale, Utf8String preferredDisplayType, int contentFlags, INavNodeKeysContainerCR inputKeys, 
        INavNodeLocaterCR nodesLocater, IPropertyCategorySupplierCR categorySupplier, IUserSettings const& settings, ECExpressionsCache& ecexpressionsCache,
        RelatedPathsCache& relatedPathsCache, PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache, 
        JsonNavNodesFactory const& nodesFactory, IJsonLocalState const* localState)
        {
        return new ContentProviderContext(ruleset, locale, preferredDisplayType, contentFlags, inputKeys, nodesLocater, 
            categorySupplier, settings, ecexpressionsCache, relatedPathsCache, polymorphicallyRelatedClassesCache, nodesFactory, localState);
        }
    static ContentProviderContextPtr Create(ContentProviderContextCR other) {return new ContentProviderContext(other);}
    ~ContentProviderContext();

    // General
    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    int GetContentFlags() const {return m_contentFlags;}
    INavNodeLocaterCR GetNodesLocater() const {return m_nodesLocater;}
    IPropertyCategorySupplierCR GetCategorySupplier() const {return m_categorySupplier;}
    bool IsNestedContent() const {return m_isNestedContent;}
    void SetIsNestedContent(bool value) {m_isNestedContent = value;}
    INavNodeKeysContainerCR GetInputKeys() const {return *m_inputNodeKeys;}
    void SetInputKeys(INavNodeKeysContainerCR inputNodeKeys) {m_inputNodeKeys = &inputNodeKeys;}
    
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
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ContentProvider : RefCountedBase
{    
private:
    ContentProviderContextPtr m_context;
    PageOptions m_pageOptions;
    ContentQueryExecutor* m_executor;
    bvector<ContentSetItemPtr> m_records;
    bool m_initialized;
    mutable size_t m_contentSetSize;
    mutable bool m_fullContentSetSizeDetermined;
    mutable bmap<ContentDescriptor::NestedContentField const*, NestedContentProviderPtr> m_nestedContentProviders;

private:
    NestedContentProviderPtr GetNestedContentProvider(ContentDescriptor::NestedContentField const&, bool) const;
    void LoadNestedContent(ContentSetItemR) const;
    void LoadNestedContentFieldValue(ContentSetItemR, ContentDescriptor::NestedContentField const&, bool) const;
    void LoadCompositePropertiesFieldValue(ContentSetItemR, ContentDescriptor::ECPropertiesField const&) const;
    
protected:
    ECPRESENTATION_EXPORT ContentProvider(ContentProviderContextR);
    ECPRESENTATION_EXPORT ContentProvider(ContentProviderCR);
    ECPRESENTATION_EXPORT ~ContentProvider();
    virtual ContentDescriptorCP _GetContentDescriptor() const = 0;
    virtual ContentQueryCPtr _GetQuery() const = 0;
    virtual ContentProviderPtr _Clone() const = 0;
    virtual void _Reset();

public:
    void Initialize();
    ContentProviderPtr Clone() const {return _Clone();}

    ContentProviderContextR GetContextR() const {return *m_context;}
    ContentProviderContextCR GetContext() const {return GetContextR();}
    
    ContentDescriptorCP GetContentDescriptor() const {return _GetContentDescriptor();}

    PageOptionsCR GetPageOptions() const {return m_pageOptions;}
    ECPRESENTATION_EXPORT void SetPageOptions(PageOptions options);

    ECPRESENTATION_EXPORT bool GetContentSetItem(ContentSetItemPtr& item, size_t index) const;
    ECPRESENTATION_EXPORT size_t GetContentSetSize() const;
    ECPRESENTATION_EXPORT size_t GetFullContentSetSize() const;

    void InvalidateContent();
};

/*=================================================================================**//**
* Content provider used to get content based on presentation rule specifications.
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct SpecificationContentProvider : ContentProvider
{
private:
    ContentRuleInstanceKeysList m_rules;
    mutable ContentDescriptorCPtr m_descriptor;
    mutable ContentQueryPtr m_query;
    mutable bmap<ContentRuleCP, IParsedInput const*> m_inputCache;
private:
    ECPRESENTATION_EXPORT SpecificationContentProvider(ContentProviderContextR, ContentRuleInstanceKeysList);
    ECPRESENTATION_EXPORT SpecificationContentProvider(SpecificationContentProviderCR);
    ContentQueryCPtr CreateQuery(ContentDescriptorCP) const;
protected:
    ContentDescriptorCP _GetContentDescriptor() const override;
    ContentQueryCPtr _GetQuery() const override;
    ContentProviderPtr _Clone() const override {return new SpecificationContentProvider(*this);}
    void _Reset() override;
public:
    static SpecificationContentProviderPtr Create(ContentProviderContextR context, ContentRuleInstanceKeysList const& specs) {return new SpecificationContentProvider(context, specs);}
    static SpecificationContentProviderPtr Create(ContentProviderContextR context, ContentRuleInstanceKeys const& spec)
        {
        ContentRuleInstanceKeysList specs;
        specs.insert(spec);
        return SpecificationContentProvider::Create(context, specs);
        }
    ~SpecificationContentProvider();
    SpecificationContentProviderPtr Clone() const {return new SpecificationContentProvider(*this);}
    
    ECPRESENTATION_EXPORT void SetContentDescriptor(ContentDescriptorCR descriptor);
};

/*=================================================================================**//**
* Content provider used to get nested content based on NestedContentField.
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct NestedContentProvider : ContentProvider
{
private:
    ContentDescriptor::NestedContentField const& m_field;
    bvector<ECClassInstanceKey> m_primaryInstanceKeys;
    mutable ContentQueryCPtr m_query;
    mutable ContentQueryCPtr m_adjustedQuery;
    bool m_mergedResults;
private:
    ECPRESENTATION_EXPORT NestedContentProvider(ContentProviderContextR, ContentDescriptor::NestedContentField const&);
    ECPRESENTATION_EXPORT NestedContentProvider(NestedContentProviderCR);
    ContentQueryCPtr CreateQuery(ContentDescriptor::NestedContentField const&) const;
protected:
    ContentDescriptorCP _GetContentDescriptor() const override;
    ContentQueryCPtr _GetQuery() const override;
    ContentProviderPtr _Clone() const override {return new NestedContentProvider(*this);}
    void _Reset() override;
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
* @bsiclass                                     Grigas.Petraitis                04/2016
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
