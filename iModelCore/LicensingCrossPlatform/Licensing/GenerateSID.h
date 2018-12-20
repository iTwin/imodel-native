/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/GenerateSID.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include <vector>

#define NUM_SHA1HASH_ITERATIONS    17

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct GenerateSID
    {
    private:
        Utf8String PrepareUserData(Utf8StringCR userName, Utf8StringCR machineName) const;
        Utf8String PrepareMachineData(Utf8StringCR machineName) const;
        Utf8String GenerateSHA1Hash(Utf8StringCR inputString) const;
        std::vector<Byte> CalculateHash(std::vector<Byte> byteString) const;

    public:
        LICENSING_EXPORT Utf8String GetUserSID(Utf8StringCR userName, Utf8StringCR machineName) const;
        LICENSING_EXPORT Utf8String GetMachineSID(Utf8StringCR machineName) const;
    };

END_BENTLEY_LICENSING_NAMESPACE