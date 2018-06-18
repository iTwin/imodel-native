/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/ECObjects/StandardUnitsHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/NonCopyableClass.h>
#include <Bentley/DateTime.h>
#include <ECObjects/ECObjects.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
//! Allows access to the standard Units schema. Caches it to prevent reparsing.
//!
//! The schema held by the helper should never be added to a schema as a reference, in 
//! that case always locate through the schema through the same context as the schema that
//! needs the reference.
//! @bsiclass
//=======================================================================================    
struct StandardUnitsHelper final
    {
private:
    //static class
    StandardUnitsHelper();
    ~StandardUnitsHelper() {}

public:
    //! Retrieves a Unit from the standard units schema.
    //! @param[in]  unitName Name of the unit to be retrieved
    //! @return A const pointer to an ECUnit if the named unit exists in the standard Units schema; otherwise, nullptr.
    ECOBJECTS_EXPORT static ECUnitCP GetUnit(Utf8CP unitName);
    //! Retrieves the held schema
    //! @return The held schema
    ECOBJECTS_EXPORT static ECSchemaPtr GetSchema();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
