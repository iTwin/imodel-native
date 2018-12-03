/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcToken.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "OidcToken.h"
USING_NAMESPACE_BENTLEY_DGN

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OidcToken::OidcToken(Utf8String token) : m_token(token)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OidcToken::OidcToken(OidcToken const& other): m_token(other.m_token)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool OidcToken::IsSupported() const
    {
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String OidcToken::ToAuthorizationString() const
    {
    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR OidcToken::AsString() const
    {
    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool OidcToken::operator==(ISecurityToken const& other) const
    {
    const OidcToken* token = dynamic_cast<const OidcToken*>(&other);
    if (token == nullptr) 
        return false;
    return m_token == token->m_token;
    }
