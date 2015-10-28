/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandardCustomAttributeHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
//! DateTimeInfo contains the meta data held by the custom attribute \b %DateTimeInfo on an 
//! ECProperty of type DgnPlatform::PRIMITIVETYPE_DateTime.
//! @remarks 
//! Date time values in ECObjects are represented by the DateTime class. Each DateTime instance can 
//! contain metadata about the actual date time value (see DateTime::Info). 
//! In order to preserve the metadata when persisting a DateTime, clients can decorate the respective
//! ECProperty with the \b %DateTimeInfo custom attribute from the standard ECSchema \b Bentley_Standard_CustomAttributes.
//! @bsiclass
//=======================================================================================    
struct DateTimeInfo
    {
private: 
    bool m_isKindNull;
    bool m_isComponentNull;
    DateTime::Info m_info;

    static const DateTime::Kind DEFAULT_KIND;
    static const DateTime::Component DEFAULT_COMPONENT;
    static const DateTime::Info s_default;

public:
    //***** Construction ******
//__PUBLISH_SECTION_END__
    //Intentionally use the compiler-generated versions of copy constructor, assignment operator, and destructor
//__PUBLISH_SECTION_START__

    //! Initializes a new instance of the DateTimeInfo type.
    ECOBJECTS_EXPORT DateTimeInfo ();

    //! Initializes a new instance of the DateTimeInfo type.
    //! @param[in] metadata object from which the DateTimeInfo will be initialized.
    ECOBJECTS_EXPORT explicit DateTimeInfo (DateTime::Info const& metadata);

    //__PUBLISH_SECTION_END__
    DateTimeInfo (bool isKindNull, DateTime::Kind kind, bool isComponentNull, DateTime::Component component); 
    //__PUBLISH_SECTION_START__

    //! Compares this DateTimeInfo to @p rhs.
    //! @param [in] rhs DateTimeInfo to compare this against
    //! @return true if both objects are equal, false otherwise
    ECOBJECTS_EXPORT bool operator== (DateTimeInfo const& rhs) const;

    //! Compares this DateTimeInfo to @p rhs for inequality.
    //! @param [in] rhs DateTimeInfo to compare this against
    //! @return true if the objects are not equal, false otherwise
    ECOBJECTS_EXPORT bool operator!= (DateTimeInfo const& rhs) const;

    //! Indicates whether the DateTime::Kind is unset or not.
    //! @return true, if the DateTime::Kind is unset. false, otherwise
    ECOBJECTS_EXPORT bool IsKindNull () const;
    //! Indicates whether the DateTime::Component is unset or not.
    //! @return true, if the DateTime::Component is unset. false, otherwise
    ECOBJECTS_EXPORT bool IsComponentNull () const;

    //__PUBLISH_SECTION_END__
    //! Gets the content of this object as DateTime::Info.
    //! @remarks Should only be called if DateTimeInfo::IsKindNull and DateTimeInfo::IsComponentNull are not true.
    //! @return DateTime::Info representing the content of this object
    DateTime::Info const& GetInfo () const;
    //__PUBLISH_SECTION_START__

    //! Gets the content of this object as DateTime::Info.
    //! @remarks if \p useDefaultIfUnset is true, fills in default values for date time kind
    //!         and date time component if they are unset.
    //!         @see GetDefault
    //! @param[in] useDefaultIfUnset if true, default values are filled in, if a member of this object is unset,
    //!            if false, no default values are filled in. The value of unset members is undefined.
    //!            Callers have to check the unset status first using DateTimeInfo::IsKindNull and DateTimeInfo::IsComponentNull
    //! @return DateTime::Info representing the content of this object
    ECOBJECTS_EXPORT DateTime::Info GetInfo (bool useDefaultIfUnset) const; 
    
    //! Gets a DateTimeInfo object with the default values used by ECObjects.
    //! @remarks The default values are DateTime::Kind::Unspecified and DateTime::Component::DateAndTime.
    //! @return Default DateTime::Info
    ECOBJECTS_EXPORT static DateTime::Info const& GetDefault (); 

    //! Checks whether the RHS object matches this object.
    //! @remarks If one of the members
    //!          of this object is null, the RHS counterpart is ignored and the
    //!          members are considered matching.
    //! @param[in] rhs RHS
    //! @return true, if the RHS matches this object. false otherwise
    ECOBJECTS_EXPORT bool IsMatchedBy (DateTime::Info const& rhs) const;

    //! Generates a text representation of this object.
    //! @return Text representation of this object
    ECOBJECTS_EXPORT WString ToString () const;
    };

//=======================================================================================    
//! StandardCustomAttributeHelper provides APIs to access items of the Bentley standard schemas
//! @bsiclass
//=======================================================================================    
struct StandardCustomAttributeHelper : NonCopyableClass
    {
    //__PUBLISH_SECTION_END__
private:
    static WCharCP const SYSTEMSCHEMA_CA_NAME;
    static WCharCP const DYNAMICSCHEMA_CA_NAME;
		static WCharCP const SYSTEMSCHEMA_CA_SCHEMA;

    //static class
    StandardCustomAttributeHelper ();
    ~StandardCustomAttributeHelper ();
    //__PUBLISH_SECTION_START__

public:
    //! Retrieves the content of the @b %DateTimeInfo custom attribute from the specified date time ECProperty.
    //! @remarks The @b %DateTimeInfo custom attribute is defined in the standard schema @b Bentley_Standard_CustomAttributes.
    //!          See also DateTimeInfo.
    //! @param[out] dateTimeInfo the retrieved content of the %DateTimeInfo custom attribute
    //! @param[in] dateTimeProperty the date time ECProperty from which the custom attribute is to be retrieved
    //! @return true if @p dateTimeProperty contains the %DateTimeInfo custom attribute, false if @p dateTimeProperty 
    //!         doesn't contain the %DateTimeInfo custom attribute or in case of errors.
    ECOBJECTS_EXPORT static bool TryGetDateTimeInfo (DateTimeInfoR dateTimeInfo, ECPropertyCR dateTimeProperty);

    //! Indicates whether the specified schema is a @b system schema (in contrast to a user-supplied schema) by
    //! checking whether the @b %SystemSchema custom attribute from the standard schema @b Bentley_Standard_CustomAttributes
    //! is assigned to the schema.
    //! @remarks A system schema is a schema used and managed internally by the software.
    //! @param[in] schema Schema to check
    //! @return true, if @p schema is a system schema. false, otherwise
    ECOBJECTS_EXPORT static bool IsSystemSchema (ECSchemaCR schema);

    //! Indicates whether the specified schema is a so-called @b dynamic schema by
    //! checking whether the @b %DynamicSchema custom attribute from the standard schema @b Bentley_Standard_CustomAttributes
    //! is assigned to the schema.
    //! @remarks A dynamic schema is an application-generated schema where schema name is used as namespace for classes.
    //! @param[in] schema Schema to check
    //! @return true, if @p schema is a dynamic schema. false, otherwise
    ECOBJECTS_EXPORT static bool IsDynamicSchema (ECSchemaCR schema);

    //! Marks a schema as @b dynamic schema by adding the custom attribute @b DynamicSchema from the standard schema @b 
    //! Bentley_Standard_CustomAttributes to it.
    //! If the standard schema is not yet referenced, an error will be returned.
    //! @remarks A dynamic schema is an application-generated schema where schema name is used as namespace for classes.
    //! @param[in] schema Schema to mark as dynamic
    //! @param[in] isDynamicSchema true, if this schema should be marked as dynamic schema. false, otherwise.
    //! @return A status code indicating success or error
    ECOBJECTS_EXPORT static ECObjectsStatus SetIsDynamicSchema (ECSchemaR schema, bool isDynamicSchema);

    //! Returns the specified CustomAttribute ECClass
    //! @param[in] attributeName The name of the CustomAttribute ECClass
    //! @return An ECClassCP, if the attribute is found.  NULL otherwise.
    ECOBJECTS_EXPORT static ECClassCP GetCustomAttributeClass(WCharCP attributeName);

    //! Creates a custom attribute instance for the given attribute
    //! @param[in] attributeName The name of the custom attribute to create
    //! @return An instance of the given custom attribute
    ECOBJECTS_EXPORT static IECInstancePtr CreateCustomAttributeInstance(WCharCP attributeName);
    };
	
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

//__PUBLISH_SECTION_END__
