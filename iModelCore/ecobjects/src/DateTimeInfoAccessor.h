/*--------------------------------------------------------------------------------------+
|
|     $Source: src/DateTimeInfoAccessor.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ECObjects.h>
#include <ECObjects/StandardCustomAttributeHelper.h>
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================    
//! DateTimeInfoAccessor allows to access the DateTimeInfo custom attribute on a date time ECProperty
//! @bsiclass
//=======================================================================================    
struct DateTimeInfoAccessor : NonCopyableClass
    {
private:
    static WCharCP const DATETIMEINFO_CLASSNAME;
    static WCharCP const DATETIMEINFO_KIND_PROPERTYNAME;
    static WCharCP const DATETIMEINFO_COMPONENT_PROPERTYNAME;

    //static class
    DateTimeInfoAccessor ();
    ~DateTimeInfoAccessor ();

    static bool TryParseKind (bool& isKindNull, DateTime::Kind& kind, ECValueCR ecValue);
    static bool TryParseComponent (bool& isComponentNull, DateTime::Component& component, ECValueCR ecValue);

public:
    //! Retrieves the DateTimeInfo meta data from the specified DateTime ECProperty.
    //! @param[out] dateTimeInfo the retrieved DateTimeInfo meta data
    //! @param[in] dateTimeProperty the DateTime ECProperty from which the meta data is to be retrieved
    //! @return true if the ECProperty contains the DateTimeInfo meta data, false if the
    //!         ECProperty doesn't contain the DateTimeInfo meta data or in case of errors.
    static bool TryGetFrom (DateTimeInfo& dateTimeInfo, ECPropertyCR dateTimeProperty);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

