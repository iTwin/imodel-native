/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECValue.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <windows.h>

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SystemTime::SystemTime
(
unsigned short year, 
unsigned short month, 
unsigned short day, 
unsigned short hour, 
unsigned short minute, 
unsigned short second, 
unsigned short milliseconds
)
    {
    wYear =  (year >= 1601 && year < 9999) ? year : 1601;
    wMonth = (month > 0 && month <= 12)? month : 1;
    wDay = (day > 0 && day <= 31) ? day : 1; // TODO: need better validation
    wHour = (hour >= 0 && hour < 24) ? hour : 0;
    wMinute = (minute >= 0 && minute < 60) ? minute : 0;
    wSecond = (second >= 0 && second < 60) ? second : 0;
    wMilliseconds = (milliseconds >= 0 && milliseconds < 1000) ? milliseconds : 0;
    }

/*---------------------------------------------------------------------------------**//**
* Time in local time zone
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SystemTime SystemTime::GetLocalTime()
    {
    SYSTEMTIME wtime;
    ::GetLocalTime(&wtime);
    SystemTime time;
    memcpy (&time, &wtime, sizeof(time));
    return time;
    }

/*---------------------------------------------------------------------------------**//**
* UTC time
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SystemTime SystemTime::GetSystemTime()
    {
    SYSTEMTIME wtime;
    ::GetSystemTime(&wtime);
    SystemTime time;
    memcpy (&time, &wtime, sizeof(time));
    return time;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::SetReadOnly(bool isReadOnly) 
    { 
    m_isReadOnly = isReadOnly; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsReadOnly() const 
    { 
    return m_isReadOnly; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValue::IsNull() const 
    { 
    return m_isNull; 
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
* Copies this value, including all strings and array values held by this value.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::ConstructUninitialized()
    {
    int size = sizeof (ECValue); // currently 32 bytes
    memset (this, 0xBAADF00D, size); // avoid accidental misinterpretation of uninitialized data
    
    m_valueKind    = VALUEKIND_Uninitialized;
    m_isNull            = true;
    m_isReadOnly        = false;
    }
    
/*---------------------------------------------------------------------------------**//**
* Copies this value, including all strings and array values held by this value.
* It duplicates string values, even if the original did not hold a duplicate that is was responsible for freeing.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::DeepCopy (ECValueCR v)
    {
    memcpy (this, &v, sizeof(ECValue));

    if (IsNull())
        return;
        
    switch (m_valueKind)
        {            
        case VALUEKIND_Struct:
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
void            ECValue::SetToNull()
    {
    if (IsString())
        SetString (NULL);
    else if (IsStruct())    
        m_structInstance = NULL;
    
    if (!IsArray()) // arrays can never be null       
        m_isNull = true; //WIP_FUSION handle other types
    }    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::Clear()
    {
    if (IsNull())
        {
        m_valueKind = VALUEKIND_Uninitialized;
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
        
    if (IsStruct())
        {
        m_structInstance = NULL;
        }
        
    m_isNull = true;
    m_valueKind = VALUEKIND_Uninitialized;            
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::~ECValue()
    {
    Clear();
    memset (this, 0xBAADF00D, sizeof(ECValue)); // try to make use of destructed values more obvious
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueR ECValue::operator= (ECValueCR rhs)
    {
    DeepCopy(rhs);
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
    DeepCopy (v);
    }
    
/*---------------------------------------------------------------------------------**//**
*  Construct a Null EC::ECValue (of a specific type, but with IsNull = true)
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (ValueKind classification) : m_valueKind(classification), m_isNull(true)
    {
    }       

/*---------------------------------------------------------------------------------**//**
*  Construct a Null EC::ECValue (of a specific type, but with IsNull = true)
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (PrimitiveType primitiveType) : m_primitiveType(primitiveType), m_isNull(true)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (::Int32 integer32)
    {
    ConstructUninitialized();
    SetInteger (integer32);
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (::Int64 long64)
    {
    ConstructUninitialized();
    SetLong (long64);
    };
         
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (DPoint2dR point2d)
    {
    ConstructUninitialized();
    SetPoint2D (point2d);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (DPoint3dR point3d)
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
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (SystemTime& time)
    {
    ConstructUninitialized();
    SetDateTime (time);
    };

/*---------------------------------------------------------------------------------**//**
* @param holdADuplicate     If true, EC::ECValue will make a duplicate, otherwise 
* EC::ECValue holds the original pointer. Intended only for use when initializing arrays of strings, to avoid duplicating them twice.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (const wchar_t * string, bool holdADuplicate)
    {
    ConstructUninitialized();
    SetString (string, holdADuplicate);
    };

/*---------------------------------------------------------------------------------**//**
* The ECValue is never responsible for freeing the memory... its creator is. 
* The consumer of the ECValue should make a copy of the memory.
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (const byte * data, size_t size)
    {
    ConstructUninitialized();
    SetBinary (data, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType   ECValue::GetPrimitiveType() const
    {
    PRECONDITION (IsPrimitive() && "Tried to get the primitive type of an EC::ECValue that is not classified as a primitive.", (PrimitiveType)0);    
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    10/10
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt       ECValue::SetPrimitiveType (PrimitiveType primitiveType)
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
::Int32         ECValue::GetInteger() const
    {
    PRECONDITION (IsInteger() && "Tried to get integer value from an EC::ECValue that is not an integer.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_integer32;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetInteger (::Int32 integer)
    {
    m_isNull    = false;
    m_primitiveType  = PRIMITIVETYPE_Integer;
    m_integer32 = integer;
    
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
::Int64         ECValue::GetLong() const
    {
    PRECONDITION (IsLong() && "Tried to get long64 value from an EC::ECValue that is not an long64.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_long64;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetLong (::Int64 long64)
    {
    m_isNull    = false;
    m_primitiveType  = PRIMITIVETYPE_Long;
    m_long64    = long64;
    
    return SUCCESS;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
double          ECValue::GetDouble() const
    {
    PRECONDITION (IsDouble() && "Tried to get double value from an EC::ECValue that is not an double.", std::numeric_limits<double>::quiet_NaN());
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", std::numeric_limits<double>::quiet_NaN());
    return m_double;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetDouble (double value)
    {
    m_isNull    = false;
    m_primitiveType  = PRIMITIVETYPE_Double;
    m_double    = value;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool          ECValue::GetBoolean() const
    {
    PRECONDITION (IsBoolean() && "Tried to get boolean value from an EC::ECValue that is not a boolean.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_boolean;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetBoolean (bool value)
    {
    m_isNull         = false;
    m_primitiveType  = PRIMITIVETYPE_Boolean;
    m_boolean        = value;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Int64          ECValue::GetDateTimeTicks() const
    {
    PRECONDITION (IsDateTime() && "Tried to get DateTime value from an EC::ECValue that is not a DateTime.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_dateTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetDateTimeTicks (Int64 value)
    {
    m_isNull         = false;
    m_primitiveType  = PRIMITIVETYPE_DateTime;
    m_dateTime       = value;
    
    return SUCCESS;
    }

//////////////////////////////////////////////////////////////////////////////////////////
// Native Code
// FILETIME   - A file time is a 64-bit value that represents the number of 100-nanosecond 
//              intervals that have elapsed since 00:00:00 01/01/1601.
//
// SYSTEMTIME - A structure that specifies a date and time, using individual members for 
//              the month, day, year, weekday, hour, minute, second, and millisecond.
//
//---------------------------------------------------------------------------------------
// Managed Code
// DateTime   - The DateTime.Ticks value stored in a ECXAttribute represents the number 
//              of 100-nanosecond  intervals that have elapsed since 00:00:00 01/01/01 
//////////////////////////////////////////////////////////////////////////////////////////

static const Int64 TICKADJUSTMENT = 504911232000000000;     // ticks between 01/01/01 and 01/01/1601

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt          ECValue::GetDateTime (SystemTime& systemTime) 
    {
    Int64 systemDateTicks = GetDateTimeTicks ();

    // m_dateTime is number of ticks since 00:00:00 01/01/01 - Fileticks are relative to 00:00:00 01/01/1601
    systemDateTicks -= TICKADJUSTMENT; 
    FILETIME fileTime;
    fileTime.dwLowDateTime  = systemDateTicks & 0xffffffff;
    fileTime.dwHighDateTime = systemDateTicks >> 32;
    SYSTEMTIME  tempTime;
    if (FileTimeToSystemTime (&fileTime, &tempTime))
        {
        memcpy (&systemTime, &tempTime, sizeof(systemTime));
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt          ECValue::SetDateTime (SystemTime& systemTime) 
    {
    FILETIME fileTime;
    SYSTEMTIME wtime;
    memcpy (&wtime, &systemTime, sizeof(wtime));

    // m_dateTime is number of ticks since 00:00:00 01/01/01 - Fileticks are relative to 00:00:00 01/01/1601
    if (SystemTimeToFileTime (&wtime, &fileTime))
        {
        Int64 systemDateTicks = (Int64)fileTime.dwLowDateTime | ((Int64)fileTime.dwHighDateTime << 32);
        systemDateTicks += TICKADJUSTMENT; 
        return SetDateTimeTicks (systemDateTicks);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d          ECValue::GetPoint2D() const
    {
    DPoint2d badValue = {0.0,0.0};
    PRECONDITION (IsPoint2D() && "Tried to get Point2D value from an EC::ECValue that is not a Point2D.", badValue);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", badValue);
    return m_dPoint2d;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetPoint2D (DPoint2dR value)
    {
    m_isNull         = false;
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

    PRECONDITION (IsPoint3D() && "Tried to get Point3D value from an EC::ECValue that is not a Point3D.", badValue);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", badValue);
    return m_dPoint3d;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetPoint3D (DPoint3dR value)
    {
    m_isNull         = false;
    m_primitiveType  = PRIMITIVETYPE_Point3D;
    m_dPoint3d       = value;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
const wchar_t * ECValue::GetString() const
    {
    PRECONDITION (IsString() && "Tried to get string value from an EC::ECValue that is not a string.", L"<Programmer Error: Attempted to get string value from EC::ECValue that is not a string.>");
    return m_stringInfo.m_string;
    };
    
/*---------------------------------------------------------------------------------**//**
* @param holdADuplicate     IN  If true, EC::ECValue will make a duplicate, otherwise 
*                               EC::ECValue holds the original pointer
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECValue::SetString (const wchar_t * string, bool holdADuplicate)
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
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
const byte * ECValue::GetBinary(size_t& size) const
    {
    PRECONDITION (IsBinary() && "Tried to get binarydata from an EC::ECValue that is not binary.", NULL);
    size = m_binaryInfo.m_size;
    return m_binaryInfo.m_data;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECValue::SetBinary (const byte * data, size_t size)
    {
    PRECONDITION (NULL != data, ERROR);
    Clear();

    m_primitiveType = PRIMITIVETYPE_Binary;
    m_binaryInfo.m_data = data;
    m_binaryInfo.m_size = size;
    m_isNull = false;

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  ECValue::GetStruct() const
    {
    PRECONDITION (IsStruct() && "Tried to get struct value from an EC::ECValue that is not a struct.", 0);
    return m_structInstance;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetStruct (IECInstanceR structInstance)
    {
    Clear();
    m_isNull    = false;
    m_valueKind = VALUEKIND_Struct;    
    m_structInstance = &structInstance;        
    
    return SUCCESS;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
std::wstring    ECValue::ToString () const
    {
    if (IsNull())
        return L"<null>";
        
    std::wostringstream valueAsString;
    
    if (IsArray())
        {
        ArrayInfo arrayInfo = GetArrayInfo();
        valueAsString << "Count: " << arrayInfo.GetCount() << " IsFixedSize: " << arrayInfo.IsFixedCount();
        }
    else if (IsStruct())
        {
        valueAsString << "IECInstance containing struct value";
        }
    else
        {
        switch (m_valueKind)
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
                valueAsString << L"EC::ECValue::ToString needs work... unsupported data type";
                break;          
                }            
            }
        }
        
    return valueAsString.str();
    }
    
/*---------------------------------------------------------------------------------**//**
* @param        capacity IN  Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetStructArrayInfo (UInt32 count, bool isFixedCount)
    {
    Clear();
        
    m_valueKind                = VALUEKIND_Array;

    m_arrayInfo.InitializeStructArray (count, isFixedCount);
    
    m_isNull = false; // arrays are never null
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @param        capacity IN  Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECValue::SetPrimitiveArrayInfo (PrimitiveType primitiveElementType, UInt32 count, bool isFixedSize)
    {
    Clear();
        
    m_valueKind                = VALUEKIND_Array;

    m_arrayInfo.InitializePrimitiveArray (primitiveElementType, count, isFixedSize);
    
    m_isNull = false; // arrays are never null
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayInfo       ECValue::GetArrayInfo() const
    {
    assert (IsArray());
    
    return m_arrayInfo;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArrayInfo::InitializeStructArray (UInt32 count, bool isFixedCount)
    {
    m_arrayKind = ARRAYKIND_Struct;
    m_count           = count;
    m_isFixedCount     = isFixedCount;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ArrayInfo::InitializePrimitiveArray (PrimitiveType elementPrimitiveType, UInt32 count, bool isFixedCount)
    {
    m_elementPrimitiveType = elementPrimitiveType;
    m_count           = count;
    m_isFixedCount    = isFixedCount;
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
    return GetKind() == ARRAYKIND_Primitive; 
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ArrayInfo::IsStructArray() const
    {        
    return GetKind() == ARRAYKIND_Struct; 
    }  


END_BENTLEY_EC_NAMESPACE
