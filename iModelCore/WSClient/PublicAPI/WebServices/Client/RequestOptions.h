/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Client/RequestOptions.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Jonathan.Que    04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//! DEPRECATED- Use WSChangeset::Options instead
struct RequestOptions
    {
    public:
        //! DEPRECATED- Use WSChangeset::Options::FailureStrategy instead
        enum class FailureStrategy
            {
            Continue,
            Stop,
            Default = Stop
            };
        //! DEPRECATED- Use WSChangeset::Options::ResponseContent instead
        enum class ResponseContent
            {
            FullInstance,
            Empty,
            InstanceId,
            Default = FullInstance
            };

    private:
        FailureStrategy m_failureStrategy = FailureStrategy::Default;
        ResponseContent m_responseContent = ResponseContent::Default;
        bool m_refreshInstances = false;
        Json::Value m_customRequestOptions = Json::objectValue;

    private:
        static Utf8CP GetFailureStrategyStr(FailureStrategy failureStrategy);
        static Utf8CP GetResponseContentStr(ResponseContent responseContent);

    public:
        //! DEPRECATED- Use WSChangeset::Options::GetFailureStrategy instead
        FailureStrategy GetFailureStrategy() const { return m_failureStrategy; };

        //! DEPRECATED- Use WSChangeset::Options::SetFailureStrategy instead
        void SetFailureStrategy(FailureStrategy option) { m_failureStrategy = option; };

        //! DEPRECATED- Use WSChangeset::Options::GetResponseContent instead
        ResponseContent GetResponseContent() const { return m_responseContent; };

        //! DEPRECATED- Use WSChangeset::Options::SetResponseContent instead
        void SetResponseContent(ResponseContent option) { m_responseContent = option; };

        //! DEPRECATED- Use WSChangeset::Options::GetRefreshInstances instead
        bool GetShouldRefreshInstances() const { return m_refreshInstances; };

        //! DEPRECATED- Use WSChangeset::Options::SetShouldRefreshInstances instead
        void SetShouldRefreshInstances(bool option) { m_refreshInstances = option; };

        //! DEPRECATED- Use WSChangeset::Options::GetCustomOptions instead
        Json::Value GetCustomOption() const { return m_customRequestOptions; }

        //! DEPRECATED- Use WSChangeset::Options::SetCustomOption instead
        void SetCustomRequestOption(Utf8String customOption, Json::Value value) { m_customRequestOptions[customOption] = value; }

        //! DEPRECATED- Use WSChangeset::Options::operator== instead
        bool operator ==(const RequestOptions& other) const
            {
            return
                other.m_failureStrategy == m_failureStrategy &&
                other.m_responseContent == m_responseContent &&
                other.m_refreshInstances == m_refreshInstances &&
                other.m_customRequestOptions == m_customRequestOptions;
            };

        //! DEPRECATED- Use WSChangeset::ToRequestJson instead
        //! Convert options to JSON for server
        WSCLIENT_EXPORT void ToJson(JsonValueR jsonOut) const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
