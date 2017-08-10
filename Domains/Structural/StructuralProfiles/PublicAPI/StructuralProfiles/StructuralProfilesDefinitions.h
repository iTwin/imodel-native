/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralProfiles/PublicAPI/StructuralProfiles/StructuralProfilesDefinitions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <StructuralDomain\StructuralCommon\StructuralCommonDefinitions.h>

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME                        "StructuralProfiles"
#define BENTLEY_STRUCTURAL_PROFILES_SCHEMA_PATH                        L"ECSchemas/Domain/StructuralProfiles.ecschema.xml"
#define BENTLEY_STRUCTURAL_PROFILES_SCHEMA(className)                  BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME "." className
#define BENTLEY_STRUCTURAL_PROFILES_SCHEMA_CODE                        BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME
#define BENTLEY_STRUCTURAL_PROFILES_AUTHORITY                          BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------
#define STRUCTURAL_PROFILES_CLASS_Profile "Profile"
#define STRUCTURAL_PROFILES_CLASS_ConstantProfile "ConstantProfile"
#define STRUCTURAL_PROFILES_CLASS_BuiltUpProfile "BuiltUpProfile"
#define STRUCTURAL_PROFILES_CLASS_BuiltUpProfileComponent "BuiltUpProfileComponent"
#define STRUCTURAL_PROFILES_CLASS_ParametricProfile "ParametricProfile"
#define STRUCTURAL_PROFILES_CLASS_VaryingProfileZone "VaryingProfileZone"
#define STRUCTURAL_PROFILES_CLASS_PublishedProfile "PublishedProfile"
#define STRUCTURAL_PROFILES_CLASS_VaryingProfile "VaryingProfile"
#define STRUCTURAL_PROFILES_CLASS_VaryingProfileByZone "VaryingProfileByZone"

//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_##__name__)); }

//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(__name__) \
    STRUCTURAL_DOMAIN_EXPORT static __name__##CPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    STRUCTURAL_DOMAIN_EXPORT static __name__##Ptr GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------

STRUCTURAL_POINTER_TYPEDEFS(Profile)
STRUCTURAL_POINTER_TYPEDEFS(ConstantProfile)
STRUCTURAL_POINTER_TYPEDEFS(BuiltUpProfile)
STRUCTURAL_POINTER_TYPEDEFS(BuiltUpProfileComponent)
STRUCTURAL_POINTER_TYPEDEFS(ParametricProfile)
STRUCTURAL_POINTER_TYPEDEFS(VaryingProfileZone)
STRUCTURAL_POINTER_TYPEDEFS(PublishedProfile)
STRUCTURAL_POINTER_TYPEDEFS(VaryingProfile)
STRUCTURAL_POINTER_TYPEDEFS(VaryingProfileByZone)
