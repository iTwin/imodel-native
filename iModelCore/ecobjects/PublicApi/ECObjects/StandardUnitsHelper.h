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
    //! @param[out] unit Unit retrieved. Nullptr if not found
    //! @param[in]  unitName Name of the unit to be retrieved
    //! @return ECObjectsStatus::Success in case of success, ECObjectsStatus::NotFound if not found
    ECOBJECTS_EXPORT static ECObjectsStatus GetUnit(ECUnitCP& unit, Utf8CP unitName);
    //! Retrieves an InvertedUnit from the standard units schema. 
    //! @param[out] invertedUnit InvertedUnit retrieved. Nullptr if not found
    //! @param[in]  unitName Name of the inverted unit to be retrieved
    //! @return ECObjectsStatus::Success in case of success, ECObjectsStatus::NotFound if not found
    ECOBJECTS_EXPORT static ECObjectsStatus GetInvertedUnit(ECUnitCP& invertedUnit, Utf8CP unitName);
    //! Retrieves a Constant from the standard units schema. 
    //! @param[out] constant Constant retrieved. Nullptr if not found
    //! @param[in]  unitName Name of the constant to be retrieved
    //! @return ECObjectsStatus::Success in case of success, ECObjectsStatus::NotFound if not found
    ECOBJECTS_EXPORT static ECObjectsStatus GetConstant(ECUnitCP& constant, Utf8CP unitName);
    //! Retrieves a Phenomenon from the standard units schema. 
    //! @param[out] phenom Phenomenon retrieved. Nullptr if not found
    //! @param[in]  phenomName Name of the phenomenon to be retrieved
    //! @return ECObjectsStatus::Success in case of success, ECObjectsStatus::NotFound if not found
    ECOBJECTS_EXPORT static ECObjectsStatus GetPhenomenon(PhenomenonCP& phenom, Utf8CP phenomName);
    //! Retrieves a UnitSystem from the standard units schema. 
    //! @param[out] system Unit System retrieved. Nullptr if not found
    //! @param[in]  systemName Name of the system to be retrieved
    //! @return ECObjectsStatus::Success in case of success, ECObjectsStatus::NotFound if not found
    ECOBJECTS_EXPORT static ECObjectsStatus GetUnitSystem(UnitSystemCP& system, Utf8CP systemName);
    };
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE
