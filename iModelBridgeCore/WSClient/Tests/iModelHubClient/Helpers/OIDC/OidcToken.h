/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../../Common.h"
#include <Bentley\WString.h>
#include <WebServices\Connect\ISecurityToken.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct OidcToken> OidcTokenPtr;
struct OidcToken : public ISecurityToken
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

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
