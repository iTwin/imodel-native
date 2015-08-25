/*--------------------------------------------------------------------------------------+
|
|     $Source: PCLWrapper/PublicAPI/IDefines.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifndef DLLEXPORT
#define DLLEXPORT __declspec( dllexport )
#endif

#ifndef DLLIMPORT
#define DLLIMPORT __declspec( dllimport )
#endif

#if defined (__PCLWRAPPER_BUILD__)
#   define PCLWRAPPER_EXPORT           DLLEXPORT
#else
#   define PCLWRAPPER_EXPORT           DLLIMPORT
#endif

#define BEGIN_PCLWRAPPER_NAMESPACE                BEGIN_BENTLEY_NAMESPACE namespace PCLUtility {
#define END_PCLWRAPPER_NAMESPACE                  END_BENTLEY_NAMESPACE}
#define USING_NAMESPACE_PCLWRAPPER                using namespace Bentley::PCLUtility;
