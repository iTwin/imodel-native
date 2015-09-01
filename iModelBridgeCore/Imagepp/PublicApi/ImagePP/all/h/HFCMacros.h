//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMacros.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMacros
//-----------------------------------------------------------------------------
#pragma once

// MultiPlatform Note
//
// Don't remove the IPP in IPP##pi_ClassName##Destroyer, it is necessary with GCC.

//-----------------------------------------------------------------------------
// Declare the singleton
//      - static member to store the instance
//      - static function to obtain the instance
//      - friend the destoryer
//-----------------------------------------------------------------------------
#define HFC_DECLARE_SINGLETON_DLL(HDLL, pi_ClassName) \
    public: \
        HDLL static pi_ClassName* GetInstance(); \
        HDLL static bool         IsCreated(); \
    private: \
        struct IPP##pi_ClassName##Destroyer; \
        static pi_ClassName*    s_pInstance;

#define HFC_DECLARE_SINGLETON(pi_ClassName) HFC_DECLARE_SINGLETON_DLL(, pi_ClassName)


//-----------------------------------------------------------------------------
// Implement the singleton
//      - static member initialization
//      - static function to obtain the instance
//      - The destroyer that frees the instance.
//-----------------------------------------------------------------------------
#define HFC_IMPLEMENT_SINGLETON(pi_ClassName) \
pi_ClassName*   pi_ClassName::s_pInstance = 0; \
bool pi_ClassName::IsCreated() \
{ \
    return (s_pInstance != 0); \
} \
pi_ClassName* pi_ClassName::GetInstance() \
{ \
    if (s_pInstance == 0) \
        s_pInstance = new pi_ClassName(); \
    return s_pInstance; \
} \
static struct pi_ClassName::IPP##pi_ClassName##Destroyer \
{ \
    ~IPP##pi_ClassName##Destroyer() \
    { \
        if (pi_ClassName::s_pInstance) \
        { \
            delete pi_ClassName::s_pInstance; \
            pi_ClassName::s_pInstance = 0; \
        } \
    } \
} s_IPP##pi_ClassName##Destroyer; \



//-----------------------------------------------------------------------------
// Declare a singleton with life cycle control by an Host
// Class must derive from HostObjectBase
//      - static member to store the instance
//      - static function to obtain the instance
//-----------------------------------------------------------------------------
#define HFC_DECLARE_HOSTOBJECT_SINGLETON_DLL(HDLL, pi_HostNamespace, pi_ClassName) \
    public: \
    HDLL static pi_ClassName* GetInstance(); \
    HDLL static bool         IsCreated(); \
    private: \
    static pi_HostNamespace::Host::Key s_##pi_ClassName##Key;

#define HFC_DECLARE_HOSTOBJECT_SINGLETON(pi_HostNamespace, pi_ClassName) HFC_DECLARE_HOSTOBJECT_SINGLETON_DLL(, pi_HostNamespace, pi_ClassName)


//-----------------------------------------------------------------------------
// Implement the singleton
//      - static member initialization
//      - static function to obtain the instance
//-----------------------------------------------------------------------------
#define HFC_IMPLEMENT_HOSTOBJECT_SINGLETON(pi_HostNamespace, pi_ClassName) \
    pi_HostNamespace::Host::Key   pi_ClassName::s_##pi_ClassName##Key; \
    bool pi_ClassName::IsCreated() \
{ \
    return (pi_HostNamespace::GetHost().GetHostObject(s_##pi_ClassName##Key) != 0); \
} \
    pi_ClassName* pi_ClassName::GetInstance() \
{ \
    pi_ClassName*    pInstance = (pi_ClassName*)BentleyApi::ImagePP::ImageppLib::GetHost().GetHostObject(s_##pi_ClassName##Key); \
    if (pInstance == 0) \
        pi_HostNamespace::GetHost().SetHostObject(s_##pi_ClassName##Key,pInstance = new pi_ClassName()); \
    return pInstance; \
} 


