/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/NonCopyableClass.h>
#include <Bentley/DateTime.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECInstance.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//! @addtogroup ECObjectsGroup
//! @beginGroup

//=======================================================================================    
//! StandardCustomAttributeHelper provides APIs to access items of the Bentley standard schemas
//! @remarks Deprecated. Only use to access legacy schema, Bentley_Standard_CustomAttributes.
//! @bsiclass
//=======================================================================================    
struct StandardCustomAttributeHelper : NonCopyableClass
    {
private:
    //static class
    StandardCustomAttributeHelper ();
    ~StandardCustomAttributeHelper ();

public:
    //! Retrieves the DateTimeInfo metadata from the specified date time ECProperty.
    //! @remarks The DateTimeInfo metadata is defined through the \b %DateTimeInfo custom attribute (defined in the standard schema 
    //! @b Bentley_Standard_CustomAttributes) on a date time ECProperty.
    //! See also DateTimeInfo.
    //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute. If the property did not
    //!             carry the %DateTimeInfo custom attribute, the parameter remains unmodified.
    //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
    //! @return ECObjectsStatus::Success in case of success, error codes in case of parsing errors or if @p dateTimeProperty 
    //! is not of type ::PRIMITIVETYPE_DateTime. 
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeInfo (DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty);

    //! Returns the specified CustomAttribute ECClass
    //! @param[in] attributeName The name of the CustomAttribute ECClass
    //! @return An ECClassCP, if the attribute is found.  NULL otherwise.
    ECOBJECTS_EXPORT static ECClassCP GetCustomAttributeClass(Utf8CP attributeName);

    //! Creates a custom attribute instance for the given attribute
    //! @param[in] attributeName The name of the custom attribute to create
    //! @return An instance of the given custom attribute
    ECOBJECTS_EXPORT static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attributeName);
    };

//=======================================================================================    
//! CoreCustomAttributeHelper provides APIs to access items of the Bentley standard schema, 
//! CoreCustomAttributes
//! @bsiclass
//=======================================================================================    
struct CoreCustomAttributeHelper final
    {
private:
    //static class
    CoreCustomAttributeHelper();
    ~CoreCustomAttributeHelper();

public:
    //! Retrieves the DateTimeInfo metadata from the specified date time ECProperty.
    //! @remarks The DateTimeInfo metadata is defined through the \b %DateTimeInfo custom attribute (defined in the standard schema 
    //! @b CoreCustomAttributes) on a date time ECProperty.
    //! See also DateTimeInfo.
    //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute. If the property did not
    //!             carry the %DateTimeInfo custom attribute, the parameter remains unmodified.
    //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
    //! @return ECObjectsStatus::Success in case of success, error codes in case of parsing errors or if @p dateTimeProperty 
    //! is not of type ::PRIMITIVETYPE_DateTime. 
    ECOBJECTS_EXPORT static ECObjectsStatus GetDateTimeInfo (DateTime::Info& dateTimeInfo, ECPropertyCR dateTimeProperty);

    //! Checks whether the specified ECClass has the @b ClassHasCurrentTimeStampProperty CustomAttribute and, if yes,
    //! returns the ECProperty pointed to by the custom attribute which is meant to hold a current time stamp.
    //! @param[out] currentTimeStampProp Retrieved current timestamp property
    //! @param[in] ecClass ECClass to look for the @b ClassHasCurrentTimeStampProperty CustomAttribute
    //! @return SUCCESS if the class has the custom attribute and the property could be retrieved successfully - or
    //! if the class doesn't have the custom attribute. In the latter case, @p currentTimeStampProp is set to nullptr.
    //! ERROR if the custom attribute is malformed or is not pointing to a date time property.
    ECOBJECTS_EXPORT static BentleyStatus GetCurrentTimeStampProperty(PrimitiveECPropertyCP& currentTimeStampProp, ECClassCR ecClass);

    //! Returns the specified ECCustomAttributeClass from the CoreCustomAttributes schema
    //! @param[in] attributeName The name of the ECCustomAttributeClass
    //! @return An ECCustomAttributeClass, if the class is found in the CoreCustomAttributes schema. Otherwise, nullptr will be returned.
    ECOBJECTS_EXPORT static ECCustomAttributeClassCP GetCustomAttributeClass(Utf8CP attributeName);

    //! Returns the specified ECClass from the CoreCustomAttributes schema
    //! @param[in] attributeName The name of the ECClass
    //! @return An ECClass, if the class is found in the CoreCustomAttributes schema. Otherwise, nullptr will be returned.
    ECOBJECTS_EXPORT static ECClassCP GetClass(Utf8CP attributeName);

    //! Creates a custom attribute instance for the given custom attribute from the CoreCustomAttributes schema
    //! @remarks The only supported custom attributes at this time are SupplementalSchemaMetaData, SupplementalProvenance, and
    //! IsMixin. If any other custom attributes are desired, use GetCustomAttributeClass and create an instance from the resulting
    //! class.
    //! @param[in] attributeName The name of the ECCustomAttributeClass to create an ECInstance of.
    //! @return An IECInstance of the given custom attribute name, if it is one of the supported custom attributes. Otherwise, nullptr will be returned.
    ECOBJECTS_EXPORT static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attributeName);

    ECOBJECTS_EXPORT static ECSchemaPtr GetSchema();
    };

//=======================================================================================    
//! ConversionCustomAttributeHelper provides APIs to access items of the Bentley standard schema, 
//! ECv3ConversionAttributes
//! @bsiclass
//=======================================================================================    
struct ConversionCustomAttributeHelper final
    {
    private:
        //static class
        ConversionCustomAttributeHelper();
        ~ConversionCustomAttributeHelper();

    public:
        //! Creates a custom attribute instance for the given custom attribute from the ECv3ConversionAttributes schema
        //! @remarks The only supported custom attribute at this time is PropertyRenamed. 
        //! @param[in] attributeName The name of the ECCustomAttributeClass to create an ECInstance of.
        //! @return An IECInstance of the given custom attribute name, if it is a supported custom attribute. Otherwise, nullptr will be returned.
        ECOBJECTS_EXPORT static IECInstancePtr CreateCustomAttributeInstance(Utf8CP attributeName);

        ECOBJECTS_EXPORT static void Reset();
    };

/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE
