/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECValue.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECObjects.h>
#include <Geom\GeomApi.h>

BEGIN_BENTLEY_EC_NAMESPACE

//=======================================================================================    
//! SystemTime structure is used to set and get time data from ECValue objects.
//! @group "ECInstance"
//! @see ECValue
//=======================================================================================    
struct SystemTime
{
public:
    unsigned short wYear;
    unsigned short wMonth;
    unsigned short wDayOfWeek;
    unsigned short wDay;
    unsigned short wHour;
    unsigned short wMinute;
    unsigned short wSecond;
    unsigned short wMilliseconds;

    ECOBJECTS_EXPORT SystemTime(unsigned short year=1601, unsigned short month=1, unsigned short day=1, unsigned short hour=0, unsigned short minute=0, unsigned short second=0, unsigned short milliseconds=0);
    ECOBJECTS_EXPORT static SystemTime GetLocalTime();
    ECOBJECTS_EXPORT static SystemTime GetSystemTime();
    ECOBJECTS_EXPORT bwstring      ToString ();
    };

//=======================================================================================    
//! Information about an array in an EC::IECInstance. Does not contain the actual elements.
//! @group "ECInstance"
//! @see ECValue
//=======================================================================================    
struct ArrayInfo  // FUSION_WIP: this could also fit into 8 bytes if packed properly
    {
private:
    union
        {
        ArrayKind       m_arrayKind;
        PrimitiveType   m_elementPrimitiveType;
        };    
    bool                m_isFixedCount;     // FUSION_WIP Store this as some other (blittable) type.
    UInt32              m_count;
            
public:
    void InitializeStructArray (UInt32 count, bool isFixedSize); // cannot have a real constructor due to inclusion in a union
    void InitializePrimitiveArray (PrimitiveType elementPrimitiveType, UInt32 count, bool isFixedCount); // cannot have a real constructor due to inclusion in a union
    
    ECOBJECTS_EXPORT UInt32          GetCount() const;
    ECOBJECTS_EXPORT bool            IsFixedCount() const;    
    ECOBJECTS_EXPORT bool            IsPrimitiveArray() const;
    ECOBJECTS_EXPORT bool            IsStructArray() const;
    ECOBJECTS_EXPORT ValueKind       GetKind() const;    
    ECOBJECTS_EXPORT PrimitiveType   GetElementPrimitiveType() const;
    };

//=======================================================================================    
//! Variant-like object representing the value of a conceptual ECPropertyValue. 
//! It does not represent a "live" reference into the underlying EC::IECInstance 
//! (or the object that the EC::IECInstance represents). Changing the EC::ECValue will not affect
//! the EC::IECInstance unless you subsequently call SetValue() with it.
//! 
//! @group "ECInstance"
//=======================================================================================    
struct ECValue
    {
private:        
    union
        {
        ValueKind       m_valueKind;
        PrimitiveType   m_primitiveType;
        };
    bool                m_isNull;     
    bool                m_isReadOnly; // Really indicates that the property from which this came is readonly... not the value itself.
    
protected:    
    typedef bvector<ECValue>  ValuesVector;
    typedef bvector<ECValue>* ValuesVectorP;
    
    struct StringInfo
        {
        const wchar_t *     m_string;
        bool                m_freeWhenDone;   // WIP_FUSION: this could be stored in the "header"... shared with other DataTypes that need to be freed
        };                                    //             and it would make max size of StringInfo be 8 bytes

    struct BinaryInfo
        {
        const byte *        m_data;
        size_t              m_size;
        bool                m_freeWhenDone;
        };
        
    union
        {
        bool            m_boolean;
        ::Int32         m_integer32;
        ::Int64         m_long64;
        double          m_double;
        StringInfo      m_stringInfo;
        ::Int64         m_dateTime;
        DPoint2d        m_dPoint2d;   
        DPoint3d        m_dPoint3d;   
        ArrayInfo       m_arrayInfo;
        BinaryInfo      m_binaryInfo;
        
        IECInstanceP  m_structInstance;
        };

    void DeepCopy (ECValueCR v);
    void ConstructUninitialized();
    inline void FreeMemory ();
         
public:
    ECOBJECTS_EXPORT void            Clear();
    ECOBJECTS_EXPORT ECValueR        operator= (ECValueCR rhs);
    
    ECOBJECTS_EXPORT ~ECValue();
    
    ECOBJECTS_EXPORT ECValue ();
    ECOBJECTS_EXPORT ECValue (ECValueCR v);
    ECOBJECTS_EXPORT explicit ECValue (ValueKind classification);
    ECOBJECTS_EXPORT explicit ECValue (PrimitiveType primitiveType);

    ECOBJECTS_EXPORT explicit ECValue (::Int32 integer32);
    ECOBJECTS_EXPORT explicit ECValue (::Int64 long64);
    ECOBJECTS_EXPORT explicit ECValue (double doubleVal);
    ECOBJECTS_EXPORT explicit ECValue (const wchar_t * string, bool holdADuplicate = true);
    ECOBJECTS_EXPORT explicit ECValue (const byte * blob, size_t size);
    ECOBJECTS_EXPORT explicit ECValue (DPoint2dCR point2d);
    ECOBJECTS_EXPORT explicit ECValue (DPoint3dCR point3d);
    ECOBJECTS_EXPORT explicit ECValue (bool value);
    ECOBJECTS_EXPORT explicit ECValue (SystemTime& time);

    ECOBJECTS_EXPORT void           SetReadOnly(bool isReadOnly);

    ECOBJECTS_EXPORT bool           IsReadOnly() const;
    ECOBJECTS_EXPORT bool           IsNull() const;
    ECOBJECTS_EXPORT void           SetToNull();

    ECOBJECTS_EXPORT ValueKind      GetKind() const;
    ECOBJECTS_EXPORT bool           IsUninitialized () const;
    
    ECOBJECTS_EXPORT bool           IsString () const;
    ECOBJECTS_EXPORT bool           IsInteger () const;
    ECOBJECTS_EXPORT bool           IsLong () const;
    ECOBJECTS_EXPORT bool           IsDouble () const;
    ECOBJECTS_EXPORT bool           IsBinary () const;
    ECOBJECTS_EXPORT bool           IsBoolean () const;
    
    ECOBJECTS_EXPORT bool           IsPoint2D () const; 
    ECOBJECTS_EXPORT bool           IsPoint3D () const; 
    ECOBJECTS_EXPORT bool           IsDateTime () const; 

    ECOBJECTS_EXPORT bool           IsArray () const;
    ECOBJECTS_EXPORT bool           IsStruct () const;
    ECOBJECTS_EXPORT bool           IsPrimitive () const;
        
    ECOBJECTS_EXPORT PrimitiveType  GetPrimitiveType() const;
    ECOBJECTS_EXPORT BentleyStatus  SetPrimitiveType(PrimitiveType primitiveElementType);

    ECOBJECTS_EXPORT ECObjectsStatus  SetStructArrayInfo (UInt32 count, bool isFixedSize);
    ECOBJECTS_EXPORT ECObjectsStatus  SetPrimitiveArrayInfo (PrimitiveType primitiveElementtype, UInt32 count, bool isFixedSize);
    ECOBJECTS_EXPORT ArrayInfo      GetArrayInfo() const;
    
    ECOBJECTS_EXPORT Int32          GetInteger () const;
    ECOBJECTS_EXPORT BentleyStatus  SetInteger (Int32 integer);
    
    ECOBJECTS_EXPORT Int64          GetLong () const;
    ECOBJECTS_EXPORT BentleyStatus  SetLong (Int64 long64);
 
    ECOBJECTS_EXPORT bool           GetBoolean () const;
    ECOBJECTS_EXPORT BentleyStatus  SetBoolean (bool value);

    //! @returns    The double held by the ECValue, or std::numeric_limits<double>::quiet_NaN() if it is not a double or IsNull
    ECOBJECTS_EXPORT double         GetDouble () const;
    ECOBJECTS_EXPORT BentleyStatus  SetDouble (double value);  
        
    ECOBJECTS_EXPORT const wchar_t *GetString () const;
                     const wchar_t *GetString0 () const {return m_stringInfo.m_string;}
    ECOBJECTS_EXPORT BentleyStatus  SetString (const wchar_t * string, bool holdADuplicate = true);

    ECOBJECTS_EXPORT const byte *   GetBinary (size_t& size) const;
    ECOBJECTS_EXPORT BentleyStatus  SetBinary (const byte * data, size_t size, bool holdADuplicate = false);
    
    ECOBJECTS_EXPORT IECInstancePtr  GetStruct() const;
    ECOBJECTS_EXPORT BentleyStatus   SetStruct (IECInstanceP structInstance);
        
    ECOBJECTS_EXPORT SystemTime     GetDateTime() const;
    ECOBJECTS_EXPORT BentleyStatus  SetDateTime (SystemTime& systemTime); 

    ECOBJECTS_EXPORT Int64          GetDateTimeTicks() const;
    ECOBJECTS_EXPORT BentleyStatus  SetDateTimeTicks (Int64 value);

    ECOBJECTS_EXPORT DPoint2d       GetPoint2D() const;
    ECOBJECTS_EXPORT BentleyStatus  SetPoint2D (DPoint2dCR value);

    ECOBJECTS_EXPORT DPoint3d       GetPoint3D() const;
    ECOBJECTS_EXPORT BentleyStatus  SetPoint3D (DPoint3dCR value);

    static UInt32                   GetFixedPrimitiveValueSize (PrimitiveType primitiveType);

    //! This is intended for debugging purposes, not for presentation purposes.
    ECOBJECTS_EXPORT bwstring   ToString () const;
    
    };

END_BENTLEY_EC_NAMESPACE


