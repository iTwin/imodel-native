/*--------------------------------------------------------------------------------------+
 |
 |     $Source: SecureStore.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <BeSecurity/SecureStore.h>
#include <Bentley/Base64Utilities.h>
#include <vector>

#if defined (BETHREAD_USE_PTHREAD)
#include <pthread.h>
#endif

#if defined(ANDROID)
#include <openssl/evp.h>
#include <android/log.h>
#include <Bentley/BeJStringUtilities.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#endif

#if defined (BENTLEY_WINRT)
#include <Stringapiset.h>
#include <Windows.h>
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Core;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
#endif

#if defined(BENTLEY_WIN32)
#include <Windows.h>
#include <Wincrypt.h>
#endif

USING_NAMESPACE_BENTLEY_SECURITY

#define LOCAL_STATE_NAMESPACE "fe_shape"

#if defined(ANDROID)
#define KEY_ALIAS "MobileDgnSecureStore"
#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_SECURITY))
jclass                  s_keyStoreCipherJClass = nullptr;
static JavaVM*          s_jvm;
static pthread_key_t    s_destructorKey;
#endif

#if defined (BENTLEY_WIN32) || defined(ANDROID)
#include <openssl/evp.h>
#endif

#if defined (BENTLEY_WIN32) || defined(ANDROID) || (BENTLEY_WINRT)

const unsigned char s_key[] =
    {
    0xdc, 0xbc, 0x10, 0x4d, 0x76, 0x6b, 0xd5, 0x79, 0x4c, 0x80, 0x4, 0xe, 0x17,
    0x91, 0x19, 0xb9, 0x48, 0xf7, 0x7e, 0x53, 0x6f, 0x7b, 0xb3, 0xb, 0xd5,
    0xbe, 0x78, 0xbf, 0x5, 0xc4, 0xc, 0x84, 0x54, 0x6d, 0x93, 0x4b, 0xde, 0x1c,
    0xd4, 0x62, 0x28, 0x18, 0xe2, 0x95, 0x4, 0x27, 0x56, 0x42, 0x0, 0x61, 0xe,
    0x57, 0xf6, 0x74, 0x84, 0x70, 0x92, 0x0, 0x7c, 0xda, 0x40, 0x9f, 0x1
    };

const unsigned char s_iv[] =
    {
    0x23, 0x56, 0x5c, 0x26, 0x3e, 0xa5, 0x31, 0x13, 0x90, 0xc9, 0xa4, 0xff,
    0xbc, 0xd8, 0x6d, 0x44, 0xd8, 0x6d, 0x44
    };

#endif

#if defined (BENTLEY_WINRT)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Sam.Rockwell    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IBuffer^ EncryptWinRT(
    const unsigned char* key,
    const unsigned char* iv,
    Utf8CP &password,
    Platform::String^ &strAlgName,
    uint32_t keyLength,
    BinaryStringEncoding encoding,
    IBuffer^ &ivWinRT,
    CryptographicKey^ &keyWinRT)
    {
    // Initialize the intialization vector.
    ivWinRT = nullptr;

    // Initialize the binary encoding value.
    encoding = BinaryStringEncoding::Utf8;

    // Create a buffer that contains the encoded message to be encrypted.

    WString bwString = WString (password, true);
    Platform::String^ pString = ref new Platform::String (bwString.GetWCharCP ());
    IBuffer^ buffMsg = CryptographicBuffer::ConvertStringToBinary (pString, encoding);

    // Open a symmetric algorithm provider for the specified algorithm.
    SymmetricKeyAlgorithmProvider^ objAlg = SymmetricKeyAlgorithmProvider::OpenAlgorithm (strAlgName);

    // Demonstrate how to retrieve the name of the algorithm used.
    Platform::String^ strAlgNameUsed = objAlg->AlgorithmName;

    // Determine whether the message length is a multiple of the block length.
    // This is not necessary for PKCS #7 algorithms which automatically pad the
    // message to an appropriate length.
    if (strAlgName->Equals (L"AES_CBC_PKCS7") == false)
        {
        if ((buffMsg->Length % objAlg->BlockLength) != 0)
            {
            throw "Message buffer length must be multiple of block length.";
            }
        }

    // Create a symmetric key.
    //IBuffer^ keyMaterial = CryptographicBuffer::GenerateRandom(keyLength);
    Utf8String utf8String = "";
    for (int i = 0; i < 63; ++i)
        {
        utf8String += key[i];
        }
    WString bwString1 = WString (utf8String.c_str (), true);
    Platform::String^ keyMaterialString = ref new Platform::String (bwString1.c_str ());
    IBuffer^ keyMaterialBuffer = CryptographicBuffer::ConvertStringToBinary (keyMaterialString, encoding);
    keyWinRT = objAlg->CreateSymmetricKey (keyMaterialBuffer);

    //BeAssert(0 == strcmp(utf8CP, utf8CPAssert.c_str()));

    // CBC algorithms require an initialization vector. Here, a random
    // number is used for the vector.
    if (strAlgName->Equals (L"AES_CBC_PKCS7") == true)
        {
        Utf8String utf8String1 = "";
        for (int i = 0; i < 19; ++i)
            {
            utf8String1 += iv[i];
            }
        WString bwString2 = WString (utf8String1.c_str (), true);
        Platform::String^ ivString = ref new Platform::String (bwString2.GetWCharCP ());
        ivWinRT = CryptographicBuffer::ConvertStringToBinary (ivString, encoding);
        }

    // Encrypt the data and return.
    IBuffer^ buffEncrypt = CryptographicEngine::Encrypt (keyWinRT, buffMsg, ivWinRT);
    return buffEncrypt;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Sam.Rockwell    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DecryptWinRT(
    const unsigned char* key,
    const unsigned char* iv,
    Platform::String^ &strAlgName,
    IBuffer^ &buffEncrypt,
    IBuffer^ &ivWinRT,
    BinaryStringEncoding encoding,
    CryptographicKey^ &keyWinRT)
    {
    IBuffer^ buffDecrypted;

    SymmetricKeyAlgorithmProvider^ objAlg = SymmetricKeyAlgorithmProvider::OpenAlgorithm (strAlgName);

    // Get the key from 
    Utf8String utf8String = "";
    for (int i = 0; i < 63; ++i)
        {
        utf8String += key[i];
        }
    WString bwString1 = WString (utf8String.c_str (), true);
    Platform::String^ keyMaterialString = ref new Platform::String (bwString1.GetWCharCP ());
    IBuffer^ keyMaterialBuffer = CryptographicBuffer::ConvertStringToBinary (keyMaterialString, encoding);
    keyWinRT = objAlg->CreateSymmetricKey (keyMaterialBuffer);

    // Get the iv
    Utf8String utf8String1 = "";
    for (int i = 0; i < 19; ++i)
        {
        utf8String1 += iv[i];
        }
    WString bwString2 = WString (utf8String1.c_str (), true);
    Platform::String^ ivString = ref new Platform::String (bwString2.GetWCharCP ());
    ivWinRT = CryptographicBuffer::ConvertStringToBinary (ivString, encoding);

    buffDecrypted = CryptographicEngine::Decrypt (keyWinRT, buffEncrypt, ivWinRT);

    Platform::String^ strDecrypted = CryptographicBuffer::ConvertBinaryToString (encoding, buffDecrypted);
    WString wStrDecrypted (strDecrypted->Data ());
    Utf8String password (wStrDecrypted);

    return password;
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SecureStore::SecureStore(ILocalState& customLocalState) :
m_localState (customLocalState)
    {
    }



/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SecureStore::~SecureStore()
    {
    }

#if !defined(ANDROID) && !defined(BENTLEYCONFIG_OS_APPLE_IOS)
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SecureStore::Initialize(void* arg)
    {
    }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE_IOS)

#if defined(ANDROID)
//---------------------------------------------------------------------------------------
// @bsimethod
//--------------+------------------------------------------------------------------------
static void threadDestructor(void *param)
    {
    LOG.debugv("threadDestructor, param=0x%x\n", param);
    s_jvm->DetachCurrentThread();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vincas.Razma    07/2015
//---------------------------------------------------------------------------------------
static JNIEnv* AttachCurrentThread ()
    {
    if (nullptr == s_jvm)
        {
        BeAssert (false);
        return nullptr;
        }
    
    // make sure the calling thread is attached to jvm
    JNIEnv* threadEnv = (JNIEnv*)pthread_getspecific(s_destructorKey);
    if (nullptr != threadEnv)
        return threadEnv;

    s_jvm->AttachCurrentThread (&threadEnv, nullptr);

    // threadEnv is not being used in threadDestructor(), but the
    // destructor won't be called if thread-specific value is NULL, so we
    // need to set it to something:
    int status = pthread_setspecific (s_destructorKey, threadEnv);
    if (status != 0)
        {
        LOG.errorv ("pthread_setspecific failed with %d\n", status);
        BeAssert (false);
        }

    JNIEnv* env = AttachCurrentThread ();
    BeAssert (env == threadEnv);
    return env;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vincas.Razma    07/2015
//---------------------------------------------------------------------------------------
static jclass GetGlobalClassRef (JNIEnv *env, const char* className)
    {
    if (nullptr == env)
        {
        BeAssert (false);
        return nullptr;
        }

    env->PushLocalFrame (1);

    jclass clazz = env->FindClass (className);
    if (nullptr == clazz)
        {
        LOG.errorv ("Could not find class %s", className);
        BeAssert (false);
        return nullptr;
        }

    jclass clazzRef = (jclass)env->NewGlobalRef (clazz);

    env->PopLocalFrame (nullptr);

    return clazzRef;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SecureStore::Initialize(void* arg)
    {
    JNIEnv* env = (JNIEnv*) arg;
    int status = pthread_key_create(&s_destructorKey, &threadDestructor);
    if (status != 0)
        {
        LOG.errorv("pthread_key_create failed with %d\n", status);
        BeAssert(false);
        }
    env->GetJavaVM (&s_jvm);
    s_keyStoreCipherJClass = GetGlobalClassRef (env, "com/bentley/android/security/KeyStoreCipher");
    BeAssert (nullptr != s_keyStoreCipherJClass);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
* Calls relevant KeyStoreCipher class methods.
* See KeyStoreCipher.java for encryption/decryption implementation details.
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CallJavaKeyStoreCipherMethod(Utf8CP methodName, Utf8CP input, Utf8CP alias)
    {
    JNIEnv* env = AttachCurrentThread();
    BeAssert (nullptr != env);

    // Local references will be attributed to this frame
    env->PushLocalFrame (16);

    auto methodSignature = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;";
    jmethodID  methodId = env->GetStaticMethodID (s_keyStoreCipherJClass, methodName, methodSignature);
    BeAssert (nullptr != methodId);

    jstring inputJ = BeJStringUtilities::JStringFromUtf8 (env, input);
    jstring aliasJ = BeJStringUtilities::JStringFromUtf8 (env, alias);
    jstring valueJ = (jstring)env->CallStaticObjectMethod (s_keyStoreCipherJClass, methodId, inputJ, aliasJ);

    Utf8String value;
    BeJStringUtilities::InitUtf8StringFromJString (env, value, valueJ);

    // Release local references making objects collectable
    env->PopLocalFrame (nullptr);

    return value;
    }

#endif

#if defined (BENTLEY_WIN32) || defined(ANDROID)
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ClearValueUsingLocalStateAndOpenSSL(ILocalState& localState, Utf8StringCR identifier)
    {
    if (identifier.empty ())
        {
        return;
        }
    localState.SaveValue (LOCAL_STATE_NAMESPACE, identifier.c_str (), "null");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String LoadValueUsingLocalStateAndOpenSSL(ILocalState& localState, Utf8StringCR identifier)
    {
    if (identifier.empty ())
        {
        return nullptr;
        }

    Utf8String encodedValue = Base64Utilities::Decode (localState.GetValue (LOCAL_STATE_NAMESPACE, identifier.c_str ()));

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_set_padding (&ctx, 0);
    EVP_CIPHER_CTX_init (&ctx);

    EVP_DecryptInit_ex (&ctx, EVP_aes_256_gcm (), nullptr, s_key, s_iv);

    std::vector<unsigned char> outbuf;
    // See documentation for EVP_DecryptUpdate at the page below for explanation of
    // the required size for outbuf:
    // https://www.openssl.org/docs/crypto/EVP_EncryptInit.html
    outbuf.resize (encodedValue.size () + EVP_CIPHER_CTX_block_size (&ctx));
    int outlen;
    if (!EVP_DecryptUpdate (&ctx, &outbuf[0], &outlen, (const unsigned char *)encodedValue.c_str (), (int)encodedValue.size ()))
        {
        return nullptr;
        }

    EVP_CIPHER_CTX_cleanup (&ctx);

    return Utf8String ((CharCP)&outbuf[0], (size_t)outlen);
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SecureStore::SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8CP value)
    {
    Utf8String identifier = CreateIdentifier (nameSpace, key);
    if (identifier.empty ())
        {
        return;
        }

    if (Utf8String::IsNullOrEmpty (value))
        {
        m_localState.SaveValue (LOCAL_STATE_NAMESPACE, identifier.c_str (), "");
        return;
        }

#if defined(ANDROID) || defined (BENTLEY_WIN32)

    Utf8String encrypted = Encrypt(value);
    m_localState.SaveValue(LOCAL_STATE_NAMESPACE, identifier.c_str(), encrypted);

#elif defined (BENTLEY_WINRT)

    // Initialize encryption
    Platform::String^ strAlgName = SymmetricAlgorithmNames::AesCbcPkcs7;
    uint32_t keyLength = 32;                                      // Length of the key in bytes
    BinaryStringEncoding encoding = BinaryStringEncoding::Utf8; // Binary encoding value
    IBuffer^ ivWinRT;                                           // Initialization vector
    CryptographicKey^ keyWinRT;                                 // Symmetric key

    // Encrypt
    IBuffer^ buffEncrypted = EncryptWinRT (s_key, s_iv, value, strAlgName, keyLength, encoding, ivWinRT, keyWinRT);

    Platform::String^ buffEncryptedString = CryptographicBuffer::EncodeToBase64String (buffEncrypted);
    Utf8String encodedvalue (buffEncryptedString->Data ());

    // Save
    m_localState.SaveValue (LOCAL_STATE_NAMESPACE, identifier.c_str (), encodedvalue);

#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::LoadValue (Utf8CP nameSpace, Utf8CP key)
    {
    Utf8String identifier = CreateIdentifier (nameSpace, key);
    if (identifier.empty ())
        {
        return nullptr;
        }

#if defined(ANDROID) || defined (BENTLEY_WIN32)

    Utf8String encrypted = m_localState.GetValue (LOCAL_STATE_NAMESPACE, identifier.c_str ());
    return Decrypt(encrypted.c_str());

#elif defined (BENTLEY_WINRT)
    // Initialize the encryption process
    Platform::String^ strAlgName = SymmetricAlgorithmNames::AesCbcPkcs7;    //Do not change this or you will need to update encryption/decryption logic
    BinaryStringEncoding encoding = BinaryStringEncoding::Utf8;     // Binary encoding value
    IBuffer^ ivWinRT;                                               // Initialization vector
    CryptographicKey^ keyWinRT;                                     // Symmetric key
    SymmetricKeyAlgorithmProvider^ objAlg = SymmetricKeyAlgorithmProvider::OpenAlgorithm (strAlgName);

    // Read data
    Utf8String utf8Value = m_localState.GetValue (LOCAL_STATE_NAMESPACE, identifier.c_str ());
    if (utf8Value.empty()) // if value is empty then this represents a deleted item and needs no decryption
        return Utf8String("");

    WString bwString = WString (utf8Value.c_str (), true);
    Platform::String^ pString = ref new Platform::String (bwString.c_str ());
    IBuffer^ buffEncrypted = CryptographicBuffer::DecodeFromBase64String (pString);

    // Decrypt
    return DecryptWinRT (s_key, s_iv, strAlgName, buffEncrypted, ivWinRT, encoding, keyWinRT);

#else

    #pragma message("Unsupported platform")
    BeAssert(false && "Not implemented.");
    return nullptr;

#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::CreateIdentifier (Utf8CP nameSpace, Utf8CP key)
    {
    if (Utf8String::IsNullOrEmpty (nameSpace) || Utf8String::IsNullOrEmpty (key))
        {
        return "";
        }
    if (nullptr != strchr (nameSpace, ':'))
        {
        BeAssert (false && "Namespace cannot contain ':'");
        return "";
        }
    return Utf8PrintfString ("%s:%s", nameSpace, key);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::LegacyLoadValue (Utf8CP nameSpace, Utf8CP key)
    {
#if defined (BENTLEY_WIN32) || defined(ANDROID)
    return LoadValueUsingLocalStateAndOpenSSL (m_localState, CreateIdentifier (nameSpace, key));
#else
    // No changes
    return LoadValue (nameSpace, key);
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SecureStore::LegacyClearValue (Utf8CP nameSpace, Utf8CP key)
    {
#if defined (BENTLEY_WIN32) || defined(ANDROID)
    return ClearValueUsingLocalStateAndOpenSSL (m_localState, CreateIdentifier (nameSpace, key));
#else
    // No changes
    SaveValue (nameSpace, key, nullptr);
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::Encrypt(Utf8CP value)
    {
    if (nullptr == value)
        {
        static const Utf8CP empty = "";
        value = empty;
        }

#if defined(ANDROID)

    return CallJavaKeyStoreCipherMethod("Encrypt", value, KEY_ALIAS);

#elif defined (BENTLEY_WINRT)

    BeAssert(false && "Not implemented. Investigate if CryptProtectData() approach could be compiled");
    return nullptr;

#elif defined (BENTLEY_WIN32)

    DATA_BLOB plaintextBlob;
    plaintextBlob.pbData = (BYTE*) value;
    plaintextBlob.cbData = (DWORD) strlen(value);

    DATA_BLOB ciphertextBlob;
    ciphertextBlob.pbData = NULL;
    ciphertextBlob.cbData = 0;

    if (!CryptProtectData(&plaintextBlob, NULL, NULL, NULL, NULL, 0, &ciphertextBlob))
        {
        BeAssert(false && "wncryption failed");
        //DGNCLIENTFX_LOGE(L"SecureStore: encryption failed (GLE=%d)", GetLastError());
        return nullptr;
        }

    Utf8String encrypted = Base64Utilities::Encode((Utf8CP) ciphertextBlob.pbData, (size_t) ciphertextBlob.cbData);
    LocalFree(ciphertextBlob.pbData);
    return encrypted;

#else

    #pragma message("Unsupported platform")
    BeAssert(false && "Not implemented.");
    return nullptr;

#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SecureStore::Decrypt(Utf8CP encrypted)
    {
    if (Utf8String::IsNullOrEmpty(encrypted))
        return nullptr;

#if defined(ANDROID)

    return CallJavaKeyStoreCipherMethod("Decrypt", encrypted, KEY_ALIAS);

#elif defined (BENTLEY_WINRT)

    BeAssert(false && "Not implemented. Investigate if CryptUnprotectData() approach could be compiled");
    return nullptr;

#elif defined (BENTLEY_WIN32)
    // Windows Desktop and Windows Store Apps

    Utf8String ciphertext = Base64Utilities::Decode(encrypted);
    if (ciphertext.empty())
        return nullptr;

    DATA_BLOB ciphertextBlob;
    ciphertextBlob.pbData = (BYTE*) ciphertext.c_str();
    ciphertextBlob.cbData = (DWORD) ciphertext.length();

    DATA_BLOB plaintextBlob;
    plaintextBlob.pbData = NULL;
    plaintextBlob.cbData = 0;;

    if (!CryptUnprotectData(&ciphertextBlob, NULL, NULL, NULL, NULL, 0, &plaintextBlob))
        {
        BeAssert(false && "decryption failed");
        //DGNCLIENTFX_LOGE(L"SecureStore: decryption failed (GLE=%d)", GetLastError());
        return nullptr;
        }

    Utf8String decrypted((CharCP) plaintextBlob.pbData, (size_t) plaintextBlob.cbData);
    LocalFree(plaintextBlob.pbData);
    return decrypted;

#else

    #pragma message("Unsupported platform")
    BeAssert(false && "Not implemented.");
    return nullptr;

#endif
    }

#endif // !defined (BENTLEYCONFIG_OS_APPLE_IOS)
