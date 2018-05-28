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
struct RequestOptions
    {
    public:
        enum class FailureStrategy
            {
            Continue,
            Stop,
            Default = Stop
            };

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
        FailureStrategy GetFailureStrategy() const { return m_failureStrategy; };
        void SetFailureStrategy(FailureStrategy option) { m_failureStrategy = option; };

        ResponseContent GetResponseContent() const { return m_responseContent; };
        void SetResponseContent(ResponseContent option) { m_responseContent = option; };

        bool GetShouldRefreshInstances() const { return m_refreshInstances; };
        void SetShouldRefreshInstances(bool option) { m_refreshInstances = option; };

        Json::Value GetCustomOption() const { return m_customRequestOptions; }
        void SetCustomRequestOption(Utf8String customOption, Json::Value value) { m_customRequestOptions[customOption] = value; }

        bool operator ==(const RequestOptions& other) const
            {
            return
                other.m_failureStrategy == m_failureStrategy &&
                other.m_responseContent == m_responseContent &&
                other.m_refreshInstances == m_refreshInstances &&
                other.m_customRequestOptions == m_customRequestOptions;
            };

        //! Convert options to JSON for server
        WSCLIENT_EXPORT void ToJson(JsonValueR jsonOut) const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
