/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auiprovider.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          IECPresentationProvider::GetProviderId () const
    {
    return _GetProviderId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         IECPresentationProvider::GetProviderName () const
    {
    return _GetProviderName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationProvider::ProviderType    IECPresentationProvider::GetProviderType () const
    {
    return _GetProviderType();
    }
