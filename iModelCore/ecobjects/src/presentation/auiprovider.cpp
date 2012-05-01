/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auiprovider.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16          ECPresentationProvider::GetProviderId () const
    {
    return _GetProviderId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         ECPresentationProvider::GetProviderName () const
    {
    return _GetProviderName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationProvider::ProviderType    ECPresentationProvider::GetProviderType () const
    {
    return _GetProviderType();
    }