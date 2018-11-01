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
        //! Initialize before using.
        //! [Winows]    Does nothing
        //! [iOS]       Does nothing
        //! [Android]   SecureStore uses KeyStore to store private keys. Initialize with JNIEnv*
        BESECURITY_EXPORT static void Initialize(void* arg);
        //__PUBLISH_SECTION_START__
    public:
        //! Create new secure store object with default local state to store encrypted data.
        BESECURITY_EXPORT SecureStore(ILocalState& localState);
        BESECURITY_EXPORT ~SecureStore();

        //! Stores value for given key in namespace. If value is empty, key-value pair is deleted. If key or namespace is empty nothing is done.
        //! Value is encrypted with Encrypt() and stored to local state.
        BESECURITY_EXPORT void SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP value) override;

        //! Loads value for given key in namespace. See SaveValue() for more info. Empty string is returned when value not found or other error occurs.
        //! Value is loaded from local state and decrypted with Decrypt().
        BESECURITY_EXPORT Utf8String LoadValue (Utf8CP nameSpace, Utf8CP key) override;

        //! Protect data using platform specific encryption.
        //! Windows:
        //!     Encryption is done using OS APIs, security is per-user and per-device.
        //! iOS:
        //!     Encryption keys are stored using private app Keychain. Security is per-app.
        //! Android:
        //!     Encryption keys are stored using KeyStore. Security is per-app.
        //!         Note: API versions prior to 18 cannot use KeyStore if device lock screen is not password/pin/pattern protected.
        //!         If that is the case, popup requiring for user to set lock screen password is shown and app is shut down.
        BESECURITY_EXPORT Utf8String Encrypt(Utf8CP value) override;

        //! Decrypt value that was encrypted using Encrypt(). See Encrypt() for more info.
        BESECURITY_EXPORT Utf8String Decrypt(Utf8CP value) override;
    };

END_BENTLEY_SECURITY_NAMESPACE
