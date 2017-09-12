/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/JsonAdapter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECSqlStatement.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//=======================================================================================
//! Adapts the rows returned by an ECSqlStatement to the JSON wire format. 
//! @see ECSqlStatement
//! @ingroup ECDbGroup
//! @bsiclass                                                 Ramanujam.Raman      08/2012
//+===============+===============+===============+===============+===============+======
struct JsonECSqlSelectAdapter final: NonCopyableClass
    {
    public:
        //=======================================================================================
        //! Options to control the format of the JSON returned by methods in the JsonECSqlSelectAdapter.
        // @bsiclass                                                Ramanujam.Raman      10/2012
        //+===============+===============+===============+===============+===============+======
        enum class FormatOptions
        {
        Default,
        LongsAreIds //!< ECProperties of type Long are treated as Ids and therefore formatted as Ids (i.e. as hex strings)
        };

    private:
        ECSqlStatementCR m_ecsqlStatement;
        FormatOptions m_formatOptions = FormatOptions::Default;

        BentleyStatus JsonFromPropertyValue(JsonValueR, IECSqlValue const&) const;
        BentleyStatus JsonFromCell(JsonValueR, IECSqlValue const&) const;

        BentleyStatus JsonFromNavigation(JsonValueR, IECSqlValue const&) const;
        BentleyStatus JsonFromStruct(JsonValueR, IECSqlValue const&) const;
        BentleyStatus JsonFromArray(JsonValueR, IECSqlValue const&, ECN::ECPropertyCR) const;
        BentleyStatus JsonFromStructArray(JsonValueR, IECSqlValue const&) const;
        BentleyStatus JsonFromPrimitiveArray(JsonValueR, IECSqlValue const&, ECN::ECPropertyCR) const;
        BentleyStatus JsonFromPrimitive(JsonValueR, IECSqlValue const&, ECN::ECPropertyCR) const;

        void JsonFromClassKey(JsonValueR, ECN::ECClassCR) const;
        BentleyStatus JsonFromInstanceId(JsonValueR, IECSqlValue const&) const;

        static Utf8String GetClassKey(ECN::ECClassCR);

    public:

        //! Initializes a new JsonECSqlSelectAdapter instance for the specified ECSqlStatement. 
        //! @param[in] ecsqlStatement Prepared ECSqlStatement
        //! @param[in] formatOptions Options to control the output. 
        //! @see ECSqlStatement
        JsonECSqlSelectAdapter(ECSqlStatement const& ecsqlStatement, FormatOptions formatOptions = FormatOptions::Default) : m_ecsqlStatement(ecsqlStatement), m_formatOptions(formatOptions) {}
        ~JsonECSqlSelectAdapter() {}

        //! Gets the entire current row as JSON
        //! @param [out] currentRow Property values for the current row.
        //! @return false if there was an error in retrieving values. true otherwise.
        //! @remarks The values are organized as an array of values, each representing an instance. 
        //!
        //! Summary information on instances can be obtained through special property keys starting
        //! with "$". e.g., $ECInstanceId.
        //!
        //! @see JsonECSqlSelectAdapter::GetRowInstance
        //! @code
        //! [
        //! {
        //!     "$ECInstanceId" : StringValue,
        //!     "$ECClassKey" : StringValue,
        //!     "PropertyName1": <ECJsonValue>
        //!     "PropertyName2": <ECJsonValue>
        //!     ...
        //! },
        //! {
        //!     "$ECInstanceId" : StringValue,
        //!     "$ECClassKey" : StringValue,
        //!     "PropertyName3": <ECJsonValue>
        //!     "PropertyName4": <ECJsonValue>
        //!     ...
        //! }
        //! ...
        //! ]
        //! }
        //! @endcode
        //! where ECJsonValue: NullValue | PrimitiveValue | StructValue | ArrayValue
        //! and where PrimitiveValue is formatted like this:
        //!     - Binary/Blob: Base64 string
        //!     - Boolean: true / false
        //!     - DateTime: JulianDay double
        //!     - Double: 3.5
        //!     - Integer:  2, 
        //!     - Long/Int64:
        //!         - @ref JsonECSqlSelectAdapter::FormatOptions::Default "FormatOptions::Default": as string because of Int64 JavaScript issues
        //!         - @ref JsonECSqlSelectAdapter::FormatOptions::LongsAreIds "FormatOptions::LongsAreIds": as hex string
        //!     - Geometry: Base64 string of FlatBuffer Blob of the Geometry
        //!     - Point2d: {"x" : 1.1, "y" : 2.2} 
        //!     - Point3d: {"x" : 1.1, "y" : 2.2, "z" : 3.3} 
        //!     - String: "Test"
        ECDB_EXPORT bool GetRow(JsonValueR currentRow) const;

        //! Gets a specific instance from the current row as JSON
        //! @param [out] ecJsonInstance JSON representation of the ECInstance as a property-value map. 
        //! @param [in] ecClassId ECClassId indicating the class of the instance needed from the current row 
        //! @return false if there was an error in retrieving values. true otherwise.
        //! @remarks The method returns only the instance of the type requested.
        //! The format of the JSON is very similar to that described in JsonECSqlSelectAdapter::GetRow except that there 
        //! is just one top level instance, instead of an array of instances. 
        //! @code
        //! {
        //!     "$ECInstanceId" : StringValue,
        //!     "PropertyName1": <ECJsonValue>
        //!     "PropertyName2": <ECJsonValue>
        //!     ...
        //! }
        //! @endcode
        //! @see JsonECSqlSelectAdapter::GetRow
        ECDB_EXPORT bool GetRowInstance(JsonValueR ecJsonInstance, ECN::ECClassId ecClassId) const;


        //! Gets the first instance from the current row as JSON
        //! @param [out] json JSON representation of the ECInstance as a property-value map. 
        //! @return false if there was an error in retrieving values. true otherwise.
        //! @remarks Uses the class of the first column, and thereafter ignores any columns that don't 
        //! belong to the same class. 
        //! The format of the JSON is very similar to that described in JsonECSqlSelectAdapter::GetRow except that there 
        //! is just one top level instance, instead of an array of instances. 
        //! @code
        //! [
        //! {
        //!     "$ECInstanceId" : StringValue,
        //!     "PropertyName1": <ECJsonValue>
        //!     "PropertyName2": <ECJsonValue>
        //!     ...
        //! }
        //! @endcode
        //! @see JsonECSqlSelectAdapter::GetRow
        ECDB_EXPORT bool GetRowInstance(JsonValueR json) const;

        //! Gets the current row in the ImodelJs JSON wire format.
        //! @param [out] json ImodelJs JSON wire format representation of value
        //! @return false if there was an error in retrieving values. true otherwise.
        ECDB_EXPORT bool GetRowForImodelJs(JsonValueR json);
    };

//=================================================================================
//! Reads ECInstances in the JSON wire format.
//! @remarks This is mainly a convenience wrapper over @ref JsonECSqlSelectAdapter. 
//! The recommended use is for a simple retrieval of instances
//! of a class.
//! @ingroup ECDbGroup
// @bsiclass                                                 Ramanujam.Raman      09/2013
//+===============+===============+===============+===============+===============+======
struct JsonReader final : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        mutable ECSqlStatement m_statement;
        JsonECSqlSelectAdapter::FormatOptions m_formatOptions = JsonECSqlSelectAdapter::FormatOptions::Default;
        bool m_isValid = false;

        BentleyStatus Initialize(ECN::ECClassCR);
        BentleyStatus Initialize(ECN::ECClassId);

    public:
        //! Initializes a new JsonReader instance for the specified class. 
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be retrieved. 
        //! @param[in] formatOptions Options to control the output. 
        ECDB_EXPORT JsonReader(ECDbCR ecdb, ECN::ECClassCR ecClass, JsonECSqlSelectAdapter::FormatOptions formatOptions = JsonECSqlSelectAdapter::FormatOptions::Default);

        //! Initializes a new JsonReader instance for the specified class. 
        //! @param ecdb [in] ECDb
        //! @param ecClassId [in] ECClassId indicating the class of the instance that needs to be retrieved. 
        //! @param[in] formatOptions Options to control the output. 
        ECDB_EXPORT JsonReader(ECDbCR ecdb, ECN::ECClassId ecClassId, JsonECSqlSelectAdapter::FormatOptions formatOptions = JsonECSqlSelectAdapter::FormatOptions::Default);
        ~JsonReader() {}

        //! Indicates whether this JsonReader is valid and can be used to retrieve instances.
        //! It is not valid, if @p ecClass is not mapped for example.
        //! @return true if the reader is valid. false if it cannot be used.
        bool IsValid() const { return m_isValid; }

        //! Reads (only) the specified instance in the JSON format. 
        //! @param [out] jsonInstance JSON representation of the ECInstance as a JSON object made up of property-value pairs.
        //! @param ecInstanceId [in] ECInstanceId pointing to the instance that needs to be retrieved. 
        //! @remarks The returned JSON is a JSON object of property value pairs for the specified instance. See 
        //! @ref JsonECSqlSelectAdapter::GetRowInstance for more details. 
        ECDB_EXPORT BentleyStatus Read(JsonValueR jsonInstance, ECInstanceId ecInstanceId) const;
    };

//=======================================================================================
//! Insert JSON instances into ECDb file.
//@bsiclass                                                 Ramanujam.Raman      02/2013
//+===============+===============+===============+===============+===============+======
struct JsonInserter final : NonCopyableClass
    {
    private:
        ECN::ECClassCR m_ecClass;
        ECInstanceInserter m_ecinstanceInserter;

    public:
        //! Initializes a new JsonInserter instance for the specified class. 
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be inserted. 
        //! @param[in] writeToken Token required to execute ECSQL INSERT statements if 
        //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
        //! If the option is not set, nullptr can be passed for @p writeToken.
        //! @remarks Holds some cached state to speed up future inserts of the same class. Keep the 
        //! inserter around when inserting many instances of the same class. 
        JsonInserter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken) : m_ecClass(ecClass), m_ecinstanceInserter(ecdb, ecClass, writeToken) {}

        //! Indicates whether this JsonInserter is valid and can be used to insert JSON instances.
        //! It is not valid, if the underlying ECClass is not mapped or not instantiable for example.
        //! @return true if the inserter is valid and can be used for inserting. false if it cannot be used for inserting.
        bool IsValid() const { return m_ecinstanceInserter.IsValid(); }

        //! Inserts the instance
        //! @param[out] newInstanceKey the ECInstanceKey generated for the inserted instance
        //! @param[in] jsonValue the instance data
        //! @return BE_SQLITE_OK in case of success, error codes otherwise
        ECDB_EXPORT DbResult Insert(ECInstanceKey& newInstanceKey, JsonValueCR jsonValue) const;

        //! Inserts the instance and updates the $ECInstanceId field with the generated ECInstanceId
        //! @param[in] jsonValue the instance data
        //! @return BE_SQLITE_OK in case of success, error codes otherwise
        ECDB_EXPORT DbResult Insert(JsonValueR jsonValue) const;

        //! Insert an instance created from the specified jsonValue
        //! @param[out] newInstanceKey the ECInstanceKey generated for the inserted instance
        //! @param[in] jsonValue the instance data
        //! @return BE_SQLITE_OK in case of success, error codes otherwise
        ECDB_EXPORT DbResult Insert(ECInstanceKey& newInstanceKey, RapidJsonValueCR jsonValue) const;
    };

//=======================================================================================
//! Update EC content in the ECDb file through JSON values
//@bsiclass                                                 Ramanujam.Raman      02/2013
//+===============+===============+===============+===============+===============+======
struct JsonUpdater final : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        ECN::ECClassCR m_ecClass;
        ECInstanceUpdater m_ecinstanceUpdater;

        ECN::IECInstancePtr CreateEmptyInstance(ECN::ECClassCR ecClass) const { return ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0); }
        ECN::IECInstancePtr CreateEmptyInstance(ECInstanceKeyCR instanceKey) const;
        ECN::IECInstancePtr CreateEmptyRelInstance(ECN::ECRelationshipClassCR ecRelClass, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const;

    public:
        //! Initializes a new JsonUpdater instance for the specified class. 
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be updated. 
        //! @param[in] writeToken Token required to execute ECSQL UPDATE statements if 
        //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
        //! If the option is not set, nullptr can be passed for @p writeToken.
        //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the JsonUpdater.
        //!            Pass without ECSQLOPTIONS keyword.
        //! @remarks Holds some cached state to speed up future updates of the same class. Keep the 
        //! inserter around when updating many instances of the same class. 
        JsonUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions = nullptr) : m_ecdb(ecdb), m_ecClass(ecClass), m_ecinstanceUpdater(ecdb, ecClass, writeToken, ecsqlOptions) {}

        //! Indicates whether this JsonUpdater is valid and can be used to update JSON instances.
        //! It is not valid, if the underlying ECClass is not mapped or not instantiable for example.
        //! @return true if the updater is valid and can be used for updating. false if it cannot be used for updating.
        bool IsValid() const { return m_ecinstanceUpdater.IsValid(); }

        //! Updates an instance from the specified jsonValue
        //! @param[in] instanceId the ECInstanceId of the instance to update
        //! @param[in] jsonValue the instance data
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        ECDB_EXPORT DbResult Update(ECInstanceId instanceId, JsonValueCR jsonValue) const;

        //! Update  a relationship instance from the specified jsonValue and source/target keys
        //! @param[in] instanceId the ECInstanceId of the instance to update
        //! @param[in] jsonValue the instance data
        //! @param[in] sourceKey ECInstanceKey for the source of the relationship
        //! @param[in] targetKey ECInstanceKey for the target of the relationship
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        ECDB_EXPORT DbResult Update(ECInstanceId instanceId, JsonValueCR jsonValue, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const;

        //! Update an instance from the specified jsonValue
        //! @param[in] instanceId the ECInstanceId of the instance to update
        //! @param[in] jsonValue the instance data
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        ECDB_EXPORT DbResult Update(ECInstanceId instanceId, RapidJsonValueCR jsonValue) const;

        //! Update  a relationship instance from the specified jsonValue and source/target keys
        //! @param[in] instanceId the ECInstanceId of the instance to update
        //! @param[in] jsonValue the instance data
        //! @param[in] sourceKey ECInstanceKey for the source of the relationship
        //! @param[in] targetKey ECInstanceKey for the target of the relationship
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        ECDB_EXPORT DbResult Update(ECInstanceId instanceId, RapidJsonValueCR jsonValue, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const;
    };

//=======================================================================================
//! Delete EC content in the ECDb file
//@bsiclass                                                 Ramanujam.Raman      02/2013
//+===============+===============+===============+===============+===============+======
struct JsonDeleter final : NonCopyableClass
    {
    private:
        ECInstanceDeleter m_ecinstanceDeleter;

    public:
        //! Construct an deleter for the specified class. 
        //! @param ecdb [in] ECDb
        //! @param ecClass [in] ECClass of the instance that needs to be deleted. 
        //! @param [in] writeToken Token required to execute ECSQL UPDATE statements if 
        //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
        //! If the option is not set, nullptr can be passed for @p writeToken.
        //! @remarks Holds some cached state to speed up future deletes of the same class. Keep the 
        //! deleter around when deleting many instances of the same class. 
        JsonDeleter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken) : m_ecinstanceDeleter(ecdb, ecClass, writeToken) {}

        //! Indicates whether this JsonDeleter is valid and can be used to delete instances.
        //! It is not valid, if the underlying ECClass is not mapped for example.
        //! @return true if the deleter is valid and can be used for deleting. false if it cannot be used for delete.
        bool IsValid() const { return m_ecinstanceDeleter.IsValid(); }

        //! Deletes the instance identified by the supplied ECInstanceId
        //! @param[in] instanceId the ECInstanceId of the instance to delete
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        DbResult Delete(ECInstanceId instanceId) const { return m_ecinstanceDeleter.Delete(instanceId); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
