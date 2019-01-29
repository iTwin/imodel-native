/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcToken.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley\WString.h>
#include <WebServices\Connect\ISecurityToken.h>
#include <iModelBridge/iModelBridgeFwkRegistry.h>

BEGIN_BENTLEY_DGN_NAMESPACE
/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct OidcToken> OidcTokenPtr;
struct OidcToken : public WebServices::ISecurityToken
    {
    private:
        Utf8String m_token;

    public:
        //! Construct token from string
        OidcToken(Utf8String token);
        OidcToken(OidcToken const& other);
        //! Check whenver given token is valid xml and supported
        bool IsSupported() const override;
        //! Create string for authorization header
        Utf8String ToAuthorizationString() const override;
        //! Return original token representation
        Utf8StringCR AsString() const override;
        //! Compare contents of two tokens for equality
        bool operator==(ISecurityToken const& other) const override;
    };

END_BENTLEY_DGN_NAMESPACE