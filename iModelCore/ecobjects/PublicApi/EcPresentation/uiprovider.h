/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/uiprovider.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ecprovider.h>
#include <EcPresentation/uienabler.h>

EC_TYPEDEFS (UIECEnabler);
EC_TYPEDEFS (IAUIProvider);

BEGIN_BENTLEY_EC_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IAUIProvider: public IECProvider
    {
    private:
        //!Get the enabler for the classInstance
        ECOBJECTS_EXPORT virtual ECEnablerPtr    _GetEnabler (ECClassCR classInstance) override;

    protected:
        
        virtual UIECEnablerPtr  _GetUIEnabler (ECClassCR classInstance) = 0;
    public:
        
        
        ECOBJECTS_EXPORT UIECEnablerPtr GetUIEnabler (ECClassCR classInstance);
    };

END_BENTLEY_EC_NAMESPACE