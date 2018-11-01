/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ISecurityToken.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                julius.cepukenas    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ISecurityToken> ISecurityTokenPtr;
struct ISecurityToken
    {
    public:
        virtual ~ISecurityToken() {};

        //! Check whenver given token is valid format
        virtual bool IsSupported() const = 0;

        //! Create string for authorization header
        virtual Utf8String ToAuthorizationString() const = 0;

        //! Return original token representation
        virtual Utf8StringCR AsString() const = 0;

        //! Compare contents of two tokens for equality
        virtual bool operator==(const ISecurityToken& other) const = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
