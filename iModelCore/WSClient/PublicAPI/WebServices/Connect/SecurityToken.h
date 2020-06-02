/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/ISecurityToken.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct SecurityToken : public ISecurityToken
    {
private:
    Utf8String m_token;
    DateTime m_expirationDate;

public:
    //! Construct empty token
    SecurityToken() : m_token(""), m_expirationDate(DateTime()) {};
    //! Construct token from string
    SecurityToken(Utf8String token, DateTime date = DateTime()) : m_token(std::move(token)), m_expirationDate(std::move(date)) {};
    //! Check whenver given token is valid format
    bool IsSupported() const override { return !m_token.empty(); };
    //! Create string for authorization header
    Utf8String ToAuthorizationString() const override { return m_token; };
    //! Return original token representation
    Utf8StringCR AsString() const override { return m_token; };
    //! Returns expiration date if available and is known
    DateTime GetExpirationDate() const override { return m_expirationDate; };
    //! Compare contents of two tokens for equality
    bool operator==(const ISecurityToken& other) const override { return m_token == other.ToAuthorizationString(); };
    };

DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(SecurityToken);
typedef std::shared_ptr<SecurityToken> SecurityTokenPtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
