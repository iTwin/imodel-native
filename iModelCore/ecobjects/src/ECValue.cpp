/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECValue.cpp $
|
|   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#if defined (_WIN32) // WIP_NONPORT
#include <windows.h>
#endif //defined (_WIN32) // WIP_NONPORT

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
WString SystemTime::ToString
(
)
    {
#if defined (_WIN32) // WIP_NONPORT
    std::wostringstream valueAsString;
    valueAsString << "#" << wYear << "/" << wMonth << "/" << wDay << "-" << wHour << ":" << wMinute << ":" << wSecond << ":" << wMilliseconds << "#";
    return valueAsString.str().c_str();
#else
    return L"";
#endif
    }

/*---------------------------------------------------------------------------------**//**
* Time in local time zone
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SystemTime SystemTime::GetLocalTime()
    {
#if defined (_WIN32) // WIP_NONPORT
    SYSTEMTIME wtime;
    ::GetLocalTime(&wtime);
    SystemTime time;
    memcpy (&time, &wtime, sizeof(time));
    return time;
#elif defined (__unix__)
    // *** NEEDS WORK: Change SystemTime to use a portable time concept such as what's offered in BeTimeUtilities
    SystemTime time;
    return time;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* UTC time
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SystemTime SystemTime::GetSystemTime()
    {
#if defined (_WIN32) // WIP_NONPORT
    SYSTEMTIME wtime;
    ::GetSystemTime(&wtime);
    SystemTime time;
    memcpy (&time, &wtime, sizeof(time));
    return time;
#elif defined (__unix__)
    // *** NEEDS WORK: Change SystemTime to use a portable time concept such as what's offered in BeTimeUtilities
    SystemTime time;
    return time;
#endif
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
    m_valueKind         = VALUEKIND_Uninitialized;
    m_isNull            = true;
    m_isReadOnly        = false;
    m_memoryCallback    = NULL;
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

    switch (m_valueKind)
        {            
        case VALUEKIND_Struct:
            {
            m_structInstance->AddRef();
            break;
            }

        case PRIMITIVETYPE_Binary:
            {
            //WIP_FUSION 
            assert (false && "Needs work: can we copy a binary value? BinaryInfo::m_data is a pointer into somebody's storage container?!");
            break;
            }

        case PRIMITIVETYPE_String:
            {
            // Only make a copy of the string if the original object had a copy.
            if (m_stringInfo.m_freeWhenDone)
                {
                m_stringInfo.m_freeWhenDone = false; // prevent SetString from attempting to free the string that was temporarily copied by memset
                SetString (v.m_stringInfo.m_string);
                }

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
            break;
                        
        default:
            assert (false); // type not handled
        }
    }

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
                free (const_cast<WCharP>(m_stringInfo.m_string));
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
    From (rhs, false);
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
ECValue::ECValue (ECValueCR v, bool doDeepCopy)
    {
    ConstructUninitialized();
    From (v, doDeepCopy);
    }
    
/*---------------------------------------------------------------------------------**//**
*  Construct a Null EC::ECValue (of a specific type, but with IsNull = true)
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (ValueKind classification) : m_valueKind(classification), m_isNull(true), m_memoryCallback(NULL)
    {
    }       

/*---------------------------------------------------------------------------------**//**
*  Construct a Null EC::ECValue (of a specific type, but with IsNull = true)
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue::ECValue (PrimitiveType primitiveType) : m_primitiveType(primitiveType), m_isNull(true), m_memoryCallback(NULL)
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
ECValue::ECValue (WCharCP string, bool holdADuplicate)
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
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECValue::From (ECValueCR v, bool doDeepCopy)
    {
    if (doDeepCopy)
        DeepCopy (v);
    else
        ShallowCopy (v);
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

static const Int64 TICKADJUSTMENT = 504911232000000000LL;     // ticks between 01/01/01 and 01/01/1601

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SystemTime          ECValue::GetDateTime () const
    {
    SystemTime systemTime;
    Int64      systemDateTicks = GetDateTimeTicks ();

    memset (&systemTime, 0, sizeof(systemTime));

#if defined (_WIN32) // WIP_NONPORT
    // m_dateTime is number of ticks since 00:00:00 01/01/01 - Fileticks are relative to 00:00:00 01/01/1601
    systemDateTicks -= TICKADJUSTMENT; 
    FILETIME fileTime;
    fileTime.dwLowDateTime  = systemDateTicks & 0xffffffff;
    fileTime.dwHighDateTime = systemDateTicks >> 32;
    SYSTEMTIME  tempTime;
    if (FileTimeToSystemTime (&fileTime, &tempTime))
        memcpy (&systemTime, &tempTime, sizeof(systemTime));
#elif defined (__unix__)
    // *** NEEDS WORK: Change SystemTime to use a portable time concept such as what's offered in BeTimeUtilities
#endif

    return systemTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus          ECValue::SetDateTime (SystemTime& systemTime) 
    {
#if defined (_WIN32) // WIP_NONPORT
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
#elif defined (__unix__)
    // *** NEEDS WORK: Change SystemTime to use a portable time concept such as what's offered in BeTimeUtilities
#endif

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
WCharCP ECValue::GetString() const
    {
    PRECONDITION (IsString() && "Tried to get string value from an EC::ECValue that is not a string.", L"<Programmer Error: Attempted to get string value from EC::ECValue that is not a string.>");
    return m_stringInfo.m_string;
    };

/*---------------------------------------------------------------------------------**//**
* @param[in] holdADuplicate     If true, EC::ECValue will make a duplicate, otherwise 
*                               EC::ECValue holds the original pointer
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECValue::SetString (WCharCP string, bool holdADuplicate)
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
        m_stringInfo.m_string = BeStringUtilities::Wcsdup (string);
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
WString    ECValue::ToString () const
    {
#if defined (_WIN32) // WIP_NONPORT

    if (IsNull())
        return L"<null>";
        
    std::wostringstream valueAsString; // Not used in all cases, but avoid predicting buffer size when it is used.
    
    if (IsArray())
        {
        ArrayInfo arrayInfo = GetArrayInfo();
        valueAsString << "Count: " << arrayInfo.GetCount() << " IsFixedSize: " << arrayInfo.IsFixedCount();
        }
    else if (IsStruct())
        {
        return L"IECInstance containing struct value";
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
                return GetString();
                }            
            case PRIMITIVETYPE_Boolean:
                {
                return GetBoolean() ? L"true" : L"false";
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
                return timeDate.ToString();
                }
            default:
                {
                return L"EC::ECValue::ToString needs work... unsupported data type";
                }            
            }
        }
        
    return valueAsString.str().c_str();

#elif defined (__unix__) // WIP_NONPORT
        // *** NEEDS WORK: iostreams not supported on Android
    return L"";
#endif
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
* @param[in]        capacity Estimated size of the array.
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
* @param[in]        capacity Estimated size of the array.
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
void ECValueAccessor::Clone (ECValueAccessorCR accessor)
    {
    m_locationVector.clear();

    FOR_EACH (ECValueAccessor::Location const & location, accessor.GetLocationVectorCR())
        PushLocation (*location.enabler, location.propertyIndex, location.arrayIndex);
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
void                                            ECValueAccessor::PushLocation (ECEnablerCR newEnabler, WCharCP accessString, int newArrayIndex)
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
void                                            ECValueAccessor::PushLocation (IECInstanceCR instance, WCharCP accessString, int newArrayIndex)
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
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                                            ECValueAccessor::Clear ()
    {
    m_locationVector.clear();
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
UInt32                                          ECValueAccessor::GetDepth() const
    {
    return (UInt32)m_locationVector.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP                                 ECValueAccessor::GetAccessString () const
    {
    UInt32 depth = GetDepth();
    if (0 == depth)
        return NULL;

    return GetAccessString (depth-1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP                                 ECValueAccessor::GetAccessString (UInt32 depth) const
    {
    int propertyIndex         = m_locationVector[depth].propertyIndex;
    ECEnablerCR enabler       = * m_locationVector[depth].enabler;
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
    UInt32 depth = GetDepth();
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

    // strip [] from array names
    size_t bracketIndex = name.rfind ('[');
    if (WString::npos != bracketIndex)
        {
        name = name.substr (0, bracketIndex);
        }

    return name;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString                                        ECValueAccessor::GetDebugAccessString() const
    {
#if defined (_WIN32) // WIP_NONPORT
    std::wstringstream temp;
    for(UInt32 depth = 0; depth < GetDepth(); depth++)
        {
        if(depth > 0)
            temp << " -> ";
        temp << "{" << m_locationVector[depth].propertyIndex;
        if(m_locationVector[depth].arrayIndex > -1)
            temp << "," << m_locationVector[depth].arrayIndex;
        temp << "}" << GetAccessString (depth);
        }
    return temp.str().c_str();
#elif defined (__unix__) // WIP_NONPORT
    // *** NEEDS WORK: iostreams not supported on Android
    return L"";
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString                                        ECValueAccessor::GetManagedAccessString() const
    {
#if defined (_WIN32) // WIP_NONPORT
    std::wstringstream temp;
    for(UInt32 depth = 0; depth < GetDepth(); depth++)
        {
        if(depth > 0)
            temp << ".";

        WCharCP str = GetAccessString (depth);
        WCharCP lastDot = wcsrchr (str, L'.');

        if (NULL != lastDot)
            str = lastDot+1;

        temp << str;
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
#elif defined (__unix__) // WIP_NONPORT
    // *** NEEDS WORK: iostreams not supported on Android
    return L"";
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                                            ECValueAccessor::MatchesEnabler(UInt32 depth, ECEnablerCR other) const
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

#define NUM_INDEX_BUFFER_CHARS 63
#define NUM_ACCESSSTRING_BUFFER_CHARS 1023

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassP getClassFromSchema (ECSchemaCR rootSchema, WCharCP className)
    {
    ECClassP classP = rootSchema.GetClassP (className);
    if (classP)
        return classP;

    FOR_EACH (ECSchemaCP refSchema, rootSchema.GetReferencedSchemas())
        {
        classP = getClassFromSchema (*refSchema, className);
        if (classP)
            return classP;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassP getPropertyFromClass (ECClassCR enablerClass, WCharCP propertyName)
    {
    ECPropertyP propertyP = enablerClass.GetPropertyP (propertyName);
    if (!propertyP)
        return NULL;

    WString typeName = propertyP->GetTypeName();
    return getClassFromSchema (enablerClass.GetSchema(), typeName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus getECValueAccessorUsingManagedAccessString (wchar_t* asBuffer, wchar_t* indexBuffer, ECValueAccessor& va, ECEnablerCR  enabler, WCharCP managedPropertyAccessor)
    {
    ECObjectsStatus status;
    UInt32          propertyIndex;
    WString        asBufferStr;

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

        // see if the accessstring is to an array
        asBufferStr = managedPropertyAccessor;
        asBufferStr.append (L"[]");
        status = enabler.GetPropertyIndex (propertyIndex, asBufferStr.c_str());

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

    UInt32 indexValue = -1;
    swscanf (indexBuffer, L"%ud", &indexValue);

    ECValue  arrayVal;

    asBufferStr = asBuffer;
    asBufferStr.append (L"[]");

    status = enabler.GetPropertyIndex (propertyIndex, asBufferStr.c_str());
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    // if no character after closing bracket then we just want the array, else we are dealing with a member of a struct array
    if (0 == *(pos2+1))
        {
        va.PushLocation (enabler, propertyIndex, -1);
        return ECOBJECTS_STATUS_Success;
        }

    WString str = asBuffer; 

    ECClassP structClass = getPropertyFromClass (enabler.GetClass(), asBuffer);
    if (!structClass)
        return ECOBJECTS_STATUS_Error;

    EC::ECEnablerP enablerP = const_cast<EC::ECEnablerP>(&enabler);
    StandaloneECEnablerPtr structEnabler = dynamic_cast<StandaloneECEnablerP>(enablerP->GetEnablerForStructArrayMember (structClass->GetSchema().GetName().c_str(), structClass->GetName().c_str()).get());
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
    wchar_t         asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS+1];
    wchar_t         indexBuffer[NUM_INDEX_BUFFER_CHARS+1];
    va.Clear ();
    return getECValueAccessorUsingManagedAccessString (asBuffer, indexBuffer, va, instance.GetEnabler(), managedPropertyAccessor);
    }

#ifdef NO_MORE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECValueAccessor::GetECValue (ECValue& v, IECInstanceCR instance)
    {
    ECObjectsStatus status            = ECOBJECTS_STATUS_Success;
    IECInstanceCP  currentInstance    = &instance;
    IECInstancePtr structInstancePtr;
    for (UInt32 depth = 0; depth < GetDepth(); depth ++)
        {
        Location loc = m_locationVector[depth];

        if (!(loc.enabler == &currentInstance->GetEnabler()))
            return ECOBJECTS_STATUS_Error;

        if (-1 != loc.arrayIndex)
            status = instance.GetValue (v, loc.propertyIndex, loc.arrayIndex);
        else
            status = instance.GetValue (v, loc.propertyIndex);

        if (ECOBJECTS_STATUS_Success != status)
            return status;

        if (v.IsStruct() && 0 <= loc.arrayIndex)
            {
            structInstancePtr = v.GetStruct();
            if (structInstancePtr.IsNull())
                return ECOBJECTS_STATUS_Error;

            // get property and next location
            currentInstance = structInstancePtr.get();
            }
        }

    return status;
    }
#endif

///////////////////////////////////////////////////////////////////////////////////////////
//  ECPropertyValue
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue () {}
ECPropertyValue::ECPropertyValue (IECInstanceCR instance) : m_instance (const_cast <IECInstanceP> (&instance)) {}
ECValueCR           ECPropertyValue::GetValue ()            const    { return m_ecValue; }
IECInstanceCR       ECPropertyValue::GetInstance ()         const    { return *m_instance; }
ECValueAccessorCR   ECPropertyValue::GetValueAccessor ()    const    { return m_accessor; }
ECValueAccessorR    ECPropertyValue::GetValueAccessorR ()            { return m_accessor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECPropertyValue::EvaluateValue ()
    {
    return m_instance->GetValueUsingAccessor (m_ecValue, m_accessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue (ECPropertyValueCR from)
    {
    m_instance = from.m_instance;
    m_accessor = from.m_accessor;
    m_ecValue.From (from.m_ecValue, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue::ECPropertyValue (IECInstanceCR instance, ECValueAccessorCR accessor)
    :
    m_instance (const_cast <IECInstanceP> (&instance)), m_accessor (accessor)
    {
    EvaluateValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool                ECPropertyValue::HasChildValues () const
    {
    return m_ecValue.IsStruct() || m_ecValue.IsArray();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionPtr  ECPropertyValue::GetChildValues () const
    {
    if (m_ecValue.IsStruct() || m_ecValue.IsArray())
        return new ECValuesCollection (*this);

    return new ECValuesCollection ();
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  ECValuesCollectionIterator
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue ECValuesCollectionIterator::GetFirstPropertyValue (IECInstanceCR instance)
    {
    ECValueAccessor firstPropertyAccessor;

    ECEnablerCR enabler = instance.GetEnabler();

    if (1 == enabler.GetPropertyCount())
        return ECPropertyValue ();

    UInt32  firstIndex = enabler.GetFirstPropertyIndex (0);

    if (0 != firstIndex)
        firstPropertyAccessor.PushLocation (enabler, firstIndex, -1);    

    return ECPropertyValue (instance, firstPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollectionIterator::ECValuesCollectionIterator (IECInstanceCR instance)
    :
    m_propertyValue (GetFirstPropertyValue (instance))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyValue ECValuesCollectionIterator::GetChildPropertyValue (ECPropertyValueCR parentPropertyValue)
    {
    ECValueCR       parentValue = parentPropertyValue.GetValue();
    ECValueAccessor childAccessor (parentPropertyValue.GetValueAccessor());

    if (parentValue.IsArray())
        {
        ArrayInfo   arrayInfo  = parentValue.GetArrayInfo();
        UInt32      arrayCount = arrayInfo.GetCount();

        if (0 < arrayCount)
            childAccessor.DeepestLocation().arrayIndex = 0;
        else
            childAccessor.PopLocation();  // WIP_FUSION: better to Clear the accessor?
                                          //             test this with an array with no members
        }
    else
    if (parentValue.IsStruct())
        {
        UInt32          pathLength  = childAccessor.GetDepth();

        if ( ! EXPECTED_CONDITION (0 < pathLength))
            return ECPropertyValue();

        if (ECValueAccessor::INDEX_ROOT != childAccessor[pathLength - 1].arrayIndex)
            {
            IECInstancePtr  structInstance  = parentValue.GetStruct();
            if (structInstance.IsValid())
                {
                ECEnablerCR     enabler         = structInstance->GetEnabler();
                UInt32          firstIndex      = enabler.GetFirstPropertyIndex (0);

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
            UInt32          parentIndex =  childAccessor[pathLength - 1].propertyIndex;
            ECEnablerCR     enabler     = *childAccessor[pathLength - 1].enabler;
            UInt32          firstIndex  =  enabler.GetFirstPropertyIndex (parentIndex);

            if (0 != firstIndex)
                childAccessor.PushLocation (enabler, firstIndex, -1);
            else
                childAccessor.PopLocation();  // WIP_FUSION: better to Clear the accessor?
                                              //             test this with a struct with no members
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
bool            ECValuesCollectionIterator::IsAtEnd() const
    {
    return 0 == m_propertyValue.GetValueAccessor().GetDepth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECValuesCollectionIterator::IsDifferent(ECValuesCollectionIterator const& otherIter) const
    {
    return m_propertyValue.GetValueAccessor() != otherIter.m_propertyValue.GetValueAccessor();
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

    if (ECValueAccessor::INDEX_ROOT != currentAccessor.DeepestLocation().arrayIndex)
        {
        /*--------------------------------------------------------------------------
          If we are on an array member get the next member
        --------------------------------------------------------------------------*/
        ECValueAccessor arrayAccessor (currentAccessor);
        arrayAccessor.DeepestLocation().arrayIndex = ECValueAccessor::INDEX_ROOT;

//WIP_FUSION: wouldn't it be better to get the arrayCount once when the iterator is created?
        ECValue         ecValue;
        ECObjectsStatus status = m_propertyValue.GetInstance().GetValueUsingAccessor (ecValue, arrayAccessor);

        if ( ! EXPECTED_CONDITION (ECOBJECTS_STATUS_Success == status))
            return;

        if ( ! EXPECTED_CONDITION (ecValue.IsArray()))
            return;

        ArrayInfo   arrayInfo  = ecValue.GetArrayInfo();
        UInt32      arrayCount = arrayInfo.GetCount();

        currentAccessor.DeepestLocation().arrayIndex++;

        // If that was the last member of the array, we are done
        if (currentAccessor.DeepestLocation().arrayIndex >= (Int32) arrayCount)
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
        UInt32      pathLength   = currentAccessor.GetDepth();
        UInt32      currentIndex = currentAccessor[pathLength - 1].propertyIndex;
        UInt32      parentIndex = 0;

        // If we are inside an embedded struct get the struct index from the accessor's path
        if (pathLength > 1 && ECValueAccessor::INDEX_ROOT == currentAccessor[pathLength - 2].arrayIndex)
            parentIndex = currentAccessor[pathLength - 2].propertyIndex;

        ECEnablerCP enabler   = currentAccessor[pathLength - 1].enabler;
        UInt32      nextIndex = enabler->GetNextPropertyIndex (parentIndex, currentIndex);

        currentAccessor.DeepestLocation().propertyIndex = nextIndex;

        // If that was the last index in the current struct, we are done
        if (0 == nextIndex)
            {
            currentAccessor.Clear();
            return;
            }
        }

    m_propertyValue.EvaluateValue ();
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
            UInt32      arrayCount = arrayInfo.GetCount();

            if (0 == arrayCount)
                return const_iterator ();
            }

        iter = new ECValuesCollectionIterator (m_parentPropertyValue);
        }

    return const_iterator (*iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECValuesCollection::const_iterator ECValuesCollection::end () const
    {
    return const_iterator ();
    }

END_BENTLEY_EC_NAMESPACE
