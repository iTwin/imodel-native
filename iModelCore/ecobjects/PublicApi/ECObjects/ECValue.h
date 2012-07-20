/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECValue.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <Bentley/VirtualCollectionIterator.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <Geom/GeomApi.h>
struct _FILETIME;

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<ECPropertyValue> ECPropertyValuePtr;
typedef RefCountedPtr<ECValuesCollection> ECValuesCollectionPtr;

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
    ECOBJECTS_EXPORT WString      ToString ();
    ECOBJECTS_EXPORT bool          operator== (const SystemTime&) const;

    ECOBJECTS_EXPORT BentleyStatus  InitFromFileTime (_FILETIME const& fileTime);
    ECOBJECTS_EXPORT BentleyStatus  InitFromUnixMillis (UInt64 unixMillis);
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
//! @ingroup ECObjectsGroup
//! Variant-like object representing the value of an ECPropertyValue. 
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
        WCharCP             m_string;
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
        bool                m_boolean;
        ::Int32             m_integer32;
        ::Int64             m_long64;
        double              m_double;
        StringInfo          m_stringInfo;
        ::Int64             m_dateTime;
        DPoint2d            m_dPoint2d;   
        DPoint3d            m_dPoint3d; 
        ArrayInfo           m_arrayInfo;
        BinaryInfo          m_binaryInfo;
        IECInstanceP        m_structInstance;   // The ECValue class calls AddRef and Release for the member as needed
        };

    void DeepCopy (ECValueCR v);
    void ShallowCopy (ECValueCR v);
    void ConstructUninitialized();
    inline void FreeMemory ();
         
public:
    ECOBJECTS_EXPORT void            Clear();
    ECOBJECTS_EXPORT ECValueR        operator= (ECValueCR rhs);
    
    ECOBJECTS_EXPORT ~ECValue();
    
    ECOBJECTS_EXPORT ECValue ();
    ECOBJECTS_EXPORT ECValue (ECValueCR v, bool doDeepCopy = false);
    ECOBJECTS_EXPORT explicit ECValue (ValueKind classification);
    ECOBJECTS_EXPORT explicit ECValue (PrimitiveType primitiveType);

    ECOBJECTS_EXPORT explicit ECValue (::Int32 integer32);
    ECOBJECTS_EXPORT explicit ECValue (::Int64 long64);
    ECOBJECTS_EXPORT explicit ECValue (double doubleVal);
    ECOBJECTS_EXPORT explicit ECValue (WCharCP string, bool holdADuplicate = true);
    ECOBJECTS_EXPORT explicit ECValue (const byte * blob, size_t size);
    ECOBJECTS_EXPORT explicit ECValue (DPoint2dCR point2d);
    ECOBJECTS_EXPORT explicit ECValue (DPoint3dCR point3d);
    ECOBJECTS_EXPORT explicit ECValue (bool value);
    ECOBJECTS_EXPORT explicit ECValue (SystemTime const& time);

    ECOBJECTS_EXPORT void           SetReadOnly(bool isReadOnly);

    ECOBJECTS_EXPORT bool           IsReadOnly() const;
    ECOBJECTS_EXPORT bool           IsNull() const;
    ECOBJECTS_EXPORT void           SetToNull();
    ECOBJECTS_EXPORT void           From(ECValueCR v, bool doDeepCopy);

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
    ECOBJECTS_EXPORT bool           IsIGeometry() const;

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
        
    ECOBJECTS_EXPORT WCharCP        GetString () const;
/*__PUBLISH_SECTION_END__*/
                     WCharCP        GetString0 () const {return m_stringInfo.m_string;}
/*__PUBLISH_SECTION_START__*/
    ECOBJECTS_EXPORT BentleyStatus  SetString (WCharCP string, bool holdADuplicate = true);

    ECOBJECTS_EXPORT const byte *   GetBinary (size_t& size) const;
    ECOBJECTS_EXPORT BentleyStatus  SetBinary (const byte * data, size_t size, bool holdADuplicate = false);
    
    ECOBJECTS_EXPORT IECInstancePtr GetStruct() const;
    ECOBJECTS_EXPORT BentleyStatus  SetStruct (IECInstanceP structInstance);
        
    ECOBJECTS_EXPORT SystemTime     GetDateTime() const;
    ECOBJECTS_EXPORT BentleyStatus  SetDateTime (SystemTime const& systemTime); 

    ECOBJECTS_EXPORT Int64          GetDateTimeTicks() const;
    ECOBJECTS_EXPORT BentleyStatus  SetDateTimeTicks (Int64 value);

    ECOBJECTS_EXPORT DPoint2d       GetPoint2D() const;
    ECOBJECTS_EXPORT BentleyStatus  SetPoint2D (DPoint2dCR value);

    ECOBJECTS_EXPORT DPoint3d       GetPoint3D() const;
    ECOBJECTS_EXPORT BentleyStatus  SetPoint3D (DPoint3dCR value);

    static UInt32                   GetFixedPrimitiveValueSize (PrimitiveType primitiveType);

    //! This is intended for debugging purposes, not for presentation purposes.
    ECOBJECTS_EXPORT WString       ToString () const;
    
    ECOBJECTS_EXPORT bool           Equals (ECValueCR v) const;
    };

//=======================================================================================    
//! A structure used for describing the complete location of an ECValue within an ECInstance.
//! They can be thought of as the equivalent to access strings, but generally do not require
//! any string manipulation to create or use them.
//! ECValueAccessors consist of a stack of locations, each of which consist of a triplet of 
//! an ECEnabler, property index, and array index.  In cases where the array index is not 
//! applicable (primitive members or the roots of arrays), the INDEX_ROOT constant 
//! is used.  
//! @group "ECInstance"
//! @see ECValue, ECEnabler, ECPropertyValue, ECValuesCollection
//! @bsiclass 
//======================================================================================= 
struct ECValueAccessor
    {
public:

    const static int INDEX_ROOT = -1;
    struct Location
        {
        ECEnablerCP   enabler;
        int           propertyIndex;
        int           arrayIndex;
/*__PUBLISH_SECTION_END__*/
        Location (ECEnablerCP newEnabler, int newPropertyIndex, int newArrayIndex)
            : enabler (newEnabler), propertyIndex (newPropertyIndex), arrayIndex (newArrayIndex)
            {
            }
        Location ()
            :arrayIndex(INDEX_ROOT),enabler(NULL), propertyIndex(0)
            {
            }
        Location (const Location& loc)
            : enabler (loc.enabler), propertyIndex (loc.propertyIndex), arrayIndex (loc.arrayIndex)
            {
            }
/*__PUBLISH_SECTION_START__*/
        };

    typedef bvector<Location> LocationVector;

public:
    LocationVector const &   GetLocationVectorCR() const;

private:
    //"BACK" OF VECTOR IS DEEPEST ELEMENT
    LocationVector          m_locationVector;
    const LocationVector&   GetLocationVector() const;

public:
/*__PUBLISH_SECTION_END__*/

    ECOBJECTS_EXPORT Location&          operator[] (UInt32 depth);

    //! Constructs an ECValueAccessor for a given instance.
    //! @param[in]      instance         The instance that the accessor is representative of.
    //! @param[in]      newPropertyIndex The property index of the ECProperty.
    //! @param[in]      newArrayIndex    The array index of the ECProperty, or INDEX_ROOT
    ECOBJECTS_EXPORT ECValueAccessor (IECInstanceCR instance, 
                                      int newPropertyIndex, 
                                      int newArrayIndex=INDEX_ROOT);

    //! Constructs an ECValueAccessor for a given Enabler.
    //! @param[in]      enabler          The ECEnabler that the accessor is representative of.
    //! @param[in]      newPropertyIndex The property index of the ECProperty.
    //! @param[in]      newArrayIndex    The array index of the ECProperty, or INDEX_ROOT
    ECOBJECTS_EXPORT ECValueAccessor (ECEnablerCR enabler, 
                                      int newPropertyIndex, 
                                      int newArrayIndex=INDEX_ROOT);

    //! Clone an existing ECValueAccessor. Any existing locations are clear so the resulting accessor refers to the same property.
    //! @param[in]      accessor          The accessor to clone.
    ECOBJECTS_EXPORT void Clone (ECValueAccessorCR accessor);

    //! For use by the iterator.  Does not make valid accessors.
    ECOBJECTS_EXPORT ECValueAccessor (IECInstanceCR instance);
    ECOBJECTS_EXPORT ECValueAccessor (ECEnablerCR layout);

    ECOBJECTS_EXPORT const Location&        operator[] (UInt32 depth) const;
    ECOBJECTS_EXPORT ECEnablerCR            GetEnabler (UInt32 depth) const;
 
    //! Determines whether or not the ECEnabler matches that of the accessor at the given depth.
    //! @param[in]      depth           The stack depth of the Accessor's ECEnablerPtr.
    //! @param[in]      other           The ECEnablerPtr to compare to.
    //! @return         true if the ECEnablerPtr are equivalent, otherwise false.
    ECOBJECTS_EXPORT bool                   MatchesEnabler (UInt32 depth, ECEnablerCR other) const;

    //! Looks up and returns the ECProperty associated with this accessor
    ECOBJECTS_EXPORT EC::ECPropertyCP       GetECProperty() const;

/*__PUBLISH_SECTION_START__*/

    ECOBJECTS_EXPORT UInt32                 GetDepth() const;

    //! Gets the native-style access string for a given stack depth.  This access string does 
    //! not contain an array index, and is compatible with the Get/Set methods in IECInstance.
    //! @param[in]      depth           The stack depth of the native access string.
    //! @return         The access string.
    //! @see            IECInstance
    ECOBJECTS_EXPORT WCharCP                GetAccessString (UInt32 depth) const;
    ECOBJECTS_EXPORT WCharCP                GetAccessString () const;

    ECOBJECTS_EXPORT void  PushLocation (ECEnablerCR, int propertyIndex, int arrayIndex=INDEX_ROOT);
    ECOBJECTS_EXPORT void  PushLocation (ECEnablerCR, WCharCP,   int arrayIndex=INDEX_ROOT);

    ECOBJECTS_EXPORT void  PushLocation (IECInstanceCR, int propertyIndex, int arrayIndex=INDEX_ROOT);
    ECOBJECTS_EXPORT void  PushLocation (IECInstanceCR, WCharCP,   int arrayIndex=INDEX_ROOT);

    ECOBJECTS_EXPORT void       PopLocation ();
    ECOBJECTS_EXPORT Location&  DeepestLocation ();
    ECOBJECTS_EXPORT Location const&  DeepestLocationCR () const;

    ECOBJECTS_EXPORT void  Clear ();

    ECOBJECTS_EXPORT WString               GetDebugAccessString () const;
    ECOBJECTS_EXPORT WString               GetPropertyName () const;

    //! Constructs an empty ECValueAccessor.
    ECOBJECTS_EXPORT ECValueAccessor () { }

    //! Constructs a copy of a ECValueAccessor.
    //! @param[in]      accessor         The accessor to be copied.
    ECOBJECTS_EXPORT ECValueAccessor (ECValueAccessorCR accessor);

    //! Gets the managed-style access string for this Accessor.  Includes the array indicies,
    //! and traverses structs when necessary.  This full access string can be used with 
    //! managed code or the InteropHelper.
    //! @see            ECInstanceInteropHelper
    ECOBJECTS_EXPORT WString               GetManagedAccessString () const;

    ECOBJECTS_EXPORT bool                   operator!=(ECValueAccessorCR accessor) const;
    ECOBJECTS_EXPORT bool                   operator==(ECValueAccessorCR accessor) const;

    ECOBJECTS_EXPORT static ECObjectsStatus PopulateValueAccessor (ECValueAccessor& va, IECInstanceCR instance, WCharCP managedPropertyAccessor);
    };

/*__PUBLISH_SECTION_END__*/
struct ECValuesCollection;
struct ECValuesCollectionIterator;

/*__PUBLISH_SECTION_START__*/

//=======================================================================================  
//! @ingroup ECObjectsGroup
//! Relates an ECProperty with an ECValue. Used when iterating over the values of an ECInstance
//! @bsiclass 
//======================================================================================= 
struct ECPropertyValue : RefCountedBase
    {
/*__PUBLISH_SECTION_END__*/
friend struct ECValuesCollection;
friend struct ECValuesCollectionIterator;

private:
    IECInstanceCP       m_instance;
    ECValueAccessor     m_accessor;
    ECValue             m_ecValue;


public:
    ECPropertyValue ();
    ECPropertyValue (ECPropertyValueCR);
    ECPropertyValue (IECInstanceCR);
    ECOBJECTS_EXPORT ECPropertyValue (IECInstanceCR, ECValueAccessorCR);
    ECOBJECTS_EXPORT ECPropertyValue (IECInstanceCR, ECValueAccessorCR, ECValueCR);

    ECValueAccessorR    GetValueAccessorR ();
    ECObjectsStatus     EvaluateValue ();

/*__PUBLISH_SECTION_START__*/
public:
    ECOBJECTS_EXPORT IECInstanceCR          GetInstance () const;

    ECOBJECTS_EXPORT ECValueCR              GetValue () const;
    ECOBJECTS_EXPORT ECValueAccessorCR      GetValueAccessor () const;
    
    //! Indicates whether the value is an array or struct
    ECOBJECTS_EXPORT bool                   HasChildValues () const;
    
    //! For array and struct values, gets a virtual collection of the embedded values
    ECOBJECTS_EXPORT ECValuesCollectionPtr  GetChildValues () const;
    // TODO?? ECOBJECTS_EXPORT ECPropertyValuePtr     GetPropertyValue (IECInstanceCR, WCharCP propertyName) const;
    };

//=======================================================================================  
//! @see ECValue, ECValueAccessor, ECValuesCollection
//! @bsiclass 
//======================================================================================= 
struct ECValuesCollectionIterator : RefCountedBase, std::iterator<std::forward_iterator_tag, ECPropertyValue const>
    {
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECValuesCollection;

    ECPropertyValue     m_propertyValue;

    ECValuesCollectionIterator (IECInstanceCR);
    ECValuesCollectionIterator (ECPropertyValueCR parentPropertyValue);
    ECValuesCollectionIterator ();

    ECPropertyValue     GetFirstPropertyValue (IECInstanceCR);
    ECPropertyValue     GetChildPropertyValue (ECPropertyValueCR parentPropertyValue);

/*__PUBLISH_SECTION_START__*/

public:
    ECOBJECTS_EXPORT bool               IsDifferent(ECValuesCollectionIterator const& iter) const;
    ECOBJECTS_EXPORT void               MoveToNext ();
    ECOBJECTS_EXPORT ECPropertyValue const& GetCurrent () const;
    };

//=======================================================================================    
//! @bsiclass 
//======================================================================================= 
struct ECValuesCollection : RefCountedBase
    {
/*__PUBLISH_SECTION_END__*/
    friend struct ECPropertyValue;

private:
    ECPropertyValue     m_parentPropertyValue;

    ECValuesCollection ();
    ECValuesCollection (ECPropertyValueCR parentPropValue);
    ECValuesCollection (IECInstanceCR);
/*__PUBLISH_SECTION_START__*/

public:

    typedef VirtualCollectionIterator<ECValuesCollectionIterator> const_iterator;

    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;

    ECOBJECTS_EXPORT static ECValuesCollectionPtr Create (IECInstanceCR);
    ECOBJECTS_EXPORT static ECValuesCollectionPtr Create (ECPropertyValueCR);
    ECOBJECTS_EXPORT ECPropertyValueCR  GetParentProperty () const;
    };

END_BENTLEY_EC_NAMESPACE

//__PUBLISH_SECTION_END__
#include <boost/foreach.hpp>
BENTLEY_ENABLE_BOOST_FOREACH_CONST_ITERATOR(Bentley::EC::ECValuesCollection)
//__PUBLISH_SECTION_START__
