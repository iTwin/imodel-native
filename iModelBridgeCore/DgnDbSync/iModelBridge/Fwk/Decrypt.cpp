/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/Decrypt.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Decrypt.h"
#if defined(_WIN32)
#include <Windows.h>
#include <dpapi.h>
#endif
#include <Bentley\BeStringUtilities.h>
#include <Bentley\Base64Utilities.h>
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            CryptoHelper::DecryptString(Utf8StringR decrypedString, Utf8StringCR encryptedString)
    {
    decrypedString = encryptedString;
#if defined (_WIN32)
    bvector<Byte> byteArray;
    Base64Utilities::Decode(byteArray, encryptedString);

    DATA_BLOB blob;
    blob.cbData = (DWORD)byteArray.size();
    blob.pbData = &byteArray[0];

    DATA_BLOB unencryptedBlob;
    //LPWSTR pDescrOut = NULL;

    DATA_BLOB entropy;
    unsigned char entropyKey[] = {3, 7, 2, 1, 9};
    entropy.cbData = 5;
    entropy.pbData = entropyKey;
    if (!::CryptUnprotectData(&blob, NULL, &entropy, NULL, NULL, 0, &unencryptedBlob))
        return;
    wchar_t* data = (wchar_t*) (unencryptedBlob.pbData);
    decrypedString = Utf8String(data);
    LocalFree(unencryptedBlob.pbData);
#endif
    }