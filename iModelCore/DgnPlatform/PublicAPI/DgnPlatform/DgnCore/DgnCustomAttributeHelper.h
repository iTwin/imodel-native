/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnCustomAttributeHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================    
//! DgnCustomAttributeHelper is a convenience API for the custom attributes defined
//! in the dgn standard ECSchema
//! @bsiclass
//=======================================================================================    
struct DgnCustomAttributeHelper
    {
private:
    DgnCustomAttributeHelper();
    ~DgnCustomAttributeHelper();

public:
    //! Tries to retrieve the HandlerInfo custom attribute from the specified ECClass.
    //! @param[out] requiredForModification Indicates whether a handler is required for instances of an ECClass to be modified.
    //! @param[out] allowDeleteIfMissing Indicates whether instances of an ECClass can be deleted if a handler is missing.
    //! @param[in] ecClass ECClass to retrieve the custom attribute from.
    //! @param[in] includeInherited true to include inherited values, false to only read local info.
    //! @return true if @p ecClass has the custom attribute. false, if @p ecClass doesn't have the custom attribute
    DGNPLATFORM_EXPORT static bool TryGetHandlerInfo(bool& requiredForModification, bool& allowDeleteIfMissing, ECN::ECClassCR ecClass, bool includeInherited = true);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
