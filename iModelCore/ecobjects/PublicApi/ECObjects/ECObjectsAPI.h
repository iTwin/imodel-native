/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjectsAPI.h $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
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
WIP
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/

#include <string>
#include <vector>

#include <ECObjects\ECEnabler.h>
#include <ECObjects\ECInstance.h>
#include <ECObjects\ECSchema.h>
#include <ECObjects\ECValue.h>