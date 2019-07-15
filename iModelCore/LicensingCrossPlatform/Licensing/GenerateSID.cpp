/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "GenerateSID.h"

#include <Bentley/BeStringUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <Bentley/WString.h>

#include <openssl/evp.h>

#if ! defined(SIZEOF_WCHAR)
#if WCHAR_MAX > 0x10000
#define SIZEOF_WCHAR 4
#else
#define SIZEOF_WCHAR 2
#endif
#endif

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GenerateSID::PrepareUserData(Utf8StringCR userName, Utf8StringCR machineName) const
    {
    Utf8String user = userName;
    user.ToLower();
    Utf8String newUserName = user;

    Utf8String machine = machineName;
    machine.ToLower();

    if (user.Contains("\\"))
        {
        newUserName = user.substr(user.find("\\", 0) + 1, user.length());
        }
    else if (user.Contains("/"))
        {
        newUserName = user.substr(user.find("/", 0) + 1, user.length());
        }
    
    if (newUserName.Contains("administrator") || newUserName.Contains("system"))
        {
        newUserName.Sprintf("%s\\%s", machine.c_str(), newUserName.c_str());
        }

    return newUserName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GenerateSID::PrepareMachineData(Utf8StringCR machineName) const
    {
    static Utf8String newMachineName = machineName;
    newMachineName.ToLower();

    
    if (newMachineName.Equals("::1") || newMachineName.Equals("127.0.0.1"))
        {
        newMachineName = "localhost";
        }

    return newMachineName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<Byte> GenerateSID::CalculateHash(std::vector<Byte> byteString) const
    {
    unsigned char binaryHash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;
    std::vector<Byte> hashBinaryString;
    EVP_MD_CTX *mdctx;

    if ((mdctx = EVP_MD_CTX_create()) == NULL)
        return hashBinaryString;

    if (!EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL))
        return hashBinaryString;

    if (!EVP_DigestUpdate(mdctx, &byteString[0], byteString.size()))
        return hashBinaryString;

    if (!EVP_DigestFinal_ex(mdctx, binaryHash, &hashLen))
        return hashBinaryString;

    EVP_MD_CTX_free(mdctx);

    hashBinaryString.resize(hashLen);
    memcpy(&hashBinaryString[0], binaryHash, hashLen);

    return hashBinaryString;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                        Jason.Wichert 4/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GenerateSID::GenerateSHA1Hash(Utf8StringCR inputString) const
    {
    std::vector<Byte> result;
    std::vector<Byte> data;
    WString fullString;
    Utf8String input = inputString;

    // Get Unicode bytes
    BeStringUtilities::Utf8ToWChar(fullString, (Utf8CP) input.c_str());

    result.resize(fullString.length() * sizeof(WChar));

    memcpy(&result[0], fullString.c_str(), result.size());

    // on linux and macOS, wchar_t has a byte size of 4, on windows it is 2
    std::vector<Byte> adjustedResult;
#if SIZEOF_WCHAR == 2
    adjustedResult = result;
#else
    // make byte array the same as in windows, to get an identical hash result
    adjustedResult.resize(result.size() / 2);

    int j = 0;
    for (int i = 0; i < result.size(); i += 2)
        {
        // take just the first 2 bytes of the array
        adjustedResult[j++] = result[i++];
        adjustedResult[j++] = result[i++];
        }
#endif

    for (int i = 0; i < NUM_SHA1HASH_ITERATIONS; i++)
        {
        data = adjustedResult;
        adjustedResult = CalculateHash(data);
        }

    return Base64Utilities::Encode((Utf8CP) &adjustedResult[0], adjustedResult.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GenerateSID::GetUserSID(Utf8StringCR userName, Utf8StringCR machineName) const
    {
    return GenerateSHA1Hash(PrepareUserData(userName, machineName));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GenerateSID::GetMachineSID(Utf8StringCR machineName) const
    {
    return GenerateSHA1Hash(PrepareMachineData(machineName));
    }
