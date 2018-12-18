/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/Utils/UrlSafeBase64Utilities.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/UrlSafeBase64Utilities.h>

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlSafeBase64Utilities::ToBase64(Utf8StringCR urlSafeBase64)
    {
    Utf8String newString = urlSafeBase64;
    newString.ReplaceAll("-", "+");
    newString.ReplaceAll("_", "/");

    while (newString.size() % 4 != 0)
        newString += "=";

    return newString;
    }