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

    static Utf8CP const DATETIMEKIND_UTC_STR;
    static WCharCP const DATETIMEKIND_UTC_WSTR;
    static Utf8CP const DATETIMEKIND_UNSPECIFIED_STR;
    static WCharCP const DATETIMEKIND_UNSPECIFIED_WSTR;
    static Utf8CP const DATETIMEKIND_LOCAL_STR;
    static WCharCP const DATETIMEKIND_LOCAL_WSTR;
    static Utf8CP const DATETIMECOMPONENT_DATETIME_STR;
    static WCharCP const DATETIMECOMPONENT_DATETIME_WSTR;
    static Utf8CP const DATETIMECOMPONENT_DATE_STR;
    static WCharCP const DATETIMECOMPONENT_DATE_WSTR;

    //static class
    DateTimeInfoAccessor ();
    ~DateTimeInfoAccessor ();

    static bool TryParseKind (bool& isKindNull, DateTime::Kind& kind, ECValueCR ecValue);
    static bool TryParseKind (bool& isKindNull, DateTime::Kind& kind, Utf8CP kindStr);
    static bool TryParseKind (bool& isKindNull, DateTime::Kind& kind, Utf16CP kindStr);
    static bool TryParseComponent (bool& isComponentNull, DateTime::Component& component, ECValueCR ecValue);
    static bool TryParseComponent (bool& isComponentNull, DateTime::Component& component, Utf8CP componentStr);
    static bool TryParseComponent (bool& isComponentNull, DateTime::Component& component, Utf16CP componentStr);

public:
    //! Retrieves the DateTimeInfo meta data from the specified DateTime ECProperty.
    //! @param[out] dateTimeInfo the retrieved DateTimeInfo meta data
    //! @param[in] dateTimeProperty the DateTime ECProperty from which the meta data is to be retrieved
    //! @return true if the ECProperty contains the DateTimeInfo meta data, false if the
    //!         ECProperty doesn't contain the DateTimeInfo meta data or in case of errors.
    static bool TryGetFrom (DateTimeInfo& dateTimeInfo, ECPropertyCR dateTimeProperty);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

