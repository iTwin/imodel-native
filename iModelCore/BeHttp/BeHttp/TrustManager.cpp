/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/TrustManager.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "TrustManager.h"

#include <Bentley/BeTimeUtilities.h>
#include "WebLogging.h"

#if defined (BETHREAD_USE_PTHREAD)
#include <pthread.h>
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<bvector<X509*>> TrustManager::GetSystemTrustedCertificates()
    {
    static auto certs = ReadSystemTrustedCertificatesX509();
    return certs;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<bvector<X509*>> TrustManager::ReadSystemTrustedCertificatesX509()
    {
    auto start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    auto certsBinary = ReadSystemTrustedCertificatesBinary();
    if (nullptr == certsBinary)
        {
        LOG.error("TrustManager: Failed to get system trusted certificates");
        return nullptr;
        }

    auto certsX509 = std::make_shared<bvector<X509*>>();
    certsX509->reserve(certsBinary->size());

    for (auto& certBinary : *certsBinary)
        {
        auto data = (const unsigned char *) certBinary.data();
        auto size = certBinary.size();

        X509* certX509 = d2i_X509(nullptr, &data, (long) size);
        if (nullptr == certX509)
            {
            LOG.error("TrustManager: Failed to parse X509 certificate");
            continue;
            }

        certsX509->push_back(certX509);
        }

    if (certsX509->size() != certsBinary->size())
        {
        LOG.errorv("TrustManager: failed to parse %d/%d system certificates",
            certsBinary->size() - certsX509->size(), certsBinary->size());
        }

    // Sort to get reproducible result
    std::sort(certsX509->begin(), certsX509->end(), [] (X509* a, X509* b)
        {
        return X509_NAME_cmp(X509_get_issuer_name(a), X509_get_issuer_name(b));
        });

    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        {
        char buffer[256];
        for (auto& certX509 : *certsX509)
            {
            X509_NAME* name = X509_get_issuer_name(certX509);
            X509_NAME_oneline(name, buffer, sizeof(buffer));
            LOG.debugv("TrustManager: Parsed X509 certificate '%s'", buffer);
            }
        }

    auto end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();

    LOG.debugv("TrustManager: read %d system trusted certificates in %.2f seconds.", certsX509->size(), (end - start) / 1000);

    return certsX509;
    }

#if defined(ANDROID)

#include <Bentley/BeJStringUtilities.h>

static jclass           s_beHttpJClass = nullptr;
static JavaVM*          s_jvm = nullptr;
static pthread_key_t    s_destructorKey;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void threadDestructor(void *param)
    {
    LOG.debugv("threadDestructor, param=0x%x\n", param);
    s_jvm->DetachCurrentThread();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static JNIEnv* AttachCurrentThread()
    {
    if (nullptr == s_jvm)
        return nullptr;

    // make sure the calling thread is attached to jvm
    JNIEnv* threadEnv = (JNIEnv*) pthread_getspecific(s_destructorKey);
    if (nullptr != threadEnv)
        return threadEnv;

    s_jvm->AttachCurrentThread(&threadEnv, nullptr);

    // threadEnv is not being used in threadDestructor(), but the
    // destructor won't be called if thread-specific value is NULL, so we
    // need to set it to something:
    int status = pthread_setspecific(s_destructorKey, threadEnv);
    if (status != 0)
        {
        LOG.errorv("pthread_setspecific failed with %d\n", status);
        BeAssert(false);
        }

    JNIEnv* env = AttachCurrentThread();
    BeAssert(env == threadEnv);
    return env;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static jclass GetGlobalClassRef(JNIEnv *env, const char* className)
    {
    if (nullptr == env)
        return nullptr;

    env->PushLocalFrame(1);

    jclass clazz = env->FindClass(className);
    if (nullptr == clazz)
        {
        env->ExceptionClear();
        LOG.errorv("Could not find class %s", className);
        return nullptr;
        }

    jclass clazzRef = (jclass) env->NewGlobalRef(clazz);

    env->PopLocalFrame(nullptr);

    return clazzRef;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TrustManager::Initialize(void* arg)
    {
    JNIEnv* env = (JNIEnv*) arg;
    int status = pthread_key_create(&s_destructorKey, &threadDestructor);
    if (status != 0)
        {
        LOG.errorv("pthread_key_create failed with %d", status);
        return;
        }

    JavaVM* jvm = nullptr;
    env->GetJavaVM(&jvm);

    s_beHttpJClass = GetGlobalClassRef(env, "com/bentley/android/http/BeHttp");
    if (nullptr == s_beHttpJClass)
        {
        LOG.error("TrustManager: Missing JAVA classes, load com.bentley.http.BeHttp package first!");
        return;
        }

    s_jvm = jvm;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<bvector<bvector<Byte>>> TrustManager::ReadSystemTrustedCertificatesBinary()
    {
    JNIEnv* env = AttachCurrentThread();
    if (nullptr == env)
        {
        LOG.error("TrustManager: Not initialized, call C++ HttpClient::InitializePlatform() first!");
        BeAssert(false);
        return nullptr;
        }

    // Local references will be attributed to this frame
    env->PushLocalFrame(16);

    jmethodID  methodId = env->GetStaticMethodID(s_beHttpJClass, "ReadSystemTrustedCertificatesBinary", "()[[B");
    if (nullptr == methodId)
        return nullptr;

    jstring valueJ = (jstring) env->CallStaticObjectMethod(s_beHttpJClass, methodId);
    if (nullptr == valueJ)
        return nullptr;

    jobjectArray* valueArrayJ = reinterpret_cast<jobjectArray*>(&valueJ);
    jsize valueArrayLenJ = env->GetArrayLength(*valueArrayJ);

    auto certs = std::make_shared<bvector<bvector<Byte>>>();
    certs->resize(valueArrayLenJ);

    for (jsize i = 0; i < valueArrayLenJ; i++)
        {
        jobject objArrayJ = env->GetObjectArrayElement(*valueArrayJ, i);
        jbyteArray* byteArrayJ = reinterpret_cast<jbyteArray*>(&objArrayJ);

        jsize byteArrayLenJ = env->GetArrayLength(*byteArrayJ);
        certs->at(i).resize(byteArrayLenJ);

        env->GetByteArrayRegion(*byteArrayJ, 0, byteArrayLenJ, reinterpret_cast<jbyte*>(certs->at(i).data()));
        }

    // Release local references making objects collectable
    env->PopLocalFrame(nullptr);

    return certs;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TrustManager::GetImplementationDescription() 
    { 
    return "TLS certificate integration: AndroidCAStore";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool TrustManager::CanUseSystemTrustedCertificates()
    {
    return true;
    }

#else // ANDROID

void TrustManager::Initialize(void* arg) {}
bool TrustManager::CanUseSystemTrustedCertificates() { return false; }
Utf8String TrustManager::GetImplementationDescription() { return ""; }
std::shared_ptr<bvector<bvector<Byte>>> TrustManager::ReadSystemTrustedCertificatesBinary() { return nullptr; }

#endif // NON-ANDROID