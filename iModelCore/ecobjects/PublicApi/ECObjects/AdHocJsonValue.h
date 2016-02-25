/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/AdHocJsonValue.h $
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

//=======================================================================================
//! AdHocJsonValue represents a JSON value used by the AdHocJson ExtendedType. 
//! It's the standard JSON value with additional optional meta-data on units, 
//! type and other presentation information. The JSON can only contain primitive EC Types. 
//!
//! The AdHocJsonValue in turns contains a collection of property values, or name-value pairs:
//! "name": <value> 
//! the optional meta data is setup in another node at the same level as - 
//! "$name": <meta-data>
//! where meta-data is a JSON struct value - 
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
struct AdHocJsonValue
{
private:
    Json::Value m_value;

    template<typename MetaDataType>
    void SetMetaData(Utf8CP name, Utf8CP metaDataName, MetaDataType const& metaDataValue)
        {
        Utf8String metaDataKey = Utf8PrintfString("%s%s", META_DATA_KEY_PREFIX, name);

        Json::Value& entry = m_value[metaDataKey.c_str()];
        if (!entry.isObject())
            entry = Json::objectValue;

        entry[metaDataName] = metaDataValue;
        }

    Json::Value const* GetMetaData(Utf8CP name, Utf8CP metaDataName) const;
    Json::Value const* GetValue(Utf8CP name) const;

    void SetType(Utf8CP name, ECN::PrimitiveType primitiveType);

public:
    //! Constructor
    AdHocJsonValue() : m_value(Json::objectValue) {}

    //! Initialize from a JSON string (from a stored string)
    ECOBJECTS_EXPORT BentleyStatus FromString(Utf8CP jsonStr);

    //! Convert to a JSON string (to a string that be stored)
    ECOBJECTS_EXPORT Utf8String ToString();

    /* Set values */

    //! Set a Boolean property value
    //! @param[in] name Name of the property
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValueBoolean(Utf8CP name, bool value);

    //! Set an Integer property value
    //! @param[in] name Name of the property
    //! @param[in] value Value of the property
    //! @param[in] units String representing the units of the property value
    ECOBJECTS_EXPORT void SetValueInt(Utf8CP name, int32_t value, Utf8CP units = nullptr);

    //! Set an Int64 property value
    //! @param[in] name Name of the property
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValueInt64(Utf8CP name, int64_t value);

    //! Set a Double property value
    //! @param[in] name Name of the property
    //! @param[in] value Value of the property
    //! @param[in] units String representing the units of the property value
    ECOBJECTS_EXPORT void SetValueDouble(Utf8CP name, double value, Utf8CP units = nullptr);

    //! Set a Text property value
    //! @param[in] name Name of the property
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValueText(Utf8CP name, Utf8CP value);

    //! Set a DPoint2d property value
    //! @param[in] name Name of the property
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValuePoint2D(Utf8CP name, DPoint2dCR value);

    //! Set an DPoint3d property value
    //! @param[in] name Name of the property
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValuePoint3D(Utf8CP name, DPoint3dCR value);

    //! Set a DateTime property value
    //! @param[in] name Name of the property
    //! @param[in] value Value of the property
    ECOBJECTS_EXPORT void SetValueDateTime(Utf8CP name, DateTimeCR value);

    //! Set a property value from an ECValue (invariant)
    //! @param[in] name Key for the property
    //! @param[in] value Value of the property
    //! @return true if the setting was successful. false otherwise. 
    //! @remarks Wraps over the other primitive SetValue methods. 
    ECOBJECTS_EXPORT bool SetValueEC(Utf8CP name, ECN::ECValueCR value);

    //! Set a property value to null
    //! @param[in] name Name of the property
    ECOBJECTS_EXPORT void SetValueNull(Utf8CP name);

    //! Remove a property value
    //! @param[in] name Name of the property
    ECOBJECTS_EXPORT void RemoveValue(Utf8CP name);

    //! Removes all the property values
    void Clear() { m_value = Json::objectValue; }

    //! Returns true if there aren't any property values and the container is empty
    bool IsEmpty() { return m_value.empty(); }

    /* Get values */

    //! Checks if a property value by the name exists and is not null
    //! @param[in] name Name of the property
    //! @return true if the value exists and is not null. false otherwise.
    bool IsValueNull(Utf8CP name) const { return !m_value.isMember(name)  || m_value[name].isNull(); }

    //! Get a Boolean property value
    //! @param[in] name Name of the property
    ECOBJECTS_EXPORT bool GetValueBoolean(Utf8CP name) const;

    //! Get a Text property value
    //! @param[in] name Key for the property
    ECOBJECTS_EXPORT Utf8CP GetValueText(Utf8CP name) const;

    //! Get a Double property value
    //! @param[in] name Key for the property
    ECOBJECTS_EXPORT double GetValueDouble(Utf8CP name) const;

    //! Get an Integer property value
    //! @param[in] name Key for the property
    ECOBJECTS_EXPORT int GetValueInt(Utf8CP name) const;

    //! Get an Int64 property value
    //! @param[in] name Key for the property
    ECOBJECTS_EXPORT int64_t GetValueInt64(Utf8CP name) const;

    //! Get a DPoint2d property value
    //! @param[in] name Key for the property
    ECOBJECTS_EXPORT DPoint2d GetValuePoint2D(Utf8CP name) const;

    //! Get a DPoint3d property value
    //! @param[in] name Key for the property
    ECOBJECTS_EXPORT DPoint3d GetValuePoint3D(Utf8CP name) const;

    //! Get a DateTime property value
    //! @param[in] name Key for the property
    ECOBJECTS_EXPORT DateTime GetValueDateTime(Utf8CP name) const;

    //! Get the property value as an ECValue (invariant)
    //! @param[in] name Key for the property
    ECOBJECTS_EXPORT ECValue GetValueEC(Utf8CP name) const;

    /* Set meta-data */

    //! Set the units of the property value
    //! @param[in] name Name of the property
    //! @param[in] units String representing the units of the property value
    void SetUnits(Utf8CP name, Utf8CP units) { SetMetaData<Utf8CP>(name, UNITS_FIELD_NAME, units); }

    //! Set the extended type to be used to show or edit the property value
    //! @param[in] name Name of the property
    //! @param[in] extendedType Name of the extended type
    void SetExtendedType(Utf8CP name, Utf8CP extendedType) { SetMetaData<Utf8CP>(name, EXTENDEDTYPE_FIELD_NAME, extendedType); }

    //! Set the category for the property value (typically used for presentation)
    //! @param[in] name Name of the property
    //! @param[in] category Name of the category
    void SetCategory(Utf8CP name, Utf8CP category) { SetMetaData<Utf8CP>(name, CATEGORY_FIELD_NAME, category); }

    //! Set the property value to be hidden (or shown)
    //! @param[in] name Name of the property
    //! @param[in] isHidden Pass true to hide the property.
    void SetHidden(Utf8CP name, bool isHidden) { SetMetaData<bool>(name, HIDDEN_FIELD_NAME, isHidden); }

    //! Set the priority of the property value (typically used for presentation)
    //! @param[in] name Name of the property
    //! @param[in] priority Pass a number to indicate the priority. @see PropertySortPriority
    void SetPriority(Utf8CP name, int priority) { SetMetaData<int>(name, PRIORITY_FIELD_NAME, priority); }

    //! Set that the property value is to be treated as read only.
    //! @param[in] name Name of the property
    //! @param[in] isReadOnly Pass true to make the property read only.
    void SetReadOnly(Utf8CP name, bool isReadOnly) { SetMetaData<bool>(name, READONLY_FIELD_NAME, isReadOnly); }

    /* Get meta-data */

    //! Get the type of the property value
    //! @param[out] type Type of the property value
    //! @param[in] name Name of the property
    //! @return true if the type was set. false otherwise. 
    ECOBJECTS_EXPORT bool GetType(PrimitiveType& type, Utf8CP name) const;

    //! Get the extended type used to show or edit the property value 
    //! @param[out] extendedType Name of the extended type
    //! @param[in] name Name of the property
    //! @return true if an extended type was set. false otherwise. 
    ECOBJECTS_EXPORT bool GetExtendedType(Utf8StringR extendedType, Utf8CP name) const;

    //! Get the units of the property value
    //! @param[out] units String representing the units of the property
    //! @param[in] name Name of the property
    //! @return true if the read only attribute was set. false otherwise. 
    ECOBJECTS_EXPORT bool GetUnits(Utf8StringR units, Utf8CP name) const;

    //! Get the category for the property value (typically used for presentation)
    //! @param[out] category Name of the category
    //! @param[in] name Name of the property
    //! @return true if a category was set. false otherwise. 
    ECOBJECTS_EXPORT bool GetCategory(Utf8StringR category, Utf8CP name) const;

    //! Get the flag indicating if the property value should be hidden (or shown)
    //! @param[out] isHidden true if the property is to be hidden.
    //! @param[in] name Name of the property
    //! @return true if the hidden attribute was set. false otherwise. 
    ECOBJECTS_EXPORT bool GetHidden(bool& isHidden, Utf8CP name) const;

    //! Get the priority of the property value (typically used for presentation)
    //! @param[out] priority A number that indicates the the priority. @see PropertySortPriority
    //! @param[in] name Name of the property
    //! @return true if the priority was set. false otherwise. 
    ECOBJECTS_EXPORT bool GetPriority(int& priority, Utf8CP name) const;
    
    //! Get the flag indicating if the property value should be treated as read only.
    //! @param[out] isReadOnly true if the property should be read only.
    //! @param[in] name Name of the property
    //! @return true if the read only attribute was set. false otherwise. 
    ECOBJECTS_EXPORT bool GetReadOnly(bool& isReadOnly, Utf8CP name) const;
};

END_BENTLEY_ECOBJECT_NAMESPACE
