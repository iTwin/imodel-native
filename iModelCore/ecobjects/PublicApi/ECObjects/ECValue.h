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
    ECOBJECTS_EXPORT bool          operator== (const SystemTime&) const;
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
        IECInstanceP        m_structInstance;
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
    
    ECOBJECTS_EXPORT IECInstancePtr GetStruct() const;
    ECOBJECTS_EXPORT BentleyStatus  SetStruct (IECInstanceP structInstance);
        
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
    ECOBJECTS_EXPORT bwstring       ToString () const;
    
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
//! Because of the way ECInstances are laid out in memory, the stack only increases in 
//! size when the accessor describes a value within an element within an array of structs.
//! (A struct that is not part of an array will become part of its parent's ClassLayout.)
//! @group "ECInstance"
//! @see ECValue, ECEnabler, ECValueAccessorPair, ECValueAccessorPairCollection
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
            {
            }
        Location (const Location& loc)
            : enabler (loc.enabler), propertyIndex (loc.propertyIndex), arrayIndex (loc.arrayIndex)
            {
            }
/*__PUBLISH_SECTION_START__*/
        };

    typedef bvector<Location> LocationVector;

private:
    //"BACK" OF VECTOR IS DEEPEST ELEMENT
    LocationVector          m_locationVector;

    Location&               operator[] (UInt32 depth);
    const LocationVector&   GetLocationVector() const;

public:
    //friend ECValueAccessorPairCollectionIterator;
    friend ECValueAccessorPairCollection;
    friend ECValueAccessorPair;

/*__PUBLISH_SECTION_END__*/

    //! Constructs an ECValueAccessor for a given instance.
    //! @param[in]      instance         The instance that the accessor is representative of.
    //! @param[in]      newPropertyIndex The property index of the ECProperty.
    //! @param[in]      newArrayIndex    The array index of the ECProperty, or INDEX_ROOT
    ECOBJECTS_EXPORT ECValueAccessor (IECInstanceCR instance, 
                                      int newPropertyIndex, 
                                      int newArrayIndex);

    //! Constructs an ECValueAccessor for a given Enabler.
    //! @param[in]      enabler          The ECEnabler that the accessor is representative of.
    //! @param[in]      newPropertyIndex The property index of the ECProperty.
    //! @param[in]      newArrayIndex    The array index of the ECProperty, or INDEX_ROOT
    ECOBJECTS_EXPORT ECValueAccessor (ECEnablerCR enabler, 
                                      int newPropertyIndex, 
                                      int newArrayIndex);

    //! For use by the iterator.  Does not make valid accessors.
    ECOBJECTS_EXPORT ECValueAccessor (IECInstanceCR instance);
    ECOBJECTS_EXPORT ECValueAccessor (ECEnablerCR layout);

    ECOBJECTS_EXPORT const Location&        operator[] (UInt32 depth) const;
    ECOBJECTS_EXPORT UInt32                 GetDepth() const;

    ECOBJECTS_EXPORT ECEnablerCR            GetEnabler (UInt32 depth) const;
 
    //! Determines whether or not the ECEnabler matches that of the accessor at the given depth.
    //! @param[in]      depth           The stack depth of the Accessor's ECEnablerPtr.
    //! @param[in]      other           The ECEnablerPtr to compare to.
    //! @return         true if the ECEnablerPtr are equivalent, otherwise false.
    ECOBJECTS_EXPORT bool                   MatchesEnabler (UInt32 depth, ECEnablerCR other) const;

/*__PUBLISH_SECTION_START__*/

    //! Gets the native-style access string for a given stack depth.  This access string does 
    //! not contain an array index, and is compatible with the Get/Set methods in IECInstance.
    //! @param[in]      depth           The stack depth of the native access string.
    //! @return         The access string.
    //! @see            IECInstance
    ECOBJECTS_EXPORT const wchar_t *        GetAccessString (UInt32 depth) const;

    ECOBJECTS_EXPORT void  PushLocation (ECEnablerCR, int, int);
    ECOBJECTS_EXPORT void  PushLocation (ECEnablerCR, const wchar_t *, int);

    ECOBJECTS_EXPORT void  PushLocation (IECInstanceCR, int, int);
    ECOBJECTS_EXPORT void  PushLocation (IECInstanceCR, const wchar_t *, int);

    ECOBJECTS_EXPORT void  PopLocation ();
    Location&              DeepestLocation ();

    ECOBJECTS_EXPORT bwstring               GetDebugAccessString () const;

    //! Constructs an empty ECValueAccessor.
    ECOBJECTS_EXPORT ECValueAccessor () { }

    //! Constructs a copy of a ECValueAccessor.
    //! @param[in]      accessor         The accessor to be copied.
    ECOBJECTS_EXPORT ECValueAccessor (ECValueAccessorCR accessor);

    //! Gets the managed-style access string for this Accessor.  Includes the array indicies,
    //! and traverses structs when necessary.  This full access string can be used with 
    //! managed code or the InteropHelper.
    //! @see            ECInstanceInteropHelper
    ECOBJECTS_EXPORT bwstring               GetManagedAccessString () const;

    ECOBJECTS_EXPORT bool                   operator!=(ECValueAccessorCR accessor) const;
    ECOBJECTS_EXPORT bool                   operator==(ECValueAccessorCR accessor) const;
    };

//=======================================================================================    
//! A structure that pairs ECValues along with their accessors.
//! @group "ECInstance"
//! @see ECValue, ECValueAccessor
//! @bsiclass 
//======================================================================================= 
struct ECValueAccessorPair
    {
public:
    friend ECValueAccessorPairCollection;

    ECValue          m_value;
    ECValueAccessor  m_valueAccessor;
    ECOBJECTS_EXPORT ECValueAccessorPair ();
    ECOBJECTS_EXPORT ECValueAccessorPair (ECValueAccessorPairCR pair);
    ECOBJECTS_EXPORT ECValueAccessorPair (ECValueCR value, ECValueAccessorCR accessor);

    ECOBJECTS_EXPORT void                     SetValue (ECValueCR value);
    ECOBJECTS_EXPORT void                     SetAccessor (ECValueAccessorCR accessor);
    ECOBJECTS_EXPORT ECValueCR                GetValue () const;
    ECOBJECTS_EXPORT ECValueAccessorCR        GetAccessor () const;
    };

//=======================================================================================  
//! @see ECValue, ECValueAccessor, ECValueAccessorPairCollection
//! @bsiclass 
//======================================================================================= 
struct ECValueAccessorPairCollectionOptions : RefCountedBase
    {
private:
/*__PUBLISH_SECTION_END__*/
    bool             m_includeNullValues;
    IECInstanceCR    m_instance;
    ECValueAccessorPairCollectionOptions (IECInstanceCR instance, bool includeNullValues);
/*__PUBLISH_SECTION_START__*/
public:
    ECOBJECTS_EXPORT static RefCountedPtr<ECValueAccessorPairCollectionOptions> Create 
        (
        IECInstanceCR   instance,
        bool            includeNullValues
        );

    ECOBJECTS_EXPORT IECInstanceCR GetInstance ()           const;
    ECOBJECTS_EXPORT bool          GetIncludesNullValues () const;

    ECOBJECTS_EXPORT void          SetIncludesNullValues (bool includesNullValues);
    };
typedef RefCountedPtr<ECValueAccessorPairCollectionOptions> ECValueAccessorPairCollectionOptionsPtr;

// The template below is part of the Bentley API on the trunk, but does not yet exist on the branch.
// Currently, ECValueAccessorPairCollection is the only consumer of this template on the branch,
// so the template is copied here temporarily.  This should be removed during a merge.

/*=================================================================================**//**
* This template is used by iterators that hide their implementation from the
* published API.  Hiding the implementation allows it to be improved, for example
* by adding new data members, without requiring callers to recompile.
*
* To use the template, an iterator class must:
*   1) Satisfy the requirements of RefCountedPtr usually by deriving from RefCountedBase.
*   2) Provide a typedef for its return type, ex:
*       typedef DgnModelRefP ReturnType;
*   3) Provide the following methods:        
*       bool             IsDifferent(MyIterator const& rhs) const;
*       void             MoveToNext ();
*       ReturnType       GetCurrent () const;
*       bool             IsAtEnd () const;
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <typename IteratorImplementation>
struct VirtualCollectionIterator : std::iterator<std::forward_iterator_tag, typename IteratorImplementation::ReturnType>
{
private:
    RefCountedPtr<IteratorImplementation> m_implementation;

public:
    VirtualCollectionIterator () { }
    VirtualCollectionIterator (IteratorImplementation& state) : m_implementation (&state)
        {
        if (m_implementation->IsAtEnd())
            m_implementation = NULL;
        }

    typename IteratorImplementation::ReturnType  operator*() const
        {
        return m_implementation->GetCurrent();
        }

    bool        operator!=(VirtualCollectionIterator const& rhs) const
        {
        if (m_implementation.IsNull() && rhs.m_implementation.IsNull())
            return false;

        if (m_implementation.IsNull() != rhs.m_implementation.IsNull())
            return true;

        return m_implementation->IsDifferent (*rhs.m_implementation.get());
        }
        
    VirtualCollectionIterator&   operator++()
        {
        m_implementation->MoveToNext();

        if (m_implementation->IsAtEnd())
            m_implementation = NULL;

        return *this;
        }
};

//=======================================================================================  
//! @see ECValue, ECValueAccessor, ECValueAccessorPairCollection
//! @bsiclass 
//======================================================================================= 
struct ECValueAccessorPairCollectionIterator : RefCountedBase
    {
private:
/*__PUBLISH_SECTION_END__*/
    friend ECValueAccessorPairCollection;

    ECObjectsStatus                           m_status;
    ECValueAccessor                           m_currentAccessor;
    ECValue                                   m_currentValue;
    ECValueAccessorPairCollectionOptionsPtr   m_options;

    void            NextArrayElement();
    void            NextProperty();
    UInt32          CurrentMaxArrayLength();
    UInt32          CurrentMaxPropertyCount();

    ECValueAccessorPairCollectionIterator (ECValueAccessorPairCollectionOptionsR options);
    ECValueAccessorPairCollectionIterator ();
/*__PUBLISH_SECTION_START__*/
public:
    typedef ECValueAccessorPair             ReturnType;
    ECOBJECTS_EXPORT bool                   IsDifferent(ECValueAccessorPairCollectionIterator const& iter) const;
    ECOBJECTS_EXPORT void                   MoveToNext ();
    ECOBJECTS_EXPORT ECValueAccessorPair    GetCurrent () const;
    ECOBJECTS_EXPORT bool                   IsAtEnd () const;
    };

//=======================================================================================  
//! ECValueAccessorPairCollection describes a set of ECValues and accessors that make up
//! an ECInstance.  These values are found as needed.  It supports STL-like iteration, 
//! and will recursively return values that are part of inner structs and struct array 
//! members.  It will never return an ECValue representing a struct or an array.
//! @see ECValue, ECValueAccessor, ECValueAccessorPair
//! @bsiclass 
//======================================================================================= 
struct ECValueAccessorPairCollection
    {
private:
    ECValueAccessorPairCollectionOptionsPtr  m_options;

public:
    ECOBJECTS_EXPORT ECValueAccessorPairCollection (ECValueAccessorPairCollectionOptionsR options);

    typedef VirtualCollectionIterator<ECValueAccessorPairCollectionIterator> const_iterator;

    ECOBJECTS_EXPORT const_iterator begin () const;
    ECOBJECTS_EXPORT const_iterator end ()   const;
    };

END_BENTLEY_EC_NAMESPACE


