/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECValue.cpp $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Copies this value, including all strings and array values held by this value.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void Value::ConstructUninitialized()
    {
    int size = sizeof (Value); // currently 32 bytes
    memset (this, 0xBAADF00D, size); // avoid accidental misinterpretation of uninitialized data
    
    m_dataType      = DATATYPE_Uninitialized;
    m_isNull        = true;
    m_isReadOnly    = false;
    }
    
/*---------------------------------------------------------------------------------**//**
* Copies this value, including all strings and array values held by this value.
* It duplicates string values, even if the original did not hold a duplicate that is was responsible for freeing.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void Value::DeepCopy (ValueCR v)
    {
    memcpy (this, &v, sizeof(Value));
    /*ConstructUninitialized ();    
    
    m_dataType      = v.m_dataType;
    m_isNull        = v.m_isNull;
    m_isReadOnly    = v.m_isReadonly;
    */
    if (IsNull())
        return;
        
    switch (m_dataType)
        {
        case DATATYPE_String:
            m_stringInfo.m_freeWhenDone = false; // prevent SetString from attempting to free the string that was temporarily copied by memset
            SetString (v.m_stringInfo.m_string);
            break;
            
        case DATATYPE_Struct:
            {
            assert (false && "Needs work: copy the struct value");
            break;
            }
        // the memset takes care of these...            
        case DATATYPE_Integer32:
        case DATATYPE_Long64:
            break;
                        
        default:
            assert (false); // type not handled
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            Value::SetToNull()
    {
    if (IsString())
        SetString (NULL);
    else
        m_isNull = true; //WIP_FUSION handle other types
    }    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void Value::Clear()
    {
    if (IsNull())
        {
        m_dataType = DATATYPE_Uninitialized;
        return;
        }
        
    if (IsUninitialized())
        {
        m_isNull = true;
        return;
        }
        
    if (IsString())
        {
        if (m_stringInfo.m_freeWhenDone)
            free (const_cast<wchar_t *>(m_stringInfo.m_string));
            
        m_stringInfo.m_string = NULL;
        }
        
    m_isNull = true;
    m_dataType = DATATYPE_Uninitialized;            
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Value::~Value()
    {
    Clear();
    memset (this, 0xBAADF00D, sizeof(Value)); // try to make use of destructed values more obvious
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ValueR Value::operator= (ValueCR rhs)
    {
    DeepCopy(rhs);
    return *this;
    }        
    

/*---------------------------------------------------------------------------------**//**
* Construct an uninitialized value
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Value ()
    {
    ConstructUninitialized();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Value (ValueCR v)
    {
    DeepCopy (v);
    }
    
/*---------------------------------------------------------------------------------**//**
*  Construct a Null EC::Value (of a specific type, but with IsNull = true)
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Value (DataType dataType) : m_dataType(dataType), m_isNull(true)
    {
    }       
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Value (::Int32 integer32)
    {
    ConstructUninitialized();
    SetInteger (integer32);
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Value (::Int64 long64)
    {
    ConstructUninitialized();
    SetLong (long64);
    };
            
/*---------------------------------------------------------------------------------**//**
* @param holdADuplicate     IN  If true, EC::Value will make a duplicate, otherwise 
* EC::Value holds the original pointer. Intended only for use when initializing arrays of strings, to avoid duplicating them twice.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Value (const wchar_t * string, bool holdADuplicate)
    {
    ConstructUninitialized();
    SetString (string, holdADuplicate);
    };    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
::Int32 Value::GetInteger() const
    {
    assert (IsInteger() && "Tried to get integer value from an EC::Value that is not an integer.");
    assert (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined");
    return m_integer32;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Value::SetInteger (::Int32 integer)
    {
    m_isNull    = false;
    m_dataType  = DATATYPE_Integer32;
    m_integer32 = integer;
    
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
::Int64 Value::GetLong() const
    {
    assert (IsLong() && "Tried to get long64 value from an EC::Value that is not an long64.");
    assert (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined");
    return m_long64;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Value::SetLong (::Int64 long64)
    {
    m_isNull    = false;
    m_dataType  = DATATYPE_Long64;
    m_long64    = long64;
    
    return SUCCESS;
    }        
            
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
const wchar_t * Value::GetString() const
    {
    assert (IsString() && "Tried to get string value from an EC::Value that is not a string.");
    if (!IsString()) return L"<Programmer Error: Attempted to get string value from EC::Value that is not a string.>"; //WIP_FUSION log as an error... no good can come of it
    return m_stringInfo.m_string;
    };
    
/*---------------------------------------------------------------------------------**//**
* @param holdADuplicate     IN  If true, EC::Value will make a duplicate, otherwise 
*                               EC::Value holds the original pointer
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Value::SetString (const wchar_t * string, bool holdADuplicate)
    {
    Clear();
        
    m_dataType = DATATYPE_String;
    m_stringInfo.m_freeWhenDone = holdADuplicate; // if we hold a duplicate, we are responsible for freeing it
    
    if (NULL == string)
        {
        m_stringInfo.m_string = NULL;
        return SUCCESS;
        }

    m_isNull = false;

    if (holdADuplicate)    
        m_stringInfo.m_string = _wcsdup (string);
    else
        m_stringInfo.m_string = string;
            
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @param        capacity IN  Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt         Value::SetArrayInfo (DataType elementDataType, UInt32 count, bool isFixedSize, bool isReadOnly)
    {
    Clear();
        
    m_dataType                      = DATATYPE_Array;
    m_isReadOnly                    = isReadOnly;    

    m_arrayInfo.Initialize (elementDataType, count, isFixedSize);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayInfo         Value::GetArrayInfo()
    {
    assert (IsArray());
    
    return m_arrayInfo;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void      ArrayInfo::Initialize (DataType elementDataType, UInt32 count, bool isFixedSize)
    {
    m_elementDataType = elementDataType;
    m_count           = count;
    m_isFixedSize     = isFixedSize;
    }
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32      ArrayInfo::GetCount() const
    {
    return m_count;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool      ArrayInfo::IsFixedSize() const
    {
    return m_isFixedSize;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DataType    ArrayInfo::GetElementDataType() const
    {
    return m_elementDataType;
    }  

END_BENTLEY_EC_NAMESPACE
