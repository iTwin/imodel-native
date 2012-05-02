/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/eccontentdefinition.h $
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
struct IECContentDefinition: public IAUIItem
    {

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IAUIContentServiceProvider : public ECPresentationProvider
    {
    protected:
        
        virtual IECContentDefinitionPtr _GetContent (IECPresentationViewDefinitionCR viewDef) const = 0;
        virtual ProviderType    _GetProviderType() const override {return ContentService;}
    public:

        ECOBJECTS_EXPORT IECContentDefinitionPtr GetContent (IECPresentationViewDefinitionCR viewDef) const;

    };

END_BENTLEY_EC_NAMESPACE