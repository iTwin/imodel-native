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
struct          IAUIProvider
    {
    protected:
        virtual UInt16          _GetProviderId(void) const = 0;
        virtual WCharCP         _GetProviderName () const = 0;
        
    public:
        //! Get a unique identifier associated with this  UI Provider
        ECOBJECTS_EXPORT    UInt16  GetProviderId () const;

        //! Get a unique name associated with this  UI Provider
        ECOBJECTS_EXPORT    WCharCP GetProviderName () const;
    };

END_BENTLEY_EC_NAMESPACE