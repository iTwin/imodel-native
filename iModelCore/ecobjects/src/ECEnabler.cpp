/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECEnabler.cpp $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <assert.h>

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Must be called from the constructor of your Enabler.
* It cannot be called in the base constructor because you cannot dynamic_cast to
* a derived type in the base constructor.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                Enabler::Initialize()
    {
    assert (!m_initialized && "You should only call Initialize() once. It doesn't really hurt, but it probably means you have a logic problem somewhere.");
    m_initialized = true;
    
    // amortize the cost of the dynamic_cast over a lifetime of get/set calls
    m_iGetValue = dynamic_cast<IGetValueCP>(this);
    }

/*---------------------------------------------------------------------------------**//**
* Call this method rather than dynamic_casting the EC::Enabler on your own, because
* dynamic_cast is more expensive than you might have imagined, and it is better to do
* it once and save the result.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IGetValueCP         Enabler::IGetValue() const
    {
    assert (m_initialized && "You should call Initialize() in the constructor of your subclass of Enabler. "
                               "It performs initialization that cannot be performed in the base constructor.");
    return m_iGetValue;
    }
    
END_BENTLEY_EC_NAMESPACE
    