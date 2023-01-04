/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>

PUSH_CLANG_IGNORE(unknown-warning-option)
PUSH_CLANG_IGNORE(implicit-const-int-float-conversion)
#include <folly/futures/Future.h>
#include <folly/Executor.h>
#include <folly/Unit.h>
POP_CLANG_IGNORE
POP_CLANG_IGNORE

#include <ECDb/ECDbApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <BeRapidJson/BeRapidJson.h>

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
// Define the ECPresentation logger namespaces
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_ECPRESENTATION                     "ECPresentation"
#define LOGGER_NAMESPACE_ECPRESENTATION_PERFORMANCE         LOGGER_NAMESPACE_ECPRESENTATION ".Performance"
#define LOGGER_NAMESPACE_ECPRESENTATION_CONNECTIONS         LOGGER_NAMESPACE_ECPRESENTATION ".Connections"
#define LOGGER_NAMESPACE_ECPRESENTATION_TASKS               LOGGER_NAMESPACE_ECPRESENTATION ".Tasks"
#define LOGGER_NAMESPACE_ECPRESENTATION_HIERARCHIES         LOGGER_NAMESPACE_ECPRESENTATION ".Navigation"
#define LOGGER_NAMESPACE_ECPRESENTATION_HIERARCHIES_CACHE   LOGGER_NAMESPACE_ECPRESENTATION_HIERARCHIES ".Cache"
#define LOGGER_NAMESPACE_ECPRESENTATION_CONTENT             LOGGER_NAMESPACE_ECPRESENTATION ".Content"
#define LOGGER_NAMESPACE_ECPRESENTATION_UPDATE              LOGGER_NAMESPACE_ECPRESENTATION ".Update"
#define LOGGER_NAMESPACE_ECPRESENTATION_UPDATE_HIERARCHIES  LOGGER_NAMESPACE_ECPRESENTATION_UPDATE_HIERARCHIES ".Hierarchies"
#define LOGGER_NAMESPACE_ECPRESENTATION_UPDATE_CONTENT      LOGGER_NAMESPACE_ECPRESENTATION_UPDATE_CONTENT ".Content"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULES               LOGGER_NAMESPACE_ECPRESENTATION ".Rules"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESET_VARIABLES   LOGGER_NAMESPACE_ECPRESENTATION ".RulesetVariables"
#define LOGGER_NAMESPACE_ECPRESENTATION_ECEXPRESSIONS       LOGGER_NAMESPACE_ECPRESENTATION ".ECExpressions"
#define LOGGER_NAMESPACE_ECPRESENTATION_SERIALIZATION       LOGGER_NAMESPACE_ECPRESENTATION ".Serialization"

//-----------------------------------------------------------------------------------------
// Use the ECPresentation namespace
//-----------------------------------------------------------------------------------------
BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

END_BENTLEY_ECPRESENTATION_NAMESPACE

#define PRESENTATION_RULESET_SCHEMA_NAME        "PresentationRules"
#define PRESENTATION_RULESET_ELEMENT_CLASS_NAME "Ruleset"
#define PRESENTATION_RULESET_FULL_CLASS_NAME    PRESENTATION_RULESET_SCHEMA_NAME "." PRESENTATION_RULESET_ELEMENT_CLASS_NAME

#define ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
