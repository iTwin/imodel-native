/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjectsAPI.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//! @cond NODOC

#include <hash_map>

#ifndef BENTLEY_EXCLUDE_WINDOWS_HEADERS
#include <msxml6\msxml6.tlh>
#endif
/*__PUBLISH_SECTION_START__*/

/*=================================================================================**//**
* @mainpage ECObjects Public API Documentation
*
* This file describes the public API for writing applications using the ECObjects library.
*
* \section Namespaces
* All of the ECObjects APIs are contained within the Bentley::EC namespace. 
*
* \section headerFiles Header Files
* By far the most convenient and efficient way of accessing the classes in the ECObjects API is to use:

\code
    #include <ECObjectsAPI.h>
\endcode

* in each of your source files. This will include \e all of the individual header files in the API.
* @note If you have more than one source file linked into a single .dll, you should <b>probably use precompiled
* header files</b> (see Microsoft Visual C++ documentation for details.)
*
* \section ECObjectsApiOverview Overview
*
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/

#include <string>
#include <vector>
#include <map>
#include <limits>

#include <ECObjects\ECEnabler.h>
#include <ECObjects\ECInstance.h>
#include <ECObjects\ECSchema.h>
#include <ECObjects\ECValue.h>
#include <ECObjects\MemoryInstanceSupport.h>
#include <ECObjects\StandaloneInstance.h>

/*__PUBLISH_SECTION_END__*/
#include <ECObjects\DesignByContract.h>

// This define is checked from the ECObjects Published ATPs to ensure that they are building against the published header files.
#define NON_PUBLISHED_HEADER_INCLUDED

//! @endcond NODOC
