/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include "RulesDrivenECPresentationManagerImplBase.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestRulesDrivenECPresentationManagerImpl : RulesDrivenECPresentationManagerImplBase
{
    typedef std::function<IRulesPreprocessorPtr(IConnectionCR, Utf8StringCR, Utf8StringCR, IUsedUserSettingsListener*)> Handler_GetRulesPreprocessor;

    typedef std::function<INavNodesDataSourcePtr(IConnectionCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetRootNodes;
    typedef std::function<size_t(IConnectionCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetRootNodesCount;
    typedef std::function<INavNodesDataSourcePtr(IConnectionCR, NavNodeCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetChildNodes;
    typedef std::function<size_t(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetChildNodesCount;
    typedef std::function<NavNodeCPtr(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetParent;
    typedef std::function<NavNodeCPtr(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR)> Handler_GetNode;
    typedef std::function<bvector<NavNodeCPtr>(IConnectionCR, Utf8CP, NavigationOptions const&, ICancelationTokenCR)> Handler_GetFilteredNodes;

    typedef std::function<bvector<SelectClassInfo>(IConnectionCR, Utf8CP, int, bvector<ECClassCP> const&, ContentOptions const&, ICancelationTokenCR)> Handler_GetContentClasses;
    typedef std::function<ContentDescriptorCPtr(IConnectionCR, Utf8CP, int, KeySetCR, SelectionInfo const*, ContentOptions const&, ICancelationTokenCR)> Handler_GetContentDescriptor;
    typedef std::function<ContentCPtr(IConnectionCR, ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)> Handler_GetContent;
    typedef std::function<size_t(IConnectionCR, ContentDescriptorCR, ICancelationTokenCR)> Handler_GetContentSetSize;
    typedef std::function<LabelDefinitionCPtr(IConnectionCR, KeySetCR, ICancelationTokenCR)> Handler_GetDisplayLabel;

    typedef std::function<void()> Handler_OnUpdateRecordsHandlerChanged;
    typedef std::function<void(ECInstanceChangeEventSource&)> Handler_OnECInstanceChangeEventSourceRegistered;
    typedef std::function<void(ECInstanceChangeEventSource&)> Handler_OnECInstanceChangeEventSourceUnregister;

private:
    Handler_GetRootNodes m_rootNodesHandler;
    Handler_GetRootNodesCount m_rootNodesCountHandler;
    Handler_GetChildNodes m_childNodesHandler;
    Handler_GetChildNodesCount m_childNodesCountHandler;
    Handler_GetParent m_getParentHandler;
    Handler_GetNode m_getNodeHandler;
    Handler_GetFilteredNodes m_filteredNodesHandler;

    Handler_GetContentClasses m_contentClassesHandler;
    Handler_GetContentDescriptor m_contentDescriptorHandler;
    Handler_GetContent m_contentHandler;
    Handler_GetContentSetSize m_contentSetSizeHandler;
    Handler_GetDisplayLabel m_displayLabelHandler;

protected:
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

    bvector<SelectClassInfo> _GetContentClasses(IConnectionCR connection, Utf8CP displayType, int contentFlags, bvector<ECClassCP> const& inputClasses, ContentOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_contentClassesHandler)
            return m_contentClassesHandler(connection, displayType, contentFlags, inputClasses, options, cancelationToken);
        return bvector<SelectClassInfo>();
        }
    ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR connection, Utf8CP displayType, int contentFlags, KeySetCR inputKeys, SelectionInfo const* selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken) override
        {
        if (m_contentDescriptorHandler)
            return m_contentDescriptorHandler(connection, displayType, contentFlags, inputKeys, selectionInfo, options, cancelationToken);
        return nullptr;
        }
    ContentCPtr _GetContent(IConnectionCR connection, ContentDescriptorCR descriptor, PageOptionsCR pageOptions, ICancelationTokenCR cancelationToken) override
        {
        if (m_contentHandler)
            return m_contentHandler(connection, descriptor, pageOptions, cancelationToken);
        return nullptr;
        }
    size_t _GetContentSetSize(IConnectionCR connection, ContentDescriptorCR descriptor, ICancelationTokenCR cancelationToken) override
        {
        if (m_contentSetSizeHandler)
            return m_contentSetSizeHandler(connection, descriptor, cancelationToken);
        return 0;
        }
    LabelDefinitionCPtr _GetDisplayLabel(IConnectionCR connection, KeySetCR key, ICancelationTokenCR cancelationToken) override
        {
        if (m_displayLabelHandler)
            return m_displayLabelHandler(connection, key, cancelationToken);
        return LabelDefinition::Create();
        }

public:
    TestRulesDrivenECPresentationManagerImpl(RulesDrivenECPresentationManager::Params const& params)
        : RulesDrivenECPresentationManagerImplBase(params)
        {}

    void SetRootNodesHandler(Handler_GetRootNodes handler) {m_rootNodesHandler = handler;}
    void SetRootNodesCountHandler(Handler_GetRootNodesCount handler) {m_rootNodesCountHandler = handler;}
    void SetChildNodesHandler(Handler_GetChildNodes handler) {m_childNodesHandler = handler;}
    void SetChildNodesCountHandler(Handler_GetChildNodesCount handler) {m_childNodesCountHandler = handler;}
    void SetGetParentHandler(Handler_GetParent handler) {m_getParentHandler = handler;}
    void SetGetNodeHandler(Handler_GetNode handler) {m_getNodeHandler = handler;}
    void SetGetFilteredNodesHandler(Handler_GetFilteredNodes handler) {m_filteredNodesHandler = handler;}

    void SetContentClassesHandler(Handler_GetContentClasses handler) {m_contentClassesHandler = handler;}
    void SetContentDescriptorHandler(Handler_GetContentDescriptor handler) {m_contentDescriptorHandler = handler;}
    void SetContentHandler(Handler_GetContent handler) {m_contentHandler = handler;}
    void SetContentSetSizeHandler(Handler_GetContentSetSize handler) {m_contentSetSizeHandler = handler;}
    void SetDisplayLabelHandler(Handler_GetDisplayLabel handler) {m_displayLabelHandler = handler;}
};

END_ECPRESENTATIONTESTS_NAMESPACE
