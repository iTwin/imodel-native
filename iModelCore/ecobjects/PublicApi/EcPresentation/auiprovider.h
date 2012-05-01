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
struct          ECPresentationProvider
    {
    enum ProviderType
        {
        ViewService     = 1,
        ContentService  = 1<<1,
        CommandService  = 1<<2,
        ImageService    = 1<<3,
        };

    protected:
        virtual UInt16          _GetProviderId(void) const = 0;
        virtual WCharCP         _GetProviderName (void) const = 0;
        virtual ProviderType    _GetProviderType (void) const = 0;

    public:
        //! Get a unique identifier associated with this  UI Provider
        ECOBJECTS_EXPORT    UInt16  GetProviderId () const;

        //! Get a unique name associated with this  UI Provider
        ECOBJECTS_EXPORT    WCharCP GetProviderName () const;

        //! Get a unique name associated with this  UI Provider
        ECOBJECTS_EXPORT    ProviderType GetProviderType () const;
    };

END_BENTLEY_EC_NAMESPACE