/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/SecurityToken.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/ISecurityToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct SecurityToken : public ISecurityToken
    {
    private:
        Utf8String m_token;

    public:
        //! Construct empty token
        SecurityToken() { m_token = ""; };
        //! Construct token from string
        SecurityToken(Utf8String token) { m_token = token; };
        //! Check whenver given token is valid format
        bool IsSupported() const override { if (m_token == "") return false; return true; };
        //! Create string for authorization header
        Utf8String ToAuthorizationString() const override { return m_token; };
        //! Return original token representation
        Utf8StringCR AsString() const override { return m_token; };
        //! Compare contents of two tokens for equality
        bool operator==(const ISecurityToken& other) const override { return m_token == other.ToAuthorizationString(); };
        };

typedef SecurityToken& SecurityTokenR;
typedef const SecurityToken& SecurityTokenCR;
typedef SecurityToken* SecurityTokenP;
typedef const SecurityToken* SecurityTokenCP;
typedef std::shared_ptr<SecurityToken> SecurityTokenPtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
