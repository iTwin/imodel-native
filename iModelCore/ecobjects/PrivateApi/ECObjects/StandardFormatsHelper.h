/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/ECObjects/StandardFormatsHelper.h $
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
//! Allows access to the standard Formats schema. Caches it to prevent reparsing
//! @bsiclass
//=======================================================================================    
struct StandardFormatsHelper final
    {
private:
    //static class
    StandardFormatsHelper();
    ~StandardFormatsHelper() {}

public:
    //! Retrieves a Format from the standard formats schema.
    //! @param[in]  unitName Name of the format to be retrieved
    //! @return A const pointer to an ECFormat if the named format exists in the standard Formats schema; otherwise, nullptr.
    ECOBJECTS_EXPORT static ECUnitCP GetFormat(Utf8CP unitName);
    //! Retrieves the held schema
    //! @return The held schema
    ECOBJECTS_EXPORT static ECSchemaPtr GetSchema();
    };
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE
