/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/IECSqlValue.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECSqlColumnInfo.h>
#include <ECDb/ECSqlStatus.h>
#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)
struct IECSqlPrimitiveValue;
#endif

struct IECSqlStructValue;
struct IECSqlArrayValue;

//=======================================================================================
//! The IECSqlValue represents the value of a specific ECSQL column in the current
//! row of the result set of an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE IECSqlValue : NonCopyableClass
    {
    private:
        virtual ECSqlColumnInfoCR _GetColumnInfo() const = 0;

        virtual bool _IsNull() const = 0;

        virtual IECSqlPrimitiveValue const& _GetPrimitive() const = 0;
        virtual IECSqlStructValue const& _GetStruct() const = 0;
        virtual IECSqlArrayValue const& _GetArray() const = 0;


    public:
        virtual ~IECSqlValue() {}

        //! Gets the metadata of this value
        //! @return ECSQL column metadata.
        ECDB_EXPORT ECSqlColumnInfoCR GetColumnInfo() const;

        //! Indicates whether the value is %NULL or not.
        //! @remarks When is a compound value %NULL?
        //!     - Point2d: if X, @b and Y is %NULL
        //!     - Point3d: if X, Y, and @b and Z is %NULL
        //!     - Struct: If all members are %NULL
        //!     - Navigation: If Id is %NULL (see also @ref ECDbNavigationProperties)
        //!     - Primitive/struct array: true if unset, false otherwise, even if an empty array was set
        //!     So there is a difference between the array being %NULL and empty.
        //! @return true if value is %NULL, false otherwise
        ECDB_EXPORT bool IsNull() const;

        //! Gets value as a BLOB
        //! @param[out] blobSize the size of the blob in bytes.
        //! @return The BLOB value
        ECDB_EXPORT void const* GetBlob(int* blobSize = nullptr) const;

        //! Gets the value as a boolean value.
        //! @return Value as boolean
        //! @note Possible errors:
        //! - property is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
        //!   those types can implicitly be converted into each other.
        ECDB_EXPORT bool GetBoolean() const;

        //! Gets the value as a DateTime value.
        //! @return DateTime value
        //! @note Possible errors:
        //! - property data type is not DateTime
        //! @see @ref ECDbCodeSampleECSqlStatementAndDateTimeProperties
        ECDB_EXPORT DateTime GetDateTime() const;

#if !defined (DOCUMENTATION_GENERATOR)
        //! Gets the value as a DateTime Julian Day in milliseconds.
        //! @param[out] metadata DateTime metadata.
        //! @return Julian Day in milliseconds
        //! @see BentleyApi::DateTime::FromJulianDay
        ECDB_EXPORT uint64_t GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const;

        //! Gets the value as DateTime Julian Day.
        //! @param[out] metadata DateTime metadata.
        //! @return Julian Day
        //! @see BentleyApi::DateTime::FromJulianDay
        ECDB_EXPORT double GetDateTimeJulianDays(DateTime::Info& metadata) const;
#endif

        //! Gets the value as a double
        //! @return Double value
        //! @note Possible errors:
        //! - property is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
        //!   those types can implicitly be converted into each other.
        ECDB_EXPORT double GetDouble() const;

        //! Gets the value as an integer
        //! @return Integer value
        //! @note Possible errors:
        //! - property is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
        //!   those types can implicitly be converted into each other.
        ECDB_EXPORT int GetInt() const;

        //! Gets the value as Int64
        //! @return Int64 value
        //! @note Possible errors:
        //! - property is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
        //!   those types can implicitly be converted into each other.
        ECDB_EXPORT int64_t GetInt64() const;

        //! Gets the value as uint64_t
        //! @return uint64_t value
        //! @note Possible errors:
        //! - property is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
        //!   those types can implicitly be converted into each other.
        uint64_t GetUInt64() const { return (uint64_t) GetInt64(); }

        //! Gets the value as UTF-8 encoded string
        //! @return UTF-8 encoded string value
        //! @note Possible errors:
        //! - property is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
        //!   those types can implicitly be converted into each other.
        ECDB_EXPORT Utf8CP GetText() const;

        //! Gets the value as Point2d
        //! @return Point2d value
        //! @note Possible errors:
        //! - property data type is not Point2d
        ECDB_EXPORT DPoint2d GetPoint2d() const;

        //! Gets the value as Point3d
        //! @return Point3d value
        //! @note Possible errors:
        //! - property data type is not Point3d
        ECDB_EXPORT DPoint3d GetPoint3d() const;

        //! Gets the value as an IGeometry value.
        //! @return Column value as IGeometry
        //! @note Possible errors:
        //! - property data type is not IGeometry
        ECDB_EXPORT IGeometryPtr GetGeometry() const;

        //! Gets the value as a BeInt64Id
        //! @remarks As @ref ECInstanceId "ECInstanceIds" are BeInt64Ids, you can use
        //! this method to get ECInstanceId values.
        //! @return BeInt64Id value
        //! @note Possible errors:
        //! - property data does not hold a BeInt64Id
        template <class TBeInt64Id>
        TBeInt64Id GetId() const { return TBeInt64Id(GetUInt64()); }

        //! Gets the value as a BeGuid
        //! @return BeGuid value
        //! @note Possible errors:
        //! - property data does not hold a BeGuid
        ECDB_EXPORT BeGuid GetGuid() const;

        //! Gets the value as a NavigationECProperty value
        //! @param[out] relationshipECClassId ECClassId of the ECRelationshipClass used to navigate to the related ECInstance.
        //!             You can pass nullptr to this parameter if you don't want it to be returned
        //! @return ECInstanceId of the related ECInstance
        //! @note Possible errors:
        //! - property does not refer to a NavigationECProperty
        ECDB_EXPORT BeInt64Id GetNavigation(ECN::ECClassId* relationshipECClassId) const;

        //! Gets the value as a NavigationECProperty value
        //! @param[out] relationshipECClassId ECClassId of the ECRelationshipClass used to navigate to the related ECInstance.
        //!             You can pass nullptr to this parameter if you don't want it to be returned
        //! @return ECInstanceId of the related ECInstance
        //! @note Possible errors:
        //! - property does not refer to a NavigationECProperty
        template <class TBeInt64Id>
        TBeInt64Id GetNavigation(ECN::ECClassId* relationshipECClassId)  const { return TBeInt64Id(GetNavigation(relationshipECClassId).GetValueUnchecked()); }

        //! Used to access the value if it is a struct value
        //! @return Struct value
        //! @note Possible errors:
        //! - property data type is not an ECStruct
        ECDB_EXPORT IECSqlStructValue const& GetStruct() const;

        //! Used to access the value if it is an array value
        //! @return Array value
        //! @note Possible errors:
        //! - property data type is not an array
        ECDB_EXPORT IECSqlArrayValue const& GetArray() const;
    };

//=======================================================================================
//! Represents the struct value of a specific ECSQL column in the current
//! row of the result set of an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE IECSqlStructValue : NonCopyableClass
    {
    private:
        virtual int _GetMemberCount() const = 0;
        virtual IECSqlValue const& _GetValue(int  columnIndex) const = 0;
        
    public:
        virtual ~IECSqlStructValue() {}

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
struct EXPORT_VTABLE_ATTRIBUTE IECSqlArrayValue : NonCopyableClass
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

                bool IsAtEnd() const { return IsEndIterator() || m_arrayValue->_IsAtEnd(); }
                bool IsEndIterator() const { return m_arrayValue == nullptr; }

            public:
                // end iterator
                const_iterator() : m_arrayValue(nullptr) {}
                ECDB_EXPORT explicit const_iterator(IECSqlArrayValue const& arrayValue);
                ~const_iterator() {}
                //copyable
                const_iterator(const_iterator const& rhs) : m_arrayValue(rhs.m_arrayValue) {}
                ECDB_EXPORT const_iterator& operator= (const_iterator const& rhs);
                //moveable
                const_iterator(const_iterator&& rhs) : m_arrayValue(std::move(rhs.m_arrayValue)) { rhs.m_arrayValue = nullptr; }
                ECDB_EXPORT const_iterator& operator= (const_iterator&& rhs);

                ECDB_EXPORT IECSqlValue const* operator* () const;
                ECDB_EXPORT const_iterator& operator++ ();
                ECDB_EXPORT bool operator== (const_iterator const& rhs) const;
                bool operator!= (const_iterator const& rhs) const { return !(*this == rhs); }
            };
    private:
        virtual void _MoveNext(bool onInitializingIterator = false) const = 0;
        virtual bool _IsAtEnd() const = 0;
        virtual IECSqlValue const* _GetCurrent() const = 0;
        virtual int _GetArrayLength() const = 0;

    public:
        virtual ~IECSqlArrayValue() {}

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
