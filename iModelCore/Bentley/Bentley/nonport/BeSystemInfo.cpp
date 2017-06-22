/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeSystemInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <windows.h>
#endif

#if defined (BENTLEY_WINRT)
    #include <VersionHelpers.h>
    using namespace Windows::Security::ExchangeActiveSyncProvisioning;
#endif

#if defined (BENTLEY_WIN32)
    #include <Wincrypt.h>
    #include <Wtsapi32.h>    
    #include <Bentley/Base64Utilities.h>
#endif

#include "../BentleyInternal.h"

#if defined (__unix__)
    #include <sys/time.h>
    #include <sys/resource.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <iostream>
    #include <fstream>
#endif

#if defined (__APPLE__)
    #include <sys/sysctl.h>
#elif defined (ANDROID) || defined (__linux)
    #undef __unused
    #include <linux/sysctl.h>
    #include <sys/sysinfo.h>
#endif

#if defined (ANDROID)
    #include <sys/system_properties.h>
#endif

#include <Logging/bentleylogging.h>
#include <Bentley/BeSystemInfo.h>

#if defined (ANDROID) && defined (NOTNOW)
static void getMemStats()
    {
        struct rusage rUsageTemp;

        getrusage (RUSAGE_SELF, &rUsageTemp);

        int    h1 = open ("/proc/self", O_RDONLY);
        printf ("after open of self h1 = %d, errno = %d\n", h1, errno);

        char    f2[256];
        sprintf (f2, "/proc/%d", getpid());
        int h2 = open (f2, O_RDONLY);
        printf ("after open of %s h1 = %d, errno = %d\n", f2, h1, errno);
    }
#endif

#if defined (ANDROID)
static Utf8String s_deviceId;
#endif
//---------------------------------------------------------------------------------------
// @bsimethod                                   Vincas.Razma                    05/15
//---------------------------------------------------------------------------------------
void BeSystemInfo::CacheAndroidDeviceId (Utf8StringCR deviceId)
    {
#if defined (ANDROID)
    s_deviceId = deviceId;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
uint64_t BeSystemInfo::GetAmountOfPhysicalMemory ()
    {
#if defined (ANDROID) || defined (__linux)
    struct sysinfo myInfo;
    sysinfo(&myInfo);
    return myInfo.totalram;
#elif defined (__APPLE__)
    int     name[2];
    size_t  sizeOutput;
    uint64_t retval = 0;

    name[0] = CTL_HW;
    name[1] = HW_PHYSMEM;
    sizeOutput = sizeof (retval);
    sysctl (name, 2, (Byte*)&retval, &sizeOutput, NULL, 0);
    BeAssert (4 == sizeOutput);

    return retval;
#elif defined (BENTLEY_WIN32)
    MEMORYSTATUSEX  memoryStatus;
    memoryStatus.dwLength = sizeof (memoryStatus);

    GlobalMemoryStatusEx (&memoryStatus);
    return memoryStatus.ullTotalPhys;
#else
    return 0;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetModelName ()
    {
#if defined (__APPLE__)
    int     name[2];
    size_t  sizeOutput;
    Byte output[64];

    name[0] = CTL_HW;
    name[1] = HW_MODEL;
    sizeOutput = sizeof (output) - 1;
    sysctl (name, 2, output, &sizeOutput, NULL, 0);
    output[sizeOutput] = 0;
    return Utf8String ((CharCP)output);
#else
    return "";
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetMachineName ()
    {
#if defined (BENTLEY_WIN32)
    return "";

#elif defined (__APPLE__)
    int     name[2];
    size_t  sizeOutput;
    Byte output[64];

    name[0] = CTL_HW;
    name[1] = HW_MACHINE;
    sizeOutput = sizeof (output) - 1;
    sysctl (name, 2, output, &sizeOutput, NULL, 0);
    output[sizeOutput] = 0;
    return Utf8String ((CharCP)output);

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
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
uint32_t BeSystemInfo::GetNumberOfCpus()
    {
#if defined (__APPLE__)
    int     name [2];
    size_t  sizeOutput;
    Byte output[64];

    name[0] = CTL_HW;
    name[1] = HW_NCPU;
    sizeOutput = sizeof (output);
    sysctl(name, 2, output, &sizeOutput, NULL, 0);

    uint32_t retval = 0;
    for (unsigned i = 0; i < sizeOutput; ++i)
        retval += output[i] << (8 * i);

    return retval;

#elif defined (ANDROID) || defined (__linux)
    return sysconf (_SC_NPROCESSORS_ONLN);

#elif defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo (&systemInfo);

    return systemInfo.dwNumberOfProcessors;

#else
    return 0;

#endif
    }

// iOS implementations are located in BeSystemInfo.mm file
#if !defined (__APPLE__)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Grigas.Petraitis                12/14
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetOSName ()
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
Utf8String BeSystemInfo::GetOSVersion ()
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
    return ""; // now way to get the version?..

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
            BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->errorv ("CryptAcquireContext failed with error code %d", ::GetLastError ());
            return ERROR;
            }
        }

    HCRYPTHASH hHash;
    if (FALSE == ::CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->errorv ("CryptCreateHash failed with error code %d", ::GetLastError ());
        return ERROR;
        }

    if (FALSE == ::CryptHashData(hHash, (BYTE const*)unhashedValue.data(), (DWORD)unhashedValue.length(), 0))
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->errorv ("CryptHashData failed with error code %d", ::GetLastError ());
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
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->errorv ("No host name when looking for SID");
        return "";
        }

    Utf8String computerName;
    hostName.GetNextToken(computerName, ".", 0);
    if (computerName.empty())
        computerName = hostName;

    BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->infov ("Getting Host SID for full host '%s'", hostName.c_str());
    BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->infov ("Getting Host SID for reduced host '%s'", computerName.c_str());
    
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

    if (TRUE == ::GetComputerNameExW (ComputerNamePhysicalDnsFullyQualified, (WCharP)computerName.data(), &nameLength))
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->infov (L"GetHostName: GetComputerName: '%ls'", computerName.c_str());
        }
    else
        {
        BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->errorv (L"Unable to get host name via GetComputerName (GLE=%d)", GetLastError());
        }

    if (computerName.empty())
        {
        WCharCP computerNameP = NULL;
        if (NULL != (computerNameP = ::_wgetenv(L"COMPUTERNAME")))
            {
            computerName.assign(computerNameP);
            BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->infov (L"GetHostName: from COMPUTERNAME: '%ls'", computerName.c_str());
            }
        else
            {
            computerName.assign(L"UnknownHostName");
            BentleyApi::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->errorv (L"Unknown hostname in GetHostName");
            }
        }
    return Utf8String(computerName.c_str());
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                   Vincas.Razma                    05/15
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetDeviceId ()
    {
#if defined (BENTLEY_WINRT)
    // https://msdn.microsoft.com/en-us/jj553431
    Windows::System::Profile::HardwareToken^ token = Windows::System::Profile::HardwareIdentification::GetPackageSpecificToken (nullptr);
    Windows::Storage::Streams::IBuffer^ ashwid = token->Id;
    Windows::Storage::Streams::DataReader^ dataReader = Windows::Storage::Streams::DataReader::FromBuffer (ashwid);
    Utf8String id;
    char byteAsString[8];

    for (size_t iByte = 0; iByte < ashwid->Length; ++iByte)
        {
        unsigned char b = dataReader->ReadByte ();
        if (0 != _itoa_s ((int)b, byteAsString, _countof (byteAsString), 10))
            {
            BeAssert (false); continue;
            }

        if (id.size () > 0)
            id += "-";

        id += byteAsString;
        }

    BeAssert (id.size () > 0);

    return id;
#elif defined (BENTLEY_WIN32)
    return GetHostSID (GetHostName ());

#elif defined (ANDROID)
    if (s_deviceId.size () > 0)
        return s_deviceId;

    BeAssert (false && "For android \"BeSystemInfo::CacheAndroidDeviceId (Utf8StringCR deviceId)\" must be called before requesting for DeviceId.");
    return "";
#else
    BeAssert (false);
    return "";
#endif
    }

#endif
