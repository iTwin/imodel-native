/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECInstanceAdapter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECSqlStatement.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//======================================================================================
//! Allows reading of EC data from an @ref ECDbFile "ECDb file" 
//! directly as @ref ECN::IECInstance "ECInstances". 
//! Works with the @ref ECSqlStatement to adapt the rows read from the %ECDb file into 
//! @ref ECN::IECInstance "ECInstances".
//! Note: 
//!     * When using Polymorphic queries, the SELECT statement must include the ECClassId
//! column in order to correctly determine the derived class.  Example: "Select Prop1, Prop2, ECClassId FROM"
//! @see ECSqlStatement
//! @ingroup ECDbGroup
// @bsiclass                                                 Carole.MacDonald      08/2013
//+===============+===============+===============+===============+===============+======
struct ECInstanceECSqlSelectAdapter final
    {
private:
    typedef BentleyStatus(ECInstanceECSqlSelectAdapter::*ColumnHandler)(ECN::IECInstanceR instance, IECSqlValue const& value) const;
    bvector<ColumnHandler> m_columnHandlers;

    ECSqlStatement const& m_ecSqlStatement;
    bool m_isValid;
    int m_ecClassIdColumnIndex;
    int m_sourceECClassIdColumnIndex;
    int m_targetECClassIdColumnIndex;
    bool m_isSingleClassSelectClause;

    //not copyable
    ECInstanceECSqlSelectAdapter(ECInstanceECSqlSelectAdapter const&) = delete;
    ECInstanceECSqlSelectAdapter& operator=(ECInstanceECSqlSelectAdapter const&) = delete;

    BentleyStatus SetInstanceId(ECN::IECInstanceR instance, IECSqlValue const& value) const;
    BentleyStatus SetInstanceData(ECN::IECInstanceR instance, bool usesClassIdFilter) const;
    BentleyStatus SetPropertyData(ECN::IECInstanceR instance, Utf8CP parentPropertyAccessString, IECSqlValue const& value) const;
    BentleyStatus SetPropertyData(ECN::IECInstanceR instance, IECSqlValue const& value) const { return SetPropertyData(instance, nullptr, value); }
    BentleyStatus SetRelationshipSource(ECN::IECInstanceR instance, IECSqlValue const& value) const;
    BentleyStatus SetRelationshipTarget(ECN::IECInstanceR instance, IECSqlValue const& value) const;

    BentleyStatus SetStructArrayElement(ECN::ECValueR val, ECN::ECClassCR structType, IECSqlValue const& value) const;
    BentleyStatus SetPrimitiveValue(ECN::ECValueR val, ECN::PrimitiveType primitiveType, IECSqlValue const& value) const;
    BentleyStatus SetNavigationValue(ECN::IECInstanceR instance, IECSqlValue const& value) const;
    ECN::IECInstancePtr FindRelationshipEndpoint(ECInstanceId endpointInstanceId, ECN::ECClassId endpointClassId, ECN::StandaloneECRelationshipInstance*, bool isSource) const;
    BentleyStatus CreateColumnHandlers();

public:
    //! Creates a new instance of the adapter
    //! @param[in] statement Prepared ECSqlStatement
    ECDB_EXPORT explicit ECInstanceECSqlSelectAdapter(ECSqlStatement const& statement);

    //! Indicates whether this ECInstanceECSqlSelectAdapter is valid and can be used to retrieve ECInstances.
    //! For example, it is not valid, if the adapter was created with an unprepared ECSqlStatement.
    bool IsValid() const { return m_isValid; }

    //! Creates an IECInstancePtr from the current row of the ECSqlStatement.  
    //! This method can only be used if the ECSQL select clause is made up of properties of a single ECClass.
    //! If the ECSQL select clause is made up of properties from more than one ECClass, this method
    //! returns nullptr. Consider calling ECInstanceECSqlSelectAdapter::GetInstance(ECN::ECClassId) instead.
    //! @return the ECInstance from the current row, or nullptr in case of errors
    ECDB_EXPORT ECN::IECInstancePtr GetInstance() const;

    //! Populates the specified @p ecInstance from the current row of the ECSqlStatement.  
    //! @remarks This overload should be used if callers want the adapter to populate non-default IECInstance implementations.
    //! If there are properties from multiple ECClasses in the row,
    //! only those from the ECClass of @p ecInstance are used and all others are ignored.
    //! This implies that @p ecInstance remains unmodified if non of the SELECT clause items matches with the properties
    //! of @p ecInstance.
    //! @param[out] ecInstance ECInstance to populate
    //! @return SUCCESS or ERROR
    ECDB_EXPORT BentleyStatus GetInstance(ECN::IECInstanceR ecInstance) const;

    //! Creates an IECInstancePtr from the current row of the ECSqlStatement for the given ECClass.  
    ECDB_EXPORT ECN::IECInstancePtr GetInstance(ECN::ECClassId) const;

    //! Retrieves the ECInstanceId from the current row in the ECSqlStatement.  
    //! The SELECT clause must specifically request the ECInstanceId property in order for this to work
    //! (unless doing 'SELECT *', in which case the ECInstanceId will be retrieved automatically).
    //! @param[out] id  the ECInstanceId of the instance for the current row
    //! @returns SUCCESS or ERROR
    ECDB_EXPORT BentleyStatus GetInstanceId(ECInstanceId& id) const;
    };

//======================================================================================
//! Inserts ECInstances in the form of ECN::IECInstance into the ECDb file.
//! This is a convenience class that adapts the input ECN::IECInstance object to an ECSQL
//! statement.
//! @see ECSqlStatement
//! @ingroup ECDbGroup
// @bsiclass                                                 Carole.MacDonald      01/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceInserter final
    {
#if !defined (DOCUMENTATION_GENERATOR)
public:
    struct Impl;
#endif

private:
    Impl* m_impl;

    //not copyable
    ECInstanceInserter(ECInstanceInserter const&) = delete;
    ECInstanceInserter& operator=(ECInstanceInserter const&) = delete;

public:
    //! Instantiates a new ECInstanceInserter.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass of ECInstances this inserter can insert
    //! @param [in] writeToken Token required to execute ECSQL INSERT statements if 
    //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
    //! If the option is not set, nullptr can be passed for @p writeToken.
    ECDB_EXPORT ECInstanceInserter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken);
    ECDB_EXPORT ~ECInstanceInserter();

    //! Indicates whether this ECInstanceInserter is valid and can be used to insert ECInstances.
    //! It is not valid, if @p ecClass is not mapped or not instantiable for example.
    //! @return true if the inserter is valid and can be used for inserting. false if it cannot be used for inserting.
    ECDB_EXPORT bool IsValid() const;

    //! Inserts an instance into the @ref ECDbFile "ECDb file".
    //! @remarks @p instance can either be a regular or a relationship instance. However, ECInstanceInserter::InsertRelationship
    //! is often more convenient when inserting relationships, especially if the relationship doesn't have any ECProperties.
    //! @param[out] newInstanceKey ECInstanceKey of the inserted instance.
    //! @param[in] instance Instance to insert
    //! @param[in] autogenerateECInstanceId true, if ECDb should auto-generate an ECInstanceId (default),
    //!            false, if ECDb should not auto-generate the ECInstanceId. In that case @p userProvidedECInstanceId,
    //!            or if null, the instance id of @p instance will be used.
    //! @param[in] userProvidedECInstanceId User provided ECInstanceId. Pass nullptr if @p autogenerateECInstanceId is true or if
    //!            the instance id of @p instance should be used
    //! @note Disabling auto-generation should be used with care. It is only needed in exceptional cases.
    //! When disabling auto-generation the caller is responsible for handling primary key constraint 
    //! violations, and generally uniqueness of ECInstanceIds within the ECDb file is no longer guaranteed.
    //! @return BE_SQLITE_OK in case of success, error codes otherwise
    ECDB_EXPORT DbResult Insert(ECInstanceKey& newInstanceKey, ECN::IECInstanceCR instance, bool autogenerateECInstanceId = true, ECInstanceId const* userProvidedECInstanceId = nullptr) const;

    //! Inserts a relationship instance into the @ref ECDbFile "ECDb file".
    //! @remarks This is a convenience method that allows you to avoid having to construct a fully-defined ECN::IECRelationshipInstance
    //! with source and target ECInstances to be set. 
    //! @param[out] newInstanceKey ECInstanceKey of the inserted instance.
    //! @param[in] sourceId SourceECInstanceId
    //! @param[in] targetId TargetECInstanceId
    //! @param[in] relationshipProperties If the ECN::ECRelationshipClass has ECProperties, pass its values via this parameter. Note: In this
    //! case @ref ECN::IECRelationshipInstance::GetSource "IECRelationshipInstance::GetSource" and @ref ECN::IECRelationshipInstance::GetTarget "IECRelationshipInstance::GetTarget"
    //! don't have to be set in @p relationshipProperties
    //! @param[in] autogenerateECInstanceId true, if ECDb should auto-generate an ECInstanceId (default),
    //!            false, if ECDb should not auto-generate the ECInstanceId. In that case @p userProvidedECInstanceId
    //!            will be used.
    //! @param[in] userProvidedECInstanceId User provided ECInstanceId. Pass nullptr if @p autogenerateECInstanceId is true
    //! @note Disabling auto-generation should be used with care. It is only needed in exceptional cases.
    //! When disabling auto-generation the caller is responsible for handling primary key constraint 
    //! violations, and generally uniqueness of ECInstanceIds within the ECDb file is no longer guaranteed.
    //! @return BE_SQLITE_OK in case of success, error codes otherwise
    ECDB_EXPORT DbResult InsertRelationship(ECInstanceKey& newInstanceKey, ECInstanceId sourceId, ECInstanceId targetId, ECN::IECRelationshipInstanceCP relationshipProperties = nullptr, bool autogenerateECInstanceId = true, ECInstanceId const* userProvidedECInstanceId = nullptr) const;

    //! Inserts an instance into the @ref ECDbFile "ECDb file".
    //! @param[in, out] instance Instance to insert. If @p autogenerateECInstanceId is true, 
    //!            the generated ECInstanceId will be set in @p instance
    //! @param[in] autogenerateECInstanceId true, if ECDb should auto-generate an ECInstanceId (default),
    //!            false, if ECDb should not auto-generate the ECInstanceId. In that case the instance id of @p instance will be used
    //! @note Disabling auto-generation should be used with care. It is only needed in exceptional cases.
    //! When disabling auto-generation the caller is responsible for handling primary key constraint 
    //! violations, and generally uniqueness of ECInstanceIds within the ECDb file is no longer guaranteed.
    //! @return BE_SQLITE_OK in case of success, error codes otherwise
    ECDB_EXPORT DbResult Insert(ECN::IECInstanceR instance, bool autogenerateECInstanceId = true) const;
    };

//======================================================================================
//! Updates ECInstances in the ECDb file.
//! This is a convenience class that adapts the input ECN::IECInstance object to an ECSQL
//! statement.
//! @see ECSqlStatement
//! @ingroup ECDbGroup
// @bsiclass                                                 Carole.MacDonald      01/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceUpdater final
    {
#if !defined (DOCUMENTATION_GENERATOR)
public:
    struct Impl;
#endif

private:
    Impl const* m_impl;

    //not copyable
    ECInstanceUpdater(ECInstanceUpdater const&) = delete;
    ECInstanceUpdater& operator=(ECInstanceUpdater const&) = delete;

public:
    //! Instantiates a new ECInstanceUpdater.
    //! @remarks Check ECInstanceUpdater::IsValid afterwards to make sure the updater was created successfully
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass of ECInstances this updater can update
    //! @param [in] writeToken Token required to execute ECSQL UPDATE statements if 
    //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
    //! If the option is not set, nullptr can be passed for @p writeToken.
    //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the ECInstanceUpdater.
    //!            Pass without ECSQLOPTIONS keyword.
    ECDB_EXPORT ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions = nullptr);

    //! Instantiates a new ECInstanceUpdater.
    //! @remarks Check ECInstanceUpdater::IsValid afterwards to make sure the updater was created successfully
    //! @param[in] ecdb ECDb file handle
    //! @param[in] instance The property values that are set on this IECInstance will be used to create column bindings.
    //! @param [in] writeToken Token required to execute ECSQL UPDATE statements if 
    //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
    //! If the option is not set, nullptr can be passed for @p writeToken.
    //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the ECInstanceUpdater.
    //!            Pass without ECSQLOPTIONS keyword.
    //! @remarks All instances that subsequently use this Updater are presumed to have the same property values set.
    ECDB_EXPORT ECInstanceUpdater(ECDbCR ecdb, ECN::IECInstanceCR instance, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions = nullptr);

    //! Instantiates a new ECInstanceUpdater.
    //! @remarks Check ECInstanceUpdater::IsValid afterwards to make sure the updater was created successfully
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass if ECInstances this updater can update
    //! @param [in] writeToken Token required to execute ECSQL UPDATE statements if 
    //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
    //! If the option is not set, nullptr can be passed for @p writeToken.
    //! @param[in] propertyIndexesToBind A list of property indices that should be used to create the column bindings.  Properties are assumed to all come from the same class.
    //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the ECInstanceUpdater.
    //!            Pass without ECSQLOPTIONS keyword.
    //! @remarks All instances that subsequently use this Updater are presumed to have the same property values set.
    ECDB_EXPORT ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions = nullptr);

    //! Instantiates a new ECInstanceUpdater.
    //! @remarks Check ECInstanceUpdater::IsValid afterwards to make sure the updater was created successfully
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass if ECInstances this updater can update
    //! @param [in] writeToken Token required to execute ECSQL UPDATE statements if 
    //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
    //! If the option is not set, nullptr can be passed for @p writeToken.
    //! @param[in] propertiesToBind A list of ECProperties that should be used to create the column bindings.
    //! @param[in] ecsqlOptions ECSQLOPTIONS clause appended to the ECSQL generated by the ECInstanceUpdater.
    //!            Pass without ECSQLOPTIONS keyword.
    //! @remarks All instances that subsequently use this Updater are presumed to have the same property values set.
    ECDB_EXPORT ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, bvector<ECN::ECPropertyCP> const& propertiesToBind, Utf8CP ecsqlOptions = nullptr);

    ECDB_EXPORT ~ECInstanceUpdater();

    //! Indicates whether this ECInstanceUpdater is valid and can be used to update ECInstances.
    //! It is not valid, if @p ecClass is not mapped for example.
    //! @return true if the updater is valid and can be used for updating. false if it cannot be used for updating.
    ECDB_EXPORT bool IsValid() const;


    //! Updates the data in the ECDb file that corresponds to the specified ECInstance.
    //! @remarks If the input ECInstance @p instance contains values for readonly ECProperties, those values will be ignored.
    //! Values for calculated properties however will be updated as ECDb cannot evaluate calculated properties itself.
    //! Passing the ECSQL option @b ReadonlyPropertiesAreUpdatable to the constructor of the updater, modifies the default
    //! and values of readonly properties are updated.
    //! @param[in] instance ECInstance for which the corresponding row is to be updated.
    //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL UPDATE. This means,
    //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
    ECDB_EXPORT DbResult Update(ECN::IECInstanceCR instance) const;
    };

//======================================================================================
//! Deletes ECInstances in the ECDb file.
//! This is a convenience class that adapts the input ECN::IECInstance object to an ECSQL
//! statement.
//! @see ECSqlStatement
//! @ingroup ECDbGroup
// @bsiclass                                                 Carole.MacDonald      02/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceDeleter final
    {
private:
    ECDbCR m_ecdb;
    ECN::ECClassCR m_ecClass;
    mutable ECSqlStatement m_statement;
    bool m_isValid;

    //not copyable
    ECInstanceDeleter(ECInstanceDeleter const&) = delete;
    ECInstanceDeleter& operator=(ECInstanceDeleter const&) = delete;

    void Initialize(ECCrudWriteToken const*);

public:
    //! Instantiates a new ECInstanceDeleter.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass of ECInstances this deleter can delete
    //! @param [in] writeToken Token required to execute ECSQL DELETE statements if 
    //! the ECDb file was set-up with the "require ECSQL write token" option (for example all DgnDb files require the token).
    //! If the option is not set, nullptr can be passed for @p writeToken.
    ECDB_EXPORT ECInstanceDeleter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken);
    ~ECInstanceDeleter() {}

    //! Indicates whether this ECInstanceDeleter is valid and can be used to delete ECInstances.
    //! It is not valid, if @p ecClass is not mapped for example.
    //! @return true if the deleter is valid and can be used for deleting. false if it cannot be used for delete.
    bool IsValid() const { return m_isValid; }

    //! Deletes the ECInstance with the specified ECInstanceId.
    //! @param[in] ecInstanceId Id of the ECInstance to delete
    //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL. This means,
    //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
    ECDB_EXPORT DbResult Delete(ECInstanceId ecInstanceId) const;

    //! Deletes the given ECInstance.
    //! @param[in] ecInstance ECInstance to delete
    //! @return BE_SQLITE_OK in case of successful execution of the underlying ECSQL. This means,
    //! BE_SQLITE_OK is also returned if the specified ECInstance does not exist in the file. Error codes otherwise.
    ECDB_EXPORT DbResult Delete(ECN::IECInstanceCR ecInstance) const;
    };
END_BENTLEY_SQLITE_EC_NAMESPACE