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
//! Adapts the rows returned by an ECSqlStatement to the JSON format (see @ref BentleyApi::ECN::ECJsonSystemNames).
//! 
//! @see ECSqlStatement, BentleyApi::ECN::ECJsonSystemNames
//! @ingroup ECDbGroup
//! @bsiclass                                                 08/2012
//+===============+===============+===============+===============+===============+======
struct JsonECSqlSelectAdapter final: NonCopyableClass
    {
    public:
        struct FormatOptions final
            {
            private:
                ECN::ECJsonInt64Format m_int64Format = ECN::ECJsonInt64Format::AsDecimalString;

            public:
                //! Initializes a default BentleyApi::ECN::ECJsonInt64Format object with ECJsonInt64Format::AsDecimalString
                FormatOptions() {}
                explicit FormatOptions(ECN::ECJsonInt64Format int64Format) : m_int64Format(int64Format) {}

                ECN::ECJsonInt64Format GetInt64Format() const { return m_int64Format; }
            };
    private:
        ECSqlStatementCR m_ecsqlStatement;
        FormatOptions m_formatOptions;
   
    public:

        //! Initializes a new JsonECSqlSelectAdapter instance for the specified ECSqlStatement. 
        //! @param[in] ecsqlStatement Prepared ECSqlStatement
        //! @param[in] formatOptions Options to control the output. 
        //! @see ECSqlStatement
        JsonECSqlSelectAdapter(ECSqlStatement const& ecsqlStatement, FormatOptions const& formatOptions = FormatOptions()) : m_ecsqlStatement(ecsqlStatement), m_formatOptions(formatOptions) {}
        ~JsonECSqlSelectAdapter() {}

        //! Gets the current row as JSON object with pairs of property name value for each
        //! item in the ECSQL select clause.
        //! 
        //! The JSON returned is the @ref BentleyApi::ECN::ECJsonSystemNames "EC JSON format".
        //! The ECSQL select clause is what exclusively determines what property name value pairs
        //! the JSON will contain.
        //! The ECSQL system properties are converted to the respective EC JSON format system members:
        //! ECSQL  | JSON Format | JSON Format Data Type
        //! ------ | ------------| ---------------------
        //! %ECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::Id "id" | Hex String
        //! ECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::ClassName "className" | "<Schema Name>.<Class Name>"
        //! SourceECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::SourceId "sourceId" | Hex String
        //! SourceECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::SourceClassName "sourceClassName" | "<Schema Name>.<Class Name>"
        //! TargetECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::TargetId "targetId" | Hex String
        //! TargetECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::TargetClassName "targetClassName" | "<Schema Name>.<Class Name>"
        //! {%Navigation Property}.Id | {navigation Property}.@ref BentleyApi::ECN::ECJsonSystemNames::Navigation::Id "id" | "<Schema Name>.<RelationshipClass Name>"
        //! {%Navigation Property}.RelECClassId | {navigation Property}.@ref BentleyApi::ECN::ECJsonSystemNames::Navigation::RelClassName "relClassName" | "<Schema Name>.<RelationshipClass Name>"
        //! {Point2d/Point3d Property}.X | {point2d/point3d Property}.@ref BentleyApi::ECN::ECJsonSystemNames::Point::X "x" | double
        //! {Point2d/Point3d Property}.Y | {point2d/point3d Property}.@ref BentleyApi::ECN::ECJsonSystemNames::Point::Y "y" | double
        //! {%Point3d Property}.Z | {point3d Property}.@ref BentleyApi::ECN::ECJsonSystemNames::Point::Z "z" | double
        //!
        //! ####Examples
        //! For the ECSQL <c>SELECT ECInstanceId, ECClassId, Name, Age FROM myschema.Employee WHERE ...</c>
        //! the returned JSON format would be this:
        //! 
        //!     {
        //!         "id" : "0x13A",
        //!         "className" : "mySchema.Employee",
        //!         "name": "Sally Smith",
        //!         "age": 30
        //!     }
        //!
        //! For the ECSQL <c>SELECT Name, Age FROM myschema.Employee WHERE ...</c>
        //! the returned JSON format would be this:
        //! 
        //!     {
        //!         "name": "Sally Smith",
        //!         "age": 30
        //!     }
        //! 
        //! Using expressions or aliases or nesting property accessors in the ECSQL select clause
        //! affect the JSON member names, not the JSON structure.
        //! @note When using expressions in the SELECT clause it is recommended to assign a column alias to them.
        //! The JSON member name will then be the alias instead of the full expression.
        //! @param [out] json current row as JSON object of property name value pairs
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus GetRow(JsonValueR json) const;

        //! Gets only the columns from the current row that refer to the specified ECClass
        //!
        //! Example:
        //! Assume the ECSQL <c>SELECT Employee.ECInstanceId, Employee.Name, Employee.Age, Company.ECInstanceId, Company.Name FROM myschema.Employee JOIN myschema.Company USING myschema.CompanyEmploysEmployee</c>.
        //!
        //! If the %ECClassId of @c Employee was passed to the method, the resulting JSON would be:
        //! 
        //!     {
        //!     "id" : "0x123",
        //!     "name": "Sally Smith",
        //!     "age": 30
        //!     }
        //!
        //! If the %ECClassId of @c Company was passed to the method, the resulting JSON would be:
        //! 
        //!     {
        //!     "id" : "0x332",
        //!     "Name": "ACME"
        //!     }
        //!
        //! @param [out] json Current row values of the column of the specified class as JSON object of property name value pairs
        //! @param [in] classId ECClassId indicating the class of the instance needed from the current row 
        //! @return false if there was an error in retrieving values. true otherwise.
        ECDB_EXPORT BentleyStatus GetRowInstance(JsonValueR json, ECN::ECClassId classId) const;
    };

//=================================================================================
//! Reads ECInstances as JSON in the @ref BentleyApi::ECN::ECJsonSystemNames "EC JSON format".
//! @remarks This is mainly a convenience wrapper over @ref JsonECSqlSelectAdapter
//! using an ECSQL that selects all properties of the ECClass plus ECInstanceId and ECClassId.
//! @ingroup ECDbGroup
// @bsiclass                                                 Ramanujam.Raman      09/2013
//+===============+===============+===============+===============+===============+======
struct JsonReader final : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        mutable ECSqlStatement m_statement;
        JsonECSqlSelectAdapter::FormatOptions m_formatOptions;
        bool m_isValid = false;

        BentleyStatus Initialize(ECN::ECClassCR);
        BentleyStatus Initialize(ECN::ECClassId);

    public:
        //! Initializes a new JsonReader instance for the specified class. 
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be retrieved. 
        //! @param[in] formatOptions Options to control the output. 
        ECDB_EXPORT JsonReader(ECDbCR ecdb, ECN::ECClassCR ecClass, JsonECSqlSelectAdapter::FormatOptions const& formatOptions = JsonECSqlSelectAdapter::FormatOptions());

        //! Initializes a new JsonReader instance for the specified class. 
        //! @param ecdb [in] ECDb
        //! @param ecClassId [in] ECClassId indicating the class of the instance that needs to be retrieved. 
        //! @param[in] formatOptions Options to control the output. 
        ECDB_EXPORT JsonReader(ECDbCR ecdb, ECN::ECClassId ecClassId, JsonECSqlSelectAdapter::FormatOptions const& formatOptions = JsonECSqlSelectAdapter::FormatOptions());
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
//! @remarks The JSON must be in the @ref BentleyApi::ECN::ECJsonSystemNames "EC JSON Format".
//@bsiclass                                                 Ramanujam.Raman      02/2013
//+===============+===============+===============+===============+===============+======
struct JsonInserter final : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
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
        JsonInserter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken) : m_ecdb(ecdb), m_ecClass(ecClass), m_ecinstanceInserter(ecdb, ecClass, writeToken) {}

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
//! @remarks The JSON must be in the @ref BentleyApi::ECN::ECJsonSystemNames "EC JSON Format".
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


END_BENTLEY_SQLITE_EC_NAMESPACE
