/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjectsAPI.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//#define EC_TRACE_MEMORY
// This file describes the public API for writing applications using the ECObjects library.
/*__PUBLISH_SECTION_START__*/

/*=================================================================================**//**
@if ECOBJECTS_MAINPAGE
    @mainpage ECObjects Public API Documentation
@endif
\image html BentleyLOGO_4C_no-tag.gif

@section Namespaces
All of the ECObjects APIs are contained within the BentleyApi::ECN namespace. 

@section headerFiles Header Files
By far the most convenient and efficient way of accessing the classes in the ECObjects API is to use:

@code
    #include <ECObjects/ECObjectsAPI.h>
@endcode

in each of your source files. This will include \e all of the individual header files in the API.
@note If you have more than one source file linked into a single .dll, you should <b>probably use precompiled
header files</b> (see Microsoft Visual C++ documentation for details.)

@section ECObjectsApiOverview Overview
ECObjects is a native implementation of the .NET Bentley.ECObjects namespace.  It provides for a
subset of classes from the managed API, including ECSchemas, ECClasses, and ECInstances.

@section Logging
ECObjects makes use of the BSI logger for logging warnings and errors in the code, as well
as debug information.  It is incumbent upon the calling application to configure a logger to
capture this information.

* @bsiclass
+===============+===============+===============+===============+===============+======*/

#include <string>
#include <vector>
#include <map>
#include <limits>

#include <ECObjects/ECEnabler.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECUnit.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECName.h>
#include <ECObjects/SchemaResourceKeyHelper.h>
#include <ECObjects/ECSchemaConverter.h>
#include <ECObjects/ECSchemaDownConverter.h>
#include <ECObjects/ECSchemaValidator.h>
#include <ECObjects/SupplementalSchema.h>
#include <ECObjects/ECContext.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECExpressions.h>
#include <ECObjects/ECDBuffer.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/StandaloneECRelationshipInstance.h>
#include <ECObjects/PresentationMetadataHelper.h>
#include <ECObjects/StandardCustomAttributeHelper.h>
#include <Bentley/ScopedArray.h>
#include <ECObjects/AdHocJsonValue.h>
#include <ECObjects/ECJsonUtilities.h>
#include <ECObjects/ECRelationshipPath.h>

#include <ECObjects/DesignByContract.h>
