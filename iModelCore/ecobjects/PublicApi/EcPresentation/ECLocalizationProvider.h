/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ECLocalizationProvider.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Andrius.Zonys                   05/2012
+===============+===============+===============+===============+===============+======*/
struct ECPresentationLocalizationProvider : public IECPresentationProvider
    {
    protected:
        virtual ProviderType      _GetProviderType() const override {return LocalizationService;}
        virtual WString           _GetString (WCharCP rscFileName,  unsigned int tableId, unsigned int rscId) = 0;

    public:
        ECOBJECTS_EXPORT WString  GetString (WCharCP rscFileName, unsigned int tableId, unsigned int rscId);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
