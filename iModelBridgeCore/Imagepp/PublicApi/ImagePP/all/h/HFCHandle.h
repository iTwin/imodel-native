//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHandle.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCHandle
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE

#if defined (ANDROID) || defined (__APPLE__)
typedef void*  HFCHandle;       //DM-Android  Need to verify HFCSynchro.h,hpp
#elif _WIN32
typedef HANDLE  HFCHandle;
#endif

END_IMAGEPP_NAMESPACE

