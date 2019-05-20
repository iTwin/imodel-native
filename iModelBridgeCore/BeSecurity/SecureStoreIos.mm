/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#include <BeSecurity/SecureStore.h>
#import "BEKeychainItem.h"
#import <CommonCrypto/CommonCryptor.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_SECURITY

#define KEY_NAMESPACE   "com.bentley.SecureStore"
#define KEY_ALIAS       "Key"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NSString* GetBundleSeedId()
    {
    // There is no easy way to get this value, so workaround is to pick first (or defalt) keychain access
    // group and parse it to extract first member - "<bundleSeedId>.<accessGroup>"

    NSDictionary *query = [NSDictionary dictionaryWithObjectsAndKeys:
                           (__bridge NSString *)kSecClassGenericPassword, (__bridge NSString *)kSecClass,
                           @"bundleSeedID", kSecAttrAccount,
                           @"", kSecAttrService,
                           (id)kCFBooleanTrue, kSecReturnAttributes,
                           nil];

    CFDictionaryRef result = nil;
    OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)query, (CFTypeRef *)&result);
    if (status == errSecItemNotFound)
        status = SecItemAdd((__bridge CFDictionaryRef)query, (CFTypeRef *)&result);

    if (status != errSecSuccess)
        return nil;

    NSString *accessGroup = [(__bridge NSDictionary *)result objectForKey:(__bridge NSString *)kSecAttrAccessGroup];
    NSArray *components = [accessGroup componentsSeparatedByString:@"."];
    NSString *bundleSeedID = [[components objectEnumerator] nextObject];
    CFRelease(result);
    return bundleSeedID;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BEKeychainItem* CreateKeychainItem (Utf8CP nameSpace, Utf8CP key)
    {
    id identifier = [NSString stringWithFormat:@"%s.%s", nameSpace, key];

    // Using AppId [$(teamID).com.example.AppOne] to use private Keychain. Using nil would pick up first accessGroup that could be shared.
    // https://developer.apple.com/documentation/security/keychain_services/keychain_items/sharing_access_to_keychain_items_among_a_collection_of_apps?language=objc
    // https://developer.apple.com/documentation/security/ksecattraccessgroup?language=objc

	id teamId = GetBundleSeedId();
	id bundleId = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"];
	id appId = [NSString stringWithFormat: @"%@.%@", teamId, bundleId];

    return [[BEKeychainItem alloc] initWithIdentifier:identifier accessGroup:appId];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SaveValueInternal(Utf8CP nameSpace, Utf8CP key, Utf8CP value)
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
Utf8String LoadValueInternal(Utf8CP nameSpace, Utf8CP key)
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
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetKeyBytesStr(bool createNewIfNeeded)
    {
    Utf8String keyStr = LoadValueInternal(KEY_NAMESPACE, KEY_ALIAS);

    if (keyStr.empty() && createNewIfNeeded)
        {
        uint8_t keyBytes[kCCKeySizeAES128];
        if (0 != SecRandomCopyBytes (kSecRandomDefault , kCCKeySizeAES128, keyBytes))
            {
            BeAssert(false);
            return nullptr;
            }

        keyStr = Base64Utilities::Encode((Utf8CP) keyBytes, (size_t) kCCKeySizeAES128);

        SaveValueInternal(KEY_NAMESPACE, KEY_ALIAS, keyStr.c_str());
        keyStr = LoadValueInternal(KEY_NAMESPACE, KEY_ALIAS);
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

    Utf8String keyStr = GetKeyBytesStr(true);
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

    Utf8String keyStr = GetKeyBytesStr(false);
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
