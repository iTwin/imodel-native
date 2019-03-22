/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECInstanceChangesDirector.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/RulesDriven/ECInstanceChangeHandlers.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct ECInstanceChangesDirector
{
    typedef bset<IECInstanceChangeHandlerPtr, IECInstanceChangeHandlerPtrComparer> HandlersSet;

private:
    HandlersSet const& m_handlers;
    ILocalizationProvider const* m_provider;

public:
    ECInstanceChangesDirector(HandlersSet const& handlers, ILocalizationProvider const* provider)
        : m_handlers(handlers), m_provider(provider)
        {}
    bvector<ECInstanceChangeResult> Handle(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
