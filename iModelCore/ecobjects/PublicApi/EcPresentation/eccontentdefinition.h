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
struct          IAUIContentServiceProvider : public IAUIProvider
    {
    protected:
        
        virtual IECContentDefinitionPtr _GetContent (IECViewDefinitionCR viewDef) const = 0;

    public:

        ECOBJECTS_EXPORT IECContentDefinitionPtr GetContent (IECViewDefinitionCR viewDef) const;

    };

END_BENTLEY_EC_NAMESPACE