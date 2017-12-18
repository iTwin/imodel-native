#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>


#ifdef __PROFILES_DOMAIN_BUILD__
#define PROFILES_DOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
#define PROFILES_DOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif


#define BEGIN_BENTLEY_PROFILES_NAMESPACE namespace Profiles {
#define END_BENTLEY_PROFILES_NAMESPACE } 
#define USING_NAMESPACE_BENTLEY_PROFILES using namespace BentleyApi::Profiles;

#define PROFILES_POINTER_SUFFIX_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_PROFILES_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_PROFILES_NAMESPACE

#define PROFILES_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_PROFILES_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_PROFILES_NAMESPACE

#define PROFILES_POINTER_TYPEDEFS(_structname_) \
    PROFILES_POINTER_SUFFIX_TYPEDEFS(_structname_) \
    PROFILES_REFCOUNTED_TYPEDEFS(_structname_)

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_PROFILES_SCHEMA_NAME                        "Profiles"
#define BENTLEY_PROFILES_SCHEMA_PATH                        L"ECSchemas/Domain/Profiles.ecschema.xml"
#define BENTLEY_PROFILES_SCHEMA(className)                  BENTLEY_PROFILES_SCHEMA_NAME "." className
#define BENTLEY_PROFILES_SCHEMA_CODE                        BENTLEY_PROFILES_SCHEMA_NAME
#define BENTLEY_PROFILES_AUTHORITY                          BENTLEY_PROFILES_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
// #define PROFILES_COMMON_CLASS_Profiles                    "Profiles"
#define PROFILES_ProfilesPartition                        "ProfilesPartition"
#define PROFILES_ProfilesModel                            "ProfilesModel"

//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------
#define PROFILES_CLASS_Profile "Profile"
#define PROFILES_CLASS_ConstantProfile "ConstantProfile"
#define PROFILES_CLASS_BuiltUpProfile "BuiltUpProfile"
#define PROFILES_CLASS_BuiltUpProfileComponent "BuiltUpProfileComponent"
#define PROFILES_CLASS_ParametricProfile "ParametricProfile"
#define PROFILES_CLASS_PublishedProfile "PublishedProfile"

#define BE_ECCLASS_NAME(__val__)   static constexpr Utf8CP ecclass_name_##__val__() {return #__val__;}

//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_PROFILES_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BENTLEY_PROFILES_SCHEMA_NAME, PROFILES_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BENTLEY_PROFILES_SCHEMA_NAME, PROFILES_CLASS_##__name__)); }

//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(__name__) \
    PROFILES_DOMAIN_EXPORT static __name__##CPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    PROFILES_DOMAIN_EXPORT static __name__##Ptr GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
PROFILES_POINTER_TYPEDEFS(ProfilesPartition)
PROFILES_POINTER_TYPEDEFS(ProfilesModel)
PROFILES_POINTER_TYPEDEFS(Profile)
PROFILES_POINTER_TYPEDEFS(ConstantProfile)
PROFILES_POINTER_TYPEDEFS(BuiltUpProfile)
PROFILES_POINTER_TYPEDEFS(BuiltUpProfileComponenet)
PROFILES_POINTER_TYPEDEFS(ParametricProfile)
PROFILES_POINTER_TYPEDEFS(PublishedProfile)

