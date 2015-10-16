/*--------------------------------------------------------------------------------------+
|
|     $Source: src/DateTimeInfoAccessor.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
	static WCharCP const DATETIMEINFO_SCHEMANAME;
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

    static void LogPropertyNotFoundError (WCharCP propertyName);

public:
    //! Retrieves the content of the \b %DateTimeInfo custom attribute from the specified date time ECProperty.
    //! @remarks The \b %DateTimeInfo custom attribute is defined in the standard schema \b Bentley_Standard_CustomAttributes.
    //!          See also DateTimeInfo.
    //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute
    //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
    //! @return true if \p dateTimeProperty contains the %DateTimeInfo custom attribute, false if \p dateTimeProperty 
    //!         doesn't contain the %DateTimeInfo custom attribute or in case of errors.
    static bool TryGetFrom (DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

