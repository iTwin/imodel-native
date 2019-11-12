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

public:
    //! Construct empty token
    SecurityToken() {};
    //! Construct token from string
    SecurityToken(Utf8String token) : m_token(std::move(token)) {};
    //! Check whenver given token is valid format
    bool IsSupported() const override { return !m_token.empty(); };
    //! Create string for authorization header
    Utf8String ToAuthorizationString() const override { return m_token; };
    //! Return original token representation
    Utf8StringCR AsString() const override { return m_token; };
    //! Compare contents of two tokens for equality
    bool operator==(const ISecurityToken& other) const override { return m_token == other.ToAuthorizationString(); };
    };

DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(SecurityToken);
typedef std::shared_ptr<SecurityToken> SecurityTokenPtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
