/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/ECDb/ECSqlStatement.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeSQLite/ECDb/IECSqlValue.h>
#include <BeSQLite/ECDb/IECSqlBinder.h>
#include <BeSQLite/ECDb/ECInstanceId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECSqlStatement Step API status values.
//! @ingroup ECDbGroup
// @bsienum                                                      Krischan.Eberle  05/2013
//+===============+===============+===============+===============+===============+======
enum class ECSqlStepStatus
    {
    HasRow, //!< ECSqlStatement::Step returned data which is ready to be processed by the caller.
    Done, //!< ECSqlStatement::Step reached the end of the result set. No further data can be processed by the caller.
    Error //!< ECSqlStatement::Step failed.
    };


//=======================================================================================
//! ECSqlEventArgs for the ECSQL events fired when executing an ECSqlStatement
//! @see ECSqlEventHandler
//! @see ECSqlStatement::RegisterEventHandler
//! @ingroup ECDbGroup
// @bsiclass                                                      Affan.Khan    06/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlEventArgs : NonCopyableClass
    {
public:
    typedef bvector<ECInstanceKey> ECInstanceKeyList;

private:
    ECInstanceKeyList m_instanceKeyList;

//__PUBLISH_CLASS_VIRTUAL__

public:
//__PUBLISH_SECTION_END__
    //! Initializes a new ECSqlEventArgs object.
    ECSqlEventArgs ();
    //__PUBLISH_SECTION_START__

    ECDB_EXPORT ~ECSqlEventArgs ();

    //! Returns the keys of the ECInstances affected by the event
    //! @return List of keys of the ECInstances affected 
    ECDB_EXPORT ECInstanceKeyList const& GetInstanceKeys () const;

    //__PUBLISH_SECTION_END__
    ECInstanceKeyList& GetInstanceKeysR ();
    //__PUBLISH_SECTION_START__
    };

//=======================================================================================
//! ECSqlEventHandler allows clients to listen to the instances affected by executing
//! an ECSqlStatement.
//! This is the abstract base class for client handlers. Clients register the handlers
//! via ECSqlStatement::RegisterEventHandler.
//! @ingroup ECDbGroup
// @bsiclass                                                      Affan.Khan    06/2014
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECSqlEventHandler
    {
public:
    //=======================================================================================
    //! Type of the event fired when executing an ECSqlStatement
    // @bsiclass                                                          Affan.Khan  07/2014
    //+===============+===============+===============+===============+===============+======
    enum class EventType
        {
        Insert, //!< When an ECSQL INSERT statement was executed
        Update, //!< When an ECSQL UPDATE statement was executed
        Delete //!< When an ECSQL DELETE statement was executed
        };

private:
    //! Call back called by ECSqlStatement when an event was fired.
    //! @param[in] eventType Type of the event fired
    //! @param[in] args EventArgs that contain changeset of the changes 
    //! caused or will be caused by last CRUD operation.
    ECDB_EXPORT virtual void _OnEvent (EventType eventType, ECSqlEventArgs const& args) = 0;

protected:
    //! Initializes a new ECSqlEventHandler object.
    ECDB_EXPORT ECSqlEventHandler ();

public:
    ECDB_EXPORT virtual ~ECSqlEventHandler ();

    //__PUBLISH_SECTION_END__
    //! Calls the handler (only used by ECDb).
    //! @param[in] eventType Type of the event fired
    //! @param[in] args EventArgs that contain changeset of the changes caused or will be caused by last CRUD operation.
    void OnEvent (EventType eventType, ECSqlEventArgs const& args);
    //__PUBLISH_SECTION_START__
    };

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
struct ECSqlStatement : NonCopyableClass
    {
public:
    struct Impl;

private:
    Impl* m_pimpl;

public:
    //! Initializes a new unprepared ECSqlStatement.
    ECDB_EXPORT ECSqlStatement ();
    //! Destroys the ECSqlStatement and all internal resources.
    ECDB_EXPORT ~ECSqlStatement ();
    //! Move constructor
    ECDB_EXPORT ECSqlStatement (ECSqlStatement&& rhs);
    //! Move assignment operator
    ECDB_EXPORT ECSqlStatement& operator= (ECSqlStatement&& rhs);

    //! Finalizes this ECSqlStatement, i.e. frees any internal resources.
    //! @remarks Before calling ECDb::CloseDb explicitly make sure that all prepared ECSqlStatements
    //! are finalized.
    //! After having called Finalize, the ECSqlStatement can be re-prepared.
    ECDB_EXPORT void Finalize ();

    //! Prepares the statement with the specified ECSQL
    //! @param[in] ecdb ECDb context
    //! @param[in] ecsql ECSQL
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus Prepare (ECDbR ecdb, Utf8CP ecsql);

    //! Indicates whether this statement is already prepared or not.
    //! @return true, if it is prepared. false otherwise
    ECDB_EXPORT bool IsPrepared () const;
    
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
    ECDB_EXPORT ECSqlStatus BindNull (int parameterIndex);

    //! Binds a boolean value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindBoolean (int parameterIndex, bool value);

    //! Binds a binary / BLOB value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @param[in] binarySize Size of the BLOB in bytes
    //! @param[in] makeCopy Flag that indicates whether a private copy of the blob is done or not. Only pass 
    //! IECSqlBinder::MakeCopy::No if @p value remains valid until
    //!            the statement's bindings are cleared.
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindBinary (int parameterIndex, const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy);

    //! Binds a DateTime value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindDateTime (int parameterIndex, DateTimeCR value);

    //! Binds a double value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindDouble (int parameterIndex, double value);

    //! Binds an IGeometry value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindGeometry (int parameterIndex, IGeometryCR value);

    //! Binds a 32-bit integer value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindInt (int parameterIndex, int value);

    //! Binds a 64-bit integer value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindInt64 (int parameterIndex, int64_t value);

    //! Binds a Point2D value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindPoint2D (int parameterIndex, DPoint2dCR value);

    //! Binds a Point3D value to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindPoint3D (int parameterIndex, DPoint3dCR value);

    //! Binds a UTF-8 encoded string to the parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @param[in] makeCopy indicates whether ECSqlStatement should make a private copy of @p value or not.
    //!             Only pass IECSqlBinder::MakeCopy::No if @p value will remain valid until the statement's bindings are cleared.
    //! @param[in] byteCount Number of bytes (not characters) in @p value. If negative, it will be calculated from value. Passing this value is only an optimization. 
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindText (int parameterIndex, Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount = -1);

    //! Binds a BeRepositoryBasedId to the parameter
    //! @remarks As @ref ECInstanceId "ECInstanceIds" are repository-based ids, you can use
    //! this method to bind them to a parameter.
    //! @param[in] parameterIndex Parameter index
    //! @param[in] value Value to bind
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus BindId (int parameterIndex, BeRepositoryBasedId value);

    //! Gets a binder which is used to bind a struct value to the specified parameter
    //! @param[in] parameterIndex Parameter index
    //! @remarks In case of error, e.g. if the parameter is not a struct, a no-op binder will be returned. Calling methods on the no-op binder
    //! returns the appropriate error-code.
    //! @return Struct parameter binder
    ECDB_EXPORT IECSqlStructBinder& BindStruct (int parameterIndex);

    //! Gets a binder which is used to bind an array to the specified parameter
    //! @param[in] parameterIndex Parameter index
    //! @param[in] initialArrayCapacity Initial capacity of the array to bind. 
    //! @remarks In case of error, e.g. if the parameter
    //! is not an array, a no-op binder will be returned. Calling methods on the no-op binder
    //! returns the appropriate error-code.
    //! @return Array parameter binder
    ECDB_EXPORT IECSqlArrayBinder& BindArray (int parameterIndex, uint32_t initialArrayCapacity);
    
    //! Gets a binder to bind a value to the parameter at the specified index.
    //! @param[in] parameterIndex Parameter index
    //! @remarks In case of error, e.g. if the parameter index is out of bounds
    //! a no-op binder will be returned. Calling methods on the no-op binder
    //! returns the appropriate error-code.
    //! @return Parameter binder
    ECDB_EXPORT IECSqlBinder& GetBinder (int parameterIndex);

    //! Gets the parameter index for a named parameter
    //! 
    //! @e Example
    //!  ECSqlStatement statement;
    //!  statement.Prepare (ecdb, "SELECT * FROM myschema.Foo Where ECInstanceId = :id");
    //!  ECSqlStatus stat = statement.BindInt64 (statement.GetParameterIndex ("id"), 123);
    //!
    //! @param[in] parameterName Name of the binding parameter
    //! @return Parameter index
    ECDB_EXPORT int GetParameterIndex (Utf8CP parameterName) const;

    //! Resets all parameter bindings
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus ClearBindings ();

    //! @}

    //! Perform a single step on this statement
    //! @remarks For select statements, ECSqlStepStatus::HasRow indicates that data (a row) is returned which is ready to be 
    //!          processed by the caller, and ECSqlStepStatus::Done is returned after a
    //!          successful execution of the step. For non-select statements (Insert, Update, Delete) ECSqlStepStatus::Done is always 
    //!          returned in case of success and error codes in case of error.
    //!          When ECSqlStepStatus::Done was returned, Step should not be called again.
    //! @return ECSqlStepStatus::HasRow if Step returned data which is ready to be processed by the caller. 
    //!         ECSqlStepStatus::Done if the reader has finished executing successfully.
    //!         Error codes in case of errors.
    ECDB_EXPORT ECSqlStepStatus Step ();

    //! Perform a single step on this (previously prepared) @b insert statement
    //! @remarks This overload is intended for insert statements only as it returns the ECInstanceKey of the inserted row.
    //! @param[out] ecInstanceKey The ECInstanceKey of the inserted row.
    //! @return ECSqlStepStatus::Done if the statement has finished executing successfully.
    //!         Error codes in case of errors.
    ECDB_EXPORT ECSqlStepStatus Step (ECInstanceKey& ecInstanceKey) const;

    //! Resets the statement, so that it can be reiterated.
    //! @remarks Reset does not clear the bindings of the statement. Call ECSqlStatement::ClearBindings to do that.
    //! @return ECSqlStatus::Success or error codes
    ECDB_EXPORT ECSqlStatus Reset ();

    //! Gets the number of ECSQL columns in the result set returned after calling Step on a SELECT statement.
    //! @return Number of ECSQL columns in the result set
    ECDB_EXPORT int GetColumnCount () const;

    //! @name Get metadata for a given column in the result set
    //! @{

    //! Gets the metadata about the specified ECSQL column in the result set.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return ECSQL column metadata.
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT ECSqlColumnInfoCR GetColumnInfo (int columnIndex) const;

    //! @}

    //! @name Get values for a given column in the result set
    //! @{

    //! Indicates whether the value for the column at the given index in the result set is %NULL or not.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return true if column value is %NULL, false otherwise
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT bool IsValueNull (int columnIndex) const;

    //! Gets the binary / blob value of the specific column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @param[out] binarySize the size of the blob in bytes.
    //! @return The binary value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not binary
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT void const* GetValueBinary (int columnIndex, int* binarySize = nullptr) const;

    //! Gets the value of the specific column as a boolean value.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Column value as boolean
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT bool GetValueBoolean (int columnIndex) const;

    //! Gets the DateTime value of the specific column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return DateTime value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not DateTime
    //! - @p columnIndex is out of bounds
    //! @see @ref ECDbCodeSampleECSqlStatementAndDateTimeProperties
    ECDB_EXPORT DateTime GetValueDateTime (int columnIndex) const;

    //! Gets the value of the specific column as a double value.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Column value as double
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT double GetValueDouble (int columnIndex) const;

    //! Gets the value of the specific column as an IGeometry value.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Column value as IGeometry
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not IGeometry
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT IGeometryPtr GetValueGeometry (int columnIndex) const;

    //! Gets the value of the specific column as an integer value.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Column value as integer
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT int GetValueInt (int columnIndex) const;

    //! Gets the value of the specific column as an Int64 value.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Column value as Int64
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT int64_t GetValueInt64 (int columnIndex) const;

    //! Gets the Point2D value of the specific column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Point2D column value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not Point2D
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT DPoint2d GetValuePoint2D (int columnIndex) const;

    //! Gets the Point3D value of the specific column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Point3D column value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not Point3D
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT DPoint3d GetValuePoint3D (int columnIndex) const;

    //! Gets the value of the specific column as a string value.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Column value as string
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column is not of one of the basic primitive types (boolean, integer, long, double, string). Only 
    //!   those types can implicitly be converted into each other.
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT Utf8CP GetValueText (int columnIndex) const;

    //! Gets the value as a BeRepositoryBasedId
    //! @remarks As @ref ECInstanceId "ECInstanceIds" are repository-based ids, you can use
    //! this method to get ECInstanceId values.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return BeRepositoryBasedId value
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data does not hold a repository-based id
    template <class TBeRepositoryBasedId>
    TBeRepositoryBasedId GetValueId (int columnIndex) const
        {
        return TBeRepositoryBasedId (GetValueInt64 (columnIndex));
        }

    //! Gets the array value of the specified column.
    //! @param[in] columnIndex Index of ECSQL column in result set (0-based)
    //! @return Array reader for the column
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not an array
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT IECSqlArrayValue const& GetValueArray (int columnIndex) const;

    //! Gets the struct value of the specified column.
    //! @return Struct value for the column
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - column data type is not an ECStruct
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT IECSqlStructValue const& GetValueStruct (int columnIndex) const;

    //! Gets the value of the specified column.
    //! @remarks This is the generic way of getting the value of a specified column in the result set. 
    //! All other GetValueXXX methods are convenience methods around GetValue.
    //! @return Value for the column
    //! @note See @ref ECSqlStatementErrorReporting for how to detect errors when calling this method.
    //! @note Possible errors:
    //! - @p columnIndex is out of bounds
    ECDB_EXPORT IECSqlValue const& GetValue (int columnIndex) const;

    //! @}

    //! @name Get status of last operation
    //! @{

    //! Gets the status returned from the last call to a method of ECSqlStatement.
    //! If the last call to ECSqlStatement was successful, ECSqlStatus::Success will be returned.
    //! @return Status from last call to ECSqlStatement
    //! @see @ref ECSqlStatementErrorReporting
    ECDB_EXPORT ECSqlStatus GetLastStatus () const;

    //! Gets the status message for the status returned from the last call to a method of ECSqlStatement.
    //! If the last call to ECSqlStatement was successful, i.e. return ECSqlStatus::Success, the status message
    //! will correspond to the successful call (and will therefore usually be empty). 
    //! @return Status message from last call to ECSqlStatement
    //! @see @ref ECSqlStatementErrorReporting
    ECDB_EXPORT Utf8String GetLastStatusMessage () const;
    //! @}

    //! @name Event Handling
    //! @{

    //! Registers an ECSqlEventHandler.
    //! @remarks ECSqlEventHandler provide a way for clients to listen to changes performed by executing the ECSqlStatement.
    //! @note ECSqlStatement does not take over ownership of the handler. Callers are responsible for ensuring
    //! that the handler remains valid as long as it is registered with the ECSqlStatement.
    //! @param[in] handler EventHandler to register
    //! @return ECSqlStatus::Success if registration was successful. Error codes otherwise
    ECDB_EXPORT ECSqlStatus RegisterEventHandler (ECSqlEventHandler& handler);

    //! Unregisters a ECSqlEventHandler.
    //! @param[in] handler Removes the given handler from the statement.
    //! @return ECSqlStatus::Success if unregistration was successful. Error codes otherwise
    ECDB_EXPORT ECSqlStatus UnregisterEventHandler (ECSqlEventHandler& handler);


    //! Unregisters all event handlers.
    //! @return ECSqlStatus::Success if unregistration was successful. Error codes otherwise
    ECDB_EXPORT ECSqlStatus UnregisterAllEventHandlers ();
    //! @}

//__PUBLISH_SECTION_END__

    //! Gets the ECSQL statement string
    //! @remarks Results in an error if the ECSqlStatement has not been prepared yet
    //! @return ECSQL statement string
    //! @see @ref ECSqlStatementErrorReporting
    ECDB_EXPORT Utf8CP GetECSql () const;

    //! Gets the SQLite SQL statement to which this ECSQL statement was translated to internally.
    //! @remarks Results in an error if the ECSqlStatement has not been prepared yet
    //! @return SQLite SQL statement
    //! @see @ref ECSqlStatementErrorReporting
    ECDB_EXPORT Utf8CP GetNativeSql () const;

    //! Gets the ECDb handle which was used to prepare this statement.
    //! @remarks Only call this for a prepared statement!
    //! @return ECDb handle used to prepare this statement or nullptr if statement is not prepared.
    ECDbP GetECDb () const;
//__PUBLISH_SECTION_START__
    };

typedef ECSqlStatement const& ECSqlStatementCR;

//__PUBLISH_SECTION_END__

//=======================================================================================
//! Unpublished helper that parses an ECSQL and formats the result to a string.
//=======================================================================================
struct ECSqlParseTreeFormatter
    {
private:
    ECSqlParseTreeFormatter ();
    ~ECSqlParseTreeFormatter ();

public:
    ECDB_EXPORT static bool ParseAndFormatECSqlParseNodeTree (Utf8StringR parseNodeTree, Utf8StringR error, Utf8CP ecsql);
    ECDB_EXPORT static bool ParseAndFormatECSqlExpTree (Utf8StringR expTree, Utf8StringR expTreeToECSql, Utf8StringR error, Utf8CP ecsql, ECDbR db);
    };

//__PUBLISH_SECTION_START__

END_BENTLEY_SQLITE_EC_NAMESPACE
