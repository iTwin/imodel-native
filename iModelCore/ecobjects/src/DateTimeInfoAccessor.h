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
    static Utf8CP const DATETIMEINFO_CLASSNAME;
    static Utf8CP const DATETIMEINFO_KIND_PROPERTYNAME;
    static Utf8CP const DATETIMEINFO_COMPONENT_PROPERTYNAME;

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

    static void LogPropertyNotFoundError (Utf8CP propertyName);

public:
    //! Retrieves the DateTimeInfo metadata from the specified date time ECProperty.
    //! @remarks The DateTimeInfo metadata is defined through the \b %DateTimeInfo custom attribute (defined in the standard schema 
    //! @b Bentley_Standard_CustomAttributes) on a date time ECProperty.
    //! See also DateTimeInfo.
    //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute. If the property did not
    //!             carry the %DateTimeInfo custom attribute, the resulting @p dateTimeInfo's 'IsXXXNull' flags are set to true.
    //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
    //! @return ::ECOBJECTS_STATUS_Success in case of success, error codes in case of parsing errors or if @p dateTimeProperty 
    //! is not of type ::PRIMITIVETYPE_DateTime. 
    static ECObjectsStatus GetFrom (DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

