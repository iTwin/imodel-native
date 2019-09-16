/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <Logging/bentleylogging.h>

typedef std::function<void(NativeLogging::SEVERITY sev, WCharCP msg)> GenericLogWriter;

struct GenericLogProviderActivator
    {
    //! Requires non-published APIs to work, hides implementation
    //! @param writer - simple function object to implement custom output for log messages. Receives severity and formatted message.
    //! @return SUCCESS/ERROR
    static int Activate(GenericLogWriter writer);
    };
