/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDbTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <BeSQLite/BeSQLite.h>
#include <ECObjects/ECObjectsAPI.h>

#if defined (__ECDB_BUILD__)
    #define ECDB_EXPORT EXPORT_ATTRIBUTE
#else
    #define ECDB_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_SQLITE_EC_NAMESPACE   BEGIN_BENTLEY_SQLITE_NAMESPACE namespace EC {
#define END_BENTLEY_SQLITE_EC_NAMESPACE     } END_BENTLEY_SQLITE_NAMESPACE
#define USING_NAMESPACE_BENTLEY_SQLITE_EC   using namespace BentleyApi::BeSQLite::EC;

#define ECDB_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_SQLITE_EC_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_SQLITE_EC_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)
#define ECDB_TYPEDEFS_PTR(_name_)   \
    ECDB_TYPEDEFS(_name_)           \
    BEGIN_BENTLEY_SQLITE_EC_NAMESPACE    \
    typedef RefCountedPtr<_name_>         _name_##Ptr;  \
    END_BENTLEY_SQLITE_EC_NAMESPACE

#define ECDB_TYPEDEFS2(_type_,_name_) \
    BEGIN_BENTLEY_SQLITE_EC_NAMESPACE \
        typedef _type_  _name_; \
        typedef _name_*          _name_##P;  \
        typedef _name_&          _name_##R;  \
        typedef _name_ const*    _name_##CP; \
        typedef _name_ const&    _name_##CR; \
    END_BENTLEY_SQLITE_EC_NAMESPACE

#endif

#define ECINSTANCE_ID_CLASS(classname) BEBRIEFCASEBASED_ID_SUBCLASS(classname,BeSQLite::EC::ECInstanceId)

//Published types
ECDB_TYPEDEFS(ECDb);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Options for how to specify the ECSchema when calling ECDbSchemaManager::GetECClass
//! @ingroup ECDbGroup
// @bsiclass                                                Muhammad.zaighum      10/2014
//+===============+===============+===============+===============+===============+======
enum class ResolveSchema
    {
    BySchemaName, //!< ECClass is qualified by schema name
    BySchemaAlias, //!< ECClass is qualified by schema alias
    AutoDetect//!< Detect automatically whether ECClass is qualified by schema name or alias
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

