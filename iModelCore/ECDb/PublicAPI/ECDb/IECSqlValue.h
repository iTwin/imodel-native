/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/IECSqlValue.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECSqlColumnInfo.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//__PUBLISH_SECTION_END__
struct IECSqlPrimitiveValue;
//__PUBLISH_SECTION_START__

struct IECSqlStructValue;
struct IECSqlArrayValue;

//=======================================================================================
//! The IECSqlValue represents the value of a specific ECSQL column in the current
//! row of the result set of an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct IECSqlValue : NonCopyableClass
    {
private:
    virtual ECSqlColumnInfoCR _GetColumnInfo() const = 0;

    virtual bool _IsNull() const = 0;

    //__PUBLISH_SECTION_END__
    virtual IECSqlPrimitiveValue const& _GetPrimitive() const = 0;
    //__PUBLISH_SECTION_START__
    virtual IECSqlStructValue const& _GetStruct() const = 0;
    virtual IECSqlArrayValue const& _GetArray() const = 0;


public:
    ECDB_EXPORT virtual ~IECSqlValue() {}

    //! Gets the metadata of this value
    //! @return ECSQL column metadata.
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    ECDB_EXPORT ECSqlColumnInfoCR GetColumnInfo() const;

    //! Indicates whether the value is %NULL or not.
    //! @return true if column value is %NULL, false otherwise
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    ECDB_EXPORT bool IsNull() const;

    //! Gets value as a binary / blob
    //! @param[out] binarySize the size of the blob in bytes.
    //! @return The binary value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    ECDB_EXPORT void const* GetBinary(int* binarySize = nullptr) const;

    //! Gets the value as a boolean value.
    //! @return Value as boolean
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    ECDB_EXPORT bool GetBoolean() const;

    //! Gets the value as a DateTime value.
    //! @return DateTime value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not DateTime
    //! @see @ref ECDbCodeSampleECSqlStatementAndDateTimeProperties
    ECDB_EXPORT DateTime GetDateTime() const;

//__PUBLISH_SECTION_END__
    //! Gets the value as DateTime Julian Day ticks in hecto-nanoseconds.
    //! @param[out] metadata DateTime metadata.
    //! @return Julian Day ticks in hecto-nanoseconds
    //! @see BentleyApi::DateTime::FromJulianDay
    ECDB_EXPORT uint64_t GetDateTimeJulianDaysHns(DateTime::Info& metadata) const;

    //! Gets the value as DateTime Julian Day.
    //! @param[out] metadata DateTime metadata.
    //! @return Julian Day
    //! @see BentleyApi::DateTime::FromJulianDay
    ECDB_EXPORT double GetDateTimeJulianDays(DateTime::Info& metadata) const;

//__PUBLISH_SECTION_START__

    //! Gets the value as a double
    //! @return Double value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    ECDB_EXPORT double GetDouble() const;

    //! Gets the value as an integer
    //! @return Integer value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    ECDB_EXPORT int GetInt() const;

    //! Gets the value as Int64
    //! @return Int64 value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    ECDB_EXPORT int64_t GetInt64() const;

    //! Gets the value as UTF-8 encoded string
    //! @return UTF-8 encoded string value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    ECDB_EXPORT Utf8CP GetText() const;

    //! Gets the value as Point2D
    //! @return Point2D value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not Point2D
    ECDB_EXPORT DPoint2d GetPoint2D () const;

    //! Gets the value as Point3D
    //! @return Point3D value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not Point3D
    ECDB_EXPORT DPoint3d GetPoint3D () const;

    //__PUBLISH_SECTION_END__
    //! Gets the value as a Bentley Geometry Flatbuffer blob.
    //! @return Bentley Geometry Flatbuffer blob
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not Geometry
    void const* GetGeometryBlob(int* blobSize = nullptr) const;
    //__PUBLISH_SECTION_START__

    //! Gets the value as an IGeometry value.
    //! @return Column value as IGeometry
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not IGeometry
    ECDB_EXPORT IGeometryPtr GetGeometry() const;
   
    //! Gets the value as a BeInt64Id
    //! @remarks As @ref ECInstanceId "ECInstanceIds" are BeInt64Ids, you can use
    //! this method to get ECInstanceId values.
    //! @return BeInt64Id value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data does not hold a BeInt64Id
    template <class TBeInt64Id>
    TBeInt64Id GetId() const { return TBeInt64Id(GetInt64()); }

    //! Used to access the value if it is a struct value
    //! @return Struct value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not an ECStruct
    ECDB_EXPORT IECSqlStructValue const& GetStruct() const;

    //! Used to access the value if it is an array value
    //! @return Array value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not an array
    ECDB_EXPORT IECSqlArrayValue const& GetArray() const;
    };

//=======================================================================================
//! Represents the struct value of a specific ECSQL column in the current
//! row of the result set of an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct IECSqlStructValue : NonCopyableClass
    {
private:
    virtual int _GetMemberCount() const = 0;
    virtual IECSqlValue const& _GetValue(int  columnIndex) const = 0;

public:
    ECDB_EXPORT virtual ~IECSqlStructValue() {}

    //! Returns the number of struct members this struct value has.
    //! @return number of struct members of this struct value.
    ECDB_EXPORT int GetMemberCount() const;
    
    //! Returns the value of the struct member at the given index
    ECDB_EXPORT IECSqlValue const& GetValue(int structMemberIndex) const;
    };

//=======================================================================================
//! Represents the array value of a specific ECSQL column in the current
//! row of the result set of an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct IECSqlArrayValue : NonCopyableClass
    {
public:
    //=======================================================================================
    //! An iterator to iterate over the array elements of an IECSqlArrayValue
    // @bsiclass                                                 Krischan.Eberle    03/2014
    //+===============+===============+===============+===============+===============+======
    struct const_iterator : std::iterator<std::forward_iterator_tag, IECSqlValue const*>
        {
    private:
        IECSqlArrayValue const* m_arrayValue;

        bool IsAtEnd() const;
        bool IsEndIterator() const;

    public:
        // end iterator
        ECDB_EXPORT const_iterator();
        ECDB_EXPORT explicit const_iterator(IECSqlArrayValue const& arrayValue);
        ECDB_EXPORT ~const_iterator();
        //copyable
        ECDB_EXPORT const_iterator(const_iterator const& rhs);
        ECDB_EXPORT const_iterator& operator= (const_iterator const& rhs);
        //moveable
        ECDB_EXPORT const_iterator(const_iterator&& rhs);
        ECDB_EXPORT const_iterator& operator= (const_iterator&& rhs);

        ECDB_EXPORT IECSqlValue const* operator* () const;
        ECDB_EXPORT const_iterator& operator++ ();
        ECDB_EXPORT bool operator== (const_iterator const& rhs) const;
        ECDB_EXPORT bool operator!= (const_iterator const& rhs) const;
        };
private:
    virtual void _MoveNext(bool onInitializingIterator = false) const = 0;
    virtual bool _IsAtEnd() const = 0;
    virtual IECSqlValue const* _GetCurrent() const = 0;
    virtual int _GetArrayLength() const = 0;

public:
    ECDB_EXPORT virtual ~IECSqlArrayValue() {}

    //! Gets an iterator for iterating the array
    //! @remarks This invalidates any other already existing iterators of the same IECSqlArrayValue.
    //! @return Array iterator
    ECDB_EXPORT const_iterator begin() const;
    //! Gets the array's end iterator
    //! @return Array's end iterator
    ECDB_EXPORT const_iterator end() const;

    //! Gets the number of elements in the array.
    //! @remarks If the array length is not needed upfront, prefer to iterate through the array using
    //! its iterator and count the instances while going. Calling GetArrayLength can be costly as it might require
    //! %ECDb to perform a separate query into the %ECDb file.
    //! @return number of elements in the array
    ECDB_EXPORT int GetArrayLength() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
