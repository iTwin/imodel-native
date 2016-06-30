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

//typedef std::shared_ptr<struct IDgnDbServerEventParser> IDgnDbServerEventParserPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDgnDbServerEventParser
    {
    public:
        DGNDBSERVERCLIENT_EXPORT virtual Json::Value GenerateEventSubscriptionWSChangeSetJson
            (
            bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr,
            Utf8String eventSubscriptionId = ""
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
        DgnDbServerEventParser(DgnDbServerEventParser const&);              // Don't Implement
        void operator=(DgnDbServerEventParser const&);                      // Don't implement

    public:
        DGNDBSERVERCLIENT_EXPORT static DgnDbServerEventParser& GetInstance();
        
        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventPtr ParseEvent
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const override;

        DGNDBSERVERCLIENT_EXPORT Json::Value GenerateEventSubscriptionWSChangeSetJson
            (
            bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr,
            Utf8String eventSubscriptionId = ""
            ) const override;

        DGNDBSERVERCLIENT_EXPORT Json::Value GenerateEventSubscriptionWSObjectJson
            (
            bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes = nullptr,
            Utf8String eventSubscriptionId = ""
            ) const;

        DGNDBSERVERCLIENT_EXPORT Json::Value GenerateEventSASJson() const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventSubscriptionPtr ParseEventSubscription(Json::Value jsonResponse) const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventSASPtr ParseEventSAS(Json::Value jsonResponse) const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEvent::DgnDbServerEventType GetEventType(DgnDbServerEventPtr eventPtr) const override;

        DGNDBSERVERCLIENT_EXPORT std::shared_ptr<struct DgnDbServerLockEvent> GetLockEvent(DgnDbServerEventPtr eventPtr);

        DGNDBSERVERCLIENT_EXPORT std::shared_ptr<struct DgnDbServerRevisionEvent> GetRevisionEvent(DgnDbServerEventPtr eventPtr);

        DGNDBSERVERCLIENT_EXPORT std::shared_ptr<struct DgnDbServerCodeEvent> GetCodeEvent(DgnDbServerEventPtr eventPtr);
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE