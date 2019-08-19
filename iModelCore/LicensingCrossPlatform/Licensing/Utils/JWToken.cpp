/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/JWToken.h>

#include <Bentley/BeStringUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <json/value.h>
#include "../Logging.h"

#include <Licensing/Utils/UrlSafeBase64Utilities.h>

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JWToken::JWToken(JsonValueCR header, JsonValueCR payload) :
    m_header (header),
    m_payload (payload)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<JWToken> JWToken::Create(Utf8StringCR token, Utf8StringCR certificate)
    {
    bvector<Utf8String> parts;


    BeStringUtilities::Split(token.c_str(), ".", parts);

    if (parts.size() != 3)
        {
        return nullptr;
        }

    Utf8String header = Base64Utilities::Decode(parts[0]);
    Json::Value headerJson;

    if (!Json::Reader::Parse(header, headerJson))
        {
        return nullptr;
        }

    Utf8String payload = Base64Utilities::Decode(parts[1]);
    Json::Value payloadJson;
    if (!Json::Reader::Parse(payload, payloadJson))
        {
        return nullptr;
        }

    auto jwtToken = std::shared_ptr<JWToken>(new JWToken(headerJson, payloadJson));

    if (!certificate.empty() && jwtToken->Validate(parts[0] + "." + parts[1], parts[2], certificate) != SUCCESS)
        {
        return nullptr;
        }

    return jwtToken;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JWToken::GetAlg() const
    {
    return m_header["alg"].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JWToken::GetClaim(Utf8StringCR claim) const
    {
    return m_payload[claim].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JWToken::Validate(Utf8StringCR headerWithPayload, Utf8StringCR signature, Utf8StringCR certificate) const
    {
    BentleyStatus result = ERROR;

    Utf8String certificateDecoded = Base64Utilities::Decode(certificate);

    BIO* bio = BIO_new_mem_buf(certificateDecoded.c_str(), (int)certificateDecoded.size());
    X509* cert = d2i_X509_bio(bio, NULL);
    EVP_PKEY* pkey = nullptr;
    if (cert != nullptr)
        {
        pkey = X509_get_pubkey(cert);
        if (pkey != nullptr)
            result = EVP_Validate(pkey, headerWithPayload, signature);
        }

    if (pkey)
        EVP_PKEY_free(pkey);
    if (bio)
        BIO_free(bio);

    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JWToken::EVP_Validate(EVP_PKEY* pkey, Utf8StringCR headerWithPayload, Utf8StringCR signature) const
    {
    Utf8String signatureBase64 = UrlSafeBase64Utilities::ToBase64(signature);
    Utf8String signatureDecoded = Base64Utilities::Decode(signatureBase64);

    const EVP_MD *alg;

    Utf8String jwtAlg = GetAlg();
    if (jwtAlg.EqualsI("RS256") || jwtAlg.EqualsI("ES256"))
        alg = EVP_sha256();
    else if (jwtAlg.EqualsI("RS384") || jwtAlg.EqualsI("ES384"))
        alg = EVP_sha384();
    else if (jwtAlg.EqualsI("RS512") || jwtAlg.EqualsI("ES512"))
        alg = EVP_sha512();
    else
        {
        BeAssert(false && "Unsupported JWT algorithm");
        return ERROR;
        }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_create();

    bool status = EVP_DigestVerifyInit(mdctx, NULL, alg, NULL, pkey) == 1 &&
                  EVP_DigestVerifyUpdate(mdctx, headerWithPayload.c_str(), headerWithPayload.size()) == 1 &&
                  EVP_DigestVerifyFinal(mdctx, (const unsigned char*)signatureDecoded.c_str(), signatureDecoded.size()) == 1;

    if (mdctx)
        EVP_MD_CTX_destroy(mdctx);

    return status ? SUCCESS : ERROR;
    }