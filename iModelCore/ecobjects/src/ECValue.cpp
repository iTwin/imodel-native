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
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
 bool SystemTime::operator== (const SystemTime& rhs) const
     {
     return 0 == memcmp (this, &rhs, sizeof(SystemTime));
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring SystemTime::ToString
(
)
    {
    std::wostringstream valueAsString;
    valueAsString << "#" << wYear << "/" << wMonth << "/" << wDay << "-" << wHour << ":" << wMinute << ":" << wSecond << ":" << wMilliseconds << "#";
    return valueAsString.str().c_str();
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
#ifndef NDEBUG
    memset (this, 0xBAADF00D, size); // avoid accidental misinterpretation of uninitialized data
#endif
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
    if (this == &v)
        return;

    memcpy (this, &v, sizeof(ECValue));

    if (IsNull())
        return;
        
    switch (m_valueKind)
        {            
        case VALUEKIND_Struct:
            {
            //WIP_FUSION 
            assert (false && "Needs work: copy the struct value");
            break;
            }

        case VALUEKIND_Array:
            {
            //WIP_FUSION 
            assert (false && "It's impossible to 'copy' an array -- the data is not here");
            break;
            }

        case PRIMITIVETYPE_Binary:
            {
            //WIP_FUSION 
            assert (false && "Needs work: can we copy a binary value? BinaryInfo::m_data is a pointer into somebody's storage container?!");
            break;
            }

        case PRIMITIVETYPE_String:
            m_stringInfo.m_freeWhenDone = false; // prevent SetString from attempting to free the string that was temporarily copied by memset
            SetString (v.m_stringInfo.m_string);
            break;

        // the memcpy takes care of these...            
        case PRIMITIVETYPE_Boolean:
        case PRIMITIVETYPE_Integer:
        case PRIMITIVETYPE_Long:
        case PRIMITIVETYPE_Double:
        case PRIMITIVETYPE_Point2D:
        case PRIMITIVETYPE_Point3D:
        case PRIMITIVETYPE_DateTime:
            break;
                        
        default:
            assert (false); // type not handled
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValue::FreeMemory ()
    {
    if (m_isNull)
        return;

    switch (m_primitiveType)
        {
        case PRIMITIVETYPE_String:
            if ((m_stringInfo.m_freeWhenDone) && (m_stringInfo.m_string != NULL))
                free (const_cast<wchar_t *>(m_stringInfo.m_string));
            return;

        case PRIMITIVETYPE_Binary:
            if ((m_binaryInfo.m_data != NULL) && (m_binaryInfo.m_freeWhenDone))
                free ((void*)m_binaryInfo.m_data);
            return;

        case (PrimitiveType)VALUEKIND_Struct:
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
    //if (IsNull())
    //    return;

    FreeMemory ();
    memset (&m_binaryInfo, 0, sizeof m_binaryInfo);
    m_isNull = true;
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

    SetToNull();
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
::Int32         ECValue::GetInteger() const
    {
    PRECONDITION (IsInteger() && "Tried to get integer value from an EC::ECValue that is not an integer.", 0);
    PRECONDITION (!IsNull() && "Getting the value of a NULL non-string primitive is ill-defined", 0);
    return m_integer32;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECValue::SetInteger (::Int32 integer)
    {
    Clear();
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
BentleyStatus       ECValue::SetLong (::Int64 long64)
    {
    Clear();
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
BentleyStatus       ECValue::SetDouble (double value)
    {
    Clear();
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
BentleyStatus       ECValue::SetBoolean (bool value)
    {
    Clear();
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
BentleyStatus       ECValue::SetDateTimeTicks (Int64 value)
    {
    Clear();
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
SystemTime          ECValue::GetDateTime () const
    {
    SystemTime systemTime;
    Int64      systemDateTicks = GetDateTimeTicks ();

    memset (&systemTime, 0, sizeof(systemTime));

    // m_dateTime is number of ticks since 00:00:00 01/01/01 - Fileticks are relative to 00:00:00 01/01/1601
    systemDateTicks -= TICKADJUSTMENT; 
    FILETIME fileTime;
    fileTime.dwLowDateTime  = systemDateTicks & 0xffffffff;
    fileTime.dwHighDateTime = systemDateTicks >> 32;
    SYSTEMTIME  tempTime;
    if (FileTimeToSystemTime (&fileTime, &tempTime))
        memcpy (&systemTime, &tempTime, sizeof(systemTime));

    return systemTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus          ECValue::SetDateTime (SystemTime& systemTime) 
    {
    Clear();
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
BentleyStatus       ECValue::SetPoint2D (DPoint2dCR value)
    {
    Clear();
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
BentleyStatus       ECValue::SetPoint3D (DPoint3dCR value)
    {
    Clear();
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
BentleyStatus ECValue::SetString (const wchar_t * string, bool holdADuplicate)
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
BentleyStatus ECValue::SetBinary (const byte * data, size_t size, bool holdADuplicate)
    {
    Clear();

    m_primitiveType = PRIMITIVETYPE_Binary;
    m_binaryInfo.m_freeWhenDone = holdADuplicate;
    
    if (NULL == data)
        {
        m_binaryInfo.m_data = NULL;
        m_binaryInfo.m_size = 0;
        return SUCCESS;
        }    
    

    m_isNull = false;

    m_binaryInfo.m_size = size;    
    if (holdADuplicate)
        {
        void * duplicateData = malloc (size);
        memcpy (duplicateData, data, size);        
        m_binaryInfo.m_data = (const byte *)duplicateData;
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
    PRECONDITION (IsStruct() && "Tried to get struct value from an EC::ECValue that is not a struct.", 0);
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
            
    m_isNull    = false;

    m_structInstance = structInstance;        
    m_structInstance->AddRef();
    
    return SUCCESS;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
bwstring    ECValue::ToString () const
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
            case PRIMITIVETYPE_Boolean:
                {
                valueAsString << GetBoolean()?"true":"false";
                break;          
                }
            case PRIMITIVETYPE_Point2D:
                {
                DPoint2d point = GetPoint2D();
                valueAsString << "{" << point.x << "," << point.y << "}";
                break;          
                }
            case PRIMITIVETYPE_Point3D:
                {
                DPoint3d point = GetPoint3D();
                valueAsString << "{" << point.x << "," << point.y << ","<< point.z << "}";
                break;          
                }
            case PRIMITIVETYPE_DateTime:
                {
                SystemTime timeDate = GetDateTime();
                valueAsString << timeDate.ToString();
                break;          
                }
            default:
                {
                valueAsString << L"EC::ECValue::ToString needs work... unsupported data type";
                break;          
                }            
            }
        }
        
    return valueAsString.str().c_str();
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
        assert(false && "Comparison of two arrays not implemented in Equals(); there's no way to check the elements");
        return false;
        }
    if (IsStruct())
        return m_structInstance == v.m_structInstance;
    if (GetPrimitiveType() != v.GetPrimitiveType())
        return false;
    if (IsString())
        return 0 == wcscmp (GetString0(), v.GetString0());
    if (IsBinary())
        {
        if (m_binaryInfo.m_size != v.m_binaryInfo.m_size)
            return false;
        if (m_binaryInfo.m_data == v.m_binaryInfo.m_data)
            return true;
        return 0 == memcmp (m_binaryInfo.m_data, v.m_binaryInfo.m_data, m_binaryInfo.m_size);
        }
    size_t primitiveValueSize = (size_t) GetFixedPrimitiveValueSize (GetPrimitiveType());
    //&m_boolean points to the first memory address of the union (as does every other union member)
    return 0 == memcmp (&m_boolean, &v.m_boolean, primitiveValueSize);
    }

/*---------------------------------------------------------------------------------**//**
* @param        capacity IN  Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus   ECValue::SetStructArrayInfo (UInt32 count, bool isFixedCount)
    {
    Clear();
        
    m_valueKind                = VALUEKIND_Array;

    m_arrayInfo.InitializeStructArray (count, isFixedCount);
    
    m_isNull = false; // arrays are never null
    
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @param        capacity IN  Estimated size of the array.
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECValue::SetPrimitiveArrayInfo (PrimitiveType primitiveElementType, UInt32 count, bool isFixedSize)
    {
    Clear();
        
    m_valueKind                = VALUEKIND_Array;

    m_arrayInfo.InitializePrimitiveArray (primitiveElementType, count, isFixedSize);
    
    m_isNull = false; // arrays are never null
    
    return ECOBJECTS_STATUS_Success;
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
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          ECValue::GetFixedPrimitiveValueSize (PrimitiveType primitivetype)
    {
    switch (primitivetype)
        {
        case EC::PRIMITIVETYPE_Integer:
            return sizeof(Int32);
        case EC::PRIMITIVETYPE_Long:
            return sizeof(Int64);
        case EC::PRIMITIVETYPE_Double:
            return sizeof(double);
        case PRIMITIVETYPE_Boolean:
            return sizeof(bool); 
        case PRIMITIVETYPE_Point2D:
            return 2*sizeof(double);
        case PRIMITIVETYPE_Point3D:
            return 3*sizeof(double);
        case PRIMITIVETYPE_DateTime:
            return sizeof(Int64); //ticks
        default:
            DEBUG_FAIL("Most datatypes have not yet been implemented... or perhaps you have passed in a variable-sized type.");
            return 0;
        }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor (IECInstanceCR instance, int newPropertyIndex, int newArrayIndex)
    {
    PushLocation (instance, newPropertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor (ECEnablerCR enabler, int newPropertyIndex, int newArrayIndex)
    {
    PushLocation (enabler, newPropertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::ECValueAccessor (ECValueAccessorCR accessor)
    : m_locationVector (accessor.GetLocationVector())
    {
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
ECEnablerCR                                     ECValueAccessor::GetEnabler (UInt32 depth) const
    {
    return * m_locationVector[depth].enabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::Location&                      ECValueAccessor::operator[] (UInt32 depth)
    {
    return m_locationVector[depth];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
const ECValueAccessor::Location&                ECValueAccessor::operator[] (UInt32 depth) const
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
void                                            ECValueAccessor::PushLocation (ECEnablerCR newEnabler, const wchar_t* accessString, int newArrayIndex)
    {
    UInt32 propertyIndex;
    ECObjectsStatus status = newEnabler.GetPropertyIndex(propertyIndex, accessString);
    if (ECOBJECTS_STATUS_Success != status)
        {
        assert (false && "Could not resolve property index for this access string");
        return;
        }
    PushLocation (newEnabler, (int)propertyIndex, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::PushLocation (IECInstanceCR instance, const wchar_t* accessString, int newArrayIndex)
    {
    PushLocation (instance.GetEnabler(), accessString, newArrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::PopLocation()
    {
    m_locationVector.pop_back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessor::Location&                      ECValueAccessor::DeepestLocation()
    {
    return m_locationVector.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32                                          ECValueAccessor::GetDepth() const
    {
    return (UInt32)m_locationVector.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
const wchar_t *                                 ECValueAccessor::GetAccessString (UInt32 depth) const
    {
    int propertyIndex         = m_locationVector[depth].propertyIndex;
    ECEnablerCR enabler       = * m_locationVector[depth].enabler;
    const wchar_t* accessString;
    if (ECOBJECTS_STATUS_Success == enabler.GetAccessString (accessString, propertyIndex))
        return accessString;
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring                                        ECValueAccessor::GetDebugAccessString() const
    {
    std::wstringstream temp;
    for(UInt32 depth = 0; depth < GetDepth(); depth++)
        {
        if(depth > (UInt32)0)
            temp << " -> ";
        temp << "{" << m_locationVector[depth].propertyIndex;
        if(m_locationVector[depth].arrayIndex > -1)
            temp << "," << m_locationVector[depth].arrayIndex;
        temp << "}" << GetAccessString (depth);
        }
    return temp.str().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring                                        ECValueAccessor::GetManagedAccessString() const
    {
    std::wstringstream temp;
    for(UInt32 depth = 0; depth < GetDepth(); depth++)
        {
        if(depth > (UInt32)0)
            temp << ".";
        temp << GetAccessString (depth);
        //If the current index is an array element,
        if(m_locationVector[depth].arrayIndex > -1)
            {
            //Delete the last ']' from the access string and write a number.
            std::streamoff position = temp.tellp();
            temp.seekp(position - 1);
            temp << m_locationVector[depth].arrayIndex;
            temp << "]";
            }
        }
    return temp.str().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                            ECValueAccessor::MatchesEnabler(UInt32 depth, ECEnablerCR other) const
    {
    // 
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
    for(UInt32 depth = 0; depth < GetDepth(); depth ++)
        {
        if((*this)[depth].enabler != accessor[depth].enabler
            || (*this)[depth].propertyIndex != accessor[depth].propertyIndex
            || (*this)[depth].arrayIndex    != accessor[depth].arrayIndex)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPair::ECValueAccessorPair (ECValueAccessorPairCR pair)
    : m_valueAccessor (pair.GetAccessor())
    {
    SetValue (pair.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPair::ECValueAccessorPair (ECValueCR value, ECValueAccessorCR accessor)
    : m_valueAccessor (accessor)
    {
    SetValue (value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessorPair::SetValue (ECValueCR value)
    {
    if(value.IsNull())
        return;
    if(value.IsArray())
        {
        ArrayInfo info = value.GetArrayInfo();
        if(info.IsStructArray())
            m_value.SetStructArrayInfo (info.GetCount(), info.IsFixedCount());
        else 
            m_value.SetPrimitiveArrayInfo (info.GetElementPrimitiveType(), info.GetCount(), info.IsFixedCount());
        }
    else if(value.IsStruct())
        m_value.SetStruct (value.GetStruct().get());
    else
        m_value = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessorPair::SetAccessor (ECValueAccessorCR accessor)
    {
    m_valueAccessor = accessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueCR                                       ECValueAccessorPair::GetValue() const
    {
    return m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
const                                           ECValueAccessor& ECValueAccessorPair::GetAccessor() const
    {
    return m_valueAccessor;
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  ECValueAccessorPairCollection
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPairCollection::ECValueAccessorPairCollection (ECValueAccessorPairCollectionOptionsR options)
    {
    m_options = & options;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPairCollection::const_iterator ECValueAccessorPairCollection::begin () const
    {
    RefCountedPtr<ECValueAccessorPairCollectionIterator> iter = new ECValueAccessorPairCollectionIterator (*m_options);
    return const_iterator (*iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPairCollection::const_iterator ECValueAccessorPairCollection::end () const
    {
    return ECValueAccessorPairCollection::const_iterator ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPairCollectionOptions::ECValueAccessorPairCollectionOptions (IECInstanceCR instance, bool includeNullValues)
    : m_instance (instance), m_includeNullValues (includeNullValues)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPairCollectionOptionsPtr ECValueAccessorPairCollectionOptions::Create (IECInstanceCR instance, bool includeNullValues)
    {
    return new ECValueAccessorPairCollectionOptions (instance, includeNullValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceCR   ECValueAccessorPairCollectionOptions::GetInstance() const
    {
    return m_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValueAccessorPairCollectionOptions::GetIncludesNullValues() const
    {
    return m_includeNullValues;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValueAccessorPairCollectionOptions::SetIncludesNullValues (bool includeNullValues)
    {
    m_includeNullValues = includeNullValues;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPairCollectionIterator::ECValueAccessorPairCollectionIterator (ECValueAccessorPairCollectionOptionsR options)
    : m_currentAccessor (options.GetInstance(), 
                         ECValueAccessor::INDEX_ROOT, 
                         ECValueAccessor::INDEX_ROOT)
    {
    m_options = & options;
    m_status  = ECOBJECTS_STATUS_Success;
    MoveToNext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPairCollectionIterator::ECValueAccessorPairCollectionIterator ()
    : m_options (NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValueAccessorPairCollectionIterator::IsAtEnd() const
    {
    return (UInt32)0 == m_currentAccessor.GetDepth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValueAccessorPairCollectionIterator::IsDifferent(ECValueAccessorPairCollectionIterator const& otherIter) const
    {
    return m_currentAccessor != otherIter.m_currentAccessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValueAccessorPairCollectionIterator::MoveToNext()
    {
    do
        {
        if(0 == m_currentAccessor.GetDepth())
            break;
        if(!m_currentValue.IsNull() && m_currentValue.IsArray() || 0 <= m_currentAccessor.DeepestLocation().arrayIndex)
            NextArrayElement();
        else    
            NextProperty();
        } while((m_currentValue.IsNull() && !m_options->GetIncludesNullValues()) 
                || m_currentValue.IsArray()
                || m_currentValue.IsStruct());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValueAccessorPairCollectionIterator::NextProperty()
    {
    m_currentAccessor.DeepestLocation().propertyIndex++;
    m_currentAccessor.DeepestLocation().arrayIndex = ECValueAccessor::INDEX_ROOT;
    if(m_currentAccessor.DeepestLocation().propertyIndex >= (int)CurrentMaxPropertyCount())
        {
        //Current property index is out of range for this class or struct.
        m_currentAccessor.PopLocation();
        if(0 < m_currentAccessor.GetDepth())
            NextArrayElement();
        }
    m_status = m_options->GetInstance().GetValueUsingAccessor (m_currentValue, m_currentAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECValueAccessorPairCollectionIterator::NextArrayElement()
    {
    //There are still values left in the array, increase the index and continue grabbing values.
    m_currentAccessor.DeepestLocation().arrayIndex++;
    if(m_currentAccessor.DeepestLocation().arrayIndex >= (int)CurrentMaxArrayLength())
        return NextProperty();
    m_status = m_options->GetInstance().GetValueUsingAccessor (m_currentValue, m_currentAccessor);
    if(!m_currentValue.IsNull() && m_currentValue.IsStruct())
        {
        //We're currently in a struct array, so we have to push an index pair to start stepping through the struct.
        assert (NULL != &m_currentValue.GetStruct() && "NULL struct value retrieved from an ECValue that claims to be a struct");
        if (NULL != &m_currentValue.GetStruct())
            {
            m_currentAccessor.PushLocation (*m_currentValue.GetStruct(),
                                            ECValueAccessor::INDEX_ROOT,
                                            ECValueAccessor::INDEX_ROOT);
            }
        NextProperty();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ECValueAccessorPairCollectionIterator::CurrentMaxArrayLength()
    {
    int currentArrayIndex = m_currentAccessor.DeepestLocation().arrayIndex;
    m_currentAccessor.DeepestLocation().arrayIndex = ECValueAccessor::INDEX_ROOT;
    ECValue tempValue;
    m_status = m_options->GetInstance().GetValueUsingAccessor (tempValue, m_currentAccessor);
    ArrayInfo info = tempValue.GetArrayInfo();
    m_currentAccessor.DeepestLocation().arrayIndex = currentArrayIndex;
    return info.GetCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ECValueAccessorPairCollectionIterator::CurrentMaxPropertyCount()
    {
    return m_currentAccessor.DeepestLocation().enabler->GetPropertyCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECValueAccessorPair         ECValueAccessorPairCollectionIterator::GetCurrent () const
    {
    assert (ECOBJECTS_STATUS_Success == m_status && "Error while attempting to retrieve a value in ECValueAccessor enumeration."); 
    ECValueAccessorPair v(m_currentValue, m_currentAccessor);
    return v;
    }

END_BENTLEY_EC_NAMESPACE
