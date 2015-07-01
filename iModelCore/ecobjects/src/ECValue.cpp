/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECValue.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <Bentley/BeAssert.h>
#include <Bentley/ValueFormat.h>
#include <GeomSerialization/GeomSerializationApi.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
enum ECValueStateFlags ENUM_UNDERLYING_TYPE(unsigned char)
    {
    ECVALUE_STATE_None                              = 0x00,
    ECVALUE_STATE_IsNull                            = 0x01,
    ECVALUE_STATE_IsReadOnly                        = 0x02,      // Really indicates that the property from which this came is readonly... not the value itself.
    ECVALUE_STATE_IsLoaded                          = 0x04,
    ECVALUE_STATE_AllowPointersIntoInstanceMemory   = 0x08
    };

enum ECValueOwnedDataFlags ENUM_UNDERLYING_TYPE(unsigned char)
    {
    ECVALUE_DATA_Binary         = 1 << 0,
    ECVALUE_DATA_Utf8           = 1 << 1,
    ECVALUE_DATA_Utf16          = 1 << 2,
#if !defined (_WIN32)
    ECVALUE_DATA_WChar          = 1 << 3,
#endif
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isDataOwned (uint8_t const& flags, ECValueOwnedDataFlags flag)        { return 0 != (flags & flag); }
static void setDataOwned (uint8_t& flags, ECValueOwnedDataFlags flag, bool owned) { flags = owned ? (flags | flag) : (flags & ~flag); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::SetWChar (WCharCP str, uint8_t& flags, bool owned)
    {
#if defined (_WIN32)
    SetUtf16 ((Utf16CP)str, flags, owned);
#else
    if (NULL == str)
        owned = false;

    setDataOwned (flags, ECVALUE_DATA_WChar, owned);
    m_wchar = owned ? BeStringUtilities::Wcsdup (str) : str;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::SetUtf16 (Utf16CP str, uint8_t& flags, bool owned)
    {
    if (NULL == str)
        owned = false;

    setDataOwned (flags, ECVALUE_DATA_Utf16, owned);
    if (!owned)
        m_utf16 = str;
    else
        {
        size_t size = 1;    // null terminator
#if defined (_WIN32)
        size += wcslen ((WCharCP)str);
#else
        while (0 != *(str++))
            ++size;

        str -= size;
#endif
        size *= sizeof(Utf16Char);
        m_utf16 = (Utf16CP)malloc (size);
        memcpy (const_cast<Utf16P>(m_utf16), str, size);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::SetUtf8 (Utf8CP str, uint8_t& flags, bool owned)
    {
    if (NULL == str)
        owned = false;

    setDataOwned (flags, ECVALUE_DATA_Utf8, owned);
    m_utf8 = owned ? BeStringUtilities::Strdup (str) : str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::ConvertToUtf8 (uint8_t& flags)
    {
    if (NULL == m_utf8)
        {
        Utf8String buf;
        if (NULL != m_utf16)
            {
            // ###TODO: eww...can we avoid this?
            WString wBuf;
            BeStringUtilities::Utf16ToWChar (wBuf, m_utf16);
            BeStringUtilities::WCharToUtf8 (buf, wBuf.c_str());
            }
#if !defined (_WIN32)
        else if (NULL != m_wchar)
            BeStringUtilities::WCharToUtf8 (buf, m_wchar);
#endif
        m_utf8 = BeStringUtilities::Strdup (buf.c_str());
        setDataOwned (flags, ECVALUE_DATA_Utf8, NULL != m_utf8);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::ConvertToUtf16 (uint8_t& flags)
    {
    if (NULL == m_utf16)
        {
        Utf16Buffer buf;
        if (NULL != m_utf8)
            {
            if (0 == *m_utf8)       // BeStringUtilities will give us back an empty buffer for an empty string...not what we want
                buf.push_back (0);
            else
                BeStringUtilities::Utf8ToUtf16 (buf, m_utf8);
            }
#if !defined (_WIN32)
        else if (NULL != m_wchar)
            BeStringUtilities::WCharToUtf16 (buf, m_wchar);
#endif

        if (!buf.empty())
            {
            size_t size = buf.size() * sizeof(Utf16Char);
            m_utf16 = (Utf16CP)malloc (size);
            memcpy (const_cast<Utf16P>(m_utf16), &buf[0], size);
            setDataOwned (flags, ECVALUE_DATA_Utf16, true);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::StringInfo::FreeAndClear (uint8_t& flags)
    {
    if (isDataOwned (flags, ECVALUE_DATA_Utf16))
        free (const_cast<Utf16P>(m_utf16));

    if (isDataOwned (flags, ECVALUE_DATA_Utf8))
        free (const_cast<Utf8P>(m_utf8));

#if !defined (_WIN32)
    if (isDataOwned (flags, ECVALUE_DATA_WChar))
        free (const_cast<WCharP>(m_wchar));
#endif

    flags = 0;
    SetNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
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
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
bool ECValue::StringInfo::IsUtf8 () const
    {
    return m_utf8 != NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECValue::StringInfo::GetWChar (uint8_t& flags)
    {
#if defined (_WIN32)
    return (WCharCP)GetUtf16 (flags);
#else
    if (NULL == m_wchar)
        {
        // ###TODO: Note we do a copy into the WString, and then another copy from WString to our buffer
        // Do we have reliable methods for determining the required number of bytes required to store the converted string which would allow us to avoid using WString?
        WString buf;
        if (NULL != m_utf8)
            BeStringUtilities::Utf8ToWChar (buf, m_utf8);
        else if (NULL != m_utf16)
            BeStringUtilities::Utf16ToWChar (buf, m_utf16);
        else
            return NULL;

        m_wchar = BeStringUtilities::Wcsdup (buf.c_str());
        setDataOwned (flags, ECVALUE_DATA_WChar, true);
        }
            
    return m_wchar;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECValue::StringInfo::GetUtf8 (uint8_t& flags)
    {
    ConvertToUtf8 (flags);  // if we already have Utf8 this does nothing
    return m_utf8;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf16CP ECValue::StringInfo::GetUtf16 (uint8_t& flags)
    {
    ConvertToUtf16 (flags); // if we already have Utf16 this does nothing
    return m_utf16;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::StringInfo::Equals (ECValue::StringInfo const& rhs, uint8_t& flags)
    {
    // We already know both strings are not null
    // Depending on the encodings held by each StringInfo we may need to perform conversion...might as well store the converted value while we're at it.
    if (NULL != rhs.m_utf8)
        {
        if (NULL != m_utf8)
            return 0 == strcmp (m_utf8, rhs.m_utf8);
        else
            {
            ConvertToUtf8 (flags);
            return Equals (rhs, flags);
            }
        }
    else if (NULL != rhs.m_utf16)
        {
        if (NULL != m_utf16)
            return 0 == BeStringUtilities::CompareUtf16 (m_utf16, rhs.m_utf16);
#if !defined (_WIN32)
        else if (NULL != m_wchar)
            return 0 == BeStringUtilities::CompareUtf16WChar (rhs.m_utf16, m_wchar);
#endif
        else
            {
            ConvertToUtf16 (flags);
            return Equals (rhs, flags);
            }
        }
#if !defined (_WIN32)
    else if (NULL != rhs.m_wchar)
        {
        if (NULL != m_wchar)
            return 0 == wcscmp (m_wchar, rhs.m_wchar);
        else if (NULL != m_utf16)
            return 0 == BeStringUtilities::CompareUtf16WChar (m_utf16, rhs.m_wchar);
        else
            {
            ConvertToUtf16 (flags);
            return Equals (rhs, flags);
            }
        }
#endif

    BeAssert (false && "It should not be possible to compare StringInfos where one or both contains all null strings");
    return false;
    }

//*********************** ECValue::DateTimeInfo ***************************************
//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
void ECValue::DateTimeInfo::Set (::int64_t ceTicks)
    {
    m_ceTicks = ceTicks;
    m_isMetadataSet = false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECValue::DateTimeInfo::Set (DateTimeCR dateTime)
    {
    //No support for local DateTimes (yet?) as client might expect this to do time zone
    //conversions - which we want the client / application side to do as it is nearly
    //impossible to do time zone conversions right in a generic and portable way.
    PRECONDITION (dateTime.GetInfo ().GetKind () != DateTime::Kind::Local, ERROR);

    int64_t ceTicks = 0LL;
    BentleyStatus stat = dateTime.ToCommonEraTicks (ceTicks);
    if (stat != SUCCESS)
        {
        return stat;
        }

    Set (ceTicks);
    return SetMetadata (dateTime.GetInfo ());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
::int64_t ECValue::DateTimeInfo::GetCETicks () const
    {
    return m_ceTicks;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECValue::DateTimeInfo::GetDateTime (DateTimeR dateTime) const
    {
    DateTime::Info info;
    if (!TryGetMetadata (info))
        {
        info = ECN::DateTimeInfo::GetDefault ();
        }

    return DateTime::FromCommonEraTicks (dateTime, m_ceTicks, info);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
bool ECValue::DateTimeInfo::TryGetMetadata (DateTime::Info& metadata) const
    {
    if (!IsMetadataSet ())
        {
        return false;
        }

    metadata = DateTime::Info (m_kind, m_component);
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECValue::DateTimeInfo::SetMetadata (DateTimeInfoCR caMetadata)
    {
    //DateTimeInfo::IsNull indicates whether the metadata is unset or not. Only if it is not unset, store anything
    //in the ECValue
    if (caMetadata.IsNull ())
        return SUCCESS;

    DateTime::Info metadata = caMetadata.GetInfo (true);
    return SetMetadata (metadata);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ECValue::DateTimeInfo::SetMetadata (DateTime::Info const& metadata)
    {
    //No support for local DateTimes (yet?) as client might expect this to do time zone
    //conversions - which we want the client / application side to do as it is nearly
    //impossible to do time zone conversions right in a generic and portable way.
    if (metadata.GetKind () == DateTime::Kind::Local)
        {
        LOG.error (L"DateTime kind 'Local' not supported.");
        BeAssert (false && L"DateTime kind 'Local' not supported.");
        return ERROR;
        }

    m_kind = metadata.GetKind ();
    m_component = metadata.GetComponent ();
    m_isMetadataSet = true;

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
bool ECValue::DateTimeInfo::MetadataMatches (DateTimeInfoCR caDateTimeMetadata) const
    {
    DateTime::Info const& rhsInfo = caDateTimeMetadata.GetInfo ();

    //if ECValue doesn't have meta data (e.g. in case when SetDateTimeTicks was used to populate it),
    //no metadata check will be done. I.e. the CA metadata will implicitly become the CA of the ECValue.
    return !m_isMetadataSet || ((caDateTimeMetadata.IsKindNull () || m_kind == rhsInfo.GetKind ()) &&
        (caDateTimeMetadata.IsComponentNull () || m_component == rhsInfo.GetComponent ()));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+-------
WString ECValue::DateTimeInfo::MetadataToString () const
    {
    DateTime::Info metadata;
    if (!TryGetMetadata (metadata))
        {
        return L"";
        }

    WString str;
    //reserve for the longest possible string 
    str.reserve (36);
    str.append (L"Kind: ");
    str.append (DateTime::Info::KindToString (m_kind));
    str.append (L" Component: ");
    str.append (DateTime::Info::ComponentToString (m_component));

    return str;
    }

//*********************** ECValue ***************************************

/*---------------------------------------------------------------------------------**//**
*  Really indicates that the property from which this came is readonly... not the value itself.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetIsReadOnly(bool isReadOnly) 
    { 
    if (isReadOnly)
        m_stateFlags |= ((uint8_t)ECVALUE_STATE_IsReadOnly); 
    else
        m_stateFlags &= ~((uint8_t)ECVALUE_STATE_IsReadOnly); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsReadOnly() const 
    { 
    return 0 != (m_stateFlags & ECVALUE_STATE_IsReadOnly); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetIsNull(bool isNull)  
    { 
    if (isNull)
        m_stateFlags |= ((uint8_t)ECVALUE_STATE_IsNull); 
    else
        m_stateFlags &= ~((uint8_t)ECVALUE_STATE_IsNull); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsNull() const 
    { 
    return 0 != (m_stateFlags & ECVALUE_STATE_IsNull); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetIsLoaded(bool isLoaded)
    { 
    if (isLoaded)
        m_stateFlags |= ((uint8_t)ECVALUE_STATE_IsLoaded); 
    else
        m_stateFlags &= ~((uint8_t)ECVALUE_STATE_IsLoaded); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::AllowsPointersIntoInstanceMemory() const
    {
    return 0 != (m_stateFlags & ECVALUE_STATE_AllowPointersIntoInstanceMemory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::SetAllowsPointersIntoInstanceMemory (bool allow)
    {
    if (allow)
        m_stateFlags |= ((uint8_t)ECVALUE_STATE_AllowPointersIntoInstanceMemory);
    else
        m_stateFlags &= ~((uint8_t)ECVALUE_STATE_AllowPointersIntoInstanceMemory);
    }

/*---------------------------------------------------------------------------------**//**
* It is up to the instance implementation to set this flag. For MemoryBased instances
* this bit is set in the _GetValue method when it checks the IsLoaded flag for the 
* property.
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsLoaded() const 
    { 
    return 0 != (m_stateFlags & ECVALUE_STATE_IsLoaded); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ValueKind       ECValue::GetKind() const 
    { 
    return (ValueKind) (m_valueKind & 0xFF);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsUninitialized () const 
    { 
    return GetKind() == VALUEKIND_Uninitialized; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsString () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_String; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/13
//+---------------+---------------+---------------+---------------+---------------+------
bool            ECValue::IsUtf8 () const 
    { 
    return IsString () && m_stringInfo.IsUtf8 ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsBinary () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Binary; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsInteger () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Integer; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsLong () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Long; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsDouble () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Double; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsBoolean () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Boolean; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsPoint2D () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Point2D; 
    }
                                       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsPoint3D () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Point3D; 
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsDateTime () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_DateTime; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsIGeometry() const
    {
    return m_primitiveType == PRIMITIVETYPE_IGeometry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsArray () const 
    { 
    return GetKind() == VALUEKIND_Array; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsStruct () const 
    { 
    return GetKind() == VALUEKIND_Struct; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsPrimitive () const 
    { 
    return GetKind() == VALUEKIND_Primitive; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::ConstructUninitialized()
    {
#ifndef NDEBUG
    int size = sizeof (ECValue);
    memset (this, 0xBAADF00D, size); // avoid accidental misinterpretation of uninitialized data
#endif
    m_valueKind  = VALUEKIND_Uninitialized;
    m_stateFlags = ECVALUE_STATE_IsNull;
    m_ownershipFlags = 0;
    }
    

/*---------------------------------------------------------------------------------**//**
* Copies this value object without allocating any additional memory.  The copy will
* hold copies on any external pointers held by the original.
* @bsimethod                                                    JoshSchifter    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::ShallowCopy (ECValueCR v)
    {     
    if (this == &v)
        return;

    memcpy (this, &v, sizeof(ECValue));

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
            if (isDataOwned (m_ownershipFlags, ECVALUE_DATA_Binary))
                {
                setDataOwned (m_ownershipFlags, ECVALUE_DATA_Binary, false);
                SetBinary (v.m_binaryInfo.m_data, v.m_binaryInfo.m_size, true);
                }
            break;
            }

        case PRIMITIVETYPE_String:
            {
            // NB: Original comment: "Only make a copy of the string if the original object had a copy."
            // ^ We don't know the lifetime of the original object vs. that of 'this', so we must always copy.

            // ###TODO: only copy the preferred encoding? Copy all encodings contained in other StringInfo?
            m_ownershipFlags = 0; // prevent FreeAndClear() from attempting to free the strings that were temporarily copied by memset
            m_stringInfo.FreeAndClear (m_ownershipFlags);
            if (NULL != v.m_stringInfo.m_utf8)
                SetUtf8CP (v.m_stringInfo.m_utf8);
            else if (NULL != v.m_stringInfo.m_utf16)
                SetUtf16CP (v.m_stringInfo.m_utf16);
#if !defined (_WIN32)
            else if (NULL != v.m_stringInfo.m_wchar)
                SetString (v.m_stringInfo.m_wchar);
#endif
            else
                SetString (NULL);

            break;
            }

        // the memcpy takes care of these...            
        case VALUEKIND_Array:
        case PRIMITIVETYPE_Boolean:
        case PRIMITIVETYPE_Integer:
        case PRIMITIVETYPE_Long:
        case PRIMITIVETYPE_Double:
        case PRIMITIVETYPE_Point2D:
        case PRIMITIVETYPE_Point3D:
        case PRIMITIVETYPE_DateTime:
        case PRIMITIVETYPE_IGeometry:
            break;
                        
        default:
            BeAssert (false); // type not handled
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::FreeMemory ()
    {
    if (IsNull())
        return;

    unsigned short primitiveType = m_primitiveType;
    switch (primitiveType)
        {
        case PRIMITIVETYPE_String:
            m_stringInfo.FreeAndClear (m_ownershipFlags);
            return;

        case PRIMITIVETYPE_Binary:
        case PRIMITIVETYPE_IGeometry:
            if (NULL != m_binaryInfo.m_data && isDataOwned (m_ownershipFlags, ECVALUE_DATA_Binary))
                free ((void*)m_binaryInfo.m_data);
            return;

        case VALUEKIND_Struct:
            if ((m_structInstance != NULL))
                m_structInstance->Release();
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetToNull()
    {        
    FreeMemory ();
    SetIsNull (true);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
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

    FreeMemory ();
    m_stateFlags = newStateFlags;
    m_valueKind = VALUEKIND_Uninitialized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::~ECValue()
    {
    FreeMemory ();
#ifndef NDEBUG
    memset (this, 0xBAADF00D, sizeof(ECValue)); // try to make use of destructed values more obvious
#endif
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueR ECValue::operator= (ECValueCR rhs)
    {
    From (rhs);
    return *this;
    }        
    
/*---------------------------------------------------------------------------------**//**
* Construct an uninitialized value
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue ()
    {
    ConstructUninitialized();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (ECValueCR v)
    {
    ConstructUninitialized();
    From (v);
    }
    
/*---------------------------------------------------------------------------------**//**
*  Construct a Null ECN::ECValue (of a specific type, but with IsNull = true)
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (ValueKind classification) : m_valueKind(classification), m_stateFlags(ECVALUE_STATE_IsNull), m_ownershipFlags(0)
    {
    }       

/*---------------------------------------------------------------------------------**//**
*  Construct a Null ECN::ECValue (of a specific type, but with IsNull = true)
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (PrimitiveType primitiveType) : m_primitiveType(primitiveType), m_stateFlags(ECVALUE_STATE_IsNull), m_ownershipFlags(0)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (::int32_t integer32)
    {
    ConstructUninitialized();
    SetInteger (integer32);
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (::int64_t long64)
    {
    ConstructUninitialized();
    SetLong (long64);
    };
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (double doubleVal)
    {
    ConstructUninitialized();
    SetDouble (doubleVal);
    };
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (DPoint2dCR point2d)
    {
    ConstructUninitialized();
    SetPoint2D (point2d);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (DPoint3dCR point3d)
    {
    ConstructUninitialized();
    SetPoint3D (point3d);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (bool value)
    {
    ConstructUninitialized();
    SetBoolean (value);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Krischan.Eberle                  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (DateTimeCR dateTime)
    {
    ConstructUninitialized();
    SetDateTime (dateTime);
    };

/*---------------------------------------------------------------------------------**//**
* @param holdADuplicate     If true, ECN::ECValue will make a duplicate, otherwise 
* ECN::ECValue holds the original pointer. Intended only for use when initializing arrays of strings, to avoid duplicating them twice.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (WCharCP string, bool holdADuplicate) //needswork: add an overload that takes utf8
    {
    ConstructUninitialized();
    SetString (string, holdADuplicate);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (Utf8CP string, bool holdADuplicate)
    {
    ConstructUninitialized();
    SetUtf8CP (string, holdADuplicate);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (Utf16CP string, bool holdADuplicate)
    {
    ConstructUninitialized();
    SetUtf16CP (string, holdADuplicate);
    };

/*---------------------------------------------------------------------------------**//**
* The ECValue constructed by this overload is not responsible for freeing the memory... its creator is. 
* The consumer of the ECValue should make a copy of the memory.
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (const Byte * data, size_t size)
    {
    ConstructUninitialized();
    SetBinary (data, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECValue::From (ECValueCR v)
    {
    ShallowCopy (v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType   ECValue::GetPrimitiveType() const
    {
    PRECONDITION (IsPrimitive() && "Tried to get the primitive type of an ECN::ECValue that is not classified as a primitive.", (PrimitiveType)0);    
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    10/10
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus       ECValue::SetPrimitiveType (PrimitiveType primitiveType)
    {
    if (m_primitiveType != primitiveType)
        {
        Clear();
        m_primitiveType = primitiveType;
        }
        
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
::int32_t       ECValue::GetInteger() const
    {
    PRECONDITION (IsInteger() && "Tried to get integer value from an ECN::ECValue that is not an integer.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_integer32;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetInteger (::int32_t integer)
    {
    Clear();
    SetIsNull (false);
    m_primitiveType  = PRIMITIVETYPE_Integer;
    m_integer32 = integer;
    
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
::int64_t       ECValue::GetLong() const
    {
    PRECONDITION (IsLong() && "Tried to get long64 value from an ECN::ECValue that is not an long64.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_long64;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetLong (::int64_t long64)
    {
    Clear();
    SetIsNull (false);
    m_primitiveType  = PRIMITIVETYPE_Long;
    m_long64    = long64;
    
    return SUCCESS;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
double          ECValue::GetDouble() const
    {
    PRECONDITION (IsDouble() && "Tried to get double value from an ECN::ECValue that is not an double.", std::numeric_limits<double>::quiet_NaN());
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", std::numeric_limits<double>::quiet_NaN());
    return m_double;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetDouble (double value)
    {
    Clear();
    SetIsNull (false);
    m_primitiveType  = PRIMITIVETYPE_Double;
    m_double    = value;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool          ECValue::GetBoolean() const
    {
    PRECONDITION (IsBoolean() && "Tried to get boolean value from an ECN::ECValue that is not a boolean.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_boolean;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetBoolean (bool value)
    {
    Clear();
    SetIsNull (false);
    m_primitiveType  = PRIMITIVETYPE_Boolean;
    m_boolean        = value;
    
    return SUCCESS;
    }

//////////////////////////////////////////////////////////////////////////////////////////
// DateTime   - The DateTime ticks are the number of 100-nanosecond intervals 
//              that have elapsed since the beginning of the Common Era epoch (0001-01-01 00:00:00 UTC)
//              (This is the same as in managed ECObjects and .NET's System.DateTime respectively)
//////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECValue::GetDateTimeTicks () const
    {
    PRECONDITION (IsDateTime() && "Tried to get DateTime value from an ECN::ECValue that is not a DateTime.", 0LL);
    PRECONDITION (!IsNull(), 0LL);
    return m_dateTimeInfo.GetCETicks ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ECValue::GetDateTimeTicks (bool& hasMetadata, DateTime::Info& metadata) const
    {
    const int64_t ceTicks = GetDateTimeTicks ();

    hasMetadata = m_dateTimeInfo.TryGetMetadata (metadata);

    return ceTicks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Krischan.Eberle             10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime ECValue::GetDateTime () const
    {
    PRECONDITION (IsDateTime() && "Tried to get DateTime value from an ECN::ECValue that is not a DateTime.", DateTime ());
    PRECONDITION (!IsNull(), DateTime ());

    DateTime dateTime;
    BentleyStatus stat = m_dateTimeInfo.GetDateTime (dateTime);
    POSTCONDITION (stat == SUCCESS, DateTime ());

    return dateTime;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECValue::GetDateTimeUnixMillis () const
    {
    PRECONDITION (IsDateTime() && "Tried to get DateTime value from an ECN::ECValue that is not a DateTime.", 0LL);
    PRECONDITION (!IsNull(), 0LL);

    int64_t commonEraTicks = m_dateTimeInfo.GetCETicks ();
    uint64_t jdInHns = DateTime::CommonEraTicksToJulianDay (commonEraTicks);
    return DateTime::JulianDayToUnixMilliseconds (jdInHns);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetDateTimeTicks (int64_t ceTicks)
    {
    Clear ();
    SetIsNull (false);
    m_primitiveType = PRIMITIVETYPE_DateTime;
    m_dateTimeInfo.Set (ceTicks);
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus ECValue::SetDateTimeTicks (int64_t ceTicks, DateTime::Info const& dateTimeMetadata) 
    {
    SetDateTimeTicks (ceTicks);
    return m_dateTimeInfo.SetMetadata (dateTimeMetadata);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus          ECValue::SetDateTime (DateTimeCR dateTime) 
    {
    Clear ();
    m_primitiveType = PRIMITIVETYPE_DateTime;
    //in case of error, keep IsNull set.
    BentleyStatus stat = m_dateTimeInfo.Set (dateTime);
    if (stat == SUCCESS)
        {
        SetIsNull (false);
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus           ECValue::SetLocalDateTimeFromUnixMillis (int64_t millis)
    {
    // Old PropertyEnablers would convert properties like file create date or element modified time to local time
    // We reluctantly reproduce that behavior to avoid regressions. Incoming time is UTC
    DateTime utc;
    if (SUCCESS == DateTime::FromUnixMilliseconds (utc, millis))
        {
        DateTime local;
        if (SUCCESS != utc.ToLocalTime (local))
            local = utc;
        else
            {
            // Uhh...we don't actually support local DateTime values? Yet we produce them? See assertion in ECValue::SetDateTime()
            // So convert the value back to "unspecified" DateTime.
            local = DateTime (DateTime::Kind::Unspecified, local.GetYear(), local.GetMonth(), local.GetDay(), local.GetHour(), local.GetMinute(), local.GetSecond(), local.GetHectoNanosecond());
            }

        return SetDateTime (local);
        }
    else
        {
        Clear();
        m_primitiveType = PRIMITIVETYPE_DateTime;
        return ERROR;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus ECValue::SetDateTimeMetadata (DateTimeInfoCR dateTimeInfo)
    {
    PRECONDITION (IsDateTime () && !IsNull (), ERROR);
    return m_dateTimeInfo.SetMetadata (dateTimeInfo);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+-----
bool ECValue::IsDateTimeMetadataSet () const
    {
    return IsDateTime () && !IsNull () && m_dateTimeInfo.IsMetadataSet ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+-----
bool ECValue::DateTimeInfoMatches (DateTimeInfoCR dateTimeInfo) const
    {
    return IsDateTime () && !IsNull () && m_dateTimeInfo.MetadataMatches (dateTimeInfo);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+-----
WString ECValue::DateTimeMetadataToString () const
    {
    if (!IsDateTime () || IsNull ())
        {
        return L"";
        }

    return m_dateTimeInfo.MetadataToString ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d          ECValue::GetPoint2D() const
    {
    DPoint2d badValue = {0.0,0.0};
    PRECONDITION (IsPoint2D() && "Tried to get Point2D value from an ECN::ECValue that is not a Point2D.", badValue);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", badValue);
    return m_dPoint2d;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetPoint2D (DPoint2dCR value)
    {
    Clear();
    SetIsNull (false);
    m_primitiveType  = PRIMITIVETYPE_Point2D;
    m_dPoint2d       = value;
    
    return SUCCESS;
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d          ECValue::GetPoint3D() const
    {
    DPoint3d badValue = {0.0,0.0,0.0};

    PRECONDITION (IsPoint3D() && "Tried to get Point3D value from an ECN::ECValue that is not a Point3D.", badValue);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", badValue);
    return m_dPoint3d;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetPoint3D (DPoint3dCR value)
    {
    Clear();
    SetIsNull (false);
    m_primitiveType  = PRIMITIVETYPE_Point3D;
    m_dPoint3d       = value;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECValue::GetString() const
    {
    PRECONDITION (IsString() && "Tried to get string value from an ECN::ECValue that is not a string.", L"<Programmer Error: Attempted to get string value from ECN::ECValue that is not a string.>");
    return IsNull() ? NULL : m_stringInfo.GetWChar (m_ownershipFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   09/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ECValue::OwnsString () const
    {
    if (!IsString ())
        {
        BeAssert (false && L"<Programmer Error: Attempted to call string related method from ECN::ECValue that is not a string.>");
        return false;
        }

    if (IsNull ())
        return false;

    
#if defined (_WIN32)
    const ECValueOwnedDataFlags flags = ECVALUE_DATA_Utf16;
#else
    const ECValueOwnedDataFlags flags = ECVALUE_DATA_WChar;
#endif

    return isDataOwned (m_ownershipFlags, flags);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP  ECValue::GetUtf8CP() const
    {
    if (!IsString())
        {
        BeAssert (false && L"<Programmer Error: Attempted to get string value from ECN::ECValue that is not a string.>");
        return NULL;
        }

    return IsNull() ? NULL : m_stringInfo.GetUtf8 (m_ownershipFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   09/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ECValue::OwnsUtf8CP () const
    {
    if (!IsString ())
        {
        BeAssert (false && L"<Programmer Error: Attempted to call string related method from ECN::ECValue that is not a string.>");
        return false;
        }

    return IsNull () ? false : isDataOwned (m_ownershipFlags, ECVALUE_DATA_Utf8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf16CP ECValue::GetUtf16CP() const
    {
    if (!IsString())
        {
        BeAssert (false && L"<Programmer Error: Attempted to get string value from ECN::ECValue that is not a string.>");
        return NULL;
        }

    return IsNull() ? NULL : m_stringInfo.GetUtf16 (m_ownershipFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   09/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ECValue::OwnsUtf16CP () const
    {
    if (!IsString ())
        {
        BeAssert (false && L"<Programmer Error: Attempted to call string related method from ECN::ECValue that is not a string.>");
        return false;
        }

    return IsNull () ? false : isDataOwned (m_ownershipFlags, ECVALUE_DATA_Utf16);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValue::InitForString (void const * str)
    {
    Clear();
    m_stringInfo.SetNull();
    m_primitiveType = PRIMITIVETYPE_String;
    SetIsNull (NULL == str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetString (WCharCP string, bool holdADuplicate)
    {
    InitForString (string);
    m_stringInfo.SetWChar (string, m_ownershipFlags, holdADuplicate);
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetUtf8CP (Utf8CP string, bool holdADuplicate)
    {
    InitForString (string);
    m_stringInfo.SetUtf8 (string, m_ownershipFlags, holdADuplicate);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetUtf16CP (Utf16CP string, bool holdADuplicate)
    {
    InitForString (string);
    m_stringInfo.SetUtf16 (string, m_ownershipFlags, holdADuplicate);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                 1/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ECValue::GetIGeometry() const
    {
    PRECONDITION(IsIGeometry() && "Tried to get an IGeometry from an ECN::ECValue that is not geometry", NULL);
    bvector<Byte> geomBuffer;
    geomBuffer.resize(m_binaryInfo.m_size);
    memcpy(&geomBuffer[0], m_binaryInfo.m_data, m_binaryInfo.m_size);

    return BentleyGeometryFlatBuffer::BytesToGeometry (geomBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                 1/2015
+---------------+---------------+---------------+---------------+---------------+------*/
const Byte * ECValue::GetIGeometry(size_t& size) const
    {
    PRECONDITION (IsIGeometry() && "Tried to get binarydata from an ECN::ECValue that is not binary.", NULL);
    size = m_binaryInfo.m_size;
    return m_binaryInfo.m_data;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetIGeometry(const Byte * data, size_t size, bool holdADuplicate)
    {
    Clear();

    m_primitiveType = PRIMITIVETYPE_IGeometry;
    return SetBinaryInternal(data, size, holdADuplicate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                 1/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetIGeometry(IGeometryCR geometry)
    {
    Clear();
    m_primitiveType = PRIMITIVETYPE_IGeometry;
    bvector<Byte> buffer;
    BentleyGeometryFlatBuffer::GeometryToBytes (geometry, buffer);
    return SetBinaryInternal(buffer.data(), buffer.size(), true);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
const Byte * ECValue::GetBinary(size_t& size) const
    {
    PRECONDITION ((IsBinary() || IsIGeometry()) && "Tried to get binarydata from an ECN::ECValue that is not binary.", NULL);
    size = m_binaryInfo.m_size;
    return m_binaryInfo.m_data;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetBinary (const Byte * data, size_t size, bool holdADuplicate)
    {
    Clear();

    m_primitiveType = PRIMITIVETYPE_Binary;
    return SetBinaryInternal(data, size, holdADuplicate);
    }

BentleyStatus ECValue::SetBinaryInternal(const Byte * data, size_t size, bool holdADuplicate)
    {
    setDataOwned (m_ownershipFlags, ECVALUE_DATA_Binary, holdADuplicate);
    
    if (NULL == data)
        {
        m_binaryInfo.m_data = NULL;
        m_binaryInfo.m_size = 0;
        return SUCCESS;
        }    
    

    SetIsNull (false);

    m_binaryInfo.m_size = size;    
    if (holdADuplicate)
        {
        void * duplicateData = malloc (size);
        memcpy (duplicateData, data, size);        
        m_binaryInfo.m_data = (const Byte *)duplicateData;
        }
    else
        m_binaryInfo.m_data = data;

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  ECValue::GetStruct() const
    {
    PRECONDITION (IsStruct() && "Tried to get struct value from an ECN::ECValue that is not a struct.", 0);
    return m_structInstance;    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetStruct (IECInstanceP structInstance)
    {
    Clear();

    m_valueKind = VALUEKIND_Struct;    
        
    if (NULL == structInstance)
        {
        m_structInstance = NULL;
        return SUCCESS;
        }  
            
    SetIsNull (false);

    m_structInstance = structInstance;        
    m_structInstance->AddRef();
    
    return SUCCESS;
    }    

extern void convertByteArrayToString (WStringR outString, const Byte *byteData, size_t numBytes);
extern bool convertStringToByteArray (bvector<Byte>& byteData, WCharCP stringData);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECValue::ConvertPrimitiveToString (WStringR str) const
    {
    PRECONDITION (IsPrimitive() && "ECValue::ConvertPrimitiveToString() requires a primitive value", false);    
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
        Byte const* bytes = GetBinary (nBytes);
        if (NULL != bytes)
            convertByteArrayToString (str, bytes, nBytes);
        }
        break;
    case PRIMITIVETYPE_Boolean:
        str = GetBoolean() ? L"True" : L"False";
        break;
    case PRIMITIVETYPE_DateTime:
        str.Sprintf (L"%lld", GetDateTimeTicks());
        break;
    case PRIMITIVETYPE_Double:
        str.Sprintf (L"%.17g", GetDouble());
        break;
    case PRIMITIVETYPE_Integer:
        str.Sprintf (L"%d", GetInteger());
        break;
    case PRIMITIVETYPE_Long:
        str.Sprintf (L"%lld", GetLong());
        break;
    case PRIMITIVETYPE_Point2D:
        {
        DPoint2d pt = GetPoint2D();
        str.Sprintf (L"%.17g,%.17g", pt.x, pt.y);
        }
        break;
    case PRIMITIVETYPE_Point3D:
        {
        DPoint3d pt = GetPoint3D();
        str.Sprintf (L"%.17g,%.17g,%.17g", pt.x, pt.y, pt.z);
        }
        break;
    case PRIMITIVETYPE_String:
        str = GetString();
        break;
    default:
        BeAssert (false);
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
static WString formatDouble (double d)
    {
    WString str;
    str.Sprintf (L"%.17f", d);
    auto dotPos = str.find ('.');
    if (WString::npos != dotPos)
        {
        auto nonZeroPos = str.length() - 1;
        while (nonZeroPos > dotPos && str[nonZeroPos] == '0')
            --nonZeroPos;

        if (nonZeroPos == dotPos)
            //We need to keep ".0" at the end to make sure we keep information about the value type,
            //this will be needed during value parsing.
            str.erase (dotPos + 2, WString::npos);
        else if (nonZeroPos < str.length() - 1)
            str.erase (nonZeroPos+1, WString::npos);
        }

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::ConvertPrimitiveToECExpressionLiteral (WStringR expr) const
    {
    if (IsNull())
        {
        expr = L"Null";
        return true;
        }

    PRECONDITION (IsPrimitive() && "ECValue::ConvertPrimitiveToECExpressionLiteral requires a primitive value", false);
    
    switch (GetPrimitiveType())
        {
    case PRIMITIVETYPE_Boolean:     expr = GetBoolean() ? L"True" : L"False"; return true;
    case PRIMITIVETYPE_DateTime:    expr.Sprintf (L"@%lld", GetDateTimeTicks()); return true;
    case PRIMITIVETYPE_Double:      expr = formatDouble (GetDouble()); return true;
    case PRIMITIVETYPE_Integer:     expr.Sprintf (L"%d", GetInteger()); return true;
    case PRIMITIVETYPE_Long:        expr.Sprintf (L"%lld", GetLong()); return true;
    case PRIMITIVETYPE_Point2D:     expr.Sprintf (L"{%ls,%ls}", formatDouble (GetPoint2D().x).c_str(), formatDouble (GetPoint2D().y).c_str()); return true;
    case PRIMITIVETYPE_Point3D:
        {
        DPoint3d pt = GetPoint3D();
        expr.Sprintf (L"{%ls,%ls,%ls}", formatDouble(pt.x).c_str(), formatDouble(pt.y).c_str(), formatDouble(pt.z).c_str());
        }
        return true;
    case PRIMITIVETYPE_String:
        {
        // Must escape quotes...
        WString s (GetString());
        s.ReplaceAll(L"\"", L"\"\"");
        expr.Sprintf (L"\"%ls\"", s.c_str());
        }
        return true;
    case PRIMITIVETYPE_Binary:
    case PRIMITIVETYPE_IGeometry:
        return false;
    default:
        BeAssert (false && L"Unsupported PrimitiveType");
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::ConvertToPrimitiveFromString (PrimitiveType primitiveType)
    {
    if (!IsString())
        {
        BeAssert (false);   // It's a private method, ought to have checked preconditions...
        return false;
        }
    else if (PRIMITIVETYPE_String == primitiveType)
        return true;
    else if (IsNull())
        {
        SetPrimitiveType (primitiveType);
        return true;
        }
            
    WCharCP str = GetString();
    switch (primitiveType)
        {
    case PRIMITIVETYPE_Binary:
        {
        bvector<Byte> bytes;
        if (!convertStringToByteArray (bytes, str))
            return false;

        SetBinary (&bytes.front(), bytes.size(), true);
        }
        break;
    case PRIMITIVETYPE_Boolean:
        if (0 == BeStringUtilities::Wcsicmp (L"true", str) || 0 == wcscmp (L"1", str))
            SetBoolean (true);
        else if (0 == BeStringUtilities::Wcsicmp (L"false", str) || 0 == wcscmp (L"0", str))
            SetBoolean (false);
        else
            return false;
        break;
    case PRIMITIVETYPE_DateTime:
    case PRIMITIVETYPE_Long:
        {
        int64_t i;
        if (1 != BE_STRING_UTILITIES_SWSCANF (str, L"%lld", &i))
            return false;
        else if (PRIMITIVETYPE_Long == primitiveType)
            SetLong (i);
        else
            SetDateTimeTicks (i);
        }
        break;
    case PRIMITIVETYPE_Double:
        {
        double d;
        if (1 == BE_STRING_UTILITIES_SWSCANF (str, L"%lg", &d))
            SetDouble (d);
        else
            return false;
        }
        break;
    case PRIMITIVETYPE_Integer:
        {
        int64_t i;
        if (1 == BE_STRING_UTILITIES_SWSCANF (str, L"%lld", &i))
            {
            if (INT_MAX >= i && INT_MIN <= i)
                SetInteger ((int32_t)i);
            else
                return false;
            }
        else
            return false;
        }
        break;
    case PRIMITIVETYPE_Point2D:
        {
        DPoint2d pt;
        if (2 == BE_STRING_UTILITIES_SWSCANF (str, L"%lg,%lg", &pt.x, &pt.y))
            SetPoint2D (pt);
        else
            return false;
        }
        break;
    case PRIMITIVETYPE_Point3D:
        {
        DPoint3d pt;
        if (3 == BE_STRING_UTILITIES_SWSCANF (str, L"%lg,%lg,%lg", &pt.x, &pt.y, &pt.z))
            SetPoint3D (pt);
        else
            return false;
        }
        break;
    default:
        BeAssert (false);
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
WString    ECValue::ToString () const
    {
    if (IsNull())
        return L"<null>";
        
    WString str;
    
    if (IsArray())
        {
        ArrayInfo arrayInfo = GetArrayInfo();
        str.Sprintf (L"Count: %d IsFixedSize: %d", arrayInfo.GetCount(), arrayInfo.IsFixedCount());
        }
    else if (IsStruct())
        {
        return L"IECInstance containing struct value";
        }
    else if (PRIMITIVETYPE_DateTime == m_primitiveType)
        str = GetDateTime().ToString(); // want something more readable than the ticks
    else if (!ConvertPrimitiveToString (str))
        str = L"<error>";
        
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::CanConvertToPrimitiveType (PrimitiveType type) const
    {
    ECValue v (*this);
    return v.ConvertToPrimitiveType (type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::ConvertToPrimitiveType (PrimitiveType newType)
    {
    if (IsNull())
        {
        SetPrimitiveType (newType);
        return true;
        }
    else if (!IsPrimitive())
        return false;
    else if (newType == GetPrimitiveType())
        return true;
    else if (PRIMITIVETYPE_String == newType)
        {
        WString strVal = ToString();
        SetString (strVal.c_str());
        return true;
        }
    else if (IsString())
        return ConvertToPrimitiveFromString (newType);

    PrimitiveType curType = GetPrimitiveType();
    switch (newType)
        {
    case PRIMITIVETYPE_Integer:
        {
        int32_t i;
        if (PRIMITIVETYPE_Double == curType)
            {
            double roundingTerm = DoubleOps::AlmostEqual (GetDouble(), 0.0) ? 0.0 : GetDouble() > 0.0 ? 0.5 : -0.5;
            i = (int32_t)(GetDouble() + roundingTerm);
            }
        else if (PRIMITIVETYPE_Boolean == curType)
            i = GetBoolean() ? 1 : 0;
        else if (PRIMITIVETYPE_Long == curType)
            {
            if (INT_MAX >= GetLong() && INT_MIN <= GetLong())
                i = (int32_t)GetLong();
            else
                return false;
            }
        else
            return false;

        SetInteger (i);
        }
        return true;
    case PRIMITIVETYPE_Long:
        {
        int64_t i;
        if (PRIMITIVETYPE_Double == curType)
            {
            double roundingTerm = DoubleOps::AlmostEqual (GetDouble(), 0.0) ? 0.0 : GetDouble() > 0.0 ? 0.5 : -0.5;
            i = (int64_t)(GetDouble() + roundingTerm);
            }
        else if (PRIMITIVETYPE_Boolean == curType)
            i = GetBoolean() ? 1 : 0;
        else if (PRIMITIVETYPE_Integer == curType)
            i = (int64_t)GetInteger();
        else
            return false;

        SetLong (i);
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

        SetDouble (d);
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
            b = !DoubleOps::AlmostEqual (GetDouble(), 0.0);
        else
            return false;

        SetBoolean (b);
        }
        return true;
    case PRIMITIVETYPE_Point3D:
        if (PRIMITIVETYPE_Point2D == curType)
            {
            SetPoint3D (DPoint3d::FromXYZ (GetPoint2D().x, GetPoint2D().y, 0.0));
            return true;
            }
        return false;
    case PRIMITIVETYPE_Point2D:
        if (PRIMITIVETYPE_Point3D == curType)
            {
            SetPoint2D (DPoint2d::From (GetPoint3D().x, GetPoint3D().y));
            return true;
            }
        return false;
    default:
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  Performs a binary comparison against another ECValue.
* @bsimethod                                                    Dylan.Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
bool              ECValue::Equals (ECValueCR v) const
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
    if (GetPrimitiveType() != v.GetPrimitiveType())
        return false;
    if (IsString())
        return m_stringInfo.Equals (v.m_stringInfo, m_ownershipFlags);
    if (IsBinary() || IsIGeometry ())
        {
        if (m_binaryInfo.m_size != v.m_binaryInfo.m_size)
            return false;
        if (m_binaryInfo.m_data == v.m_binaryInfo.m_data)
            return true;
        return 0 == memcmp (m_binaryInfo.m_data, v.m_binaryInfo.m_data, m_binaryInfo.m_size);
        }
    if (IsDouble())
        return DoubleOps::AlmostEqual (GetDouble(), v.GetDouble());
    if (IsPoint3D ())
        return DPoint3dOps::AlmostEqual (GetPoint3D (), v.GetPoint3D ());

    if (IsPoint2D())
         return DPoint2dOps::AlmostEqual (GetPoint2D (), v.GetPoint2D ());

    size_t primitiveValueSize = (size_t) GetFixedPrimitiveValueSize (GetPrimitiveType());
    //&m_boolean points to the first memory address of the union (as does every other union member)
    return 0 == memcmp (&m_boolean, &v.m_boolean, primitiveValueSize);
    }

/*---------------------------------------------------------------------------------**//**
* @param[in]        capacity Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus   ECValue::SetStructArrayInfo (uint32_t count, bool isFixedCount)
    {
    Clear();
        
    m_valueKind                = VALUEKIND_Array;

    m_arrayInfo.InitializeStructArray (count, isFixedCount);
    
    SetIsNull (false); // arrays are never null
    
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @param[in]        capacity Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECValue::SetPrimitiveArrayInfo (PrimitiveType primitiveElementType, uint32_t count, bool isFixedSize)
    {
    Clear();
        
    m_valueKind                = VALUEKIND_Array;

    m_arrayInfo.InitializePrimitiveArray (primitiveElementType, count, isFixedSize);
    
    SetIsNull (false); // arrays are never null
    
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayInfo       ECValue::GetArrayInfo() const
    {
    BeAssert (IsArray());
    
    return m_arrayInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
uint32_t        ECValue::GetFixedPrimitiveValueSize (PrimitiveType primitivetype)
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
        case PRIMITIVETYPE_Point2D:
            return 2 * sizeof(double);
        case PRIMITIVETYPE_Point3D:
            return 3 * sizeof(double);
        case PRIMITIVETYPE_DateTime:
            return sizeof(int64_t); //ticks
        default:
            DEBUG_FAIL("Most datatypes have not yet been implemented... or perhaps you have passed in a variable-sized type.");
            return 0;
        }
    }
      
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArrayInfo::InitializeStructArray (uint32_t count, bool isFixedCount)
    {
    m_arrayKind = ARRAYKIND_Struct;
    m_count           = count;
    m_isFixedCount     = isFixedCount;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArrayInfo::InitializePrimitiveArray (PrimitiveType elementPrimitiveType, uint32_t count, bool isFixedCount)
    {
    m_elementPrimitiveType = elementPrimitiveType;
    m_count           = count;
    m_isFixedCount    = isFixedCount;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        ArrayInfo::GetCount() const
    {
    return m_count;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ArrayInfo::IsFixedCount() const
    {
    return m_isFixedCount;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ValueKind       ArrayInfo::GetKind() const
    {        
    return (ValueKind) (m_arrayKind & 0xFF); 
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType   ArrayInfo::GetElementPrimitiveType() const
    {        
    PRECONDITION (IsPrimitiveArray() && "Tried to get the element primitive type of an ArrayInfo that is not classified as a primitive array.", (PrimitiveType)0);    
    return m_elementPrimitiveType;
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ArrayInfo::IsPrimitiveArray() const
    {        
    return GetKind() == VALUEKIND_Primitive; 
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ArrayInfo::IsStructArray() const
    {        
    return GetKind() == VALUEKIND_Struct; 
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor (IECInstanceCR instance, int newPropertyIndex, int newArrayIndex) : m_isAdhoc (false)
    {
    PushLocation (instance, newPropertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor (ECEnablerCR enabler, int newPropertyIndex, int newArrayIndex) : m_isAdhoc (false)
    {
    PushLocation (enabler, newPropertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor (ECValueAccessorCR accessor)
    : m_locationVector (accessor.GetLocationVector()), m_isAdhoc (accessor.IsAdhocProperty())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValueAccessor::Clone (ECValueAccessorCR accessor)
    {
    m_locationVector.clear();

    for (ECValueAccessor::Location const & location: accessor.GetLocationVectorCR())
        PushLocation (*location.GetEnabler(), location.GetPropertyIndex(), location.GetArrayIndex());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::LocationVector const &   ECValueAccessor::GetLocationVectorCR() const
    {
    return m_locationVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
const ECValueAccessor::LocationVector&          ECValueAccessor::GetLocationVector() const
    {
    return m_locationVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnablerCR                                     ECValueAccessor::GetEnabler (uint32_t depth) const
    {
    return * m_locationVector[depth].GetEnabler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::Location&                      ECValueAccessor::operator[] (uint32_t depth)
    {
    return m_locationVector[depth];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
const ECValueAccessor::Location&                ECValueAccessor::operator[] (uint32_t depth) const
    {
    return m_locationVector[depth];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::PushLocation (ECEnablerCR newEnabler, int newPropertyIndex, int newArrayIndex)
    {
    m_locationVector.push_back (Location (& newEnabler, newPropertyIndex, newArrayIndex));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::PushLocation (IECInstanceCR instance, int newPropertyIndex, int newArrayIndex)
    {
    PushLocation (instance.GetEnabler(), newPropertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                            ECValueAccessor::PushLocation (ECEnablerCR newEnabler, WCharCP accessString, int newArrayIndex)
    {
    uint32_t propertyIndex;
    ECObjectsStatus status = newEnabler.GetPropertyIndex(propertyIndex, accessString);
    if (ECOBJECTS_STATUS_Success != status)
        {
        //BeAssert (false && "Could not resolve property index for this access string");
        return false;
        }
    PushLocation (newEnabler, (int)propertyIndex, newArrayIndex);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                            ECValueAccessor::PushLocation (IECInstanceCR instance, WCharCP accessString, int newArrayIndex)
    {
    return PushLocation (instance.GetEnabler(), accessString, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::PopLocation()
    {
    m_locationVector.pop_back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::Clear ()
    {
    m_locationVector.clear();
    m_isAdhoc = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::Location&                      ECValueAccessor::DeepestLocation()
    {
    return m_locationVector.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::Location const&  ECValueAccessor::DeepestLocationCR () const
    {
    return m_locationVector.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t                                        ECValueAccessor::GetDepth() const
    {
    return (uint32_t)m_locationVector.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP                                 ECValueAccessor::GetAccessString () const
    {
    uint32_t depth = GetDepth();
    if (0 == depth)
        return NULL;

    return GetAccessString (depth-1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP                                 ECValueAccessor::GetAccessString (uint32_t depth) const
    {
    int propertyIndex         = m_locationVector[depth].GetPropertyIndex();
    ECEnablerCR enabler       = * m_locationVector[depth].GetEnabler();
    WCharCP accessString;
    if (ECOBJECTS_STATUS_Success == enabler.GetAccessString (accessString, propertyIndex))
        return accessString;
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString                                        ECValueAccessor::GetPropertyName() const
    {
    uint32_t depth = GetDepth();
    if (0 == depth)
        return WString();

    WString name = GetAccessString (GetDepth()-1);

    // get the name after the last .
    size_t lastDotIndex = name.rfind ('.');
    if (WString::npos != lastDotIndex)
        {
        lastDotIndex++;
        size_t len =  name.length()-lastDotIndex;
        name = name.substr (lastDotIndex, len);
        }

    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECValueAccessor::Location::GetECProperty() const
    {
    if (NULL == m_cachedProperty && NULL != m_enabler)
        m_cachedProperty = m_enabler->LookupECProperty (m_propertyIndex);

    return m_cachedProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECValueAccessor::GetECProperty() const
    {
    return DeepestLocationCR().GetECProperty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString                                        ECValueAccessor::GetDebugAccessString() const
    {
    WString temp;
    for(uint32_t depth = 0; depth < GetDepth(); depth++)
        {
        if(depth > 0)
            temp.append (L" -> ");
        temp.append (WPrintfString(L"{%d", m_locationVector[depth].GetPropertyIndex()));
        if(m_locationVector[depth].GetArrayIndex() > -1)
            temp.append (WPrintfString(L",%d", m_locationVector[depth].GetArrayIndex()));
        temp.append (L"}");
        temp.append (GetAccessString (depth));
        }
    return temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString                                        ECValueAccessor::GetManagedAccessString() const
    {
    WString temp;
    for(uint32_t depth = 0; depth < GetDepth(); depth++)
        {
        if(depth > 0)
            temp.append (L".");

        WCharCP str = GetAccessString (depth);
        WCharCP lastDot = wcsrchr (str, L'.');

        if (NULL != lastDot)
            str = lastDot+1;

        temp.append (str);
        //If the current index is an array element,
        if(m_locationVector[depth].GetArrayIndex() > -1)
            {
            temp.append (L"[");
            temp.append (WPrintfString(L"%d", m_locationVector[depth].GetArrayIndex()));
            temp.append (L"]");
            }
        }
    return temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                            ECValueAccessor::MatchesEnabler(uint32_t depth, ECEnablerCR other) const
    {
    ECEnablerCR enabler = GetEnabler (depth);
    return & enabler == & other;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValueAccessor::operator!=(ECValueAccessorCR accessor) const
    {
    if(GetDepth() != accessor.GetDepth())
        return true;
    for(uint32_t depth = 0; depth < GetDepth(); depth ++)
        {
        if((*this)[depth].GetEnabler() != accessor[depth].GetEnabler()
            || (*this)[depth].GetPropertyIndex() != accessor[depth].GetPropertyIndex()
            || (*this)[depth].GetArrayIndex()    != accessor[depth].GetArrayIndex())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValueAccessor::operator==(ECValueAccessorCR accessor) const
    {
    return !(*this != accessor);
    }

#define NUM_INDEX_BUFFER_CHARS 63
#define NUM_ACCESSSTRING_BUFFER_CHARS 1023

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
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
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassCP getStructArrayClass (ECClassCR enablerClass, WCharCP propertyName)
    {
    WCharCP dotPos = wcschr (propertyName, '.');
    if (NULL != dotPos)
        {
        WString structName (propertyName, dotPos);
        ECClassCP structClass = getStructArrayClass (enablerClass, structName.c_str());
        if (NULL == structClass)
            { BeAssert (false); return NULL; }

        return getStructArrayClass (*structClass, dotPos+1);
        }

    ECPropertyP propertyP = enablerClass.GetPropertyP (propertyName);
    if (!propertyP)
        return NULL;

    if (auto structProperty = propertyP->GetAsStructProperty ())
        return &structProperty->GetType ();
    else if (auto arrayProp = propertyP->GetAsArrayProperty ())
        return arrayProp->GetStructElementType ();

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus getECValueAccessorUsingManagedAccessString (wchar_t* asBuffer, wchar_t* indexBuffer, ECValueAccessor& va, ECEnablerCR  enabler, WCharCP managedPropertyAccessor)
    {
    ECObjectsStatus status;
    uint32_t        propertyIndex;

    // see if access string specifies an array
    WCharCP pos1 = wcschr (managedPropertyAccessor, L'[');

    // if not an array then we have a primitive that we can access directly 
    if (NULL == pos1)
        {
        status = enabler.GetPropertyIndex (propertyIndex, managedPropertyAccessor);
        if (ECOBJECTS_STATUS_Success == status)
            {
            va.PushLocation (enabler, propertyIndex, -1);
            return ECOBJECTS_STATUS_Success;
            }

        status = enabler.GetPropertyIndex (propertyIndex, managedPropertyAccessor);

        if (ECOBJECTS_STATUS_Success != status)
            return status;

        va.PushLocation (enabler, propertyIndex, -1);
        return ECOBJECTS_STATUS_Success;
        }

    size_t numChars = 0;
    numChars = pos1 - managedPropertyAccessor;
    wcsncpy(asBuffer, managedPropertyAccessor, numChars>NUM_ACCESSSTRING_BUFFER_CHARS?NUM_ACCESSSTRING_BUFFER_CHARS:numChars);
    asBuffer[numChars]=0;

    WCharCP pos2 = wcschr (pos1+1, L']');
    if (!pos2)
        return ECOBJECTS_STATUS_Error;

    numChars = pos2 - pos1 - 1;
    wcsncpy(indexBuffer, pos1+1, numChars>NUM_INDEX_BUFFER_CHARS?NUM_INDEX_BUFFER_CHARS:numChars);
    indexBuffer[numChars]=0;

    uint32_t indexValue = -1;
    BE_STRING_UTILITIES_SWSCANF (indexBuffer, L"%ud", &indexValue);

    ECValue  arrayVal;

    status = enabler.GetPropertyIndex (propertyIndex, asBuffer);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    // if no character after closing bracket then we just want the array, else we are dealing with a member of a struct array
    if (0 == *(pos2+1))
        {
        va.PushLocation (enabler, propertyIndex, indexValue);
        return ECOBJECTS_STATUS_Success;
        }

    WString str = asBuffer; 

    ECClassCP structClass = getStructArrayClass (enabler.GetClass(), asBuffer);
    if (!structClass)
        return ECOBJECTS_STATUS_Error;

    ECN::ECEnablerP enablerP = const_cast<ECN::ECEnablerP>(&enabler);
    StandaloneECEnablerPtr structEnabler = dynamic_cast<StandaloneECEnablerP>(enablerP->GetEnablerForStructArrayMember (structClass->GetSchema().GetSchemaKey(), structClass->GetName().c_str()).get());
    if (structEnabler.IsNull())
        return ECOBJECTS_STATUS_Error;

    va.PushLocation (enabler, propertyIndex, indexValue);

    return getECValueAccessorUsingManagedAccessString (asBuffer, indexBuffer, va, *structEnabler, pos2+2); // move to character after "]." in access string.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::PopulateValueAccessor (ECValueAccessor& va, IECInstanceCR instance, WCharCP managedPropertyAccessor)
    {
    return PopulateValueAccessor (va, instance.GetEnabler(), managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::PopulateValueAccessor (ECValueAccessor& va, ECEnablerCR enabler, WCharCP accessor)
    {
    wchar_t         asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS+1];
    wchar_t         indexBuffer[NUM_INDEX_BUFFER_CHARS+1];
    va.Clear ();
    return getECValueAccessorUsingManagedAccessString (asBuffer, indexBuffer, va, enabler, accessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::PopulateValueAccessor (ECValueAccessor& va, IECInstanceCR instance, WCharCP accessor, bool includeAdhocs)
    {
    auto status = PopulateValueAccessor (va, instance, accessor);
    if (ECOBJECTS_STATUS_PropertyNotFound == status && includeAdhocs)
        {
        // Find the array index of the ad-hoc property value with the specified name
        va.Clear();
        for (auto const& containerIndex : AdhocContainerPropertyIndexCollection (instance.GetEnabler()))
            {
            AdhocPropertyQuery adhoc (instance, containerIndex);
            uint32_t arrayIndex;
            if (adhoc.GetPropertyIndex (arrayIndex, accessor))
                {
                va.PushLocation (instance.GetEnabler(), adhoc.GetContainerPropertyIndex(), arrayIndex);
                va.m_isAdhoc = true;
                return ECOBJECTS_STATUS_Success;
                }
            }
        }

    return status;
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  ECPropertyValue
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue () : m_instance(NULL), m_evaluated(false) {}
ECPropertyValue::ECPropertyValue (IECInstanceCR instance) : m_instance (&instance), m_evaluated(false) {}
ECValueCR           ECPropertyValue::GetValue ()            const    { EvaluateValue(); return m_ecValue; }
IECInstanceCR       ECPropertyValue::GetInstance ()         const    { return *m_instance; }
ECValueAccessorCR   ECPropertyValue::GetValueAccessor ()    const    { return m_accessor; }
ECValueAccessorR    ECPropertyValue::GetValueAccessorR ()            { return m_accessor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValuePtr     ECPropertyValue::GetPropertyValue (IECInstanceCR instance, WCharCP propertyAccessor)
    {
    ECValueAccessor va;

    if (ECOBJECTS_STATUS_Success != ECValueAccessor::PopulateValueAccessor (va, instance, propertyAccessor))
        return NULL;

    return  new ECPropertyValue (instance, va);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
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
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECPropertyValue::EvaluateValue () const
    {
    if (!m_evaluated)
        {
        // m_ecValue.Clear();
        m_evaluated = true;
        return NULL != m_instance ? m_instance->GetValueUsingAccessor (m_ecValue, m_accessor) : ECOBJECTS_STATUS_Error;
        }
    else
        return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue (ECPropertyValueCR from)
    {
    m_instance = from.m_instance;
    m_accessor = from.m_accessor;
    m_ecValue.From (from.m_ecValue);
    m_evaluated = from.m_evaluated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue (IECInstanceCR instance, ECValueAccessorCR accessor)
    :
    m_instance (&instance), m_accessor (accessor), m_evaluated (false)
    {
    // EvaluateValue(); performance: do this lazily
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue (IECInstanceCR instance, ECValueAccessorCR accessor, ECValueCR v)
    :
    m_instance (&instance), m_accessor (accessor), m_ecValue (v), m_evaluated (true)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECPropertyValue::Initialize (IECInstanceCR instance, WCharCP accessString, ECValueCR v)
    {
    ECObjectsStatus status = ECValueAccessor::PopulateValueAccessor (m_accessor, instance, accessString);
    if (ECOBJECTS_STATUS_Success == status)
        {
        m_instance = &instance;
        m_ecValue = v;
        m_evaluated = true;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool                ECPropertyValue::HasChildValues () const
    {
    // Avoid evaluating value if we can answer this by looking at the ECProperty
    // Note: performance: the accessor caches the ECProperty, since we often request it more than once
    ECPropertyCP prop = m_accessor.GetECProperty();
    ArrayECPropertyCP arrayProp;
    if (NULL == prop || prop->GetIsPrimitive())
        return false;
    else if (NULL != (arrayProp = prop->GetAsArrayProperty()) && ARRAYKIND_Primitive == arrayProp->GetKind() && -1 != m_accessor.DeepestLocationCR().GetArrayIndex())
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
        BeAssert (m_ecValue.IsArray());
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionPtr  ECPropertyValue::GetChildValues () const
    {
    return HasChildValues() ? new ECValuesCollection (*this) : new ECValuesCollection();
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  ECValuesCollectionIterator
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue ECValuesCollectionIterator::GetFirstPropertyValue (IECInstanceCR instance)
    {
    ECEnablerCR enabler = instance.GetEnabler();
    if (0 == enabler.GetClass().GetPropertyCount())
        return ECPropertyValue ();

    uint32_t firstIndex = enabler.GetFirstPropertyIndex (0);
    ECValueAccessor firstPropertyAccessor;
    if (0 != firstIndex)
        firstPropertyAccessor.PushLocation (enabler, firstIndex, -1);    

    return ECPropertyValue (instance, firstPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionIterator::ECValuesCollectionIterator (IECInstanceCR instance)
    :
    m_propertyValue (GetFirstPropertyValue (instance)),
    m_arrayCount (-1)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue ECValuesCollectionIterator::GetChildPropertyValue (ECPropertyValueCR parentPropertyValue)
    {
    m_arrayCount = -1;
    ECValueCR       parentValue = parentPropertyValue.GetValue();
    ECValueAccessor childAccessor (parentPropertyValue.GetValueAccessor());

    if (parentValue.IsArray())
        {
        ArrayInfo   arrayInfo  = parentValue.GetArrayInfo();
        uint32_t    arrayCount = arrayInfo.GetCount();

        if (0 < arrayCount)
            {
            m_arrayCount = arrayCount;
            childAccessor.DeepestLocation().SetArrayIndex (0);
            }
        else
            childAccessor.PopLocation();
        }
    else /* if (parentValue.IsStruct()) ###TODO: concept of an ECValue containing an embedded struct is undefined. */
        {
        uint32_t        pathLength  = childAccessor.GetDepth();

        if ( ! EXPECTED_CONDITION (0 < pathLength))
            return ECPropertyValue();

        if (ECValueAccessor::INDEX_ROOT != childAccessor[pathLength - 1].GetArrayIndex())
            {
            IECInstancePtr  structInstance  = parentValue.GetStruct();
            if (structInstance.IsValid())
                {
                ECEnablerCR     enabler         = structInstance->GetEnabler();
                uint32_t        firstIndex      = enabler.GetFirstPropertyIndex (0);

                childAccessor.PushLocation (enabler, firstIndex, -1);
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
            uint32_t        parentIndex =  childAccessor[pathLength - 1].GetPropertyIndex();
            ECEnablerCR     enabler     = *childAccessor[pathLength - 1].GetEnabler();
            uint32_t        firstIndex  =  enabler.GetFirstPropertyIndex (parentIndex);

            if (0 != firstIndex)
                childAccessor.PushLocation (enabler, firstIndex, -1);
            else
                childAccessor.PopLocation();
            }
        }

    return ECPropertyValue (parentPropertyValue.GetInstance(), childAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionIterator::ECValuesCollectionIterator (ECPropertyValueCR parentPropertyValue)
    :
    m_propertyValue (GetChildPropertyValue (parentPropertyValue))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
//bool            ECValuesCollectionIterator::IsAtEnd() const
//    {
//    return 0 == m_propertyValue.GetValueAccessor().GetDepth();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
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
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue const& ECValuesCollectionIterator::GetCurrent () const
    {
    return m_propertyValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValuesCollectionIterator::MoveToNext()
    {
    ECValueAccessorR    currentAccessor = m_propertyValue.GetValueAccessorR();    

    if (ECValueAccessor::INDEX_ROOT != currentAccessor.DeepestLocation().GetArrayIndex())
        {
        /*--------------------------------------------------------------------------
          If we are on an array member get the next member
        --------------------------------------------------------------------------*/
        if (!EXPECTED_CONDITION (0 <= m_arrayCount))
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
        uint32_t    pathLength   = currentAccessor.GetDepth();
        uint32_t    currentIndex = currentAccessor[pathLength - 1].GetPropertyIndex();
        uint32_t    parentIndex = 0;

        // If we are inside an embedded struct get the struct index from the accessor's path
        if (pathLength > 1 && ECValueAccessor::INDEX_ROOT == currentAccessor[pathLength - 2].GetArrayIndex())
            parentIndex = currentAccessor[pathLength - 2].GetPropertyIndex();

        ECEnablerCP enabler   = currentAccessor[pathLength - 1].GetEnabler();
        uint32_t    nextIndex = enabler->GetNextPropertyIndex (parentIndex, currentIndex);

        currentAccessor.DeepestLocation().SetPropertyIndex (nextIndex);

        // If that was the last index in the current struct, we are done
        if (0 == nextIndex)
            {
            currentAccessor.Clear();
            return;
            }
        }

    m_propertyValue.ResetValue ();
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  ECValuesCollection
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::ECValuesCollection ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::ECValuesCollection (ECPropertyValueCR parentPropValue)
    : m_parentPropertyValue (parentPropValue)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::ECValuesCollection (IECInstanceCR instance)
    : m_parentPropertyValue (instance)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionPtr ECValuesCollection::Create (IECInstanceCR instance)
    {
    return new ECValuesCollection (instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionPtr ECValuesCollection::Create (ECPropertyValueCR  parentProperty)
    {
    if (parentProperty.HasChildValues ())
        return new ECValuesCollection (parentProperty);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValueCR  ECValuesCollection::GetParentProperty () const
    {
    return m_parentPropertyValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::const_iterator ECValuesCollection::begin () const
    {
    RefCountedPtr<ECValuesCollectionIterator> iter;
    
    if (0 == m_parentPropertyValue.GetValueAccessor().GetDepth())
        iter = new ECValuesCollectionIterator (m_parentPropertyValue.GetInstance());
    else
        {
        ECValueCR       parentValue = m_parentPropertyValue.GetValue();
        if (parentValue.IsArray())
            {
            ArrayInfo   arrayInfo  = parentValue.GetArrayInfo();
            uint32_t    arrayCount = arrayInfo.GetCount();

            if (0 == arrayCount)
                return ECValuesCollection::end ();
            }

        iter = new ECValuesCollectionIterator (m_parentPropertyValue);
        }

    return const_iterator (*iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionIterator::ECValuesCollectionIterator() : m_arrayCount(-1)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::const_iterator ECValuesCollection::end () const
    {
    // WIP_FUSION: can we reduce the amount of dynamic allocation associated with iterating an ECValuesCollection?
    if (m_end.IsNull())
        m_end = new ECValuesCollectionIterator();

    return const_iterator (*m_end);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
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
* @bsimethod                                                    Paul.Connelly   01/13
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

    NumericFormat () : insertThousandsSeparator(false), precisionType(PrecisionType::Decimal), widthBeforeDecimal(0), minDecimalPrecision(0), maxDecimalPrecision(0), multiplier(1.0), insertPos(-1) { }

    DoubleFormatterPtr ToFormatter() const
        {
        DoubleFormatterPtr fmtr = DoubleFormatter::Create();
        fmtr->SetLeadingZero (widthBeforeDecimal > 0);
        fmtr->SetTrailingZeros (maxDecimalPrecision > 0);
        fmtr->SetPrecision (precisionType, maxDecimalPrecision);
        fmtr->SetInsertThousandsSeparator (insertThousandsSeparator);
        return fmtr;
        }

    static bool     ExtractStandardFormatPrecision (WCharCP fmt, uint32_t& precision);
    static bool     ApplyStandardNumericFormat (WStringR formatted, WCharCP fmt, double d);
    static void     ParseCustomFormatString(WStringR formatted, WCharCP fmt, NumericFormat& numFormat, bool ignoreExponent);
    static bool     ApplyCustomNumericFormat(WStringR formatted, WCharCP fmt, double d, bool* onlyZeros);
    static WCharCP  ParseNumberFormat (NumericFormat& numFormat, WCharCP start);
    static WCharCP  SkipLiteralString (WCharCP start);
public:
    static bool                 FormatDouble (WStringR formatted, WCharCP formatString, double d);
    static bool                 FormatInteger (WStringR formatted, WCharCP formatString, int64_t i);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::ExtractStandardFormatPrecision (WCharCP fmt, uint32_t& precision)
    {
    // Expect 0-2 digits specifying a precision or width from 0-99
    // Don't overwrite value of precision unless one is explicitly specified or cannot be extracted
    bool isValidFormat = true;
    BeAssert (NULL != fmt);
    if (*fmt)
        {
        precision = 0;
        if (iswdigit (*fmt))
            {
            precision = (uint32_t)(*fmt - '0');
            if (*++fmt)
                {
                if (iswdigit (*fmt))
                    {
                    precision *= 10;
                    precision += (uint32_t)(*fmt - '0');
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
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::FormatInteger (WStringR formatted, WCharCP fmt, int64_t i)
    {
    // Check for a couple of standard formats specific to integers
    if (NULL != fmt)
        {
        WChar spec = *fmt;
        WChar lspec = towlower (spec);
        if ('d' == lspec || 'x' == lspec)
            {
            uint32_t precision = 0;
            if (ExtractStandardFormatPrecision (fmt + 1, precision))
                {
                WChar buf[100]; // because max width is 99
                switch (lspec)
                    {
                    case 'x':       // hexadecimal
                        {
                        HexFormatOptions opts = HexFormatOptions::LeadingZeros;
                        if ('X' == spec)
                            opts = (HexFormatOptions)(static_cast<int>(opts) | static_cast<int>(HexFormatOptions::Uppercase));

                        BeStringUtilities::FormatUInt64 (buf, _countof(buf), (uint64_t)i, opts, static_cast <uint8_t> (precision));
                        }
                        break;
                    case 'd':       // decimal
                        {
                        BeStringUtilities::Snwprintf (buf, _countof(buf), L"%0*lld", precision, i);
                        }
                        break;
                    }

                formatted = buf;
                return true;
                }
            }
        }

    // Treat as double
    return FormatDouble (formatted, fmt, (double)i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::ApplyStandardNumericFormat (WStringR formatted, WCharCP fmt, double d)
    {
    // Limited support for following standard .NET specifiers. All can be upper- or lower-case, all take an optional width/precision from 0-99
    //  E: scientific notation. Default precision 6.
    //  C: Currency. Treated as F
    //  F: Fixed-point. Default precision 2.
    //  N: Includes group separators
    //  R: Round-trippable. Treated as F with maximum precision.
    //  P: Multiply input by 100, append '%'
    //  G: Treated as fixed-point.

    BeAssert (NULL != fmt && 0 != *fmt);

    WChar spec = *fmt,
          lspec = towlower (spec);

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
    if (!ExtractStandardFormatPrecision (fmt + 1, precision))
        return false;

    DoubleFormatterPtr fmtr = DoubleFormatter::Create();

    if (ignoreExtractedPrecision)
        precision = MAX_PRECISION;
    else
        fmtr->SetTrailingZeros (false);

    fmtr->SetLeadingZero (true);
    fmtr->SetInsertThousandsSeparator (groupSeparators);
    fmtr->SetPrecision (precisionType, (Byte)precision);

    formatted = fmtr->ToString (d);

    if (appendPercent)
        formatted.append (1, L'%');
    else if (ignoreExtractedPrecision && formatted.length() > 1)
        {
        // DoubleFormatter is going to give us trailing zeros. Strip them off
        size_t endPos = formatted.length() - 1,
               startPos = formatted.find ('.');

        while (formatted[endPos] == '0')
            --endPos;

        if (endPos == startPos)
            --endPos;   // only zeros follow the decimal point, so remove it.

        formatted.erase (endPos + 1);
        }
        
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void NumericFormat::ParseCustomFormatString(WStringR formatted, WCharCP fmt, NumericFormat& numFormat, bool ignoreExponent)
    {
    while (0 != *fmt)
        {
        switch (*fmt)
            {
            case '\'':
            case '"':       // literal string
                {
                WCharCP endQuote = SkipLiteralString(fmt);
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

                    while (L'0' == *fmt)
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
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::ApplyCustomNumericFormat(WStringR formatted, WCharCP fmt, double d, bool* onlyZeros)
    {
    WString originalFmt = fmt;
    formatted.clear();
    
    NumericFormat numFormat;
    ParseCustomFormatString(formatted, fmt, numFormat, false);

    if (NULL != onlyZeros)
        *onlyZeros = false;
    
    // It's possible the format string did not actually contain any placeholders for the digits, in which case we have no formatting to do
    if (WString::npos == numFormat.insertPos)
        {
        // Need to go back and put processed characters such as exponents back in.
        formatted.clear();
        ParseCustomFormatString(formatted, fmt, numFormat, true);
        
        return true;
        }

    DoubleFormatterPtr fmtr = numFormat.ToFormatter();
    WString formattedDouble = fmtr->ToString(d * numFormat.multiplier);

    // Caller needs to know if rounding or precision of the format caused this to be only 0's.
    // Formatter can also insert things such as +,-,.,e,E, etc., so I think the best check is for non-zero digits.
    if (NULL != onlyZeros)
        *onlyZeros = !std::any_of(formattedDouble.begin(), formattedDouble.end(), [&](WChar const& c) { return c >= L'1' && c <= L'9'; });

    // We have to pad width with leading zeros, DoubleFormatter doesn't support it.
    if (numFormat.widthBeforeDecimal > 0)
        {
        size_t endPos = formattedDouble.find('.');
        if (WString::npos == endPos)
            endPos = formattedDouble.length();

        if ((uint32_t)endPos < numFormat.widthBeforeDecimal)
            formattedDouble.insert((size_t)0, numFormat.widthBeforeDecimal - (uint32_t)endPos, '0');
        }
    else if (formattedDouble.length() > 0 && formattedDouble[0] == '0')
        {
        // DoubleFormatter ignores our leading zero setting if the value of the double is zero
        formattedDouble.erase(0, 1);
        }

    // And we have to remove trailing zeros
    if (numFormat.minDecimalPrecision < numFormat.maxDecimalPrecision)
        {
        size_t decimalPos = (uint32_t)formattedDouble.find('.');
        if (WString::npos != decimalPos)
            {
            uint32_t minPos = (uint32_t)decimalPos + 1 + numFormat.minDecimalPrecision, // the minimum number of decimal digits to keep, regardless of whether or not they are zero
                maxPos = (uint32_t)decimalPos + 1 + numFormat.maxDecimalPrecision;

            if (maxPos >= (uint32_t)formattedDouble.length())
                maxPos = (uint32_t)formattedDouble.length() - 1;

            while (maxPos >= minPos && '0' == formattedDouble[maxPos])
                formattedDouble.erase(maxPos--);

            // trailing decimal point?
            if ((uint32_t)decimalPos == maxPos)
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
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumericFormat::FormatDouble (WStringR formatted, WCharCP fmt, double d)
    {
    if (ApplyStandardNumericFormat (formatted, fmt, d))
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
    bvector<WString> sections;
    BeStringUtilities::Split(fmt, L";", L"\\", sections);

    // Do something arbitrary for bad data.
    if (UNEXPECTED_CONDITION(0 == sections.size()))
        return ApplyStandardNumericFormat(formatted, L"g", d);
    
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
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP NumericFormat::SkipLiteralString (WCharCP start)
    {
    WCharCP end = start + 1;
    WChar quoteChar = *start;

    while (*end)
        {
        if (quoteChar == *end)
            break;

        ++end;
        }

    return end;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP NumericFormat::ParseNumberFormat (NumericFormat& numFormat, WCharCP start)
    {
    bool foundZero = false;
    bool stopProcessing = false;
    WCharCP cur = start;
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
                if (*(cur+1) == '.' || *(cur+1) == ',')
                    {
                    // Is a scaling factor if immediately precedes decimal point
                    numFormat.multiplier /= 1000.0;
                    break;
                    }
                else if (*(cur+1) != '0' && *(cur+1) != '#')
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
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECValue::ApplyDotNetFormatting (WStringR out, WCharCP fmt) const
    {
    if (IsNull())
        return false;

    switch (GetPrimitiveType())
        {
    case PRIMITIVETYPE_Integer:
        return NumericFormat::FormatInteger (out, fmt, GetInteger());
    case PRIMITIVETYPE_Long:
        return NumericFormat::FormatInteger (out, fmt, GetLong());
    case PRIMITIVETYPE_Double:
        return NumericFormat::FormatDouble (out, fmt, GetDouble());
    default:
        BeAssert (false && L"Call ECValue::SupportsDotNetFormatting() to determine if this ECValue can be formatted");
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocPropertyQuery::AdhocPropertyQuery (IECInstanceCR host, WCharCP accessString)
    : AdhocPropertyMetadata (host.GetEnabler(), accessString), m_host (host)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocPropertyQuery::AdhocPropertyQuery (IECInstanceCR host, uint32_t propertyIndex)
    : AdhocPropertyMetadata (host.GetEnabler(), propertyIndex), m_host (host)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocPropertyMetadata::IsSupported (ECEnablerCR enabler, WCharCP accessString)
    {
    AdhocPropertyMetadata meta (enabler, accessString, false);
    return meta.IsSupported();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocPropertyMetadata::IsSupported (ECEnablerCR enabler, uint32_t propIdx)
    {
    AdhocPropertyMetadata meta (enabler, propIdx, false);
    return meta.IsSupported();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocPropertyMetadata::AdhocPropertyMetadata (ECEnablerCR enabler, WCharCP containerAccessString, bool loadMetadata)
    : m_containerIndex (0)
    {
    uint32_t containerIndex = 0;
    if (ECOBJECTS_STATUS_Success == enabler.GetPropertyIndex (containerIndex, containerAccessString))
        Init (enabler, containerIndex, loadMetadata);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocPropertyMetadata::AdhocPropertyMetadata (ECEnablerCR enabler, uint32_t propIdx, bool loadMetadata)
    : m_containerIndex (0)
    {
    Init (enabler, propIdx, loadMetadata);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocPropertyMetadata::AdhocPropertyMetadata (ECEnablerCR enabler, WCharCP containerAccessString)
    : AdhocPropertyMetadata (enabler, containerAccessString, true)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocPropertyMetadata::AdhocPropertyMetadata (ECEnablerCR enabler, uint32_t propIdx)
    : AdhocPropertyMetadata (enabler, propIdx, true)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocPropertyMetadata::Init (ECEnablerCR enabler, uint32_t containerIndex, bool loadMetadata)
    {
    // find struct array property
    ECPropertyCP prop = enabler.LookupECProperty (containerIndex);
    ArrayECPropertyCP arrayProp = nullptr != prop ? prop->GetAsArrayProperty() : nullptr;
    ECClassCP structClass = nullptr;
    if (nullptr == arrayProp || ARRAYKIND_Struct != arrayProp->GetKind() || nullptr == (structClass = arrayProp->GetStructElementType()))
        return false;

    // find custom attribute on struct class
    IECInstancePtr attr = structClass->GetCustomAttribute (L"AdhocPropertyContainerDefinition");
    if (attr.IsNull())
        return false;

    // validate required metadata is defined, and load it if requested
    ECValue v;
    v.SetAllowsPointersIntoInstanceMemory (true);

    static const WCharCP s_propertyNames[(size_t)Index::MAX] =
        {
        L"NameProperty", L"DisplayLabelProperty", L"ValueProperty", L"TypeProperty", L"UnitProperty", L"ExtendTypeProperty", L"IsReadOnlyProperty", L"IsHiddenProperty"
        };

    for (size_t i = 0; i < _countof (s_propertyNames); i++)
        {
        if (ECOBJECTS_STATUS_Success == attr->GetValue (v, s_propertyNames[i]) && !v.IsNull() && v.IsString())
            {
            prop = structClass->GetPropertyP (v.GetString());
            if (nullptr != prop)
                {
                if (loadMetadata)
                    m_metadataPropertyNames[i] = v.GetString(); 
                }
            else
                return false;
            }
        else if (IsRequiredMetadata (static_cast<Index>(i)))
            return false;
        }

    // only considered valid when m_containerIndex set to non-zero
    m_containerIndex = containerIndex;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocPropertyMetadata::IsRequiredMetadata (Index index)
    {
    return Index::Name == index || Index::Value == index;
    }

/*---------------------------------------------------------------------------------**//**
* From managed...See ECAdhocProperties::GetKnownTypeForCode
* @bsistruct                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
enum class PrimitiveTypeCode
    {
    String  = 0,
    Integer,
    Long,
    Double,
    DateTime,
    Boolean,
    Binary,
    Point2D,
    Point3D
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocPropertyMetadata::PrimitiveTypeForCode (PrimitiveType& type, int32_t code)
    {
    switch (code)
        {
        case (int32_t)PrimitiveTypeCode::String:     type = PRIMITIVETYPE_String; return true;
        case (int32_t)PrimitiveTypeCode::Integer:    type = PRIMITIVETYPE_Integer; return true;
        case (int32_t)PrimitiveTypeCode::Long:       type = PRIMITIVETYPE_Long; return true;
        case (int32_t)PrimitiveTypeCode::Double:     type = PRIMITIVETYPE_Double; return true;
        case (int32_t)PrimitiveTypeCode::DateTime:   type = PRIMITIVETYPE_DateTime; return true;
        case (int32_t)PrimitiveTypeCode::Boolean:    type = PRIMITIVETYPE_Boolean; return true;
        case (int32_t)PrimitiveTypeCode::Point2D:    type = PRIMITIVETYPE_Point2D; return true;
        case (int32_t)PrimitiveTypeCode::Point3D:    type = PRIMITIVETYPE_Point3D; return true;
        default:                            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocPropertyMetadata::CodeForPrimitiveType (int32_t& code, PrimitiveType type)
    {
    switch (type)
        {
        case PRIMITIVETYPE_String:      code = static_cast<int32_t> (PrimitiveTypeCode::String); return true;
        case PRIMITIVETYPE_Integer:     code = static_cast<int32_t> (PrimitiveTypeCode::Integer); return true;
        case PRIMITIVETYPE_Long:        code = static_cast<int32_t> (PrimitiveTypeCode::Long); return true;
        case PRIMITIVETYPE_Double:      code = static_cast<int32_t> (PrimitiveTypeCode::Double); return true;
        case PRIMITIVETYPE_DateTime:    code = static_cast<int32_t> (PrimitiveTypeCode::DateTime); return true;
        case PRIMITIVETYPE_Boolean:     code = static_cast<int32_t> (PrimitiveTypeCode::Boolean); return true;
        case PRIMITIVETYPE_Point2D:     code = static_cast<int32_t> (PrimitiveTypeCode::Point2D); return true;
        case PRIMITIVETYPE_Point3D:     code = static_cast<int32_t> (PrimitiveTypeCode::Point3D); return true;
        default:                        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocPropertyMetadata::IsSupported() const
    {
    return 0 != m_containerIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP AdhocPropertyMetadata::GetPropertyName (Index index) const
    {
    auto const& name = m_metadataPropertyNames[static_cast<size_t> (index)];
    return !name.empty() ? name.c_str() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr AdhocPropertyQuery::GetEntry (uint32_t index) const
    {
    if (!IsSupported())
        return nullptr;

    ECValue v;
    if (ECOBJECTS_STATUS_Success != m_host.GetValue (v, GetContainerPropertyIndex(), index) || v.IsNull())
        return nullptr;
    else if (!v.IsStruct())
        {
        BeAssert (false);
        return nullptr;
        }
    else
        return v.GetStruct();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetValue (ECValueR v, uint32_t index, WCharCP accessor) const
    {
    auto entry = GetEntry (index);
    return entry.IsValid() ? entry->GetValue (v, accessor) : ECOBJECTS_STATUS_PropertyNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocPropertyQuery::GetPropertyIndex (uint32_t& index, WCharCP accessString) const
    {
    if (!IsSupported())
        return false;

    ECValue v;
    if (ECOBJECTS_STATUS_Success == m_host.GetValue (v, GetContainerPropertyIndex()) && v.IsArray())
        {
        uint32_t count = v.GetArrayInfo().GetCount();
        for (uint32_t i = 0; i < count; i++)
            if (ECOBJECTS_STATUS_Success == m_host.GetValue (v, GetContainerPropertyIndex(), i) && !v.IsNull() && v.IsStruct())
                {
                IECInstancePtr instance = v.GetStruct();
                v.SetAllowsPointersIntoInstanceMemory (true);
                if (ECOBJECTS_STATUS_Success == instance->GetValue (v, GetPropertyName (Index::Name)) && !v.IsNull() && v.IsString() && 0 == wcscmp (accessString, v.GetString()))
                    {
                    index = i;
                    return true;
                    }
                }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t AdhocPropertyQuery::GetCount() const
    {
    ECValue v;
    if (IsSupported() && ECOBJECTS_STATUS_Success == m_host.GetValue (v, GetContainerPropertyIndex()) && v.IsArray())
        return v.GetArrayInfo().GetCount();
    else
        return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetName (WStringR name, uint32_t index) const { return GetString (name, index, Index::Name); }
ECObjectsStatus AdhocPropertyQuery::GetExtendedTypeName (WStringR name, uint32_t index) const { return GetString (name, index, Index::ExtendType); }
ECObjectsStatus AdhocPropertyQuery::GetUnitName (WStringR name, uint32_t index) const { return GetString (name, index, Index::Unit); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetDisplayLabel (WStringR label, uint32_t index) const
    {
    auto status = GetString (label, index, Index::DisplayLabel);
    if (ECOBJECTS_STATUS_Success != status)
        {
        WString name;
        status = GetName (name, index);
        if (ECOBJECTS_STATUS_Success == status)
            ECNameValidation::DecodeFromValidName (label, name);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetPrimitiveType (PrimitiveType& type, uint32_t index) const
    {
    auto entry = GetEntry (index);
    if (entry.IsNull())
        return ECOBJECTS_STATUS_PropertyNotFound;

    WCharCP propName = GetPropertyName (Index::Type);
    if (nullptr == propName)
        {
        // defaults to string if no property to specify otherwise
        type = PRIMITIVETYPE_String;
        return ECOBJECTS_STATUS_Success;
        }

    ECValue v;
    if (ECOBJECTS_STATUS_Success != entry->GetValue (v, propName) || !v.IsInteger())
        return ECOBJECTS_STATUS_Error;
    else if (v.IsNull())
        {
        type = PRIMITIVETYPE_String;
        return ECOBJECTS_STATUS_Success;
        }
    else
        return PrimitiveTypeForCode (type, v.GetInteger()) ? ECOBJECTS_STATUS_Success : ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetValue (ECValueR propertyValue, uint32_t index) const
    {
    // avoid looking up the struct instance repeatedly.
    auto entry = GetEntry (index);
    if (entry.IsNull())
        return ECOBJECTS_STATUS_PropertyNotFound;

    // get value type
    PrimitiveType type = PRIMITIVETYPE_String;
    WCharCP propName = GetPropertyName (Index::Type);
    ECValue v;
    if (nullptr != propName)
        {
        auto status = entry->GetValue (v, propName);
        if (ECOBJECTS_STATUS_Success == status)
            {
            // null => use default type (string)
            if (!v.IsNull() && (!v.IsInteger() || !PrimitiveTypeForCode (type, v.GetInteger())))
                status = ECOBJECTS_STATUS_Error;
            }

        if (ECOBJECTS_STATUS_Success != status)
            return status;
        }

    // get value
    auto status = GetValue (propertyValue, *entry, Index::Value);
    if (ECOBJECTS_STATUS_Success != status)
        return status;
    else if (!propertyValue.IsString() && !propertyValue.IsNull())
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_Error;
        }

    // convert string value to desired type
    return propertyValue.ConvertToPrimitiveType (type) ? ECOBJECTS_STATUS_Success : ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::IsReadOnly (bool& isReadOnly, uint32_t index) const
    {
    ECValue v;
    auto status = GetValue (v, index, Index::IsReadOnly);
    isReadOnly = false;
    if (ECOBJECTS_STATUS_Success == status && v.IsBoolean() && !v.IsNull())
        isReadOnly = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::IsHidden (bool& isHidden, uint32_t index) const
    {
    ECValue v;
    auto status = GetValue (v, index, Index::IsHidden);
    isHidden = false;
    if (ECOBJECTS_STATUS_Success == status && v.IsBoolean() && !v.IsNull())
        isHidden = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetString (WStringR str, uint32_t index, Index which) const
    {
    auto entry = GetEntry (index);
    return entry.IsValid() ? GetString (str, *entry, which) : ECOBJECTS_STATUS_PropertyNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetString (WStringR str, IECInstanceCR instance, Index which) const
    {
    ECValue v;
    v.SetAllowsPointersIntoInstanceMemory (true);
    auto status = GetValue (v, instance, which);
    if (ECOBJECTS_STATUS_Success != status)
        return status;
    else if (!v.IsString() && !v.IsNull())
        return ECOBJECTS_STATUS_DataTypeMismatch;

    str = v.GetString();
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetValue (ECValueR v, uint32_t index, Index which) const
    {
    auto entry = GetEntry (index);
    return entry.IsValid() ? GetValue (v, *entry, which) : ECOBJECTS_STATUS_PropertyNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyQuery::GetValue (ECValueR v, IECInstanceCR instance, Index which) const
    {
    WCharCP propName = GetPropertyName (which);
    if (nullptr == propName)
        {
        if (IsRequiredMetadata (which))
            return ECOBJECTS_STATUS_Error;

        v.Clear();
        return ECOBJECTS_STATUS_Success;
        }
    else
        return instance.GetValue (v, propName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocPropertyEdit::AdhocPropertyEdit (IECInstanceR host, WCharCP accessString)
    : AdhocPropertyQuery (host, accessString)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocPropertyEdit::AdhocPropertyEdit (IECInstanceR host, uint32_t propIdx)
    : AdhocPropertyQuery (host, propIdx)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::SetValue (uint32_t index, WCharCP accessor, ECValueCR v)
    {
    auto entry = GetEntry (index);
    return entry.IsValid() ? entry->SetValue (accessor, v) : ECOBJECTS_STATUS_PropertyNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::SetName (uint32_t index, WCharCP name)
    {
    if (!ECNameValidation::IsValidName (name))
        return ECOBJECTS_STATUS_Error;

    auto entry = GetEntry (index);
    if (entry.IsNull())
        return ECOBJECTS_STATUS_PropertyNotFound;
    else
        return entry->SetValue (GetPropertyName (Index::Name), ECValue (name, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::SetDisplayLabel (uint32_t index, WCharCP label, bool andSetName)
    {
    auto entry = GetEntry (index);
    if (entry.IsNull())
        return ECOBJECTS_STATUS_PropertyNotFound;

    auto propName = GetPropertyName (Index::DisplayLabel);
    if (nullptr == propName)
        {
        if (!andSetName)
            return ECOBJECTS_STATUS_Error;
        }
    else
        {
        auto status = entry->SetValue (propName, ECValue (label, false));
        if (ECOBJECTS_STATUS_Success != status)
            return status;
        }

    if (andSetName)
        {
        WString name;
        ECNameValidation::EncodeToValidName (name, label);
        return entry->SetValue (GetPropertyName (Index::Name), ECValue (name.c_str(), false));
        }
    else
        return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::SetValue (uint32_t index, ECValueCR inputV)
    {
    PrimitiveType type;
    auto status = GetPrimitiveType (type, index);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    ECValue v (inputV);
    WString strRep;
    if (!v.ConvertToPrimitiveType (type) || !v.ConvertPrimitiveToString (strRep))
        return ECOBJECTS_STATUS_DataTypeMismatch;

    auto entry = GetEntry (index);
    if (entry.IsNull())
        return ECOBJECTS_STATUS_PropertyNotFound;

    return entry->SetValue (GetPropertyName (Index::Value), ECValue (strRep.c_str(), false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::SetIsReadOnly (uint32_t index, bool isReadOnly)
    {
    auto entry = GetEntry (index);
    if (entry.IsNull())
        return ECOBJECTS_STATUS_PropertyNotFound;

    auto propName = GetPropertyName (Index::IsReadOnly);
    if (nullptr == propName)
        return ECOBJECTS_STATUS_OperationNotSupported;

    return entry->SetValue (propName, ECValue (isReadOnly));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::SetIsHidden (uint32_t index, bool isHidden)
    {
    auto entry = GetEntry (index);
    if (entry.IsNull())
        return ECOBJECTS_STATUS_PropertyNotFound;

    auto propName = GetPropertyName (Index::IsHidden);
    if (nullptr == propName)
        return ECOBJECTS_STATUS_OperationNotSupported;

    return entry->SetValue (propName, ECValue (isHidden));
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct RevertAdhocProperty
    {
private:
    AdhocPropertyEdit&  m_edit;
    uint32_t            m_index;
    bool                m_revert;
public:
    RevertAdhocProperty (AdhocPropertyEdit& edit) : m_edit (edit), m_index (edit.GetCount()), m_revert (true) { }
    ~RevertAdhocProperty()
        {
        if (m_revert)
            m_edit.Remove (m_index);
        }

    void                Clear() { m_revert = false; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP AdhocPropertyQuery::GetStructClass() const
    {
    auto prop = GetHost().GetEnabler().LookupECProperty (GetContainerPropertyIndex());
    auto arrayProp = nullptr != prop ? prop->GetAsArrayProperty() : nullptr;
    auto structClass = nullptr != arrayProp ? arrayProp->GetStructElementType() : nullptr;
    if (nullptr == structClass)
        BeAssert (false);

    return structClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr AdhocPropertyQuery::GetStructEnabler() const
    {
    auto structClass = GetStructClass();
    return GetHost().GetEnablerR().GetEnablerForStructArrayMember (structClass->GetSchema().GetSchemaKey(), structClass->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::Swap (uint32_t propIdxA, uint32_t propIdxB)
    {
    auto entryA = GetEntry (propIdxA), entryB = GetEntry (propIdxB);
    if (entryA.IsNull() || entryB.IsNull())
        return ECOBJECTS_STATUS_PropertyNotFound;

    ECValue v;
    v.SetStruct (entryA.get());
    auto status = GetHostR().SetValue (GetContainerPropertyIndex(), v, propIdxB);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    v.SetStruct (entryB.get());
    status = GetHostR().SetValue (GetContainerPropertyIndex(), v, propIdxA);
    if (ECOBJECTS_STATUS_Success != status)
        {
        v.SetStruct (entryA.get());
        GetHostR().SetValue (GetContainerPropertyIndex(), v, propIdxA);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::Add (WCharCP name, ECValueCR v, WCharCP displayLabel, WCharCP unitName, WCharCP extendedTypeName, bool isReadOnly, bool hidden)
    {
    if (!IsSupported())
        return ECOBJECTS_STATUS_OperationNotSupported;

    if (!ECNameValidation::IsValidName (name))
        return ECOBJECTS_STATUS_Error;

    auto type = PRIMITIVETYPE_String;
    if (!v.IsNull())
        {
        if (!v.IsPrimitive())
            return ECOBJECTS_STATUS_DataTypeMismatch;

        type = v.GetPrimitiveType();
        if (nullptr == GetPropertyName (Index::Type) && type != PRIMITIVETYPE_String)
            return ECOBJECTS_STATUS_OperationNotSupported;  // no property to hold type, so all properties are strings.
        }

    if (PRIMITIVETYPE_String != type && nullptr == GetPropertyName (Index::Type))
        return ECOBJECTS_STATUS_DataTypeMismatch;   // need a property to hold the type if it's not string...

    WString strRep;
    ECValue vAsStr (v);
    if (!vAsStr.ConvertPrimitiveToString (strRep))
        return ECOBJECTS_STATUS_DataTypeMismatch;

    if (nullptr != unitName)
        {
        switch (type)
            {
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_Long:
            case PRIMITIVETYPE_Double:
                break;
            default:
                return ECOBJECTS_STATUS_OperationNotSupported;  // type does not support units.
            }
        }

    // If a property already exists by the same name, we want to replace it (as per managed implementation).
    // And we want to preserve order within array, because that controls order in which ad-hocs are displayed in UI.
    uint32_t existingPropertyIndex = 0;
    bool replacing = GetPropertyIndex (existingPropertyIndex, name);

    auto status = replacing ? ECOBJECTS_STATUS_Success : GetHostR().AddArrayElements (GetContainerPropertyIndex(), 1);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    BeAssert (GetCount() > 0);

    // Ensure that if anything below fails, we remove the new struct array member.
    RevertAdhocProperty revert (*this);

    // Create a new struct array instance.
    auto enabler = GetStructEnabler();
    auto entry = enabler.IsValid() ? enabler->CreateInstance() : nullptr;
    if (entry.IsNull())
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_Error;
        }

    if (ECOBJECTS_STATUS_Success != (status = entry->SetValue (GetPropertyName (Index::Name), ECValue (name, false))) ||
        ECOBJECTS_STATUS_Success != (status = entry->SetValue (GetPropertyName (Index::Value), ECValue (strRep.c_str(), false))))
        return status;

    if (nullptr != displayLabel && ECOBJECTS_STATUS_Success != (status = entry->SetValue (GetPropertyName (Index::DisplayLabel), ECValue (displayLabel, false))))
        return status;

    if (nullptr != unitName && ECOBJECTS_STATUS_Success != (status = entry->SetValue (GetPropertyName (Index::Unit), ECValue (unitName, false))))
        return status;

    if (nullptr != extendedTypeName && ECOBJECTS_STATUS_Success != (status = entry->SetValue (GetPropertyName (Index::ExtendType), ECValue (extendedTypeName, false))))
        return status;

    if (isReadOnly && ECOBJECTS_STATUS_Success != (status = entry->SetValue (GetPropertyName (Index::IsReadOnly), ECValue (isReadOnly))))
        return status;

    if (hidden && ECOBJECTS_STATUS_Success != (status = entry->SetValue (GetPropertyName (Index::IsHidden), ECValue (hidden))))
        return status;

    int32_t typeCode;
    if (PRIMITIVETYPE_String != type && (!CodeForPrimitiveType (typeCode, type) || ECOBJECTS_STATUS_Success != (status = entry->SetValue (GetPropertyName (Index::Type), ECValue (typeCode)))))
        return status;

    // set the struct to the array
    ECValue structV;
    structV.SetStruct (entry.get());
    status = GetHostR().SetValue (GetContainerPropertyIndex(), structV, replacing ? existingPropertyIndex : GetCount() - 1);
    if (ECOBJECTS_STATUS_Success != status)
        {
        BeAssert (false);
        return status;
        }

    revert.Clear();

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::Remove (uint32_t index)
    {
    if (!IsSupported())
        return ECOBJECTS_STATUS_OperationNotSupported;
    else if (index >= GetCount())
        return ECOBJECTS_STATUS_PropertyNotFound;
    else
        return GetHostR().RemoveArrayElement (GetContainerPropertyIndex(), index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::Clear()
    {
    return IsSupported() ? GetHostR().ClearArray (GetContainerPropertyIndex()) : ECOBJECTS_STATUS_OperationNotSupported;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AdhocPropertyEdit::CopyFrom (AdhocPropertyQueryCR query, bool preserveValues)
    {
    if (!IsSupported() || !query.IsSupported())
        return ECOBJECTS_STATUS_Error;

    auto enabler = GetStructEnabler();
    if (enabler.IsNull())
        return ECOBJECTS_STATUS_Error;

    bmap<WString, ECValue> preservedValues;
    if (preserveValues)
        {
        uint32_t count = GetCount();
        for (uint32_t i = 0; i < count; i++)
            {
            IECInstancePtr entry = GetEntry (i);
            WString name;
            ECValue v;
            if (entry.IsValid() && ECOBJECTS_STATUS_Success == GetString (name, *entry, Index::Name) && ECOBJECTS_STATUS_Success == GetValue (v, *entry, Index::Value))
                preservedValues[name] = v;
            }
        }

    auto status = Clear();
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    uint32_t count = query.GetCount();
    if (0 == count)
        return ECOBJECTS_STATUS_Success;

    status = GetHostR().AddArrayElements (GetContainerPropertyIndex(), count);
    if (ECOBJECTS_STATUS_Success != status)
        {
        BeAssert (false && "Failed to add array elements...existing values will be lost");
        return ECOBJECTS_STATUS_Error;
        }

    WString name;
    bmap<WString, ECValue>::const_iterator found;
    for (uint32_t i = 0; i < count; i++)
        {
        IECInstancePtr from = query.GetEntry (i);
        if (from.IsNull())
            {
            GetHostR().RemoveArrayElement (GetContainerPropertyIndex(), i);
            continue;
            }

        auto newEntry = enabler->CreateInstance();
        ECValue v;
        v.SetStruct (newEntry.get());
        if (newEntry.IsNull() || ECOBJECTS_STATUS_Success != newEntry->CopyValues (*from) || ECOBJECTS_STATUS_Success != GetHostR().SetValue (GetContainerPropertyIndex(), v, i))
            {
            GetHostR().RemoveArrayElement (GetContainerPropertyIndex(), i);
            continue;
            }

        if (preserveValues && ECOBJECTS_STATUS_Success == query.GetString (name, *from, Index::Name) && preservedValues.end() != (found = preservedValues.find (name)))
            {
            ECValue v = found->second;
            PrimitiveType type;
            if (ECOBJECTS_STATUS_Success == GetPrimitiveType (type, i) && v.ConvertToPrimitiveType (type))
                SetValue (i, v);
            }
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
AdhocContainerPropertyIndexCollection::const_iterator::const_iterator (ECEnablerCR enabler, bool isEnd)
    : m_enabler (enabler), m_current (isEnd ? 0 : enabler.GetFirstPropertyIndex (0))
    {
    if (!ValidateCurrent())
        MoveNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AdhocContainerPropertyIndexCollection::const_iterator::ValidateCurrent() const
    {
    return !IsEnd() && AdhocPropertyMetadata::IsSupported (m_enabler, m_current);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
void AdhocContainerPropertyIndexCollection::const_iterator::MoveNext()
    {
    if (!IsEnd())
        {
        m_current = m_enabler.GetNextPropertyIndex (0, m_current);
        if (!IsEnd() && !ValidateCurrent())
            MoveNext();
        }
    }

END_BENTLEY_ECOBJECT_NAMESPACE
