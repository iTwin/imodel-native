/*--------------------------------------------------------------------------------------+
 |
 |     $Source: SecureStore.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <BeSecurity/SecureStore.h>
#include <Bentley/Base64Utilities.h>

#if defined (BETHREAD_USE_PTHREAD)
#include <pthread.h>
#endif

#if defined(ANDROID) || defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
#include <openssl/evp.h>
#endif

#if defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
#include <Windows.h>
#include <Wincrypt.h>
#include <dpapi.h>
#endif

USING_NAMESPACE_BENTLEY_SECURITY

#define LOCAL_STATE_NAMESPACE "fe_shape"
#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_SECURITY))

#if defined(ANDROID)
#include <Bentley/BeJStringUtilities.h>
#define KEY_ALIAS "MobileDgnSecureStore"
jclass                  s_keyStoreCipherJClass = nullptr;
static JavaVM*          s_jvm;
static pthread_key_t    s_destructorKey;
#endif

#if defined (BENTLEY_WIN32) || defined(ANDROID) || defined(BENTLEY_WINRT)

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
        m_localState.SaveValue (LOCAL_STATE_NAMESPACE, identifier.c_str (), "null");
        return;
        }

#if defined(ANDROID) || defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    Utf8String encrypted = Encrypt(value);
    m_localState.SaveValue(LOCAL_STATE_NAMESPACE, identifier.c_str(), encrypted);
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

#if defined(ANDROID) || defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    Utf8String encrypted = m_localState.GetValue (LOCAL_STATE_NAMESPACE, identifier.c_str ());
    return Decrypt(encrypted.c_str());
#else
    BeAssert("Platform not supported!");
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

#elif defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    DATA_BLOB plaintextBlob;
    plaintextBlob.pbData = (BYTE*) value;
    plaintextBlob.cbData = (DWORD) strlen(value);

    DATA_BLOB ciphertextBlob;
    ciphertextBlob.pbData = NULL;
    ciphertextBlob.cbData = 0;

    if (!CryptProtectData(&plaintextBlob, NULL, NULL, NULL, NULL, 0, &ciphertextBlob))
        {
        BeAssert(false && "wncryption failed");
        LOG.errorv("SecureStore: encryption failed (GLE=%d)", GetLastError());
        return nullptr;
        }

    Utf8String encrypted = Base64Utilities::Encode((Utf8CP) ciphertextBlob.pbData, (size_t) ciphertextBlob.cbData);
    LocalFree(ciphertextBlob.pbData);
    return encrypted;
        
#else

    BeAssert("Platform not supported!");
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

#elif defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
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
        LOG.errorv("SecureStore: decryption failed (GLE=%d)", GetLastError());
        return nullptr;
        }

    Utf8String decrypted((CharCP) plaintextBlob.pbData, (size_t) plaintextBlob.cbData);
    LocalFree(plaintextBlob.pbData);
    return decrypted;
        
#else

    BeAssert("Platform not supported!");
    return nullptr;

#endif
    }

#endif // !defined (BENTLEYCONFIG_OS_APPLE_IOS)
