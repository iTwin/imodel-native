/*--------------------------------------------------------------------------------------+
 |
 |     $Source: SecureStoreIos.mm $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <BeSecurity/SecureStore.h>
#import "BEKeychainItem.h"
#import <CommonCrypto/CommonCryptor.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_SECURITY

#define KEY_NAMESPACE   "Keys"
#define KEY_ALIAS       "MobileDgnSecureStore"

static NSString* g_accessGroupPrefix = nil;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SecureStore::Initialize (void* prefix)
    {
    g_accessGroupPrefix = [NSString stringWithUTF8String:(const char*)prefix];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NSString* CreateAccessGroup (Utf8CP postfix)
    {
    if (nil == g_accessGroupPrefix)
         {
         BeAssert (false && "Keychain access group prefix not initialized");
         // Return invalid access group
         return @"";
         }
         
    return [g_accessGroupPrefix stringByAppendingString:[NSString stringWithUTF8String:postfix]];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BEKeychainItem* CreateKeychainItem (Utf8CP accessGroupPostfix, Utf8CP identifier)
    {
    return [[BEKeychainItem alloc] 
                initWithIdentifier:[NSString stringWithUTF8String:identifier] 
                accessGroup:CreateAccessGroup (accessGroupPostfix)];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SecureStore::SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP value)
    {
    if (Utf8String::IsNullOrEmpty (nameSpace) ||
        Utf8String::IsNullOrEmpty (key))
        {
        return;
        }

    BEKeychainItem* item = CreateKeychainItem (nameSpace, key);
    if (Utf8String::IsNullOrEmpty (value))
        {
        [item deleteItem];
        return;
        }

    item.username = [NSString stringWithUTF8String:key];
    item.password = [NSString stringWithUTF8String:value];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::LoadValue (Utf8CP nameSpace, Utf8CP key)
    {
    if (Utf8String::IsNullOrEmpty (nameSpace) ||
        Utf8String::IsNullOrEmpty (key))
        {
        return "";
        }

    BEKeychainItem* item = CreateKeychainItem (nameSpace, key);
    return [item.password UTF8String];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::LegacyLoadValue (Utf8CP nameSpace, Utf8CP key)
    {
    // Graphite0503 logic
    return LoadValue ("WSB", Utf8PrintfString ("%s:%s", nameSpace, key).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SecureStore::LegacyClearValue (Utf8CP nameSpace, Utf8CP key)
    {
    // Graphite0503 logic
    SaveValue ("WSB", Utf8PrintfString ("%s:%s", nameSpace, key).c_str(), nullptr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetKeyBytesStr(ISecureStore& store, bool createNewIfNeeded)
    {
    Utf8String keyStr = store.LoadValue(KEY_NAMESPACE, KEY_ALIAS);

    if (keyStr.empty() && createNewIfNeeded)
        {
        uint8_t keyBytes[kCCKeySizeAES128];
        if (0 != SecRandomCopyBytes (kSecRandomDefault , kCCKeySizeAES128, keyBytes))
            {
            BeAssert(false);
            return nullptr;
            }

        keyStr = Base64Utilities::Encode((Utf8CP) keyBytes, (size_t) kCCKeySizeAES128);

        store.SaveValue (KEY_NAMESPACE, KEY_ALIAS, keyStr.c_str());
        keyStr = store.LoadValue(KEY_NAMESPACE, KEY_ALIAS);
        }

    if (keyStr.empty())
        {
        BeAssert(false);
        return nullptr;
        }

    return Base64Utilities::Decode(keyStr.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::Encrypt(Utf8CP valueStr)
    {
    if (nullptr == valueStr)
        {
        static const Utf8CP empty = "";
        valueStr = empty;
        }

    Utf8String keyStr = GetKeyBytesStr(*this, true);    
    if (keyStr.empty())
        {
        BeAssert(false);
        return nullptr;
        }

    // Create buffer for output
    size_t valueSize = strlen(valueStr);
    size_t ivSize = kCCBlockSizeAES128;
    size_t cryptBufferSize = valueSize + kCCBlockSizeAES128;
    size_t totalBufferSize = ivSize + cryptBufferSize;
    std::unique_ptr<char[]> output(new char[totalBufferSize]);

    // Generate initialization vector into start of output
    if (0 != SecRandomCopyBytes (kSecRandomDefault , ivSize, (uint8_t*)output.get()))
        {
        BeAssert(false);
        return nullptr;
        }

    char* iv = output.get();
    char* cryptBuffer = output.get() + ivSize;

    size_t resultSize = 0;
    CCCryptorStatus cryptStatus = CCCrypt
        (
        kCCEncrypt, kCCAlgorithmAES128, kCCOptionPKCS7Padding,
        keyStr.c_str(), kCCKeySizeAES128, iv,
        valueStr, valueSize,
        cryptBuffer, cryptBufferSize, &resultSize
        );

    if (kCCSuccess != cryptStatus) 
        {
        BeAssert(false);
        return nullptr;
        }

    return Base64Utilities::Encode(output.get(), ivSize + resultSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::Decrypt(Utf8CP valueStr)
    {
    if (Utf8String::IsNullOrEmpty(valueStr))
        return nullptr;

    Utf8String keyStr = GetKeyBytesStr(*this, false);
    if (keyStr.empty())
        return nullptr;

    Utf8String input = Base64Utilities::Decode(valueStr, strlen(valueStr));

    // Start of input contains initialization vector
    size_t ivSize = kCCBlockSizeAES128;
    if (input.size() <= ivSize)
        {
        BeAssert(false);
        return nullptr;
        }

    // Encrypted value is next to iv
    const char* iv = input.c_str();
    const char* encrypted = input.c_str() + ivSize;
    size_t encryptedSize = input.size() - ivSize;

    size_t outputSize = input.size() + kCCBlockSizeAES128;
    std::unique_ptr<char[]> output(new char[outputSize]);

    size_t resultSize = 0;
    CCCryptorStatus cryptStatus = CCCrypt
        (
        kCCDecrypt, kCCAlgorithmAES128, kCCOptionPKCS7Padding,
        keyStr.c_str(), kCCKeySizeAES128, iv,
        encrypted, encryptedSize,
        output.get(), outputSize, &resultSize
        );

    if (kCCSuccess != cryptStatus) 
        {
        BeAssert(false);
        return nullptr;
        }

    return Utf8String(output.get(), resultSize);
    }
