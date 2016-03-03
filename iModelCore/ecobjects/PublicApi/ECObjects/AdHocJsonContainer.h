/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/AdHocJsonContainer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECObjects.h>
#include <json/value.h>

#define META_DATA_KEY_PREFIX "$"

#define TYPE_FIELD_NAME "Type"
#define UNITS_FIELD_NAME "Units"
#define CATEGORY_FIELD_NAME "Category"
#define HIDDEN_FIELD_NAME "Hidden"
#define READONLY_FIELD_NAME "ReadOnly"
#define EXTENDEDTYPE_FIELD_NAME "ExtendedType"
#define PRIORITY_FIELD_NAME "Priority"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct AdHocJsonContainer;

//=======================================================================================
//! A property value in an AdHocJsonContainer with methods to get/set values and attributes. 
//! Attributes are typically used to store units, type and other presentation information. 
//! @see AdHocJsonContainer
// @bsiclass                                                 Ramanujam.Raman   02/16
//=======================================================================================
struct AdHocJsonPropertyValue
{
friend struct AdHocJsonContainer;

enum class GetStatus
    {
    Found = 0,
    NotFound = 1
    };

private:
    Json::Value& m_value;
    Utf8String m_name;
    Utf8String m_attributeKey;

    AdHocJsonPropertyValue(Json::Value& value, Utf8CP name) : m_value(value), m_name(name)
        {
        m_attributeKey = GetAttributeKey(name);
        }

    template<typename AttributeType>
    void SetAttribute(Utf8CP attributeName, AttributeType const& attributeValue)
        {
        Json::Value& entry = m_value[m_attributeKey.c_str()];
        if (!entry.isObject())
            entry = Json::objectValue;

        entry[attributeName] = attributeValue;
        }

    void SetType(ECN::PrimitiveType primitiveType);

    Json::Value const* GetAttribute(Utf8CP attributeName) const;
    Json::Value const* GetJsonValue() const;

    static Utf8String GetAttributeKey(Utf8CP propertyName)
        {
        BeAssert(!Utf8String::IsNullOrEmpty(propertyName));
        Utf8PrintfString attributeKey("%s%s", META_DATA_KEY_PREFIX, propertyName ? propertyName : "");
        return attributeKey;
        }

public:
    /* Set values */

    //! Set a Boolean value
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValueBoolean(bool value);

    //! Set an Integer value
    //! @param[in] value Value of the property
    //! @param[in] units String representing the units of the value
    ECOBJECTS_EXPORT void SetValueInt(int32_t value, Utf8CP units = nullptr);

    //! Set an Int64 value
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValueInt64(int64_t value);

    //! Set a Double value
    //! @param[in] value Value of the property
    //! @param[in] units String representing the units of the value
    ECOBJECTS_EXPORT void SetValueDouble(double value, Utf8CP units = nullptr);

    //! Set a Text value
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValueText(Utf8CP value);

    //! Set a DPoint2d value
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValuePoint2D(DPoint2dCR value);

    //! Set an DPoint3d value
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValuePoint3D(DPoint3dCR value);

    //! Set a DateTime value
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValueDateTime(DateTimeCR value);

    //! Set a value from an ECValue (invariant)
    //! @param[in] value Value of the property
    //! @return SUCCESS if the setting was successful. ERROR otherwise. 
    //! @remarks Wraps over the other primitive SetValue methods. 
    ECOBJECTS_EXPORT BentleyStatus SetValueEC(ECN::ECValueCR value);

    //! Set a value to null
    ECOBJECTS_EXPORT void SetValueNull();

    //! Remove the value
    ECOBJECTS_EXPORT void RemoveValue();

    /* Get values */

    //! Checks if a value by the name exists and is not null
    //! @return true if the value exists and is not null. false otherwise.
    bool IsValueNull() const { return !m_value.isMember(m_name.c_str()) || m_value[m_name.c_str()].isNull(); }

    //! Get a Boolean value
    ECOBJECTS_EXPORT bool GetValueBoolean() const;

    //! Get a Text value
    ECOBJECTS_EXPORT Utf8CP GetValueText() const;

    //! Get a Double value
    ECOBJECTS_EXPORT double GetValueDouble() const;

    //! Get an Integer value
    ECOBJECTS_EXPORT int GetValueInt() const;

    //! Get an Int64 value
    ECOBJECTS_EXPORT int64_t GetValueInt64() const;

    //! Get a DPoint2d value
    ECOBJECTS_EXPORT DPoint2d GetValuePoint2D() const;

    //! Get a DPoint3d value
    ECOBJECTS_EXPORT DPoint3d GetValuePoint3D() const;

    //! Get a DateTime value
    ECOBJECTS_EXPORT DateTime GetValueDateTime() const;

    //! Get the value as an ECValue (invariant)
    ECOBJECTS_EXPORT ECValue GetValueEC() const;

    /* Set attributes */

    //! Set the units of the property value
    //! @param[in] units String representing the units of the property value
    void SetUnits(Utf8CP units) { SetAttribute<Utf8CP>(UNITS_FIELD_NAME, units); }

    //! Set the extended type to be used to show or edit the property value
    //! @param[in] extendedType Name of the extended type
    void SetExtendedType(Utf8CP extendedType) { SetAttribute<Utf8CP>(EXTENDEDTYPE_FIELD_NAME, extendedType); }

    //! Set the category for the property value (typically used for presentation)
    //! @param[in] category Name of the category
    void SetCategory(Utf8CP category) { SetAttribute<Utf8CP>(CATEGORY_FIELD_NAME, category); }

    //! Set the property value to be hidden (or shown)
    //! @param[in] isHidden Pass true to hide the property.
    void SetHidden(bool isHidden) { SetAttribute<bool>(HIDDEN_FIELD_NAME, isHidden); }

    //! Set the priority of the property value (typically used for presentation)
    //! @param[in] priority Pass a number to indicate the priority. @see PropertySortPriority
    void SetPriority(int priority) { SetAttribute<int>(PRIORITY_FIELD_NAME, priority); }

    //! Set that the property value is to be treated as read only.
    //! @param[in] isReadOnly Pass true to make the property read only.
    void SetReadOnly(bool isReadOnly) { SetAttribute<bool>(READONLY_FIELD_NAME, isReadOnly); }

    /* Get attributes */

    //! Get the type of the property value
    //! @param[out] type Type of the property value
    //! @return GetStatus::Found if the property was set. GetStatus::NotFound otherwise. 
    ECOBJECTS_EXPORT GetStatus GetType(PrimitiveType& type) const;

    //! Get the extended type used to show or edit the property value 
    //! @param[out] extendedType Name of the extended type
    //! @return GetStatus::Found if the property was set. GetStatus::NotFound otherwise. 
    ECOBJECTS_EXPORT GetStatus GetExtendedType(Utf8StringR extendedType) const;

    //! Get the units of the property value
    //! @param[out] units String representing the units of the property
    //! @return GetStatus::Found if the property was set. GetStatus::NotFound otherwise. 
    ECOBJECTS_EXPORT GetStatus GetUnits(Utf8StringR units) const;

    //! Get the category for the property value (typically used for presentation)
    //! @param[out] category Name of the category
    //! @return GetStatus::Found if the property was set. GetStatus::NotFound otherwise. 
    ECOBJECTS_EXPORT GetStatus GetCategory(Utf8StringR category) const;

    //! Get the flag indicating if the property value should be hidden (or shown)
    //! @param[out] isHidden true if the property is to be hidden.
    //! @return GetStatus::Found if the property was set. GetStatus::NotFound otherwise. 
    ECOBJECTS_EXPORT GetStatus GetHidden(bool& isHidden) const;

    //! Get the priority of the property value (typically used for presentation)
    //! @param[out] priority A number that indicates the the priority. @see PropertySortPriority
    //! @return GetStatus::Found if the property was set. GetStatus::NotFound otherwise. 
    ECOBJECTS_EXPORT GetStatus GetPriority(int& priority) const;
    
    //! Get the flag indicating if the property value should be treated as read only.
    //! @param[out] isReadOnly true if the property should be read only.
    //! @return GetStatus::Found if the property was set. GetStatus::NotFound otherwise. 
    ECOBJECTS_EXPORT GetStatus GetReadOnly(bool& isReadOnly) const;
};


//=======================================================================================
//! AdHocJsonContainer represents a collection of property values used by the AdHocJson 
//! ExtendedType. It wraps around a standard JSON objectValue, and includes additional, 
//! optional attributes on the contained property values - units, type, and other 
//! presentation information. 
//!
//! The AdHocJsonContainer contains a collection of name-value pairs:
//! "name": \<value\>
//! the optional attributes are setup in another node at the same level:
//! "$name": \<attributes\>
//! where the attributes is in-turn a JSON struct value, with the following format:
//! {"type": <Int|Boolean|Text|DPoint3d|DPoint2d>, "units": "unitsString", "category": "CategoryLabel", "hidden": true/false ...} 
//! 
//! Example: 
//! {
//! "FirstName" : "John Doe",
//! "$FirstName": {"type": "Text", "priority" : 1000, "category": "Personal Info"}
//! "Age" : 67,
//! "Employed" : true,
//! "Location" : {"x": "1.1", "y": "2.2", "z" : "3.3"},
//! "$Location" : {"type": "DPoint3d", "hidden": true},
//! }
//! 
// @bsiclass                                                 Ramanujam.Raman   02/16
//=======================================================================================
struct AdHocJsonContainer
{
private:
    Json::Value m_value;

public:
    //! Constructor
    AdHocJsonContainer() : m_value(Json::objectValue) {}

    //! Initialize from a JSON string (from a stored string)
    ECOBJECTS_EXPORT BentleyStatus FromString(Utf8CP jsonStr);

    //! Convert to a JSON string (to a string that be stored)
    ECOBJECTS_EXPORT Utf8String ToString();

    //! Get the property value by name
    //! @param[in] name Name of the property
    AdHocJsonPropertyValue Get(Utf8CP name) { return AdHocJsonPropertyValue(m_value, name); }

    //! Check if property value is contained
    //! @param[in] name Name of the property
    //! @return true if the value exists. false otherwise.
    bool Contains(Utf8CP name) const { return m_value.isMember(name); }

    //! Remove a property value
    //! @param[in] name Name of the property
    void Remove(Utf8CP name) { AdHocJsonPropertyValue(m_value, name).RemoveValue(); }

    //! Removes all the property values
    void Clear() { m_value = Json::objectValue; }

    //! Returns true if there aren't any property values and the container is empty
    bool IsEmpty() { return m_value.empty(); }
};

END_BENTLEY_ECOBJECT_NAMESPACE
