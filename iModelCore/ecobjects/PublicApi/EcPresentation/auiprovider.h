/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auiprovider.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once


BEGIN_BENTLEY_ECOBJECT_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IECPresentationProvider : public RefCountedBase
    {
    enum ProviderType
        {
        ViewService         = 1,
        ContentService      = 1<<1,
        CommandService      = 1<<2,
        ImageService        = 1<<3,
        LocalizationService = 1<<4,
        SelectionService    = 1<<5,
        JournalService      = 1<<6,
        };

    protected:
        virtual size_t          _GetProviderId(void) const {return reinterpret_cast<size_t>(this);}
        virtual WCharCP         _GetProviderName (void) const = 0;
        virtual ProviderType    _GetProviderType (void) const = 0;

    public:
        //! Get a unique identifier associated with this  UI Provider
        ECOBJECTS_EXPORT    size_t  GetProviderId () const;

        //! Get a unique name associated with this  UI Provider
        ECOBJECTS_EXPORT    WCharCP GetProviderName () const;

        //! Get a unique name associated with this  UI Provider
        ECOBJECTS_EXPORT    ProviderType GetProviderType () const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

//__PUBLISH_SECTION_END__