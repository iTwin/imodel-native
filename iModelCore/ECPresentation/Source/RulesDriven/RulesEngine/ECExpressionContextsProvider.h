/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECExpressionContextsProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECObjects/ECExpressions.h>
#include <ECPresentation/NavNode.h>
#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2015
+===============+===============+===============+===============+===============+======*/
struct ECExpressionContextsProvider : NonCopyableClass
{
    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                04/2016
    +===============+===============+===============+===============+===============+======*/
    struct ContextParametersBase
    {
    private:
        BeSQLite::EC::ECDbCR m_connection;
        IUserSettings const& m_userSettings;
        IUsedUserSettingsListener* m_usedSettingsListener;
    public:
        ContextParametersBase(BeSQLite::EC::ECDbCR connection, IUserSettings const& userSettings, IUsedUserSettingsListener* usedSettingsListener)
            : m_connection(connection), m_userSettings(userSettings), m_usedSettingsListener(usedSettingsListener)
            {}
        BeSQLite::EC::ECDbCR GetConnection() const {return m_connection;}
        IUserSettings const& GetUserSettings() const {return m_userSettings;}
        IUsedUserSettingsListener* GetUsedSettingsListener() const {return m_usedSettingsListener;}
    };

    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                04/2016
    +===============+===============+===============+===============+===============+======*/
    struct NodeRulesContextParameters : ContextParametersBase
    {
    private:
        NavNodeCP m_parentNode;
    public:
        NodeRulesContextParameters(NavNodeCP parentNode, BeSQLite::EC::ECDbCR connection, 
            IUserSettings const& userSettings, IUsedUserSettingsListener* usedSettingsListener)
            : ContextParametersBase(connection, userSettings, usedSettingsListener), m_parentNode(parentNode)
            {}
        NavNodeCP GetParentNode() const {return m_parentNode;}
    };
    
    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                04/2016
    +===============+===============+===============+===============+===============+======*/
    struct ContentRulesContextParameters : ContextParametersBase
    {
    private:
        INavNodeLocaterCR m_nodeLocater;
        Utf8String m_contentDisplayType;
        Utf8String m_selectionProviderName;
        bool m_isSubSelection;
        NavNodeKeyCP m_selectedNodeKey;
    public:
        ContentRulesContextParameters(Utf8CP contentDisplayType, Utf8CP selectionProviderName, bool isSubSelection, BeSQLite::EC::ECDbCR connection, 
            INavNodeLocaterCR nodeLocater, NavNodeKeyCP selectedNodeKey, IUserSettings const& userSettings, IUsedUserSettingsListener* usedSettingsListener)
            : ContextParametersBase(connection, userSettings, usedSettingsListener), m_contentDisplayType(contentDisplayType), 
            m_selectionProviderName(selectionProviderName), m_isSubSelection(isSubSelection), m_nodeLocater(nodeLocater), m_selectedNodeKey(selectedNodeKey)
            {}
        Utf8StringCR GetContentDisplayType() const {return m_contentDisplayType;}
        Utf8StringCR GetSelectionProviderName() const {return m_selectionProviderName;}
        bool IsSubSelection() const {return m_isSubSelection;}
        INavNodeLocaterCR GetNodeLocater() const {return m_nodeLocater;}
        NavNodeKeyCP GetSelectedNodeKey() const {return m_selectedNodeKey;}
    };
    
    /*=================================================================================**//**
    * @bsiclass                                     Grigas.Petraitis                04/2016
    +===============+===============+===============+===============+===============+======*/
    struct CustomizationRulesContextParameters : ContextParametersBase
    {
    private:
        NavNodeCR m_node;
        NavNodeCPtr m_parentNode;
    public:
        CustomizationRulesContextParameters(NavNodeCR node, NavNodeCP parentNode, BeSQLite::EC::ECDbCR connection, 
            IUserSettings const& userSettings, IUsedUserSettingsListener* usedSettingsListener)
            : ContextParametersBase(connection, userSettings, usedSettingsListener), m_node(node), m_parentNode(parentNode)
            {}
        NavNodeCR GetNode() const {return m_node;}
        NavNodeCP GetParentNode() const {return m_parentNode.get();}
    };

private:
    ECExpressionContextsProvider() {};

public:
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetNodeRulesContext(NodeRulesContextParameters const&);
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetContentRulesContext(ContentRulesContextParameters const&);
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetCustomizationRulesContext(CustomizationRulesContextParameters const&);
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetCalculatedPropertyContext(NavNodePtr const&, IUserSettings const&);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct ECExpressionsCache
{
private:
    bmap<Utf8String, NodePtr> m_cache;

public:
    ECExpressionsCache() {}
    NodePtr Get(Utf8CP expression) const;
    void Add(Utf8CP expression, Node&);
    void Clear();
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct IECExpressionsCacheProvider
{
protected:
    virtual ECExpressionsCache& _Get(Utf8CP rulesetId) = 0;
public:
    virtual ~IECExpressionsCacheProvider() {}
    ECExpressionsCache& Get(Utf8CP rulesetId) {return _Get(rulesetId);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct ECExpressionsHelper : NonCopyableClass
{
private:
    ECExpressionsCache& m_cache;
public:
    ECExpressionsHelper(ECExpressionsCache& cache) : m_cache(cache) {}
    bool EvaluateECExpression(ECValueR result, Utf8StringCR expression, ExpressionContextR context);
    ECPRESENTATION_EXPORT NodePtr GetNodeFromExpression(Utf8CP expression);
    ECPRESENTATION_EXPORT Utf8String ConvertToECSql(Utf8StringCR expression);
    ECPRESENTATION_EXPORT bvector<Utf8String> GetUsedClasses(Utf8StringCR expression);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
