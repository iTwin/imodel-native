/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SymbologyRuleSets.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include <ECObjects/ECExpressionNode.h>
#include <DgnPlatform/DgnCore/ViewDisplayRules.h>


struct RuleDefinition
    {
    WString     m_test;
    WString     m_trueAction;
    WString     m_falseAction;
    bool        m_stopOnAction;
    bool        m_alwaysTrue;

    ECN::NodePtr    m_testNode;
    ECN::NodePtr    m_trueNode;
    ECN::NodePtr    m_falseNode;

    RuleDefinition(WCharCP rule, WCharCP action) : m_test(rule), m_trueAction(action), m_stopOnAction(false), m_alwaysTrue(false) {}
    RuleDefinition(WCharCP rule, WCharCP action, WCharCP elseAction) : m_test(rule), m_trueAction(action), m_falseAction(elseAction), m_stopOnAction(false) {}
    void EvaluateAction(bool& stop, ECN::ExpressionContextR context) const;
    void Parse();
    };

typedef RefCountedPtr<struct RuleSet> RuleSetPtr;
struct RuleSet : RefCountedBase
    {
private:
    //  Rules to initialize display params from element
    bvector<RuleDefinition>    m_rules;

    Utf8String                 m_name;
    RuleSet (Utf8CP name) : m_name(name) {}

public:

    void AppendRule(WCharCP rule, WCharCP action)
        {
        m_rules.push_back(RuleDefinition(rule, action));
        }

    void AppendRule(WCharCP rule, WCharCP action, WCharCP elseAction)
        {
        m_rules.push_back(RuleDefinition(rule, action, elseAction));
        }

    Utf8StringCR GetName() const { return m_name; }
    void ParseRules();
    void PreprocessRules(ECN::ExpressionContextR context) {}
    bvector<RuleDefinition> const& GetRules() { return m_rules; }

    static RuleSetPtr Create(Utf8CP name) { return new RuleSet(name); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void RuleDefinition::Parse()
    {
    if (m_test.size() > 0)
        {
        m_testNode = ECN::ECEvaluator::ParseValueExpressionAndCreateTree(m_test.c_str());
        m_alwaysTrue = false;
        }
    else
        m_alwaysTrue = true;

    BeAssert(m_trueAction.size() > 0);
    //  This performs an assignment or calls a method;
    m_trueNode = ECN::ECEvaluator::ParseAssignmentExpressionAndCreateTree(m_trueAction.c_str());
    if (m_falseAction.size() > 0)
        m_falseNode = ECN::ECEvaluator::ParseAssignmentExpressionAndCreateTree(m_falseAction.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void RuleDefinition::EvaluateAction(bool& stop, ECN::ExpressionContextR context) const
    {
    bool    testTrue = m_alwaysTrue;
    if (!testTrue)
        {
        ECN::EvaluationResult    evalResult;
        m_testNode->GetValue(evalResult, context);
        testTrue = evalResult.GetECValue()->GetBoolean();
        }

    bool performedAction = false;
    if (testTrue)
        {
        performedAction = true;
        ECN::EvaluationResult    evalResult;
        m_trueNode->GetValue(evalResult, context);
        }
    else if (m_falseNode.IsValid())
        {
        performedAction = true;
        ECN::EvaluationResult    evalResult;
        m_falseNode->GetValue(evalResult, context);
        }

    stop = performedAction && m_stopOnAction;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void RuleSet::ParseRules()
    {
    for (RuleDefinition& rule : m_rules)
        rule.Parse();
    }

//---------------------------------------------------------------------------------------
// @bsistruct                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
struct SymbologyRulesImpl : SymbologyRules
    {
private:
    //  This is a list of rules that other rules can invoke.
    bvector<RuleSetPtr>                     m_ruleSets;
    RuleSetPtr                              m_topRules;

    RuleSetPtr AddNewRuleSet(Utf8CP name);

    void InitializeNullParentOverrides();
    void InitializeParentOverrides();
    void InitializeTopRules();

protected:
    virtual void _ParseRules() override;
    virtual void _PreprocessRules(ECN::ExpressionContextR context) override;
    virtual void _Evaluate(ECN::ExpressionContextR context) override;

public:
    SymbologyRulesImpl() {};
    void Initialize();

    static SymbologyRulesImpl* GetRules();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
RuleSetPtr SymbologyRulesImpl::AddNewRuleSet(Utf8CP name)
    {
    RuleSetPtr ruleSet = RuleSet::Create(name);
    m_ruleSets.push_back(ruleSet);
    return ruleSet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
SymbologyRulesImpl* SymbologyRulesImpl::GetRules()
    {
    static SymbologyRulesImpl* s_rules;
    if (nullptr == s_rules)
        {
        s_rules = new SymbologyRulesImpl();
        s_rules->InitializeTopRules();
        }

    return s_rules;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRulesImpl::InitializeNullParentOverrides()
    {
    RuleSetPtr ruleSet = AddNewRuleSet("NullParentOverrides");

    ruleSet->AppendRule(L"displayParams.ColorIsByCell", L"displayParams.Color=0");
    ruleSet->AppendRule(L"displayParams.FillColorIsByCell", L"displayParams.FillColor=0");
    ruleSet->AppendRule(L"displayParams.StyleIsByCell", L"displayParams.Style=level.Style");
    ruleSet->AppendRule(L"displayParams.WeightIsByCell", L"displayParams.Weight=0");
    ruleSet->AppendRule(L"displayParams.PriorityIsByCell", L"displayParams.Priority=0");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRulesImpl::InitializeParentOverrides()
    {
    RuleSetPtr ruleSet = AddNewRuleSet("ParentOverrides");

    ruleSet->AppendRule(nullptr, L"override.AdjustLevel()");
    ruleSet->AppendRule(L"displayParams.ColorIsByCell", L"displayParams.Color=override.Color");
    //  What about fill color?  ApplyByParentOverrides doesn't set it.
    ruleSet->AppendRule(L"displayParams.StyleIsByCell", L"displayParams.Style=override.Style");
    ruleSet->AppendRule(L"displayParams.WeightIsByCell", L"displayParams.Weight=override.Weight");
    ruleSet->AppendRule(L"displayParams.PriorityIsByCell", L"displayParams.Priority=override.Priority");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRulesImpl::InitializeTopRules()
    {
    RuleSetPtr ruleSet = AddNewRuleSet("TopRules");
    m_topRules = ruleSet;

    ruleSet->AppendRule(nullptr, L"InitializeDisplayParamsFromElement()");
    ruleSet->AppendRule(nullptr, L"view.ResolveEffectiveDisplayParams()");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRulesImpl::Initialize()
    {
    InitializeTopRules();
#if defined (NOTNOW)
    //  Set up the rules for resolving the effective display parameters
    m_effectiveDisplayParams.AppendRule(L"context.HasParentOverrides", L"context.ApplyParentOverrides()", L"context.ApplyNoParentOverrides()");
#endif
    InitializeParentOverrides();
    InitializeNullParentOverrides();
    }

//---------------------------------------------------------------------------------------
// @bsistruct                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
struct SymbologyRulesExpressionResolver : ECN::ExpressionResolver
{
    SymbologyRulesExpressionResolver(ECN::ExpressionContextR context) : ECN::ExpressionResolver(context) {}
    virtual ECN::ResolvedTypeNodePtr _ResolvePrimaryList (ECN::PrimaryListNodeR primaryList) override;
    static RefCountedPtr<SymbologyRulesExpressionResolver> Create(ECN::ExpressionContextR context) { return new SymbologyRulesExpressionResolver(context); }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
ECN::ResolvedTypeNodePtr SymbologyRulesExpressionResolver::_ResolvePrimaryList (ECN::PrimaryListNodeR primaryList)
    {
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRulesImpl::_ParseRules()
    {
    for (RuleSetPtr& ruleSet : m_ruleSets)
        {
        ruleSet->ParseRules();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRulesImpl::_PreprocessRules(ECN::ExpressionContextR context)
    {
    RefCountedPtr<SymbologyRulesExpressionResolver> resolver = SymbologyRulesExpressionResolver::Create(context);

    for (RuleSetPtr& ruleSet : m_ruleSets)
        {
        ruleSet->PreprocessRules(context);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRulesImpl::_Evaluate(ECN::ExpressionContextR context)
    {
    bool stop = false;
    for (RuleDefinition const & rule : m_topRules->GetRules())
        {
        rule.EvaluateAction(stop, context);
        if (stop)
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRules::PreprocessRules(ECN::ExpressionContextR context)
    {
    _PreprocessRules(context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRules::ParseRules()
    {
    _ParseRules();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
SymbologyRulesPtr SymbologyRules::Create()
    {
    SymbologyRulesImpl*   rules = new SymbologyRulesImpl();
    rules->Initialize();

    return rules;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void SymbologyRules::Evaluate(ECN::ExpressionContextR context)
    {
    _Evaluate(context);
    }