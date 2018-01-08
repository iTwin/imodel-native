#pragma once
/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/Decrypt.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

class CryptoHelper
    {
    public:
    static void    DecryptString(Utf8String& decrypedString, Utf8StringCR encryptedString);
    };