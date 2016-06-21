/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerEventSAS.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbServerResult.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct DgnDbServerEventSAS> DgnDbServerEventSASPtr;

DEFINE_TASK_TYPEDEFS(DgnDbServerEventSASPtr, DgnDbServerEventSAS);

struct DgnDbServerEventSAS
    {
    //__PUBLISH_SECTION_END__
    private:
        Utf8String m_sasToken;
        Utf8String m_baseAddress;

        DgnDbServerEventSAS(Utf8String sasToken, Utf8String baseAddress);
        //__PUBLISH_SECTION_START__
    public:
        DGNDBSERVERCLIENT_EXPORT static DgnDbServerEventSASPtr Create(Utf8String sasToken, Utf8String baseAddress);
        DGNDBSERVERCLIENT_EXPORT Utf8String GetSASToken();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetBaseAddress();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE