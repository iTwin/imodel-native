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

#if get_these_from_geomlibs // WIP_FUSION
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
#endif

BEGIN_BENTLEY_EC_NAMESPACE

    
//! Information about an array in an EC::Instance. Does not contain the actual elements.
//! @group "ECInstance"
//! @see Value
struct ArrayInfo  // FUSION_WIP: this could also fit into 8 bytes if packed properly
    {
private:
    union
        {
        ArrayElementClassification  m_elementClassification;
        PrimitiveType               m_elementPrimitiveType;
        };    
    bool            m_isFixedSize;     // FUSION_WIP Store this as some other (blittable) type.
    UInt32          m_count;
            
public:
    void InitializeStructArray (UInt32 count, bool isFixedSize); // cannot have a real constructor due to inclusion in a union
    void InitializePrimitiveArray (PrimitiveType elementPrimitiveType, UInt32 count, bool isFixedSize); // cannot have a real constructor due to inclusion in a union
    UInt32      GetCount() const;
    bool        IsFixedSize() const;    
    bool        IsPrimitiveArray() const;
    bool        IsStructArray() const;
    ValueClassification GetElementClassification() const;    
    PrimitiveType       GetElementPrimitiveType() const;
    };

//! Variant-like object representing the value of a conceptual ECPropertyValue. 
//! It does not represent a "live" reference into the underlying EC::Instance 
//! (or the object that the EC::Instance represents). Changing the EC::Value will not affect
//! the EC::Instance unless you subsequently call SetValue() with it.
//! 
//! @group "ECInstance"
struct Value
    {
private:        
    union
        {
        ValueClassification     m_classification;
        PrimitiveType           m_primitiveType;
        };
    bool        m_isNull;     
    bool        m_isReadOnly; // Really indicates that the property from which this came is readonly... not the value itself.
    
protected:    
    typedef std::vector<Value>  ValuesVector;
    typedef std::vector<Value>* ValuesVectorP;
    
    struct StringInfo
        {
        const wchar_t *     m_string;
        bool                m_freeWhenDone;   // WIP_FUSION: this could be stored in the "header"... shared with other DataTypes that need to be freed
        };                                    //             and it would make max size of StringInfo be 8 bytes
        
    union
        {
        bool            m_boolean;
        ::Int32         m_integer32;
        ::Int64         m_long64;
        double          m_double;
        StringInfo      m_stringInfo;
        const wchar_t * m_dateTime;
#if get_these_from_geomlibs
        DPoint2d        m_dPoint2d;   
        DPoint3d        m_dpoint3d;   
#endif
        ArrayInfo       m_arrayInfo;
        };

    void DeepCopy (ValueCR v);
    void ConstructUninitialized();
                
public:
    ECOBJECTS_EXPORT void            Clear();
    ECOBJECTS_EXPORT ValueR          operator= (ValueCR rhs);
    
    ECOBJECTS_EXPORT ~Value();
    
    ECOBJECTS_EXPORT Value ();
    ECOBJECTS_EXPORT Value (ValueCR v);
    ECOBJECTS_EXPORT Value (ValueClassification classification);
    ECOBJECTS_EXPORT Value (PrimitiveType primitiveType);
    
    ECOBJECTS_EXPORT explicit Value (::Int32 integer32);
    ECOBJECTS_EXPORT explicit Value (::Int64 long64);
    ECOBJECTS_EXPORT explicit Value (const wchar_t * string, bool holdADuplicate = true);

    ECOBJECTS_EXPORT void     SetReadOnly(bool isReadOnly);

    ECOBJECTS_EXPORT bool     IsReadOnly() const;
    ECOBJECTS_EXPORT bool     IsNull() const;
    ECOBJECTS_EXPORT void     SetToNull();

    ECOBJECTS_EXPORT ValueClassification GetClassification() const;
    ECOBJECTS_EXPORT bool     IsUninitialized () const;
    
    ECOBJECTS_EXPORT bool     IsString () const;
    ECOBJECTS_EXPORT bool     IsInteger () const;
    ECOBJECTS_EXPORT bool     IsLong () const;
    ECOBJECTS_EXPORT bool     IsDouble () const;
    
    ECOBJECTS_EXPORT bool     IsArray () const;
    ECOBJECTS_EXPORT bool     IsStruct () const;
    ECOBJECTS_EXPORT bool     IsPrimitive () const;
        
    ECOBJECTS_EXPORT PrimitiveType   GetPrimitiveType() const;

    ECOBJECTS_EXPORT StatusInt       SetStructArrayInfo (UInt32 count, bool isFixedSize, bool isReadOnly);
    ECOBJECTS_EXPORT StatusInt       SetPrimitiveArrayInfo (PrimitiveType primitiveElementtype, UInt32 count, bool isFixedSize, bool isReadOnly);
    ECOBJECTS_EXPORT ArrayInfo       GetArrayInfo();
    
    ECOBJECTS_EXPORT Int32           GetInteger() const;
    ECOBJECTS_EXPORT StatusInt       SetInteger (Int32 integer);
    
    ECOBJECTS_EXPORT Int64           GetLong() const;
    ECOBJECTS_EXPORT StatusInt       SetLong (Int64 long64);
    
    //! @returns    The double held by the Value, or std::numeric_limits<double>::quiet_NaN() if it is not a double or IsNull
    ECOBJECTS_EXPORT double          GetDouble() const;
    ECOBJECTS_EXPORT StatusInt       SetDouble (double value);  
        
    ECOBJECTS_EXPORT const wchar_t * GetString() const;
    ECOBJECTS_EXPORT StatusInt       SetString (const wchar_t * string, bool holdADuplicate = true);
    
    //! This is intended for debugging purposes, not for presentation purposes.
    ECOBJECTS_EXPORT std::wstring    ToString () const;
    
    };

END_BENTLEY_EC_NAMESPACE


