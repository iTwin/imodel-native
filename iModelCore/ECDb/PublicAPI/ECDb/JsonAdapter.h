/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECSqlStatement.h>
#include <json/json.h>
#include <BeRapidJson/BeRapidJson.h>

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
struct JsonECSqlSelectAdapter final
    {
    public:
        enum class MemberNameCasing
            {
            KeepOriginal, //!< Member name as returned from ECSQL
            LowerFirstChar //!< First character of the member name is lowered. This does not apply to system members.
            };
        enum class BlobMode
            {
            ArrayOfInt,
            Base64String            
            };
        enum class RowFormat
            {
            Custom,
            IModelJs
            };
        struct FormatOptions final
            {
            private:
                MemberNameCasing m_memberNameCasing = MemberNameCasing::KeepOriginal;
                ECN::ECJsonInt64Format m_int64Format = ECN::ECJsonInt64Format::AsDecimalString;
                BlobMode m_blobMode = BlobMode::Base64String;
                Utf8String m_base64Header;
                RowFormat m_rowFormat = RowFormat::Custom;
            public:
                //! Initializes a default FormatOptions object
                //! with MemberCasingMode::KeepOriginal and ECJsonInt64Format::AsDecimalString
                FormatOptions() {}
                //! Initializes a new FormatOptions object
                //!@param[in] memberNameCasing Defines how the member names in the resulting JSON will be formatted.
                //!           Casing of system member names is not affected by this.
                //!@param[in] int64Format Defines how ECProperty values of type Int64 / Long will be formatted
                FormatOptions(MemberNameCasing memberNameCasing, ECN::ECJsonInt64Format int64Format, BlobMode blobMode = BlobMode::Base64String)
                    :m_memberNameCasing(memberNameCasing), m_int64Format(int64Format), m_blobMode(blobMode), m_rowFormat(RowFormat::Custom)
                    {}
                ECDB_EXPORT void SetRowFormat(RowFormat fmt);
                RowFormat GetRowFormat() const { return m_rowFormat; }
                MemberNameCasing GetMemberCasingMode() const { return m_memberNameCasing; }
                BlobMode GetBlobMode() const { return m_blobMode; }
                ECN::ECJsonInt64Format GetInt64Format() const { return m_int64Format; }
                Utf8StringCR GetBase64Header() const { return m_base64Header; }
            };

    private:
        struct CacheImpl;
        ECSqlStatement const& m_ecsqlStatement;
        uint64_t m_ecsqlHash;
        FormatOptions m_formatOptions;
        bool m_copyMemberNames = true;
        CacheImpl* m_cacheImpl;
        //not copyable
        JsonECSqlSelectAdapter(JsonECSqlSelectAdapter const&) = delete;
        JsonECSqlSelectAdapter& operator=(JsonECSqlSelectAdapter const&) = delete;

    public:
        //! Initializes a new JsonECSqlSelectAdapter instance for the specified ECSqlStatement. 
        //! @param[in] ecsqlStatement Prepared ECSqlStatement
        //! @param[in] formatOptions Options to control the output. 
        //! @param[in] copyMemberNames if true, the resulting JSON objects own the member name strings.
        //!            if false, the adapter owns the member names, and the JSON objects just have references to it.
        //!            This can be used for a performance and memory optimization, but callers must make sure the
        //!            the adapter object lives at least as long as the generated JSON objects 
        //! @see ECSqlStatement
        ECDB_EXPORT JsonECSqlSelectAdapter(ECSqlStatement const& ecsqlStatement, FormatOptions const& formatOptions = FormatOptions(), bool copyMemberNames = true);
        ECDB_EXPORT~JsonECSqlSelectAdapter();

        //! Gets the current row as JSON object with pairs of property name value for each
        //! item in the ECSQL select clause.
        //! 
        //! The ECSQL select clause is what exclusively determines which property name value pairs
        //! the JSON will contain.
        //!
        //! The JSON returned is the @ref BentleyApi::ECN::ECJsonSystemNames "EC JSON format".
        //! The ECSQL system properties are converted to the respective EC JSON format system members:
        //! ECSQL  | JSON Format | JSON Format Data Type
        //! ------ | ------------| ---------------------
        //! @c %ECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::Id "id" | Hex String
        //! @c ECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::ClassName "className" | "<Schema Name>.<Class Name>"
        //! @c SourceECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::SourceId "sourceId" | Hex String
        //! @c SourceECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::SourceClassName "sourceClassName" | "<Schema Name>.<Class Name>"
        //! @c TargetECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::TargetId "targetId" | Hex String
        //! @c TargetECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::TargetClassName "targetClassName" | "<Schema Name>.<Class Name>"
        //! &lt;%Navigation Property&gt;.<c>Id</c> | &lt;%Navigation Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Navigation::Id "id" | "<Schema Name>.<RelationshipClass Name>"
        //! &lt;%Navigation Property&gt;.<c>RelECClassId</c> | &lt;%Navigation Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Navigation::RelClassName "relClassName" | "<Schema Name>.<RelationshipClass Name>"
        //! &lt;Point2d/Point3d Property&gt;.<c>X</c> | &lt;Point2d/Point3d Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Point::X "x" | double
        //! &lt;Point2d/Point3d Property&gt;.<c>Y</c> | &lt;Point2d/Point3d Property&gt;..@ref BentleyApi::ECN::ECJsonSystemNames::Point::Y "y" | double
        //! &lt;%Point3d Property&gt;.<c>Z</c> | &lt;Point3d Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Point::Z "z" | double
        //!
        //! ####Examples
        //! For the ECSQL <c>SELECT %ECInstanceId, ECClassId, Name, Age FROM myschema.Employee WHERE ...</c>
        //! the returned JSON format would be this:
        //! 
        //!     {
        //!         "id" : "0x13A",
        //!         "className" : "mySchema.Employee",
        //!         "Name": "Sally Smith",
        //!         "Age": 30
        //!     }
        //!
        //! For the ECSQL <c>SELECT Name, Age FROM myschema.Employee WHERE ...</c>
        //! the returned JSON format would be this:
        //! 
        //!     {
        //!         "Name": "Sally Smith",
        //!         "Age": 30
        //!     }
        //! 
        //! Using expressions or aliases or nesting property accessors in the ECSQL select clause
        //! affect the JSON member names, not the JSON structure.
        //! @note When using expressions in the SELECT clause it is recommended to assign a column alias to them.
        //! The JSON member name will then be the alias instead of the full expression.
        //! @param [out] json current row as JSON object of property name value pairs
        //! @param[in] appendToJson If true, the JSON property name value pairs of the retrieved row will
        //! be appended to @p json. In this case, @p json must be a JSON object.
        //! If false, @p json will just contain the retrieved row data. If @p json contained
        //! members before the call, those will be cleared.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus GetRow(JsonValueR json, bool appendToJson = false) const;

        ECDB_EXPORT BentleyStatus GetRow(JsonValueR json, ECSqlStatement const& stmt, bool appendToJson = false) const;

        //! Gets the current row as JSON object with pairs of property name value for each
        //! item in the ECSQL select clause.
        //! 
        //! The ECSQL select clause is what exclusively determines which property name value pairs
        //! the JSON will contain.
        //!
        //! The JSON returned is the @ref BentleyApi::ECN::ECJsonSystemNames "EC JSON format".
        //! The ECSQL system properties are converted to the respective EC JSON format system members:
        //! ECSQL  | JSON Format | JSON Format Data Type
        //! ------ | ------------| ---------------------
        //! @c %ECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::Id "id" | Hex String
        //! @c ECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::ClassName "className" | "<Schema Name>.<Class Name>"
        //! @c SourceECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::SourceId "sourceId" | Hex String
        //! @c SourceECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::SourceClassName "sourceClassName" | "<Schema Name>.<Class Name>"
        //! @c TargetECInstanceId | @ref BentleyApi::ECN::ECJsonSystemNames::TargetId "targetId" | Hex String
        //! @c TargetECClassId | @ref BentleyApi::ECN::ECJsonSystemNames::TargetClassName "targetClassName" | "<Schema Name>.<Class Name>"
        //! &lt;%Navigation Property&gt;.<c>Id</c> | &lt;%Navigation Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Navigation::Id "id" | "<Schema Name>.<RelationshipClass Name>"
        //! &lt;%Navigation Property&gt;.<c>RelECClassId</c> | &lt;%Navigation Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Navigation::RelClassName "relClassName" | "<Schema Name>.<RelationshipClass Name>"
        //! &lt;Point2d/Point3d Property&gt;.<c>X</c> | &lt;Point2d/Point3d Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Point::X "x" | double
        //! &lt;Point2d/Point3d Property&gt;.<c>Y</c> | &lt;Point2d/Point3d Property&gt;..@ref BentleyApi::ECN::ECJsonSystemNames::Point::Y "y" | double
        //! &lt;%Point3d Property&gt;.<c>Z</c> | &lt;Point3d Property&gt;.@ref BentleyApi::ECN::ECJsonSystemNames::Point::Z "z" | double
        //!
        //! ####Examples
        //! For the ECSQL <c>SELECT %ECInstanceId, ECClassId, Name, Age FROM myschema.Employee WHERE ...</c>
        //! the returned JSON format would be this:
        //! 
        //!     {
        //!         "id" : "0x13A",
        //!         "className" : "mySchema.Employee",
        //!         "Name": "Sally Smith",
        //!         "Age": 30
        //!     }
        //!
        //! For the ECSQL <c>SELECT Name, Age FROM myschema.Employee WHERE ...</c>
        //! the returned JSON format would be this:
        //! 
        //!     {
        //!         "Name": "Sally Smith",
        //!         "Age": 30
        //!     }
        //! 
        //! Using expressions or aliases or nesting property accessors in the ECSQL select clause
        //! affect the JSON member names, not the JSON structure.
        //! @note When using expressions in the SELECT clause it is recommended to assign a column alias to them.
        //! The JSON member name will then be the alias instead of the full expression.
        //! @param [out] json current row as JSON object of property name value pairs
        //! @param[in] allocator Allocator to use to copy the values into the RapidJson value.
        //! @param[in] appendToJson If true, the JSON property name value pairs of the retrieved row will
        //! be appended to @p json. In this case, @p json must be a JSON object.
        //! If false, @p json will just contain the retrieved row data. If @p json contained
        //! members before the call, those will be cleared.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus GetRow(RapidJsonValueR json, rapidjson::MemoryPoolAllocator<>& allocator, bool appendToJson = false) const;

        //! Gets the current row as JSON object with pairs of property name value for each
        //! item in the ECSQL select clause that refer to the specified ECClass.
        //!
        //! The ECSQL select clause is what exclusively determines which property name value pairs
        //! the JSON will contain.
        //!
        //! @see JsonECSqlSelectAdapter::GetRow for details on the ECJSON format.
        //!
        //! Example:
        //! Assume the ECSQL <c>SELECT Employee.ECInstanceId, Employee.Name, Employee.Age, Company.ECInstanceId, Company.Name FROM myschema.Employee JOIN myschema.Company USING myschema.CompanyEmploysEmployee</c>.
        //!
        //! If the %ECClassId of @c Employee was passed to the method, the resulting JSON would be:
        //! 
        //!     {
        //!     "id" : "0x123",
        //!     "Name": "Sally Smith",
        //!     "Age": 30
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

        //! Gets the current row as JSON object with pairs of property name value for each
        //! item in the ECSQL select clause that refer to the specified ECClass.
        //!
        //! The ECSQL select clause is what exclusively determines which property name value pairs
        //! the JSON will contain.
        //!
        //! @see JsonECSqlSelectAdapter::GetRow for details on the ECJSON format.
        //!
        //! Example:
        //! Assume the ECSQL <c>SELECT Employee.ECInstanceId, Employee.Name, Employee.Age, Company.ECInstanceId, Company.Name FROM myschema.Employee JOIN myschema.Company USING myschema.CompanyEmploysEmployee</c>.
        //!
        //! If the %ECClassId of @c Employee was passed to the method, the resulting JSON would be:
        //! 
        //!     {
        //!     "id" : "0x123",
        //!     "Name": "Sally Smith",
        //!     "Age": 30
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
        //! @param[in] allocator Allocator to use to copy the values into the RapidJson value.
        //! @return false if there was an error in retrieving values. true otherwise.
        ECDB_EXPORT BentleyStatus GetRowInstance(RapidJsonValueR json, ECN::ECClassId classId, rapidjson::MemoryPoolAllocator<>& allocator) const;
    };

//=================================================================================
//! Reads ECInstances as JSON in the @ref BentleyApi::ECN::ECJsonSystemNames "EC JSON format".
//! @remarks This is mainly a convenience wrapper over @ref JsonECSqlSelectAdapter
//! using an ECSQL that selects all properties of the ECClass plus ECInstanceId and ECClassId.
//! @ingroup ECDbGroup
// @bsiclass                                                                09/2013
//+===============+===============+===============+===============+===============+======
struct JsonReader final
    {
    private:
        ECDbCR m_ecdb;
        mutable ECSqlStatement m_statement;
        JsonECSqlSelectAdapter::FormatOptions m_formatOptions;
        bool m_isValid = false;

        //not copyable
        JsonReader(JsonReader const&) = delete;
        JsonReader& operator=(JsonReader const&) = delete;

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

        //! Reads (only) the specified instance in the JSON format. 
        //! @param [out] jsonInstance JSON representation of the ECInstance as a JSON object made up of property-value pairs.
        //! @param ecInstanceId [in] ECInstanceId pointing to the instance that needs to be retrieved. 
        //! @param[in] allocator Allocator to use to copy the values into the RapidJson value.
        //! @remarks The returned JSON is a JSON object of property value pairs for the specified instance. See 
        //! @ref JsonECSqlSelectAdapter::GetRowInstance for more details. 
        ECDB_EXPORT BentleyStatus Read(RapidJsonValueR jsonInstance, ECInstanceId ecInstanceId, rapidjson::MemoryPoolAllocator<>& allocator) const;
    };




//=======================================================================================
//! Insert JSON instances into ECDb file.
//! @remarks The JSON must be in the @ref BentleyApi::ECN::ECJsonSystemNames "ECJSON Format".
//@bsiclass                                                                   02/2013
//+===============+===============+===============+===============+===============+======
struct JsonInserter final
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

        //not copyable
        JsonInserter(JsonInserter const&) = delete;
        JsonInserter& operator=(JsonInserter const&) = delete;

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
//! Update EC content in the ECDb file through @ref BentleyApi::ECN::ECJsonSystemNames "ECJSON" values
//! @remarks The input JSON must contain the members which the updater is supposed to update. 
//!
//! @note To simplify round-trip workflows, JsonUpdater::Options can be used to control how not-updatable should be treated if
//! present in the input ECJSON.
//!
//! ### How the JsonUpdater works
//! During construction the JsonUpdater generates and prepares an ECSQL UPDATE statement from the specified class and property names.
//! The list of property names makes up the SET clause of the UPDATE statement, i.e. it is the list that indicates which properties
//! are to be updated by the updater. <b>If no property name list is passed, all properties of the class will show up in the SET clause.</b>
//!
//! @em Example:
//! JsonUpdater initialization | Generated underlying ECSQL
//! ---------------------------|----------------------------
//! ECClass 'MySchema.MyClass' and properties {"Prop1','Prop2' } | <c>UPDATE ONLY MySchema.MyClass SET Prop1=?, Prop2=? WHERE ECInstanceId=?</c>
//! ECClass 'MySchema.MyClass' | <c>UPDATE ONLY MySchema.MyClass SET Prop1=?, Prop2=?, Prop3=?, Prop4=?,... WHERE ECInstanceId=?</c>
//!
//! When JsonUpdater::Update is called, the values from the input ECJSON are bound to the members in the ECSQL UPDATE SET clause.
//! That implies:
//!     - The ECJSON may only contain members that match the list of properties passed at construction time
//!     - The ECJSON may not contain the instance id. It is passed as separate argument to the Update method.
//!     - Any properties not contained in the ECJSON will be nulled out because no value is bound to its parameter in the SET clause.
//!     (Unbound parameters in SQLite are treated as if NULL was bound to them).
//! 
//! The benefit of this approach is to avoid re-preparing a new ECSQL statement for every update. It comes at the cost that the set of properties
//! to be updated remains fixed for the life-time of a JsonUpdater instance. Consider using the JsonECSqlBinder helper API if you need more
//! flexibility with the set of properties to be updated.
//@bsiclass                                                           09/2017
//+===============+===============+===============+===============+===============+======
struct JsonUpdater final
    {
    public:
        //=======================================================================================
        //! Options that control how the JsonUpdater treats @ref ECSqlSystemProperties.
        //! They are never updatable in ECSQL. But in round-trip workflows the JSON might contain them. These
        //! workflows can then decide to have the JsonUpdater ignore them.
        //! @see BentleyApi::ECN::ECJsonSystemNames
        //@bsienum                                                           09/2017
        //+===============+===============+===============+===============+===============+======
        enum class SystemPropertiesOption
            {
            //!JsonUpdater fails if the input ECJSON contains system properties.
            Fail,
            //! JsonUpdater ignores system properties in the input ECJSON.
            Ignore
            };

        //=======================================================================================
        //! Options that control how the JsonUpdater treats @ref BentleyApi::ECN::ECProperty::GetIsReadOnly "read-only" properties.
        //! @note This does not affect system properties. Use SystemPropertiesOption to control behavior for them.
        //@bsienum                                                           09/2017
        //+===============+===============+===============+===============+===============+======
        enum class ReadonlyPropertiesOption
            {
            //!JsonUpdater fails if the class or list of properties it is initialized with contains read-only properties.
            Fail,
            //! JsonUpdater ignores read-only properties.
            Ignore,
            //! JsonUpdater treats read-only properties as writable properties. 
            //! @remarks This option is equivalent to the @ref ECSQLOptions "ECSQLOPTION" @b ReadonlyPropertiesAreUpdatable
            Update
            };

        //=======================================================================================
        //! Options that control the behavior of the JsonUpdater.
        //@bsiclass                                                           09/2017
        //+===============+===============+===============+===============+===============+======
        struct Options final
            {
            private:
                SystemPropertiesOption m_systemProps = SystemPropertiesOption::Fail;
                ReadonlyPropertiesOption m_readonlyProps = ReadonlyPropertiesOption::Fail;
                Utf8String m_ecsqlOptions;
                bool m_isValid = true;

            public:
                //! Initializes a new default Options object.
                //! Default values:
                //!     - JsonUpdater::SystemPropertiesOption::Fail
                //!     - JsonUpdater::ReadonlyPropertiesOption::Fail
                //!     - ECSQLOPTIONS: empty
                Options() {}

                //! Initializes a new Options object with JsonUpdater::SystemPropertiesOption set to its default value.
                //! @remarks ReadonlyPropertiesOption::Update is equivalent to the @ref ECSQLOptions "ECSQLOPTION" @b ReadonlyPropertiesAreUpdatable.
                //! Therefore, do not use the ECSQLOPTION @c ReadonlyPropertiesAreUpdatable here. Options::IsValid returns false in that case. Use the JsonUpdater::ReadonlyPropertiesOption argument instead.
                //! @param[in] readonlyPropertiesOption Determines how read-only properties are to be handled by the updater.
                //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the JsonUpdater.
                //!            Pass without ECSQLOPTIONS keyword.
                explicit Options(ReadonlyPropertiesOption readonlyPropertiesOption, Utf8CP ecsqlOptions = nullptr) : Options(SystemPropertiesOption::Fail, readonlyPropertiesOption, ecsqlOptions) {}

                //! Initializes a new Options object.
                //! @remarks ReadonlyPropertiesOption::Update is equivalent to the @ref ECSQLOptions "ECSQLOPTION" @b ReadonlyPropertiesAreUpdatable.
                //! Therefore, do not use the ECSQLOPTION @c ReadonlyPropertiesAreUpdatable here. Options::IsValid returns false in that case. Use the JsonUpdater::ReadonlyPropertiesOption argument instead.
                //! @param[in] systemPropertiesOption Determines how system properties are to be handled by the updater.
                //! @param[in] readonlyPropertiesOption Determines how read-only properties are to be handled by the updater.
                //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the JsonUpdater.
                //!            Pass without ECSQLOPTIONS keyword.
                ECDB_EXPORT Options(SystemPropertiesOption systemPropertiesOption, ReadonlyPropertiesOption readonlyPropertiesOption, Utf8CP ecsqlOptions = nullptr);

                bool IsValid() const { return m_isValid; }
                //! Default: JsonUpdater::SystemPropertiesOption::Fail
                SystemPropertiesOption GetSystemPropertiesOption() const { return m_systemProps; }
                //! Default: JsonUpdater::ReadonlyPropertiesOption::Fail
                ReadonlyPropertiesOption GetReadonlyPropertiesOption() const { return m_readonlyProps; }
                //! Default: unset
                Utf8StringCR GetECSqlOptions() const { return m_ecsqlOptions; }
            };

    private:
        struct BindingInfo final
            {
            private:
                uint32_t m_parameterIndex = 0;
                ECN::ECPropertyCP m_property = nullptr;
            public:
                BindingInfo() {}
                BindingInfo(uint32_t paramIndex, ECN::ECPropertyCR prop) : m_parameterIndex(paramIndex), m_property(&prop) {}

                bool SkipBinding() const { return m_parameterIndex == 0; }

                uint32_t GetParameterIndex() const { return m_parameterIndex; }
                ECN::ECPropertyCR GetProperty() const { BeAssert(m_property != nullptr); return *m_property; }
            };

        struct CompareIUtf8Ascii
            {
            bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::StricmpAscii(s1, s2) < 0; }
            };

        ECDbCR m_ecdb;
        ECN::ECClassCR m_ecClass;
        Options m_options;

        Utf8String m_jsonClassName;
        mutable ECSqlStatement m_statement;
        bmap<Utf8CP, BindingInfo, CompareIUtf8Ascii> m_bindingMap;
        int m_idParameterIndex = 0;
        bool m_isValid = false;

        //not copyable
        JsonUpdater(JsonUpdater const&) = delete;
        JsonUpdater& operator=(JsonUpdater const&) = delete;

        BentleyStatus Initialize(bvector<Utf8CP> const* propertyNames, ECCrudWriteToken const*);

    public:
        //! Initializes a new JsonUpdater instance for the specified class.
        //! @remarks The SET clause of the underlying ECSQL UPDATE will contain @b all properties of @p ecClass.
        //! That means, any properties not contained in the incoming JSON will be nulled-out. As this is often not the desired
        //! behavior, consider using the other constructor that takes a list of property names.
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be updated. 
        //! @param[in] writeToken Token required to execute ECSQL UPDATE statements if 
        //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
        //! If the option is not set, nullptr can be passed for @p writeToken.
        //! @param[in] options Options that control the behavior of the JsonUpdater
        ECDB_EXPORT JsonUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, Options const& options = Options());

        //! Initializes a new JsonUpdater instance for the specified class. 
        //! @param[in] ecdb ECDb
        //! @param[in] ecClass ECClass of the instance that needs to be updated. 
        //! @param[in] propertyNames ECClass of the instance that needs to be updated. 
        //! @param[in] writeToken Token required to execute ECSQL UPDATE statements if 
        //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
        //! If the option is not set, nullptr can be passed for @p writeToken.
        //! @param[in] options Options that control the behavior of the JsonUpdater
        ECDB_EXPORT JsonUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, bvector<Utf8CP> const& propertyNames, ECCrudWriteToken const* writeToken, Options const& options = Options());

        //! Indicates whether this JsonUpdater is valid and can be used to update JSON instances.
        //! It is not valid, if the underlying ECClass is abstract or not mapped for example.
        //! @return true if the updater is valid and can be used for updating. false if it cannot be used for updating.
        bool IsValid() const { return m_isValid; }

        //! Updates an instance from the specified ECJSON
        //! @param[in] instanceId the ECInstanceId of the instance to update
        //! @param[in] json the instance data
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        ECDB_EXPORT DbResult Update(ECInstanceId instanceId, JsonValueCR json) const;

        //! Update an instance from the specified ECJSON
        //! @param[in] instanceId the ECInstanceId of the instance to update
        //! @param[in] json the instance data
        //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
        //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
        ECDB_EXPORT DbResult Update(ECInstanceId instanceId, RapidJsonValueCR json) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
