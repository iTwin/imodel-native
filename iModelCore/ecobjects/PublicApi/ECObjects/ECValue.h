/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECValue.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <Bentley/VirtualCollectionIterator.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <Geom/GeomApi.h>
struct _FILETIME;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<ECPropertyValue> ECPropertyValuePtr;
typedef RefCountedPtr<ECValuesCollection> ECValuesCollectionPtr;

//=======================================================================================    
//! SystemTime structure is used to set and get time data from ECValue objects.
//! @see ECValue
//=======================================================================================    
struct SystemTime
{
public:
    unsigned short wYear; //!< The year component
    unsigned short wMonth; //!< The month component (1-12; January = 1) 
    unsigned short wDayOfWeek; //!< The day of the week (0-6; Sunday = 0)
    unsigned short wDay; //!< Day of month (1-31)
    unsigned short wHour; //!< The hour component 
    unsigned short wMinute; //!< The minute component
    unsigned short wSecond; //!< The second component
    unsigned short wMilliseconds; //!< The milliseconds component

    //! Creates a new SystemTime instance to the specified year, month, day, hour, minute, second, and millisecond
    //! @param[in] year         The year component (1 through 9999)
    //! @param[in] month        The month component (1-12)
    //! @param[in] day          The day (1 through the number of days in month)
    //! @param[in] hour         The hours (0-23)
    //! @param[in] minute       The minutes (0-59)
    //! @param[in] second       The seconds (0-59)
    //! @param[in] milliseconds The milliseconds (0-999)
    ECOBJECTS_EXPORT SystemTime(unsigned short year=1601, unsigned short month=1, unsigned short day=1, unsigned short hour=0, unsigned short minute=0, unsigned short second=0, unsigned short milliseconds=0);

    ECOBJECTS_EXPORT static SystemTime GetLocalTime(); //!< Returns a SystemTime instance representing the current Local Time.
    ECOBJECTS_EXPORT static SystemTime GetSystemTime(); //!< Returns a SystemTime instance representing the current system time expressed in Coordinated Univeral Time (UTC)
    ECOBJECTS_EXPORT WString      ToString (); //!< Returns the time as a string: M/D/Y-H:M:S:MS
    ECOBJECTS_EXPORT bool          operator== (const SystemTime&) const; //!< Compares two SystemTime instances for equality

    //! Given a FileTime object, creates the equivalent SystemTime object
    //! @param[in] fileTime The _FILETIME object containing the specified time
    ECOBJECTS_EXPORT BentleyStatus  InitFromFileTime (_FILETIME const& fileTime);

    //! Initializes this SystemTime object from unix milliseconds
    //! @param[in] unixMillis   The unix millisecond representation of the specified time
    ECOBJECTS_EXPORT BentleyStatus  InitFromUnixMillis (UInt64 unixMillis);
    };

//=======================================================================================    
//! Information about an array in an IECInstance. Does not contain the actual elements.
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
    //! Initialize the array as a struct array with the given number of entries
    //! @param[in] count        How many elements the array (initially) holds
    //! @param[in] isFixedSize  Indicates whether the array can grow or not
    void InitializeStructArray (UInt32 count, bool isFixedSize); // cannot have a real constructor due to inclusion in a union

    //! Initialize the array as a primitive array with the given number of entries
    //! @param[in] elementPrimitiveType The PrimitiveType of elements that this array can hold
    //! @param[in] count                How many elements the array (initially) holds
    //! @param[in] isFixedCount         Indicates whether the array can grow or not
    void InitializePrimitiveArray (PrimitiveType elementPrimitiveType, UInt32 count, bool isFixedCount); // cannot have a real constructor due to inclusion in a union
    
    ECOBJECTS_EXPORT UInt32          GetCount() const;                  //!< Returns the number of entries in this array
    ECOBJECTS_EXPORT bool            IsFixedCount() const;              //!< Returns whether this is a fixed size array or not
    ECOBJECTS_EXPORT bool            IsPrimitiveArray() const;          //!< Returns whether this is a primitive array
    ECOBJECTS_EXPORT bool            IsStructArray() const;             //!< Returns whether this is a struct array
    ECOBJECTS_EXPORT ValueKind       GetKind() const;                   //!< Returns the kind of array this is (primitive or struct)
    ECOBJECTS_EXPORT PrimitiveType   GetElementPrimitiveType() const;   //!< Returns the primitive type that this array was initialized with
    };

//! Variant-like object representing the value of an ECPropertyValue. 
//! It does not represent a "live" reference into the underlying IECInstance 
//! (or the object that the IECInstance represents). Changing the ECValue will not affect
//! the IECInstance unless you subsequently call SetValue() with it.
struct ECValue
    {
public:
    //! Performs a shallow copy
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
    BentleyStatus       SetBinaryInternal (const byte * data, size_t size, bool holdADuplicate = false);
    bool                ConvertToPrimitiveFromString (PrimitiveType primitiveType);

protected:    
    typedef bvector<ECValue>  ValuesVector;
    typedef bvector<ECValue>* ValuesVectorP;
    
    //! Structure to hold information about a binary type
    struct BinaryInfo
        {
        const byte *        m_data; //!< The actual binary data
        size_t              m_size; //!< The size of the data
        };

    //! Structure to hold information about String values
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
        // All the business with the flags parameters is so that StringInfo can modify ECValue's ownership flags.
        // If we stored the flags on StringInfo, we would increase the size of the union.

        //! Returns the stored string information as a WChar
        //! @param[out]  flags   A flag indicating whether this ECValue owns the data
        //! @returns The data as a WChar const pointer
        WCharCP             GetWChar (UInt8& flags);

        //! Returns the stored string information as Utf8
        //! @param[out]  flags   A flag indicating whether this ECValue owns the data
        //! @returns The data as a Utf8 const pointer
        Utf8CP              GetUtf8 (UInt8& flags);

        //! Returns the stored string information as Utf16
        //! @param[out]  flags   A flag indicating whether this ECValue owns the data
        //! @returns The data as a Utf16 const pointer
        Utf16CP             GetUtf16 (UInt8& flags);

        //! Sets the string data as WChar
        //!@param[in] str       The string data to store
        //!@param[out] flags    A flag indicating whether this ECValue owns the data
        //!@param[in] makeCopy  Indicates whether the passed in WCharCP str should be stored, or whether a copy should be made
        void                SetWChar (WCharCP str, UInt8& flags, bool makeCopy);

        //! Sets the string data as Utf8
        //!@param[in] str       The string data to store
        //!@param[out] flags    A flag indicating whether this ECValue owns the data
        //!@param[in] makeCopy  Indicates whether the passed in WCharCP str should be stored, or whether a copy should be made
        void                SetUtf8 (Utf8CP str, UInt8& flags, bool makeCopy);

        //! Sets the string data as Utf16
        //!@param[in] str       The string data to store
        //!@param[out] flags    A flag indicating whether this ECValue owns the data
        //!@param[in] makeCopy  Indicates whether the passed in WCharCP str should be stored, or whether a copy should be made
        void                SetUtf16 (Utf16CP str, UInt8& flags, bool makeCopy);

        //! Frees (if necessary) the string data and clears out the memory
        //! @param[out] flags   Gets reset to 0, indicating the data is not owned
        void                FreeAndClear (UInt8& flags);

        //! does not free pointers - used to init from Uninitialized ECValue state
        void                SetNull();

        //! Compares two StringInfo objects for equality
        //! @param[in] rhs  The StringInfo object to compare this object to
        //! @param[out] flags   A flag indicating whether the ECValue owns the data in the StringInfo object
        bool                Equals (StringInfo const& rhs, UInt8& flags);
        };

    //! The union storing the actual data of this ECValue
    union
        {
        bool                m_boolean;      //!< If a Boolean primitive type, holds the bool value
        ::Int32             m_integer32;    //!< If an Int32 primitive type, holds the Int32 value
        ::Int64             m_long64;       //!< If an Int64 primitive type, holds the Int64 value
        double              m_double;       //!< If a double primitive type, holds the double value
        //! If a String primitive type, holds the StringInfo struct defining the string
        mutable StringInfo  m_stringInfo;       // mutable so that we can convert to requested encoding on demand
        ::Int64             m_dateTime;     //!< If a DateTime primitive, holds the DateTime value as an Int64 representation
        DPoint2d            m_dPoint2d;     //!< If a DPoint2d primitive, holds the DPoint2d value
        DPoint3d            m_dPoint3d;     //!< If a DPoint3d primitive, holds the DPoint3d value
        ArrayInfo           m_arrayInfo;    //!< If an array value, holds the ArrayInfo struct defining the array
        BinaryInfo          m_binaryInfo;   //!< If a binary value, holds the BinaryInfo struct defining the binary data
        IECInstanceP        m_structInstance;   //!< The ECValue class calls AddRef and Release for the member as needed
        };

    void ConstructUninitialized(); //!< Constructs an uninitialized ECValue object
    inline void FreeMemory (); //!< If appropriate for the value type, frees the memory used to store the value
         
public:
    ECOBJECTS_EXPORT void            Clear(); //!< Clears memory, if necessary, and sets the value back to an uninitialized state
    ECOBJECTS_EXPORT ECValueR        operator= (ECValueCR rhs); //!< Compares two ECValues for equality
    
    ECOBJECTS_EXPORT ~ECValue(); //!< Destructor
    
    ECOBJECTS_EXPORT ECValue (); //!< Default Constructor.  Construct an uninitialized value
    
    //! Constructs a new ECValue based on the passed in ECValue
    //! @param[in] v    The ECValue to initialize the new ECValue from
    ECOBJECTS_EXPORT ECValue (ECValueCR v);

    //! Constructs an uninitialized ECValue of the specified ValueKind
    //! @param[in] classification   The type to set this new ECValue to
    ECOBJECTS_EXPORT explicit ECValue (ValueKind classification);

    //! Constructs an uninitialized ECValue of the specified PrimitiveType
    //! @param[in] primitiveType The type to set this new ECValue to
    ECOBJECTS_EXPORT explicit ECValue (PrimitiveType primitiveType);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_Integer
    //! @param[in] integer32 Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (::Int32 integer32);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_Long
    //! @param[in] long64 Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (::Int64 long64);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_Double
    //! @param[in] doubleVal Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (double doubleVal);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_String
    //! @param[in] string           Value to initialize this ECValue from
    //! @param[in] holdADuplicate   true, if a copy of \p string should be held in the ECValue object.
    //!                             false, otherwise.
    ECOBJECTS_EXPORT explicit ECValue (WCharCP string, bool holdADuplicate = true);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_String
    //! @param[in] string           Value to initialize this ECValue from
    //! @param[in] holdADuplicate   true, if a copy of \p string should be held in the ECValue object.
    //!                             false, otherwise.
    ECOBJECTS_EXPORT explicit ECValue (Utf8CP string, bool holdADuplicate = true);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_String
    //! @param[in] string           Value to initialize this ECValue from
    //! @param[in] holdADuplicate   true, if a copy of \p string should be held in the ECValue object.
    //!                             false, otherwise.
    ECOBJECTS_EXPORT explicit ECValue (Utf16CP string, bool holdADuplicate = true);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_Binary
    //! @Note No copy of \p blob is created. Use ECValue::SetBinary otherwise.
    //! @see ECValue::SetBinary
    //! @param[in] blob Value to initialize this ECValue from
    //! @param[in] size Size in bytes of the blob
    ECOBJECTS_EXPORT explicit ECValue (const byte * blob, size_t size);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_Point2D
    //! @param[in] point2d Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (DPoint2dCR point2d);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_Point3D
    //! @param[in] point3d Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (DPoint3dCR point3d);

    //! Initializes a new instance of ECValue from the given value.  Type is set to ::PRIMITIVETYPE_Boolean
    //! @param[in] value Value to initialize this ECValue from
    ECOBJECTS_EXPORT explicit ECValue (bool value);

    //! Constructs a new SystemTime ECValue
    //! @param[in] time The SystemTime value to store
    ECOBJECTS_EXPORT explicit ECValue (SystemTime const& time);

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
    
    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_String (regardless of encoding).
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_String. false otherwise.
    ECOBJECTS_EXPORT bool           IsString () const;
    //! Indicates whether the content of this ECValue is of type #PRIMITIVETYPE_Integer.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_Integer. false otherwise.
    ECOBJECTS_EXPORT bool           IsInteger () const;
    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_Long.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_Long. false otherwise.
    ECOBJECTS_EXPORT bool           IsLong () const;
    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_Double.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_Double. false otherwise.
    ECOBJECTS_EXPORT bool           IsDouble () const;
    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_Binary.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_Binary. false otherwise.
    ECOBJECTS_EXPORT bool           IsBinary () const;
    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_Boolean.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_Boolean. false otherwise.
    ECOBJECTS_EXPORT bool           IsBoolean () const;

    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_Point2D.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_Point2D. false otherwise.
    ECOBJECTS_EXPORT bool           IsPoint2D () const;
    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_Point3D.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_Point3D. false otherwise.
    ECOBJECTS_EXPORT bool           IsPoint3D () const;
    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_DateTime.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_DateTime. false otherwise.
    ECOBJECTS_EXPORT bool           IsDateTime () const;
    //! Indicates whether the content of this ECValue is of type ::PRIMITIVETYPE_IGeometry.
    //! @return true if the ECValue content is of type ::PRIMITIVETYPE_IGeometry. false otherwise.
    ECOBJECTS_EXPORT bool           IsIGeometry() const;

    //! Indicates whether the content of this ECValue is an array (::VALUEKIND_Array).
    //! @return true if the ECValue content is an array. false otherwise.
    ECOBJECTS_EXPORT bool           IsArray () const;
    //! Indicates whether the content of this ECValue is a struct (::VALUEKIND_Struct).
    //! @return true if the ECValue content is a struct. false otherwise.
    ECOBJECTS_EXPORT bool           IsStruct () const;
    //! Indicates whether the content of this ECValue is of a primitive type (::VALUEKIND_Primitive).
    //! @return true if the ECValue content is of a primitive type. false otherwise.
    ECOBJECTS_EXPORT bool           IsPrimitive () const;
    //! Gets the PrimitiveType of this ECValue        
    ECOBJECTS_EXPORT PrimitiveType  GetPrimitiveType() const; 

    //! Sets the PrimitiveType of this ECValue
    //! @param[in] primitiveElementType The type of primitive that this ECValue holds.
    ECOBJECTS_EXPORT BentleyStatus  SetPrimitiveType(PrimitiveType primitiveElementType);
    
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
    ECOBJECTS_EXPORT bool           ConvertPrimitiveToString (WStringR str) const;

    // Attempts to convert this ECValue's primitive value to a literal ECExpression
    // Does not support binary or IGeometry
    ECOBJECTS_EXPORT bool           ConvertPrimitiveToECExpressionLiteral (WStringR expression) const;

    // Attempts to format the underlying value using the specified .NET-style format string.
    // Typically the format string originated from an ECCustomAttribute.
    // Currently only supports numeric types: double, int, and long
    // Use DgnPlatform::IECInteropStringFormatter() for more full-featured formatting.
    ECOBJECTS_EXPORT bool           ApplyDotNetFormatting (WStringR formatted, WCharCP formatString) const;
    ECOBJECTS_EXPORT bool           SupportsDotNetFormatting() const;
/*__PUBLISH_SECTION_START__*/

    //! Defines the StructArray for this ECValue
    //! @param[in] count        The initial size of the array
    //! @param[in] isFixedSize  Indicates whether this array can grow or not
    ECOBJECTS_EXPORT ECObjectsStatus  SetStructArrayInfo (UInt32 count, bool isFixedSize);

    //! Defines the primitive array for this ECValue
    //! @param[in] primitiveElementtype The type of primitive the array will hold
    //! @param[in] count                The initial size of the array
    //! @param[in] isFixedSize          Indicates whether this array can grow or not
    ECOBJECTS_EXPORT ECObjectsStatus  SetPrimitiveArrayInfo (PrimitiveType primitiveElementtype, UInt32 count, bool isFixedSize);
    
    //! Returns the array information defining this ECValue
    ECOBJECTS_EXPORT ArrayInfo      GetArrayInfo() const;
    
    //! Returns the integer value, if this ECValue holds an Integer 
    ECOBJECTS_EXPORT Int32          GetInteger () const;  
    //! Sets the value of this ECValue to the given integer
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to an Integer Primitive
    //! @param[in] integer  The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetInteger (Int32 integer); 
    
    //! Returns the long value, if this ECValue holds a long
    ECOBJECTS_EXPORT Int64          GetLong () const;
    //! Sets the value of this ECValue to the given long
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a Long Primitive
    //! @param[in] long64  The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetLong (Int64 long64);
 
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
    //! @Note If the encoding of the string in the ECValue differs from the encoding of what is to be returned, the string
    //!       is automatically converted. To avoid string conversions call ECValue::IsUtf8 first.
    //! @return string content
    ECOBJECTS_EXPORT WCharCP        GetString () const;
    //! Gets the string content of this ECValue in UTF-8 encoding.
    //! @Note If the encoding of the string in the ECValue differs from the encoding of what is to be returned, the string
    //!       is automatically converted. To avoid string conversions call ECValue::IsUtf8 first.
    //! @return string content in UTF-8 encoding
    ECOBJECTS_EXPORT Utf8CP         GetUtf8CP () const;
    //! Returns the string value as a Utf16CP, if this ECValue holds a string
    ECOBJECTS_EXPORT Utf16CP        GetUtf16CP () const;    // the only real caller of this should be ECDBuffer

    //! Sets the value of this ECValue to the given string
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a string Primitive
    //! @param[in] string           The value to set
    //! @param[in] holdADuplicate   Flag specifying whether the ECValue should make its own copy of the string, or store the actual pointer passed in
    ECOBJECTS_EXPORT BentleyStatus  SetString (WCharCP string, bool holdADuplicate = true);
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
    ECOBJECTS_EXPORT const byte *   GetBinary (size_t& size) const;

    //! Sets the value of this ECValue to the given byte array
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a binary Primitive
    //! @param[in] data             The value to set
    //! @param[in] size             The size of the data
    //! @param[in] holdADuplicate   Flag specifying whether the ECValue should make its own copy of the string, or store the actual pointer passed in
    ECOBJECTS_EXPORT BentleyStatus  SetBinary (const byte * data, size_t size, bool holdADuplicate = false);

    //! Returns the IGeometry as binary data, and sets the size of the data, if this ECValue holds an IGeometry
    ECOBJECTS_EXPORT const byte *   GetIGeometry (size_t& size) const;
    //! Sets the value of this ECValue to the given IGeometry, as binary byte data.
    ECOBJECTS_EXPORT BentleyStatus  SetIGeometry (const byte * data, size_t size, bool holdADuplicate = false);

    //! Gets the struct instance of this ECValue, if the ECValue holds a struct
    ECOBJECTS_EXPORT IECInstancePtr GetStruct() const;
    //!Sets the specified struct instance in the ECValue. 
    //! \Note ECValue doesn't create a copy of \p structInstance. Its ref-count is incremented by this method though.
    //! @param[in] structInstance   The value to set
    //!@return SUCCESS or ERROR
    ECOBJECTS_EXPORT BentleyStatus  SetStruct (IECInstanceP structInstance);
        
    //! Returns the SystemTime value, if this ECValue holds a SystemTime
    ECOBJECTS_EXPORT SystemTime     GetDateTime() const;
    //! Sets the value of this ECValue to the given SystemTime
    //! @remarks This call will always succeed.  Previous data is cleared, and the type of the ECValue is set to a DateTime primitive
    //! @param[in] systemTime   The value to set
    ECOBJECTS_EXPORT BentleyStatus  SetDateTime (SystemTime const& systemTime); 

    //! Gets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @Note Ignores the date time metadata. Use ECValue::GetDateTime if you need the metadata.
    //! @return DateTime value as ticks since the beginning of the Common Era epoch.
    ECOBJECTS_EXPORT Int64          GetDateTimeTicks () const;

    //! Returns the DateTime value as milliseconds since the beginning of the Unix epoch.
    //! The Unix epoch begins at 1970-01-01 00:00:00 UTC.
    //! DateTimes before the Unix epoch are negative.
    //! @Note Ignores the date time metadata. Use ECValue::GetDateTime if you need the metadata.
    //! @return DateTime as milliseconds since the beginning of the Unix epoch.

    //! Returns the SystemTime value as Unix milliseconds, if this ECValue holds a SystemTime
    ECOBJECTS_EXPORT UInt64         GetDateTimeUnixMillis() const;
    //! Sets the DateTime value as ticks since the beginning of the Common Era epoch.
    //! @remarks Ticks are 100 nanosecond intervals (i.e. 1 tick is 1 hecto-nanosecond). The Common Era
    //! epoch begins at 0001-01-01 00:00:00 UTC.
    //! @Note If the ECProperty to which this ECValue will be applied contains the %DateTimeInfo custom attribute,
    //! the ticks will be enriched with the metadata from the custom attribute.
    //! @param[in] ceTicks DateTime Common Era ticks to set
    //! @return SUCCESS or ERROR
    ECOBJECTS_EXPORT BentleyStatus  SetDateTimeTicks (Int64 ceTicks);

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
    static UInt32                   GetFixedPrimitiveValueSize (PrimitiveType primitiveType);

    //! This is intended for debugging purposes, not for presentation purposes.
    ECOBJECTS_EXPORT WString       ToString () const;
    
    //! Checks 2 ECValues for equality.
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
        mutable ECPropertyCP    m_cachedProperty;
/*__PUBLISH_SECTION_END__*/
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

/*__PUBLISH_SECTION_END__*/
public:
    LocationVector const &   GetLocationVectorCR() const;

/*__PUBLISH_SECTION_START__*/
private:
    //"BACK" OF VECTOR IS DEEPEST ELEMENT
    LocationVector          m_locationVector;
/*__PUBLISH_SECTION_END__*/
    const LocationVector&   GetLocationVector() const;

public:
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
public:
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

    //! Get ECPropertyValue given an access string. The access string is in the format of managed accessors
    ECOBJECTS_EXPORT static ECPropertyValuePtr     GetPropertyValue (IECInstanceCR, WCharCP propertyAccessor);
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
public:
    typedef VirtualCollectionIterator<ECValuesCollectionIterator> const_iterator;
/*__PUBLISH_SECTION_END__*/
    friend struct ECPropertyValue;

private:
    ECPropertyValue                                     m_parentPropertyValue;
    mutable RefCountedPtr<ECValuesCollectionIterator>   m_end;

    ECValuesCollection ();
    ECValuesCollection (ECPropertyValueCR parentPropValue);
public:
    ECOBJECTS_EXPORT ECValuesCollection (IECInstanceCR);
/*__PUBLISH_SECTION_START__*/
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

#pragma make_public (Bentley::ECN::ECValuesCollection)
//__PUBLISH_SECTION_START__
