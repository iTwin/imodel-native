/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/ECDb/ECDbTypes.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
    BEGIN_BENTLEY_SQLITE_EC_NAMESPACE \
        struct _name_; \
        typedef _name_*          _name_##P;  \
        typedef _name_&          _name_##R;  \
        typedef _name_ const*    _name_##CP; \
        typedef _name_ const&    _name_##CR; \
    END_BENTLEY_SQLITE_EC_NAMESPACE

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

//Published types
ECDB_TYPEDEFS(ECDb);

#if !defined (DOCUMENTATION_GENERATOR)
//Internal types which are referenced by private sections of the public API.
ECDB_TYPEDEFS_PTR(PropertyMap);
ECDB_TYPEDEFS_PTR(DbColumn);
#endif


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
    InvalidECSql, //!< An invalid ECSQL (e.g. because of a syntax or semantic error in the ECSQL text) was passed to ECSqlStatement::Prepare.
    NotYetSupported, //!< Results from a call to a method / operation which is not yet supported / implemented.
    ProgrammerError //!< Indicates an internal error / bug in the ECSqlStatement API.
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

//__PUBLISH_SECTION_END__

ECDB_TYPEDEFS(ECDbMap);
ECDB_TYPEDEFS(BeRepositoryBasedIdSequence);

ECDB_TYPEDEFS_PTR(ClassMap);
ECDB_TYPEDEFS_PTR(InstanceInserter);
ECDB_TYPEDEFS_PTR(InstanceDeleter);
ECDB_TYPEDEFS_PTR(InstanceUpdater);
ECDB_TYPEDEFS_PTR(ECDbSchemaWriter);
ECDB_TYPEDEFS_PTR(ECDbSchemaReader);

ECDB_TYPEDEFS_PTR(CustomAttributeTracker);

#define USING_NAMESPACE_BENTLEY_EC using namespace BentleyApi::ECN;
