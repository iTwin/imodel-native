/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Connect/PasswordPersistence.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <MobileDgn/MobileDgnApplication.h>
#include <WebServices/Common.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct PasswordPersistence
    {
private:
    MobileDgn::ILocalState& m_localState;

public:
    //! Create new persistence with default MobileDgnApp LocalState or custom.
    //! NOTE: For iOS LocalState is not used - KeyChain is used instead. Assert will be fired if app does not have required access group.
    WS_EXPORT PasswordPersistence (MobileDgn::ILocalState* customLocalState = nullptr);

    //! Saves a new identifier & password pair. If password is empty, this pair is deleted. If identifier is empty, nothing is done.
    WS_EXPORT void SavePassword (Utf8CP identifier, Utf8CP password);

    //! Loads password for given identifier. Empty string is returned when password not found or empty identifier given.
    WS_EXPORT Utf8String LoadPassword (Utf8CP identifier);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE