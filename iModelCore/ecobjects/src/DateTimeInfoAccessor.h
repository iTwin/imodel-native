/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>
#include <Bentley/Nullable.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================    
//! DateTimeInfoAccessor allows to access the DateTimeInfo custom attribute on a date time ECProperty
//! @bsiclass
//=======================================================================================    
struct DateTimeInfoAccessor final
    {
    private:
        //static class
        DateTimeInfoAccessor() = delete;
        ~DateTimeInfoAccessor() = delete;

        static bool TryParseKind(Nullable<DateTime::Kind>& kind, ECValueCR ecValue);
        static bool TryParseComponent(Nullable<DateTime::Component>& component, ECValueCR ecValue);

        static void LogPropertyNotFoundError(Utf8CP propertyName);

    public:
        //! Retrieves the DateTimeInfo metadata from the specified date time ECProperty.
        //! @remarks The DateTimeInfo metadata is defined through the \b %DateTimeInfo custom attribute (defined in the standard schema 
        //! @b CoreCustomAttributes) on a date time ECProperty.
        //! See also DateTimeInfo.
        //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute. If the property did not
        //!             carry the %DateTimeInfo custom attribute, the parameter remains unchanged.
        //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
        //! @return ::ECObjectsStatus::Success in case of success, error codes in case of parsing errors or if @p dateTimeProperty 
        //! is not a primitive or primitive array property of type ::PRIMITIVETYPE_DateTime. 
        static ECObjectsStatus GetFrom(DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

