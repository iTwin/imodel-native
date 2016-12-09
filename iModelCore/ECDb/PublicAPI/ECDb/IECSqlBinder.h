/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/IECSqlBinder.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECSqlStatus.h>
#include <ECObjects/ECObjectsAPI.h>
#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct IECSqlPrimitiveBinder;
struct IECSqlStructBinder;
struct IECSqlArrayBinder;

//=======================================================================================
//! IECSqlBinder is used to bind a value to a binding parameter in an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    05/2013
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE IECSqlBinder : NonCopyableClass
    {
public:
    //=======================================================================================
    // Used to specify whether the binder should create an owned copy of the value to be bound
    // or not.
    // @bsiclass                                                 Krischan.Eberle    05/2013
    //+===============+===============+===============+===============+===============+======
    enum class MakeCopy
        {
        //! No copy of the value to be bound is made. Caller has to ensure that the value will 
        //! remain valid until the ECSqlStatement's bindings are cleared. 
        No,
        //! A copy of the value to be bound is made. The binder owns the copy and frees it accordingly.
        Yes
        };

private:
    virtual ECSqlStatus _BindNull() = 0;
    virtual IECSqlPrimitiveBinder& _BindPrimitive() = 0;
    virtual IECSqlStructBinder& _BindStruct() = 0;
    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) = 0;

public:
    virtual ~IECSqlBinder() {}

    //! Binds an ECSQL @c %NULL to the parameter
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindNull();

    //! Binds a boolean value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindBoolean(bool value);

    //! Binds a binary / BLOB value to the parameter
    //! @param[in] value Value to bind
    //! @param[in] binarySize Size of the BLOB in bytes
    //! @param[in] makeCopy Flag that indicates whether a private copy of the blob is done or not. 
    //!            Only pass IECSqlBinder::MakeCopy::No if @p value remains valid until
    //!            the statement's bindings are cleared.
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy);

    //! Binds a DateTime value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindDateTime(DateTimeCR value);

#if !defined (DOCUMENTATION_GENERATOR)
    //! Binds a %DateTime value expressed as Julian Day to the parameter
    //! @param[in] julianDayTicks DateTime value as Julian Day
    //! @param[in] metadata DateTime metadata. Pass an empty DateTime::Info object if no metadata exist for the ticks
    //! @return ECSqlStatus::Success or error codes
    //! @see BentleyApi::DateTime::ToJulianDay
    ECDB_EXPORT ECSqlStatus BindDateTime(double julianDay, DateTime::Info const& metadata);

    //! Binds a %DateTime value expressed as Julian Day ticks to the parameter
    //! @param[in] julianDayMsec DateTime value as Julian Day in milliseconds
    //! @param[in] metadata DateTime metadata. Pass an empty DateTime::Info object if no metadata exist for the ticks
    //! @return ECSqlStatus::Success or error codes
    //! @see BentleyApi::DateTime::ToJulianDay
    ECDB_EXPORT ECSqlStatus BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata);

#endif

    //! Binds a double value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindDouble(double value);

    //! Binds an IGeometry value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindGeometry(IGeometryCR value);

#if !defined (DOCUMENTATION_GENERATOR)
    //! Binds a Bentley Geometry FlatBuffer blob to the parameter
    //! @param[in] value Value to bind
    //! @param[in] blobSize Size of the BLOB in bytes
    //! @param[in] makeCopy Flag that indicates whether a private copy of the blob is done or not. 
    //!            Only pass IECSqlBinder::MakeCopy::No if @p value remains valid until
    //!            the statement's bindings are cleared.
    //! @return ECSqlStatus::Success or error codes
    ECSqlStatus BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy);
#endif

    //! Binds a 32-bit integer value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindInt(int value);

    //! Binds a 64-bit integer value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindInt64(int64_t value);

    //! Binds a Point2d value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindPoint2d(DPoint2dCR value);

    //! Binds a Point3d value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindPoint3d(DPoint3dCR value);

    //! Binds a UTF-8 encoded string to the parameter
    //! @param[in] value Value to bind
    //! @param[in] makeCopy indicates whether ECSqlStatement should make a private copy of @p value or not.
    //!             Only pass IECSqlBinder::MakeCopy::No if @p value will remain valid until the statement's bindings are cleared.
    //! @param[in] byteCount Number of bytes (not characters) in @p value. If negative, it will be calculated from value. Passing this value is only an optimization. 
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount = -1);

    //! Binds a BeInt64Id to the parameter. If the id is invalid, NULL is bound to the parameter.
    //! @remarks As @ref ECInstanceId "ECInstanceIds" are BeInt64Id, you can use
    //! this method to bind them to a parameter.
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECSqlStatus BindId(BeInt64Id value) { return value.IsValid() ? BindInt64((int64_t) value.GetValue()) : BindNull(); }

    //! Binds a BeGuid to the parameter. If the GUID is invalid, NULL is bound to the parameter.
    //! @param[in] guid BeGuid to bind
    //! @param[in] makeCopy indicates whether ECSqlStatement should make a private copy of @p guid or not.
    //!             Only pass IECSqlBinder::MakeCopy::No if @p guid will remain valid until the statement's bindings are cleared.
    //! @return ECSqlStatus::Success or error codes
    ECSqlStatus BindGuid(BeGuidCR guid, IECSqlBinder::MakeCopy makeCopy) { return guid.IsValid() ? BindBinary(&guid, sizeof(guid), makeCopy) : BindNull(); }

    //! Binds to a NavigationECProperty parameter.
    //! @param[in] relatedInstanceId ECInstanceId of the related object. The id must be valid.
    //! @param[in] relationshipECClassId ECClassId of the ECRelationshipClass to navigate to the related ECInstance.
    //!            If an invalid @p relationshipECClassId is passed, NULL will be bound to it. This is only correct
    //!            if the relationshipECClassId is optional. ECDb does not validate the input.
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindNavigation(BeInt64Id relatedInstanceId, ECN::ECClassId relationshipECClassId = ECN::ECClassId());
    
    //! Binds a VirtualSet to the SQL function @b InVirtualSet.
    //! The parameter must be the first parameter in the InVirtualSet function.
    //! @param[in] virtualSet to bind
    //! @return ECSqlStatus::Success or error codes
    //! @see @ref ECDbCodeSampleECSqlStatementVirtualSets
    ECSqlStatus BindVirtualSet(VirtualSet const& virtualSet) { return BindInt64((int64_t) &virtualSet); }

    //! Gets a binder which is used to bind struct values
    //! @remarks In case of error, e.g. if the parameter is not a struct, a no-op binder will be returned. Calling methods on the no-op binder
    //! returns the appropriate error-code.
    //! @return Struct parameter binder
    ECDB_EXPORT IECSqlStructBinder& BindStruct();
    
    //! Gets a binder which is used to bind primitive array values.
    //! @remarks In case of error, e.g. if the parameter
    //! is not a primitive array, a no-op binder will be returned. Calling methods on the no-op binder
    //! returns the appropriate error-code.
    //! @param[in] initialCapacity Initial capacity of the array to bind. 
    //! @return Array parameter binder
    ECDB_EXPORT IECSqlArrayBinder& BindArray(uint32_t initialCapacity);
    };

//=======================================================================================
//! IECSqlStructBinder is used to bind a struct value to a binding parameter in an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE IECSqlStructBinder : NonCopyableClass
    {
private:
    virtual IECSqlBinder& _GetMember(Utf8CP structMemberPropertyName) = 0;
    virtual IECSqlBinder& _GetMember(ECN::ECPropertyId structMemberPropertyId) = 0;

public:
    virtual ~IECSqlStructBinder() {}

    //! Binds a value to the specified struct member property
    //! @param[in] structMemberPropertyName Property name of the struct member to bind the value to
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT IECSqlBinder& GetMember(Utf8CP structMemberPropertyName);

#if !defined (DOCUMENTATION_GENERATOR)
    IECSqlBinder& GetMember(ECN::ECPropertyId structMemberPropertyId);
#endif
    };

//=======================================================================================
//! The IECSqlArrayBinder is used to bind an array to a binding parameter
//! in an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE IECSqlArrayBinder : NonCopyableClass
    {
private:
    virtual IECSqlBinder& _AddArrayElement() = 0;

public:
    virtual ~IECSqlArrayBinder() {}

    //! Adds a new array element to the array to be bound to the parameter and
    //! returns the new element's binder to bind a value to that element
    //! @return Binder to bind a value to the new array element
    ECDB_EXPORT IECSqlBinder& AddArrayElement();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
