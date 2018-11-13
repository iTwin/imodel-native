/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/SamlTokenTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SamlTokenTests.h"
#include <WebServices/Connect/SamlToken.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, CopyCtor_OriginalEmpty_Empty)
    {
    auto original = std::make_shared<SamlToken>();
    SamlToken copy(*original);
    original = nullptr;
    EXPECT_TRUE(copy.IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, CopyCtor_OriginalWithLifetimeAndOriginalDestroyed_LifetimeCopied)
    {
    auto original = std::make_shared<SamlToken>(StubSamlTokenXML(42));
    auto copy = std::make_shared<SamlToken>(*original);

    ASSERT_EQ(42, original->GetLifetime());
    ASSERT_EQ(42, copy->GetLifetime());

    original = nullptr;

    EXPECT_EQ(42, copy->GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, CopyCtor_OriginalWithLifetimeAndCopyDestroyed_LifetimeSameInOriginal)
    {
    auto original = std::make_shared<SamlToken>(StubSamlTokenXML(42));
    auto copy = std::make_shared<SamlToken>(*original);

    ASSERT_EQ(42, original->GetLifetime());
    ASSERT_EQ(42, copy->GetLifetime());

    copy = nullptr;

    EXPECT_EQ(42, original->GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, AssignmentOperator_Self_LifetimeSame)
    {
    auto token = std::make_shared<SamlToken>(StubSamlTokenXML(111));
    ASSERT_EQ(111, token->GetLifetime());

    *token = *token;
    ASSERT_EQ(111, token->GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, AssignmentOperator_BothWithLifetimeAndRightDestroyed_LifetimeCopied)
    {
    auto token1 = std::make_shared<SamlToken>(StubSamlTokenXML(111));
    auto token2 = std::make_shared<SamlToken>(StubSamlTokenXML(222));

    ASSERT_EQ(111, token1->GetLifetime());
    ASSERT_EQ(222, token2->GetLifetime());

    *token1 = *token2;

    ASSERT_EQ(222, token1->GetLifetime());
    ASSERT_EQ(222, token2->GetLifetime());

    token2 = nullptr;

    EXPECT_EQ(222, token1->GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, AssignmentOperator_BothWithLifetimeAndLeftDestroyed_LifetimeSameInOriginal)
    {
    auto token1 = std::make_shared<SamlToken>(StubSamlTokenXML(111));
    auto token2 = std::make_shared<SamlToken>(StubSamlTokenXML(222));

    ASSERT_EQ(111, token1->GetLifetime());
    ASSERT_EQ(222, token2->GetLifetime());

    *token1 = *token2;

    ASSERT_EQ(222, token1->GetLifetime());
    ASSERT_EQ(222, token2->GetLifetime());

    token1 = nullptr;

    EXPECT_EQ(222, token2->GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsEmpty_DefaultCtor_True)
    {
    EXPECT_TRUE(SamlToken().IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsEmpty_CtorWithEmptyString_True)
    {
    EXPECT_TRUE(SamlToken("").IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsEmpty_CtorWithNotEmptyString_FALSE)
    {
    EXPECT_FALSE(SamlToken("something").IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsSupported_DefaultCtor_False)
    {
    EXPECT_FALSE(SamlToken().IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsSupported_InvalidXml_False)
    {
    EXPECT_FALSE(SamlToken("this is not xml").IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsSupported_ValidAssertionWithVersions_SuportedOnlyVersion_1_1)
    {
    Utf8String tokenStr;
    tokenStr = R"(<saml:Assertion MajorVersion="1" MinorVersion="0" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"/>)";
    EXPECT_FALSE(SamlToken(tokenStr).IsSupported());

    tokenStr = R"(<saml:Assertion MajorVersion="2" MinorVersion="0" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"/>)";
    EXPECT_FALSE(SamlToken(tokenStr).IsSupported());

    tokenStr = R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"/>)";
    EXPECT_TRUE(SamlToken(tokenStr).IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, ToAuthorizationString_AnyStringPassed_ReturnsFormattedAuthorizationHeaderValue)
    {
    EXPECT_EQ(Base64Utilities::Encode("TestToken"), SamlToken("TestToken").ToAuthorizationString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, AsString_AnyStringPassed_ReturnsSame)
    {
    EXPECT_EQ("TestToken", SamlToken("TestToken").AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsValidAt_XmlConditionWithInvalidDateValue_ReturnsFalse)
    {
    Utf8String tokenStr =
        R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            <saml:Conditions 
                NotBefore   ="2000-01-01T00:00:00.000Z"
                NotOnOrAfter="notADate">
            </saml:Conditions>
        </saml:Assertion>)";

    SamlToken token(tokenStr);
    BeTest::SetFailOnAssert(false);
    EXPECT_FALSE(token.IsValidAt(DateTime(DateTime::Kind::Utc, 2100, 01, 01, 0, 0, 0)));
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsValidAt_XmlConditionWithDates_ValidOnlyIfBetweenDates)
    {
    Utf8String tokenStr =
        R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            <saml:Conditions 
                NotBefore   ="2000-01-01T00:00:00.000Z"
                NotOnOrAfter="2000-01-03T00:00:00.000Z">
            </saml:Conditions>
        </saml:Assertion>)";

    SamlToken token(tokenStr);

    EXPECT_FALSE(token.IsValidAt(DateTime(DateTime::Kind::Utc, 1999, 12, 30, 23, 59, 59)));
    EXPECT_FALSE(token.IsValidAt(DateTime(DateTime::Kind::Utc, 2000, 01, 03, 0, 0, 0)));

    EXPECT_TRUE(token.IsValidAt(DateTime(DateTime::Kind::Utc, 2000, 01, 01, 0, 0, 0)));
    EXPECT_TRUE(token.IsValidAt(DateTime(DateTime::Kind::Utc, 2000, 01, 02, 23, 59, 59)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsValidNow_InvalidToken_False)
    {
    SamlToken token(StubSamlTokenXML(0));
    EXPECT_FALSE(token.IsValidNow(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsValidNow_ValidToken_True)
    {
    SamlToken token(StubSamlTokenXML(100));
    EXPECT_TRUE(token.IsValidNow(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsValidNow_ValidTokenAndOffsetIsOutOfValidity_False)
    {
    SamlToken token(StubSamlTokenXML(100));
    EXPECT_FALSE(token.IsValidNow(100));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, IsValidNow_ValidTokenAndOffsetIsInValidity_True)
    {
    SamlToken token(StubSamlTokenXML(100));
    EXPECT_TRUE(token.IsValidNow(4));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, GetLifetime_InvalidToken_Zero)
    {
    SamlToken token;
    EXPECT_EQ(0, token.GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, GetLifetime_TokenWithValidLifetime_LifetimeReturned)
    {
    SamlToken token(StubSamlTokenXML(42));
    EXPECT_EQ(42, token.GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, GetAttributes_AttributesExist_SuccessAndReturnsAttributes)
    {
    Utf8String tokenStr =
        R"( <saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
                <saml:AttributeStatement>
                    <saml:Attribute AttributeName="Foo">
                        <saml:AttributeValue>A</saml:AttributeValue>
                    </saml:Attribute>
                    <saml:Attribute AttributeName="Boo">
                        <saml:AttributeValue>B</saml:AttributeValue>
                    </saml:Attribute>
                </saml:AttributeStatement>
            </saml:Assertion>)";

    SamlToken token(tokenStr);

    bmap<Utf8String, Utf8String> attributes;
    auto status = token.GetAttributes(attributes);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ("A", attributes["Foo"]);
    EXPECT_EQ("B", attributes["Boo"]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, GetAttributes_NoAttributes_SuccessWithNoAttributes)
    {
    Utf8String tokenStr =
        R"( <saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            </saml:Assertion>)";

    SamlToken token(tokenStr);

    bmap<Utf8String, Utf8String> attributes;
    auto status = token.GetAttributes(attributes);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(attributes.empty());
    EXPECT_TRUE(token.IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, GetX509Certificate_SignatureExists_SuccessAndReturnsElementContent)
    {
    Utf8String tokenStr =
        R"( <saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
                <ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#">
                    <KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#">
                        <X509Data>
                            <X509Certificate>TestCertificate</X509Certificate>
                        </X509Data>
                    </KeyInfo>
                </ds:Signature>
            </saml:Assertion>)";

    SamlToken token(tokenStr);
    Utf8String cert;
    auto status = token.GetX509Certificate(cert);

    EXPECT_EQ(SUCCESS, status);
    EXPECT_EQ("TestCertificate", cert);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, GetX509Certificate_SignatureDoesNotExist_ReturnsError)
    {
    Utf8String tokenStr =
        R"( <saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            </saml:Assertion>)";

    SamlToken token(tokenStr);
    Utf8String cert;
    auto status = token.GetX509Certificate(cert);

    EXPECT_EQ(ERROR, status);
    EXPECT_EQ("", cert);
    EXPECT_TRUE(token.IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, OperatorEquals_TokensWithSameValue_True)
    {
    EXPECT_TRUE(SamlToken("Foo") == SamlToken("Foo"));

    Utf8String tokenStr = R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"></saml:Assertion>)";
    EXPECT_TRUE(SamlToken(tokenStr) == SamlToken(tokenStr));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SamlTokenTests, OperatorNotEquals_TokensWithDifferentValues_False)
    {
    EXPECT_FALSE(SamlToken("A") == SamlToken("B"));

    Utf8String tokenStr1 = R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"></saml:Assertion>)";
    Utf8String tokenStr2 = R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion"><ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#"></ds:Signature></saml:Assertion>)";
    EXPECT_FALSE(SamlToken(tokenStr1) == SamlToken(tokenStr2));
    }
