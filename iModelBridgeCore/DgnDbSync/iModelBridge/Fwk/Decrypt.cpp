/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    
    char* stringBlob = new char[unencryptedBlob.cbData +1];
    memset(stringBlob, 0, unencryptedBlob.cbData+1);
    memcpy(stringBlob, unencryptedBlob.pbData, unencryptedBlob.cbData);
    decrypedString = Utf8String(stringBlob);
    delete[] stringBlob;
    LocalFree(unencryptedBlob.pbData);
#endif
    }