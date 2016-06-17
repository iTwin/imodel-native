/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/DgnDbServerEventParserTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../Utils/WebServicesTestsHelper.h"

Utf8String StubHttpResponseEmpty();
Utf8String StubHttpResponseEmptyJson();
Utf8String StubHttpResponseInvalid();
Utf8String StubHttpResponseValidLockEvent();
Utf8String StubHttpResponseValidRevisionEvent();
Utf8String StubHttpResponseInvalidLockEvent();
Utf8String StubHttpResponseInvalidRevisionEvent();
Utf8String StubHttpResponseValidEventSubscriptionResponse();
Utf8String StubHttpResponseValidEventSASResponse();
Utf8String StubHttpResponseInvalidEventSubscriptionResponse();
Utf8String StubHttpResponseInvalidEventSASResponse();
Utf8String StubGenerateValidEventSASJson();
Utf8String StubGenerateInvalidEventSASJson();
Utf8String StubGenerateValidEventSubscriptionJsonSingleEvent();
Utf8String StubGenerateValidEventSubscriptionJsonNoEvent();
Utf8String StubGenerateInvalidEventSubscriptionJson();
Utf8CP     StubHttpResponseEmptyContentType();
Utf8CP     StubHttpResponseInvalidContentType();
Utf8CP     StubHttpResponseValidLockEventContentType();
Utf8CP     StubHttpResponseInvalidLockEventContentType();
Utf8CP     StubHttpResponseValidRevisionEventContentType();
Utf8CP     StubHttpResponseInvalidRevisionEventContentType();

class DgnDbServerEventParserTests : public BaseMockHttpHandlerTest
    {
    void SetUp() override;
    };