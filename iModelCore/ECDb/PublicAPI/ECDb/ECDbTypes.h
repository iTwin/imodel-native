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

