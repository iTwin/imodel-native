/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ecprovider.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ecprovider.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t        IECProvider::GetProviderId () const
    {
    return _GetProviderId ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         IECProvider::GetProviderName () const
    {
    return _GetProviderName ();
    }
