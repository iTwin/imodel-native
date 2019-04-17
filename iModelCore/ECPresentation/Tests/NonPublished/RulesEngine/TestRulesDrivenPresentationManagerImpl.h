/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include "../../../Source/RulesDriven/RulesEngine/PresentationManagerImpl.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestRulesDrivenECPresentationManagerImpl : RulesDrivenECPresentationManager::Impl
{
    typedef std::function<IRulesPreprocessorPtr(IConnectionCR, Utf8StringCR, Utf8StringCR, IUsedUserSettingsListener*)> Handler_GetRulesPreprocessor;

    typedef std::function<INavNodesDataSourcePtr(IConnectionCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetRootNodes;
    typedef std::function<size_t(IConnectionCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetRootNodesCount;
    typedef std::function<INavNodesDataSourcePtr(IConnectionCR, NavNodeCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetChildNodes;
    typedef std::function<size_t(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetChildNodesCount;
    typedef std::function<bool(IConnectionCR, NavNodeCR, ECInstanceKeyCR, NavigationOptions const&, ICancelationTokenCR)> Handler_HasChild;
    typedef std::function<NavNodeCPtr(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetParent;
    typedef std::function<NavNodeCPtr(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetNode;
    typedef std::function<bvector<NavNodeCPtr>(IConnectionCR, Utf8CP, NavigationOptions const&, ICancelationTokenCR)> Handler_GetFilteredNodes;
    typedef std::function<void(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR)> Handler_OnNodeChecked;
    typedef std::function<void(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR)> Handler_OnNodeUnchecked;
    typedef std::function<void(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR)> Handler_OnNodeExpanded;
    typedef std::function<void(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR)> Handler_OnNodeCollapsed;
    typedef std::function<void(IConnectionCR, NavigationOptions const&, ICancelationTokenCR)> Handler_OnAllNodesCollapsed;

    typedef std::function<bvector<SelectClassInfo>(IConnectionCR, Utf8CP, bvector<ECClassCP> const&, ContentOptions const&, ICancelationTokenCR)> Handler_GetContentClasses;
    typedef std::function<ContentDescriptorCPtr(IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, ContentOptions const&, ICancelationTokenCR)> Handler_GetContentDescriptor;
    typedef std::function<ContentCPtr(ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)> Handler_GetContent;
    typedef std::function<size_t(ContentDescriptorCR, ICancelationTokenCR)> Handler_GetContentSetSize;
    typedef std::function<Utf8String(IConnectionCR, KeySetCR, ICancelationTokenCR)> Handler_GetDisplayLabel;

    typedef std::function<bvector<ECInstanceChangeResult>(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR)> Handler_SaveValueChange;

    typedef std::function<void()> Handler_OnUpdateRecordsHandlerChanged;
    typedef std::function<void(ECInstanceChangeEventSource&)> Handler_OnECInstanceChangeEventSourceRegistered;
    typedef std::function<void(ECInstanceChangeEventSource&)> Handler_OnECInstanceChangeEventSourceUnregister;
    typedef std::function<void()> Handler_OnCategoriesChanged;

private:
    Handler_GetRulesPreprocessor m_rulesPreprocessorHandler;

    Handler_GetRootNodes m_rootNodesHandler;
    Handler_GetRootNodesCount m_rootNodesCountHandler;
    Handler_GetChildNodes m_childNodesHandler;
    Handler_GetChildNodesCount m_childNodesCountHandler;
    Handler_HasChild m_hasChildHandler;
    Handler_GetParent m_getParentHandler;
    Handler_GetNode m_getNodeHandler;
    Handler_GetFilteredNodes m_filteredNodesHandler;
    Handler_OnNodeChecked m_nodeCheckedHandler;
    Handler_OnNodeUnchecked m_nodeUncheckedHandler;
    Handler_OnNodeExpanded m_nodeExpandedHandler;
    Handler_OnNodeCollapsed m_nodeCollapsedHandler;
    Handler_OnAllNodesCollapsed m_allNodesCollapsedHandler;

    Handler_GetContentClasses m_contentClassesHandler;
    Handler_GetContentDescriptor m_contentDescriptorHandler;
    Handler_GetContent m_contentHandler;
    Handler_GetContentSetSize m_contentSetSizeHandler;
    Handler_GetDisplayLabel m_displayLabelHandler;

    Handler_SaveValueChange m_saveValueChangeHandler;

    Handler_OnUpdateRecordsHandlerChanged m_updateRecordsHandlerChangedHandler;
    Handler_OnECInstanceChangeEventSourceRegistered m_ecInstanceChangeEventSourceRegisteredHandler;
    Handler_OnECInstanceChangeEventSourceUnregister m_ecInstanceChangeEventSourceUnregisterHandler;
    Handler_OnCategoriesChanged m_categoriesChangedHandler;

protected:
    IRulesPreprocessorPtr _GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener) const override
        {
        if (m_rulesPreprocessorHandler)
            return m_rulesPreprocessorHandler(connection, rulesetId, locale, usedSettingsListener);
        return nullptr;
        }

    INavNodesDataSourcePtr _GetRootNodes(IConnectionCR connection, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_rootNodesHandler)
            return m_rootNodesHandler(connection, pageOptions, options, cancelationToken);
        return nullptr;
        }
    size_t _GetRootNodesCount(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_rootNodesCountHandler)
            return m_rootNodesCountHandler(connection, options, cancelationToken);
        return 0;
        }
    INavNodesDataSourcePtr _GetChildren(IConnectionCR connection, NavNodeCR parent, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_childNodesHandler)
            return m_childNodesHandler(connection, parent, pageOptions, options, cancelationToken);
        return nullptr;
        }
    size_t _GetChildrenCount(IConnectionCR connection, NavNodeCR parent, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_childNodesCountHandler)
            return m_childNodesCountHandler(connection, parent, options, cancelationToken);
        return 0;
        }
    bool _HasChild(IConnectionCR connection, NavNodeCR parent, ECInstanceKeyCR childKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_hasChildHandler)
            return m_hasChildHandler(connection, parent, childKey, options, cancelationToken);
        return false;
        }
    NavNodeCPtr _GetParent(IConnectionCR connection, NavNodeCR child, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_getParentHandler)
            return m_getParentHandler(connection, child, options, cancelationToken);
        return nullptr;
        }
    NavNodeCPtr _GetNode(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_getNodeHandler)
            return m_getNodeHandler(connection, nodeKey, options, cancelationToken);
        return nullptr;
        }
    bvector<NavNodeCPtr> _GetFilteredNodes(IConnectionCR connection, Utf8CP filterText, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_filteredNodesHandler)
            return m_filteredNodesHandler(connection, filterText, options, cancelationToken);
        return bvector<NavNodeCPtr>();
        }
    void _OnNodeChecked(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_nodeCheckedHandler)
            return m_nodeCheckedHandler(connection, nodeKey, options, cancelationToken);
        }
    void _OnNodeUnchecked(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_nodeUncheckedHandler)
            return m_nodeUncheckedHandler(connection, nodeKey, options, cancelationToken);
        }
    void _OnNodeExpanded(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_nodeExpandedHandler)
            return m_nodeExpandedHandler(connection, nodeKey, options, cancelationToken);
        }
    void _OnNodeCollapsed(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_nodeCollapsedHandler)
            return m_nodeCollapsedHandler(connection, nodeKey, options, cancelationToken);
        }
    void _OnAllNodesCollapsed(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_allNodesCollapsedHandler)
            return m_allNodesCollapsedHandler(connection, options, cancelationToken);
        }

    bvector<SelectClassInfo> _GetContentClasses(IConnectionCR connection, Utf8CP displayType, bvector<ECClassCP> const& inputClasses, ContentOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_contentClassesHandler)
            return m_contentClassesHandler(connection, displayType, inputClasses, options, cancelationToken);
        return bvector<SelectClassInfo>();
        }
    ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR connection, Utf8CP displayType, KeySetCR inputKeys, SelectionInfo const* selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_contentDescriptorHandler)
            return m_contentDescriptorHandler(connection, displayType, inputKeys, selectionInfo, options, cancelationToken);
        return nullptr;
        }
    ContentCPtr _GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions, ICancelationTokenCR cancelationToken) override
        {
        if (m_contentHandler)
            return m_contentHandler(descriptor, pageOptions, cancelationToken);
        return nullptr;
        }
    size_t _GetContentSetSize(ContentDescriptorCR descriptor, ICancelationTokenCR cancelationToken) override
        {
        if (m_contentSetSizeHandler)
            return m_contentSetSizeHandler(descriptor, cancelationToken);
        return 0;
        }
    Utf8String _GetDisplayLabel(IConnectionCR connection, KeySetCR key, ICancelationTokenCR cancelationToken) override
        {
        if (m_displayLabelHandler)
            return m_displayLabelHandler(connection, key, cancelationToken);
        return "";
        }

    bvector<ECInstanceChangeResult> _SaveValueChange(IConnectionCR connection, bvector<ChangedECInstanceInfo> const& changedInstances, Utf8CP propertyAccessor, ECValueCR value) override
        {
        if (m_saveValueChangeHandler)
            return m_saveValueChangeHandler(connection, changedInstances, propertyAccessor, value);
        return bvector<ECInstanceChangeResult>();
        }

    void _OnUpdateRecordsHandlerChanged() override
        {
        if (m_updateRecordsHandlerChangedHandler)
            m_updateRecordsHandlerChangedHandler();
        }
    void _OnECInstanceChangeEventSourceRegistered(ECInstanceChangeEventSource& source) override
        {
        if (m_ecInstanceChangeEventSourceRegisteredHandler)
            m_ecInstanceChangeEventSourceRegisteredHandler(source);
        }
    void _OnECInstanceChangeEventSourceUnregister(ECInstanceChangeEventSource& source) override
        {
        if (m_ecInstanceChangeEventSourceUnregisterHandler)
            m_ecInstanceChangeEventSourceUnregisterHandler(source);
        }
    void _OnCategoriesChanged() override
        {
        if (m_categoriesChangedHandler)
            m_categoriesChangedHandler();
        }

public:
    TestRulesDrivenECPresentationManagerImpl(IRulesDrivenECPresentationManagerDependenciesFactory const& dependenciesFactory, Params const& params)
        : RulesDrivenECPresentationManager::Impl(dependenciesFactory, params)
        {}

    void SetRulesPreprocessorHandler(Handler_GetRulesPreprocessor handler) {m_rulesPreprocessorHandler = handler;}

    void SetRootNodesHandler(Handler_GetRootNodes handler) {m_rootNodesHandler = handler;}
    void SetRootNodesCountHandler(Handler_GetRootNodesCount handler) {m_rootNodesCountHandler = handler;}
    void SetChildNodesHandler(Handler_GetChildNodes handler) {m_childNodesHandler = handler;}
    void SetChildNodesCountHandler(Handler_GetChildNodesCount handler) {m_childNodesCountHandler = handler;}
    void SetHasChildHandler(Handler_HasChild handler) {m_hasChildHandler = handler;}
    void SetGetParentHandler(Handler_GetParent handler) {m_getParentHandler = handler;}
    void SetGetNodeHandler(Handler_GetNode handler) {m_getNodeHandler = handler;}
    void SetGetFilteredNodesHandler(Handler_GetFilteredNodes handler) {m_filteredNodesHandler = handler;}
    void SetNodeCheckedHandler(Handler_OnNodeChecked handler) {m_nodeCheckedHandler = handler;}
    void SetNodeUncheckedHandler(Handler_OnNodeUnchecked handler) {m_nodeUncheckedHandler = handler;}
    void SetNodeExpandedHandler(Handler_OnNodeExpanded handler) {m_nodeExpandedHandler = handler;}
    void SetNodeCollapsedHandler(Handler_OnNodeCollapsed handler) {m_nodeCollapsedHandler = handler;}
    void SetAllNodesCollapsedHandler(Handler_OnAllNodesCollapsed handler) {m_allNodesCollapsedHandler = handler;}

    void SetContentClassesHandler(Handler_GetContentClasses handler) {m_contentClassesHandler = handler;}
    void SetContentDescriptorHandler(Handler_GetContentDescriptor handler) {m_contentDescriptorHandler = handler;}
    void SetContentHandler(Handler_GetContent handler) {m_contentHandler = handler;}
    void SetContentSetSizeHandler(Handler_GetContentSetSize handler) {m_contentSetSizeHandler = handler;}
    void SetDisplayLabelHandler(Handler_GetDisplayLabel handler) {m_displayLabelHandler = handler;}

    void SetSaveValueChangeHandler(Handler_SaveValueChange handler) {m_saveValueChangeHandler = handler;}

    void SetUpdateRecordsHandlerChangedHandler(Handler_OnUpdateRecordsHandlerChanged handler) {m_updateRecordsHandlerChangedHandler = handler;}
    void SetSetECInstanceChangeEventSourceRegisteredHandler(Handler_OnECInstanceChangeEventSourceRegistered handler) {m_ecInstanceChangeEventSourceRegisteredHandler = handler;}
    void SetECInstanceChangeEventSourceUnregisterHandler(Handler_OnECInstanceChangeEventSourceUnregister handler) {m_ecInstanceChangeEventSourceUnregisterHandler = handler;}
    void SetCategoriesChangedHandler(Handler_OnCategoriesChanged handler) {m_categoriesChangedHandler = handler;}
};

END_ECPRESENTATIONTESTS_NAMESPACE
