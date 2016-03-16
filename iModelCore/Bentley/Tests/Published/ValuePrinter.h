/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ValuePrinter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeVersion.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpStatus.h>
#include <BeHttp/HttpResponse.h>
#include "TestsHelper.h"
#include <ostream>
#include <iostream>

USING_NAMESPACE_BENTLEY_TASKS
USING_NAMESPACE_BENTLEY_HTTP

// Http
std::ostream& operator << (std::ostream &o, CredentialsCR creds);
std::ostream& operator << (std::ostream &o, ConnectionStatus status);
std::ostream& operator << (std::ostream &o, HttpStatus status);
std::ostream& operator << (std::ostream &o, BeVersionCR version);

// Threading
std::ostream& operator << (std::ostream &o, AsyncTask::Priority value);

// Bentley types
BEGIN_BENTLEY_UNIT_TESTS_NAMESPACE

namespace Json
    {
    //void PrintTo (const Value& value, ::std::ostream* os);
    }

void PrintTo (const WString& value, ::std::ostream* os);
void PrintTo (const Utf8String& value, ::std::ostream* os);
void PrintTo (BentleyStatus value, ::std::ostream* os);

END_BENTLEY_UNIT_TESTS_NAMESPACE
