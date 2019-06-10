/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#ifndef _formsdomaindefinitions_included_
#define _formsdomaindefinitions_included_

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <BeSQLite\BeSQLite.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>


#ifdef __FORMS_DOMAIN_BUILD__
    #define FORMS_DOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
    #define FORMS_DOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif


#define BEGIN_BENTLEY_FORMS_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Forms {
#define END_BENTLEY_FORMS_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_FORMS using namespace BentleyApi::Forms;

#define FORMS_POINTER_SUFFIX_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_FORMS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_FORMS_NAMESPACE

#define FORMS_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_FORMS_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_FORMS_NAMESPACE

#define FORMS_POINTER_TYPEDEFS(_structname_) \
    FORMS_POINTER_SUFFIX_TYPEDEFS(_structname_) \
    FORMS_REFCOUNTED_TYPEDEFS(_structname_)

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_FORMS_SCHEMA_NAME                        "Forms"
#define BENTLEY_FORMS_SCHEMA_PATH                        L"ECSchemas/Domain/Forms.ecschema.xml"
#define BENTLEY_FORMS_SCHEMA(className)                  BENTLEY_FORMS_SCHEMA_NAME "." className
#define BENTLEY_FORMS_SCHEMA_CODE                        BENTLEY_FORMS_SCHEMA_NAME
#define BENTLEY_FORMS_AUTHORITY                          BENTLEY_FORMS_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define FORMS_CLASS_FormAspect                              "Form"
#define FORMS_CLASS_ProfiledExtrusion                       "ProfiledExtrusion"
#define FORMS_CLASS_CurvedExtrusion                         "CurvedExtrusion"
#define FORMS_CLASS_StraightExtrusion                       "StraightExtrusion"
#define FORMS_CLASS_CurvedProfiledExtrusion                 "CurvedProfiledExtrusion"
#define FORMS_CLASS_StraightProfiledExtrusion               "StraightProfiledExtrusion"

#define FORMS_CLASS_StraightProfiledExtrusionHasProfile     "StraightProfiledExtrusionHasProfile"
#define FORMS_CLASS_CurvedProfiledExtrusionHasStartProfile  "CurvedProfiledExtrusionHasStartProfile"
#define FORMS_CLASS_CurvedProfiledExtrusionHasEndProfile    "CurvedProfiledExtrusionHasEndProfile"

BEGIN_BENTLEY_FORMS_NAMESPACE

BEBRIEFCASEBASED_ID_SUBCLASS(ProfileId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(StartProfileId, Dgn::DgnElementId)
BEBRIEFCASEBASED_ID_SUBCLASS(EndProfileId, Dgn::DgnElementId)

typedef BeSQLite::IdSet<ProfileId> ProfileIdSet;               //!< IdSet with ClassificationId members.
typedef BeSQLite::IdSet<StartProfileId> StartProfileIdSet;     //!< IdSet with StartProfileId members.
typedef BeSQLite::IdSet<EndProfileId> EndProfileIdSet;         //!< IdSet with EndProfileId members.

END_BENTLEY_FORMS_NAMESPACE

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
FORMS_POINTER_TYPEDEFS(Form)
FORMS_POINTER_TYPEDEFS(ProfiledExtrusion)
FORMS_POINTER_TYPEDEFS(CurvedExtrusion)
FORMS_POINTER_TYPEDEFS(StraightExtrusion)
FORMS_POINTER_TYPEDEFS(CurvedProfiledExtrusion)
FORMS_POINTER_TYPEDEFS(StraightProfiledExtrusion)


#endif