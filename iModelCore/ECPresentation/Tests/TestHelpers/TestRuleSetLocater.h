/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/TestHelpers/TestRuleSetLocater.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    TestRuleSetLocater() {}

protected:
    bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
    bvector<Utf8String> _GetRuleSetIds() const override;
    void _InvalidateCache(Utf8CP rulesetId) override;
    int _GetPriority() const override {return 9999;}

public:
    ~TestRuleSetLocater();
    static RefCountedPtr<TestRuleSetLocater> Create() {return new TestRuleSetLocater();}
    void AddRuleSet(PresentationRuleSetR ruleset);
    void Clear();
};
typedef RefCountedPtr<TestRuleSetLocater> TestRuleSetLocaterPtr;

END_ECPRESENTATIONTESTS_NAMESPACE