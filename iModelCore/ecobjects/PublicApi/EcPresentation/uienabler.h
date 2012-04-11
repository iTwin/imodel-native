/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/uienabler.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <EcPresentation/uiitem.h>

EC_TYPEDEFS (UIECEnabler);


BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<UIECEnabler>  UIECEnablerPtr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  UIECEnabler: public ECEnabler
    {
    protected:
        virtual IAUIItemPtr     _GetUIItem (IECInstanceP instanceData) = 0;

    public:

        UIECEnabler (ECClassCR classInstance, IStandaloneEnablerLocaterP structStandaloneEnablerLocater)
            :ECEnabler(classInstance, structStandaloneEnablerLocater)
            {}

        ECOBJECTS_EXPORT static ECClassCR GetUIClass (WCharCP className);

        ECOBJECTS_EXPORT IAUIItemPtr    GetUIItem (IECInstanceP instanceData);
    };

END_BENTLEY_EC_NAMESPACE