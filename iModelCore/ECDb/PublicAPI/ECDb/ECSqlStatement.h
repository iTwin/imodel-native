/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECInstanceId.h>
#include <ECDb/IECSqlValue.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/SchemaManager.h>
#include <list>
#include <json/json.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECSqlStatement is used to perform Create, Read, Update, Delete operations (@b CRUD)
//! against @b ECInstances in an @ref ECDbFile "ECDb file".
//!
//! See @ref ECSqlStatementOverview for details on how to use ECSqlStatement.
//!
//! It is safe to use multiple ECSqlStatements in multiple threads. A given ECSqlStatement
//! can only be used in a single thread.
//!
//! @see @ref ECSqlStatementOverview, @ref ECDbOverview, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
//! @nosubgrouping
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECSqlStatement
    {
    public:
        struct Impl;

    private:
        Impl* m_pimpl = nullptr;

        //not copyable
        ECSqlStatement(ECSqlStatement const&) = delete;
        ECSqlStatement& operator=(ECSqlStatement const&) = delete;

    public:
        //! Initializes a new unprepared ECSqlStatement.
        ECDB_EXPORT ECSqlStatement();
        //! Destroys the ECSqlStatement and all internal resources.
        ECDB_EXPORT virtual ~ECSqlStatement();
        //! Move constructor
        ECDB_EXPORT ECSqlStatement(ECSqlStatement&& rhs);
        //! Move assignment operator
        ECDB_EXPORT ECSqlStatement& operator=(ECSqlStatement&& rhs);

        //! Finalizes this ECSqlStatement, i.e. frees any internal resources.
        //! @remarks Before calling ECDb::CloseDb explicitly make sure that all prepared ECSqlStatements
        //! are finalized.
        //! After having called Finalize, the ECSqlStatement can be re-prepared.
        //! Any ECSqlEventHandler currently being registered with this ECSqlStatement stay registered.
        ECDB_EXPORT void Finalize();

        //! Prepares the statement with the specified ECSQL
        //! @param[in] ecdb ECDb context
        //! @param[in] ecsql ECSQL
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus Prepare(ECDb const& ecdb, Utf8CP ecsql) { return Prepare(ecdb, ecsql, nullptr, true); }

        //! Prepares the statement with the specified ECSQL
        //! @param[in] ecdb ECDb context
        //! @param[in] ecsql ECSQL
        //! @param[in] logErrors true: Prepare errors will be logged. false: Prepare errors will not be logged
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus Prepare(ECDb const& ecdb, Utf8CP ecsql, bool logErrors) { return Prepare(ecdb, ecsql, nullptr, logErrors); }

        //! Prepares the statement with the specified ECSQL
        //! @param[in] ecdb ECDb context
        //! @param[in] ecsql ECSQL
        //! @param [in] token Token required to execute ECSQL INSERT, UPDATE, DELETE statements if
        //! the ECDb file was set-up with the option "ECSQL write token validation".
        //! If the option is not set, nullptr can be passed for @p token.
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus Prepare(ECDb const& ecdb, Utf8CP ecsql, ECCrudWriteToken const* token) { return Prepare(ecdb, ecsql, token, true); }

        //! Prepares the statement with the specified ECSQL
        //! @param[in] ecdb ECDb context
        //! @param[in] ecsql ECSQL
        //! @param [in] token Token required to execute ECSQL INSERT, UPDATE, DELETE statements if
        //! the ECDb file was set-up with the option "ECSQL write token validation".
        //! If the option is not set, nullptr can be passed for @p token.
        //! @param[in] logErrors true: Prepare errors will be logged. false: Prepare errors will not be logged
        //! @return ECSqlStatus::Success or error codes
        ECDB_EXPORT ECSqlStatus Prepare(ECDb const& ecdb, Utf8CP ecsql, ECCrudWriteToken const* token, bool logErrors);

        //! Prepares the statement with the specified @b SELECT ECSQL for multi-threading scenarios.
        //! @remarks This special type of prepare uses two different ECDb connections to the @b same ECDb file. One connection
        //! is used to parse the ECSQL (represented by @p schemaManager), the other connection is used to execute the statement
        //! (represented by @p dataSourceECDb). This can be leveraged in multi-threading scenarios
        //! where one thread would open the parse connection, and another thread the executing connection.
        //! @note The following conditions must be met. Otherwise an error is returned
        //!     * Only ECSQL SELECT is supported
        //!     * Both connections must point to the very same ECDb file.
        //!     * @p dataSourceECDb must be read-only
        //!
        //! @see @ref ECDbCodeSampleExecuteECSqlSelectMultiThreaded for a code example
        //! @param[in] schemaManager SchemaManager that is to be used to parse the ECSQL. e.g. as returned from  @ref BentleyApi::BeSQLite::EC::ECDb::Schemas() "ECDb::Schemas()"
        //! @param[in] dataSourceECDb Connection to the same %ECDb file which is to be used to execute ECSqlStatment. Must be read-only, but can be in a
        //! another thread than @p schemaManager
        //! @param[in] selectECSql SELECT ECSQL
        //! @param[in] logErrors true: Prepare errors will be logged. false: Prepare errors will not be logged
        //! @return ECSqlStatus::Success or error codes
        ECDB_EXPORT ECSqlStatus Prepare(SchemaManager const& schemaManager, Db const& dataSourceECDb, Utf8CP selectECSql, bool logErrors = true);

        //! Indicates whether this statement is already prepared or not.
        //! @return true, if it is prepared. false otherwise
        ECDB_EXPORT bool IsPrepared() const;

        //! @name Methods to bind values to an ECSQL parameter
        //! @{

        //! Binds an ECSQL @c %NULL to the parameter
        //! @param[in] parameterIndex Parameter index
        //!
        //! Example:
        //! The code
        //!     ECSqlStatement statement;
        //!     statement.Prepare (ecdb, "SELECT ECInstanceId, ECClassId FROM myschema.Foo WHERE MyProp <> ?");
        //!     statement.BindNull (1);
        //!
        //! finds all Foo rows where MyProp is unset.
        //!
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindNull(int parameterIndex) { return GetBinder(parameterIndex).BindNull(); }

        //! Binds a boolean value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindBoolean(int parameterIndex, bool value) { return GetBinder(parameterIndex).BindBoolean(value); }

        //! Binds a BLOB value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @param[in] blobSize Size of the BLOB in bytes
        //! @param[in] makeCopy Flag that indicates whether a private copy of the blob is done or not. Only pass
        //! IECSqlBinder::MakeCopy::No if @p value remains valid until
        //!            the statement's bindings are cleared.
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindBlob(int parameterIndex, const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) { return GetBinder(parameterIndex).BindBlob(value, blobSize, makeCopy); }

        //! Binds a zeroblob of the specified size to a parameter.
        //! @remarks A zeroblob is a BLOB consisting of @p blobSize bytes of 0x00.
        //! SQLite manages these zeroblobs very efficiently. Zeroblobs can be used to reserve space for a BLOB that
        //! is later written using incremental BLOB I/O.
        //! @param[in] parameterIndex Parameter index
        //! @param[in] blobSize The number of bytes for the zeroblob.
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindZeroBlob(int parameterIndex, int blobSize) { return GetBinder(parameterIndex).BindZeroBlob(blobSize); }

        //! Binds a DateTime value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindDateTime(int parameterIndex, DateTimeCR value) { return GetBinder(parameterIndex).BindDateTime(value); }

        //! Binds a double value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindDouble(int parameterIndex, double value) { return GetBinder(parameterIndex).BindDouble(value); }

        //! Binds an IGeometry value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindGeometry(int parameterIndex, IGeometryCR value) { return GetBinder(parameterIndex).BindGeometry(value); }

        //! Binds a 32-bit integer value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindInt(int parameterIndex, int value) { return GetBinder(parameterIndex).BindInt(value); }

        //! Binds an @ref BentleyApi::ECN::ECEnumeration "ECEnumeration" value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindEnum(int parameterIndex, ECN::ECEnumeratorCR value) { return GetBinder(parameterIndex).BindEnum(value); }

        //! Binds a 64-bit integer value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindInt64(int parameterIndex, int64_t value) { return GetBinder(parameterIndex).BindInt64(value); }

        //! Binds a Point2d value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindPoint2d(int parameterIndex, DPoint2dCR value) { return GetBinder(parameterIndex).BindPoint2d(value); }

        //! Binds a Point3d value to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindPoint3d(int parameterIndex, DPoint3dCR value) { return GetBinder(parameterIndex).BindPoint3d(value); }

        //! Binds a UTF-8 encoded string to the parameter
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind
        //! @param[in] makeCopy indicates whether ECSqlStatement should make a private copy of @p value or not.
        //!             Only pass IECSqlBinder::MakeCopy::No if @p value will remain valid until the statement's bindings are cleared.
        //! @param[in] byteCount Number of bytes (not characters) in @p value. If negative, it will be calculated from value. Passing this value is only an optimization.
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindText(int parameterIndex, Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount = -1) { return GetBinder(parameterIndex).BindText(value, makeCopy, byteCount); }

        //! Binds a BeInt64Id subclass to the parameter. Binds NULL if the value is not valid.
        //! @param[in] parameterIndex Parameter index
        //! @param[in] value Value to bind.
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindId(int parameterIndex, BeInt64Id value) { return GetBinder(parameterIndex).BindId(value); }

        //! Binds a BeGuid to the parameter. If the GUID is invalid, NULL is bound to the parameter.
        //! @param[in] parameterIndex Parameter index
        //! @param[in] guid BeGuid to bind
        //! @param[in] makeCopy indicates whether ECSqlStatement should make a private copy of @p guid or not.
        //!             Only pass IECSqlBinder::MakeCopy::No if @p guid will remain valid until the statement's bindings are cleared.
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindGuid(int parameterIndex, BeSQLite::BeGuidCR guid, IECSqlBinder::MakeCopy makeCopy = IECSqlBinder::MakeCopy::No) { return GetBinder(parameterIndex).BindGuid(guid, makeCopy); }

        //! Binds to a NavigationECProperty parameter.
        //! @param[in] parameterIndex Parameter index
        //! @param[in] relatedInstanceId ECInstanceId of the related object. The id must be valid.
        //! @param[in] relationshipECClassId ECClassId of the ECRelationshipClass to navigate to the related ECInstance.
        //!            If an invalid @p relationshipECClassId is passed, NULL will be bound to it. This is only correct
        //!            if the relationshipECClassId is optional. ECDb does not validate the input.
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindNavigationValue(int parameterIndex, BeInt64Id relatedInstanceId, ECN::ECClassId relationshipECClassId = ECN::ECClassId()) { return GetBinder(parameterIndex).BindNavigation(relatedInstanceId, relationshipECClassId); }

        //! Binds a VirtualSet to the SQL function @b InVirtualSet.
        //! The parameter must be the first parameter in the InVirtualSet function.
        //! @param[in] parameterIndex Parameter index
        //! @param[in] virtualSet to bind
        //! @return ECSqlStatus::Success or error codes
        //! @see @ref ECDbCodeSampleECSqlStatementVirtualSets
        ECSqlStatus BindVirtualSet(int parameterIndex, std::shared_ptr<VirtualSet> virtualSet) { return GetBinder(parameterIndex).BindVirtualSet(virtualSet); }

        //! Binds an IdSet to the SQL function @b InVirtualSet.
        //! The parameter must be the first parameter in the InVirtualSet function.
        //! @param[in] parameterIndex Parameter index
        //! @param[in] idSet to bind
        //! @return ECSqlStatus::Success or error codes
        ECSqlStatus BindIdSet(int parameterIndex, std::shared_ptr<VirtualSet> virtualSet) { return GetBinder(parameterIndex).BindIdSet(virtualSet); }

        //! Gets a binder to bind a value to the parameter at the specified index.
        //! @param[in] parameterIndex Parameter index
        //! @remarks In case of error, e.g. if the parameter index is out of bounds
        //! a no-op binder will be returned. Calling methods on the no-op binder
        //! returns the appropriate error-code.
        //! @return Parameter binder
        ECDB_EXPORT IECSqlBinder& GetBinder(int parameterIndex);

        //! Gets the parameter index for a named parameter. Will log an error if parameter not found.
        //!
        //! @e Example
        //!  ECSqlStatement statement;
        //!  statement.Prepare (ecdb, "SELECT Prop1, Prop2 FROM myschema.Foo Where ECInstanceId = :id");
        //!  ECSqlStatus stat = statement.BindInt64 (statement.GetParameterIndex ("id"), 123);
        //!
        //! @param[in] parameterName Name of the binding parameter
        //! @return Parameter index
        //! @see TryGetParameterIndex
        ECDB_EXPORT int GetParameterIndex(Utf8CP parameterName) const;

        //! Gets the parameter index for a named parameter. Will not log an error if parameter not found.
        //! @param[in] parameterName Name of the binding parameter
        //! @return Parameter index
        //! @see GetParameterIndex
        ECDB_EXPORT int TryGetParameterIndex(Utf8CP parameterName) const;

        //! Resets all parameter bindings
        //! @return ECSqlStatus::Success or error codes
        ECDB_EXPORT ECSqlStatus ClearBindings();

        //! @}

        //! Perform a single step on this statement
        //! @remarks For select statements, BentleyApi::BeSQLite::DbResult::BE_SQLITE_ROW indicates that data (a row) is returned which is ready to be
        //!          processed by the caller, and BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE is returned after a
        //!          successful execution of the step. For non-select statements (Insert, Update, Delete) BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE is always
        //!          returned in case of success and error codes in case of error.
        //!          When BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE was returned, Step should not be called again.
        //! @return BentleyApi::BeSQLite::DbResult::BE_SQLITE_ROW if Step returned data which is ready to be processed by the caller.
        //!         BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE if the reader has finished executing successfully.
        //!         Error codes in case of errors.
        ECDB_EXPORT DbResult Step();

        //! Perform a single step on this (previously prepared) @b insert statement
        //! @remarks This overload is intended for insert statements only as it returns the ECInstanceKey of the inserted row.
        //! @param[out] ecInstanceKey The ECInstanceKey of the inserted row.
        //! @return BentleyApi::BeSQLite::DbResultBE_SQLITE_ROWROW if Step returned data which is ready to be processed by the caller.
        //!         BentleyApi::BeSQLite::DbResult::BE_SQLITE_DONE if the reader has finished executing successfully.
        //!         Error codes in case of errors.
        ECDB_EXPORT DbResult Step(ECInstanceKey& ecInstanceKey) const;

        //! Resets the statement, so that it can be reiterated.
        //! @remarks Reset does not clear the bindings of the statement. Call ECSqlStatement::ClearBindings to do that.
        //! @return ECSqlStatus::Success or error codes
        ECDB_EXPORT ECSqlStatus Reset();

        //! Gets the number of ECSQL columns in the result set returned after calling Step on a SELECT statement.
        //! @return Number of ECSQL columns in the result set
        ECDB_EXPORT int GetColumnCount() const;

        //! @name Get metadata for a given column in the result set
        //! @{

        //! Gets the metadata about the specified ECSQL column in the result set.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return ECSQL column metadata.
        //! @note Possible errors:
        //! - @p columnIndex is out of bounds
        ECSqlColumnInfoCR GetColumnInfo(int columnIndex) const { return GetValue(columnIndex).GetColumnInfo(); }

        //! @}

        //! @name Get values for a given column in the result set
        //! @{

        //! Indicates whether the value for the column at the given index in the result set is %NULL or not.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return true if column value is %NULL, false otherwise
        //! @note Possible errors:
        //! - @p columnIndex is out of bounds
        bool IsValueNull(int columnIndex) const { return GetValue(columnIndex).IsNull(); }

        //! Gets the BLOB value of the specific column.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @param[out] blobSize the size of the BLOB in bytes.
        //! @return The BLOB value
        //! @note Possible errors:
        //! - @p columnIndex is out of bounds
        void const* GetValueBlob(int columnIndex, int* blobSize = nullptr) const { return GetValue(columnIndex).GetBlob(blobSize); }

        //! Gets the value of the specific column as a boolean value.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Column value as boolean
        //! @note Possible errors:
        //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only
        //!   those types can implicitly be converted into each other.
        //! - @p columnIndex is out of bounds
        bool GetValueBoolean(int columnIndex) const { return GetValue(columnIndex).GetBoolean(); }

        //! Gets the DateTime value of the specific column.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return DateTime value
        //! @note Possible errors:
        //! - column data type is not DateTime
        //! - @p columnIndex is out of bounds
        //! @see @ref ECDbCodeSampleECSqlStatementAndDateTimeProperties
        DateTime GetValueDateTime(int columnIndex) const { return GetValue(columnIndex).GetDateTime(); }

        //! Gets the value of the specific column as a double value.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Column value as double
        //! @note Possible errors:
        //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only
        //!   those types can implicitly be converted into each other.
        //! - @p columnIndex is out of bounds
        double GetValueDouble(int columnIndex) const { return GetValue(columnIndex).GetDouble(); }

        //! Gets the value of the specific column as an IGeometry value.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Column value as IGeometry
        //! @note Possible errors:
        //! - column data type is not IGeometry
        //! - @p columnIndex is out of bounds
        IGeometryPtr GetValueGeometry(int columnIndex) const { return GetValue(columnIndex).GetGeometry(); }

        //! Gets the value of the specific column as an integer value.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Column value as integer
        //! @note Possible errors:
        //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only
        //!   those types can implicitly be converted into each other.
        //! - @p columnIndex is out of bounds
        int GetValueInt(int columnIndex) const { return GetValue(columnIndex).GetInt(); }

        //! Gets the value of the specific column as @ref BentleyApi::ECN::ECEnumerator "ECEnumerator".
        //! @note as ECEnumerations cannot be OR'ed, so if the value does not match any of the ECEnumerators defined
        //! in the underlying ECEnumeration, nullptr will be returned.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return ECEnumerator or nullptr, if the underlying value does not represent a defined ECEnumerator of the ECEnumeration.
        //! (OR'ed ECEnumerators are not supported)
        ECN::ECEnumeratorCP GetValueEnum(int columnIndex) const { return GetValue(columnIndex).GetEnum(); }

        //! Gets the value of the specific column as an Int64 value.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Column value as Int64
        //! @note Possible errors:
        //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only
        //!   those types can implicitly be converted into each other.
        //! - @p columnIndex is out of bounds
        int64_t GetValueInt64(int columnIndex) const { return GetValue(columnIndex).GetInt64(); }

        //! Gets the value of the specific column as an uint64_t value.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Column value as uint64_t
        //! @note Possible errors:
        //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only
        //!   those types can implicitly be converted into each other.
        //! - @p columnIndex is out of bounds
        uint64_t GetValueUInt64(int columnIndex) const { return GetValue(columnIndex).GetUInt64(); }

        //! Gets the Point2d value of the specific column.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Point2d column value
        //! @note Possible errors:
        //! - column data type is not Point2d
        //! - @p columnIndex is out of bounds
        DPoint2d GetValuePoint2d(int columnIndex) const { return GetValue(columnIndex).GetPoint2d(); }

        //! Gets the Point3d value of the specific column.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Point3d column value
        //! @note Possible errors:
        //! - column data type is not Point3d
        //! - @p columnIndex is out of bounds
        DPoint3d GetValuePoint3d(int columnIndex) const { return GetValue(columnIndex).GetPoint3d(); }

        //! Gets the value of the specific column as a string value.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return Column value as string
        //! @note Possible errors:
        //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only
        //!   those types can implicitly be converted into each other.
        //! - @p columnIndex is out of bounds
        Utf8CP GetValueText(int columnIndex) const { return GetValue(columnIndex).GetText(); }

        //! Gets the value of the specific column as a subclass of BeInt64Id
        //! @remarks As @ref ECInstanceId "ECInstanceIds" are BeInt64Ids, you can use
        //! this method to get ECInstanceId values.
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return BeInt64Id value
        //! @note Possible errors:
        //! - column data does not hold a BeInt64Id
        template <class TBeInt64Id>
        TBeInt64Id GetValueId(int columnIndex) const { return TBeInt64Id(GetValueUInt64(columnIndex)); }

        //! Gets the value of the specific column as a BeGuid
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @return BeGuid value
        //! @note Possible errors:
        //! - column data does not hold a BeGuid
        BeGuid GetValueGuid(int columnIndex) const { return GetValue(columnIndex).GetGuid(); }

        //! Gets the value as a NavigationECProperty value
        //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
        //! @param[out] relationshipECClassId ECClassId of the ECRelationshipClass used to navigate to the related ECInstance.
        //!             You can pass nullptr to this parameter if you don't want it to be returned
        //! @return ECInstanceId of the related ECInstance
        //! @note Possible errors:
        //! - column does not refer to a NavigationECProperty
        template <class TBeInt64Id>
        TBeInt64Id GetValueNavigation(int columnIndex, ECN::ECClassId* relationshipECClassId = nullptr) const { return GetValue(columnIndex).GetNavigation<TBeInt64Id>(relationshipECClassId); }

        //! Gets the value of the specified column.
        //! @remarks This is the generic way of getting the value of a specified column in the result set.
        //! All other GetValueXXX methods are convenience methods around GetValue.
        //! @return Value for the column
        //! @note Possible errors:
        //! - @p columnIndex is out of bounds
        ECDB_EXPORT IECSqlValue const& GetValue(int columnIndex) const;

        //! @}

#if !defined (DOCUMENTATION_GENERATOR)
    //! Gets the ECSQL statement string
    //! @remarks Results in an error if the ECSqlStatement has not been prepared yet
    //! @return ECSQL statement string
        ECDB_EXPORT Utf8CP GetECSql() const;

        //! Gets the SQLite SQL statement to which this ECSQL statement was translated to internally.
        //! @remarks Results in an error if the ECSqlStatement has not been prepared yet
        //! @return SQLite SQL statement
        ECDB_EXPORT Utf8CP GetNativeSql() const;

        //! Gets the ECDb handle which was used to prepare this statement.
        //! @remarks Only call this for a prepared statement!
        //! @return ECDb handle used to prepare this statement or nullptr if statement is not prepared.
        ECDB_EXPORT ECDb const* GetECDb() const;

        //! Gets Db handle use to prepare internal sqlite statement(s).
        //! @remarks Only call this for a prepared statement!
        //! @return Db handle prepare
        ECDB_EXPORT Db const* GetDataSourceDb() const;

        //! Return hash code for the ECSql
        //! @remarks Can be safely use to see if two ECSqlStatement have been prepared with same ECSql. Its case-insensitive hash of ECSql text. Its faster then using GetECSql() to compare to ECSqlStatements.
        //! @return Runtime hash which can change depending on platform and should be not persisted on disk for any later comparision or indexing.
        ECDB_EXPORT uint64_t GetHashCode() const;

        //! Return hash code for the ECSql
        //! @remarks Can be safely use to see if two ECSqlStatement have been prepared with same ECSql. Its case-insensitive hash of ECSql text. Its faster then using GetECSql() to compare to ECSqlStatements.
        //! @return Runtime hash which can change depending on platform and should be not persisted on disk for any later comparision or indexing.
        ECDB_EXPORT static uint64_t GetHashCode(Utf8CP ecsql);

#endif
    };

typedef ECSqlStatement const& ECSqlStatementCR;

struct ECSqlStatementCache;

//=======================================================================================
//! A reference-counted ECSqlStatement. ECSqlStatement is freed when last reference is released.
//! @remarks Its main purpose is to be used in a statement cache. Caching a statement allows to reuse an already prepared
//! statement and therefore avoids the cost of preparing the statement multiple times.
//! However be aware that holding on to a statement also comes at a cost (higher memory footprint), so applications
//! must consciously decide which statements need to be cached and which not, i.e. optimize
//! between the preparation cost and the memory cost.
//! Implementing a cache that holds CachedECSqlStatements is left to applications as
//! there are a lot of different ways how a cache should work - with an important criterion
//! being the desired lifetime of a statement in the cache.
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CachedECSqlStatement final : ECSqlStatement
    {
    friend struct ECSqlStatementCache;

    private:
        mutable BeAtomic<uint32_t> m_refCount;
        bool m_isInCache = true;
        ECDbCR m_ecdb;
        Db const* m_dataSourceECDb = nullptr;
        ECCrudWriteToken const* m_crudWriteToken = nullptr;
        ECSqlStatementCache const& m_cache;

        CachedECSqlStatement(ECSqlStatementCache const& cache, ECDbCR ecdb, Db const* dataSourceECDb, ECCrudWriteToken const* crudWriteToken)
            : ECSqlStatement(), m_cache(cache), m_ecdb(ecdb), m_dataSourceECDb(dataSourceECDb), m_crudWriteToken(crudWriteToken) {}

    public:
        DEFINE_BENTLEY_NEW_DELETE_OPERATORS

        ~CachedECSqlStatement() {}

        //! CachedECSqlStatements can never be finalized externally. That will corrupt the cache.
        void Finalize() = delete;

        uint32_t AddRef() const { return m_refCount.IncrementAtomicPre(); }
        uint32_t GetRefCount() const { return m_refCount.load(); }
        ECDB_EXPORT uint32_t Release();
    };

typedef RefCountedPtr<CachedECSqlStatement> CachedECSqlStatementPtr;

//=======================================================================================
//! A cache of shared ECSqlStatements that can be reused without re-preparing.
//! It can be very expensive to prepare an ECSQL statement,
//! so this class provides a way to save previously prepared statements for reuse (note, a prepared ECSqlStatement is specific to a
//! particular ECDb file)
//! The size of the cache is determined by the caller. The cache releases the
//! oldest statement when a new entry is added to a full cache.
//! @note Clients must make sure to release any cached statement and the cache itself
//! before the corresponding ECDb file is closed.
//!
//! ### Cache usage diagnostics
//! As clients can only indirectly control the lifetime of statements in the cache, diagnostics can help
//! applications analyze how often statements get popped out of the cache and later readded again.This only run in a debug build. To enable
//! ECSqlStatement cache diagnostics:
//!   name <b>Diagnostics.ECSqlStatement.Cache</b> and assign it the log severity @c @b TRACE.
//! With that enabled, %ECDb logs when a new statement was added to the cache and an existing one was removed from it.
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECSqlStatementCache final
    {
    private:
        friend struct CachedECSqlStatement;

        mutable BeMutex m_mutex;
        Utf8String m_name;
        mutable std::list<CachedECSqlStatementPtr> m_entries;
        uint32_t m_maxSize;

        //not copyable
        ECSqlStatementCache(ECSqlStatementCache const&) = delete;
        ECSqlStatementCache& operator=(ECSqlStatementCache const&) = delete;

        CachedECSqlStatement* FindEntry(ECDbCR ecdb, DbCP datasource, ECCrudWriteToken const* crudWriteToken, Utf8CP ecsql) const; // Requires m_mutex locked
        CachedECSqlStatementPtr AddStatement(CachedECSqlStatementPtr&, ECDbCR, DbCP datasource, ECCrudWriteToken const* token, Utf8CP ecsql) const; // Requires m_mutex locked
        void GetPreparedStatement(CachedECSqlStatementPtr&, ECDbCR ,DbCP, ECCrudWriteToken const*, Utf8CP, bool logPrepareErrors, ECSqlStatus* outPrepareStatus) const;
    public:
        //! Initializes a new ECSqlStatementCache of the specified size.
        //! @param [in] maxSize Maximum number of statements the cache can hold. If a new statement is added
        //! to a full cache, the oldest statement is removed.
        //! @param [in] name Name for the cache in case apps use multiple caches and need to tell between them
        ECDB_EXPORT explicit ECSqlStatementCache(uint32_t maxSize, Utf8CP name = nullptr);
        ~ECSqlStatementCache() { Empty(); }

        //! Gets a cached and prepared statement for the specified ECSQL.
        //! If there was no statement in the cache for the ECSQL, a new one will be prepared and cached.
        //! Otherwise an existing ready-to-use statement will be returned, i.e. clients neither need to call
        //! ECSqlStatement::Reset nor ECSqlStatement::ClearBindings on it.
        //! @param [in] ecdb ECDb file
        //! @param [in] ecsql ECSQL string for which to return a prepared statement
        //! @param[in] logPrepareErrors It determines when attempt to prepare statement, if to log errors or not
        //! @param[out] prepareStatus Optional statement prepare status only set if the statement needed to be prepared.
        //! @return Prepared and ready-to-use statement or nullptr in case of preparation or other errors
        CachedECSqlStatementPtr GetPreparedStatement(ECDbCR ecdb, Utf8CP ecsql, bool logPrepareErrors = true, ECSqlStatus* prepareStatus = nullptr) const
            {
            return GetPreparedStatement(ecdb, ecsql, nullptr, logPrepareErrors, prepareStatus);
            }

        //! Gets a cached and prepared statement for the specified ECSQL.
        //! If there was no statement in the cache for the ECSQL, a new one will be prepared and cached.
        //! Otherwise an existing ready-to-use statement will be returned, i.e. clients neither need to call
        //! ECSqlStatement::Reset nor ECSqlStatement::ClearBindings on it.
        //! @param [in] ecdb ECDb file
        //! @param [in] ecsql ECSQL string for which to return a prepared statement
        //! @param [in] token Token required to execute ECSQL INSERT, UPDATE, DELETE statements if
        //! the ECDb file was set-up with the "require ECSQL write token" option.
        //! If the option is not set, nullptr can be passed for @p token.
        //! @param[in] logPrepareErrors It determines when attempt to prepare statement, if to log errors or not
        //! @param[out] prepareStatus Optional statement prepare status only set if the statement needed to be prepared.
        //! @return Prepared and ready-to-use statement or nullptr in case of preparation or other errors
        ECDB_EXPORT CachedECSqlStatementPtr GetPreparedStatement(ECDbCR ecdb, Utf8CP ecsql, ECCrudWriteToken const* token, bool logPrepareErrors = true, ECSqlStatus* prepareStatus = nullptr) const;

        //! Gets a cached and prepared statement for the specified SELECT ECSQL.
        //! @remarks
        //! This overload is for multi-threading scenarios. See
        //! @ref BentleyApi::BeSQLite::EC::ECSqlStatement::Prepare(SchemaManager const&, Db const&, Utf8CP) "ECSqlStatement::Prepare(SchemaManager const&, Db const&, Utf8CP)"
        //! for details.
        //!
        //! If there was no statement in the cache for the ECSQL, a new one will be prepared and cached.
        //! Otherwise an existing ready-to-use statement will be returned, i.e. clients neither need to call
        //! ECSqlStatement::Reset nor ECSqlStatement::ClearBindings on it.
        //! @see BentleyApi::BeSQLite::EC::ECSqlStatement::Prepare(SchemaManager const&, Db const&, Utf8CP)
        //! @param[in] schemaManager SchemaManager that is to be used to parse the ECSQL. e.g. as returned from  @ref BentleyApi::BeSQLite::EC::ECDb::Schemas() "ECDb::Schemas()"
        //! @param[in] dataSourceECDb Connection to the same ECDb file which is to be used to execute ECSqlStatment. Must be read-only, but can be in a
        //! another thread than @p schemaManager
        //! @param[in] selectECSql SELECT ECSQL
        //! @param[in] logPrepareErrors It determines when attempt to prepare statement, if to log errors or not
        //! @param[out] prepareStatus Optional statement prepare status only set if the statement needed to be prepared.
        //! @return Prepared and ready-to-use statement or nullptr in case of preparation or other errors
        ECDB_EXPORT CachedECSqlStatementPtr GetPreparedStatement(SchemaManagerCR schemaManager, DbCR dataSourceECDb, Utf8CP selectECSql, bool logPrepareErrors = true, ECSqlStatus* prepareStatus = nullptr) const;

        //! Returns whether the cache is currently empty or not.
        //! @return true if cache is empty, false otherwise
        bool IsEmpty() const { return m_entries.empty(); }
        //! Returns number of currently cached statements
        //! @return Number of currently cached statements
        uint32_t Size() const { return (uint32_t) m_entries.size(); }
        //! Empties the cache, thus releasing any cached statements
        ECDB_EXPORT void Empty();

        //! Gets the name of the cache
        //! @return Cache's name or nullptr if not set
        Utf8CP GetName() const { return m_name.c_str(); }

        //! Logs the ECSQL strings of the currently cached ECSqlStatements.
        ECDB_EXPORT void Log() const;
    };

#if !defined (DOCUMENTATION_GENERATOR)
//=======================================================================================
//! Helper that parses an ECSQL and formats the result to a string.
//=======================================================================================
struct ECSqlParseTreeFormatter final
    {
    private:
        ECSqlParseTreeFormatter();
        ~ECSqlParseTreeFormatter();

    public:
        ECDB_EXPORT static BentleyStatus ParseAndFormatECSqlParseNodeTree(Utf8StringR parseNodeTree, ECDbCR, Utf8CP ecsql);
        //!@param[out] expTree Expression tree as JSON value of this format:
        //!                    {"Exp":"<exp as string>",
        //!                     "Children": [{"Exp":"<exp as string>",
        //!                                   "Children": [....]},
        //!                                   {"Exp":"<exp as string>",
        //!                                   "Children": [....]},
        //!                                   ...]}
        ECDB_EXPORT static BentleyStatus ParseAndFormatECSqlExpTree(Json::Value& expTree, Utf8StringR ecsqlFromExpTree, ECDbCR, Utf8CP ecsql);
    };

#endif

END_BENTLEY_SQLITE_EC_NAMESPACE
