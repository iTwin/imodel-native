/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/Device/Device.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "Device.h"

#if defined (BENTLEY_WINRT)
    #include <VersionHelpers.h>
    using namespace Windows::Security::ExchangeActiveSyncProvisioning;
#elif defined (BENTLEY_WIN32)
    #include <Windows.h>
    #include <Wincrypt.h>
    #include <Wtsapi32.h>    
    #include <Bentley/Base64Utilities.h>
#elif defined (ANDROID)
    #include <sys/system_properties.h>
#endif

#if defined (ANDROID)
static Utf8String s_deviceId;
//---------------------------------------------------------------------------------------
// @bsimethod                                   Vincas.Razma                    05/15
//---------------------------------------------------------------------------------------
void Device::CacheAndroidDeviceId (Utf8StringCR deviceId)
    {
    s_deviceId = deviceId;
    }
#endif

// iOS implementations are located in DgnClientFxIos.mm file
#if !defined (__APPLE__)
//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String Device::GetModelName ()
    {
#if defined (BENTLEY_WIN32)
    return "";

#elif defined (ANDROID)
    Utf8Char model[PROP_VALUE_MAX + 1];
    __system_property_get ("ro.product.model", model);
    return model;

#elif defined (BENTLEY_WINRT)
    EasClientDeviceInformation^ info = ref new  EasClientDeviceInformation ();
    Platform::String^ sku = info->SystemSku;
    return Utf8String (sku->Data ());

#else
    BeAssert (false);
    return "";
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String Device::GetOSName ()
    {
#if defined (BENTLEY_WIN32)
    return "Windows";

#elif defined (ANDROID)
    return "Android";

#elif defined (BENTLEY_WINRT)
    EasClientDeviceInformation^ info = ref new  EasClientDeviceInformation ();
    Platform::String^ os = info->OperatingSystem;
    return Utf8String (os->Data ());

#else
    BeAssert (false);
    return "";
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String Device::GetOSVersion ()
    {
#if defined (BENTLEY_WIN32)
    OSVERSIONINFO version = {0};
    version.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    ::GetVersionEx (&version);
    return Utf8PrintfString ("%d.%d", version.dwMajorVersion, version.dwMinorVersion);

#elif defined (ANDROID)
    Utf8Char model[PROP_VALUE_MAX + 1];
    __system_property_get ("ro.build.version.release", model);
    return model;

#elif defined (BENTLEY_WINRT)
    return "6.3"; // now way to get the version?..

#else
    BeAssert (false);
    return "";
#endif
    }

#if defined (BENTLEY_WIN32)

#define SHA1_HASH_SIZE 20 // size in bytes of SHA1 hash
#if !defined(DIM)
    #define DIM(a) ((sizeof(a)/sizeof((a)[0])))
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                        Grigas.Petraitis            06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt HashBytes(Utf8StringR hashedValue, Utf8StringCR unhashedValue)
    {
    HCRYPTPROV hProv;
    if (FALSE == ::CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))
        {
        if (NTE_BAD_KEYSET == GetLastError() && FALSE == ::CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
            {
            LOG.errorv("CryptAcquireContext failed with error code %d", ::GetLastError());
            return ERROR;
            }
        }

    HCRYPTHASH hHash;
    if (FALSE == ::CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
        {
        LOG.errorv("CryptCreateHash failed with error code %d", ::GetLastError());
        return ERROR;
        }

    if (FALSE == ::CryptHashData(hHash, (BYTE const*)unhashedValue.data(), (DWORD)unhashedValue.length(), 0))
        {
        LOG.errorv("CryptHashData failed with error code %d", ::GetLastError());
        return ERROR;
        }

    BYTE  encryptedBuffer[SHA1_HASH_SIZE];
    DWORD bufferLength = DIM(encryptedBuffer);
    ::CryptGetHashParam(hHash, HP_HASHVAL, encryptedBuffer, &bufferLength, 0);
    ::CryptDestroyHash(hHash);
    ::CryptReleaseContext(hProv,0);

    hashedValue.assign((Utf8CP)&encryptedBuffer[0], bufferLength);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                        Grigas.Petraitis            06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String HashString(Utf8StringCR clearString, uint32_t iterations)
    {    
    Utf8String hashedString = clearString;
    for (uint32_t iPass = 0; iPass < iterations; iPass++)
        {
        Utf8String in = hashedString;
        HashBytes(hashedString, in);
        }
    return Base64Utilities::Encode(hashedString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                        Grigas.Petraitis            06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String SidByHash(Utf8StringCR hostName)
    {
    return HashString(hostName, 17);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                        Grigas.Petraitis            06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetHostSID(Utf8StringCR hostName)
    {
    if (hostName.empty())
        {
        BeAssert(false);
        LOG.errorv("No host name when looking for SID");
        return "";
        }

    Utf8String computerName;
    hostName.GetNextToken(computerName, ".", 0);
    if (computerName.empty())
        computerName = hostName;

    LOG.infov("Getting Host SID for full host '%s'", hostName.c_str());
    LOG.infov("Getting Host SID for reduced host '%s'", computerName.c_str());
    
    computerName.ToLower();
    return SidByHash(computerName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                        Grigas.Petraitis            06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetHostName()
    {    
    DWORD nameLength = 0;
    ::GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified, NULL, &nameLength);
    
    WString computerName;
    computerName.resize(nameLength + 1);

    if (TRUE == ::GetComputerNameExW(ComputerNamePhysicalDnsFullyQualified, (WCharP)computerName.data(), &nameLength))
        {
        LOG.infov(L"GetHostName: GetComputerName: '%ls'", computerName.c_str());
        }
    else
        {
        LOG.errorv(L"Unable to get host name via GetComputerName (GLE=%d)", GetLastError());
        }

    if (computerName.empty())
        {
        WCharCP computerNameP = NULL;
        if (NULL != (computerNameP = ::_wgetenv(L"COMPUTERNAME")))
            {
            computerName.assign(computerNameP);
            LOG.infov(L"GetHostName: from COMPUTERNAME: '%ls'", computerName.c_str());
            }
        else
            {
            computerName.assign(L"UnknownHostName");
            LOG.errorv(L"Unknown hostname in GetHostName");
            }
        }
    return Utf8String(computerName.c_str());
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                   Vincas.Razma                    05/15
//---------------------------------------------------------------------------------------
Utf8String Device::GetDeviceId ()
    {
#if defined (BENTLEY_WINRT)
    // https://msdn.microsoft.com/en-us/jj553431
    Windows::System::Profile::HardwareToken^ token = Windows::System::Profile::HardwareIdentification::GetPackageSpecificToken(nullptr);
    Windows::Storage::Streams::IBuffer^ ashwid = token->Id;
    Windows::Storage::Streams::DataReader^ dataReader = Windows::Storage::Streams::DataReader::FromBuffer(ashwid);
    Utf8String id;
    char byteAsString[8];

    for (size_t iByte = 0; iByte < ashwid->Length; ++iByte)
        {
        unsigned char b = dataReader->ReadByte();
        if (0 != _itoa_s((int)b, byteAsString, _countof(byteAsString), 10))
            { BeAssert(false); continue; }
        
        if (id.size() > 0)
            id += "-";
        
        id += byteAsString;
        }

    BeAssert(id.size() > 0);

    return id;
#elif defined (BENTLEY_WIN32)
    return GetHostSID(GetHostName());

#elif defined (ANDROID)
    return s_deviceId;

#else
    BeAssert (false);
    return "";
#endif
    }

#endif
