/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/ECPresentation.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <folly/futures/Future.h>
#include <folly/Executor.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECDb/ECDbApi.h>

//-----------------------------------------------------------------------------------------
// Define export/import macros
//-----------------------------------------------------------------------------------------
#ifdef __ECPRESENTATION_DLL_BUILD__
    #define ECPRESENTATION_EXPORT EXPORT_ATTRIBUTE
#else
    #define ECPRESENTATION_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// Define the ECPresentation namespace
//-----------------------------------------------------------------------------------------
#define BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace ECPresentation {
#define END_BENTLEY_ECPRESENTATION_NAMESPACE    } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ECPRESENTATION  using namespace BentleyApi::ECPresentation;

//-----------------------------------------------------------------------------------------
// Define standard typedefs in the ECPresentation namespace
//-----------------------------------------------------------------------------------------
// P, CP, R, CR
#define ECPRESENTATION_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_ECPRESENTATION_NAMESPACE
// Ptr, CPtr
#define ECPRESENTATION_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE \
        struct _name_; \
        typedef RefCountedPtr<_name_> _name_##Ptr; \
        typedef RefCountedPtr<_name_ const> _name_##CPtr; \
    END_BENTLEY_ECPRESENTATION_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define the ECPresentation logger namespace
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_ECPRESENTATION             "ECPresentation"
#define LOGGER_NAMESPACE_ECPRESENTATION_CONNECTIONS LOGGER_NAMESPACE_ECPRESENTATION ".Connections"

//-----------------------------------------------------------------------------------------
// Use the ECPresentation namespace
//-----------------------------------------------------------------------------------------
BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

END_BENTLEY_ECPRESENTATION_NAMESPACE
