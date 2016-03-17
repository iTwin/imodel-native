/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECSqlStatement.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/IECSqlValue.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/ECInstanceId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECSqlStatement is used to perform Create, Read, Update, Delete operations (@b CRUD) 
//! against @b ECInstances in an @ref ECDbFile "ECDb file".
//!
//! See @ref ECSqlStatementOverview for details on how to use ECSqlStatement.
//!
//! @see @ref ECSqlStatementOverview, @ref ECDbOverview, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
//! @nosubgrouping
// @bsiclass                                                 Krischan.Eberle    05/2013
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECSqlStatement : NonCopyableClass
    {
public:
    struct Impl;

private:
    Impl* m_pimpl;

public:
    //! Initializes a new unprepared ECSqlStatement.
    ECDB_EXPORT ECSqlStatement();
    //! Destroys the ECSqlStatement and all internal resources.
    ECDB_EXPORT virtual ~ECSqlStatement();
    //! Move constructor
    ECDB_EXPORT ECSqlStatement(ECSqlStatement&& rhs);
    //! Move assignment operator
    ECDB_EXPORT ECSqlStatement& operator= (ECSqlStatement&& rhs);

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
    ECDB_EXPORT ECSqlStatus Prepare(ECDbCR ecdb, Utf8CP ecsql);

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
    //!     statement.Prepare (ecdb, "SELECT * FROM myschema.Foo WHERE MyProp <> ?");
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

    //! Binds a binary / BLOB value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @param[in] binarySize Size of the BLOB in bytes
    //! @param[in] makeCopy Flag that indicates whether a private copy of the blob is done or not. Only pass 
    //! IECSqlBinder::MakeCopy::No if @p value remains valid until
    //!            the statement's bindings are cleared.
    //! @return ECSqlStatus::Success or error codes
    ECSqlStatus BindBinary(int parameterIndex, const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) { return GetBinder(parameterIndex).BindBinary(value, binarySize, makeCopy); }

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

    //! Binds a 64-bit integer value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECSqlStatus BindInt64(int parameterIndex, int64_t value) { return GetBinder(parameterIndex).BindInt64(value); }

    //! Binds a Point2D value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECSqlStatus BindPoint2D(int parameterIndex, DPoint2dCR value) { return GetBinder(parameterIndex).BindPoint2D(value); }

    //! Binds a Point3D value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECSqlStatus BindPoint3D(int parameterIndex, DPoint3dCR value) { return GetBinder(parameterIndex).BindPoint3D(value); }

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

    //! Gets a binder which is used to bind a struct value to the specified parameter
    //! @param[in] parameterIndex Parameter index
    //! @remarks In case of error, e.g. if the parameter is not a struct, a no-op binder will be returned. Calling methods on the no-op binder
    //! returns the appropriate error-code.
    //! @return Struct parameter binder
    IECSqlStructBinder& BindStruct(int parameterIndex) { return GetBinder(parameterIndex).BindStruct(); }

    //! Gets a binder which is used to bind an array to the specified parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] initialArrayCapacity Initial capacity of the array to bind. 
    //! @remarks In case of error, e.g. if the parameter
    //! is not an array, a no-op binder will be returned. Calling methods on the no-op binder
    //! returns the appropriate error-code.
    //! @return Array parameter binder
    IECSqlArrayBinder& BindArray(int parameterIndex, uint32_t initialArrayCapacity) { return GetBinder(parameterIndex).BindArray(initialArrayCapacity); }
    
    //! Gets a binder to bind a value to the parameter at the specified index.
    //! @param[in] parameterIndex Parameter index
    //! @remarks In case of error, e.g. if the parameter index is out of bounds
    //! a no-op binder will be returned. Calling methods on the no-op binder
    //! returns the appropriate error-code.
    //! @return Parameter binder
    ECDB_EXPORT IECSqlBinder& GetBinder(int parameterIndex);

    //! Gets the parameter index for a named parameter
    //! 
    //! @e Example
    //!  ECSqlStatement statement;
    //!  statement.Prepare (ecdb, "SELECT * FROM myschema.Foo Where ECInstanceId = :id");
    //!  ECSqlStatus stat = statement.BindInt64 (statement.GetParameterIndex ("id"), 123);
    //!
    //! @param[in] parameterName Name of the binding parameter
    //! @return Parameter index
    ECDB_EXPORT int GetParameterIndex(Utf8CP parameterName) const;

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

    //! Gets the binary / blob value of the specific column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @param[out] binarySize the size of the blob in bytes.
    //! @return The binary value
    //! @note Possible errors:
    //! - column data type is not binary
    //! - @p columnIndex is out of bounds
    void const* GetValueBinary(int columnIndex, int* binarySize = nullptr) const { return GetValue(columnIndex).GetBinary(binarySize); }

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

    //! Gets the Point2D value of the specific column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Point2D column value
    //! @note Possible errors:
    //! - column data type is not Point2D
    //! - @p columnIndex is out of bounds
    DPoint2d GetValuePoint2D (int columnIndex) const { return GetValue(columnIndex).GetPoint2D(); }

    //! Gets the Point3D value of the specific column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Point3D column value
    //! @note Possible errors:
    //! - column data type is not Point3D
    //! - @p columnIndex is out of bounds
    DPoint3d GetValuePoint3D (int columnIndex) const { return GetValue(columnIndex).GetPoint3D(); }

    //! Gets the value of the specific column as a string value.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Column value as string
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    //! - @p columnIndex is out of bounds
    Utf8CP GetValueText(int columnIndex) const { return GetValue(columnIndex).GetText(); }

    //! Gets the value as a subclass of BeInt64Id
    //! @remarks As @ref ECInstanceId "ECInstanceIds" are BeInt64Ids, you can use
    //! this method to get ECInstanceId values.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return BeInt64Id value
    //! @note Possible errors:
    //! - column data does not hold a BeInt64Id
    template <class TBeInt64Id> TBeInt64Id GetValueId(int columnIndex) const
        {
        return TBeInt64Id(GetValueUInt64(columnIndex));
        }

    //! Gets the array value of the specified column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Array reader for the column
    //! @note Possible errors:
    //! - column data type is not an array
    //! - @p columnIndex is out of bounds
    IECSqlArrayValue const& GetValueArray(int columnIndex) const { return GetValue(columnIndex).GetArray(); }

    //! Gets the struct value of the specified column.
    //! @return Struct value for the column
    //! @note Possible errors:
    //! - column data type is not an ECStruct
    //! - @p columnIndex is out of bounds
    IECSqlStructValue const& GetValueStruct(int columnIndex) const { return GetValue(columnIndex).GetStruct(); }

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
    ECDB_EXPORT ECDbCP GetECDb() const;
#endif
    };

typedef ECSqlStatement const& ECSqlStatementCR;

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
// @bsiclass                                                    Krischan.Eberle   02/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CachedECSqlStatement : ECSqlStatement
    {
private:
    mutable uint32_t m_refCount;

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    CachedECSqlStatement() : ECSqlStatement(), m_refCount(0) {}
    ~CachedECSqlStatement() {}

    uint32_t AddRef() const {return ++m_refCount;}
    uint32_t GetRefCount() const {return m_refCount;}
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
//! applications analyze how often statements get popped out of the cache and later readded again. To enable 
//! ECSqlStatement cache diagnostics:
//! - turn on the log4cxx based @ref BentleyApi::NativeLogging "Bentley logging"
//! - in the <b>log4cxx configuration</b> define a @b logger or a <b>logging category</b> with the
//!   name <b>Diagnostics.ECSqlStatement.Cache</b> and assign it the log severity @c @b TRACE.
//! With that enabled, %ECDb logs when a new statement was added to the cache and an existing one was removed from it.
// @bsiclass                                                    Krischan.Eberle   02/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ECSqlStatementCache : NonCopyableClass
    {
private:
    mutable BeDbMutex m_mutex;
    Utf8String m_name;
    mutable bvector<CachedECSqlStatementPtr> m_entries;
    size_t m_maxSize;

    CachedECSqlStatement* FindEntry(Utf8CP ecsql) const;
    CachedECSqlStatement* AddStatement(ECDbCR ecdb, Utf8CP ecsql) const;

public:
    //! Initializes a new ECSqlStatementCache of the specified size.
    //! @param [in] maxSize Maximum number of statements the cache can hold. If a new statement is added
    //! to a full cache, the oldest statement is removed.
    //! @param [in] name Name for the cache in case apps use multiple caches and need to tell between them
    ECDB_EXPORT explicit ECSqlStatementCache(size_t maxSize, Utf8CP name = nullptr);
    ~ECSqlStatementCache() {Empty();}
    
    //! Gets a cached and prepared statement for the specified ECSQL.
    //! If there was no statement in the cache for the ECSQL, a new one will be prepared and cached.
    //! Otherwise an existing ready-to-use statement will be returned, i.e. clients neither need to call 
    //! ECSqlStatement::Reset nor ECSqlStatement::ClearBindings on it.
    //! @param [in] ecdb ECDb file
    //!  @param [in] ecsql ECSQL string for which to return a prepared statement
    //! @return Prepared and ready-to-use statement or nullptr in case of preparation or other errors
    ECDB_EXPORT CachedECSqlStatementPtr GetPreparedStatement(ECDbCR ecdb, Utf8CP ecsql) const;

    //! Returns whether the cache is currently empty or not.
    //! @return true if cache is empty, false otherwise
    bool IsEmpty() const {return m_entries.empty();}
    //! Returns number of currently cached statements
    //! @return Number of currently cached statements
    size_t Size() const {return m_entries.size();}
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
//! Unpublished helper that parses an ECSQL and formats the result to a string.
//=======================================================================================
struct ECSqlParseTreeFormatter
    {
private:
    ECSqlParseTreeFormatter();
    ~ECSqlParseTreeFormatter();

public:
    ECDB_EXPORT static BentleyStatus ParseAndFormatECSqlParseNodeTree(Utf8StringR parseNodeTree, ECDbCR, Utf8CP ecsql);
    ECDB_EXPORT static BentleyStatus ParseAndFormatECSqlExpTree(Utf8StringR expTree, Utf8StringR expTreeToECSql, ECDbCR, Utf8CP ecsql);
    };

#endif

END_BENTLEY_SQLITE_EC_NAMESPACE
