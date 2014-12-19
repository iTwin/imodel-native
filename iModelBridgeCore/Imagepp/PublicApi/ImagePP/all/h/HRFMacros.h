//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFMacros.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFMacros
//-----------------------------------------------------------------------------
// define the raster file macros
//-----------------------------------------------------------------------------
#pragma once

#define HOST_REGISTER_FILEFORMAT(pi_ClassName) \
    HRFRasterFileFactory::GetInstance()->RegisterCreator(pi_ClassName::GetInstance()); 

//-----------------------------------------------------------------------------
#define HRF_REGISTER_FILEFORMAT(pi_ClassName) \
static struct pi_ClassName##CreatorRegister \
{ \
    pi_ClassName##CreatorRegister() \
    { \
        HRFRasterFileFactory::GetInstance()->RegisterCreator(pi_ClassName::GetInstance()); \
    } \
} g_##pi_ClassName##CreatorRegister;

#define HRF_REGISTER_DLL_DIRECTORY(pi_ClassID, pi_DllDir) \
    static struct struct_##pi_ClassID##DllDir \
{ \
    struct_##pi_ClassID##DllDir() \
    { \
    HRFRasterFileFactory::GetInstance()->SetRasterDllDirectory(pi_ClassID, pi_DllDir); \
    } \
} g_##pi_ClassID##DllDir;
