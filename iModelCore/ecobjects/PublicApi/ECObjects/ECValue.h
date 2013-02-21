/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECValue.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/// @cond BENTLEY_SDK_Desktop

#include <ECObjects/VirtualCollectionIterator.h>
#include <Bentley/DateTime.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/StandardCustomAttributeHelper.h>
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<ECPropertyValue> ECPropertyValuePtr;
typedef RefCountedPtr<ECValuesCollection> ECValuesCollectionPtr;
//! @group "ECInstance"

//=======================================================================================
//! Information about an array in an ECN::IECInstance. Does not contain the actual elements.
//! @ingroup ECObjectsGroup
//! @see ECValue
//=======================================================================================
struct ArrayInfo
    {
private:
    union
        {
        ArrayKind       m_arrayKind;
        PrimitiveType   m_elementPrimitiveType;
        };
    bool                m_isFixedCount;
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
//! It does not represent a "live" reference into the underlying ECN::IECInstance
//! (or the object that the ECN::IECInstance represents). Changing the ECN::ECValue will not affect
//! the ECN::IECInstance unless you subsequently call SetValue() with it.
//!
//! @group "ECInstance"
//=======================================================================================
struct ECValue
    {
public:
    void ShallowCopy (ECValueCR v);
private:
    union
        {
        ValueKind       m_valueKind;
        PrimitiveType   m_primitiveType;
        };

    UInt8               m_stateFlags;
    mutable UInt8       m_ownershipFlags;       // mutable because string ownership may change when we perform on-demand encoding conversions...

    void                InitForString (void const * str);

protected:
    typedef bvector<ECValue>  ValuesVector;
    typedef bvector<ECValue>* ValuesVectorP;

    struct BinaryInfo
        {
        const byte *        m_data;
        size_t              m_size;
        };

    struct StringInfo
        {
    private:
        friend void ECValue::ShallowCopy (ECValueCR);

        Utf8CP              m_utf8;
        Utf16CP             m_utf16;
#if !defined (_WIN32)
        WCharCP             m_wchar;        // On Windows we use m_utf16. The presence of the extra pointer wouldn't hurt anything but want to ensure it's only used on unix.
#endif
        void                ConvertToUtf8 (UInt8& flags);
        void                ConvertToUtf16 (UInt8& flags);
    public:
        bool                IsUtf8 () const;
        // All the business with the flags parameters is so that StringInfo can modify ECValue's ownership flags.
        // If we stored the flags on StringInfo, we would increase the size of the union.
        WCharCP             GetWChar (UInt8& flags);
        Utf8CP              GetUtf8 (UInt8& flags);
        Utf16CP             GetUtf16 (UInt8& flags);

        void                SetWChar (WCharCP str, UInt8& flags, bool makeCopy);
        void                SetUtf8 (Utf8CP str, UInt8& flags, bool makeCopy);
        void                SetUtf16 (Utf16CP str, UInt8& flags, bool makeCopy);

        void                FreeAndClear (UInt8& flags);
        void                SetNull();          // does not free pointers - used to init from Uninitialized ECValue state
        bool                Equals (StringInfo const& rhs, UInt8& flags);
        };

    struct DateTimeInfo
        {
    private:
        ::Int64             m_ceTicks;
        DateTime::Kind      m_kind;
        DateTime::Component m_component;
        bool                m_isMetadataSet;

    public:
        void Set (::Int64 ceTicks);
        BentleyStatus Set (DateTimeCR dateTime);
        ::Int64 GetCETicks () const;
        BentleyStatus GetDateTime (DateTimeR dateTime) const;

        bool IsMetadataSet () const {return m_isMetadataSet;}
        bool TryGetMetadata (DateTime::Info& metadata) const;
        BentleyStatus SetMetadata (DateTime::Info const& metadata);
        BentleyStatus SetMetadata (ECN::DateTimeInfo const& dateTimeInfo);

        bool MetadataMatches (ECN::DateTimeInfo const& dateTimeInfo) const;

        WString MetadataToString () const;
        };

    union
        {
        bool                m_boolean;
        ::Int32             m_integer32;
        ::Int64             m_long64;
        double              m_double;
        mutable StringInfo  m_stringInfo;       // mutable so that we can convert to requested encoding on demand
        DateTimeInfo        m_dateTimeInfo;
        DPoint2d            m_dPoint2d;
        DPoint3d            m_dPoint3d;
        ArrayInfo           m_arrayInfo;
        BinaryInfo          m_binaryInfo;
        IECInstanceP        m_structInstance;   // The ECValue class calls AddRef and Release for the member as needed
        };

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
    ECOBJECTS_EXPORT explicit ECValue (WCharCP string, bool holdADuplicate = true);
    ECOBJECTS_EXPORT explicit ECValue (Utf8CP string, bool holdADuplicate = true);
    ECOBJECTS_EXPORT explicit ECValue (Utf16CP string, bool holdADuplicate = true);
    ECOBJECTS_EXPORT explicit ECValue (const byte * blob, size_t size);
    ECOBJECTS_EXPORT explicit ECValue (DPoint2dCR point2d);
    ECOBJECTS_EXPORT explicit ECValue (DPoint3dCR point3d);
    ECOBJECTS_EXPORT explicit ECValue (bool value);
    ECOBJECTS_EXPORT explicit ECValue (DateTimeCR dateTime);

    ECOBJECTS_EXPORT void           SetIsReadOnly(bool isReadOnly);
    ECOBJECTS_EXPORT bool           IsReadOnly() const;

    ECOBJECTS_EXPORT void           SetIsNull(bool isNull);
    ECOBJECTS_EXPORT bool           IsNull() const;

    ECOBJECTS_EXPORT void           SetIsLoaded(bool isLoaded);
    ECOBJECTS_EXPORT bool           IsLoaded() const;

    ECOBJECTS_EXPORT void           SetToNull();

    ECOBJECTS_EXPORT void           From(ECValueCR v);

    ECOBJECTS_EXPORT ValueKind      GetKind() const;
    ECOBJECTS_EXPORT bool           IsUninitialized () const;

    ECOBJECTS_EXPORT bool           IsString () const;
    bool                            IsUtf8 () const;

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

/*__PUBLISH_SECTION_END__*/
    // Attempts to convert this ECValue's primitive value to a different primitive type.
    // Currently supported conversions (motivated by ECExpressions):
    //  - double, int, and string are all interconvertible. double => int rounds
    //  - int, long, double, boolean, points, and datetime can be converted to string (uses ToString())
    //  - a null value of any primitive type can be converted a null value of any other primitive type
    ECOBJECTS_EXPORT bool           ConvertToPrimitiveType (PrimitiveType primitiveType);
/*__PUBLISH_SECTION_START__*/

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
    ECOBJECTS_EXPORT Utf8CP         GetUtf8CP () const;
    ECOBJECTS_EXPORT Utf16CP        GetUtf16CP () const;    // the only real caller of this should be ECDBuffer

    ECOBJECTS_EXPORT BentleyStatus  SetString (WCharCP string, bool holdADuplicate = true);
    ECOBJECTS_EXPORT BentleyStatus  SetUtf8CP (Utf8CP string, bool holdADuplicate = true);
    ECOBJECTS_EXPORT BentleyStatus  SetUtf16CP (Utf16CP string, bool holdADuplicate = true);    // primarily for use by ECDBuffer

    ECOBJECTS_EXPORT const byte *   GetBinary (size_t& size) const;
    ECOBJECTS_EXPORT BentleyStatus  SetBinary (const byte * data, size_t size, bool holdADuplicate = false);

    ECOBJECTS_EXPORT IECInstancePtr GetStruct() const;
    ECOBJECTS_EXPORT BentleyStatus  SetStruct (IECInstanceP structInstance);

    //! Gets the DateTime value.
    //! @return DateTime value
    ECOBJECTS_EXPORT DateTime       GetDateTime () const;

    //! Sets the DateTime value.
    //! @param[in] dateTime DateTime value to set
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT BentleyStatus  SetDateTime (DateTimeCR dateTime);

    //! Gets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @return DateTime value as ticks since the beginning of the Common Era epoch.
    ECOBJECTS_EXPORT Int64          GetDateTimeTicks () const;

    //! Gets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @params[out] hasMetadata true, if date time metadata is available in this ECValue, false otherwise.
    //! @params[out] metadata if hasMetadata is true, contains the metadata available in this ECValue.
    //! @return DateTime value as ticks since the beginning of the Common Era epoch.
    ECOBJECTS_EXPORT Int64          GetDateTimeTicks (bool& hasMetadata, DateTime::Info& metadata) const;

    //! Sets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @param[in] ceTicks DateTime Common Era ticks to set
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT BentleyStatus  SetDateTimeTicks (Int64 ceTicks);

    //! Sets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @param[in] ceTicks DateTime Common Era ticks to set
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT BentleyStatus  SetDateTimeTicks (Int64 ceTicks, DateTime::Info const& dateTimeMetadata);

    BentleyStatus SetDateTimeMetadata (ECN::DateTimeInfo const& caDateTimeMetadata);
    bool IsDateTimeMetadataSet () const;
    bool DateTimeInfoMatches (ECN::DateTimeInfo const& caDateTimeMetadata) const;
    WString DateTimeMetadataToString () const;

    //! Returns the DateTime value as milliseconds since the beginning of the Unix epoch.
    //! The Unix epoch begins at 1970-01-01 00:00:00 UTC.
    //! DateTimes before the Unix epoch are negative.
    //! @return DateTime as milliseconds since the beginning of the Unix epoch.
    ECOBJECTS_EXPORT Int64          GetDateTimeUnixMillis() const;

    ECOBJECTS_EXPORT DPoint2d       GetPoint2D() const;
    ECOBJECTS_EXPORT BentleyStatus  SetPoint2D (DPoint2dCR value);

    ECOBJECTS_EXPORT DPoint3d       GetPoint3D() const;
    ECOBJECTS_EXPORT BentleyStatus  SetPoint3D (DPoint3dCR value);

    static ECOBJECTS_EXPORT UInt32  GetFixedPrimitiveValueSize (PrimitiveType primitiveType);

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
//! @ingroup ECObjectsGroup
//! @see ECValue, ECEnabler, ECPropertyValue, ECValuesCollection
//! @bsiclass
//=======================================================================================
struct ECValueAccessor
    {
public:

    const static int INDEX_ROOT = -1;
    struct Location
        {
    private:
        ECEnablerCP             m_enabler;
        int                     m_propertyIndex;
        int                     m_arrayIndex;
/*__PUBLISH_SECTION_END__*/
        mutable ECPropertyCP    m_cachedProperty;
    public:

        Location (ECEnablerCP enabler, int propIdx, int arrayIdx) : m_enabler(enabler), m_propertyIndex(propIdx), m_arrayIndex(arrayIdx), m_cachedProperty(NULL) { }
        Location () : m_enabler(NULL), m_propertyIndex(-1), m_arrayIndex(INDEX_ROOT), m_cachedProperty(NULL) { }
        Location (const Location& loc) : m_enabler(loc.m_enabler), m_propertyIndex(loc.m_propertyIndex), m_arrayIndex(loc.m_arrayIndex), m_cachedProperty(loc.m_cachedProperty) { }

        ECPropertyCP    GetECProperty() const;
        void            SetPropertyIndex (int index)                { m_cachedProperty = NULL; m_propertyIndex = index; }
        void            SetArrayIndex (int index)                   { m_arrayIndex = index; }
        void            IncrementArrayIndex()                       { m_arrayIndex++; }
/*__PUBLISH_SECTION_START__*/
    public:
        ECEnablerCP             GetEnabler() const          { return m_enabler; }
        int                     GetPropertyIndex() const    { return m_propertyIndex; }
        int                     GetArrayIndex() const       { return m_arrayIndex; }
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
    ECOBJECTS_EXPORT ECN::ECPropertyCP       GetECProperty() const;

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
    mutable ECValue     m_ecValue;      // mutable to allow lazy initialization
    mutable bool        m_evaluated;    // mutable to allow lazy initialization

    ECObjectsStatus     EvaluateValue () const;
    void                ResetValue();   // for ECValuesCollection, which reuses the same ECPropertyValue for multiple properties
public:
    ECPropertyValue ();
    ECPropertyValue (ECPropertyValueCR);
    ECPropertyValue (IECInstanceCR);
    ECOBJECTS_EXPORT ECPropertyValue (IECInstanceCR, ECValueAccessorCR);
    ECOBJECTS_EXPORT ECPropertyValue (IECInstanceCR, ECValueAccessorCR, ECValueCR);

    ECValueAccessorR    GetValueAccessorR ();

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
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct ECValuesCollectionIterator : RefCountedBase, std::iterator<std::forward_iterator_tag, ECPropertyValue const>
    {
/*__PUBLISH_SECTION_END__*/
private:
    friend struct ECValuesCollection;

    ECPropertyValue     m_propertyValue;
    int                 m_arrayCount;

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
//! @ingroup ECObjectsGroup
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

END_BENTLEY_ECOBJECT_NAMESPACE

//__PUBLISH_SECTION_END__
#include <boost/foreach.hpp>
BENTLEY_ENABLE_BOOST_FOREACH_CONST_ITERATOR(Bentley::ECN::ECValuesCollection)
//__PUBLISH_SECTION_START__

/// @endcond BENTLEY_SDK_Desktop
