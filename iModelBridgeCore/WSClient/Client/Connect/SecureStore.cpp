/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/Connect/SecureStore.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/Connect/SecureStore.h>
#include <Bentley/Base64Utilities.h>

#if defined (BENTLEY_WINRT)
#include <Stringapiset.h>
#include <Windows.h>
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Core;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
#endif

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN

#if defined (BENTLEY_WIN32) || defined(ANDROID)
#include <openssl/evp.h>
#endif

#if defined (BENTLEY_WIN32) || defined(ANDROID) || (BENTLEY_WINRT)

#define LOCAL_STATE_NAMESPACE "fe_shape"

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
IBuffer^ Encrypt (
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
    ivWinRT = NULL;

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
Utf8String Decrypt (
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
SecureStore::SecureStore (ILocalState* customLocalState) :
m_localState (customLocalState ? *customLocalState : MobileDgnApplication::App ().LocalState ())
    {
    }

#if !defined (__APPLE__)
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

#if defined (BENTLEY_WIN32) || defined(ANDROID)
    if (Utf8String::IsNullOrEmpty (password))
        {
        m_localState.SaveValue (LOCAL_STATE_NAMESPACE, identifier.c_str (), Json::nullValue);
        return;
        }

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_set_padding (&ctx, 0);
    EVP_CIPHER_CTX_init (&ctx);

    EVP_EncryptInit_ex (&ctx, EVP_aes_256_gcm (), nullptr, s_key, s_iv);

    std::vector<unsigned char> outbuf;
    size_t passwordLen = strlen (password);
    // See documentation for EVP_EncryptUpdate at the page below for explanation of
    // the required size for outbuf:
    // https://www.openssl.org/docs/crypto/EVP_EncryptInit.html
    outbuf.resize(passwordLen + EVP_CIPHER_CTX_block_size (&ctx) - 1);
    int outlen;
    if (!EVP_EncryptUpdate (&ctx, &outbuf[0], &outlen, (const unsigned char *)password, (int)passwordLen))
        {
        return;
        }

    EVP_CIPHER_CTX_cleanup (&ctx);

    Utf8String encodedPassword = Base64Utilities::Encode ((Utf8CP)&outbuf[0], outlen);

    m_localState.SaveValue (LOCAL_STATE_NAMESPACE, identifier.c_str (), encodedPassword);
#endif

#if defined (BENTLEY_WINRT)
    // If password is empty, we are attempting to delete this item. Its password should not be encrypted.
    if (Utf8String::IsNullOrEmpty (password))
        {
        m_localState.SaveValue (LOCAL_STATE_NAMESPACE, identifier.c_str (), Json::nullValue);
        return;
        }

    // Initialize encryption
    Platform::String^ strAlgName = SymmetricAlgorithmNames::AesCbcPkcs7;
    uint32_t keyLength = 32;                                      // Length of the key in bytes
    BinaryStringEncoding encoding = BinaryStringEncoding::Utf8; // Binary encoding value
    IBuffer^ ivWinRT;                                           // Initialization vector
    CryptographicKey^ keyWinRT;                                 // Symmetric key

    // Encrypt
    IBuffer^ buffEncrypted = Encrypt (s_key, s_iv, password, strAlgName, keyLength, encoding, ivWinRT, keyWinRT);

    Platform::String^ buffEncryptedString = CryptographicBuffer::EncodeToBase64String (buffEncrypted);
    Utf8String encodedPassword (buffEncryptedString->Data ());

    // Save
    m_localState.SaveValue (LOCAL_STATE_NAMESPACE, identifier.c_str (), encodedPassword);

#endif
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

#if defined (BENTLEY_WIN32) || defined(ANDROID)
    Utf8String encodedPassword = Base64Utilities::Decode (m_localState.GetValue (LOCAL_STATE_NAMESPACE, identifier.c_str ()).asString ());

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_set_padding (&ctx, 0);
    EVP_CIPHER_CTX_init (&ctx);

    EVP_DecryptInit_ex (&ctx, EVP_aes_256_gcm (), nullptr, s_key, s_iv);

    std::vector<unsigned char> outbuf;
    // See documentation for EVP_DecryptUpdate at the page below for explanation of
    // the required size for outbuf:
    // https://www.openssl.org/docs/crypto/EVP_EncryptInit.html
    outbuf.resize (encodedPassword.size () + EVP_CIPHER_CTX_block_size (&ctx));
    int outlen;
    if (!EVP_DecryptUpdate (&ctx, &outbuf[0], &outlen, (const unsigned char *)encodedPassword.c_str (), (int)encodedPassword.size ()))
        {
        return password;
        }

    EVP_CIPHER_CTX_cleanup (&ctx);

    password = Utf8String ((CharCP)&outbuf[0], (size_t)outlen);
#endif

#if defined (BENTLEY_WINRT)
    // Initialize the encryption process
    Platform::String^ strAlgName = SymmetricAlgorithmNames::AesCbcPkcs7;    //Do not change this or you will need to update encryption/decryption logic
    BinaryStringEncoding encoding = BinaryStringEncoding::Utf8;     // Binary encoding value
    IBuffer^ ivWinRT;                                               // Initialization vector
    CryptographicKey^ keyWinRT;                                     // Symmetric key
    SymmetricKeyAlgorithmProvider^ objAlg = SymmetricKeyAlgorithmProvider::OpenAlgorithm (strAlgName);

    // Read data
    Utf8String utf8Password = m_localState.GetValue (LOCAL_STATE_NAMESPACE, identifier.c_str ()).asString();
    if (utf8Password.empty()) // if password is empty then this represents a deleted item and needs no decryption
        return Utf8String("");

    WString bwString = WString (utf8Password.c_str (), true);
    Platform::String^ pString = ref new Platform::String (bwString.c_str ());
    IBuffer^ buffEncrypted = CryptographicBuffer::DecodeFromBase64String (pString);

    // Decrypt
    password = Decrypt (s_key, s_iv, strAlgName, buffEncrypted, ivWinRT, encoding, keyWinRT);

#endif
    return password;
    }
#endif

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
