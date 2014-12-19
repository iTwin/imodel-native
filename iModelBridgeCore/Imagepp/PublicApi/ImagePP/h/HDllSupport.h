//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HDllSupport.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

// DLL support
//
// _HDLLh   --> \imagepp\h
// _HDLLu   --> UtlLibs
// _HDLLw   --> WinLibs
// _HDLLg   --> GraLibs
// _HDLLn   --> NetLibs

// See in hstdcpp.h for the default value of ... if _HDLL_SUPPORT is not defined.
//    #define _HDLLNone
//    #define _HDLLu
//    #define _HDLLw
//    #define _HDLLg
//    #define _HDLLn
//    #define _HDLLh
//
// _HDLL_SUPPORT            --> Create or use I++ DLLs instead of static libs
//
#if defined (_HDLL_SUPPORT)
#   if defined (_HDLL_EXPORTu)
#       define _HDLLu EXPORT_ATTRIBUTE
#   else
#       define _HDLLu IMPORT_ATTRIBUTE
#   endif
#   if defined (_HDLL_EXPORTw)
#       define _HDLLw EXPORT_ATTRIBUTE
#   else
#       define _HDLLw IMPORT_ATTRIBUTE
#   endif
#   if defined (_HDLL_EXPORTg)
#       define _HDLLg EXPORT_ATTRIBUTE
#   else
#       define _HDLLg IMPORT_ATTRIBUTE
#   endif
#   if defined (_HDLL_EXPORTn)
#       define _HDLLn EXPORT_ATTRIBUTE
#   else
#       define _HDLLn IMPORT_ATTRIBUTE
#   endif
#   if defined (__IPPIMAGING_BUILD__)
#       define IPPIMAGING_EXPORT EXPORT_ATTRIBUTE
#   else
#       define IPPIMAGING_EXPORT IMPORT_ATTRIBUTE
#   endif
#endif

