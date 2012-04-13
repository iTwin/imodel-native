/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auiprovider.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IAUIProvider
    {
    protected:
        virtual UInt16          _GetProviderId(void) const = 0;
        virtual WCharCP         _GetProviderName () const = 0;
        virtual IAUIItemPtr     _GetUIItem (IAUIItemInfoCR itemInfo, IAUIDataContextCP instanceData) = 0;
        
    public:
        ECOBJECTS_EXPORT IAUIItemPtr     GetUIItem (IAUIItemInfoCR itemInfo, IAUIDataContextCP instanceData);
    };

END_BENTLEY_EC_NAMESPACE