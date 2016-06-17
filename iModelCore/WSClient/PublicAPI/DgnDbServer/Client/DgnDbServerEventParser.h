/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerEventParser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerEvent.h>
#include <DgnDbServer/Client/DgnDbServerEventSubscription.h>
#include <DgnDbServer/Client/DgnDbServerEventSAS.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct IDgnDbServerEventParser> IDgnDbServerEventParserPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDgnDbServerEventParser
    {
    public:
        DGNDBSERVERCLIENT_EXPORT virtual Json::Value GenerateEventSubscriptionJson
            (
            bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr
            ) const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual Json::Value GenerateEventSASJson() const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEventSubscriptionPtr ParseEventSubscription(Json::Value jsonResponse) const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEventSASPtr ParseEventSAS(Json::Value jsonResponse) const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEventPtr ParseEvent
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEvent::DgnDbServerEventType GetEventType (DgnDbServerEventPtr eventPtr) const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnDbServerEventParser : public IDgnDbServerEventParser
    {
    private:
        DgnDbServerEventParser();

    public:
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<DgnDbServerEventParser> Create();
        
        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventPtr ParseEvent
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const override;

        DGNDBSERVERCLIENT_EXPORT Json::Value GenerateEventSubscriptionJson
            (
            bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr
            ) const override;

        DGNDBSERVERCLIENT_EXPORT Json::Value GenerateEventSASJson() const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventSubscriptionPtr ParseEventSubscription(Json::Value jsonResponse) const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventSASPtr ParseEventSAS(Json::Value jsonResponse) const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEvent::DgnDbServerEventType GetEventType
            (
            DgnDbServerEventPtr eventPtr
            ) const override;
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE