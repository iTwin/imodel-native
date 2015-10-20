/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectClientCCApiWrapper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Brad.Hadden    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConnectClientCCApiWrapper
    {
    private:

    public:
        WSCLIENT_EXPORT ConnectClientCCApiWrapper ();
        WSCLIENT_EXPORT std::wstring GetSerializedDelegateSecurityToken (std::wstring relyingParty); 
    };

END_BENTLEY_WEBSERVICES_NAMESPACE