/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#include <Bentley/WString.h>
#include <openssl/x509.h>

USING_NAMESPACE_BENTLEY

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct TrustManager
    {
    private:
        static std::shared_ptr<bvector<bvector<Byte>>> ReadSystemTrustedCertificatesBinary();
        static std::shared_ptr<bvector<X509*>> ReadSystemTrustedCertificatesX509();

    public:
        static void Initialize(void* arg);
        static bool CanUseSystemTrustedCertificates();
        static Utf8String GetImplementationDescription();
        static std::shared_ptr<bvector<X509*>> GetSystemTrustedCertificates();
    };