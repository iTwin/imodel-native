/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Connect/SecureStore.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <MobileDgn/MobileDgnApplication.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
* Class that stores encrypted string values
+---------------+---------------+---------------+---------------+---------------+------*/
struct SecureStore
    {
    private:
        MobileDgn::ILocalState& m_localState;

    private:
        Utf8String CreateIdentifier (Utf8CP nameSpace, Utf8CP key);

    public:
        //! Create new persistence with default MobileDgnApp LocalState or custom.
        //! NOTE: For iOS LocalState is not used - KeyChain is used instead. Assert will be fired if app does not have required access group.
        WS_EXPORT SecureStore (MobileDgn::ILocalState* customLocalState = nullptr);

        //! Stores value for given key in namespace. If value is empty, key-value pair is deleted. If key or namespace is empty nothing is done.
        //! Value is encrypted and stored in persistent storage.
        WS_EXPORT void SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP value);

        //! Loads value for given key in namespace. Empty string is returned when value not found or other error occur.
        WS_EXPORT Utf8String LoadValue (Utf8CP nameSpace, Utf8CP key);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
