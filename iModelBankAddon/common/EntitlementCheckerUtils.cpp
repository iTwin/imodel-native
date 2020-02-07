/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "EntitlementCheckerUtils.h"
#include <openssl/sha.h>
#include <Bentley/WString.h>
#include <Bentley/Base64Utilities.h>

using namespace IModelBank;

namespace IModelBank
{
Utf8String calculateHash(Utf8String activityId, Utf8String iModelId, Utf8String time)
{
    Utf8String salt = "Salt for iModelBank entitlement checking...";

    unsigned char digest[SHA512_DIGEST_LENGTH];
    SHA512_CTX sha512;
    SHA512_Init(&sha512);

    SHA512_Update(&sha512, activityId.c_str(), activityId.length());
    SHA512_Update(&sha512, iModelId.c_str(), iModelId.length());
    SHA512_Update(&sha512, time.c_str(), time.length());
    SHA512_Update(&sha512, salt.c_str(), salt.length());

    SHA512_Final(digest, &sha512);

    Utf8String hash;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
        hash += Utf8PrintfString("%02x", (unsigned int)digest[i]);


    return hash;
}
}
