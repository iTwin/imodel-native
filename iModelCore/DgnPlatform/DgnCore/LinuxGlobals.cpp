/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/LinuxGlobals.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// The following works around a problem getting the IID_xxx globals defined in a Linux build.
// In a Windows/MSVC build, these globals are somehow available (don't know how) without this work-around.
#if! defined (__unix__)
    #error this file is intended for the Linux build only
#endif



