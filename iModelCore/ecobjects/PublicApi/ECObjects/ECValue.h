/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECValue.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/VirtualCollectionIterator.h>
#include <Bentley/DateTime.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/StandardCustomAttributeHelper.h>
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<ECPropertyValue> ECPropertyValuePtr;
typedef RefCountedPtr<ECValuesCollection> ECValuesCollectionPtr;

//=======================================================================================    
//! Information about an array in an ECN::IECInstance. Does not contain the actual elements.
//! @addtogroup ECObjectsGroup
//! @beginGroup
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
    uint32_t            m_count;

public:
    //! Initialize the array as a struct array with the given number of entries
    //! @param[in] count        How many elements the array (initially) holds
    //! @param[in] isFixedSize  Indicates whether the array can grow or not
    void InitializeStructArray (uint32_t count, bool isFixedSize); // cannot have a real constructor due to inclusion in a union

    //! Initialize the array as a primitive array with the given number of entries
    //! @param[in] elementPrimitiveType The PrimitiveType of elements that this array can hold
    //! @param[in] count                How many elements the array (initially) holds
    //! @param[in] isFixedCount         Indicates whether the array can grow or not
    void InitializePrimitiveArray (PrimitiveType elementPrimitiveType, uint32_t count, bool isFixedCount); // cannot have a real constructor due to inclusion in a union
    
    //! Returns the number of entries in this array
    ECOBJECTS_EXPORT uint32_t        GetCount() const;
    //! Returns whether this is a fixed size array or not
    ECOBJECTS_EXPORT bool            IsFixedCount() const;
    //! Returns whether this is a primitive array
    ECOBJECTS_EXPORT bool            IsPrimitiveArray() const;
    //! Returns whether this is a struct array
    ECOBJECTS_EXPORT bool            IsStructArray() const;
    //! Returns the kind of array this is (primitive or struct)
    ECOBJECTS_EXPORT ValueKind       GetKind() const;
    //! Returns the primitive type that this array was initialized with
    ECOBJECTS_EXPORT PrimitiveType   GetElementPrimitiveType() const;
    };

//=======================================================================================
//! Variant-like object used to set and retrieve property values in \ref ECN::IECInstance "ECInstances".
//! @remarks It does not represent a "live" reference into the underlying ECN::IECInstance
//! (or the object that the ECN::IECInstance represents). Changing the ECValue will not affect
//! the IECInstance unless you subsequently call IECInstance::SetValue with it.
//=======================================================================================
struct ECValue
    {
//__PUBLISH_SECTION_END__
public:
    //! Performs a shallow copy
    void ShallowCopy (ECValueCR v);
//__PUBLISH_SECTION_START__
private:
    union
        {
        ValueKind       m_valueKind;
        PrimitiveType   m_primitiveType;
        };

    uint8_t             m_stateFlags;
    mutable uint8_t     m_ownershipFlags;       // mutable because string ownership may change when we perform on-demand encoding conversions...

    void                InitForString (void const * str);
    BentleyStatus       SetBinaryInternal (const Byte * data, size_t size, bool holdADuplicate = false);
    bool                ConvertToPrimitiveFromString (PrimitiveType primitiveType);

//protected:
    typedef bvector<ECValue>  ValuesVector;
    typedef bvector<ECValue>* ValuesVectorP;
    
    //! Structure to hold information about a binary type
    struct BinaryInfo
        {
        const Byte *        m_data; //!< The actual binary data
        size_t              m_size; //!< The size of the data
        };

    //! Structure to hold information about String values
    struct StringInfo
        {
    private:
//__PUBLISH_SECTION_END__
        friend void ECValue::ShallowCopy (ECValueCR);
//__PUBLISH_SECTION_START__

        Utf8CP              m_utf8;
        Utf16CP             m_utf16;
#if !defined (_WIN32)
        WCharCP             m_wchar;        // On Windows we use m_utf16. The presence of the extra pointer wouldn't hurt anything but want to ensure it's only used on unix.
#endif
        void                ConvertToUtf8 (uint8_t& flags);
        void                ConvertToUtf16 (uint8_t& flags);
    public:
        bool                IsUtf8 () const;
        // All the business with the flags parameters is so that StringInfo can modify ECValue's ownership flags.
        // If we stored the flags on StringInfo, we would increase the size of the union.

        //! Returns the stored string information as a WChar
        //! @param[out]  flags   A flag indicating whether this ECValue owns the data
        //! @returns The data as a WChar const pointer
        WCharCP             GetWChar (uint8_t& flags);

        //! Returns the stored string information as Utf8
        //! @param[out]  flags   A flag indicating whether this ECValue owns the data
        //! @returns The data as a Utf8 const pointer
        Utf8CP              GetUtf8 (uint8_t& flags);

        //! Returns the stored string information as Utf16
        //! @param[out]  flags   A flag indicating whether this ECValue owns the data
        //! @returns The data as a Utf16 const pointer
        Utf16CP             GetUtf16 (uint8_t& flags);

        //! Sets the string data as WChar
        //!@param[in] str       The string data to store
        //!@param[out] flags    A flag indicating whether this ECValue owns the data
        //!@param[in] makeCopy  Indicates whether the passed in WCharCP str should be stored, or whether a copy should be made
        void                SetWChar (WCharCP str, uint8_t& flags, bool makeCopy);

        //! Sets the string data as Utf8
        //!@param[in] str       The string data to store
        //!@param[out] flags    A flag indicating whether this ECValue owns the data
        //!@param[in] makeCopy  Indicates whether the passed in WCharCP str should be stored, or whether a copy should be made
        void                SetUtf8 (Utf8CP str, uint8_t& flags, bool makeCopy);

        //! Sets the string data as Utf16
        //!@param[in] str       The string data to store
        //!@param[out] flags    A flag indicating whether this ECValue owns the data
        //!@param[in] makeCopy  Indicates whether the passed in WCharCP str should be stored, or whether a copy should be made
        void                SetUtf16 (Utf16CP str, uint8_t& flags, bool makeCopy);

        //! Frees (if necessary) the string data and clears out the memory
        //! @param[out] flags   Gets reset to 0, indicating the data is not owned
        void                FreeAndClear (uint8_t& flags);

        //! does not free pointers - used to init from Uninitialized ECValue state
        void                SetNull();

        //! Compares two StringInfo objects for equality
        //! @param[in] rhs  The StringInfo object to compare this object to
        //! @param[out] flags   A flag indicating whether the ECValue owns the data in the StringInfo object
        bool                Equals (StringInfo const& rhs, uint8_t& flags);
        };

    struct DateTimeInfo
        {
    private:
        ::int64_t           m_ceTicks;
        DateTime::Kind      m_kind;
        DateTime::Component m_component;
        bool                m_isMetadataSet;

    public:
        void Set (::int64_t ceTicks);
        BentleyStatus Set (DateTimeCR dateTime);
        ::int64_t GetCETicks () const;
        BentleyStatus GetDateTime (DateTimeR dateTime) const;

        bool IsMetadataSet () const {return m_isMetadataSet;}
        bool TryGetMetadata (DateTime::Info& metadata) const;
        BentleyStatus SetMetadata (DateTime::Info const& metadata);
        BentleyStatus SetMetadata (DateTimeInfoCR dateTimeInfo);

        bool MetadataMatches (DateTimeInfoCR dateTimeInfo) const;

        Utf8String MetadataToString () const;
        };

    //! The union storing the actual data of this ECValue
    union
        {
        bool                m_boolean;          //!< If a Boolean primitive type, holds the bool value
        ::int32_t           m_integer32;        //!< If an Int32 primitive type, holds the Int32 value
        ::int64_t           m_long64;           //!< If an Int64 primitive type, holds the Int64 value
        double              m_double;           //!< If a double primitive type, holds the double value
        //! If a String primitive type, holds the StringInfo struct defining the string
        mutable StringInfo  m_stringInfo;       // mutable so that we can convert to requested encoding on demand
        DateTimeInfo        m_dateTimeInfo;     //!< If a DateTime primitive, holds the DateTime value
        DPoint2d            m_dPoint2d;         //!< If a DPoint2d primitive, holds the DPoint2d value
        DPoint3d            m_dPoint3d;         //!< If a DPoint3d primitive, holds the DPoint3d value
        ArrayInfo           m_arrayInfo;        //!< If an array value, holds the ArrayInfo struct defining the array
        BinaryInfo          m_binaryInfo;       //!< If a binary value, holds the BinaryInfo struct defining the binary data
        IECInstanceP        m_structInstance;   //!< The ECValue class calls AddRef and Release for the member as needed
        };

    //! Constructs an uninitialized ECValue object
    void ConstructUninitialized();
    //! If appropriate for the value type, frees the memory used to store the value
    inline void FreeMemory ();
         
public:
    //! Clears memory, if necessary, and sets the value back to an uninitialized state
    ECOBJECTS_EXPORT void            Clear();
    //! Compares two ECValues for equality
    ECOBJECTS_EXPORT ECValueR        operator= (ECValueCR rhs);    
    //! Destructor
    ECOBJECTS_EXPORT ~ECValue();

    //! Initializes a new instance of the ECValue type.
    ECOBJECTS_EXPORT ECValue ();
    //! Initializes a new instance of the ECValue type from the given ECValue.
    //! @param[in] v ECValue to initialize this object from
    ECOBJECTS_EXPORT ECValue (ECValueCR v);

    //! Constructs an uninitialized ECValue of the specified ValueKind
    //! @param[in] classification   The type to set this new ECValue to
    ECOBJECTS_EXPORT explicit ECValue (ValueKind classification);

    //! Constructs an uninitialized ECValue of the specified PrimitiveType
    //! @param[in] primitiveType The type to set this new ECValue to
    ECOBJECTS_EXPORT explicit ECValue (PrimitiveType primitiveType);

    //! Initializes a new instance of ECValue from the given value. Type is set to BentleyApi::ECN::PRIMITIVETYPE_Integer
    //! @param[in] integer32 Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (::int32_t integer32);

    //! Initializes a new instance of ECValue from the given value. Type is set to BentleyApi::ECN::PRIMITIVETYPE_Long
    //! @param[in] long64 Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (::int64_t long64);

    //! Initializes a new instance of ECValue from the given value. Type is set to BentleyApi::ECN::PRIMITIVETYPE_Double
    //! @param[in] doubleVal Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (double doubleVal);

    //! Initializes a new instance of ECValue from the given value.  Type is set to BentleyApi::ECN::PRIMITIVETYPE_String
    //! @param[in] string           Value to initialize this ECValue from
    //! @param[in] holdADuplicate   true, if a copy of \p string should be held in the ECValue object.
    //!                             false, otherwise.
    ECOBJECTS_EXPORT explicit ECValue (WCharCP string, bool holdADuplicate = true);

    //! Initializes a new instance of ECValue from the given value.  Type is set to BentleyApi::ECN::PRIMITIVETYPE_String
    //! @param[in] string           Value to initialize this ECValue from
    //! @param[in] holdADuplicate   true, if a copy of \p string should be held in the ECValue object.
    //!                             false, otherwise.
    ECOBJECTS_EXPORT explicit ECValue (Utf8CP string, bool holdADuplicate = true);

    //! Initializes a new instance of ECValue from the given value.  Type is set to BentleyApi::ECN::PRIMITIVETYPE_String
    //! @param[in] string           Value to initialize this ECValue from
    //! @param[in] holdADuplicate   true, if a copy of \p string should be held in the ECValue object.
    //!                             false, otherwise.
    ECOBJECTS_EXPORT explicit ECValue (Utf16CP string, bool holdADuplicate = true);

    //! Initializes a new instance of ECValue from the given value.  Type is set to BentleyApi::ECN::PRIMITIVETYPE_Binary
    //! @note No copy of \p blob is created. Use ECValue::SetBinary otherwise.
    //! @see ECValue::SetBinary
    //! @param[in] blob Value to initialize this ECValue from
    //! @param[in] size Size in bytes of the blob
    ECOBJECTS_EXPORT explicit ECValue (const Byte * blob, size_t size);

    //! Initializes a new instance of ECValue from the given value.  Type is set to BentleyApi::ECN::PRIMITIVETYPE_Point2D
    //! @param[in] point2d Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (DPoint2dCR point2d);

    //! Initializes a new instance of ECValue from the given value.  Type is set to BentleyApi::ECN::PRIMITIVETYPE_Point3D
    //! @param[in] point3d Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (DPoint3dCR point3d);

    //! Initializes a new instance of ECValue from the given value.  Type is set to BentleyApi::ECN::PRIMITIVETYPE_Boolean
    //! @param[in] value Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (bool value);

    //! Initializes a new instance of the ECValue type.
    //! @param[in] dateTime Date time value to set.
    ECOBJECTS_EXPORT explicit ECValue (DateTimeCR dateTime);

    //! Sets whether this ECValue is read-only
    //! @param[in] isReadOnly Sets the read-only status of the ECValue
    ECOBJECTS_EXPORT void           SetIsReadOnly(bool isReadOnly);

    //! Gets whether this ECValue is read-only or not
    //! @returns true if the ECValue is read-only, false otherwise
    ECOBJECTS_EXPORT bool           IsReadOnly() const;

    //! Sets whether this ECValue is NULL.
    //! @param[in] isNull   Indicates whether the ECValue is null or not
    ECOBJECTS_EXPORT void           SetIsNull(bool isNull); 

    //! Gets whether this ECValue is NULL or not
    //! @returns true if the ECValue is NULL, false otherwise
    ECOBJECTS_EXPORT bool           IsNull() const;

    //! Sets whether this ECValue has had its value loaded
    //! @param[in] isLoaded Indicates whether the value has been loaded
    ECOBJECTS_EXPORT void           SetIsLoaded(bool isLoaded); 

    //! Gets whether this ECValue's value has been loaded
    //! @returns true if the value has been loaded, false otherwise
    ECOBJECTS_EXPORT bool           IsLoaded() const; 

    //! Frees the values memory, if necessary, and sets the state to NULL.
    ECOBJECTS_EXPORT void           SetToNull(); 

    //! Does a ShallowCopy of the supplied ECValue
    //! @param[in] v    The ECValue to copy from
    ECOBJECTS_EXPORT void           From(ECValueCR v);

    //! Returns the ValueKind of this value
    ECOBJECTS_EXPORT ValueKind      GetKind() const; 
    //! Checks whether this ECValue is uninitialized
    ECOBJECTS_EXPORT bool           IsUninitialized () const; 

    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_String (regardless of encoding).
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_String. false otherwise.
    ECOBJECTS_EXPORT bool           IsString () const;
    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_String and
    //! is encoded in UTF-8.
    //! @remarks Use this method to pick the appropriate Get method to avoid unnecessary
    //!          string conversions.
    //!          \code
    //!             
    //!          ECValue v = ...;
    //!          if (v.IsUtf8 ())
    //!             {
    //!             Utf8CP string = v.GetUtf8CP ();
    //!             ...
    //!             }
    //!          else
    //!             {
    //!             WCharCP string = v.GetString ();
    //!             ...
    //!             }
    //!
    //!            \endcode
    //!
    //! @return true if the ECValue content is encoded in UTF-8. false otherwise.
    ECOBJECTS_EXPORT bool           IsUtf8 () const;

    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_Integer.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_Integer. false otherwise.
    ECOBJECTS_EXPORT bool           IsInteger () const;
    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_Long.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_Long. false otherwise.
    ECOBJECTS_EXPORT bool           IsLong () const;
    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_Double.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_Double. false otherwise.
    ECOBJECTS_EXPORT bool           IsDouble () const;
    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_Binary.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_Binary. false otherwise.
    ECOBJECTS_EXPORT bool           IsBinary () const;
    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_Boolean.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_Boolean. false otherwise.
    ECOBJECTS_EXPORT bool           IsBoolean () const;

    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_Point2D.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_Point2D. false otherwise.
    ECOBJECTS_EXPORT bool           IsPoint2D () const;
    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_Point3D.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_Point3D. false otherwise.
    ECOBJECTS_EXPORT bool           IsPoint3D () const;
    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_DateTime.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_DateTime. false otherwise.
    ECOBJECTS_EXPORT bool           IsDateTime () const;
    //! Indicates whether the content of this ECValue is of type BentleyApi::ECN::PRIMITIVETYPE_IGeometry.
    //! @return true if the ECValue content is of type BentleyApi::ECN::PRIMITIVETYPE_IGeometry. false otherwise.
    ECOBJECTS_EXPORT bool           IsIGeometry() const;

    //! Indicates whether the content of this ECValue is an array (DgnPlatform::VALUEKIND_Array).
    //! @return true if the ECValue content is an array. false otherwise.
    ECOBJECTS_EXPORT bool           IsArray () const;
    //! Indicates whether the content of this ECValue is a struct (DgnPlatform::VALUEKIND_Struct).
    //! @return true if the ECValue content is a struct. false otherwise.
    ECOBJECTS_EXPORT bool           IsStruct () const;
    //! Indicates whether the content of this ECValue is of a primitive type (DgnPlatform::VALUEKIND_Primitive).
    //! @return true if the ECValue content is of a primitive type. false otherwise.
    ECOBJECTS_EXPORT bool           IsPrimitive () const;
    //! Gets the PrimitiveType of this ECValue        
    ECOBJECTS_EXPORT PrimitiveType  GetPrimitiveType() const; 

    //! Sets the PrimitiveType of this ECValue
    //! @param[in] primitiveElementType The type of primitive that this ECValue holds.
    ECOBJECTS_EXPORT BentleyStatus  SetPrimitiveType(PrimitiveType primitiveElementType);

    ECOBJECTS_EXPORT bool           CanConvertToPrimitiveType (PrimitiveType type) const;
/*__PUBLISH_SECTION_END__*/
    // Attempts to convert this ECValue's primitive value to a different primitive type.
    // Currently supported conversions (motivated by ECExpressions):
    //  - double, int, and string are all interconvertible. double => int rounds
    //  - conversion to and from string is supported for any other primitive type
    //  - a null value of any primitive type can be converted a null value of any other primitive type
    ECOBJECTS_EXPORT bool           ConvertToPrimitiveType (PrimitiveType primitiveType);
    // Attempts to convert a primitive ECValue to a string representation suitable for serialization.
    // Fails if this ECValue is not a primitive
    // Returns an empty string if this ECValue is null
    ECOBJECTS_EXPORT bool           ConvertPrimitiveToString (Utf8StringR str) const;

    // Attempts to convert this ECValue's primitive value to a literal ECExpression
    // Does not support binary or IGeometry
    ECOBJECTS_EXPORT bool           ConvertPrimitiveToECExpressionLiteral (Utf8StringR expression) const;

    // Attempts to format the underlying value using the specified .NET-style format string.
    // Typically the format string originated from an ECCustomAttribute.
    // Currently only supports numeric types: double, int, and long
    // Use DgnPlatform::IECInteropStringFormatter() for more full-featured formatting.
    ECOBJECTS_EXPORT bool           ApplyDotNetFormatting (Utf8StringR formatted, Utf8CP formatString) const;
    ECOBJECTS_EXPORT bool           SupportsDotNetFormatting() const;

    // Get/set a flag indicating whether to copy data from an ECDBuffer into this ECValue or to allow this ECValue to store direct pointers into the ECDBuffer's data.
    // Off by default, indicating data will be copied from the ECDBuffer.
    // By setting this flag the caller indicates the pointers exposed by this ECValue will not be used after the contents of the ECDBuffer have been
    // released, modified, or moved.
    // Don't set this flag in ECValues to be returned to external callers who may not expect it to be set.
    ECOBJECTS_EXPORT bool           AllowsPointersIntoInstanceMemory() const;
    ECOBJECTS_EXPORT void           SetAllowsPointersIntoInstanceMemory (bool allow);

/*__PUBLISH_SECTION_START__*/

    //! Defines the StructArray for this ECValue
    //! @param[in] count        The initial size of the array
    //! @param[in] isFixedSize  Indicates whether this array can grow or not
    ECOBJECTS_EXPORT ECObjectsStatus  SetStructArrayInfo (uint32_t count, bool isFixedSize);

    //! Defines the primitive array for this ECValue
    //! @param[in] primitiveElementtype The type of primitive the array will hold
    //! @param[in] count                The initial size of the array
    //! @param[in] isFixedSize          Indicates whether this array can grow or not
    ECOBJECTS_EXPORT ECObjectsStatus  SetPrimitiveArrayInfo (PrimitiveType primitiveElementtype, uint32_t count, bool isFixedSize);
    
    //! Returns the array information defining this ECValue
    ECOBJECTS_EXPORT ArrayInfo      GetArrayInfo() const;
    
    //! Returns the integer value, if this ECValue holds an Integer 
    ECOBJECTS_EXPORT int32_t        GetInteger () const;  
    //! Sets the value of this ECValue to the given integer
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to an Integer Primitive
    //! @param[in] integer  The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetInteger (int32_t integer); 
    
    //! Returns the long value, if this ECValue holds a long
    ECOBJECTS_EXPORT int64_t        GetLong () const;
    //! Sets the value of this ECValue to the given long
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a Long Primitive
    //! @param[in] long64  The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetLong (int64_t long64);
 
    //! Returns the boolean value, if this ECValue holds a boolean
    ECOBJECTS_EXPORT bool           GetBoolean () const;
    //! Sets the value of this ECValue to the given bool
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a bool Primitive
    //! @param[in] value  The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetBoolean (bool value);

    //! @returns    The double held by the ECValue, or std::numeric_limits<double>::quiet_NaN() if it is not a double or IsNull
    ECOBJECTS_EXPORT double         GetDouble () const;
    //! Sets the value of this ECValue to the given double
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a double Primitive
    //! @param[in] value  The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetDouble (double value);

    //! Gets the string content of this ECValue.
    //! @note If the encoding of the string in the ECValue differs from the encoding of what is to be returned, the string
    //!       is automatically converted. To avoid string conversions call ECValue::IsUtf8 first.
    //! @return string content
    ECOBJECTS_EXPORT WCharCP        GetWCharCP () const;
    //! Gets the string content of this ECValue in UTF-8 encoding.
    //! @note If the encoding of the string in the ECValue differs from the encoding of what is to be returned, the string
    //!       is automatically converted. To avoid string conversions call ECValue::IsUtf8 first.
    //! @return string content in UTF-8 encoding
    ECOBJECTS_EXPORT Utf8CP         GetUtf8CP () const;
    //! Returns the string value as a Utf16CP, if this ECValue holds a string
    ECOBJECTS_EXPORT Utf16CP        GetUtf16CP () const;    // the only real caller of this should be ECDBuffer

    //__PUBLISH_SECTION_END__

    //! Indicates whether the ECValue instance owns the memory of WCharCP returned by ECValue::GetString.
    //! @remarks Can get return true, even if ECValue::SetWCharCP was called with holdADuplicate = false, as 
    //! ECValue does implicit encoding conversions when the requested encoding is different than the one used at set time.
    //! In those cases the ECValue always owns the converted string.
    //! @return bool if ECValue owns the WChar string value, false otherwise
    ECOBJECTS_EXPORT bool           OwnsWCharCP () const;

    //! Indicates whether the ECValue instance owns the memory of Utf8CP returned by ECValue::GetUtf8CP.
    //! @remarks Can get return true, even if ECValue::SetUtf8CP was called with holdADuplicate = false, as 
    //! ECValue does implicit encoding conversions when the requested encoding is different than the one used at set time.
    //! In those cases the ECValue always owns the converted string.
    //! @return bool if ECValue owns the Utf8CP value, false otherwise
    ECOBJECTS_EXPORT bool           OwnsUtf8CP () const;

    //! Indicates whether the ECValue instance owns the memory of Utf16CP returned by ECValue::GetUtf16CP.
    //! @remarks Can get return true, even if ECValue::SetUtf16CP was called with holdADuplicate = false, as 
    //! ECValue does implicit encoding conversions when the requested encoding is different than the one used at set time.
    //! In those cases the ECValue always owns the converted string.
    //! @return bool if ECValue owns the Utf16CP value, false otherwise
    ECOBJECTS_EXPORT bool           OwnsUtf16CP () const;
    
    //__PUBLISH_SECTION_START__

    //! Sets the value of this ECValue to the given string
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a string Primitive
    //! @param[in] string           The value to set
    //! @param[in] holdADuplicate   Flag specifying whether the ECValue should make its own copy of the string, or store the actual pointer passed in
    ECOBJECTS_EXPORT BentleyStatus  SetWCharCP (WCharCP string, bool holdADuplicate = true);
    //! Sets the value of this ECValue to the given string
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a string Primitive
    //! @param[in] string           The value to set
    //! @param[in] holdADuplicate   Flag specifying whether the ECValue should make its own copy of the string, or store the actual pointer passed in
    ECOBJECTS_EXPORT BentleyStatus  SetUtf8CP (Utf8CP string, bool holdADuplicate = true);
    //! Sets the value of this ECValue to the given string
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a string Primitive
    //! @param[in] string           The value to set
    //! @param[in] holdADuplicate   Flag specifying whether the ECValue should make its own copy of the string, or store the actual pointer passed in
    ECOBJECTS_EXPORT BentleyStatus  SetUtf16CP (Utf16CP string, bool holdADuplicate = true);    // primarily for use by ECDBuffer

    //! Returns the binary value, if this ECValue holds binary data
    //! @param[in]  size    The size of the binary data
    ECOBJECTS_EXPORT const Byte *   GetBinary (size_t& size) const;

    //! Sets the value of this ECValue to the given byte array
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a binary Primitive
    //! @param[in] data             The value to set
    //! @param[in] size             The size of the data
    //! @param[in] holdADuplicate   Flag specifying whether the ECValue should make its own copy of the string, or store the actual pointer passed in
    ECOBJECTS_EXPORT BentleyStatus  SetBinary (const Byte * data, size_t size, bool holdADuplicate = false);

    //! Returns the IGeometry, if this ECValue holds a geometry value
    ECOBJECTS_EXPORT IGeometryPtr   GetIGeometry () const;

    /*__PUBLISH_SECTION_END__*/
    //! Returns the binary value of the IGeometry
    //! @param[in]  size    The size of the binary data
    ECOBJECTS_EXPORT const Byte *   GetIGeometry (size_t& size) const;
    //! Sets the value of this ECValue to the given IGeometry, using a serialized byte array
    ECOBJECTS_EXPORT BentleyStatus  SetIGeometry (const Byte * data, size_t size, bool holdADuplicate = false);
    /*__PUBLISH_SECTION_START__*/

    ECOBJECTS_EXPORT BentleyStatus  SetIGeometry (IGeometryCR geometry);

    //! Gets the struct instance of this ECValue, if the ECValue holds a struct
    ECOBJECTS_EXPORT IECInstancePtr GetStruct() const;
    //! Sets the specified struct instance in the ECValue. 
    //! @note ECValue doesn't create a copy of \p structInstance. Its ref-count is incremented by this method though.
    //! @param[in] structInstance struct instance to set in the ECValue
    //! @return SUCCESS or ERROR
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
    //! @note Ignores the date time metadata. Use ECValue::GetDateTime if you need the metadata.
    //! @return DateTime value as ticks since the beginning of the Common Era epoch.
    ECOBJECTS_EXPORT int64_t        GetDateTimeTicks () const;

    //! Returns the DateTime value as milliseconds since the beginning of the Unix epoch.
    //! The Unix epoch begins at 1970-01-01 00:00:00 UTC.
    //! DateTimes before the Unix epoch are negative.
    //! @note Ignores the date time metadata. Use ECValue::GetDateTime if you need the metadata.
    //! @return DateTime as milliseconds since the beginning of the Unix epoch.
    ECOBJECTS_EXPORT int64_t        GetDateTimeUnixMillis() const;

    //! Gets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @param[out] hasMetadata true, if this ECValue objects contains date time metadata. false otherwise
    //! @param[out] metadata if \p hasMetadata is true, contains the metadata available in this ECValue.
    //! @return DateTime value as ticks since the beginning of the Common Era epoch.
    ECOBJECTS_EXPORT int64_t        GetDateTimeTicks (bool& hasMetadata, DateTime::Info& metadata) const;

    //! Sets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @note If the ECProperty to which this ECValue will be applied contains the %DateTimeInfo custom attribute,
    //! the ticks will be enriched with the metadata from the custom attribute.
    //! @param[in] ceTicks DateTime Common Era ticks to set
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT BentleyStatus  SetDateTimeTicks (int64_t ceTicks);

    //! Sets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @param[in] ceTicks DateTime Common Era ticks to set
    //! @param[in] dateTimeMetadata DateTime metadata to set along with the ticks.
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT BentleyStatus  SetDateTimeTicks (int64_t ceTicks, DateTime::Info const& dateTimeMetadata);

//__PUBLISH_SECTION_END__
    BentleyStatus                   SetDateTimeMetadata (DateTimeInfoCR caDateTimeMetadata);
    bool                            IsDateTimeMetadataSet () const;
    bool                            DateTimeInfoMatches (DateTimeInfoCR caDateTimeMetadata) const;
    Utf8String                      DateTimeMetadataToString () const;
    ECOBJECTS_EXPORT BentleyStatus  SetLocalDateTimeFromUnixMillis (int64_t unixMillis);
//__PUBLISH_SECTION_START__
//
    //! Returns the DPoint2d value, if this ECValue holds a Point2d
    ECOBJECTS_EXPORT DPoint2d       GetPoint2D() const;
    //! Sets the value of this ECValue to the given DPoint2d
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a DPoint2d primitive
    //! @param[in] value   The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetPoint2D (DPoint2dCR value);

    //! Returns the DPoint3d value, if this ECValue holds a Point3d
    ECOBJECTS_EXPORT DPoint3d       GetPoint3D() const;
    //! Sets the value of this ECValue to the given DPoint3d
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a DPoint3d primitive
    //! @param[in] value   The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetPoint3D (DPoint3dCR value);

    //! For fixed primitive types, returns the number of bytes required to represent the type
    //! @param[in]  primitiveType   The type to measure
    //! @returns The sizeof the given type, if it is a fixed size primitive 
    static ECOBJECTS_EXPORT uint32_t GetFixedPrimitiveValueSize (PrimitiveType primitiveType);

    //! This is intended for debugging purposes, not for presentation purposes.
    ECOBJECTS_EXPORT Utf8String       ToString () const;
    
    //! Checks 2 ECValues for equality.
    ECOBJECTS_EXPORT bool           Equals (ECValueCR v) const;
    };

//__PUBLISH_SECTION_END__

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct AdhocPropertyMetadata
    {
protected:
    enum class Index
        {
        Name        = 0,
        DisplayLabel,
        Value,
        Type,
        Unit,
        ExtendType,
        IsReadOnly,
        IsHidden,
        MAX
        };
private:
    friend struct AdhocPropertyQuery;
    friend struct AdhocPropertyEdit;

    Utf8String              m_metadataPropertyNames[(size_t)Index::MAX];    // the property names within the struct class holding the ad-hoc values and metadata
    uint32_t                m_containerIndex;                       // the property index of the struct array holding ad-hoc properties within the host's ECClass

    AdhocPropertyMetadata (ECEnablerCR enabler, Utf8CP containerAccessString, bool loadMetadata);
    AdhocPropertyMetadata (ECEnablerCR enabler, uint32_t containerPropertyIndex, bool loadMetadata);

    bool                    Init (ECEnablerCR enabler, uint32_t containerPropertyIndex, bool loadMetadata);
protected:
    Utf8CP                  GetPropertyName (Index index) const;

    static bool             IsRequiredMetadata (Index index);
    static bool             PrimitiveTypeForCode (PrimitiveType& primType, int32_t code);
    static bool             CodeForPrimitiveType (int32_t& code, PrimitiveType primType);
public:
    ECOBJECTS_EXPORT AdhocPropertyMetadata (ECEnablerCR enabler, Utf8CP containerAccessString);
    ECOBJECTS_EXPORT AdhocPropertyMetadata (ECEnablerCR enabler, uint32_t containerPropertyIndex);

    ECOBJECTS_EXPORT bool   IsSupported() const;
    uint32_t                GetContainerPropertyIndex() const { return m_containerIndex; }

    ECOBJECTS_EXPORT static bool    IsSupported (ECEnablerCR enabler, Utf8CP containerAccessString);
    ECOBJECTS_EXPORT static bool    IsSupported (ECEnablerCR enabler, uint32_t containerPropertyIndex);
    };

typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;
/*---------------------------------------------------------------------------------**//**
* Provides read-only access to ad-hoc properties defined on an IECInstance.
* Ad-hoc properties are name-value pairs stored in a struct array property on an IECInstance.
* In order for an IECInstance to support ad-hoc properties, its ECClass must define an
* AdhocPropertySpecification custom attribute containing the name of a struct array property
* with an attached AdhocPropertyContainerDefinition custom attribute specifying, at minimum,
* the names of properties to hold the name and value of each ad-hoc property. Additional
* property names can specify metadata like primitive type, dispaly label, extended type,
* EC unit name, and read-only state.
* @bsistruct                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct AdhocPropertyQuery : AdhocPropertyMetadata
    {
private:
    IECInstanceCR           m_host;
public:
    ECOBJECTS_EXPORT AdhocPropertyQuery (IECInstanceCR host, Utf8CP containerAccessString);
    ECOBJECTS_EXPORT AdhocPropertyQuery (IECInstanceCR host, uint32_t containerPropertyIndex);

    IECInstanceCR                       GetHost() const { return m_host; }

    ECOBJECTS_EXPORT bool               GetPropertyIndex (uint32_t& index, Utf8CP accessString) const;
    ECOBJECTS_EXPORT uint32_t           GetCount() const;

    ECOBJECTS_EXPORT ECObjectsStatus    GetName (Utf8StringR name, uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetDisplayLabel (Utf8StringR label, uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetPrimitiveType (PrimitiveType& type, uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetExtendedTypeName (Utf8StringR typeName, uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    GetUnitName (Utf8StringR unitName, uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    IsReadOnly (bool& isReadOnly, uint32_t index) const;
    ECOBJECTS_EXPORT ECObjectsStatus    IsHidden (bool& isHidden, uint32_t index) const;

    // For getting additional ad-hoc metadata
    ECOBJECTS_EXPORT ECObjectsStatus    GetValue (ECValueR v, uint32_t index, Utf8CP accessor) const;

    IECInstancePtr          GetEntry (uint32_t index) const;
    ECObjectsStatus         GetString (Utf8StringR str, uint32_t index, Index which) const;
    ECObjectsStatus         GetString (Utf8StringR str, IECInstanceCR instance, Index which) const;
    ECObjectsStatus         GetValue (ECValueR v, uint32_t index, Index which) const;
    ECObjectsStatus         GetValue (ECValueR v, IECInstanceCR instance, Index which) const;
    ECClassCP               GetStructClass() const;
    StandaloneECEnablerPtr  GetStructEnabler() const;
    };

/*---------------------------------------------------------------------------------**//**
* Provides read-write access to ad-hoc properties defined on an IECInstance.
* @bsistruct                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct AdhocPropertyEdit : AdhocPropertyQuery
    {
public:
    ECOBJECTS_EXPORT AdhocPropertyEdit (IECInstanceR host, Utf8CP containerAccessString);
    ECOBJECTS_EXPORT AdhocPropertyEdit (IECInstanceR host, uint32_t containerPropertyIndex);

    IECInstanceR            GetHostR()  { return const_cast<IECInstanceR>(GetHost()); }

    ECOBJECTS_EXPORT ECObjectsStatus    SetName (uint32_t index, Utf8CP name);
    ECOBJECTS_EXPORT ECObjectsStatus    SetDisplayLabel (uint32_t index, Utf8CP displayLabel, bool andSetName = false);
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (uint32_t index, ECValueCR v);
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsReadOnly (uint32_t index, bool isReadOnly);
    ECOBJECTS_EXPORT ECObjectsStatus    SetIsHidden (uint32_t index, bool isHidden);

    ECOBJECTS_EXPORT ECObjectsStatus    Add (Utf8CP name, ECValueCR v, Utf8CP displayLabel = nullptr, Utf8CP unitName = nullptr, Utf8CP extendedTypeName = nullptr, bool isReadOnly = false, bool hidden = false);
    ECOBJECTS_EXPORT ECObjectsStatus    Remove (uint32_t index);
    ECOBJECTS_EXPORT ECObjectsStatus    Clear();
    ECOBJECTS_EXPORT ECObjectsStatus    CopyFrom (AdhocPropertyQueryCR src, bool preserveValues);

    // For setting additional ad-hoc metadata
    ECOBJECTS_EXPORT ECObjectsStatus    SetValue (uint32_t index, Utf8CP accessor, ECValueCR v);

    ECOBJECTS_EXPORT ECObjectsStatus    Swap (uint32_t propIdxA, uint32_t propIdxB);
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct AdhocContainerPropertyIndexCollection
    {
private:
    ECEnablerCR     m_enabler;
public:
    AdhocContainerPropertyIndexCollection (ECEnablerCR enabler) : m_enabler (enabler) { }

    struct const_iterator : std::iterator<std::forward_iterator_tag, uint32_t const>
        {
    private:
        ECEnablerCR         m_enabler;
        uint32_t            m_current;

        bool                IsEnd() const { return 0 == m_current; }
        bool                ValidateCurrent() const;
        void                MoveNext();
    public:
        const_iterator (ECEnablerCR enabler, bool isEnd);

        uint32_t const&   operator*() const   { return m_current; }
        bool            operator==(const_iterator const& rhs) const { return &rhs.m_enabler == &m_enabler && rhs.m_current == m_current; }
        bool            operator!=(const_iterator const& rhs) const { return !(*this == rhs); }
        const_iterator& operator++()                                { MoveNext(); return *this; }
        };

    typedef const_iterator iterator;

    const_iterator      begin() const   { return const_iterator (m_enabler, false); }
    const_iterator      end() const     { return const_iterator (m_enabler, true); }
    };

//__PUBLISH_SECTION_START__

//=======================================================================================
//! A structure used for describing the complete location of an ECValue within an ECInstance.
//! They can be thought of as the equivalent to access strings, but generally do not require
//! any string manipulation to create or use them.
//! ECValueAccessors consist of a stack of locations, each of which consist of a triplet of
//! an ECEnabler, property index, and array index.  In cases where the array index is not
//! applicable (primitive members or the roots of arrays), the INDEX_ROOT constant
//! is used.
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
        mutable ECPropertyCP    m_cachedProperty;
    public:
        //! Constructs a Location pointing to a specific property value
        //! @param[in]      enabler     The enabler containing the property
        //! @param[in]      propIdx     The index of the property within the enabler
        //! @param[in]      arrayIdx    The array index of the property value, or INDEX_ROOT.
        Location (ECEnablerCP enabler, int propIdx, int arrayIdx) : m_enabler(enabler), m_propertyIndex(propIdx), m_arrayIndex(arrayIdx), m_cachedProperty(NULL) { }

        //! Constructs an empty Location.
        Location () : m_enabler(NULL), m_propertyIndex(-1), m_arrayIndex(INDEX_ROOT), m_cachedProperty(NULL) { }

        //! Copy-constructs a Location
        //! @param[in]      loc         The Location to copy
        //! @return 
        Location (const Location& loc) : m_enabler(loc.m_enabler), m_propertyIndex(loc.m_propertyIndex), m_arrayIndex(loc.m_arrayIndex), m_cachedProperty(loc.m_cachedProperty) { }
/*__PUBLISH_SECTION_END__*/
        void            SetPropertyIndex (int index)                { m_cachedProperty = NULL; m_propertyIndex = index; }
        void            SetArrayIndex (int index)                   { m_arrayIndex = index; }
        void            IncrementArrayIndex()                       { m_arrayIndex++; }
        Utf8CP         GetAccessString() const;
/*__PUBLISH_SECTION_START__*/
    public:
        //! Get the enabler associated with this Location
        //! @return     The enabler associated with this Location
        ECOBJECTS_EXPORT ECEnablerCP                     GetEnabler() const          { return m_enabler; }
        //! Get the property index identifying this Location
        //! @return     The index of the property within its enabler
        ECOBJECTS_EXPORT int                             GetPropertyIndex() const    { return m_propertyIndex; }

        //! Gets the array index of the property value associated with this Location
        //! @return     The array index, or INDEX_ROOT if no array index specified.
        ECOBJECTS_EXPORT int                             GetArrayIndex() const       { return m_arrayIndex; }

        //! Gets the ECProperty associated with this Location
        //! @return     The ECProperty, or nullptr if the ECProperty could not be evaluated.
        ECOBJECTS_EXPORT ECPropertyCP   GetECProperty() const;
        };

    typedef bvector<Location> LocationVector;

/*__PUBLISH_SECTION_END__*/
public:
    LocationVector const &   GetLocationVectorCR() const;

/*__PUBLISH_SECTION_START__*/
private:
    //"BACK" OF VECTOR IS DEEPEST ELEMENT
    LocationVector          m_locationVector;
    bool                    m_isAdhoc;
/*__PUBLISH_SECTION_END__*/
    const LocationVector&   GetLocationVector() const;

    Utf8CP                  GetAccessString (uint32_t depth, bool alwaysIncludeParentStructAccessStrings) const;
public:
    bool                                IsAdhocProperty() const { return m_isAdhoc; }

    ECOBJECTS_EXPORT Location&          operator[] (uint32_t depth);

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

    ECOBJECTS_EXPORT const Location&        operator[] (uint32_t depth) const;
    ECOBJECTS_EXPORT ECEnablerCR            GetEnabler (uint32_t depth) const;

    //! Determines whether or not the ECEnabler matches that of the accessor at the given depth.
    //! @param[in]      depth           The stack depth of the Accessor's ECEnablerPtr.
    //! @param[in]      other           The ECEnablerPtr to compare to.
    //! @return         true if the ECEnablerPtr are equivalent, otherwise false.
    ECOBJECTS_EXPORT bool                   MatchesEnabler (uint32_t depth, ECEnablerCR other) const;

    //! Looks up and returns the ECProperty associated with this accessor
    ECOBJECTS_EXPORT ECN::ECPropertyCP      GetECProperty() const;

    ECOBJECTS_EXPORT Utf8String             GetDebugAccessString () const;
/*__PUBLISH_SECTION_START__*/
public:
    //! Gets the depth of this Location within the ECValueAccessor.
    //! @return     The depth of this Location within the containing ECValueAccessor.
    ECOBJECTS_EXPORT uint32_t               GetDepth() const;

    //! Gets the native-style access string for a given stack depth.  This access string does
    //! not contain an array index, and is compatible with the Get/Set methods in IECInstance.
    //! @param[in]      depth           The stack depth of the native access string.
    //! @return         The access string.
    //! @see            IECInstance
    ECOBJECTS_EXPORT Utf8CP                 GetAccessString (uint32_t depth) const;

    //! Gets the native-style access string for the deepest Location in this ECValueAccessor. This access string does
    //! not contain an array index, and is compatible with the Get/Set methods in IECInstance.
    //! @return     The access string
    ECOBJECTS_EXPORT Utf8CP                 GetAccessString () const;

    //! Appends a Location pointing to the specified property value
    //! @param[in]      enabler         The enabler containing the property
    //! @param[in]      propertyIndex   The index of the property within its enabler
    //! @param[in]      arrayIndex      The array index of the property value, or INDEX_ROOT if no array index
    ECOBJECTS_EXPORT void  PushLocation (ECEnablerCR enabler, int propertyIndex, int arrayIndex=INDEX_ROOT);

    //! Appends a Location pointing to the specified property value
    //! @param[in]      enabler         The enabler containing the property
    //! @param[in]      accessString    The access string of the property within its enabler
    //! @param[in]      arrayIndex      The array index of the property value, or INDEX_ROOT if no array index
    //! @return     true if the access string is valid for the enabler, otherwise false
    ECOBJECTS_EXPORT bool  PushLocation (ECEnablerCR enabler, Utf8CP accessString,   int arrayIndex=INDEX_ROOT);

    //! Appends a Location pointing to the specified property value
    //! @param[in]      instance        The IECInstance containing the property value
    //! @param[in]      propertyIndex   The index of the property within the IECInstance's ECEnabler
    //! @param[in]      arrayIndex      The array index of the property value, or INDEX_ROOT if no array index
    ECOBJECTS_EXPORT void  PushLocation (IECInstanceCR instance, int propertyIndex, int arrayIndex=INDEX_ROOT);

    //! Appends a Location pointing to the specified property value
    //! @param[in]      instance     The IECInstance containing the property value
    //! @param[in]      accessString The access string identifying the property value within the IECInstance's ECEnabler
    //! @param[in]      arrayIndex   The array index of the property value, or INDEX_ROOT if no array index
    //! @return     true if the access string is valid for the IECInstance, otherwise false
    ECOBJECTS_EXPORT bool  PushLocation (IECInstanceCR instance, Utf8CP accessString,   int arrayIndex=INDEX_ROOT);

    //! Removes the deepest Location within this ECValueAccessor
    ECOBJECTS_EXPORT void       PopLocation ();

    //! Gets the deepest Location within this ECValueAccessor
    //! @return     The deepest Location within this ECValueAccessor
    ECOBJECTS_EXPORT Location&  DeepestLocation ();

    //! Gets the deepest Location within this ECValueAccessor
    //! @return     The deepest Location within this ECValueAccessor
    ECOBJECTS_EXPORT Location const&  DeepestLocationCR () const;

    //! Clears all Locations from this ECValueAccessor
    ECOBJECTS_EXPORT void  Clear ();

    //! Gets the name of the ECProperty identified by this ECValueAccessor
    //! @return     the name of the ECProperty identified by this ECValueAccessor
    ECOBJECTS_EXPORT Utf8String             GetPropertyName () const;

    //! Constructs an empty ECValueAccessor.
    ECOBJECTS_EXPORT ECValueAccessor () : m_isAdhoc (false) { }

    //! Constructs a copy of a ECValueAccessor.
    //! @param[in]      accessor         The accessor to be copied.
    ECOBJECTS_EXPORT ECValueAccessor (ECValueAccessorCR accessor);


    //! Gets the managed-style access string for this Accessor.  Includes the array indices,
    //! and traverses structs when necessary.  This full access string can be used with
    //! managed code or the InteropHelper.
    //! @see            ECInstanceInteropHelper
    //! @private
    ECOBJECTS_EXPORT Utf8String             GetManagedAccessString () const;

    //! Performs inequality comparison against the specified ECValueAccessor
    //! @param[in]      accessor The ECValueAccessor to compare against
    //! @return     true if the ECValueAccessors differ
    ECOBJECTS_EXPORT bool                   operator!=(ECValueAccessorCR accessor) const;

    //! Performs equality comparison against the specified ECValueAccessor
    //! @param[in]      accessor The ECValueAccessor to compare against
    //! @return     true if the ECValueAccessors are equivalent
    ECOBJECTS_EXPORT bool                   operator==(ECValueAccessorCR accessor) const;

    //! Populates an ECValueAccessor from an IECInstance and a managed property access string.
    //! @param[in]      va                      The ECValueAccessor to populate
    //! @param[in]      instance                The IECInstance containing the specified property value
    //! @param[in]      managedPropertyAccessor The managed access string relative to the specified IECInstance
    //! @return     ECOBJECTS_STATUS_Success if the ECValueAccessor was successfully populated, otherwise an error status
    //! @private
    ECOBJECTS_EXPORT static ECObjectsStatus PopulateValueAccessor (ECValueAccessor& va, IECInstanceCR instance, Utf8CP managedPropertyAccessor);

    //! Populates an ECValueAccessor from an ECEnabler and a managed property access string.
    //! @param[in]      va                      The ECValueAccessor to populate
    //! @param[in]      enabler                 The ECEnabler containing the specified property
    //! @param[in]      managedPropertyAccessor The managed access string relative to the specified ECEnabler
    //! @return     ECOBJECTS_STATUS_Success if the ECValueAccessor was successfully populated, otherwise an error status
    //! @private
    ECOBJECTS_EXPORT static ECObjectsStatus PopulateValueAccessor (ECValueAccessor& va, ECEnablerCR enabler, Utf8CP managedPropertyAccessor);
//__PUBLISH_SECTION_END__
    ECOBJECTS_EXPORT static ECObjectsStatus PopulateValueAccessor (ECValueAccessor& va, IECInstanceCR instance, Utf8CP managedPropertyAccessor, bool includeAdhocs);
    // We have modified an ECClass and have an ECValueAccessor defined in terms of the old ECClass. Remap it to refer to the new ECClass.
    ECOBJECTS_EXPORT static ECObjectsStatus RemapValueAccessor (ECValueAccessor& newVa, ECEnablerCR newEnabler, ECValueAccessorCR oldVa, IECSchemaRemapperCR remapper);
    // We have modified an ECClass and have an access string defined in terms of the old ECClass. Populate it according to the new ECClass, remapping property names as appropriate.
    ECOBJECTS_EXPORT static ECObjectsStatus PopulateAndRemapValueAccessor (ECValueAccessor& va, ECEnablerCR enabler, Utf8CP accessString, IECSchemaRemapperCR remapper);
//__PUBLISH_SECTION_START__
    };

/*__PUBLISH_SECTION_END__*/
struct ECValuesCollection;
struct ECValuesCollectionIterator;

/*__PUBLISH_SECTION_START__*/

//=======================================================================================
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

    ECOBJECTS_EXPORT ECObjectsStatus    Initialize (IECInstanceCR, Utf8CP accessString, ECValueCR);

    ECValueAccessorR    GetValueAccessorR ();

/*__PUBLISH_SECTION_START__*/
public:
    //! Gets the root IECInstance containing this ECPropertyValue
    //! @return     the IECInstance containing this ECPropertyValue
    ECOBJECTS_EXPORT IECInstanceCR          GetInstance () const;

    //! Gets the value of this ECPropertyValue
    //! @return     an ECValue containing the value of this ECPropertyValue
    ECOBJECTS_EXPORT ECValueCR              GetValue () const;

    //! Gets the ECValueAccessor identifying this property value
    //! @return     the ECValueAccessor identifying this property value
    ECOBJECTS_EXPORT ECValueAccessorCR      GetValueAccessor () const;

    //! Indicates whether the value is an array or struct
    ECOBJECTS_EXPORT bool                   HasChildValues () const;

    //! For array and struct values, gets a virtual collection of the embedded values
    ECOBJECTS_EXPORT ECValuesCollectionPtr  GetChildValues () const;

    //! Create an ECPropertyValue for the property of an IECInstance identified by a managed access string
    //! @param[in]      instance         The IECInstance containing the property value
    //! @param[in]      propertyAccessor The managed access string identifying the property value within the IECInstance
    //! @return     an ECPropertyValue pointing to the specified property value of the IECInstance, or nullptr if the property value could not be evaluated.
    ECOBJECTS_EXPORT static ECPropertyValuePtr     GetPropertyValue (IECInstanceCR instance, Utf8CP propertyAccessor);
    };

//=======================================================================================
//! An iterator over the ECPropertyValues contained in an ECValuesCollection
//! @see ECValue, ECValueAccessor, ECValuesCollection
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
    //! Performs inequality comparison against another ECValuesCollectionIterator
    //! @param[in]      iter The ECValuesCollectionIterator against which to compare
    //! @return     true if the iterators differ, false if they are equivalent
    ECOBJECTS_EXPORT bool               IsDifferent(ECValuesCollectionIterator const& iter) const;

    //! Advances this iterator to the next item in the collection
    ECOBJECTS_EXPORT void               MoveToNext ();

    //! Gets the ECPropertyValue currently pointed to by this iterator
    //! @return     the ECPropertyValue currently pointed to by this iterator
    ECOBJECTS_EXPORT ECPropertyValue const& GetCurrent () const;
    };

//=======================================================================================
//! A collection of all ECPropertyValues contained in an IECInstance or an ECPropertyValue.
//! @bsiclass
//=======================================================================================
struct ECValuesCollection : RefCountedBase
    {
public:
    typedef VirtualCollectionIterator<ECValuesCollectionIterator> const_iterator;
    friend struct ECPropertyValue;

private:
    ECPropertyValue                                     m_parentPropertyValue;
    mutable RefCountedPtr<ECValuesCollectionIterator>   m_end;

    ECValuesCollection ();
    ECValuesCollection (ECPropertyValueCR parentPropValue);
public:
    ECOBJECTS_EXPORT ECValuesCollection (IECInstanceCR);

    //! Gets an iterator pointing to the beginning of the collection
    //! @return an iterator pointing to the beginning of this ECValuesCollection
    ECOBJECTS_EXPORT const_iterator begin () const;

    //! Gets an iterator pointing just beyond the end of this collection
    //! @return an iterator pointing just beyond the end of this ECValuesCollection
    ECOBJECTS_EXPORT const_iterator end ()   const;

    //! Creates an ECValuesCollection representing all of the ECPropertyValues contained in the specified IECInstance
    //! @param[in]      instance The IECInstance for which to create the collection
    //! @return     A collection of ECPropertyValues contained in the specified IECInstance
    ECOBJECTS_EXPORT static ECValuesCollectionPtr Create (IECInstanceCR instance);

    //! Creates an ECValuesCollection representing all of the child ECPropertyValues contained in the specified
    //! ECPropertyValue.
    //! @param[in]      propertyValue The ECPropertyValue from which to obtain child property values
    //! @return     A collection of child ECPropertyValues, or nullptr if no such collection exists.
    ECOBJECTS_EXPORT static ECValuesCollectionPtr Create (ECPropertyValueCR propertyValue);

    //! Gets the ECPropertyValue containing this collection's property values.
    //! @return the ECPropertyValue containing this collection's property values.
    ECOBJECTS_EXPORT ECPropertyValueCR  GetParentProperty () const;
    };

/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

//#pragma make_public (ECN::ECValuesCollection)
