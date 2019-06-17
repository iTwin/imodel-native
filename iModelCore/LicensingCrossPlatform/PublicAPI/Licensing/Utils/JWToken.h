/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include <json/json.h>

#include <openssl/err.h>
#include <openssl/pem.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct JWToken
    {
private:
    Json::Value m_header;
    Json::Value m_payload;

private:
    JWToken(JsonValueCR header, JsonValueCR payload);

    Utf8String GetAlg() const;

    BentleyStatus Validate(Utf8StringCR headerWithPayload, Utf8StringCR signature, Utf8StringCR certificate) const;
    BentleyStatus EVP_Validate(EVP_PKEY* pkey, Utf8StringCR headerWithPayload, Utf8StringCR signature) const;

public:
    //! Create Json Web Token.
    //! token string token to parse
    //! certificate - if not null attempts to validate token agains given certificate.
    //! Returns nullptr if validation failed or any other error occured or token otherwise.
    LICENSING_EXPORT static std::shared_ptr<JWToken> Create(Utf8StringCR token, Utf8StringCR certificate = nullptr);

    LICENSING_EXPORT Utf8String GetClaim(Utf8StringCR claim) const;
    };

END_BENTLEY_LICENSING_NAMESPACE
