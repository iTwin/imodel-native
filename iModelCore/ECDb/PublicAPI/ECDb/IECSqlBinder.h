/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECSqlStatus.h>
#include <ECObjects/ECObjectsAPI.h>
#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//=======================================================================================
//! IECSqlBinder is used to bind a value to a binding parameter in an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE IECSqlBinder
    {
public:
    //=======================================================================================
    // Used to specify whether the binder should create an owned copy of the value to be bound
    // or not.
    // @bsiclass
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
    //not copyable
    IECSqlBinder(IECSqlBinder const&) = delete;
    IECSqlBinder& operator=(IECSqlBinder const&) = delete;

    virtual ECSqlStatus _BindNull() = 0;
    virtual ECSqlStatus _BindBoolean(bool) = 0;
    virtual ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy) = 0;
    virtual ECSqlStatus _BindZeroBlob(int blobSize) = 0;
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) = 0;
    virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) = 0;
    virtual ECSqlStatus _BindDouble(double) = 0;
    virtual ECSqlStatus _BindInt(int) = 0;
    virtual ECSqlStatus _BindInt64(int64_t) = 0;
    virtual ECSqlStatus _BindPoint2d(DPoint2dCR) = 0;
    virtual ECSqlStatus _BindPoint3d(DPoint3dCR) = 0;
    virtual ECSqlStatus _BindText(Utf8CP, IECSqlBinder::MakeCopy, int byteCount) = 0;
    virtual ECSqlStatus _BindVirtualSet(std::shared_ptr<VirtualSet>) = 0;

    virtual IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) = 0;
    virtual IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) = 0;
    virtual IECSqlBinder& _AddArrayElement() = 0;

protected:
#if !defined (DOCUMENTATION_GENERATOR)
    //not inlined to prevent being called outside ECDb
    IECSqlBinder();
#endif

public:
    virtual ~IECSqlBinder() {}

    //! Binds an ECSQL @c %NULL to the parameter
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindNull();

    //! Binds a boolean value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindBoolean(bool value);

    //! Binds a BLOB value to the parameter
    //! @param[in] value Value to bind
    //! @param[in] blobSize Size of the BLOB in bytes
    //! @param[in] makeCopy Flag that indicates whether a private copy of the blob is done or not. 
    //!            Only pass IECSqlBinder::MakeCopy::No if @p value remains valid until
    //!            the statement's bindings are cleared.
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy);

    //! Binds a zeroblob of the specified size to a parameter.
    //! @remarks A zeroblob is a BLOB consisting of @p blobSize bytes of 0x00. 
    //! SQLite manages these zeroblobs very efficiently. Zeroblobs can be used to reserve space for a BLOB that 
    //! is later written using incremental BLOB I/O. 
    //! @param[in] blobSize The number of bytes for the zeroblob.
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindZeroBlob(int blobSize);

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

    //! Binds a 32-bit integer value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindInt(int value);

    //! Binds an @ref BentleyApi::ECN::ECEnumeration "ECEnumeration" value to the parameter
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindEnum(ECN::ECEnumeratorCR value);

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
    ECSqlStatus BindGuid(BeGuidCR guid, IECSqlBinder::MakeCopy makeCopy) { return guid.IsValid() ? BindBlob(&guid, sizeof(guid), makeCopy) : BindNull(); }

    //! Binds to a NavigationECProperty parameter.
    //! @param[in] relatedInstanceId ECInstanceId of the related object. The id must be valid.
    //! @param[in] relationshipECClassId ECClassId of the ECRelationshipClass to navigate to the related ECInstance.
    //!            If an invalid @p relationshipECClassId is passed, NULL will be bound to it. This is only correct
    //!            if the relationshipECClassId is optional. ECDb does not validate the input.
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindNavigation(BeInt64Id relatedInstanceId, ECN::ECClassId relationshipECClassId = ECN::ECClassId());
    
    //! Binds a VirtualSet to the SQL function @b InVirtualSet.
    //! The parameter must be the first parameter in the InVirtualSet function.
    //! @param[in] shared pointer to virtualSet to bind
    //! @return ECSqlStatus::Success or error codes
    //! @see @ref ECDbCodeSampleECSqlStatementVirtualSets
    ECDB_EXPORT ECSqlStatus BindVirtualSet(std::shared_ptr<VirtualSet> virtualSet);

    //! Gets a binder for the specified struct member property
    //! @param[in] structMemberPropertyName Property name of the struct member to bind the value to
    //! @return The binder for the specified struct member property
    ECDB_EXPORT IECSqlBinder& operator[] (Utf8CP structMemberPropertyName);

#if !defined (DOCUMENTATION_GENERATOR)
    IECSqlBinder& operator[] (ECN::ECPropertyId structMemberPropertyId);
#endif

    //! Adds a new array element to the array to be bound to the parameter.
    //! @return The binder for the new array element
    ECDB_EXPORT IECSqlBinder& AddArrayElement();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
