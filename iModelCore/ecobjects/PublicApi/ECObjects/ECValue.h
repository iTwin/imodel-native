/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECValue.h $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECObjects.h>

// needswork, need to reconcile these with the dpoint types defined in geomlibs
struct DPoint2d
    {
    double x;
    double y;
    };
    
struct DPoint3d
    {
    double x;
    double y;
    double z;
    };
    
#include <vector>

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Enumeration of primitive datatypes supported by native "ECObjects" implementation.
* These should correspond to all of the datatypes supported in .NET ECObjects
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
enum DataType
    {
    DATATYPE_Uninitialized                  = 0,
    DATATYPE_Array                          = 1,
    DATATYPE_Struct                         = 2,
    DATATYPE_Integer32                      = 3,
    DATATYPE_Long64                         = 4,
    DATATYPE_String                         = 5,
    DATATYPE_Doubld                         = 6,
    };
    
/*---------------------------------------------------------------------------------**//**
* Information about an array in an EC::Instance. Does not contain the actual elements.
* @group "ECInstance"
* @see Value
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ArrayInfo
    {
private:
    DataType        m_elementDataType;
    bool            m_isFixedSize;
    UInt32          m_count;
            
public:
    void Initialize (DataType elementDataType, UInt32 count, bool isFixedSize); // cannot have a real constructor due to inclusion in a union
    UInt32      GetCount() const;
    bool        IsFixedSize() const;
    DataType    GetElementDataType() const;
    };

/*---------------------------------------------------------------------------------**//**
* Variant-like object representing the value of a conceptual ECPropertyValue. 
* It does not represent a "live" reference into the underlying EC::Instance 
* (or the object that the EC::Instance represents). Changing the EC::Value will not affect
* the EC::Instance unless you subsequently call SetValue() with it.
* 
* @group "ECInstance"
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct Value
    {
private:        
    DataType    m_dataType;   
    bool        m_isNull;     
    bool        m_isReadOnly; // Really indicates that the property from which this came is readonly... not the value itself.
    
protected:    
    typedef std::vector<Value>  ValuesVector;
    typedef std::vector<Value>* ValuesVectorP;
    
    struct StringInfo
        {
        const wchar_t *     m_string;
        bool                m_freeWhenDone;
        };
        
    union
        {
        bool            m_boolean;
        ::Int32         m_integer32;
        ::Int64         m_long64;
        double          m_double;
        StringInfo      m_stringInfo;
        const wchar_t * m_dateTime;
        DPoint2d        m_dPoint2d;
        DPoint3d        m_dpoint3d;
        ArrayInfo       m_arrayInfo;
        };

    void        DeepCopy (ECValueCR v);
    inline void ConstructUninitialized();
                
public:
    void            Clear();
    ECValueR          operator= (ECValueCR rhs);
    
    ~Value();
    
    Value ();
    Value (ECValueCR v);
    Value (DataType dataType);
    
    explicit Value (::Int32 integer32);
    explicit Value (::Int64 long64);
    explicit Value (const wchar_t * string, bool holdADuplicate = true);

    inline void     SetReadOnly(bool isReadOnly) { m_isReadOnly = isReadOnly; };

    inline bool     IsReadOnly()        const { return m_isReadOnly; };
    inline bool     IsNull()            const { return m_isNull; };
    void            SetToNull();

    inline DataType GetDataType()       const { return m_dataType; };
    inline bool     IsUninitialized ()  const { return m_dataType == DATATYPE_Uninitialized; };
    
    inline bool     IsString ()         const { return m_dataType == DATATYPE_String; };
    inline bool     IsInteger ()        const { return m_dataType == DATATYPE_Integer32; };
    inline bool     IsLong ()           const { return m_dataType == DATATYPE_Long64; };
    inline bool     IsArray ()          const { return m_dataType == DATATYPE_Array; };
    inline bool     IsStruct ()         const { return m_dataType == DATATYPE_Struct; };
        
    
    StatusInt       SetArrayInfo (DataType dataType, UInt32 count, bool isFixedSize, bool isReadOnly);
    ArrayInfo       GetArrayInfo();
    
    Int32           GetInteger() const;
    StatusInt       SetInteger (Int32 integer);
    
    Int64           GetLong() const;
    StatusInt       SetLong (Int64 long64);
                
    const wchar_t * GetString() const;
    StatusInt       SetString (const wchar_t * string, bool holdADuplicate = true);
    };

END_BENTLEY_EC_NAMESPACE


