/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <Bentley/BeAssert.h>
#include <Bentley/ValueFormat.h>
#include <GeomSerialization/GeomSerializationApi.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
enum ECValueStateFlags ENUM_UNDERLYING_TYPE(unsigned char)
    {
    ECVALUE_STATE_None = 0x00,
        ECVALUE_STATE_IsNull = 0x01,
        ECVALUE_STATE_IsReadOnly = 0x02,      // Really indicates that the property from which this came is readonly... not the value itself.
        ECVALUE_STATE_IsLoaded = 0x04,
        ECVALUE_STATE_AllowPointersIntoInstanceMemory = 0x08
    };

enum ECValueOwnedDataFlags ENUM_UNDERLYING_TYPE(unsigned char)
    {
    ECVALUE_DATA_Binary = 1 << 0,
        ECVALUE_DATA_Utf8 = 1 << 1,
        ECVALUE_DATA_Utf16 = 1 << 2,
#if !defined (_WIN32)
        ECVALUE_DATA_WChar = 1 << 3,
#endif
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isDataOwned(uint8_t const& flags, ECValueOwnedDataFlags flag) { return 0 != (flags & flag); }
static void setDataOwned(uint8_t& flags, ECValueOwnedDataFlags flag, bool owned) { flags = owned ? (flags | flag) : (flags & ~flag); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::SetWChar(WCharCP str, uint8_t& flags, bool owned)
    {
#if defined (_WIN32)
    SetUtf16((Utf16CP) str, flags, owned);
#else
    if (NULL == str)
        owned = false;

    setDataOwned(flags, ECVALUE_DATA_WChar, owned);
    m_wchar = owned ? BeStringUtilities::Wcsdup(str) : str;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::SetUtf16(Utf16CP str, uint8_t& flags, bool owned)
    {
    if (NULL == str)
        owned = false;

    setDataOwned(flags, ECVALUE_DATA_Utf16, owned);
    if (!owned)
        m_utf16 = str;
    else
        {
        size_t size = 1;    // null terminator
#if defined (_WIN32)
        size += wcslen((WCharCP) str);
#else
        while (0 != *(str++))
            ++size;

        str -= size;
#endif
        size *= sizeof(Utf16Char);
        m_utf16 = (Utf16CP) malloc(size);
        _Analysis_assume_(m_utf16 != nullptr);
        memcpy(const_cast<Utf16P>(m_utf16), str, size);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::SetUtf8(Utf8CP str, uint8_t& flags, bool owned)
    {
    if (NULL == str)
        owned = false;

    setDataOwned(flags, ECVALUE_DATA_Utf8, owned);
    m_utf8 = owned ? BeStringUtilities::Strdup(str) : str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::ConvertToUtf8(uint8_t& flags)
    {
    if (NULL == m_utf8)
        {
        Utf8String buf;
        if (NULL != m_utf16)
            BeStringUtilities::Utf16ToUtf8(buf, m_utf16);
#if !defined (_WIN32)
        else if (NULL != m_wchar)
            BeStringUtilities::WCharToUtf8(buf, m_wchar);
#endif
        m_utf8 = BeStringUtilities::Strdup(buf.c_str());
        setDataOwned(flags, ECVALUE_DATA_Utf8, NULL != m_utf8);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::ConvertToUtf16(uint8_t& flags)
    {
    if (NULL == m_utf16)
        {
        Utf16Buffer buf;
        if (NULL != m_utf8)
            {
            if (0 == *m_utf8)       // BeStringUtilities will give us back an empty buffer for an empty string...not what we want
                buf.push_back(0);
            else
                BeStringUtilities::Utf8ToUtf16(buf, m_utf8);
            }
#if !defined (_WIN32)
        else if (NULL != m_wchar)
            BeStringUtilities::WCharToUtf16(buf, m_wchar);
#endif

        if (!buf.empty())
            {
            size_t size = buf.size() * sizeof(Utf16Char);
            m_utf16 = (Utf16CP) malloc(size);
            _Analysis_assume_(m_utf16 != nullptr);
            memcpy(const_cast<Utf16P>(m_utf16), &buf[0], size);
            setDataOwned(flags, ECVALUE_DATA_Utf16, true);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::FreeAndClear(uint8_t& flags)
    {
    if (isDataOwned(flags, ECVALUE_DATA_Utf16))
        free(const_cast<Utf16P>(m_utf16));

    if (isDataOwned(flags, ECVALUE_DATA_Utf8))
        free(const_cast<Utf8P>(m_utf8));

#if !defined (_WIN32)
    if (isDataOwned(flags, ECVALUE_DATA_WChar))
        free(const_cast<WCharP>(m_wchar));
#endif

    flags = 0;
    SetNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::SetNull()
    {
    m_utf8 = NULL;
    m_utf16 = NULL;
#if !defined(_WIN32)
    m_wchar = NULL;
#endif
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-------
bool ECValue::StringInfo::IsUtf8() const
    {
    return m_utf8 != NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECValue::StringInfo::GetWChar(uint8_t& flags)
    {
#if defined (_WIN32)
    return (WCharCP) GetUtf16(flags);
#else
    if (NULL == m_wchar)
        {
        // ###TODO: Note we do a copy into the WString, and then another copy from WString to our buffer
        // Do we have reliable methods for determining the required number of bytes required to store the converted string which would allow us to avoid using WString?
        WString buf;
        if (NULL != m_utf8)
            BeStringUtilities::Utf8ToWChar(buf, m_utf8);
        else if (NULL != m_utf16)
            BeStringUtilities::Utf16ToWChar(buf, m_utf16);
        else
            return NULL;

        m_wchar = BeStringUtilities::Wcsdup(buf.c_str());
        setDataOwned(flags, ECVALUE_DATA_WChar, true);
        }

    return m_wchar;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECValue::StringInfo::GetUtf8(uint8_t& flags)
    {
    ConvertToUtf8(flags);  // if we already have Utf8 this does nothing
    return m_utf8;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf16CP ECValue::StringInfo::GetUtf16(uint8_t& flags)
    {
    ConvertToUtf16(flags); // if we already have Utf16 this does nothing
    return m_utf16;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::StringInfo::Equals(ECValue::StringInfo const& rhs, uint8_t& flags)
    {
    // We already know both strings are not null
    // Depending on the encodings held by each StringInfo we may need to perform conversion...might as well store the converted value while we're at it.
    if (NULL != rhs.m_utf8)
        {
        if (NULL != m_utf8)
            return 0 == strcmp(m_utf8, rhs.m_utf8);
        else
            {
            ConvertToUtf8(flags);
            return Equals(rhs, flags);
            }
        }
    else if (NULL != rhs.m_utf16)
        {
        if (NULL != m_utf16)
            return 0 == BeStringUtilities::CompareUtf16(m_utf16, rhs.m_utf16);
#if !defined (_WIN32)
        else if (NULL != m_wchar)
            return 0 == BeStringUtilities::CompareUtf16WChar(rhs.m_utf16, m_wchar);
#endif
        else
            {
            ConvertToUtf16(flags);
            return Equals(rhs, flags);
            }
        }
#if !defined (_WIN32)
    else if (NULL != rhs.m_wchar)
        {
        if (NULL != m_wchar)
            return 0 == wcscmp(m_wchar, rhs.m_wchar);
        else if (NULL != m_utf16)
            return 0 == BeStringUtilities::CompareUtf16WChar(m_utf16, rhs.m_wchar);
        else
            {
            ConvertToUtf16(flags);
            return Equals(rhs, flags);
            }
        }
#endif

    BeAssert(false && "It should not be possible to compare StringInfos where one or both contains all null strings");
    return false;
    }

//*********************** ECValue::DateTimeInfo ***************************************
//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-------
void ECValue::DateTimeInfo::Set(::int64_t ceTicks)
    {
    m_ceTicks = ceTicks;
    m_info = DateTime::Info();
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECValue::DateTimeInfo::Set(DateTimeCR dateTime)
    {
    //No support for local DateTimes (yet?) as client might expect this to do time zone
    //conversions - which we want the client / application side to do as it is nearly
    //impossible to do time zone conversions right in a generic and portable way.
    if (!dateTime.IsValid() || dateTime.GetInfo().GetKind() == DateTime::Kind::Local)
        return ERROR;

    uint64_t jd = INT64_C(0);
    if (SUCCESS != dateTime.ToJulianDay(jd))
        return ERROR;

    //ECValue holds datetime as CE hecto-nanoseconds
    const int64_t ceHns = DateTime::JulianDayToCommonEraMilliseconds(jd) * 10000;
    Set(ceHns);
    return SetMetadata(dateTime.GetInfo());
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECValue::DateTimeInfo::GetDateTime(DateTimeR dateTime) const
    {
    DateTime::Info info = IsMetadataSet() ? m_info : DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);
    //ECValue holds date time as CE hecto-nanoseconds
    uint64_t jdMsec = DateTime::CommonEraMillisecondsToJulianDay(m_ceTicks / 10000);
    return DateTime::FromJulianDay(dateTime, jdMsec, info);
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECValue::DateTimeInfo::SetMetadata(DateTime::Info const& caMetadata)
    {
    //DateTimeInfo::IsNull indicates whether the metadata is unset or not. Only if it is not unset, store anything
    //in the ECValue
    if (!caMetadata.IsValid())
        return SUCCESS;

    //No support for local DateTimes (yet?) as client might expect this to do time zone
    //conversions - which we want the client / application side to do as it is nearly
    //impossible to do time zone conversions right in a generic and portable way.
    if (caMetadata.GetKind() == DateTime::Kind::Local)
        {
        LOG.error("DateTime kind 'Local' not supported.");
        BeAssert(false && "DateTime kind 'Local' not supported.");
        return ERROR;
        }

    m_info = caMetadata;
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-------
bool ECValue::DateTimeInfo::MetadataMatches(DateTime::Info const& caDateTimeMetadata) const
    {
    //if ECValue doesn't have meta data (e.g. in case when SetDateTimeTicks was used to populate it),
    //no metadata check will be done. I.e. the CA metadata will implicitly become the CA of the ECValue.
    if (!IsMetadataSet() || !caDateTimeMetadata.IsValid() || m_info == caDateTimeMetadata)
        return true;

    // Date and DateAndTime are compatible
    if (m_info.GetComponent() == DateTime::Component::Date && caDateTimeMetadata.GetComponent() == DateTime::Component::DateAndTime)
        return true;

    if (caDateTimeMetadata.GetComponent() == DateTime::Component::Date && m_info.GetComponent() == DateTime::Component::DateAndTime)
        return true;

    return false;
    }


//*********************** ECValue::NavigationInfo ***************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECValue::NavigationInfo::SetRelationship(ECRelationshipClassCP relationshipClass)
    {
    m_isPointer = true;
    m_relClass = relationshipClass;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECValue::NavigationInfo::SetRelationship(ECClassId relationshipClassId)
    {
    m_isPointer = false;
    m_relClassId = relationshipClassId;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECRelationshipClassCP ECValue::NavigationInfo::GetRelationshipClass() const
    {
    if (!m_isPointer)
        return nullptr;
    return m_relClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECClassId ECValue::NavigationInfo::GetRelationshipClassId() const
    {
    if (m_isPointer)
        {
        if (m_relClass != nullptr && m_relClass->HasId())
            return m_relClass->GetId();

        return ECClassId();
        }

    return m_relClassId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECValue::NavigationInfo::Set(BeInt64Id id)
    {
    m_id = id;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECValue::NavigationInfo::Clear()
    {
    m_relClass = nullptr;
    m_id = BeInt64Id();
    m_isPointer = true;
    }

//*********************** ECValue ***************************************

/*---------------------------------------------------------------------------------**//**
*  Really indicates that the property from which this came is readonly... not the value itself.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetIsReadOnly(bool isReadOnly)
    {
    if (isReadOnly)
        m_stateFlags |= ((uint8_t) ECVALUE_STATE_IsReadOnly);
    else
        m_stateFlags &= ~((uint8_t) ECVALUE_STATE_IsReadOnly);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsReadOnly() const
    {
    return 0 != (m_stateFlags & ECVALUE_STATE_IsReadOnly);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetIsNull(bool isNull)
    {
    if (isNull)
        m_stateFlags |= ((uint8_t) ECVALUE_STATE_IsNull);
    else
        m_stateFlags &= ~((uint8_t) ECVALUE_STATE_IsNull);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsNull() const
    {
    return 0 != (m_stateFlags & ECVALUE_STATE_IsNull);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetIsLoaded(bool isLoaded)
    {
    if (isLoaded)
        m_stateFlags |= ((uint8_t) ECVALUE_STATE_IsLoaded);
    else
        m_stateFlags &= ~((uint8_t) ECVALUE_STATE_IsLoaded);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::AllowsPointersIntoInstanceMemory() const
    {
    return 0 != (m_stateFlags & ECVALUE_STATE_AllowPointersIntoInstanceMemory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::SetAllowsPointersIntoInstanceMemory(bool allow)
    {
    if (allow)
        m_stateFlags |= ((uint8_t) ECVALUE_STATE_AllowPointersIntoInstanceMemory);
    else
        m_stateFlags &= ~((uint8_t) ECVALUE_STATE_AllowPointersIntoInstanceMemory);
    }

/*---------------------------------------------------------------------------------**//**
* It is up to the instance implementation to set this flag. For MemoryBased instances
* this bit is set in the _GetValue method when it checks the IsLoaded flag for the
* property.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsLoaded() const
    {
    return 0 != (m_stateFlags & ECVALUE_STATE_IsLoaded);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValueKind       ECValue::GetKind() const
    {
    return (ValueKind) (m_valueKind & 0xFF);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsUninitialized() const
    {
    return GetKind() == VALUEKIND_Uninitialized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsString() const
    {
    return m_primitiveType == PRIMITIVETYPE_String;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool            ECValue::IsUtf8() const
    {
    return IsString() && m_stringInfo.IsUtf8();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsBinary() const
    {
    return m_primitiveType == PRIMITIVETYPE_Binary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsInteger() const
    {
    return m_primitiveType == PRIMITIVETYPE_Integer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsLong() const
    {
    return m_primitiveType == PRIMITIVETYPE_Long;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsDouble() const
    {
    return m_primitiveType == PRIMITIVETYPE_Double;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsBoolean() const
    {
    return m_primitiveType == PRIMITIVETYPE_Boolean;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsPoint2d() const
    {
    return m_primitiveType == PRIMITIVETYPE_Point2d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsPoint3d() const
    {
    return m_primitiveType == PRIMITIVETYPE_Point3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsDateTime() const
    {
    return m_primitiveType == PRIMITIVETYPE_DateTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsIGeometry() const
    {
    return m_primitiveType == PRIMITIVETYPE_IGeometry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsArray() const
    {
    return GetKind() == VALUEKIND_Array;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool            ECValue::IsNavigation() const
    {
    return GetKind() == VALUEKIND_Navigation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsStruct() const
    {
    return GetKind() == VALUEKIND_Struct;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsPrimitive() const
    {
    return GetKind() == VALUEKIND_Primitive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::ConstructUninitialized()
    {
#ifndef NDEBUG
    int size = sizeof(ECValue);
    memset(this, 0xBAADF00D, size); // avoid accidental misinterpretation of uninitialized data
#endif
    m_valueKind = VALUEKIND_Uninitialized;
    m_stateFlags = ECVALUE_STATE_IsNull;
    m_ownershipFlags = 0;
    }


/*---------------------------------------------------------------------------------**//**
* Copies this value object without allocating any additional memory.  The copy will
* hold copies on any external pointers held by the original.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::ShallowCopy(ECValueCR v)
    {
    if (this == &v)
        return;

    memcpy(this, &v, sizeof(ECValue));

    if (IsNull())
        return;

    unsigned short valueKind = m_valueKind;
    switch (valueKind)
        {
            case VALUEKIND_Struct:
            {
            m_structInstance->AddRef();
            break;
            }

            case PRIMITIVETYPE_Binary:
            {
            // Only make a copy of the binary data if the original object had a copy
            if (isDataOwned(m_ownershipFlags, ECVALUE_DATA_Binary))
                {
                setDataOwned(m_ownershipFlags, ECVALUE_DATA_Binary, false);
                SetBinary(v.m_binaryInfo.m_data, v.m_binaryInfo.m_size, true);
                }
            break;
            }

            case PRIMITIVETYPE_String:
            {
            // NB: Original comment: "Only make a copy of the string if the original object had a copy."
            // ^ We don't know the lifetime of the original object vs. that of 'this', so we must always copy.

            // ###TODO: only copy the preferred encoding? Copy all encodings contained in other StringInfo?
            m_ownershipFlags = 0; // prevent FreeAndClear() from attempting to free the strings that were temporarily copied by memset
            m_stringInfo.FreeAndClear(m_ownershipFlags);
            if (NULL != v.m_stringInfo.m_utf8)
                SetUtf8CP(v.m_stringInfo.m_utf8);
            else if (NULL != v.m_stringInfo.m_utf16)
                SetUtf16CP(v.m_stringInfo.m_utf16);
#if !defined (_WIN32)
            else if (NULL != v.m_stringInfo.m_wchar)
                SetWCharCP(v.m_stringInfo.m_wchar);
#endif
            else
                SetWCharCP(NULL);

            break;
            }

            // the memcpy takes care of these...
            case VALUEKIND_Array:
        case VALUEKIND_Navigation:
            case PRIMITIVETYPE_Boolean:
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_Long:
            case PRIMITIVETYPE_Double:
            case PRIMITIVETYPE_Point2d:
            case PRIMITIVETYPE_Point3d:
            case PRIMITIVETYPE_DateTime:
            case PRIMITIVETYPE_IGeometry:
                break;

            default:
                BeAssert(false); // type not handled
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::FreeMemory()
    {
    if (IsNull())
        return;

    unsigned short primitiveType = m_primitiveType;
    switch (primitiveType)
        {
            case PRIMITIVETYPE_String:
                m_stringInfo.FreeAndClear(m_ownershipFlags);
                return;

            case PRIMITIVETYPE_Binary:
            case PRIMITIVETYPE_IGeometry:
                if (NULL != m_binaryInfo.m_data && isDataOwned(m_ownershipFlags, ECVALUE_DATA_Binary))
                    free((void*) m_binaryInfo.m_data);
                return;

            case VALUEKIND_Struct:
                if ((m_structInstance != NULL))
                    m_structInstance->Release();
                return;

            case VALUEKIND_Navigation:
                m_navigationInfo.Clear();
                return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetToNull()
    {
    FreeMemory();
    SetIsNull(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::Clear()
    {
    uint8_t newStateFlags = AllowsPointersIntoInstanceMemory() ? (ECVALUE_STATE_IsNull | ECVALUE_STATE_AllowPointersIntoInstanceMemory) : ECVALUE_STATE_IsNull;

    if (IsNull())
        {
        m_valueKind = VALUEKIND_Uninitialized;
        m_stateFlags = newStateFlags;
        return;
        }

    if (IsUninitialized())
        {
        m_stateFlags = newStateFlags;
        return;
        }

    FreeMemory();
    m_stateFlags = newStateFlags;
    m_valueKind = VALUEKIND_Uninitialized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::~ECValue()
    {
    FreeMemory();
#ifndef NDEBUG
    memset(this, 0xBAADF00D, sizeof(ECValue)); // try to make use of destructed values more obvious
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueR ECValue::operator= (ECValueCR rhs)
    {
    From(rhs);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* Construct an uninitialized value
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue()
    {
    ConstructUninitialized();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(ECValueCR v)
    {
    ConstructUninitialized();
    From(v);
    }

/*---------------------------------------------------------------------------------**//**
*  Construct a Null ECN::ECValue (of a specific type, but with IsNull = true)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(ValueKind classification) : m_valueKind(classification), m_stateFlags(ECVALUE_STATE_IsNull), m_ownershipFlags(0)
    {}

/*---------------------------------------------------------------------------------**//**
*  Construct a Null ECN::ECValue (of a specific type, but with IsNull = true)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(PrimitiveType primitiveType) : m_primitiveType(primitiveType), m_stateFlags(ECVALUE_STATE_IsNull), m_ownershipFlags(0)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(::int32_t integer32)
    {
    ConstructUninitialized();
    SetInteger(integer32);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(::int64_t long64)
    {
    ConstructUninitialized();
    SetLong(long64);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(double doubleVal)
    {
    ConstructUninitialized();
    SetDouble(doubleVal);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(DPoint2dCR point2d)
    {
    ConstructUninitialized();
    SetPoint2d(point2d);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(DPoint3dCR point3d)
    {
    ConstructUninitialized();
    SetPoint3d(point3d);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(IGeometryCR geom)
    {
    ConstructUninitialized();
    SetIGeometry(geom);
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(bool value)
    {
    ConstructUninitialized();
    SetBoolean(value);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(DateTimeCR dateTime)
    {
    ConstructUninitialized();
    SetDateTime(dateTime);
    };

/*---------------------------------------------------------------------------------**//**
* @param holdADuplicate     If true, ECN::ECValue will make a duplicate, otherwise
* ECN::ECValue holds the original pointer. Intended only for use when initializing arrays of strings, to avoid duplicating them twice.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(WCharCP string, bool holdADuplicate)
    {
    ConstructUninitialized();
    SetWCharCP(string, holdADuplicate);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(Utf8CP string, bool holdADuplicate)
    {
    ConstructUninitialized();
    SetUtf8CP(string, holdADuplicate);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(Utf16CP string, bool holdADuplicate)
    {
    ConstructUninitialized();
    SetUtf16CP(string, holdADuplicate);
    };

/*---------------------------------------------------------------------------------**//**
* The ECValue constructed by this overload is not responsible for freeing the memory... its creator is.
* The consumer of the ECValue should make a copy of the memory.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue(const Byte * data, size_t size)
    {
    ConstructUninitialized();
    SetBinary(data, size);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECValue::ECValue (BeInt64Id value, ECRelationshipClassCP relationship)
    {
    ConstructUninitialized();
    SetNavigationInfo(value, relationship);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECValue::ECValue(BeInt64Id value, ECClassId relationshipClassId)
    {
    BeAssert(relationshipClassId.IsValid());

    ConstructUninitialized();
    SetNavigationInfo(value, relationshipClassId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECValue::From(ECValueCR v)
    {
    FreeMemory();
    ShallowCopy(v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType   ECValue::GetPrimitiveType() const
    {
    PRECONDITION(IsPrimitive() && "Tried to get the primitive type of an ECN::ECValue that is not classified as a primitive.", (PrimitiveType) 0);
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetPrimitiveType(PrimitiveType primitiveType)
    {
    if (m_primitiveType != primitiveType)
        {
        Clear();
        m_primitiveType = primitiveType;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
::int32_t       ECValue::GetInteger() const
    {
    PRECONDITION(IsInteger() && "Tried to get integer value from an ECN::ECValue that is not an integer.", 0);
    PRECONDITION(!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_integer32;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetInteger(::int32_t integer)
    {
    Clear();
    SetIsNull(false);
    m_primitiveType = PRIMITIVETYPE_Integer;
    m_integer32 = integer;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
::int64_t       ECValue::GetLong() const
    {
    PRECONDITION(IsLong() && "Tried to get long64 value from an ECN::ECValue that is not an long64.", 0);
    PRECONDITION(!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_long64;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetLong(::int64_t long64)
    {
    Clear();
    SetIsNull(false);
    m_primitiveType = PRIMITIVETYPE_Long;
    m_long64 = long64;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double          ECValue::GetDouble() const
    {
    PRECONDITION(IsDouble() && "Tried to get double value from an ECN::ECValue that is not an double.", std::numeric_limits<double>::quiet_NaN());
    PRECONDITION(!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", std::numeric_limits<double>::quiet_NaN());
    return m_double;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetDouble(double value)
    {
    Clear();
    SetIsNull(false);
    m_primitiveType = PRIMITIVETYPE_Double;
    m_double = value;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool          ECValue::GetBoolean() const
    {
    PRECONDITION(IsBoolean() && "Tried to get boolean value from an ECN::ECValue that is not a boolean.", 0);
    PRECONDITION(!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_boolean;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetBoolean(bool value)
    {
    Clear();
    SetIsNull(false);
    m_primitiveType = PRIMITIVETYPE_Boolean;
    m_boolean = value;

    return SUCCESS;
    }

//////////////////////////////////////////////////////////////////////////////////////////
// DateTime   - The DateTime ticks are the number of 100-nanosecond intervals
//              that have elapsed since the beginning of the Common Era epoch (0001-01-01 00:00:00 UTC)
//              (This is the same as in managed ECObjects and .NET's System.DateTime respectively)
//////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECValue::GetDateTimeTicks() const
    {
    PRECONDITION(IsDateTime() && "Tried to get DateTime value from an ECN::ECValue that is not a DateTime.", 0LL);
    PRECONDITION(!IsNull(), 0LL);
    return m_dateTimeInfo.GetCETicks();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ECValue::GetDateTimeTicks(DateTime::Info& metadata) const
    {
    if (m_dateTimeInfo.IsMetadataSet())
        metadata = m_dateTimeInfo.GetMetadata();

    return GetDateTimeTicks();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime ECValue::GetDateTime() const
    {
    PRECONDITION(IsDateTime() && "Tried to get DateTime value from an ECN::ECValue that is not a DateTime.", DateTime());
    PRECONDITION(!IsNull(), DateTime());

    DateTime dateTime;
    BentleyStatus stat = m_dateTimeInfo.GetDateTime(dateTime);
    POSTCONDITION(stat == SUCCESS, DateTime());

    return dateTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECValue::GetDateTimeUnixMillis() const
    {
    PRECONDITION(IsDateTime() && "Tried to get DateTime value from an ECN::ECValue that is not a DateTime.", 0LL);
    PRECONDITION(!IsNull(), 0LL);

    int64_t commonEraTicks = m_dateTimeInfo.GetCETicks();
    uint64_t jdInMsec = DateTime::CommonEraMillisecondsToJulianDay(commonEraTicks / 10000);
    return DateTime::JulianDayToUnixMilliseconds(jdInMsec);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus ECValue::SetDateTimeTicks(int64_t ceTicks, DateTime::Info const& dateTimeMetadata)
    {
    Clear();
    SetIsNull(false);
    m_primitiveType = PRIMITIVETYPE_DateTime;
    m_dateTimeInfo.Set(ceTicks);
    return m_dateTimeInfo.SetMetadata(dateTimeMetadata);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus ECValue::SetEnumeration(ECEnumerationCR enumeration, int32_t enumeratorValue)
    {
    if (!enumeration.GetIsStrict() || enumeration.FindEnumerator(enumeratorValue) != nullptr)
        {
        SetInteger(enumeratorValue);
        return BentleyStatus::SUCCESS;
        }
    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus ECValue::SetEnumeration(ECEnumerationCR enumeration, Utf8CP enumeratorValue)
    {
    if (!enumeration.GetIsStrict() || enumeration.FindEnumerator(enumeratorValue) != nullptr)
        {
        SetUtf8CP(enumeratorValue);
        return BentleyStatus::SUCCESS;
        }
    return BentleyStatus::ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus          ECValue::SetDateTime(DateTimeCR dateTime)
    {
    Clear();
    m_primitiveType = PRIMITIVETYPE_DateTime;
    //in case of error, keep IsNull set.
    BentleyStatus stat = m_dateTimeInfo.Set(dateTime);
    if (stat == SUCCESS)
        SetIsNull(false);

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           ECValue::SetLocalDateTimeFromUnixMillis(int64_t millis)
    {
    // Old PropertyEnablers would convert properties like file create date or element modified time to local time
    // We reluctantly reproduce that behavior to avoid regressions. Incoming time is UTC
    DateTime utc;
    if (SUCCESS == DateTime::FromUnixMilliseconds(utc, millis))
        {
        DateTime local;
        if (SUCCESS != utc.ToLocalTime(local))
            local = utc;
        else
            {
            // Uhh...we don't actually support local DateTime values? Yet we produce them? See assertion in ECValue::SetDateTime()
            // So convert the value back to "unspecified" DateTime.
            local = DateTime(DateTime::Kind::Unspecified, local.GetYear(), local.GetMonth(), local.GetDay(), local.GetHour(), local.GetMinute(), local.GetSecond(), local.GetMillisecond());
            }

        return SetDateTime(local);
        }
    else
        {
        Clear();
        m_primitiveType = PRIMITIVETYPE_DateTime;
        return ERROR;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus ECValue::SetDateTimeMetadata(DateTime::Info const& dateTimeInfo)
    {
    PRECONDITION(IsDateTime() && !IsNull(), ERROR);
    return m_dateTimeInfo.SetMetadata(dateTimeInfo);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
bool ECValue::IsDateTimeMetadataSet() const
    {
    return IsDateTime() && !IsNull() && m_dateTimeInfo.IsMetadataSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
bool ECValue::DateTimeInfoMatches(DateTime::Info const& dateTimeInfo) const
    {
    return IsDateTime() && !IsNull() && m_dateTimeInfo.MetadataMatches(dateTimeInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d          ECValue::GetPoint2d() const
    {
    DPoint2d badValue = {0.0,0.0};
    PRECONDITION(IsPoint2d() && "Tried to get Point2d value from an ECN::ECValue that is not a Point2d.", badValue);
    PRECONDITION(!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", badValue);
    return m_dPoint2d;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetPoint2d(DPoint2dCR value)
    {
    Clear();
    SetIsNull(false);
    m_primitiveType = PRIMITIVETYPE_Point2d;
    m_dPoint2d = value;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d          ECValue::GetPoint3d() const
    {
    DPoint3d badValue = {0.0,0.0,0.0};

    PRECONDITION(IsPoint3d() && "Tried to get Point3d value from an ECN::ECValue that is not a Point3d.", badValue);
    PRECONDITION(!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", badValue);
    return m_dPoint3d;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetPoint3d(DPoint3dCR value)
    {
    Clear();
    SetIsNull(false);
    m_primitiveType = PRIMITIVETYPE_Point3d;
    m_dPoint3d = value;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECValue::GetWCharCP() const
    {
    PRECONDITION(IsString() && "Tried to get string value from an ECN::ECValue that is not a string.", L"<Programmer Error: Attempted to get string value from ECN::ECValue that is not a string.>");
    return IsNull() ? NULL : m_stringInfo.GetWChar(m_ownershipFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECValue::OwnsWCharCP() const
    {
    if (!IsString())
        {
        BeAssert(false && L"<Programmer Error: Attempted to call string related method from ECN::ECValue that is not a string.>");
        return false;
        }

    if (IsNull())
        return false;


#if defined (_WIN32)
    const ECValueOwnedDataFlags flags = ECVALUE_DATA_Utf16;
#else
    const ECValueOwnedDataFlags flags = ECVALUE_DATA_WChar;
#endif

    return isDataOwned(m_ownershipFlags, flags);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP  ECValue::GetUtf8CP() const
    {
    if (!IsString())
        {
        BeAssert(false && "<Programmer Error: Attempted to get string value from ECN::ECValue that is not a string.>");
        return NULL;
        }

    return IsNull() ? NULL : m_stringInfo.GetUtf8(m_ownershipFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECValue::OwnsUtf8CP() const
    {
    if (!IsString())
        {
        BeAssert(false && "<Programmer Error: Attempted to call string related method from ECN::ECValue that is not a string.>");
        return false;
        }

    return IsNull() ? false : isDataOwned(m_ownershipFlags, ECVALUE_DATA_Utf8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf16CP ECValue::GetUtf16CP() const
    {
    if (!IsString())
        {
        BeAssert(false && "<Programmer Error: Attempted to get string value from ECN::ECValue that is not a string.>");
        return NULL;
        }

    return IsNull() ? NULL : m_stringInfo.GetUtf16(m_ownershipFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECValue::OwnsUtf16CP() const
    {
    if (!IsString())
        {
        BeAssert(false && "<Programmer Error: Attempted to call string related method from ECN::ECValue that is not a string.>");
        return false;
        }

    return IsNull() ? false : isDataOwned(m_ownershipFlags, ECVALUE_DATA_Utf16);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::InitForString(void const * str)
    {
    Clear();
    m_stringInfo.SetNull();
    m_primitiveType = PRIMITIVETYPE_String;
    SetIsNull(NULL == str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetWCharCP(WCharCP string, bool holdADuplicate)
    {
    InitForString(string);
    m_stringInfo.SetWChar(string, m_ownershipFlags, holdADuplicate);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetUtf8CP(Utf8CP string, bool holdADuplicate)
    {
    InitForString(string);
    m_stringInfo.SetUtf8(string, m_ownershipFlags, holdADuplicate);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetUtf16CP(Utf16CP string, bool holdADuplicate)
    {
    InitForString(string);
    m_stringInfo.SetUtf16(string, m_ownershipFlags, holdADuplicate);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ECValue::GetIGeometry() const
    {
    PRECONDITION(IsIGeometry() && "Tried to get an IGeometry from an ECN::ECValue that is not geometry", NULL);
    if (IsNull())
        return nullptr;

    bvector<Byte> geomBuffer;
    geomBuffer.resize(m_binaryInfo.m_size);
    memcpy(&geomBuffer[0], m_binaryInfo.m_data, m_binaryInfo.m_size);

    return BentleyGeometryFlatBuffer::BytesToGeometry(geomBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const Byte * ECValue::GetIGeometry(size_t& size) const
    {
    PRECONDITION(IsIGeometry() && "Tried to get binarydata from an ECN::ECValue that is not binary.", NULL);
    if (IsNull())
        return nullptr;
    size = m_binaryInfo.m_size;
    return m_binaryInfo.m_data;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetIGeometry(const Byte * data, size_t size, bool holdADuplicate)
    {
    Clear();
    if (0 == size)
        return SUCCESS;

    m_primitiveType = PRIMITIVETYPE_IGeometry;

    if (!BentleyGeometryFlatBuffer::IsFlatBufferFormat(data))
        return ERROR;

    return SetBinaryInternal(data, size, holdADuplicate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetIGeometry(IGeometryCR geometry)
    {
    Clear();
    m_primitiveType = PRIMITIVETYPE_IGeometry;
    bvector<Byte> buffer;
    BentleyGeometryFlatBuffer::GeometryToBytes(geometry, buffer);
    return SetBinaryInternal(buffer.data(), buffer.size(), true);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const Byte * ECValue::GetBinary(size_t& size) const
    {
    PRECONDITION((IsBinary() || IsIGeometry()) && "Tried to get binarydata from an ECN::ECValue that is not binary.", NULL);
    if (IsNull())
        return nullptr;
    size = m_binaryInfo.m_size;
    return m_binaryInfo.m_data;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetBinary(const Byte * data, size_t size, bool holdADuplicate)
    {
    Clear();

    m_primitiveType = PRIMITIVETYPE_Binary;
    return SetBinaryInternal(data, size, holdADuplicate);
    }

BentleyStatus ECValue::SetBinaryInternal(const Byte * data, size_t size, bool holdADuplicate)
    {
    setDataOwned(m_ownershipFlags, ECVALUE_DATA_Binary, holdADuplicate);

    if (NULL == data)
        {
        m_binaryInfo.m_data = NULL;
        m_binaryInfo.m_size = 0;
        return SUCCESS;
        }


    SetIsNull(false);

    m_binaryInfo.m_size = size;
    if (holdADuplicate)
        {
        void * duplicateData = malloc(size);
        _Analysis_assume_(duplicateData != nullptr);
        memcpy(duplicateData, data, size);
        m_binaryInfo.m_data = (const Byte *) duplicateData;
        }
    else
        m_binaryInfo.m_data = data;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  ECValue::GetStruct() const
    {
    PRECONDITION(IsStruct() && "Tried to get struct value from an ECN::ECValue that is not a struct.", 0);
    return m_structInstance;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetStruct(IECInstanceP structInstance)
    {
    Clear();

    m_valueKind = VALUEKIND_Struct;

    if (NULL == structInstance)
        {
        m_structInstance = NULL;
        return SUCCESS;
        }

    SetIsNull(false);

    m_structInstance = structInstance;
    m_structInstance->AddRef();

    return SUCCESS;
    }

extern void convertByteArrayToString(Utf8StringR outString, const Byte *byteData, size_t numBytes);
extern bool convertStringToByteArray(bvector<Byte>& byteData, Utf8CP stringData);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECValue::ConvertPrimitiveToString(Utf8StringR str) const
    {
    PRECONDITION(IsPrimitive() && "ECValue::ConvertPrimitiveToString() requires a primitive value", false);
    if (!IsPrimitive())
        return false;

    str.clear();
    if (IsNull())
        return true;

    switch (GetPrimitiveType())
        {
            case PRIMITIVETYPE_Binary:
            case PRIMITIVETYPE_IGeometry:
            {
            size_t nBytes;
            Byte const* bytes = GetBinary(nBytes);
            if (NULL != bytes)
                convertByteArrayToString(str, bytes, nBytes);
            }
            break;
            case PRIMITIVETYPE_Boolean:
                str = GetBoolean() ? "True" : "False";
                break;
            case PRIMITIVETYPE_DateTime:
                str.Sprintf("%" PRId64, GetDateTimeTicks());
                break;
            case PRIMITIVETYPE_Double:
                str.Sprintf("%.17g", GetDouble());
                break;
            case PRIMITIVETYPE_Integer:
                str.Sprintf("%" PRId32, GetInteger());
                break;
            case PRIMITIVETYPE_Long:
                str.Sprintf("%" PRId64, GetLong());
                break;
            case PRIMITIVETYPE_Point2d:
            {
            DPoint2d pt = GetPoint2d();
            str.Sprintf("%.17g,%.17g", pt.x, pt.y);
            }
            break;
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d pt = GetPoint3d();
            str.Sprintf("%.17g,%.17g,%.17g", pt.x, pt.y, pt.z);
            }
            break;
            case PRIMITIVETYPE_String:
                str = GetUtf8CP();
                break;
            default:
                BeAssert(false);
                return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String formatDouble(double d)
    {
    Utf8String str;
    str.Sprintf("%.17f", d);
    auto dotPos = str.find('.');
    if (Utf8String::npos != dotPos)
        {
        auto nonZeroPos = str.length() - 1;
        while (nonZeroPos > dotPos && str[nonZeroPos] == '0')
            --nonZeroPos;

        if (nonZeroPos == dotPos)
            //We need to keep ".0" at the end to make sure we keep information about the value type,
            //this will be needed during value parsing.
            str.erase(dotPos + 2, Utf8String::npos);
        else if (nonZeroPos < str.length() - 1)
            str.erase(nonZeroPos + 1, Utf8String::npos);
        }

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::ConvertPrimitiveToECExpressionLiteral(Utf8StringR expr) const
    {
    if (IsNull())
        {
        expr = "Null";
        return true;
        }

    PRECONDITION(IsPrimitive() && "ECValue::ConvertPrimitiveToECExpressionLiteral requires a primitive value", false);

    switch (GetPrimitiveType())
        {
            case PRIMITIVETYPE_Boolean:     expr = GetBoolean() ? "True" : "False"; return true;
            case PRIMITIVETYPE_DateTime:    expr.Sprintf("@%" PRId64, GetDateTimeTicks()); return true;
            case PRIMITIVETYPE_Double:      expr = formatDouble(GetDouble()); return true;
            case PRIMITIVETYPE_Integer:     expr.Sprintf("%" PRId32, GetInteger()); return true;
            case PRIMITIVETYPE_Long:        expr.Sprintf("%" PRId64, GetLong()); return true;
            case PRIMITIVETYPE_Point2d:     expr.Sprintf("{%s,%s}", formatDouble(GetPoint2d().x).c_str(), formatDouble(GetPoint2d().y).c_str()); return true;
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d pt = GetPoint3d();
            expr.Sprintf("{%s,%s,%s}", formatDouble(pt.x).c_str(), formatDouble(pt.y).c_str(), formatDouble(pt.z).c_str());
            }
            return true;
            case PRIMITIVETYPE_String:
            {
            // Must escape quotes...
            Utf8String s(GetUtf8CP());
            s.ReplaceAll("\"", "\"\"");
            expr.Sprintf("\"%s\"", s.c_str());
            }
            return true;
            case PRIMITIVETYPE_Binary:
            case PRIMITIVETYPE_IGeometry:
                return false;
            default:
                BeAssert(false && "Unsupported PrimitiveType");
                return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::ConvertToPrimitiveFromString(PrimitiveType primitiveType)
    {
    if (!IsString())
        {
        BeAssert(false);   // It's a private method, ought to have checked preconditions...
        return false;
        }
    else if (PRIMITIVETYPE_String == primitiveType)
        return true;
    else if (IsNull())
        {
        SetPrimitiveType(primitiveType);
        return true;
        }

    Utf8CP str = GetUtf8CP();
    switch (primitiveType)
        {
            case PRIMITIVETYPE_Binary:
            case PRIMITIVETYPE_IGeometry:
            {
            bvector<Byte> bytes;
            if (!convertStringToByteArray(bytes, str))
                return false;

            SetBinary(&bytes.front(), bytes.size(), true);
            }
            break;
            case PRIMITIVETYPE_Boolean:
                if (0 == BeStringUtilities::StricmpAscii("true", str) || 0 == strcmp("1", str))
                    SetBoolean(true);
                else if (0 == BeStringUtilities::StricmpAscii("false", str) || 0 == strcmp("0", str))
                    SetBoolean(false);
                else
                    return false;
                break;
            case PRIMITIVETYPE_DateTime:
            case PRIMITIVETYPE_Long:
            {
            int64_t i;
            if (1 != Utf8String::Sscanf_safe(str, "%" PRId64, &i))
                return false;
            else if (PRIMITIVETYPE_Long == primitiveType)
                SetLong(i);
            else
                SetDateTimeTicks(i);
            }
            break;
            case PRIMITIVETYPE_Double:
            {
            double d;
            if (1 == Utf8String::Sscanf_safe(str, "%lg", &d))
                SetDouble(d);
            else
                return false;
            }
            break;
            case PRIMITIVETYPE_Integer:
            {
            int64_t i;
            if (1 == Utf8String::Sscanf_safe(str, "%" PRId64, &i))
                {
                if (INT_MAX >= i && INT_MIN <= i)
                    SetInteger((int32_t) i);
                else
                    return false;
                }
            else
                return false;
            }
            break;
            case PRIMITIVETYPE_Point2d:
            {
            DPoint2d pt;
            if (2 == Utf8String::Sscanf_safe(str, "%lg,%lg", &pt.x, &pt.y))
                SetPoint2d(pt);
            else
                return false;
            }
            break;
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d pt;
            if (3 == Utf8String::Sscanf_safe(str, "%lg,%lg,%lg", &pt.x, &pt.y, &pt.z))
                SetPoint3d(pt);
            else
                return false;
            }
            break;
            default:
                BeAssert(false);
                return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String    ECValue::ToString() const
    {
    if (IsNull())
        return "<null>";

    Utf8String str;

    if (IsArray())
        {
        ArrayInfo arrayInfo = GetArrayInfo();
        str.Sprintf("Count: %" PRIu32 " IsFixedSize: %d", arrayInfo.GetCount(), arrayInfo.IsFixedCount());
        }
    else if (IsStruct())
        {
        return "IECInstance containing struct value";
        }
    else if (IsNavigation())
        {
        str.Sprintf("Id: %" PRIu64, m_navigationInfo.GetId<BeInt64Id>().GetValue());
        }
    else if (PRIMITIVETYPE_DateTime == m_primitiveType)
        str = GetDateTime().ToString(); // want something more readable than the ticks
    else if (!ConvertPrimitiveToString(str))
        str = "<error>";

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::CanConvertToPrimitiveType(PrimitiveType type) const
    {
    ECValue v(*this);
    return v.ConvertToPrimitiveType(type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::ConvertToPrimitiveType(PrimitiveType newType)
    {
    if (IsNull())
        {
        SetPrimitiveType(newType);
        return true;
        }
    else if (!IsPrimitive())
        return false;
    else if (newType == GetPrimitiveType())
        return true;
    else if (PRIMITIVETYPE_String == newType)
        {
        Utf8String strVal = ToString();
        SetUtf8CP(strVal.c_str());
        return true;
        }
    else if (IsString())
        return ConvertToPrimitiveFromString(newType);

    PrimitiveType curType = GetPrimitiveType();
    switch (newType)
        {
            case PRIMITIVETYPE_Integer:
            {
            int32_t i;
            if (PRIMITIVETYPE_Double == curType)
                {
                double roundingTerm = DoubleOps::AlmostEqual(GetDouble(), 0.0) ? 0.0 : GetDouble() > 0.0 ? 0.5 : -0.5;
                i = (int32_t) (GetDouble() + roundingTerm);
                }
            else if (PRIMITIVETYPE_Boolean == curType)
                i = GetBoolean() ? 1 : 0;
            else if (PRIMITIVETYPE_Long == curType)
                {
                if (INT_MAX >= GetLong() && INT_MIN <= GetLong())
                    i = (int32_t) GetLong();
                else
                    return false;
                }
            else
                return false;

            SetInteger(i);
            }
            return true;
            case PRIMITIVETYPE_Long:
            {
            int64_t i;
            if (PRIMITIVETYPE_Double == curType)
                {
                double roundingTerm = DoubleOps::AlmostEqual(GetDouble(), 0.0) ? 0.0 : GetDouble() > 0.0 ? 0.5 : -0.5;
                i = (int64_t) (GetDouble() + roundingTerm);
                }
            else if (PRIMITIVETYPE_Boolean == curType)
                i = GetBoolean() ? 1 : 0;
            else if (PRIMITIVETYPE_Integer == curType)
                i = (int64_t) GetInteger();
            else
                return false;

            SetLong(i);
            }
            return true;
            case PRIMITIVETYPE_Double:
            {
            double d;
            if (PRIMITIVETYPE_Integer == curType)
                d = GetInteger();
            else if (PRIMITIVETYPE_Long == curType)
                d = static_cast<double>(GetLong());     // warning C4244: '=' : conversion from 'Int64' to 'double', possible loss of data
            else if (PRIMITIVETYPE_Boolean == curType)
                d = GetBoolean() ? 1.0 : 0.0;
            else
                return false;

            SetDouble(d);
            }
            return true;
            case PRIMITIVETYPE_Boolean:
            {
            bool b;
            if (PRIMITIVETYPE_Integer == curType)
                b = GetInteger() != 0;
            else if (PRIMITIVETYPE_Long == curType)
                b = GetLong() != 0;
            else if (PRIMITIVETYPE_Double == curType)
                b = !DoubleOps::AlmostEqual(GetDouble(), 0.0);
            else
                return false;

            SetBoolean(b);
            }
            return true;
            case PRIMITIVETYPE_Point3d:
                if (PRIMITIVETYPE_Point2d == curType)
                    {
                    SetPoint3d(DPoint3d::FromXYZ(GetPoint2d().x, GetPoint2d().y, 0.0));
                    return true;
                    }
                return false;
            case PRIMITIVETYPE_Point2d:
                if (PRIMITIVETYPE_Point3d == curType)
                    {
                    SetPoint2d(DPoint2d::From(GetPoint3d().x, GetPoint3d().y));
                    return true;
                    }
                return false;
            default:
                return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  Performs a binary comparison against another ECValue.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool              ECValue::Equals(ECValueCR v) const
    {
    if (IsNull() != v.IsNull())
        return false;
    if (IsNull())
        return true;
    if (GetKind() != v.GetKind())
        return false;
    if (IsArray())
        {
        if (m_arrayInfo.GetCount() != v.m_arrayInfo.GetCount())
            return false;

        if (m_arrayInfo.IsFixedCount() != v.m_arrayInfo.IsFixedCount())
            return false;

        if (m_arrayInfo.GetKind() != v.m_arrayInfo.GetKind())
            return false;

        if (m_arrayInfo.IsPrimitiveArray() && m_arrayInfo.GetElementPrimitiveType() != v.m_arrayInfo.GetElementPrimitiveType())
            return false;

        return true;
        }
    if (IsStruct())
        return m_structInstance == v.m_structInstance;
    if (IsNavigation())
        {
        if (m_navigationInfo.GetId<BeInt64Id>() != v.m_navigationInfo.GetId<BeInt64Id>())
            return false;

        if (m_navigationInfo.GetRelationshipClass() != v.m_navigationInfo.GetRelationshipClass())
            return false;

        if (m_navigationInfo.GetRelationshipClassId() != v.m_navigationInfo.GetRelationshipClassId())
            return false;

        return true;
        }
    if (GetPrimitiveType() != v.GetPrimitiveType())
        return false;
    if (IsString())
        return m_stringInfo.Equals(v.m_stringInfo, m_ownershipFlags);
    if (IsBinary() || IsIGeometry())
        {
        if (m_binaryInfo.m_size != v.m_binaryInfo.m_size)
            return false;
        if (m_binaryInfo.m_data == v.m_binaryInfo.m_data)
            return true;
        return 0 == memcmp(m_binaryInfo.m_data, v.m_binaryInfo.m_data, m_binaryInfo.m_size);
        }
    if (IsDouble())
        return DoubleOps::AlmostEqual(GetDouble(), v.GetDouble());
    if (IsPoint3d())
        return DPoint3dOps::AlmostEqual(GetPoint3d(), v.GetPoint3d());

    if (IsPoint2d())
        return DPoint2dOps::AlmostEqual(GetPoint2d(), v.GetPoint2d());

    size_t primitiveValueSize = (size_t) GetFixedPrimitiveValueSize(GetPrimitiveType());
    //&m_boolean points to the first memory address of the union (as does every other union member)
    return 0 == memcmp(&m_boolean, &v.m_boolean, primitiveValueSize);
    }

/*---------------------------------------------------------------------------------**//**
* @param[in]        capacity Estimated size of the array.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus   ECValue::SetStructArrayInfo(uint32_t count, bool isFixedCount)
    {
    Clear();

    m_valueKind = VALUEKIND_Array;

    m_arrayInfo.InitializeStructArray(count, isFixedCount);

    SetIsNull(false); // arrays are never null

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @param[in]        capacity Estimated size of the array.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECValue::SetPrimitiveArrayInfo(PrimitiveType primitiveElementType, uint32_t count, bool isFixedSize)
    {
    Clear();

    m_valueKind = VALUEKIND_Array;

    m_arrayInfo.InitializePrimitiveArray(primitiveElementType, count, isFixedSize);

    SetIsNull(false); // arrays are never null

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayInfo       ECValue::GetArrayInfo() const
    {
    BeAssert(IsArray());

    return m_arrayInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        ECValue::GetFixedPrimitiveValueSize(PrimitiveType primitivetype)
    {
    switch (primitivetype)
        {
            case ECN::PRIMITIVETYPE_Integer:
                return sizeof(int32_t);
            case ECN::PRIMITIVETYPE_Long:
                return sizeof(int64_t);
            case ECN::PRIMITIVETYPE_Double:
                return sizeof(double);
            case PRIMITIVETYPE_Boolean:
                return sizeof(bool);
            case PRIMITIVETYPE_Point2d:
                return 2 * sizeof(double);
            case PRIMITIVETYPE_Point3d:
                return 3 * sizeof(double);
            case PRIMITIVETYPE_DateTime:
                return sizeof(int64_t); //ticks
            default:
                DEBUG_FAIL("Most datatypes have not yet been implemented... or perhaps you have passed in a variable-sized type.");
                return 0;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t        ECValue::GetNavigationValueSize (PrimitiveType primitiveType)
    {
    // A navigation value has 3 parts:
    // 1. a single bit that tells whether a pointer or id is used to identify the relationship
    // 2. an int64_t that contains either a pointer or id identifying the relationship used to set the nav value
    // 3. The actual navigation value
    uint32_t size = sizeof(int64_t);
    size += GetFixedPrimitiveValueSize(primitiveType);
    size += 1;
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArrayInfo::InitializeStructArray(uint32_t count, bool isFixedCount)
    {
    m_arrayKind = ARRAYKIND_Struct;
    m_count = count;
    m_isFixedCount = isFixedCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArrayInfo::InitializePrimitiveArray(PrimitiveType elementPrimitiveType, uint32_t count, bool isFixedCount)
    {
    m_elementPrimitiveType = elementPrimitiveType;
    m_count = count;
    m_isFixedCount = isFixedCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        ArrayInfo::GetCount() const
    {
    return m_count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ArrayInfo::IsFixedCount() const
    {
    return m_isFixedCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValueKind       ArrayInfo::GetKind() const
    {
    return (ValueKind) (m_arrayKind & 0xFF);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType   ArrayInfo::GetElementPrimitiveType() const
    {
    PRECONDITION(IsPrimitiveArray() && "Tried to get the element primitive type of an ArrayInfo that is not classified as a primitive array.", (PrimitiveType) 0);
    return m_elementPrimitiveType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ArrayInfo::IsPrimitiveArray() const
    {
    return GetKind() == VALUEKIND_Primitive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ArrayInfo::IsStructArray() const
    {
    return GetKind() == VALUEKIND_Struct;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECValue::SetNavigationInfo()
    {
    if (IsNull())
        m_navigationInfo.Clear();

    Clear();
    SetIsNull(true);
    m_valueKind = VALUEKIND_Navigation;

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECValue::SetNavigationInfo(BeInt64Id value, ECRelationshipClassCP relationshipClass)
    {
    Clear();
    SetIsNull(false);

    m_valueKind = VALUEKIND_Navigation;

    m_navigationInfo.Set(value);
    m_navigationInfo.SetRelationship(relationshipClass);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECValue::SetNavigationInfo(BeInt64Id value, ECClassId relationshipClassId)
    {
    BeAssert(relationshipClassId.IsValid() || !value.IsValid());

    Clear();
    SetIsNull(false);

    m_valueKind = VALUEKIND_Navigation;

    m_navigationInfo.Set(value);
    m_navigationInfo.SetRelationship(relationshipClassId);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECValue::NavigationInfo const& ECValue::GetNavigationInfo() const
    {
    BeAssert(IsNavigation());

    return m_navigationInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor(IECInstanceCR instance, int newPropertyIndex, int newArrayIndex) : m_isAdHoc(false)
    {
    PushLocation(instance, newPropertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor(ECEnablerCR enabler, int newPropertyIndex, int newArrayIndex) : m_isAdHoc(false)
    {
    PushLocation(enabler, newPropertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor(ECValueAccessorCR accessor)
    : m_locationVector(accessor.GetLocationVector()), m_isAdHoc(accessor.IsAdHocProperty())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValueAccessor::Clone(ECValueAccessorCR accessor)
    {
    m_locationVector.clear();

    for (ECValueAccessor::Location const & location : accessor.GetLocationVectorCR())
        PushLocation(*location.GetEnabler(), location.GetPropertyIndex(), location.GetArrayIndex());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::LocationVector const &   ECValueAccessor::GetLocationVectorCR() const
    {
    return m_locationVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const ECValueAccessor::LocationVector&          ECValueAccessor::GetLocationVector() const
    {
    return m_locationVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnablerCR                                     ECValueAccessor::GetEnabler(uint32_t depth) const
    {
    return *m_locationVector[depth].GetEnabler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::Location&                      ECValueAccessor::operator[] (uint32_t depth)
    {
    return m_locationVector[depth];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const ECValueAccessor::Location&                ECValueAccessor::operator[] (uint32_t depth) const
    {
    return m_locationVector[depth];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::PushLocation(ECEnablerCR newEnabler, int newPropertyIndex, int newArrayIndex)
    {
    m_locationVector.push_back(Location(&newEnabler, newPropertyIndex, newArrayIndex));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::PushLocation(IECInstanceCR instance, int newPropertyIndex, int newArrayIndex)
    {
    PushLocation(instance.GetEnabler(), newPropertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                            ECValueAccessor::PushLocation(ECEnablerCR newEnabler, Utf8CP accessString, int newArrayIndex)
    {
    uint32_t propertyIndex;
    ECObjectsStatus status = newEnabler.GetPropertyIndex(propertyIndex, accessString);
    if (ECObjectsStatus::Success != status)
        {
        //BeAssert (false && "Could not resolve property index for this access string");
        return false;
        }
    PushLocation(newEnabler, (int) propertyIndex, newArrayIndex);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                            ECValueAccessor::PushLocation(IECInstanceCR instance, Utf8CP accessString, int newArrayIndex)
    {
    return PushLocation(instance.GetEnabler(), accessString, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::PopLocation()
    {
    m_locationVector.pop_back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::Clear()
    {
    m_locationVector.clear();
    m_isAdHoc = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::Location&                      ECValueAccessor::DeepestLocation()
    {
    return m_locationVector.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::Location const&  ECValueAccessor::DeepestLocationCR() const
    {
    return m_locationVector.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t                                        ECValueAccessor::GetDepth() const
    {
    return (uint32_t) m_locationVector.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP                                 ECValueAccessor::GetAccessString() const
    {
    uint32_t depth = GetDepth();
    if (0 == depth)
        return NULL;

    return GetAccessString(depth - 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP                                 ECValueAccessor::GetAccessString(uint32_t depth) const
    {
    return GetAccessString(depth, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP                                 ECValueAccessor::GetAccessString(uint32_t depth, bool alwaysIncludeParentStructAccessStrings) const
    {
    int propertyIndex = m_locationVector[depth].GetPropertyIndex();
    ECEnablerCR enabler = *m_locationVector[depth].GetEnabler();
    Utf8CP accessString;
    if (ECObjectsStatus::Success == enabler.GetAccessString(accessString, propertyIndex))
        {
        // Embedded structs are weird...e.g., for "OuterStruct.InnerStruct.Property":
        //  - ECValuesCollection::Create() will create separate Location for each portion of the access string ("OuterStruct", "InnerStruct", "Property")
        //  - ECValueAccessor::PopulateValueAccessor() will create one Location for the entire access string
        // In the former case, need to strip off the struct prefix(es)
        if (!alwaysIncludeParentStructAccessStrings)
            {
            Location const* prev = depth > 0 ? &m_locationVector[depth - 1] : nullptr;
            if (nullptr != prev && prev->GetEnabler() == &enabler && prev->GetPropertyIndex() == enabler.GetParentPropertyIndex(propertyIndex))
                {
                Utf8CP lastDot = strrchr(accessString, '.');
                BeAssert(nullptr != lastDot);
                if (nullptr != lastDot)
                    accessString = lastDot + 1;
                }
            }

        return accessString;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECValueAccessor::Location::GetAccessString() const
    {
    Utf8CP accessString = nullptr;
    return nullptr != GetEnabler() && ECObjectsStatus::Success == GetEnabler()->GetAccessString(accessString, GetPropertyIndex()) ? accessString : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String                                        ECValueAccessor::GetPropertyName() const
    {
    uint32_t depth = GetDepth();
    if (0 == depth)
        return Utf8String();

    Utf8String name = GetAccessString(GetDepth() - 1);

    // get the name after the last .
    size_t lastDotIndex = name.rfind('.');
    if (Utf8String::npos != lastDotIndex)
        {
        lastDotIndex++;
        size_t len = name.length() - lastDotIndex;
        name = name.substr(lastDotIndex, len);
        }

    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECValueAccessor::Location::GetECProperty() const
    {
    if (NULL == m_cachedProperty && NULL != m_enabler)
        m_cachedProperty = m_enabler->LookupECProperty(m_propertyIndex);

    return m_cachedProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECValueAccessor::GetECProperty() const
    {
    return DeepestLocationCR().GetECProperty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String                                        ECValueAccessor::GetDebugAccessString() const
    {
    Utf8String temp;
    for (uint32_t depth = 0; depth < GetDepth(); depth++)
        {
        if (depth > 0)
            temp.append(" -> ");
        temp.append(Utf8PrintfString("{%d", m_locationVector[depth].GetPropertyIndex()));
        if (m_locationVector[depth].GetArrayIndex() > -1)
            temp.append(Utf8PrintfString(",%d", m_locationVector[depth].GetArrayIndex()));
        temp.append("}");
        temp.append(GetAccessString(depth));
        }
    return temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String                                        ECValueAccessor::GetManagedAccessString() const
    {
    Utf8String temp;
    for (uint32_t depth = 0; depth < GetDepth(); depth++)
        {
        if (depth > 0)
            temp.append(".");

        Utf8CP str = GetAccessString(depth, false);
        temp.append(str);

        //If the current index is an array element,
        if (m_locationVector[depth].GetArrayIndex() > -1)
            {
            temp.append("[");
            temp.append(Utf8PrintfString("%d", m_locationVector[depth].GetArrayIndex()));
            temp.append("]");
            }
        }
    return temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                            ECValueAccessor::MatchesEnabler(uint32_t depth, ECEnablerCR other) const
    {
    ECEnablerCR enabler = GetEnabler(depth);
    return &enabler == &other;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValueAccessor::operator!=(ECValueAccessorCR accessor) const
    {
    if (GetDepth() != accessor.GetDepth())
        return true;
    for (uint32_t depth = 0; depth < GetDepth(); depth++)
        {
        if ((*this)[depth].GetEnabler() != accessor[depth].GetEnabler()
            || (*this)[depth].GetPropertyIndex() != accessor[depth].GetPropertyIndex()
            || (*this)[depth].GetArrayIndex() != accessor[depth].GetArrayIndex())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValueAccessor::operator==(ECValueAccessorCR accessor) const
    {
    return !(*this != accessor);
    }

#define NUM_INDEX_BUFFER_CHARS 63
#define NUM_ACCESSSTRING_BUFFER_CHARS 1023

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void tokenize(const WString& str, bvector<WString>& tokens, const WString& delimiters)
    {
    // Skip delimiters at beginning.
    WString::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // Find first "non-delimiter".
    WString::size_type pos = str.find_first_of(delimiters, lastPos);

    while (WString::npos != pos || WString::npos != lastPos)
        {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassCP getStructArrayClass(ECClassCR enablerClass, Utf8CP propertyName)
    {
    Utf8CP dotPos = strchr(propertyName, '.');
    if (NULL != dotPos)
        {
        Utf8String structName(propertyName, dotPos);
        ECClassCP structClass = getStructArrayClass(enablerClass, structName.c_str());
        if (NULL == structClass)
            {
            BeAssert(false); return NULL;
            }

        return getStructArrayClass(*structClass, dotPos + 1);
        }

    ECPropertyP propertyP = enablerClass.GetPropertyP(propertyName);
    if (!propertyP)
        return NULL;

    if (auto structProperty = propertyP->GetAsStructProperty())
        return &structProperty->GetType();
    else if (auto arrayProp = propertyP->GetAsStructArrayProperty())
        return &arrayProp->GetStructElementType();

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus getECValueAccessorUsingManagedAccessString(Utf8Char* asBuffer, Utf8Char* indexBuffer, ECValueAccessor& va, ECEnablerCR  enabler, Utf8CP managedPropertyAccessor)
    {
    ECObjectsStatus status;
    uint32_t        propertyIndex;

    // see if access string specifies an array
    Utf8CP pos1 = strchr(managedPropertyAccessor, '[');

    // if not an array then we have a primitive that we can access directly
    if (NULL == pos1)
        {
        status = enabler.GetPropertyIndex(propertyIndex, managedPropertyAccessor);
        if (ECObjectsStatus::Success == status)
            {
            va.PushLocation(enabler, propertyIndex, -1);
            return ECObjectsStatus::Success;
            }

        status = enabler.GetPropertyIndex(propertyIndex, managedPropertyAccessor);

        if (ECObjectsStatus::Success != status)
            return status;

        va.PushLocation(enabler, propertyIndex, -1);
        return ECObjectsStatus::Success;
        }

    size_t numChars = 0;
    numChars = pos1 - managedPropertyAccessor;
PUSH_DISABLE_DEPRECATION_WARNINGS
    strncpy(asBuffer, managedPropertyAccessor, numChars > NUM_ACCESSSTRING_BUFFER_CHARS ? NUM_ACCESSSTRING_BUFFER_CHARS : numChars);
    asBuffer[numChars] = 0;

    Utf8CP pos2 = strchr(pos1 + 1, L']');
    if (!pos2)
        return ECObjectsStatus::Error;

    numChars = pos2 - pos1 - 1;
    strncpy(indexBuffer, pos1 + 1, numChars > NUM_INDEX_BUFFER_CHARS ? NUM_INDEX_BUFFER_CHARS : numChars);
POP_DISABLE_DEPRECATION_WARNINGS
    indexBuffer[numChars] = 0;

    uint32_t indexValue = -1;
    Utf8String::Sscanf_safe(indexBuffer, "%ud", &indexValue);

    ECValue  arrayVal;

    status = enabler.GetPropertyIndex(propertyIndex, asBuffer);
    if (ECObjectsStatus::Success != status)
        return status;

    // if no character after closing bracket then we just want the array, else we are dealing with a member of a struct array
    if (0 == *(pos2 + 1))
        {
        va.PushLocation(enabler, propertyIndex, indexValue);
        return ECObjectsStatus::Success;
        }

    Utf8String str = asBuffer;

    ECClassCP structClass = getStructArrayClass(enabler.GetClass(), asBuffer);
    if (!structClass)
        return ECObjectsStatus::Error;

    ECN::ECEnablerP enablerP = const_cast<ECN::ECEnablerP>(&enabler);
    StandaloneECEnablerPtr structEnabler = dynamic_cast<StandaloneECEnablerP>(enablerP->GetEnablerForStructArrayMember(structClass->GetSchema().GetSchemaKey(), structClass->GetName().c_str()).get());
    if (structEnabler.IsNull())
        return ECObjectsStatus::Error;

    va.PushLocation(enabler, propertyIndex, indexValue);

    return getECValueAccessorUsingManagedAccessString(asBuffer, indexBuffer, va, *structEnabler, pos2 + 2); // move to character after "]." in access string.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::PopulateValueAccessor(ECValueAccessor& va, IECInstanceCR instance, Utf8CP managedPropertyAccessor)
    {
    return PopulateValueAccessor(va, instance.GetEnabler(), managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::PopulateValueAccessor(ECValueAccessor& va, ECEnablerCR enabler, Utf8CP accessor)
    {
    Utf8Char         asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS + 1];
    Utf8Char         indexBuffer[NUM_INDEX_BUFFER_CHARS + 1];
    va.Clear();
    return getECValueAccessorUsingManagedAccessString(asBuffer, indexBuffer, va, enabler, accessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::PopulateValueAccessor(ECValueAccessor& va, IECInstanceCR instance, Utf8CP accessor, bool includeAdHocs)
    {
    auto status = PopulateValueAccessor(va, instance, accessor);
    if (ECObjectsStatus::PropertyNotFound == status && includeAdHocs)
        {
        // Find the array index of the ad-hoc property value with the specified name
        va.Clear();
        for (auto const& containerIndex : AdHocContainerPropertyIndexCollection(instance.GetEnabler()))
            {
            AdHocPropertyQuery adHoc(instance, containerIndex);
            uint32_t arrayIndex;
            if (adHoc.GetPropertyIndex(arrayIndex, accessor))
                {
                va.PushLocation(instance.GetEnabler(), adHoc.GetContainerPropertyIndex(), arrayIndex);
                va.m_isAdHoc = true;
                return ECObjectsStatus::Success;
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool remapAccessString(Utf8StringR newAccessString, ECClassCR newClass, Utf8CP oldAccessString, IECSchemaRemapperCR remapper)
    {
    bvector<Utf8String> propNames;
    BeStringUtilities::Split(oldAccessString, ".", propNames);
    ECClassCP curClass = &newClass;
    for (auto& propName : propNames)
        {
        if (nullptr == curClass)
            return false;

        remapper.ResolvePropertyName(propName, *curClass);
        ECPropertyCP prop = curClass->GetPropertyP(propName.c_str());
        if (nullptr == prop)
            return false;

        auto structProp = prop->GetAsStructProperty();
        curClass = nullptr != structProp ? &structProp->GetType() : nullptr;
        }

    newAccessString = BeStringUtilities::Join(propNames, ".");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECEnablerCP getStructArrayEnablerForDeepestLocation(ECValueAccessorCR va)
    {
    if (0 == va.GetDepth())
        return nullptr;

    auto prop = va.DeepestLocationCR().GetECProperty();
    auto arrayProp = nullptr != prop ? prop->GetAsStructArrayProperty() : nullptr;
    auto structClass = nullptr != arrayProp ? &arrayProp->GetStructElementType() : nullptr;
    if (nullptr != structClass)
        {
        // NEEDSWORK: Why is GetEnablerForStructArrayMember() const, and why does it return a ref-counted ptr? (We obviously need somebody else to keep it alive for us, which they do...)
        auto parentEnabler = const_cast<ECEnablerP> (va.DeepestLocationCR().GetEnabler());
        return nullptr != parentEnabler ? parentEnabler->GetEnablerForStructArrayMember(structClass->GetSchema().GetSchemaKey(), structClass->GetName().c_str()).get() : nullptr;
        }
    else
        return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::RemapValueAccessor(ECValueAccessor& newVa, ECEnablerCR newEnabler, ECValueAccessorCR oldVa, IECSchemaRemapperCR remapper)
    {
    newVa.Clear();
    ECEnablerCP curNewEnabler = &newEnabler;
    bool prevWasArray = false;
    for (Location const& oldLoc : oldVa.GetLocationVector())
        {
        if (prevWasArray && nullptr == (curNewEnabler = getStructArrayEnablerForDeepestLocation(newVa)))
            return ECObjectsStatus::ClassNotFound;

        Utf8String newAccessString;
        if (!remapAccessString(newAccessString, curNewEnabler->GetClass(), oldLoc.GetAccessString(), remapper))
            return ECObjectsStatus::PropertyNotFound;

        newVa.PushLocation(*curNewEnabler, newAccessString.c_str(), oldLoc.GetArrayIndex());

        prevWasArray = oldLoc.GetArrayIndex() != INDEX_ROOT;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::PopulateAndRemapValueAccessor(ECValueAccessor& va, ECEnablerCR enabler, Utf8CP accessString, IECSchemaRemapperCR remapper)
    {
    // Yeah, we're kinda re-implementing Bill's getECValueAccessorUsingManagedAccessString() here...easier than generalizing his function to handle the remapping.
    va.Clear();

    // "A.B.C[0].D[1].E" => "A.B.C", "0].D", "1].E"
    bvector<Utf8String> chunks;
    BeStringUtilities::Split(accessString, "[", chunks);
    if (chunks.empty())
        return ECObjectsStatus::Error;

    ECEnablerCP curEnabler = &enabler;
    for (Utf8String& chunk : chunks)
        {
        Utf8CP thisAccessString = chunk.c_str();
        auto bracPos = chunk.find('[');
        if (WString::npos != bracPos)
            {
            // previous was an array
            if (va.GetDepth() == 0)
                return ECObjectsStatus::Error;

            // Identify any struct subsequent array member accessor
            if (bracPos < chunk.length() - 1)
                {
                if (chunk[bracPos + 1] == '.' && chunk[bracPos + 2] != 0)
                    thisAccessString += bracPos + 2;
                else
                    return ECObjectsStatus::Error;
                }

            // extract array index
            chunk[bracPos] = 0;
            uint32_t arrayIndex;
            if (1 != Utf8String::Sscanf_safe(chunk.c_str(), "%ud", &arrayIndex))
                return ECObjectsStatus::Error;

            va.DeepestLocation().SetArrayIndex(arrayIndex);

            // If this is a struct array member, obtain an enabler for it.
            if (!Utf8String::IsNullOrEmpty(thisAccessString) && nullptr == (curEnabler = getStructArrayEnablerForDeepestLocation(va)))
                return ECObjectsStatus::Error;
            }

        if (!Utf8String::IsNullOrEmpty(thisAccessString))
            {
            Utf8String newAccessString;
            if (remapAccessString(newAccessString, curEnabler->GetClass(), thisAccessString, remapper))
                va.PushLocation(*curEnabler, newAccessString.c_str());
            else
                return ECObjectsStatus::PropertyNotFound;
            }
        }

    return ECObjectsStatus::Success;
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  ECPropertyValue
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue() : m_instance(NULL), m_evaluated(false) {}
ECPropertyValue::ECPropertyValue(IECInstanceCR instance) : m_instance(&instance), m_evaluated(false) {}
ECValueCR           ECPropertyValue::GetValue()            const { EvaluateValue(); return m_ecValue; }
IECInstanceCR       ECPropertyValue::GetInstance()         const { return *m_instance; }
ECValueAccessorCR   ECPropertyValue::GetValueAccessor()    const { return m_accessor; }
ECValueAccessorR    ECPropertyValue::GetValueAccessorR() { return m_accessor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValuePtr     ECPropertyValue::GetPropertyValue(IECInstanceCR instance, Utf8CP propertyAccessor)
    {
    ECValueAccessor va;

    if (ECObjectsStatus::Success != ECValueAccessor::PopulateValueAccessor(va, instance, propertyAccessor))
        return NULL;

    return  new ECPropertyValue(instance, va);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                ECPropertyValue::ResetValue()
    {
    if (m_evaluated)
        {
        m_ecValue.Clear();
        m_evaluated = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECPropertyValue::EvaluateValue() const
    {
    if (!m_evaluated)
        {
        // m_ecValue.Clear();
        m_evaluated = true;
        return NULL != m_instance ? m_instance->GetValueUsingAccessor(m_ecValue, m_accessor) : ECObjectsStatus::Error;
        }
    else
        return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue(ECPropertyValueCR from)
    {
    m_instance = from.m_instance;
    m_accessor = from.m_accessor;
    m_ecValue.From(from.m_ecValue);
    m_evaluated = from.m_evaluated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue(IECInstanceCR instance, ECValueAccessorCR accessor)
    :
    m_instance(&instance), m_accessor(accessor), m_evaluated(false)
    {
    // EvaluateValue(); performance: do this lazily
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue(IECInstanceCR instance, ECValueAccessorCR accessor, ECValueCR v)
    :
    m_instance(&instance), m_accessor(accessor), m_ecValue(v), m_evaluated(true)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECPropertyValue::Initialize(IECInstanceCR instance, Utf8CP accessString, ECValueCR v)
    {
    ECObjectsStatus status = ECValueAccessor::PopulateValueAccessor(m_accessor, instance, accessString);
    if (ECObjectsStatus::Success == status)
        {
        m_instance = &instance;
        m_ecValue = v;
        m_evaluated = true;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                ECPropertyValue::HasChildValues() const
    {
    // Avoid evaluating value if we can answer this by looking at the ECProperty
    // Note: performance: the accessor caches the ECProperty, since we often request it more than once
    ECPropertyCP prop = m_accessor.GetECProperty();
    PrimitiveArrayECPropertyCP arrayProp;
    if (NULL == prop || prop->GetIsPrimitive() || prop->GetIsNavigation())
        return false;
    else if (NULL != (arrayProp = prop->GetAsPrimitiveArrayProperty()) && -1 != m_accessor.DeepestLocationCR().GetArrayIndex())
        return false;   // this is a primitive array member, it has no child properties
    else if (prop->GetIsStruct())
        return true;    // embedded struct always has child values, ECValue always null

    // It's an array or struct array instance. Must evaluate value to determine if null/empty
    EvaluateValue();
    if (m_ecValue.IsNull())
        return false;       // null array (should not happen) or null struct array instance
    else if (m_ecValue.IsStruct())
        return true;        // struct array entry
#ifdef EMPTY_ARRAYS_DONT_HAVE_CHILD_VALUES
    else if (m_ecValue.IsArray() && 0 == m_ecValue.GetArrayInfo().GetCount())
        return false;       // empty array
#endif
    else
        {
        BeAssert(m_ecValue.IsArray());
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionPtr  ECPropertyValue::GetChildValues() const
    {
    return HasChildValues() ? new ECValuesCollection(*this) : new ECValuesCollection();
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  ECValuesCollectionIterator
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue ECValuesCollectionIterator::GetFirstPropertyValue(IECInstanceCR instance)
    {
    ECEnablerCR enabler = instance.GetEnabler();
    if (0 == enabler.GetClass().GetPropertyCount())
        return ECPropertyValue();

    uint32_t firstIndex = enabler.GetFirstPropertyIndex(0);
    ECValueAccessor firstPropertyAccessor;
    if (0 != firstIndex)
        firstPropertyAccessor.PushLocation(enabler, firstIndex, -1);

    return ECPropertyValue(instance, firstPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionIterator::ECValuesCollectionIterator(IECInstanceCR instance)
    :
    m_propertyValue(GetFirstPropertyValue(instance)),
    m_arrayCount(-1)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue ECValuesCollectionIterator::GetChildPropertyValue(ECPropertyValueCR parentPropertyValue)
    {
    m_arrayCount = -1;
    ECValueCR       parentValue = parentPropertyValue.GetValue();
    ECValueAccessor childAccessor(parentPropertyValue.GetValueAccessor());

    if (parentValue.IsArray())
        {
        ArrayInfo   arrayInfo = parentValue.GetArrayInfo();
        uint32_t    arrayCount = arrayInfo.GetCount();

        if (0 < arrayCount)
            {
            m_arrayCount = arrayCount;
            childAccessor.DeepestLocation().SetArrayIndex(0);
            }
        else
            childAccessor.PopLocation();
        }
    else /* if (parentValue.IsStruct()) ###TODO: concept of an ECValue containing an embedded struct is undefined. */
        {
        uint32_t        pathLength = childAccessor.GetDepth();

        if (!EXPECTED_CONDITION(0 < pathLength))
            return ECPropertyValue();

        if (ECValueAccessor::INDEX_ROOT != childAccessor[pathLength - 1].GetArrayIndex())
            {
            IECInstancePtr  structInstance = parentValue.GetStruct();
            if (structInstance.IsValid())
                {
                ECEnablerCR     enabler = structInstance->GetEnabler();
                uint32_t        firstIndex = enabler.GetFirstPropertyIndex(0);

                childAccessor.PushLocation(enabler, firstIndex, -1);
                }
            else
                {
                // if the parent struct array entry contains a NULL instance then there are no
                // child properties to iterate.
                childAccessor.Clear();
                }
            }
        else
            {
            uint32_t        parentIndex = childAccessor[pathLength - 1].GetPropertyIndex();
            ECEnablerCR     enabler = *childAccessor[pathLength - 1].GetEnabler();
            uint32_t        firstIndex = enabler.GetFirstPropertyIndex(parentIndex);

            if (0 != firstIndex)
                childAccessor.PushLocation(enabler, firstIndex, -1);
            else
                childAccessor.PopLocation();
            }
        }

    return ECPropertyValue(parentPropertyValue.GetInstance(), childAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionIterator::ECValuesCollectionIterator(ECPropertyValueCR parentPropertyValue)
    :
    m_propertyValue(GetChildPropertyValue(parentPropertyValue))
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//bool            ECValuesCollectionIterator::IsAtEnd() const
//    {
//    return 0 == m_propertyValue.GetValueAccessor().GetDepth();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValuesCollectionIterator::IsDifferent(ECValuesCollectionIterator const& otherIter) const
    {
    bool leftAtEnd = 0 == m_propertyValue.GetValueAccessor().GetDepth();
    bool rightAtEnd = 0 == otherIter.m_propertyValue.GetValueAccessor().GetDepth();
    if (leftAtEnd && rightAtEnd)
        return false;

    if (!leftAtEnd && !rightAtEnd)
        return m_propertyValue.GetValueAccessor() != otherIter.m_propertyValue.GetValueAccessor();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue const& ECValuesCollectionIterator::GetCurrent() const
    {
    return m_propertyValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValuesCollectionIterator::MoveToNext()
    {
    ECValueAccessorR    currentAccessor = m_propertyValue.GetValueAccessorR();

    if (ECValueAccessor::INDEX_ROOT != currentAccessor.DeepestLocation().GetArrayIndex())
        {
        /*--------------------------------------------------------------------------
          If we are on an array member get the next member
        --------------------------------------------------------------------------*/
        if (!EXPECTED_CONDITION(0 <= m_arrayCount))
            return;

        currentAccessor.DeepestLocation().IncrementArrayIndex();

        // If that was the last member of the array, we are done
        if (currentAccessor.DeepestLocation().GetArrayIndex() >= m_arrayCount)
            {
            currentAccessor.Clear();
            return;
            }
        }
    else
        {
        /*--------------------------------------------------------------------------
          Ask the enabler for the next sibling property.
        --------------------------------------------------------------------------*/
        uint32_t    pathLength = currentAccessor.GetDepth();
        uint32_t    currentIndex = currentAccessor[pathLength - 1].GetPropertyIndex();
        uint32_t    parentIndex = 0;

        // If we are inside an embedded struct get the struct index from the accessor's path
        if (pathLength > 1 && ECValueAccessor::INDEX_ROOT == currentAccessor[pathLength - 2].GetArrayIndex())
            parentIndex = currentAccessor[pathLength - 2].GetPropertyIndex();

        ECEnablerCP enabler = currentAccessor[pathLength - 1].GetEnabler();
        uint32_t    nextIndex = enabler->GetNextPropertyIndex(parentIndex, currentIndex);

        currentAccessor.DeepestLocation().SetPropertyIndex(nextIndex);

        // If that was the last index in the current struct, we are done
        if (0 == nextIndex)
            {
            currentAccessor.Clear();
            return;
            }
        }

    m_propertyValue.ResetValue();
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  ECValuesCollection
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::ECValuesCollection()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::ECValuesCollection(ECPropertyValueCR parentPropValue)
    : m_parentPropertyValue(parentPropValue)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::ECValuesCollection(IECInstanceCR instance)
    : m_parentPropertyValue(instance)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionPtr ECValuesCollection::Create(IECInstanceCR instance)
    {
    return new ECValuesCollection(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionPtr ECValuesCollection::Create(ECPropertyValueCR  parentProperty)
    {
    if (parentProperty.HasChildValues())
        return new ECValuesCollection(parentProperty);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValueCR  ECValuesCollection::GetParentProperty() const
    {
    return m_parentPropertyValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::const_iterator ECValuesCollection::begin() const
    {
    RefCountedPtr<ECValuesCollectionIterator> iter;

    if (0 == m_parentPropertyValue.GetValueAccessor().GetDepth())
        iter = new ECValuesCollectionIterator(m_parentPropertyValue.GetInstance());
    else
        {
        ECValueCR       parentValue = m_parentPropertyValue.GetValue();
        if (parentValue.IsArray())
            {
            ArrayInfo   arrayInfo = parentValue.GetArrayInfo();
            uint32_t    arrayCount = arrayInfo.GetCount();

            if (0 == arrayCount)
                return ECValuesCollection::end();
            }

        iter = new ECValuesCollectionIterator(m_parentPropertyValue);
        }

    return const_iterator(*iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionIterator::ECValuesCollectionIterator() : m_arrayCount(-1)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::const_iterator ECValuesCollection::end() const
    {
    // WIP_FUSION: can we reduce the amount of dynamic allocation associated with iterating an ECValuesCollection?
    if (m_end.IsNull())
        m_end = new ECValuesCollectionIterator();

    return const_iterator(*m_end);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::SupportsDotNetFormatting() const
    {
    if (IsNull() || !IsPrimitive())
        return false;

    switch (GetPrimitiveType())
        {
            case PRIMITIVETYPE_Long:
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_Double:
                return true;
            default:
                return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct NumericFormat
    {
    private:
        static const uint32_t MAX_PRECISION = 97;         // A limit imposed by Snwprintf(); values above this will produce an exception

        bool            insertThousandsSeparator;
        PrecisionType   precisionType;
        uint8_t         widthBeforeDecimal;         // pad with leading zeros to meet this width
        uint8_t         minDecimalPrecision;        // pad with trailing zeros to meet this precision
        uint8_t         maxDecimalPrecision;        // remove trailing zeros between minDecimalPrecision and this
        double          multiplier;
        size_t          insertPos;

        NumericFormat() : insertThousandsSeparator(false), precisionType(PrecisionType::Decimal), widthBeforeDecimal(0), minDecimalPrecision(0), maxDecimalPrecision(0), multiplier(1.0), insertPos(-1) {}

        DoubleFormatterPtr ToFormatter() const
            {
            DoubleFormatterPtr fmtr = DoubleFormatter::Create();
            fmtr->SetLeadingZero(widthBeforeDecimal > 0);
            fmtr->SetTrailingZeros(maxDecimalPrecision > 0);
            fmtr->SetPrecision(precisionType, maxDecimalPrecision);
            fmtr->SetInsertThousandsSeparator(insertThousandsSeparator);
            return fmtr;
            }

        static bool     ExtractStandardFormatPrecision(Utf8CP fmt, uint32_t& precision);
        static bool     ApplyStandardNumericFormat(Utf8StringR formatted, Utf8CP fmt, double d);
        static void     ParseCustomFormatString(Utf8StringR formatted, Utf8CP fmt, NumericFormat& numFormat, bool ignoreExponent);
        static bool     ApplyCustomNumericFormat(Utf8StringR formatted, Utf8CP fmt, double d, bool* onlyZeros);
        static Utf8CP   ParseNumberFormat(NumericFormat& numFormat, Utf8CP start);
        static Utf8CP   SkipLiteralString(Utf8CP start);
    public:
        static bool                 FormatDouble(Utf8StringR formatted, Utf8CP formatString, double d);
        static bool                 FormatInteger(Utf8StringR formatted, Utf8CP formatString, int64_t i);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::ExtractStandardFormatPrecision(Utf8CP fmt, uint32_t& precision)
    {
    // Expect 0-2 digits specifying a precision or width from 0-99
    // Don't overwrite value of precision unless one is explicitly specified or cannot be extracted
    bool isValidFormat = true;
    BeAssert(NULL != fmt);
    if (*fmt)
        {
        precision = 0;
        if (isdigit(*fmt))
            {
            precision = (uint32_t) (*fmt - '0');
            if (*++fmt)
                {
                if (isdigit(*fmt))
                    {
                    precision *= 10;
                    precision += (uint32_t) (*fmt - '0');
                    if (precision > MAX_PRECISION)
                        precision = MAX_PRECISION;

                    if (*++fmt)   // trailing characters
                        isValidFormat = false;
                    }
                else
                    isValidFormat = false;
                }
            }
        else
            isValidFormat = false;
        }

    return isValidFormat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::FormatInteger(Utf8StringR formatted, Utf8CP fmt, int64_t i)
    {
    // Check for a couple of standard formats specific to integers
    if (NULL != fmt)
        {
        Utf8Char spec = *fmt;
        Utf8Char lspec = (Utf8Char) tolower(spec);
        if ('d' == lspec || 'x' == lspec)
            {
            uint32_t precision = 0;
            if (ExtractStandardFormatPrecision(fmt + 1, precision))
                {
                Utf8Char buf[100]; // because max width is 99
                switch (lspec)
                    {
                        case 'x':       // hexadecimal
                        {
                        HexFormatOptions opts = HexFormatOptions::LeadingZeros;
                        if ('X' == spec)
                            opts = (HexFormatOptions) (static_cast<int>(opts) | static_cast<int>(HexFormatOptions::Uppercase));

                        BeStringUtilities::FormatUInt64(buf, _countof(buf), (uint64_t) i, opts, static_cast <uint8_t> (precision));
                        }
                        break;
                        case 'd':       // decimal
                        {
                        BeStringUtilities::Snprintf(buf, "%0*lld", precision, i);
                        }
                        break;
                    }

                formatted = buf;
                return true;
                }
            }
        }

    // Treat as double
    return FormatDouble(formatted, fmt, (double) i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::ApplyStandardNumericFormat(Utf8StringR formatted, Utf8CP fmt, double d)
    {
    // Limited support for following standard .NET specifiers. All can be upper- or lower-case, all take an optional width/precision from 0-99
    //  E: scientific notation. Default precision 6.
    //  C: Currency. Treated as F
    //  F: Fixed-point. Default precision 2.
    //  N: Includes group separators
    //  R: Round-trippable. Treated as F with maximum precision.
    //  P: Multiply input by 100, append '%'
    //  G: Treated as fixed-point.

    BeAssert(NULL != fmt && 0 != *fmt);

    Utf8Char spec = *fmt,
        lspec = (Utf8Char) tolower(spec);

    PrecisionType precisionType = PrecisionType::Decimal;
    bool groupSeparators = false,
        appendPercent = false,
        ignoreExtractedPrecision = false;
    switch (lspec)
        {
            case 'e':
                precisionType = PrecisionType::Scientific;
                break;
            case 'p':
                d *= 100.0;
                appendPercent = true;
                spec = lspec = 'f';
                break;
            case 'r':
                ignoreExtractedPrecision = true;
                spec = lspec = 'f';
                break;
            case 'n':
                groupSeparators = true;
                // fall-through intentional
            case 'c':
            case 'g':
                spec = lspec = 'f';
                break;
            case 'f':
                break;
            default:
                return false;
        }

    uint32_t precision = 16;
    if (!ExtractStandardFormatPrecision(fmt + 1, precision))
        return false;

    DoubleFormatterPtr fmtr = DoubleFormatter::Create();

    if (ignoreExtractedPrecision)
        precision = MAX_PRECISION;
    else
        fmtr->SetTrailingZeros(false);

    fmtr->SetLeadingZero(true);
    fmtr->SetInsertThousandsSeparator(groupSeparators);
    fmtr->SetPrecision(precisionType, (Byte) precision);

    formatted = fmtr->ToString(d);

    if (appendPercent)
        formatted.append(1, '%');
    else if (ignoreExtractedPrecision && formatted.length() > 1)
        {
        // DoubleFormatter is going to give us trailing zeros. Strip them off
        size_t endPos = formatted.length() - 1,
            startPos = formatted.find('.');

        while (formatted[endPos] == '0')
            --endPos;

        if (endPos == startPos)
            --endPos;   // only zeros follow the decimal point, so remove it.

        formatted.erase(endPos + 1);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NumericFormat::ParseCustomFormatString(Utf8StringR formatted, Utf8CP fmt, NumericFormat& numFormat, bool ignoreExponent)
    {
    while (0 != *fmt)
        {
        switch (*fmt)
            {
                case '\'':
                case '"':       // literal string
                {
                Utf8CP endQuote = SkipLiteralString(fmt);
                if (endQuote - fmt > 2)
                    formatted.append(fmt + 1, endQuote - fmt - 1);

                fmt = (0 != *endQuote) ? endQuote : endQuote - 1;   // in case of unclosed quote.
                }
                break;
                case '\\':      // escaped character
                    if (0 != *++fmt)
                        formatted.append(1, *fmt);

                    break;
                case '%':
                    numFormat.multiplier *= 100.0;
                    formatted.append(1, '%');
                    break;
                case ',':
                    if (numFormat.insertPos != -1)  // a comma anywhere after the number format acts as a scaling factor, IF no decimal precision specified
                        {
                        if (numFormat.maxDecimalPrecision == 0)
                            numFormat.multiplier /= 1000.0;
                        }
                    else
                        formatted.append(1, ',');
                    break;
                case 'e':
                case 'E':
                    if (ignoreExponent)
                        {
                        formatted.append(1, *fmt++);

                        if ('+' == *fmt || '-' == *fmt)
                            formatted.append(1, *fmt++);

                        while ('0' == *fmt)
                            formatted.append(1, *fmt++);
                        }
                    else
                        {
                        numFormat.precisionType = PrecisionType::Scientific;
                        // ignore exponent sign
                        fmt++;
                        if ('+' == *fmt || '-' == *fmt)
                            fmt++;

                        // ignore exponent width
                        while ('0' == *fmt)
                            fmt++;
                        }

                    continue;   // we've already moved to next token
                case '.':
                case '0':
                case '#':
                    if (numFormat.insertPos == -1)
                        {
                        numFormat.insertPos = formatted.length();
                        fmt = ParseNumberFormat(numFormat, fmt);
                        }
                    else
                        formatted.append(1, *fmt++);
                    continue;   // we've already moved to next token
                default:
                    formatted.append(1, *fmt);
                    break;
            }

        fmt++;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::ApplyCustomNumericFormat(Utf8StringR formatted, Utf8CP fmt, double d, bool* onlyZeros)
    {
    Utf8String originalFmt = fmt;
    formatted.clear();

    NumericFormat numFormat;
    ParseCustomFormatString(formatted, fmt, numFormat, false);

    if (NULL != onlyZeros)
        *onlyZeros = false;

    // It's possible the format string did not actually contain any placeholders for the digits, in which case we have no formatting to do
    if (Utf8String::npos == numFormat.insertPos)
        {
        // Need to go back and put processed characters such as exponents back in.
        formatted.clear();
        ParseCustomFormatString(formatted, fmt, numFormat, true);

        return true;
        }

    DoubleFormatterPtr fmtr = numFormat.ToFormatter();
    Utf8String formattedDouble = fmtr->ToString(d * numFormat.multiplier);

    // Caller needs to know if rounding or precision of the format caused this to be only 0's.
    // Formatter can also insert things such as +,-,.,e,E, etc., so I think the best check is for non-zero digits.
    if (NULL != onlyZeros)
        *onlyZeros = !std::any_of(formattedDouble.begin(), formattedDouble.end(), [&] (Utf8Char const& c) { return c >= '1' && c <= '9'; });

    // We have to pad width with leading zeros, DoubleFormatter doesn't support it.
    if (numFormat.widthBeforeDecimal > 0)
        {
        size_t endPos = formattedDouble.find('.');
        if (Utf8String::npos == endPos)
            endPos = formattedDouble.length();

        if ((uint32_t) endPos < numFormat.widthBeforeDecimal)
            formattedDouble.insert((size_t) 0, numFormat.widthBeforeDecimal - (uint32_t) endPos, '0');
        }
    else if (formattedDouble.length() > 0 && formattedDouble[0] == '0')
        {
        // DoubleFormatter ignores our leading zero setting if the value of the double is zero
        formattedDouble.erase(0, 1);
        }

    // And we have to remove trailing zeros
    if (numFormat.minDecimalPrecision < numFormat.maxDecimalPrecision)
        {
        size_t decimalPos = (uint32_t) formattedDouble.find('.');
        if (Utf8String::npos != decimalPos)
            {
            uint32_t minPos = (uint32_t) decimalPos + 1 + numFormat.minDecimalPrecision, // the minimum number of decimal digits to keep, regardless of whether or not they are zero
                maxPos = (uint32_t) decimalPos + 1 + numFormat.maxDecimalPrecision;

            if (maxPos >= (uint32_t) formattedDouble.length())
                maxPos = (uint32_t) formattedDouble.length() - 1;

            while (maxPos >= minPos && '0' == formattedDouble[maxPos])
                formattedDouble.erase(maxPos--);

            // trailing decimal point?
            if ((uint32_t) decimalPos == maxPos)
                formattedDouble.erase(decimalPos);
            }
        }

    if (numFormat.insertPos == formatted.length())
        formatted.append(formattedDouble);
    else if (numFormat.insertPos < formatted.length())
        formatted.insert(numFormat.insertPos, formattedDouble);
    else
        {
        BeAssert(false); return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::FormatDouble(Utf8StringR formatted, Utf8CP fmt, double d)
    {
    if (ApplyStandardNumericFormat(formatted, fmt, d))
        return true;

    // Custom formatting.
    // Limitations:
    //  -Expects a single contiguous number format like '###,##0.00#' consisting of only those four characters, with optional prefix and suffix.
    //      i.e. "xxx ##,#00.0## xxx" is valid, "## xxx 000.0 xx 00" is not - only the "##" will be replaced with digits.
    //  -Does not support custom exponent sign/width
    //  -Probably misc differences. Note that Microsoft's implementation does not match their documentation in all cases either, so we are not going to bend over backwards...

    // Support for "sections": different format strings for positive/negative/zero.
    // http://msdn.microsoft.com/en-us/library/0c899ak8(v=vs.110).aspx#SectionSeparator
    //  One section: The format string applies to all values.
    //  Two sections: The first section applies to positive values and zeros, and the second section applies to negative values. If the number to be formatted is negative, but becomes zero after rounding according to the format in the second section, the resulting zero is formatted according to the first section.
    //  Three sections: The first section applies to positive values, the second section applies to negative values, and the third section applies to zeros. The second section can be left empty(by having nothing between the semicolons), in which case the first section applies to all nonzero values. If the number to be formatted is nonzero, but becomes zero after rounding according to the format in the first or second section, the resulting zero is formatted according to the third section.
    bvector<Utf8String> sections;
    BeStringUtilities::Split(fmt, ";", "\\", sections);

    // Do something arbitrary for bad data.
    if (UNEXPECTED_CONDITION(0 == sections.size()))
        return ApplyStandardNumericFormat(formatted, "g", d);

    if (UNEXPECTED_CONDITION(sections.size() > 3))
        return ApplyCustomNumericFormat(formatted, fmt, d, NULL);

    // Process the sections as noted above.
    if (1 == sections.size())
        return ApplyCustomNumericFormat(formatted, fmt, d, NULL);

    if (2 == sections.size())
        {
        if (d >= 0.0)
            return ApplyCustomNumericFormat(formatted, sections[0].c_str(), d, NULL);

        bool onlyZeros;
        if (!ApplyCustomNumericFormat(formatted, sections[1].c_str(), d, &onlyZeros))
            return false;

        if (!onlyZeros)
            return true;

        return ApplyCustomNumericFormat(formatted, sections[0].c_str(), 0.0, NULL);
        }

    if (0.0 == d)
        return ApplyCustomNumericFormat(formatted, sections[2].c_str(), d, NULL);

    size_t trySection = ((d > 0.0) ? 0 : 1);

    bool onlyZeros;
    if (!ApplyCustomNumericFormat(formatted, sections[trySection].c_str(), d, &onlyZeros))
        return false;

    if (!onlyZeros)
        return true;

    return ApplyCustomNumericFormat(formatted, sections[2].c_str(), 0.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NumericFormat::SkipLiteralString(Utf8CP start)
    {
    Utf8CP end = start + 1;
    Utf8Char quoteChar = *start;

    while (*end)
        {
        if (quoteChar == *end)
            break;

        ++end;
        }

    return end;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NumericFormat::ParseNumberFormat(NumericFormat& numFormat, Utf8CP start)
    {
    bool foundZero = false;
    bool stopProcessing = false;
    Utf8CP cur = start;
    while (0 != *cur)
        {
        switch (*cur)
            {
                case '0':
                    foundZero = true;
                    numFormat.widthBeforeDecimal++;
                    break;
                case '#':
                    if (foundZero)
                        numFormat.widthBeforeDecimal++;
                    break;
                case ',':
                    // check if this is a trailing comma, in which case we treat it as a scaling factor, not a group separator
                    if (*(cur + 1) == '.' || *(cur + 1) == ',')
                        {
                        // Is a scaling factor if immediately precedes decimal point
                        numFormat.multiplier /= 1000.0;
                        break;
                        }
                    else if (*(cur + 1) != '0' && *(cur + 1) != '#')
                        {
                        // Is a scaling factor, will be processed by calling code
                        /*<==*/             return cur;
                        }
                    else
                        {
                        // Is a group separator
                        numFormat.insertThousandsSeparator = true;
                        break;
                        }
                case '.':
                default:
                    stopProcessing = true;
                    break;
            }

        if (stopProcessing)
            break;
        else
            ++cur;
        }

    if ('.' == *cur)
        {
        uint8_t numPlaceholders = 0;
        cur++;
        stopProcessing = false;
        while (0 != *cur)
            {
            switch (*cur)
                {
                    case '0':
                        numPlaceholders++;
                        numFormat.minDecimalPrecision = numFormat.maxDecimalPrecision = numPlaceholders;
                        break;
                    case '#':
                        numPlaceholders++;
                        numFormat.maxDecimalPrecision = numPlaceholders;
                        break;
                    default:
                        stopProcessing = true;
                        break;
                }

            if (stopProcessing)
                break;
            else
                ++cur;
            }
        }

    return cur;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::ApplyDotNetFormatting(Utf8StringR out, Utf8CP fmt) const
    {
    if (IsNull())
        return false;

    switch (GetPrimitiveType())
        {
            case PRIMITIVETYPE_Integer:
                return NumericFormat::FormatInteger(out, fmt, GetInteger());
            case PRIMITIVETYPE_Long:
                return NumericFormat::FormatInteger(out, fmt, GetLong());
            case PRIMITIVETYPE_Double:
                return NumericFormat::FormatDouble(out, fmt, GetDouble());
            default:
                BeAssert(false && "Call ECValue::SupportsDotNetFormatting() to determine if this ECValue can be formatted");
                return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocPropertyQuery::AdHocPropertyQuery(IECInstanceCR host, Utf8CP accessString)
    : AdHocPropertyMetadata(host.GetEnabler(), accessString), m_host(host)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocPropertyQuery::AdHocPropertyQuery(IECInstanceCR host, uint32_t propertyIndex)
    : AdHocPropertyMetadata(host.GetEnabler(), propertyIndex), m_host(host)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocPropertyMetadata::IsSupported(ECEnablerCR enabler, Utf8CP accessString)
    {
    AdHocPropertyMetadata meta(enabler, accessString, false);
    return meta.IsSupported();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocPropertyMetadata::IsSupported(ECEnablerCR enabler, uint32_t propIdx)
    {
    AdHocPropertyMetadata meta(enabler, propIdx, false);
    return meta.IsSupported();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocPropertyMetadata::AdHocPropertyMetadata(ECEnablerCR enabler, Utf8CP containerAccessString, bool loadMetadata)
    : m_containerIndex(0)
    {
    uint32_t containerIndex = 0;
    if (ECObjectsStatus::Success == enabler.GetPropertyIndex(containerIndex, containerAccessString))
        Init(enabler, containerIndex, loadMetadata);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocPropertyMetadata::AdHocPropertyMetadata(ECEnablerCR enabler, uint32_t propIdx, bool loadMetadata)
    : m_containerIndex(0)
    {
    Init(enabler, propIdx, loadMetadata);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocPropertyMetadata::AdHocPropertyMetadata(ECEnablerCR enabler, Utf8CP containerAccessString)
    : AdHocPropertyMetadata(enabler, containerAccessString, true)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocPropertyMetadata::AdHocPropertyMetadata(ECEnablerCR enabler, uint32_t propIdx)
    : AdHocPropertyMetadata(enabler, propIdx, true)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocPropertyMetadata::Init(ECEnablerCR enabler, uint32_t containerIndex, bool loadMetadata)
    {
    // find struct array property
    ECPropertyCP prop = enabler.LookupECProperty(containerIndex);
    StructArrayECPropertyCP arrayProp = nullptr != prop ? prop->GetAsStructArrayProperty() : nullptr;
    ECClassCP structClass = nullptr;
    if (nullptr == arrayProp || nullptr == (structClass = &arrayProp->GetStructElementType()))
        return false;

    // find custom attribute on struct class
    IECInstancePtr attr = structClass->GetCustomAttribute("Bentley_Standard_CustomAttributes", "AdhocPropertyContainerDefinition");
    if (attr.IsNull())
        return false;

    // validate required metadata is defined, and load it if requested
    ECValue v;
    v.SetAllowsPointersIntoInstanceMemory(true);

    static const Utf8CP s_propertyNames[(size_t) Index::MAX] =
        {
        "NameProperty", "DisplayLabelProperty", "ValueProperty", "TypeProperty", "UnitProperty", "ExtendTypeProperty", "IsReadOnlyProperty", "IsHiddenProperty"
        };

    for (size_t i = 0; i < _countof(s_propertyNames); i++)
        {
        if (ECObjectsStatus::Success == attr->GetValue(v, s_propertyNames[i]) && !v.IsNull() && v.IsString())
            {
            prop = structClass->GetPropertyP(v.GetUtf8CP());
            if (nullptr != prop)
                {
                if (loadMetadata)
                    m_metadataPropertyNames[i] = v.GetUtf8CP();
                }
            else
                return false;
            }
        else if (IsRequiredMetadata(static_cast<Index>(i)))
            return false;
        }

    // only considered valid when m_containerIndex set to non-zero
    m_containerIndex = containerIndex;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocPropertyMetadata::IsRequiredMetadata(Index index)
    {
    return Index::Name == index || Index::Value == index;
    }

/*---------------------------------------------------------------------------------**//**
* From managed...See ECAdHocProperties::GetKnownTypeForCode
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
enum class PrimitiveTypeCode
    {
    String = 0,
    Integer,
    Long,
    Double,
    DateTime,
    Boolean,
    Binary,
    Point2d,
    Point3d
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocPropertyMetadata::PrimitiveTypeForCode(PrimitiveType& type, int32_t code)
    {
    switch (code)
        {
            case (int32_t) PrimitiveTypeCode::String:     type = PRIMITIVETYPE_String; return true;
            case (int32_t) PrimitiveTypeCode::Integer:    type = PRIMITIVETYPE_Integer; return true;
            case (int32_t) PrimitiveTypeCode::Long:       type = PRIMITIVETYPE_Long; return true;
            case (int32_t) PrimitiveTypeCode::Double:     type = PRIMITIVETYPE_Double; return true;
            case (int32_t) PrimitiveTypeCode::DateTime:   type = PRIMITIVETYPE_DateTime; return true;
            case (int32_t) PrimitiveTypeCode::Boolean:    type = PRIMITIVETYPE_Boolean; return true;
            case (int32_t) PrimitiveTypeCode::Point2d:    type = PRIMITIVETYPE_Point2d; return true;
            case (int32_t) PrimitiveTypeCode::Point3d:    type = PRIMITIVETYPE_Point3d; return true;
            default:                            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocPropertyMetadata::CodeForPrimitiveType(int32_t& code, PrimitiveType type)
    {
    switch (type)
        {
            case PRIMITIVETYPE_String:      code = static_cast<int32_t> (PrimitiveTypeCode::String); return true;
            case PRIMITIVETYPE_Integer:     code = static_cast<int32_t> (PrimitiveTypeCode::Integer); return true;
            case PRIMITIVETYPE_Long:        code = static_cast<int32_t> (PrimitiveTypeCode::Long); return true;
            case PRIMITIVETYPE_Double:      code = static_cast<int32_t> (PrimitiveTypeCode::Double); return true;
            case PRIMITIVETYPE_DateTime:    code = static_cast<int32_t> (PrimitiveTypeCode::DateTime); return true;
            case PRIMITIVETYPE_Boolean:     code = static_cast<int32_t> (PrimitiveTypeCode::Boolean); return true;
            case PRIMITIVETYPE_Point2d:     code = static_cast<int32_t> (PrimitiveTypeCode::Point2d); return true;
            case PRIMITIVETYPE_Point3d:     code = static_cast<int32_t> (PrimitiveTypeCode::Point3d); return true;
            default:                        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocPropertyMetadata::IsSupported() const
    {
    return 0 != m_containerIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AdHocPropertyMetadata::GetPropertyName(Index index) const
    {
    auto const& name = m_metadataPropertyNames[static_cast<size_t> (index)];
    return !name.empty() ? name.c_str() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr AdHocPropertyQuery::GetEntry(uint32_t index) const
    {
    if (!IsSupported())
        return nullptr;

    ECValue v;
    if (ECObjectsStatus::Success != m_host.GetValue(v, GetContainerPropertyIndex(), index) || v.IsNull())
        return nullptr;
    else if (!v.IsStruct())
        {
        BeAssert(false);
        return nullptr;
        }
    else
        return v.GetStruct();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetValue(ECValueR v, uint32_t index, Utf8CP accessor) const
    {
    auto entry = GetEntry(index);
    return entry.IsValid() ? entry->GetValue(v, accessor) : ECObjectsStatus::PropertyNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocPropertyQuery::GetPropertyIndex(uint32_t& index, Utf8CP accessString) const
    {
    if (!IsSupported())
        return false;

    ECValue v;
    if (ECObjectsStatus::Success == m_host.GetValue(v, GetContainerPropertyIndex()) && v.IsArray())
        {
        uint32_t count = v.GetArrayInfo().GetCount();
        for (uint32_t i = 0; i < count; i++)
            if (ECObjectsStatus::Success == m_host.GetValue(v, GetContainerPropertyIndex(), i) && !v.IsNull() && v.IsStruct())
                {
                IECInstancePtr instance = v.GetStruct();
                v.SetAllowsPointersIntoInstanceMemory(true);
                if (ECObjectsStatus::Success == instance->GetValue(v, GetPropertyName(Index::Name)) && !v.IsNull() && v.IsString() && 0 == strcmp(accessString, v.GetUtf8CP()))
                    {
                    index = i;
                    return true;
                    }
                }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t AdHocPropertyQuery::GetCount() const
    {
    ECValue v;
    if (IsSupported() && ECObjectsStatus::Success == m_host.GetValue(v, GetContainerPropertyIndex()) && v.IsArray())
        return v.GetArrayInfo().GetCount();
    else
        return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetName(Utf8StringR name, uint32_t index) const { return GetString(name, index, Index::Name); }
ECObjectsStatus AdHocPropertyQuery::GetExtendedTypeName(Utf8StringR name, uint32_t index) const { return GetString(name, index, Index::ExtendType); }
ECObjectsStatus AdHocPropertyQuery::GetUnitName(Utf8StringR name, uint32_t index) const { return GetString(name, index, Index::Unit); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetDisplayLabel(Utf8StringR label, uint32_t index) const
    {
    auto status = GetString(label, index, Index::DisplayLabel);
    if (ECObjectsStatus::Success != status)
        {
        Utf8String name;
        status = GetName(name, index);
        if (ECObjectsStatus::Success == status)
            ECNameValidation::DecodeFromValidName(label, name);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetPrimitiveType(PrimitiveType& type, uint32_t index) const
    {
    auto entry = GetEntry(index);
    if (entry.IsNull())
        return ECObjectsStatus::PropertyNotFound;

    Utf8CP propName = GetPropertyName(Index::Type);
    if (nullptr == propName)
        {
        // defaults to string if no property to specify otherwise
        type = PRIMITIVETYPE_String;
        return ECObjectsStatus::Success;
        }

    ECValue v;
    if (ECObjectsStatus::Success != entry->GetValue(v, propName) || !v.IsInteger())
        return ECObjectsStatus::Error;
    else if (v.IsNull())
        {
        type = PRIMITIVETYPE_String;
        return ECObjectsStatus::Success;
        }
    else
        return PrimitiveTypeForCode(type, v.GetInteger()) ? ECObjectsStatus::Success : ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetValue(ECValueR propertyValue, uint32_t index) const
    {
    // avoid looking up the struct instance repeatedly.
    auto entry = GetEntry(index);
    if (entry.IsNull())
        return ECObjectsStatus::PropertyNotFound;

    // get value type
    PrimitiveType type = PRIMITIVETYPE_String;
    Utf8CP propName = GetPropertyName(Index::Type);
    ECValue v;
    if (nullptr != propName)
        {
        auto status = entry->GetValue(v, propName);
        if (ECObjectsStatus::Success == status)
            {
            // null => use default type (string)
            if (!v.IsNull() && (!v.IsInteger() || !PrimitiveTypeForCode(type, v.GetInteger())))
                status = ECObjectsStatus::Error;
            }

        if (ECObjectsStatus::Success != status)
            return status;
        }

    // get value
    auto status = GetValue(propertyValue, *entry, Index::Value);
    if (ECObjectsStatus::Success != status)
        return status;
    else if (!propertyValue.IsString() && !propertyValue.IsNull())
        {
        BeAssert(false);
        return ECObjectsStatus::Error;
        }

    // convert string value to desired type
    return propertyValue.ConvertToPrimitiveType(type) ? ECObjectsStatus::Success : ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::IsReadOnly(bool& isReadOnly, uint32_t index) const
    {
    ECValue v;
    auto status = GetValue(v, index, Index::IsReadOnly);
    isReadOnly = false;
    if (ECObjectsStatus::Success == status && v.IsBoolean() && !v.IsNull())
        isReadOnly = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::IsHidden(bool& isHidden, uint32_t index) const
    {
    ECValue v;
    auto status = GetValue(v, index, Index::IsHidden);
    isHidden = false;
    if (ECObjectsStatus::Success == status && v.IsBoolean() && !v.IsNull())
        isHidden = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetString(Utf8StringR str, uint32_t index, Index which) const
    {
    auto entry = GetEntry(index);
    return entry.IsValid() ? GetString(str, *entry, which) : ECObjectsStatus::PropertyNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetString(Utf8StringR str, IECInstanceCR instance, Index which) const
    {
    ECValue v;
    v.SetAllowsPointersIntoInstanceMemory(true);
    auto status = GetValue(v, instance, which);
    if (ECObjectsStatus::Success != status)
        return status;
    else if (!v.IsString() && !v.IsNull())
        return ECObjectsStatus::DataTypeMismatch;

    str = v.GetUtf8CP();
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetValue(ECValueR v, uint32_t index, Index which) const
    {
    auto entry = GetEntry(index);
    return entry.IsValid() ? GetValue(v, *entry, which) : ECObjectsStatus::PropertyNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyQuery::GetValue(ECValueR v, IECInstanceCR instance, Index which) const
    {
    Utf8CP propName = GetPropertyName(which);
    if (nullptr == propName)
        {
        if (IsRequiredMetadata(which))
            return ECObjectsStatus::Error;

        v.Clear();
        return ECObjectsStatus::Success;
        }
    else
        return instance.GetValue(v, propName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocPropertyEdit::AdHocPropertyEdit(IECInstanceR host, Utf8CP accessString)
    : AdHocPropertyQuery(host, accessString)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocPropertyEdit::AdHocPropertyEdit(IECInstanceR host, uint32_t propIdx)
    : AdHocPropertyQuery(host, propIdx)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::SetValue(uint32_t index, Utf8CP accessor, ECValueCR v)
    {
    auto entry = GetEntry(index);
    return entry.IsValid() ? entry->SetValue(accessor, v) : ECObjectsStatus::PropertyNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::SetName(uint32_t index, Utf8CP name)
    {
    if (!ECNameValidation::IsValidName(name))
        return ECObjectsStatus::Error;

    auto entry = GetEntry(index);
    if (entry.IsNull())
        return ECObjectsStatus::PropertyNotFound;
    else
        return entry->SetValue(GetPropertyName(Index::Name), ECValue(name, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::SetDisplayLabel(uint32_t index, Utf8CP label, bool andSetName)
    {
    auto entry = GetEntry(index);
    if (entry.IsNull())
        return ECObjectsStatus::PropertyNotFound;

    auto propName = GetPropertyName(Index::DisplayLabel);
    if (nullptr == propName)
        {
        if (!andSetName)
            return ECObjectsStatus::Error;
        }
    else
        {
        auto status = entry->SetValue(propName, ECValue(label, false));
        if (ECObjectsStatus::Success != status)
            return status;
        }

    if (andSetName)
        {
        Utf8String name;
        ECNameValidation::EncodeToValidName(name, label);
        return entry->SetValue(GetPropertyName(Index::Name), ECValue(name.c_str(), false));
        }
    else
        return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::SetValue(uint32_t index, ECValueCR inputV)
    {
    PrimitiveType type;
    auto status = GetPrimitiveType(type, index);
    if (ECObjectsStatus::Success != status)
        return status;

    ECValue v(inputV);
    Utf8String strRep;
    if (!v.ConvertToPrimitiveType(type) || !v.ConvertPrimitiveToString(strRep))
        return ECObjectsStatus::DataTypeMismatch;

    auto entry = GetEntry(index);
    if (entry.IsNull())
        return ECObjectsStatus::PropertyNotFound;

    return entry->SetValue(GetPropertyName(Index::Value), ECValue(strRep.c_str(), false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::SetIsReadOnly(uint32_t index, bool isReadOnly)
    {
    auto entry = GetEntry(index);
    if (entry.IsNull())
        return ECObjectsStatus::PropertyNotFound;

    auto propName = GetPropertyName(Index::IsReadOnly);
    if (nullptr == propName)
        return ECObjectsStatus::OperationNotSupported;

    return entry->SetValue(propName, ECValue(isReadOnly));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::SetIsHidden(uint32_t index, bool isHidden)
    {
    auto entry = GetEntry(index);
    if (entry.IsNull())
        return ECObjectsStatus::PropertyNotFound;

    auto propName = GetPropertyName(Index::IsHidden);
    if (nullptr == propName)
        return ECObjectsStatus::OperationNotSupported;

    return entry->SetValue(propName, ECValue(isHidden));
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct RevertAdHocProperty
    {
    private:
        AdHocPropertyEdit&  m_edit;
        uint32_t            m_index;
        bool                m_revert;
    public:
        RevertAdHocProperty(AdHocPropertyEdit& edit) : m_edit(edit), m_index(edit.GetCount()), m_revert(true) {}
        ~RevertAdHocProperty()
            {
            if (m_revert)
                m_edit.Remove(m_index);
            }

        void                Clear() { m_revert = false; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP AdHocPropertyQuery::GetStructClass() const
    {
    auto prop = GetHost().GetEnabler().LookupECProperty(GetContainerPropertyIndex());
    auto arrayProp = nullptr != prop ? prop->GetAsStructArrayProperty() : nullptr;
    auto structClass = nullptr != arrayProp ? &arrayProp->GetStructElementType() : nullptr;
    if (nullptr == structClass)
        BeAssert(false);

    return structClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr AdHocPropertyQuery::GetStructEnabler() const
    {
    auto structClass = GetStructClass();
    return GetHost().GetEnablerR().GetEnablerForStructArrayMember(structClass->GetSchema().GetSchemaKey(), structClass->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::Swap(uint32_t propIdxA, uint32_t propIdxB)
    {
    auto entryA = GetEntry(propIdxA), entryB = GetEntry(propIdxB);
    if (entryA.IsNull() || entryB.IsNull())
        return ECObjectsStatus::PropertyNotFound;

    ECValue v;
    v.SetStruct(entryA.get());
    auto status = GetHostR().SetValue(GetContainerPropertyIndex(), v, propIdxB);
    if (ECObjectsStatus::Success != status)
        return status;

    v.SetStruct(entryB.get());
    status = GetHostR().SetValue(GetContainerPropertyIndex(), v, propIdxA);
    if (ECObjectsStatus::Success != status)
        {
        v.SetStruct(entryA.get());
        GetHostR().SetValue(GetContainerPropertyIndex(), v, propIdxA);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::Add(Utf8CP name, ECValueCR v, Utf8CP displayLabel, Utf8CP unitName, Utf8CP extendedTypeName, bool isReadOnly, bool hidden)
    {
    if (!IsSupported())
        return ECObjectsStatus::OperationNotSupported;

    if (!ECNameValidation::IsValidName(name))
        return ECObjectsStatus::Error;

    auto type = PRIMITIVETYPE_String;
    if (!v.IsNull())
        {
        if (!v.IsPrimitive())
            return ECObjectsStatus::DataTypeMismatch;

        type = v.GetPrimitiveType();
        if (nullptr == GetPropertyName(Index::Type) && type != PRIMITIVETYPE_String)
            return ECObjectsStatus::OperationNotSupported;  // no property to hold type, so all properties are strings.
        }

    if (PRIMITIVETYPE_String != type && nullptr == GetPropertyName(Index::Type))
        return ECObjectsStatus::DataTypeMismatch;   // need a property to hold the type if it's not string...

    Utf8String strRep;
    ECValue vAsStr(v);
    if (!vAsStr.ConvertPrimitiveToString(strRep))
        return ECObjectsStatus::DataTypeMismatch;

    if (nullptr != unitName)
        {
        switch (type)
            {
                case PRIMITIVETYPE_Integer:
                case PRIMITIVETYPE_Long:
                case PRIMITIVETYPE_Double:
                    break;
                default:
                    return ECObjectsStatus::OperationNotSupported;  // type does not support units.
            }
        }

    // If a property already exists by the same name, we want to replace it (as per managed implementation).
    // And we want to preserve order within array, because that controls order in which ad-hocs are displayed in UI.
    uint32_t existingPropertyIndex = 0;
    bool replacing = GetPropertyIndex(existingPropertyIndex, name);

    auto status = replacing ? ECObjectsStatus::Success : GetHostR().AddArrayElements(GetContainerPropertyIndex(), 1);
    if (ECObjectsStatus::Success != status)
        return status;

    BeAssert(GetCount() > 0);

    // Ensure that if anything below fails, we remove the new struct array member.
    RevertAdHocProperty revert(*this);

    // Create a new struct array instance.
    auto enabler = GetStructEnabler();
    auto entry = enabler.IsValid() ? enabler->CreateInstance() : nullptr;
    if (entry.IsNull())
        {
        BeAssert(false);
        return ECObjectsStatus::Error;
        }

    if (ECObjectsStatus::Success != (status = entry->SetValue(GetPropertyName(Index::Name), ECValue(name, false))) ||
        ECObjectsStatus::Success != (status = entry->SetValue(GetPropertyName(Index::Value), ECValue(strRep.c_str(), false))))
        return status;

    if (nullptr != displayLabel && ECObjectsStatus::Success != (status = entry->SetValue(GetPropertyName(Index::DisplayLabel), ECValue(displayLabel, false))))
        return status;

    if (nullptr != unitName && ECObjectsStatus::Success != (status = entry->SetValue(GetPropertyName(Index::Unit), ECValue(unitName, false))))
        return status;

    if (nullptr != extendedTypeName && ECObjectsStatus::Success != (status = entry->SetValue(GetPropertyName(Index::ExtendType), ECValue(extendedTypeName, false))))
        return status;

    if (isReadOnly && ECObjectsStatus::Success != (status = entry->SetValue(GetPropertyName(Index::IsReadOnly), ECValue(isReadOnly))))
        return status;

    if (hidden && ECObjectsStatus::Success != (status = entry->SetValue(GetPropertyName(Index::IsHidden), ECValue(hidden))))
        return status;

    int32_t typeCode;
    if (PRIMITIVETYPE_String != type && (!CodeForPrimitiveType(typeCode, type) || ECObjectsStatus::Success != (status = entry->SetValue(GetPropertyName(Index::Type), ECValue(typeCode)))))
        return status;

    // set the struct to the array
    ECValue structV;
    structV.SetStruct(entry.get());
    status = GetHostR().SetValue(GetContainerPropertyIndex(), structV, replacing ? existingPropertyIndex : GetCount() - 1);
    if (ECObjectsStatus::Success != status)
        {
        BeAssert(false);
        return status;
        }

    revert.Clear();

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::Remove(uint32_t index)
    {
    if (!IsSupported())
        return ECObjectsStatus::OperationNotSupported;
    else if (index >= GetCount())
        return ECObjectsStatus::PropertyNotFound;
    else
        return GetHostR().RemoveArrayElement(GetContainerPropertyIndex(), index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::Clear()
    {
    return IsSupported() ? GetHostR().ClearArray(GetContainerPropertyIndex()) : ECObjectsStatus::OperationNotSupported;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdHocPropertyEdit::CopyFrom(AdHocPropertyQueryCR query, bool preserveValues)
    {
    if (!IsSupported() || !query.IsSupported())
        return ECObjectsStatus::Error;

    auto enabler = GetStructEnabler();
    if (enabler.IsNull())
        return ECObjectsStatus::Error;

    bmap<Utf8String, ECValue> preservedValues;
    if (preserveValues)
        {
        uint32_t count = GetCount();
        for (uint32_t i = 0; i < count; i++)
            {
            IECInstancePtr entry = GetEntry(i);
            Utf8String name;
            ECValue v;
            if (entry.IsValid() && ECObjectsStatus::Success == GetString(name, *entry, Index::Name) && ECObjectsStatus::Success == GetValue(v, *entry, Index::Value))
                preservedValues[name] = v;
            }
        }

    auto status = Clear();
    if (ECObjectsStatus::Success != status)
        return status;

    uint32_t count = query.GetCount();
    if (0 == count)
        return ECObjectsStatus::Success;

    status = GetHostR().AddArrayElements(GetContainerPropertyIndex(), count);
    if (ECObjectsStatus::Success != status)
        {
        BeAssert(false && "Failed to add array elements...existing values will be lost");
        return ECObjectsStatus::Error;
        }

    Utf8String name;
    bmap<Utf8String, ECValue>::const_iterator found;
    for (uint32_t i = 0; i < count; i++)
        {
        IECInstancePtr from = query.GetEntry(i);
        if (from.IsNull())
            {
            GetHostR().RemoveArrayElement(GetContainerPropertyIndex(), i);
            continue;
            }

        auto newEntry = enabler->CreateInstance();
        ECValue vStruct;
        vStruct.SetStruct(newEntry.get());
        if (newEntry.IsNull() || ECObjectsStatus::Success != newEntry->CopyValues(*from) || ECObjectsStatus::Success != GetHostR().SetValue(GetContainerPropertyIndex(), vStruct, i))
            {
            GetHostR().RemoveArrayElement(GetContainerPropertyIndex(), i);
            continue;
            }

        if (preserveValues && ECObjectsStatus::Success == query.GetString(name, *from, Index::Name) && preservedValues.end() != (found = preservedValues.find(name)))
            {
            ECValue vFound = found->second;
            PrimitiveType type;
            if (ECObjectsStatus::Success == GetPrimitiveType(type, i) && vFound.ConvertToPrimitiveType(type))
                SetValue(i, vFound);
            }
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AdHocContainerPropertyIndexCollection::const_iterator::const_iterator(ECEnablerCR enabler, bool isEnd)
    : m_enabler(enabler), m_current(isEnd ? 0 : enabler.GetFirstPropertyIndex(0))
    {
    if (!ValidateCurrent())
        MoveNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdHocContainerPropertyIndexCollection::const_iterator::ValidateCurrent() const
    {
    return !IsEnd() && AdHocPropertyMetadata::IsSupported(m_enabler, m_current);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AdHocContainerPropertyIndexCollection::const_iterator::MoveNext()
    {
    if (!IsEnd())
        {
        m_current = m_enabler.GetNextPropertyIndex(0, m_current);
        if (!IsEnd() && !ValidateCurrent())
            MoveNext();
        }
    }

END_BENTLEY_ECOBJECT_NAMESPACE
