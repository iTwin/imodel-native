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
#include <json/json.h>
#include <rapidjson/BeRapidJson.h>
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
//! Helper API to bind @ref BentleyApi::ECN::ECJsonSystemNames "ECJSON" values to ECSQL parameters.
//! @see ECSqlStatement, IECSqlBinder
// @bsiclass                                                  09/2017
//+===============+===============+===============+===============+===============+======
struct JsonECSqlBinder final
    {
    private:
        JsonECSqlBinder() = delete;
        ~JsonECSqlBinder() = delete;

        static ECSqlStatus BindValue(IECSqlBinder& binder, JsonValueCR memberJson, ECN::ECPropertyCR);
        static ECSqlStatus BindArrayValue(IECSqlBinder&, JsonValueCR arrayJson, ECN::PrimitiveType const*, ECN::ECStructClass const*);

        static ECSqlStatus BindValue(IECSqlBinder& binder, RapidJsonValueCR memberJson, ECN::ECPropertyCR);
        static ECSqlStatus BindArrayValue(IECSqlBinder&, RapidJsonValueCR arrayJson, ECN::PrimitiveType const*, ECN::ECStructClass const*);

    public:
        /// @name Methods to bind JSON values from the JsonCpp API
        /// @{

        //! Binds a ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the ECSQL parameter to which the value is bound to
        //! @param[in] json ECJSON value to be bound to @p binder
        //! @param[in] prop ECProperty of the parameter expression
        //! @param[in] classLocater Class locater needed to look up ECClasses or ECClassIds from ECJSON class names (e.g. via ECDb::GetClassLocater).
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindValue(IECSqlBinder& binder, JsonValueCR json, ECN::ECPropertyCR prop, ECN::IECClassLocater& classLocater);

        //! Binds a primitive ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the primitive ECSQL parameter to which the value is bound to
        //! @param[in] primitiveJson Primitive ECJSON value to be bound to @p binder
        //! @param[in] primitiveType Primitive type
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindPrimitiveValue(IECSqlBinder& binder, JsonValueCR primitiveJson, ECN::PrimitiveType primitiveType);

        //! Binds a struct ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the struct ECSQL parameter to which the value is bound to
        //! @param[in] structJson Struct ECJSON value to be bound to @p binder
        //! @param[in] structType Struct type
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindStructValue(IECSqlBinder& binder, JsonValueCR structJson, ECN::ECStructClassCR structType);

        //! Binds a primitive array EC JSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the struct ECSQL parameter to which the value is bound to
        //! @param[in] arrayJson Primitive array EC JSON value to be bound to @p binder
        //! @param[in] arrayElementType Array element type
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindPrimitiveArrayValue(IECSqlBinder& binder, JsonValueCR arrayJson, ECN::PrimitiveType arrayElementType);

        //! Binds a struct array ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the struct ECSQL parameter to which the value is bound to
        //! @param[in] arrayJson Struct array ECJSON value to be bound to @p binder
        //! @param[in] arrayElementType Array element type
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindStructArrayValue(IECSqlBinder& binder, JsonValueCR arrayJson, ECN::ECStructClassCR arrayElementType);

        //! Binds an ECJSON navigation property value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the navigation property ECSQL parameter to which the value is bound to
        //! @param[in] navJson ECJSON navigation property value to be bound to @p binder
        //! @param[in] classLocater Class locater needed to look up the relationship class from the @ref BentleyApi::ECN::ECJsonSystemNames::Navigation::RelClassName "relClassName" member (e.g. via ECDb::GetClassLocater).
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindNavigationValue(IECSqlBinder& binder, JsonValueCR navJson, ECN::IECClassLocater& classLocater);

        //! @}

        /// @name Methods to bind JSON values from the RapidJson API
        //! @{

        //! Binds a ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the ECSQL parameter to which the value is bound to
        //! @param[in] json ECJSON value to be bound to @p binder
        //! @param[in] prop ECProperty of the parameter expression
        //! @param[in] classLocater Class locater needed to look up ECClasses or ECClassIds from ECJSON class names (e.g. via ECDb::GetClassLocater).
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindValue(IECSqlBinder& binder, RapidJsonValueCR json, ECN::ECPropertyCR prop, ECN::IECClassLocater& classLocater);

        //! Binds a primitive ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the primitive ECSQL parameter to which the value is bound to
        //! @param[in] primitiveJson Primitive ECJSON value to be bound to @p binder
        //! @param[in] primitiveType Primitive type
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindPrimitiveValue(IECSqlBinder& binder, RapidJsonValueCR primitiveJson, ECN::PrimitiveType primitiveType);

        //! Binds a struct ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the struct ECSQL parameter to which the value is bound to
        //! @param[in] structJson Struct ECJSON value to be bound to @p binder
        //! @param[in] structType Struct type
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindStructValue(IECSqlBinder& binder, RapidJsonValueCR structJson, ECN::ECStructClassCR structType);

        //! Binds a primitive array ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the struct ECSQL parameter to which the value is bound to
        //! @param[in] arrayJson Primitive array ECJSON value to be bound to @p binder
        //! @param[in] arrayElementType Array element type
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindPrimitiveArrayValue(IECSqlBinder& binder, RapidJsonValueCR arrayJson, ECN::PrimitiveType arrayElementType);

        //! Binds a struct array ECJSON value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the struct ECSQL parameter to which the value is bound to
        //! @param[in] arrayJson Struct array ECJSON value to be bound to @p binder
        //! @param[in] arrayElementType Array element type
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindStructArrayValue(IECSqlBinder& binder, RapidJsonValueCR arrayJson, ECN::ECStructClassCR arrayElementType);

        //! Binds an ECJSON navigation property value to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the navigation property ECSQL parameter to which the value is bound to
        //! @param[in] navJson ECJSON navigation property value to be bound to @p binder
        //! @param[in] classLocater Class locater needed to look up the relationship class from the @ref BentleyApi::ECN::ECJsonSystemNames::Navigation::RelClassName "relClassName" member (e.g. via ECDb::GetClassLocater).
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindNavigationValue(IECSqlBinder& binder, RapidJsonValueCR navJson, ECN::IECClassLocater& classLocater);

        //! @}
    };

//=======================================================================================
//! Adapts the rows returned by an ECSqlStatement to the JSON format (see @ref BentleyApi::ECN::ECJsonSystemNames).
//! 
//! @see ECSqlStatement, BentleyApi::ECN::ECJsonSystemNames
//! @ingroup ECDbGroup
//! @bsiclass                                                               08/2012
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
        //! @c %ECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::Id "id" | Hex String
        //! @c ECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::ClassName "className" | "<Schema Name>.<Class Name>"
        //! @c SourceECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::SourceId "sourceId" | Hex String
        //! @c SourceECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::SourceClassName "sourceClassName" | "<Schema Name>.<Class Name>"
        //! @c TargetECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::TargetId "targetId" | Hex String
        //! @c TargetECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::TargetClassName "targetClassName" | "<Schema Name>.<Class Name>"
        //! &lt;%Navigation Property&gt;.<c>Id</c> | &lt;navigation Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Navigation::Id "id" | "<Schema Name>.<RelationshipClass Name>"
        //! &lt;%Navigation Property&gt;.<c>RelECClassId</c> | &lt;navigation Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Navigation::RelClassName "relClassName" | "<Schema Name>.<RelationshipClass Name>"
        //! &lt;Point2d/Point3d Property&gt;.<c>X</c> | &lt;point2d/point3d Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Point::X "x" | double
        //! &lt;Point2d/Point3d Property&gt;.<c>Y</c> | &lt;point2d/point3d Property&gt;..@ref BentleyApi::ECN::ECJsonSystemNames::Point::Y "y" | double
        //! &lt;%Point3d Property&gt;.<c>Z</c> | &lt;point3d Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Point::Z "z" | double
        //!
        //! ####Examples
        //! For the ECSQL <c>SELECT %ECInstanceId, ECClassId, Name, Age FROM myschema.Employee WHERE ...</c>
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
        //! @param[in] appendToJson If true, the JSON property name value pairs of the retrieved row will
        //! be appended to @p json. In this case, @p json must have be a JSON object.
        //! If false, @p json will just contain the retrieved row data. If @p json contained
        //! members before the call, those will be cleared.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus GetRow(JsonValueR json, bool appendToJson = false) const;

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
        //!     "name": "ACME"
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
// @bsiclass                                                                09/2013
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
//! @remarks The JSON must be in the @ref BentleyApi::ECN::ECJsonSystemNames "ECJSON Format".
//@bsiclass                                                                   02/2013
//+===============+===============+===============+===============+===============+======
struct JsonInserter final : NonCopyableClass
    {
    private:
        struct BindingInfo final
            {
            private:
                int m_parameterIndex = 0;
                ECN::ECPropertyCP m_property = nullptr;
            public:
                BindingInfo() {}
                BindingInfo(int paramIndex, ECN::ECPropertyCR prop) : m_parameterIndex(paramIndex), m_property(&prop) {}
                explicit BindingInfo(int systemParamIndex) : m_parameterIndex(systemParamIndex) {}

                int GetParameterIndex() const { return m_parameterIndex; }
                bool IsSystemProperty() const { return m_property == nullptr; }
                ECN::ECPropertyCP GetProperty() const { return m_property; }
            };

        struct CompareIUtf8Ascii
            {
            bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::StricmpAscii(s1, s2) < 0; }
            };

        ECDbCR m_ecdb;
        ECN::ECClassCR m_ecClass;
        Utf8String m_jsonClassName;

        mutable ECSqlStatement m_statement;
        bmap<Utf8CP, BindingInfo, CompareIUtf8Ascii> m_bindingMap;
        bool m_isValid = false;

        void Initialize(ECCrudWriteToken const* writeToken);

    public:
        //! Initializes a new JsonInserter instance for the specified class. 
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be inserted. 
        //! @param[in] writeToken Token required to execute ECSQL INSERT statements if 
        //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
        //! If the option is not set, nullptr can be passed for @p writeToken.
        //! @remarks Holds some cached state to speed up future inserts of the same class. Keep the 
        //! inserter around when inserting many instances of the same class. 
        ECDB_EXPORT JsonInserter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken);

        //! Indicates whether this JsonInserter is valid and can be used to insert JSON instances.
        //! It is not valid, if the underlying ECClass is not mapped or not instantiable for example.
        //! @return true if the inserter is valid and can be used for inserting. false if it cannot be used for inserting.
        bool IsValid() const { return m_isValid; }

        //! Inserts a row from the specified EC JSON object
        //! @remarks if the @ref BentleyApi::ECN::ECJsonUtilities::json_id "id" member is set in the @p json,
        //! ECDb will not generate an ECInstanceId but use the member's value instead.
        //! @param[out] key the ECInstanceKey generated for the inserted instance
        //! @param[in] json EC JSON object to insert
        //! @return BE_SQLITE_OK in case of success, error codes otherwise
        ECDB_EXPORT DbResult Insert(ECInstanceKey& key, JsonValueCR json) const;

        //! Inserts a row from the specified EC JSON object
        //! Inserts the row and adds the @ref BentleyApi::ECN::ECJsonUtilities::json_id "id" member with the generated ECInstanceId
        //! to @p json
        //! @param[in] json EC JSON object to insert
        //! @return BE_SQLITE_OK in case of success, error codes otherwise
        ECDB_EXPORT DbResult Insert(JsonValueR json) const;

        //! Insert an instance created from the specified jsonValue
        //! @remarks if the @ref BentleyApi::ECN::ECJsonUtilities::json_id "id" member is set in the @p json,
        //! ECDb will not generate an ECInstanceId but use the member's value instead.
        //! @param[out] key the ECInstanceKey generated for the inserted instance
        //! @param[in] json EC JSON object to insert
        //! @return BE_SQLITE_OK in case of success, error codes otherwise
        ECDB_EXPORT DbResult Insert(ECInstanceKey& key, RapidJsonValueCR json) const;
    };

//=======================================================================================
//! Update EC content in the ECDb file through JSON values
//! @remarks The JSON must be in the @ref BentleyApi::ECN::ECJsonSystemNames "ECJSON Format".
//@bsiclass                                                           02/2013
//+===============+===============+===============+===============+===============+======
struct JsonUpdater final : NonCopyableClass
    {
    private:
        struct BindingInfo final
            {
            private:
                int m_parameterIndex = 0;
                ECN::ECPropertyCP m_property = nullptr;
            public:
                BindingInfo() {}
                BindingInfo(int paramIndex, ECN::ECPropertyCR prop) : m_parameterIndex(paramIndex), m_property(&prop) {}

                int GetParameterIndex() const { return m_parameterIndex; }
                ECN::ECPropertyCR GetProperty() const { BeAssert(m_property != nullptr); return *m_property; }
            };

        struct CompareIUtf8Ascii
            {
            bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::StricmpAscii(s1, s2) < 0; }
            };

        ECDbCR m_ecdb;
        ECN::ECClassCR m_ecClass;
        Utf8String m_jsonClassName;

        mutable ECSqlStatement m_statement;
        bmap<Utf8CP, BindingInfo, CompareIUtf8Ascii> m_bindingMap;
        int m_idParameterIndex = 0;
        bool m_isValid = false;

        BentleyStatus Initialize(bvector<Utf8CP> const* propertyNames, Utf8CP ecsqlOptions, ECCrudWriteToken const* writeToken);

    public:
        //! Initializes a new JsonUpdater instance for the specified class. 
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be updated. 
        //! @param[in] writeToken Token required to execute ECSQL UPDATE statements if 
        //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
        //! If the option is not set, nullptr can be passed for @p writeToken.
        //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the JsonUpdater.
        //!            Pass without ECSQLOPTIONS keyword.
        ECDB_EXPORT JsonUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions = nullptr);

        //! Initializes a new JsonUpdater instance for the specified class. 
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be updated. 
        //! @param[in] propertyNames ECClass of the instance that needs to be updated. 
        //! @param[in] writeToken Token required to execute ECSQL UPDATE statements if 
        //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
        //! If the option is not set, nullptr can be passed for @p writeToken.
        //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the JsonUpdater.
        //!            Pass without ECSQLOPTIONS keyword.
        ECDB_EXPORT JsonUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, bvector<Utf8CP> const& propertyNames, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions = nullptr);

        //! Indicates whether this JsonUpdater is valid and can be used to update JSON instances.
        //! It is not valid, if the underlying ECClass is abstract or not mapped for example.
        //! @return true if the updater is valid and can be used for updating. false if it cannot be used for updating.
        bool IsValid() const { return m_isValid; }

        //! Updates an instance from the specified ECJSON
        //! @remarks All ECProperties of the underlying ECClass for which the input ECJSON does not contain a value
        //! are nulled-out.
        //! @param[in] instanceId the ECInstanceId of the instance to update
        //! @param[in] json the instance data
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        ECDB_EXPORT DbResult Update(ECInstanceId instanceId, JsonValueCR json) const;

        //! Update an instance from the specified ECJSON
        //! @remarks All ECProperties of the underlying ECClass for which the input ECJSON does not contain a value
        //! are nulled-out.
        //! @param[in] instanceId the ECInstanceId of the instance to update
        //! @param[in] json the instance data
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        ECDB_EXPORT DbResult Update(ECInstanceId instanceId, RapidJsonValueCR json) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
