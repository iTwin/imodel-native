/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDbTypes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

#define ECINSTANCE_ID_CLASS(classname) BEREPOSITORYBASED_ID_SUBCLASS(classname,BeSQLite::EC::ECInstanceId)

//Published types
ECDB_TYPEDEFS(ECDb);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! The status codes returned by ECSqlStatement
//! @ingroup ECDbGroup
// @bsienum                                                      Krischan.Eberle   07/2013
//+===============+===============+===============+===============+===============+======
enum class ECSqlStatus
    {
    Success = SUCCESS, //!< Success
    UserError, //!< Unspecified user error

    IndexOutOfBounds, //!< Index out of bounds in a call to the binding API or the get value API
    ConstraintViolation, //!< Executing an ECSQL failed because of a constraint violation in SQLite.
    InvalidECSql, //!< An invalid ECSQL (e.g. because of a syntax or semantic error in the ECSQL text) was passed to ECSqlStatement::Prepare.
    NotYetSupported, //!< Results from a call to a method / operation which is not yet supported / implemented.
    ProgrammerError //!< Indicates an internal error / bug in the ECSqlStatement API.
    };

//=======================================================================================
//! Options for how to specify the ECSchema when calling ECDbSchemaManager::GetECClass
//! @ingroup ECDbGroup
// @bsiclass                                                Muhammad.zaighum      10/2014
//+===============+===============+===============+===============+===============+======
enum class ResolveSchema
    {
    BySchemaName, //!< ECClass is qualified by schema name
    BySchemaNamespacePrefix, //!< ECClass is qualified by schema namespace prefix
    AutoDetect//!< Detect automatically whether ECClass is qualiried by schema name or namespace prefix
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

