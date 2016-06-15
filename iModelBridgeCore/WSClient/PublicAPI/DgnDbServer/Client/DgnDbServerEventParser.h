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
#include <DgnDbServer/Client/IDgnDbServerEvent.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDgnDbServerEventParser
    {
    public:
        DGNDBSERVERCLIENT_EXPORT virtual IDgnDbServerEventPtr ParseEventasJson
            (
            Utf8CP responseContentType, 
            Utf8String responseString
            ) const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual IDgnDbServerEventPtr ParseEventasString
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const = 0;

        DGNDBSERVERCLIENT_EXPORT virtual DgnDbServerEventType GetEventType
            (
            IDgnDbServerEventPtr eventPtr
            ) const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnDbServerEventParser : public IDgnDbServerEventParser
    {
    public:
        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventParser();
        DGNDBSERVERCLIENT_EXPORT IDgnDbServerEventPtr ParseEventasJson
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const override;

        DGNDBSERVERCLIENT_EXPORT IDgnDbServerEventPtr ParseEventasString
            (
            Utf8CP responseContentType,
            Utf8String responseString
            ) const override;

        DGNDBSERVERCLIENT_EXPORT DgnDbServerEventType GetEventType
            (
            IDgnDbServerEventPtr eventPtr
            ) const override;
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE