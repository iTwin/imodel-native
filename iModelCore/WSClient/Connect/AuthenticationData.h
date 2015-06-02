/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/AuthenticationData.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <MobileDgn/MobileDgnApplication.h>

struct AuthenticationData : NonCopyableClass
    {
public:
    static const Utf8String USERNAME;
    static const Utf8String PASSWORD;
    static const Utf8String TOKEN;
    };
