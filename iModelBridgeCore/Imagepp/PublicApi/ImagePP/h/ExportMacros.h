//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/ExportMacros.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <Bentley/Bentley.h>

#   if defined (__IMAGEPP_BUILD__)
#       define IMAGEPP_EXPORT EXPORT_ATTRIBUTE
#       define IMAGEPPTEST_EXPORT EXPORT_ATTRIBUTE
#   else
#       define IMAGEPP_EXPORT IMPORT_ATTRIBUTE
#       ifdef ENABLE_IMAGEPPTEST_EXPORT
#           define IMAGEPPTEST_EXPORT IMPORT_ATTRIBUTE
#       else
#           define IMAGEPPTEST_EXPORT
#       endif
#   endif

// IPP client that wish to use the static lib version need to define IPP_USING_STATIC_LIBRARIES.
#if defined(CREATE_STATIC_LIBRARIES) || defined(IPP_USING_STATIC_LIBRARIES)
    #undef IMAGEPP_EXPORT
    #define IMAGEPP_EXPORT

    #undef IMAGEPPTEST_EXPORT
    #define IMAGEPPTEST_EXPORT
#endif




