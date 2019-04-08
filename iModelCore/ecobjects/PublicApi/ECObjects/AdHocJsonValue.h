/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/AdHocJsonValue.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECObjects.h>
#include <json/value.h>
#include <json/writer.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
//! A Json::Value with methods to get/set values and attributes. 
//! Attributes are typically used to store units, type and other presentation information. 
// @bsiclass                                                 Ramanujam.Raman   02/16
//=======================================================================================
struct AdHocJsonValue : Json::Value
{
private:
    Utf8String Attributes(Utf8CP member) const {return Utf8String("$") + member;}
    static Utf8CP TYPE_FIELD_NAME() {return "Type";}
    static Utf8CP UNITS_FIELD_NAME() {return  "Units";}
    static Utf8CP CATEGORY_FIELD_NAME() {return  "Category";}
    static Utf8CP HIDDEN_FIELD_NAME() {return  "Hidden";}
    static Utf8CP READONLY_FIELD_NAME() {return  "ReadOnly";}
    static Utf8CP EXTENDEDTYPE_FIELD_NAME() {return  "ExtendedType";}
    static Utf8CP PRIORITY_FIELD_NAME() {return  "Priority";}

public:
    using Json::Value::Value;

    AdHocJsonValue& GetMemberR(Utf8CP member) {return (AdHocJsonValue&) (*this)[member];}
    AdHocJsonValue const& GetMember(Utf8CP member) const {return (AdHocJsonValue const&)(*this)[member];}
    JsonValueCR GetAttributes(Utf8CP member) const {return GetMember(Attributes(member).c_str());}
    void SetAttribute(Utf8CP member, Utf8CP attributeName, JsonValueCR value) {(*this)[Attributes(member)][attributeName] = value;}

    //! Set a Boolean value
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    void SetValueBoolean(Utf8CP member, bool value) {GetMemberR(member) = AdHocJsonValue(value); SetType(member, PRIMITIVETYPE_Boolean);}

    //! Set an Integer value
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    //! @param[in] units String representing the units of the value
    void SetValueInt(Utf8CP member, int32_t value, Utf8CP units = nullptr) {GetMemberR(member) = value;  SetType(member, PRIMITIVETYPE_Integer); if (units) SetUnits(member, units);}

    //! Set an Int64 value
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    void SetValueInt64(Utf8CP member, int64_t value) {GetMemberR(member) = AdHocJsonValue(value); SetType(member, PRIMITIVETYPE_Long);}

    //! Set a Double value
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    //! @param[in] units String representing the units of the value
    void SetValueDouble(Utf8CP member, double value, Utf8CP units = nullptr) {GetMemberR(member) = value; SetType(member, PRIMITIVETYPE_Double); if (units) SetUnits(member, units);}

    //! Set a Text value
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    void SetValueText(Utf8CP member, Utf8CP value) {GetMemberR(member) = value; SetType(member, PRIMITIVETYPE_String);}

    //! Set a DPoint2d value
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    void SetValuePoint2d(Utf8CP member, DPoint2dCR value) {auto& obj=GetMemberR(member); obj["x"] = value.x; obj["y"] = value.y; SetType(member, PRIMITIVETYPE_Point2d);}

    //! Set an DPoint3d value
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    void SetValuePoint3d(Utf8CP member, DPoint3dCR value) {auto& obj=GetMemberR(member); obj["x"] = value.x; obj["y"] = value.y; obj["z"] = value.z; SetType(member, PRIMITIVETYPE_Point3d);}

    //! Set a DateTime value
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    void SetValueDateTime(Utf8CP member, DateTimeCR value) {GetMemberR(member) = value.ToString(); SetType(member, PRIMITIVETYPE_DateTime);}

    //! Set a value from an ECValue (invariant)
    //! @param[in] member the name of the member
    //! @param[in] value Value of the property
    //! @return SUCCESS if the setting was successful. ERROR otherwise. 
    //! @remarks Wraps over the other primitive SetValue methods. 
    BentleyStatus SetValueEC(Utf8CP member, ECN::ECValueCR value)
        {
        if (!value.IsPrimitive())
            return ERROR;

        if (value.IsNull())
            {
            GetMemberR(member) = AdHocJsonValue();
            return SUCCESS;
            }

        PrimitiveType primitiveType = value.GetPrimitiveType();
        switch (primitiveType)
            {
            case PRIMITIVETYPE_Boolean:
                SetValueBoolean(member, value.GetBoolean());
                break;
            case PRIMITIVETYPE_DateTime:
                SetValueDateTime(member, value.GetDateTime());
                break;
            case PRIMITIVETYPE_Double:
                SetValueDouble(member, value.GetDouble());
                break;
            case PRIMITIVETYPE_Integer:
                SetValueInt(member, value.GetInteger());
                break;
            case PRIMITIVETYPE_Long:
                SetValueInt64(member, value.GetLong());
                break;
            case PRIMITIVETYPE_Point2d:
                SetValuePoint2d(member, value.GetPoint2d());
                break;
            case PRIMITIVETYPE_Point3d:
                SetValuePoint3d(member, value.GetPoint3d());
                break;
            case PRIMITIVETYPE_String:
                SetValueText(member, value.GetUtf8CP());
                break;
            default:
                BeAssert(false && "Cannot handle type");
                return ERROR;
            }

        return SUCCESS;
        }

    //! Remove the value
    void RemoveMember(Utf8CP member) {removeMember(member); removeMember(Attributes(member));}

    //! Get a DPoint2d value
    DPoint2d GetValuePoint2d(Utf8CP member) const {auto& obj = GetMember(member); return DPoint2d::From(obj["x"].asDouble(), obj["y"].asDouble());}

    //! Get a DPoint3d value
    DPoint3d GetValuePoint3d(Utf8CP member) const {auto& obj = GetMember(member); return DPoint3d::From(obj["x"].asDouble(), obj["y"].asDouble(), obj["z"].asDouble());}

    //! Get a DateTime value
    DateTime GetValueDateTime(Utf8CP member) const {DateTime value; DateTime::FromString(value, GetMember(member).asCString()); return value;}

    //! Get the value as an ECValue (invariant)
    ECValue GetValueEC(Utf8CP member) const 
        {
        auto& obj = GetMember(member);
        if (obj.isNull())
            return ECValue();

        switch (GetType(member))
            {
            case PRIMITIVETYPE_Boolean:
                return ECValue(obj.asBool());
            case PRIMITIVETYPE_DateTime:
                return ECValue(GetValueDateTime(member));
            case PRIMITIVETYPE_Double:
                return ECValue(obj.asDouble());
            case PRIMITIVETYPE_Integer:
                return ECValue(obj.asInt());
            case PRIMITIVETYPE_Long:
                return ECValue(obj.asInt64());
            case PRIMITIVETYPE_Point2d:
                return ECValue(GetValuePoint2d(member));
            case PRIMITIVETYPE_Point3d:
                return ECValue(GetValuePoint3d(member));
            case PRIMITIVETYPE_String:
                return ECValue(obj.asCString());
            default:
                return ECValue();
            }
        }

    void SetType(Utf8CP member, ECN::PrimitiveType primitiveType) {SetAttribute(member, TYPE_FIELD_NAME(), Json::Value((int) primitiveType));}

    //! Set the units of the property value
    //! @param[in] member the name of the member
    //! @param[in] units String representing the units of the property value
    void SetUnits(Utf8CP member, Utf8CP units) {SetAttribute(member, UNITS_FIELD_NAME(), units);}

    //! Set the extended type to be used to show or edit the property value
    //! @param[in] member the name of the member
    //! @param[in] extendedType Name of the extended type
    void SetExtendedType(Utf8CP member, Utf8CP extendedType) {SetAttribute(member, EXTENDEDTYPE_FIELD_NAME(), Json::Value(extendedType));}

    //! Set the category for the property value (typically used for presentation)
    //! @param[in] member the name of the member
    //! @param[in] category Name of the category
    void SetCategory(Utf8CP member, Utf8CP category) {SetAttribute(member, CATEGORY_FIELD_NAME(), Json::Value(category));}

    //! Set the property value to be hidden (or shown)
    //! @param[in] member the name of the member
    //! @param[in] hidden Pass true to hide the property.
    void SetHidden(Utf8CP member, bool hidden) {SetAttribute(member, HIDDEN_FIELD_NAME(), Json::Value(hidden));}

    //! Set the priority of the property value (typically used for presentation)
    //! @param[in] member the name of the member
    //! @param[in] priority Pass a number to indicate the priority. @see PropertySortPriority
    void SetPriority(Utf8CP member, int priority) {SetAttribute(member, PRIORITY_FIELD_NAME(), Json::Value(priority));}

    //! Set that the property value is to be treated as read only.
    //! @param[in] member the name of the member
    //! @param[in] isReadOnly Pass true to make the property read only.
    void SetReadOnly(Utf8CP member, bool isReadOnly) {SetAttribute(member, READONLY_FIELD_NAME(), Json::Value(isReadOnly));}

    //! Get the type of the property value
    PrimitiveType GetType(Utf8CP member) const {return (PrimitiveType) GetAttributes(member)[TYPE_FIELD_NAME()].asInt();}

    //! Get the extended type used to show or edit the property value 
    Utf8String GetExtendedType(Utf8CP member) const {return GetAttributes(member)[EXTENDEDTYPE_FIELD_NAME()].asString();}

    //! Get the units of the property value
    Utf8String GetUnits(Utf8CP member) const {return GetAttributes(member)[UNITS_FIELD_NAME()].asString();}

    //! Get the category for the property value (typically used for presentation)
    Utf8String GetCategory(Utf8CP member) const {return GetAttributes(member)[CATEGORY_FIELD_NAME()].asString();}

    //! Get the flag indicating if the property value should be hidden (or shown)
    bool GetHidden(Utf8CP member) const {return GetAttributes(member)[HIDDEN_FIELD_NAME()].asBool();}

    //! Get the priority of the property value (typically used for presentation)
    int GetPriority(Utf8CP member) const {return GetAttributes(member)[PRIORITY_FIELD_NAME()].asInt();}
    
    //! Get the flag indicating if the property value should be treated as read only.
    bool GetReadOnly(Utf8CP member) const {return GetAttributes(member)[READONLY_FIELD_NAME()].asBool();}

    BentleyStatus FromString(Utf8CP jsonStr) {return Json::Reader::Parse(jsonStr, *this) ? SUCCESS : ERROR;}

    Utf8String ToString() const {return Json::FastWriter::ToString(*this);}

    void From(Json::Value&& other) {*((Json::Value*)this) = other;}
};

typedef AdHocJsonValue& AdHocJsonValueR;
typedef AdHocJsonValue const& AdHocJsonValueCR;

END_BENTLEY_ECOBJECT_NAMESPACE
