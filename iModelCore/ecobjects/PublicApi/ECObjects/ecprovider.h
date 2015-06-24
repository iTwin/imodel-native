/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ecprovider.h $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "ECObjects.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//! @addtogroup ECObjectsGroup
//! @beginGroup 
//! @bsiclass
struct IECProvider
    {
    protected:
        //!Get the provider id 
        virtual uint16_t    _GetProviderId () const = 0;

        //!Get the provider name
        virtual WCharCP     _GetProviderName () const = 0;

    public:
        //! This should be an ID obtained from http://toolsnet.bentley.com/Signature, like ElementHandlerIds and XAttributeHandlerIds. It
        //! won't necessarily match an existing ElementHandler or XAttributeHandler Id because most providers should handle
        //! multiple types of elements/xAttributes.
        ECOBJECTS_EXPORT uint16_t       GetProviderId () const;

        //! This should be the name of the provider that will be written to the schema.
        ECOBJECTS_EXPORT WCharCP        GetProviderName () const;

        virtual ~IECProvider() { }
    };
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE