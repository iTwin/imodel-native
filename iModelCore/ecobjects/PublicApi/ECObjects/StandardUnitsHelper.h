/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandardUnitsHelper.h $
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
//! @addtogroup ECObjectsGroup
//! @beginGroup

//=======================================================================================    
//! Allows access to the standard Units schema. Caches it to prevent reparsing
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
    //! Retrieves an InvertedUnit from the standard units schema. 
    //! @param[in]  unitName Name of the inverted unit to be retrieved
    //! @return A const pointer to an inverted ECUnit if the named unit exists in the standard Units schema; otherwise, nullptr.
    ECOBJECTS_EXPORT static ECUnitCP GetInvertedUnit(Utf8CP unitName);
    //! Retrieves a Constant from the standard units schema. 
    //! @param[in]  unitName Name of the constant to be retrieved
    //! @return A const pointer to a constant ECUnit if the named unit exists in the standard Units schema; otherwise, nullptr.
    ECOBJECTS_EXPORT static ECUnitCP GetConstant(Utf8CP unitName);
    //! Retrieves a Phenomenon from the standard units schema. 
    //! @param[in]  phenomName Name of the phenomenon to be retrieved
    //! @return A const pointer to a Phenomenon if the named phenomenon exists in the standard Units schema; otherwise, nullptr.
    ECOBJECTS_EXPORT static PhenomenonCP GetPhenomenon(Utf8CP phenomName);
    //! Retrieves a UnitSystem from the standard units schema. 
    //! @param[in]  systemName Name of the system to be retrieved
    //! @return A const pointer to a UnitSystem if the named system exists in the standard Units schema; otherwise, nullptr.
    ECOBJECTS_EXPORT static UnitSystemCP GetUnitSystem(Utf8CP systemName);
    //! Retrieves the held schema
    //! @return The held schema
    ECOBJECTS_EXPORT static ECSchemaPtr GetSchema();
    };
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE
