/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECInstanceAdapter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECSqlStatement.h>
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//======================================================================================
//! Allows reading of EC data from an @ref ECDbFile "ECDb file" 
//! directly as @ref ECN::IECInstance "ECInstances". 
//! Works with the @ref ECSqlStatement to adapt the rows read from the %ECDb file into 
//! @ref ECN::IECInstance "ECInstances".
//! Note: When using Polymorphic queries, the SELECT statement must include the ClassId 
//! column in order to correctly determine the derived class.  Example: "Select *, GetECClassId() AS ECClassId"
//! @see ECSqlStatement
//! @ingroup ECDbGroup
// @bsiclass                                                 Carole.MacDonald      08/2013
//+===============+===============+===============+===============+===============+======
struct ECInstanceECSqlSelectAdapter : NonCopyableClass
    {
private:
    typedef BentleyStatus (ECInstanceECSqlSelectAdapter::*ColumnHandler)(ECN::IECInstanceR instance, IECSqlValue const& value) const;
    bvector<ColumnHandler> m_columnHandlers;

    ECSqlStatementCR m_ecSqlStatement;
    bool m_initialized;
    int m_sourceECClassIdColumnIndex;
    int m_targetECClassIdColumnIndex;
    int m_ecClassIdColumnIndex;
    bool m_isSingleClassSelectClause;

    //__PUBLISH_SECTION_END__
    bool Initialize ();
    void CreateColumnHandlers ();
    //column handlers
    BentleyStatus SetInstanceId (ECN::IECInstanceR instance, IECSqlValue const& value) const;
    BentleyStatus SetPropertyData (ECN::IECInstanceR instance, IECSqlValue const& value) const;
    BentleyStatus SetPropertyData (ECN::IECInstanceR instance, WCharCP parentPropertyAccessString, IECSqlValue const& value) const;
    BentleyStatus SetRelationshipSource (ECN::IECInstanceR instance, IECSqlValue const& value) const;
    BentleyStatus SetRelationshipTarget (ECN::IECInstanceR instance, IECSqlValue const& value) const;

    BentleyStatus SetInstanceData (ECN::IECInstanceR instance, bool usesClassIdFilter) const;
    BentleyStatus SetStructArrayElement (ECN::ECValueR val, ECN::ECClassCR structType, IECSqlValue const& value) const;
    BentleyStatus SetPrimitiveValue (ECN::ECValueR val, ECN::PrimitiveType primitiveType, IECSqlValue const& value) const;
    ECN::IECInstancePtr FindRelationshipEndpoint (int64_t endpointInstanceId, int64_t endpointClassId, ECN::StandaloneECRelationshipInstance*, bool isSource) const;
    ECN::IECInstancePtr FindRelationshipEndpoint (int64_t endpointInstanceId, ECN::ECClassCP endpointClass) const;

    //__PUBLISH_SECTION_START__
public:
    //! Creates a new instance of the adapter
    //! @param[in] ecSqlStatement Prepared statement
    //! @see ECSqlStatement
    ECDB_EXPORT ECInstanceECSqlSelectAdapter (ECSqlStatementCR ecSqlStatement);

    //! Creates an IECInstancePtr from the current row of the ecSqlStatement.  
    //! This method can only be used if the ECSQL select clause is made up of properties of a single ECClass.
    //! If the ECSQL select clause is made up of properties from more than one ECClass, this method
    //! returns nullptr. Consider calling ECInstanceECSqlSelectAdapter::GetInstance(ECN::ECClassId) instead.
    //! @return the ECInstance from the current row, or nullptr in case of errors
    ECDB_EXPORT ECN::IECInstancePtr GetInstance () const;

    //! Creates an IECInstancePtr from the current row of the ecSqlStatement for the given ECClass.  If there are properties from multiple ECClasses in the row, 
    //! only those from the requested ECClass are used and all others are ignored
    ECDB_EXPORT ECN::IECInstancePtr GetInstance (ECN::ECClassId ecClassid) const;

    //! Retrieves the ECInstanceId from the current row in the ecSqlStatement.  
    //! The SELECT clause must specifically request the ECInstanceId property in order for this to work
    //! (unless doing 'SELECT *', in which case the ECInstanceId will be retrieved automatically).
    //! @param[out] id  the ECInstanceId of the instance for the current row
    //! @returns true if the ECInstanceId was found in the current row, false otherwise
    ECDB_EXPORT bool GetInstanceId (ECInstanceId& id) const;
    };

//======================================================================================
//! Inserts ECInstances in the form of ECN::IECInstance into the ECDb file.
//! This is a convenience class that adapts the input ECN::IECInstance object to an ECSQL
//! statement.
//! @see ECSqlStatement
//! @ingroup ECDbGroup
// @bsiclass                                                 Carole.MacDonald      01/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceInserter : NonCopyableClass
    {
#if !defined (DOCUMENTATION_GENERATOR)
public:
    struct Impl;
#endif

private:
    Impl* m_impl;

public:
    //! Instantiates a new ECInstanceInserter.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass of ECInstances this inserter can insert
    ECDB_EXPORT ECInstanceInserter (ECDbCR ecdb, ECN::ECClassCR ecClass);
    ECDB_EXPORT ~ECInstanceInserter ();

    //! Indicates whether this ECInstanceInserter is valid and can be used to insert ECInstances.
    //! It is not valid, if @p ecClass is not mapped or not instantiable for example.
    //! @return true if the inserter is valid and can be used for inserting. false if it cannot be used for inserting.
    ECDB_EXPORT bool IsValid () const;

    //! Inserts an instance into the @ref ECDbFile "ECDb file".
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
    //! @return SUCCESS in case of success, ERROR otherwise
    ECDB_EXPORT BentleyStatus Insert (ECInstanceKey& newInstanceKey, ECN::IECInstanceCR instance, bool autogenerateECInstanceId = true, ECInstanceId const* userProvidedECInstanceId = nullptr) const;

    //! Inserts an instance into the @ref ECDbFile "ECDb file".
    //! @param[in, out] instance Instance to insert. If @p autogenerateECInstanceId is true, 
    //!            the generated ECInstanceId will be set in @p instance
    //! @param[in] autogenerateECInstanceId true, if ECDb should auto-generate an ECInstanceId (default),
    //!            false, if ECDb should not auto-generate the ECInstanceId. In that case the instance id of @p instance will be used
    //! @note Disabling auto-generation should be used with care. It is only needed in exceptional cases.
    //! When disabling auto-generation the caller is responsible for handling primary key constraint 
    //! violations, and generally uniqueness of ECInstanceIds within the ECDb file is no longer guaranteed.
    //! @return SUCCESS in case of success, ERROR otherwise
    ECDB_EXPORT BentleyStatus Insert (ECN::IECInstanceR instance, bool autogenerateECInstanceId = true) const;
    };

//======================================================================================
//! Updates ECInstances in the ECDb file.
//! This is a convenience class that adapts the input ECN::IECInstance object to an ECSQL
//! statement.
//! @see ECSqlStatement
//! @ingroup ECDbGroup
// @bsiclass                                                 Carole.MacDonald      01/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceUpdater : NonCopyableClass
    {
#if !defined (DOCUMENTATION_GENERATOR)
public:
    struct Impl;
#endif

private:
    Impl const* m_impl;

public:
    //! Instantiates a new ECInstanceUpdater.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass of ECInstances this updater can update
    ECDB_EXPORT ECInstanceUpdater (ECDbCR ecdb, ECN::ECClassCR ecClass);

    //! Instantiates a new ECInstanceUpdater.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] instance The property values that are set on this IECInstance will be used to create column bindings.
    //! @remarks All instances that subsequently use this Updater are presumed to have the same property values set.
    ECDB_EXPORT ECInstanceUpdater (ECDbCR ecdb, ECN::IECInstanceCR instance);

    //! Instantiates a new ECInstanceUpdater.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass if ECInstances this updater can update
    //! @param[in] propertiesToBind A list of property indices that should be used to create the column bindings.  Properties are assumed to all come from the same class.
    //! @remarks All instances that subsequently use this Updater are presumed to have the same property values set.
    ECDB_EXPORT ECInstanceUpdater (ECDbCR ecdb, ECN::ECClassCR ecClass, bvector<uint32_t>& propertiesToBind);

    ECDB_EXPORT ~ECInstanceUpdater ();

    //! Indicates whether this ECInstanceUpdater is valid and can be used to update ECInstances.
    //! It is not valid, if @p ecClass is not mapped for example.
    //! @return true if the updater is valid and can be used for updating. false if it cannot be used for updating.
    ECDB_EXPORT bool IsValid () const;


    //! Updates the data in the ECDb file that corresponds to the specified ECInstance.
    //! @param[in] instance ECInstance for which the corresponding row is to be updated.
    //! @return SUCCESS in case of successful update. ERROR if no ECInstance existed in 
    //! the ECDb file for the specified @p instance or in case of other errors.
    ECDB_EXPORT BentleyStatus Update (ECN::IECInstanceCR instance) const;
    };

//======================================================================================
//! Deletes ECInstances in the ECDb file.
//! This is a convenience class that adapts the input ECN::IECInstance object to an ECSQL
//! statement.
//! @see ECSqlStatement
//! @ingroup ECDbGroup
// @bsiclass                                                 Carole.MacDonald      02/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceDeleter : NonCopyableClass
    {
public:

private:
    ECDbCR m_ecdb;
    ECN::ECClassCR m_ecClass;
    mutable ECSqlStatement m_statement;
    bool m_isValid;

    //__PUBLISH_SECTION_END__
    void Initialize();
    //__PUBLISH_SECTION_START__

public:
    //! Instantiates a new ECInstanceDeleter.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] ecClass ECClass of ECInstances this deleter can delete
    ECDB_EXPORT ECInstanceDeleter(ECDbCR ecdb, ECN::ECClassCR ecClass);
;

    ECDB_EXPORT ~ECInstanceDeleter ();

    //! Indicates whether this ECInstanceDeleter is valid and can be used to delete ECInstances.
    //! It is not valid, if @p ecClass is not mapped for example.
    //! @return true if the deleter is valid and can be used for deleting. false if it cannot be used for delete.
    ECDB_EXPORT bool IsValid () const;

    //! Deletes the ECInstance with the specified ECInstanceId.
    //! @param[in] ecInstanceId Id of the ECInstance to delete
    //! @return SUCCESS in case of successful deletion. ERROR if no ECInstance existed for
    //! the specified @p ecInstanceId or in case of other errors.
    ECDB_EXPORT BentleyStatus Delete (ECInstanceId const& ecInstanceId) const;

    //! Deletes the given ECInstance.
    //! @param[in] ecInstance ECInstance to delete
    //! @return SUCCESS in case of successful deletion. ERROR if no ECInstance existed for
    //! the specified @p ecInstance or in case of other errors.
    ECDB_EXPORT BentleyStatus Delete (ECN::IECInstanceCR ecInstance) const;
    };
END_BENTLEY_SQLITE_EC_NAMESPACE