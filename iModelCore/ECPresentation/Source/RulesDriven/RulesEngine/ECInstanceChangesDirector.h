/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECInstanceChangesDirector.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

public:
    ECInstanceChangesDirector(HandlersSet const& handlers)
        : m_handlers(handlers)
        {}
    bvector<ECInstanceChangeResult> Handle(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
