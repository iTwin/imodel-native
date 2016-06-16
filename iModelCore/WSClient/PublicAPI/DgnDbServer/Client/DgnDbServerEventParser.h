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

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct IDgnDbServerEventParser> IDgnDbServerEventParserPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDgnDbServerEventParser
    {
    public:
        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEventPtr ParseEventasJson
            (
            Utf8CP responseContentType, 
            Utf8String responseString
            ) const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEventPtr ParseEventasString
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEvent::DgnDbServerEventType GetEventType
            (
            DgnDbServerEventPtr eventPtr
            ) const = 0;
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
        
        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventPtr ParseEventasJson
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventPtr ParseEventasString
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEvent::DgnDbServerEventType GetEventType
            (
            DgnDbServerEventPtr eventPtr
            ) const override;
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE