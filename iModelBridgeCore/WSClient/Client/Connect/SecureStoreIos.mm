/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/Connect/SecureStoreIos.mm $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <WebServices/Client/Connect/SecureStore.h>
#import "BEKeychainItem.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SecureStore::SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP password)
    {
    Utf8String identifier = CreateIdentifier (nameSpace, key);
    if (identifier.empty ())
        {
        return;
        }

    BEKeychainItem* item = [BEKeychainItem wsbKeychainItemWithIdentifier:[NSString stringWithUTF8String:identifier.c_str ()]];
    item.username = [NSString stringWithUTF8String:identifier.c_str ()];
    item.password = [NSString stringWithUTF8String:password];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::LoadValue (Utf8CP nameSpace, Utf8CP key)
    {
    Utf8String identifier = CreateIdentifier (nameSpace, key);
    Utf8String password;

    if (identifier.empty ())
        {
        return password;
        }

    BEKeychainItem* item = [BEKeychainItem wsbKeychainItemWithIdentifier:[NSString stringWithUTF8String:identifier.c_str ()]];
    return [item.password UTF8String];
    }