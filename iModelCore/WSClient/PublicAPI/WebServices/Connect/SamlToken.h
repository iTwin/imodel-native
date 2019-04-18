/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/ISecurityToken.h>
#include <WebServices/Client/WebServicesClient.h>
#include <Bentley/BeThread.h>
#include <Bentley/DateTime.h>
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct SamlToken : public ISecurityToken
    {
    private:
        Utf8String m_token;

        mutable BeMutex m_domMutex;
        mutable BeXmlDomPtr m_dom;

    private:
        BentleyStatus GetConditionDates(DateTime& notBeforeUtc, DateTime& notOnOrAfterUtc) const;
        static BentleyStatus GetAttributteDateTimeUtc(BeXmlNodeP node, Utf8CP name, DateTimeR dateTimeOut);

    public:
        //! Construct empty token
        WSCLIENT_EXPORT SamlToken();
        //! Construct token from xml string
        WSCLIENT_EXPORT SamlToken(Utf8String token);
        //! Copy constructor
        WSCLIENT_EXPORT SamlToken(const SamlToken& other);
        //! Assignment operator
        WSCLIENT_EXPORT SamlToken& operator=(const SamlToken& other);
        //! Check whenver given token is empty
        WSCLIENT_EXPORT bool IsEmpty() const;
        //! Check whenver given token is valid xml and supported
        WSCLIENT_EXPORT bool IsSupported() const override;
        //! Check whenever token is valid at given date.
        //! Note that client clock might not be correctly set. This can be workarounded by using slight future DateTime. But not always
        WSCLIENT_EXPORT bool IsValidAt(DateTimeCR dateTimeUtc) const;
        //! Check whenever token is valid at current time with given offset in minutes
        //! Note that client clock might not be correctly set.
        WSCLIENT_EXPORT bool IsValidNow(uint32_t offsetMinutes) const;
        //! Get token lifetime in minutes 
        WSCLIENT_EXPORT uint32_t GetLifetime() const;
        //! Get token attributes
        WSCLIENT_EXPORT BentleyStatus GetAttributes(bmap<Utf8String, Utf8String>& atributtesOut) const;
        //! Get base 64 encoded certificate
        WSCLIENT_EXPORT BentleyStatus GetX509Certificate(Utf8StringR certOut) const;
        //! Create string for authorization header
        WSCLIENT_EXPORT Utf8String ToAuthorizationString() const override;
        //! Return original token representation
        WSCLIENT_EXPORT Utf8StringCR AsString() const override;
        //! Compare contents of two tokens for equality
        WSCLIENT_EXPORT bool operator==(const ISecurityToken& other) const override;
    };

typedef SamlToken& SamlTokenR;
typedef const SamlToken& SamlTokenCR;
typedef SamlToken* SamlTokenP;
typedef const SamlToken* SamlTokenCP;
typedef std::shared_ptr<SamlToken> SamlTokenPtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
