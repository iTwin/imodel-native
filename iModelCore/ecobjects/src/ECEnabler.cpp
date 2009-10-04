/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECEnabler.cpp $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                Enabler::Initialize()
    {
    ECAssert (!m_initialized && "You should only call Initialize() once. It doesn't really hurt, but it probably means you have a logic problem somewhere.");
    m_initialized = true;

    // amortize the cost of the dynamic_cast over a lifetime of get/set calls
    m_iGetValue = dynamic_cast<IGetValueCP>(this);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IGetValueCP         Enabler::GetIGetValue() const
    {
    ECAssert (m_initialized && "You should call Initialize() in the constructor of your subclass of Enabler. "
                               "It performs initialization that cannot be performed in the base constructor.");
    return m_iGetValue;
    }
    
END_BENTLEY_EC_NAMESPACE
    