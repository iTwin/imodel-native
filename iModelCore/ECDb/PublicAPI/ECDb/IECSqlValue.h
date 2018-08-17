/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/IECSqlValue.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECSqlColumnInfo.h>
#include <ECDb/ECSqlStatus.h>
#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct IECSqlValueIterable;

//=======================================================================================
//! The IECSqlValue represents the value of a specific ECSQL column in the current
//! row of the result set of an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE IECSqlValue
    {
    private:
        //not copyable
        IECSqlValue(IECSqlValue const&) = delete;
        IECSqlValue& operator=(IECSqlValue const&) = delete;

        virtual ECSqlColumnInfo const& _GetColumnInfo() const = 0;

        virtual bool _IsNull() const = 0;
        virtual void const* _GetBlob(int* blobSize) const = 0;
        virtual bool _GetBoolean() const = 0;
        virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const = 0;
        virtual uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const = 0;
        virtual double _GetDouble() const = 0;
        virtual int _GetInt() const = 0;
        virtual int64_t _GetInt64() const = 0;
        virtual IGeometryPtr _GetGeometry() const = 0;
        virtual DPoint2d _GetPoint2d() const = 0;
        virtual DPoint3d _GetPoint3d() const = 0;
        virtual Utf8CP _GetText() const = 0;

        virtual IECSqlValue const& _GetStructMemberValue(Utf8CP structMemberName) const = 0;
        virtual IECSqlValueIterable const& _GetStructIterable() const = 0;

        virtual int _GetArrayLength() const = 0;
        virtual IECSqlValueIterable const& _GetArrayIterable() const = 0;
    
    protected:
#if !defined (DOCUMENTATION_GENERATOR)
        //not inlined to prevent being called outside ECDb
        IECSqlValue();
#endif
    public:
        virtual ~IECSqlValue() {}

        //! Gets the metadata of this value
        //! @return ECSQL column metadata.
        ECDB_EXPORT ECSqlColumnInfo const& GetColumnInfo() const;

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

        //! Gets the value as @ref BentleyApi::ECN::ECEnumerator "ECEnumerator"
        //! @note as ECEnumerations cannot be OR'ed, so if the value does not match any of the ECEnumerators defined
        //! in the underlying ECEnumeration, nullptr will be returned.
        //! @return ECEnumerator or nullptr, if the underlying value does not represent an exact ECEnumerator of an ECEnumeration. 
        //! (OR'ed ECEnumerators are not supported)
        ECDB_EXPORT ECN::ECEnumeratorCP GetEnum() const;

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

        //! Returns the value of the struct member of this struct IECSqlValue
        //! with the given struct member name
        //! @param[in] structMemberName Property name of the requested for which the value is to be retrieved
        //! @note this can only called for struct values
        //! @return Value of the requested struct member
        ECDB_EXPORT IECSqlValue const& operator[] (Utf8CP structMemberName) const;

        //! Gets an iterable for iterating the struct members of this struct IECSqlValue
        //! @note this can only be called for struct values
        //! @return struct value iterable
        ECDB_EXPORT IECSqlValueIterable const& GetStructIterable() const;

        //! Gets the number of elements in this array IECSqlValue
        //! @note this can only called for array values
        //! @return number of elements in the array
        ECDB_EXPORT int GetArrayLength() const;

        //! Gets an iterable for iterating the array elements of this array IECSqlValue
        //! @note this can only be called for array values
        //! @return array value iterable
        ECDB_EXPORT IECSqlValueIterable const& GetArrayIterable() const;
    };


//=======================================================================================
//! An iterable to iterate over the array elements or struct members of an IECSqlValue
//! @see IECSqlValue::GetArrayIterable, IECSqlValue::GetStructIterable
// @bsiclass                                                 Krischan.Eberle    02/2017
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE IECSqlValueIterable
    {
    public:
#if !defined (DOCUMENTATION_GENERATOR)
        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    02/2017
        //+===============+===============+===============+===============+===============+======
        struct EXPORT_VTABLE_ATTRIBUTE IIteratorState
            {
            private:
                //not copyable
                IIteratorState(IIteratorState const&) = delete;
                IIteratorState& operator=(IIteratorState const&) = delete;

            protected:
                //not inlined to prevent being called outside ECDb
                IIteratorState();

            public:
                virtual ~IIteratorState() {}

                virtual std::unique_ptr<IIteratorState> _Copy() const = 0;
                virtual void _MoveToNext(bool onInitializingIterator) const = 0;
                virtual bool _IsAtEnd() const = 0;
                virtual IECSqlValue const& _GetCurrent() const = 0;
            };
#endif

        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    02/2017
        //+===============+===============+===============+===============+===============+======
        struct const_iterator final : std::iterator<std::forward_iterator_tag, IECSqlValue const*>
            {
            private:
                std::unique_ptr<IIteratorState> m_state = nullptr;

            public:
#if !defined (DOCUMENTATION_GENERATOR)
                const_iterator() {}
                //! normal iterator
                explicit const_iterator(std::unique_ptr<IIteratorState>);
#endif
                ~const_iterator() {}

                //copyable
                ECDB_EXPORT const_iterator(const_iterator const&);
                ECDB_EXPORT const_iterator& operator=(const_iterator const&);
                //moveable
                const_iterator(const_iterator&& rhs) : m_state(std::move(rhs.m_state)) {}
                ECDB_EXPORT const_iterator& operator=(const_iterator&&);

                ECDB_EXPORT IECSqlValue const& operator*() const;
                ECDB_EXPORT const_iterator& operator++();
                ECDB_EXPORT bool operator==(const_iterator const&) const;
                bool operator!=(const_iterator const& rhs) const { return !(*this == rhs); }
            };

    private:
        //not copyable
        IECSqlValueIterable(IECSqlValueIterable const&) = delete;
        IECSqlValueIterable& operator=(IECSqlValueIterable const&) = delete;

        virtual const_iterator _CreateIterator() const = 0;

    protected:
#if !defined (DOCUMENTATION_GENERATOR)
        //not inlined to prevent being called outside ECDb
        IECSqlValueIterable();
#endif
    public:
        virtual ~IECSqlValueIterable() {}

        ECDB_EXPORT const_iterator begin() const;
        const_iterator end() const { return const_iterator(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
