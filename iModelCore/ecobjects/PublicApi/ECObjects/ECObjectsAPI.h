/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjectsAPI.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


//#define EC_TRACE_MEMORY

/*__PUBLISH_SECTION_START__*/

/*=================================================================================**//**
* @if ECOBJECTS_MAINPAGE
*     @mainpage ECObjects Public API Documentation
* @endif
*
* This file describes the public API for writing applications using the ECObjects library.
*
* \section Namespaces
* All of the ECObjects APIs are contained within the Bentley::EC namespace. 
*
* \section headerFiles Header Files
* By far the most convenient and efficient way of accessing the classes in the ECObjects API is to use:

\code
    #include <ECObjects/ECObjectsAPI.h>
\endcode

* in each of your source files. This will include \e all of the individual header files in the API.
* @note If you have more than one source file linked into a single .dll, you should <b>probably use precompiled
* header files</b> (see Microsoft Visual C++ documentation for details.)
*
* \section ECObjectsApiOverview Overview
*
* \section Logging
* ECObjects makes use of the BSI logger for logging warnings and errors in the code, as well
* as debug information.  If the calling application has configured a logger already, ECObjects
* will log to that logger.  Otherwise, ECObjects may register a new logger and configure it, according to 
* these rules:
*   -# If the environment variable "BENTLEY_LOGGING_CONFIG" is set, it will use the file pointed
*      to by this path as the configuration file.
*   -# It will next look in the same directory as the ECObjects.dll for a file named "logging.config.xml"
*   -# It will next look for $(OutRoot)WinX86(or 64)\Product\ECFrameworkNativeTest\Tests\logging.config.xml
*   -# As a default, it will not configure any logger.
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/

#include <string>
#include <vector>
#include <map>
#ifdef USE_HASHMAP_IN_CLASSLAYOUT   // WIP_NONPORT - No hashmap on Android
#include <hash_map>
#endif
#include <limits>

#include <ECObjects/ECEnabler.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECContext.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECExpressions.h>
#include <ECObjects/MemoryInstanceSupport.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects\StandaloneECRelationshipInstance.h>
#include <Bentley/ScopedArray.h>


/*__PUBLISH_SECTION_END__*/
#include <ECObjects/DesignByContract.h>
#include <ECObjects/BeXmlCommonGeometry.h>
// This define is checked from the ECObjects Published ATPs to ensure that they are building against the published header files.
#define NON_PUBLISHED_HEADER_INCLUDED

