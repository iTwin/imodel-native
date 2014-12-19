//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCMSCrypto.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation for class HFCMSCrypto
//-----------------------------------------------------------------------------
#pragma once

//######################
// INCLUDE FILES
//######################

// Check for win32 platform
#if defined(_WIN32) || defined(WIN32)

class HFCMSCrypto
    {
public:
    // Construction - destruction
    HFCMSCrypto();

    virtual
    ~HFCMSCrypto();

    // Initialization
    bool
    Initialize(const WString& pi_rContainerName,
               const WString& pi_rCryptPwd);

    // Encryption - decryption
    Byte*
    Encrypt(Byte* pi_pSrcData,
            uint32_t* pio_DataLen,
            BOOL pi_IncludeSpan = false);

    Byte*
    Decrypt(Byte* pi_pSrcData,
            uint32_t* pio_DataLen,
            BOOL pi_IncludeSpan = false);

protected:

private:

    // Not implemented
    HFCMSCrypto(const HFCMSCrypto&);
    HFCMSCrypto& operator=(const HFCMSCrypto&);

    Byte*
    GenerateSpan();

    // Attributes
    bool    m_IsOk;
    WString  m_ContainerName;
    WString    m_CryptPwd;

    HCRYPTPROV    m_hCryptProv;
    HCRYPTKEY    m_hKey;
    };

#else

#error Not supported on this platform

#endif // Win32
