/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECValue.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            Value::SetReadOnly(bool isReadOnly) 
    { 
    m_isReadOnly = isReadOnly; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsReadOnly() const 
    { 
    return m_isReadOnly; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsNull() const 
    { 
    return m_isNull; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ValueClassification     Value::GetClassification() const 
    { 
    return (ValueClassification) (m_classification & 0xFF);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsUninitialized () const 
    { 
    return GetClassification() == VALUECLASSIFICATION_Uninitialized; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsString () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_String; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsInteger () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Integer; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsLong () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Long; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsDouble () const 
    { 
    return m_primitiveType == PRIMITIVETYPE_Double; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsArray () const 
    { 
    return GetClassification() == VALUECLASSIFICATION_Array; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsStruct () const 
    { 
    return GetClassification() == VALUECLASSIFICATION_Struct; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Value::IsPrimitive () const 
    { 
    return GetClassification() == VALUECLASSIFICATION_Primitive; 
    }

/*---------------------------------------------------------------------------------**//**
* Copies this value, including all strings and array values held by this value.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            Value::ConstructUninitialized()
    {
    int size = sizeof (Value); // currently 32 bytes
    memset (this, 0xBAADF00D, size); // avoid accidental misinterpretation of uninitialized data
    
    m_classification    = VALUECLASSIFICATION_Uninitialized;
    m_isNull            = true;
    m_isReadOnly        = false;
    }
    
/*---------------------------------------------------------------------------------**//**
* Copies this value, including all strings and array values held by this value.
* It duplicates string values, even if the original did not hold a duplicate that is was responsible for freeing.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            Value::DeepCopy (ValueCR v)
    {
    memcpy (this, &v, sizeof(Value));

    if (IsNull())
        return;
        
    switch (m_classification)
        {            
        case VALUECLASSIFICATION_Struct:
            {
            assert (false && "Needs work: copy the struct value");
            break;
            }

        case PRIMITIVETYPE_String:
            m_stringInfo.m_freeWhenDone = false; // prevent SetString from attempting to free the string that was temporarily copied by memset
            SetString (v.m_stringInfo.m_string);
            break;
        // the memset takes care of these...            
        case PRIMITIVETYPE_Integer:
        case PRIMITIVETYPE_Long:
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
void            Value::Clear()
    {
    if (IsNull())
        {
        m_classification = VALUECLASSIFICATION_Uninitialized;
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
    m_classification = VALUECLASSIFICATION_Uninitialized;            
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
Value::Value (ValueClassification classification) : m_classification(classification), m_isNull(true)
    {
    }       

/*---------------------------------------------------------------------------------**//**
*  Construct a Null EC::Value (of a specific type, but with IsNull = true)
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Value (PrimitiveType primitiveType) : m_primitiveType(primitiveType), m_isNull(true)
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
* @bsimethod                                                    AdamKlatzkin    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType         Value::GetPrimitiveType() const
    {
    PRECONDITION (IsPrimitive() && "Tried to get the primitive type of an EC::Value that is not classified as a primitive.", (PrimitiveType)0);    
    return m_primitiveType;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
::Int32         Value::GetInteger() const
    {
    PRECONDITION (IsInteger() && "Tried to get integer value from an EC::Value that is not an integer.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_integer32;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Value::SetInteger (::Int32 integer)
    {
    m_isNull    = false;
    m_primitiveType  = PRIMITIVETYPE_Integer;
    m_integer32 = integer;
    
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
::Int64         Value::GetLong() const
    {
    PRECONDITION (IsLong() && "Tried to get long64 value from an EC::Value that is not an long64.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_long64;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Value::SetLong (::Int64 long64)
    {
    m_isNull    = false;
    m_primitiveType  = PRIMITIVETYPE_Long;
    m_long64    = long64;
    
    return SUCCESS;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
double          Value::GetDouble() const
    {
    PRECONDITION (IsDouble() && "Tried to get double value from an EC::Value that is not an double.", std::numeric_limits<double>::quiet_NaN());
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", std::numeric_limits<double>::quiet_NaN());
    return m_double;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Value::SetDouble (double value)
    {
    m_isNull    = false;
    m_primitiveType  = PRIMITIVETYPE_Double;
    m_double    = value;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
const wchar_t * Value::GetString() const
    {
    PRECONDITION (IsString() && "Tried to get string value from an EC::Value that is not a string.", L"<Programmer Error: Attempted to get string value from EC::Value that is not a string.>");
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
        
    m_primitiveType = PRIMITIVETYPE_String;
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
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
std::wstring    Value::ToString () const
    {
    if (IsNull())
        return L"<null>";
        
    std::wostringstream valueAsString;
    switch (m_classification)
        {
        case PRIMITIVETYPE_Integer:
            {
            valueAsString << GetInteger();
            break;
            }
        case PRIMITIVETYPE_Long:
            {
            valueAsString << GetLong();
            break;
            }            
        case PRIMITIVETYPE_Double:
            {
            valueAsString << GetDouble();
            break;
            }            
        case PRIMITIVETYPE_String:
            {
            valueAsString << GetString();
            break;          
            }
        default:
            {
            valueAsString << L"EC::Value::ToString needs work... unsupported data type";
            break;          
            }            
        }
        
    return valueAsString.str();
    }
    
/*---------------------------------------------------------------------------------**//**
* @param        capacity IN  Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Value::SetStructArrayInfo (UInt32 count, bool isFixedSize, bool isReadOnly)
    {
    Clear();
        
    m_classification                = VALUECLASSIFICATION_Array;
    m_isReadOnly                    = isReadOnly;    

    m_arrayInfo.InitializeStructArray (count, isFixedSize);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @param        capacity IN  Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Value::SetPrimitiveArrayInfo (PrimitiveType primitiveElementType, UInt32 count, bool isFixedSize, bool isReadOnly)
    {
    Clear();
        
    m_classification                = VALUECLASSIFICATION_Array;
    m_isReadOnly                    = isReadOnly;    

    m_arrayInfo.InitializePrimitiveArray (primitiveElementType, count, isFixedSize);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayInfo       Value::GetArrayInfo()
    {
    assert (IsArray());
    
    return m_arrayInfo;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArrayInfo::InitializeStructArray (UInt32 count, bool isFixedSize)
    {
    m_elementClassification = ELEMENTCLASSIFICATION_Struct;
    m_count           = count;
    m_isFixedSize     = isFixedSize;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArrayInfo::InitializePrimitiveArray (PrimitiveType elementPrimitiveType, UInt32 count, bool isFixedSize)
    {
    m_elementPrimitiveType = elementPrimitiveType;
    m_count           = count;
    m_isFixedSize     = isFixedSize;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ArrayInfo::GetCount() const
    {
    return m_count;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ArrayInfo::IsFixedSize() const
    {
    return m_isFixedSize;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ValueClassification        ArrayInfo::GetElementClassification() const
    {        
    return (ValueClassification) (m_elementClassification & 0xFF); 
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType        ArrayInfo::GetElementPrimitiveType() const
    {        
    PRECONDITION (IsPrimitiveArray() && "Tried to get the element primitive type of an ArrayInfo that is not classified as a primitive array.", (PrimitiveType)0);    
    return m_elementPrimitiveType;
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool        ArrayInfo::IsPrimitiveArray() const
    {        
    return GetElementClassification() == ELEMENTCLASSIFICATION_Primitive; 
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool        ArrayInfo::IsStructArray() const
    {        
    return GetElementClassification() == ELEMENTCLASSIFICATION_Struct; 
    }  

END_BENTLEY_EC_NAMESPACE
