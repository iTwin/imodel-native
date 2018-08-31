/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeSecurity/SecureStore.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeSecurity/BeSecurity.h>
#include <Bentley/LocalState.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_SECURITY_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ICipher> ICipherPtr;
struct ICipher
    {
    public:
        virtual Utf8String Encrypt(Utf8CP value) = 0;
        virtual Utf8String Decrypt(Utf8CP value) = 0;
    };
    
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ISecureStore> ISecureStorePtr;
struct ISecureStore : public ICipher
    {
    public:
        virtual ~ISecureStore () {};
        virtual void SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP value) = 0;
        virtual Utf8String LoadValue (Utf8CP nameSpace, Utf8CP key) = 0;

        //! DEPRECATED!
        virtual Utf8String LegacyLoadValue (Utf8CP nameSpace, Utf8CP key) { return nullptr; };
        //! DEPRECATED!
        virtual void LegacyClearValue(Utf8CP nameSpace, Utf8CP key) {};
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct SecureStore : public ISecureStore
    {
    private:
        ILocalState& m_localState;

    private:
        Utf8String CreateIdentifier (Utf8CP nameSpace, Utf8CP key);

        //__PUBLISH_SECTION_END__
    public:
        //! [iOS] SecureStore uses one keychain  access group prefix to generate full keychain access group string. 
        //!     Initialize with const char*. Prefix example: "XXXXXX.com.organization."
        //! [Android] SecureStore uses KeyStore to store private keys. Initialize with JNIEnv*
        //! [Win][WinRT] Initialize does nothing
        BESECURITY_EXPORT static void Initialize(void* arg);
        //__PUBLISH_SECTION_START__
    public:
        //! Create new persistence with default DgnClientApp::LocalState or custom.
        //! NOTE: For iOS LocalState is not used - iOS KeyChain is used instead.
        BESECURITY_EXPORT SecureStore (ILocalState& customLocalState);
        BESECURITY_EXPORT ~SecureStore ();

        //! Stores value for given key in namespace. If value is empty, key-value pair is deleted. If key or namespace is empty nothing is done.
        //! Value is encrypted and stored in persistent storage and implementation depends on platform:
        //! iOS: Using Keychain. DgnClientFxIos::Initialize() accepts keychain access group prefix that is required when using SecureStore.
        //!      Access group prefix is combined with nameSpace parameter and given access group is then used to store value with key as identifier.
        //!      Application needs to be signed with access group in order for secure store to work. Example:
        //!         Initialized access group prefix: "XXXXXX.com.organization."
        //!         nameSpace parameter: "Passwords"
        //!         key parameter: "MainPassword"
        //!         value parameter: string to be encrypted and stored
        //!         Application needs to be signed with Code Signing Entitlements to use "XXXXXX.com.organization.Passwords" keychain access group.
        //!         Value will be saved to "XXXXXX.com.organization.Passwords" access group with identifier "MainPassword"
        //!      Because global KeyChain is used, apps that have access to same access groups can override and read values with same identifiers.
        //! Android: Value is encrypted and stored locally. Encryption keys are encrypted/saved using KeyStore.
        //!      Note: API versions prior to 18 cannot use KeyStore if device lock screen is not password/pin/pattern protected.
        //!      If that is the case, popup requiring for user to set lock screen password is shown and app is shut down.
        //! Windows Desktop, Windows RT: TBD. Value is encrypted and saved locally.
        BESECURITY_EXPORT void SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP value) override;

        //! Loads value for given key in namespace. Empty string is returned when value not found or other error occurs.
        BESECURITY_EXPORT Utf8String LoadValue (Utf8CP nameSpace, Utf8CP key) override;

        //! Protect data using platform specific encryption. For more information see SaveValue().
        //! iOS: Key is stored in Keychain. Application needs to be signed with <XXXXXX.com.organization>.Keys access group.
        //! Android, Windows Desktop, Windows Store - managed automatically.
        BESECURITY_EXPORT Utf8String Encrypt(Utf8CP value) override;

        //! Decrypt value that was encrypted using Encrypt().
        BESECURITY_EXPORT Utf8String Decrypt(Utf8CP value) override;
    };

END_BENTLEY_SECURITY_NAMESPACE
