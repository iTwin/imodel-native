/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationTest.h"
#include <ECPresentation/RulesDriven/RuleSetLocater.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2015
+===============+===============+===============+===============+===============+======*/
struct TestRuleSetLocater : RefCounted<RuleSetLocater>
{
private:
    bvector<PresentationRuleSetPtr> m_rulesets;
    int m_priority;
    IConnectionCP m_designatedConnection;
    TestRuleSetLocater() : m_priority(9999), m_designatedConnection(nullptr) {}

protected:
    bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
    bvector<Utf8String> _GetRuleSetIds() const override;
    void _InvalidateCache(Utf8CP rulesetId) override;
    int _GetPriority() const override {return m_priority;}
    IConnectionCP _GetDesignatedConnection() const override {return m_designatedConnection;}

public:
    ~TestRuleSetLocater();
    static RefCountedPtr<TestRuleSetLocater> Create() {return new TestRuleSetLocater();}
    void AddRuleSet(PresentationRuleSetR ruleset);
    void SetPriority(int priority) {m_priority = priority;}
    void SetDesignatedConnection(IConnectionCP connection) {m_designatedConnection = connection;}
    void Clear();
};
typedef RefCountedPtr<TestRuleSetLocater> TestRuleSetLocaterPtr;

/*=================================================================================**//**
* The locater creates the supplied ruleset only when it's _LocateRulesets method is called.
* @bsiclass                                     Grigas.Petraitis                06/2019
+===============+===============+===============+===============+===============+======*/
struct DelayLoadingRuleSetLocater : RefCounted<RuleSetLocater>
    {
    private:
        PresentationRuleSetPtr m_ruleset;
        mutable bool m_notified;

    private:
        DelayLoadingRuleSetLocater(PresentationRuleSetR ruleset) : m_ruleset(&ruleset), m_notified(false) {}

    protected:
        bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
        bvector<Utf8String> _GetRuleSetIds() const override;
        void _InvalidateCache(Utf8CP rulesetId) override { Reset(); }
        int _GetPriority() const override { return 0; }

    public:
        ~DelayLoadingRuleSetLocater() { Reset(); }
        static RefCountedPtr<DelayLoadingRuleSetLocater> Create(PresentationRuleSetR ruleset) { return new DelayLoadingRuleSetLocater(ruleset); }
        void Reset();
    };
typedef RefCountedPtr<DelayLoadingRuleSetLocater> DelayLoadingRuleSetLocaterPtr;


END_ECPRESENTATIONTESTS_NAMESPACE
