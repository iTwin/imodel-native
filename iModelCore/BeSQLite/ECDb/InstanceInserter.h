/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/InstanceInserter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "SqlGenerator.h"

ECDB_TYPEDEFS_PTR (InstancePropertyToTableInserter);
ECDB_TYPEDEFS_PTR (PrimaryKeyGenerator);
ECDB_TYPEDEFS_PTR (BindRelationshipEnd);
ECDB_TYPEDEFS_PTR (BindRelationshipEndInstanceId);

ECDB_TYPEDEFS (Binding);
ECDB_TYPEDEFS2 (bvector<Binding>, Bindings);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define BINDING_NotBound -1

//=======================================================================================
//! @deprecated Only used by deprecated ECPersistence
//!
//! Holds binding metadata for mapping ECProperties to sql parameters or selected columns
//! in ECDbStatements
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct Binding
    {
    //! For looking up the bound/selected property in an IECInstance
    uint32_t            m_propertyIndex;

    //! Either a parameter index or column index, depending on whether the binding is used for parameters or selected columns
    //! May be BINDING_NotBound for unmapped columns
    int                 m_sqlIndex;

    //! The PropertyMap used to map, bind, and get values associated with this binding
    PropertyMapCR       m_propertyMap;

    //! The actual column that is bound
    DbColumnCP          m_column;

    //! Used to indicate which 'components' of a property are applicable
    uint16_t            m_componentMask;

    //! The enabler for which the m_propertyIndex is valid. Otherwise use propertyAccessString
    ECN::ECEnablerCR    m_enabler;

    //! primary constructor
    Binding (ECN::ECEnablerCR enabler, PropertyMapCR propertyMap, uint32_t propertyIndex, uint16_t componentMask, int sqlIndex, DbColumnCP column);

    //! Copy constructor for bvector
    Binding (BindingCR other);

    //! Needed for use in a bvector
    BindingCR operator= (BindingCR other);

    //! Binds the parameters of the statement with corresponding property values from the ecInstance
    static DbResult Bind (BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance, BindingsCR parameterBindings);
    };


/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct PrimaryKeyGenerator : RefCountedBase
    {
private:
//    DbColumnR                    m_primaryKeyColumn; -- detected as unused by clang
    int                          m_parameterIndex;
    BeRepositoryBasedIdSequenceR m_ecInstanceIdSequence;

    PrimaryKeyGenerator (int& nextParameterIndex, DbColumnR primaryKeyColumn, BeRepositoryBasedIdSequenceR ecInstanceIdSequence);

protected:
    virtual BeSQLite::DbResult _Generate (ECInstanceId* ecInstanceId, BeSQLite::Statement& statement, ECN::IECInstanceR instance, bool useSuppliedECInstanceId) const;
public:
    static PrimaryKeyGeneratorPtr Create (int& nextParameterIndex, DbColumnR primaryKeyColumn, BeRepositoryBasedIdSequenceR ecInstanceIdSequence);
    BeSQLite::DbResult Generate (ECInstanceId* ecInstanceLuid, BeSQLite::Statement& statement, ECN::IECInstanceR instance, bool useSuppliedECInstanceId) const;
    };

/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct InstanceInserter : RefCountedBase
{
typedef bvector<InstancePropertyToTableInserterPtr> PropertyToTableInserters;
private:
    PrimaryKeyGeneratorPtr      m_primaryKeyGenerator;

protected:
    SqlInsert                    m_insertBuilder;
    Utf8String                   m_sqlString;
    ECDbMapCR                    m_ecDbMap;
    ECN::ECClassCR               m_ecClass;
    BeRepositoryBasedIdSequenceR m_ecInstanceIdSequence;
    BeSQLite::Statement          m_statement;
    Bindings                     m_parameterBindings;
    PropertyToTableInserters     m_propertyToTableInserters;
    bool                         m_isPropertyToTableInsertersSetup;
    InstanceInserterCP           m_parentInserter;
private:
    ECN::ECObjectsStatus         CopyStruct (ECN::IECInstanceR ecInstance, ECN::ECPropertyValueCR propertyValue);

protected:
    InstanceInserter            (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence, InstanceInserterCP m_parentInserter);
    virtual                     ~InstanceInserter () {}

    // Initialize
    virtual InsertStatus        _Initialize();
    void                        AppendSqlToInsertIntoTable (DbTableCR table);
    InsertStatus                AppendSqlToInsertPrimaryKeyOfInstance (int& nextParameterIndex, DbTableCR table, bool setupPrimaryKeyGenerator);
    void                        AppendSqlToInsertECClassId (ECN::ECClassId classId, DbTableCR table);
    InsertStatus                AppendSqlToInsertPropertyValues (int& nextParameterIndex, IClassMap const& classMap);
    InsertStatus                TrySetupPropertyToTableInserters (IClassMap const& classMap);

    // Bind
    virtual BeSQLite::DbResult  _Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId);
    BeSQLite::DbResult          BindPrimaryKeysOfInstance (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId);
    BeSQLite::DbResult          BindPropertyValues (ECN::IECInstanceR ecInstance);
    
    // Insert
    virtual InsertStatus        _Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId);
    InsertStatus                DoInsertionsInSecondaryTables (ECInstanceId* ecInstanceLuid, ECN::IECInstanceR ecInstance);

    BeRepositoryBasedIdSequenceR GetECInstanceIdSequence () const;
    InstanceInserterCP           GetParent () const { return m_parentInserter; }
    //! Ensures that m_parameterBindings agrees with m_insertBuilder.GetParameterCount();
    InsertStatus VerifyExpectedNumberOfBindings (IClassMap const& classMap);

public:
    ECN::ECClassCR              GetClass () const { return m_ecClass; }
    static InstanceInserterPtr  Create (InsertStatus& status, ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence, InstanceInserterCP parentInserter);
    InsertStatus                Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance);
    InsertStatus                Insert (ECN::IECInstanceR ecInstance, ECInstanceId ecInstanceId);
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct InstancePropertyToTableInserter : InstanceInserter
{
friend struct InstanceInserter;
private:
    ECN::ECPropertyCR            m_ecProperty;
    WString                      m_ecPropertyAccessString;
    ECN::ECPropertyId            m_propertyIdForPersistence;
private:
    InstancePropertyToTableInserter (ECDbMapCR ecDbMap, PropertyMapToTableCR propertyMapToTable, BeRepositoryBasedIdSequenceR ecInstanceIdSequence, InstanceInserterCP parentInserter);

    // Initialize
    virtual InsertStatus        _Initialize();
    InsertStatus                AppendSqlToInsertPrimaryKeysForProperty (WStringR sql, WStringR values, int& nextParameterIndex, DbTableCR table) const;

    // Bind
    virtual BeSQLite::DbResult  _Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId) {BeAssert (false); return BE_SQLITE_ERROR;}
            BeSQLite::DbResult  _Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, uint32_t arrayIndex);
   BeSQLite::DbResult           BindPrimaryKeysForProperty (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, uint32_t arrayIndex);

    // Insert
    virtual InsertStatus        _Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId) {BeAssert (false); return INSERT_Error;}
    InsertStatus                _Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, uint32_t arrayIndex);

protected:
    virtual                     ~InstancePropertyToTableInserter () {}
    ECN::ECPropertyCR           GetECProperty() const {return m_ecProperty;}
    WStringCR                   GetECPropertyAccessString() {return m_ecPropertyAccessString;}                     
    InsertStatus                Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, uint32_t arrayIndex);
    
    static InstancePropertyToTableInserterPtr Create 
        (
        InsertStatus& status, 
        ECDbMapCR ecDbMap, 
        PropertyMapToTableCR propertyMapToTable,
        BeRepositoryBasedIdSequenceR ecInstanceIdSequence,
        InstanceInserterCP parentInserter
        );
};

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct BindRelationshipEnd : RefCountedBase
{
protected:
    int m_parameterIndex;
    ECN::ECRelationshipEnd m_relationshipEnd;
    BindRelationshipEnd (int& nextParameterIndex, ECN::ECRelationshipEnd end) : m_parameterIndex (nextParameterIndex), m_relationshipEnd (end)
        {
        nextParameterIndex++;
        }

    virtual BeSQLite::DbResult _Bind (BeSQLite::Statement& statement, ECN::IECInstanceCR instance) const = 0;

public:
    BeSQLite::DbResult Bind (BeSQLite::Statement& statement, ECN::IECInstanceCR instance) const
        {
        return _Bind (statement, instance);
        }
};

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct BindRelationshipEndInstanceId : BindRelationshipEnd
{
friend struct RelationshipInstanceLinkTableInserter;
friend struct RelationshipInstanceEndTableInserter;
private:
    BindRelationshipEndInstanceId (int& nextParameterIndex, ECN::ECRelationshipEnd end) : BindRelationshipEnd (nextParameterIndex, end) {};

    bool GetEndInstanceId (ECInstanceId& endInstanceId, ECN::IECRelationshipInstanceCR relationshipInstance) const
        {
        ECN::IECInstancePtr  endInstance = (m_relationshipEnd == ECN::ECRelationshipEnd_Source) ?
            relationshipInstance.GetSource() : relationshipInstance.GetTarget();
        if (!EXPECTED_CONDITION (endInstance.IsValid() && "Relationship end is empty"))
            return false;

        WString endInstanceIdStr = endInstance->GetInstanceId ();
        if (endInstanceIdStr.empty ())
            return false;

        return ECInstanceIdHelper::FromString (endInstanceId, endInstanceIdStr.c_str ());
        }

    virtual DbResult _Bind (BeSQLite::Statement& statement, ECN::IECInstanceCR instance) const override
        {
        ECN::IECRelationshipInstanceCP relationshipInstance = dynamic_cast<ECN::IECRelationshipInstanceCP> (&instance);
        BeAssert (nullptr != relationshipInstance);
        ECInstanceId endInstanceId;
        if (!GetEndInstanceId (endInstanceId, *relationshipInstance))
            return BE_SQLITE_ERROR;
        return statement.BindId (m_parameterIndex, endInstanceId);
        }

};

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct BindRelationshipEndClassId : BindRelationshipEnd
{
friend struct RelationshipInstanceLinkTableInserter;
friend struct RelationshipInstanceEndTableInserter;
private:
    BindRelationshipEndClassId (int& nextParameterIndex, ECN::ECRelationshipEnd end) : BindRelationshipEnd (nextParameterIndex, end) {};
    virtual DbResult _Bind (BeSQLite::Statement& statement, ECN::IECInstanceCR instance) const override
        {
        ECN::IECRelationshipInstanceCP relationshipInstance = dynamic_cast<ECN::IECRelationshipInstanceCP> (&instance);
        BeAssert (nullptr != relationshipInstance);

        ECN::ECClassId endClassId;
        ECN::IECInstancePtr  endInstance = (m_relationshipEnd == ECN::ECRelationshipEnd_Source) ?
            relationshipInstance->GetSource() : relationshipInstance->GetTarget();
        ECN::ECClassCR endClass = endInstance->GetClass ();
        endClassId = (ECN::ECClassId)endClass.GetId ();

        return statement.BindInt64 (m_parameterIndex, endClassId);
        }
};

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipInstanceInserter : InstanceInserter
{
friend InstanceInserterPtr  InstanceInserter::Create (InsertStatus& status, ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence, InstanceInserterCP parentInserter);

protected:
    RelationshipInstanceInserter (ECDbMapCR ecDbMap, ECN::ECRelationshipClassCR relClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence);
    virtual                 ~RelationshipInstanceInserter() {}
};

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipInstanceEndTableInserter : RelationshipInstanceInserter
{
friend InstanceInserterPtr  InstanceInserter::Create (InsertStatus& status, ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence, InstanceInserterCP parentInserter);

private:
    RelationshipClassEndTableMapCP      m_classMap;
    SqlUpdate                           m_updateBuilder;
    bvector<BindRelationshipEndPtr>     m_relationshipOtherEndBinders;
    BindRelationshipEndInstanceIdPtr    m_primaryKeyBinder;

    // Initialize
    virtual InsertStatus        _Initialize() override;
    void                        AppendSqlToUpdateRelationshipOtherEnd (int& nextParameterIndex, RelationshipClassEndTableMapCR relationshipClassMap);
    InsertStatus                AppendSqlToUpdatePropertyValues (int& nextParameterIndex, IClassMap const& classMap);
    InsertStatus                AppendSqlForPrimaryKeyWhereClause (int& nextParameterIndex, DbTableCR table);
    DbResult                    BindRelationshipOtherEnd (ECN::IECRelationshipInstanceCR relationshipInstance);
    DbResult                    BindPrimaryKeyWhereClause (ECInstanceId* ecInstanceId, ECN::IECRelationshipInstanceR relationshipInstance, bool useSuppliedECInstanceId);
    virtual BeSQLite::DbResult  _Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId) override;
    virtual InsertStatus        _Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId) override;

protected:
                                RelationshipInstanceEndTableInserter (ECDbMapCR ecDbMap, ECN::ECRelationshipClassCR relClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence);
    virtual                     ~RelationshipInstanceEndTableInserter() {}
};

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipInstanceLinkTableInserter : RelationshipInstanceInserter
{
friend InstanceInserterPtr  InstanceInserter::Create (InsertStatus& status, ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence, InstanceInserterCP parentInserter);
private:
    bvector<BindRelationshipEndPtr> m_relationshipEndBinders;

    virtual InsertStatus        _Initialize() override;
    void                        AppendSqlToInsertRelationships (int& nextParameterIndex);
    void                        AppendSqlForRelationshipEnd (int& nextParameterIndex, ECN::ECRelationshipEnd relationEnd, RelationshipClassLinkTableMapCR relationshipClassMap);

    DbResult                    BindRelationshipEnds (ECN::IECRelationshipInstanceCR relationshipInstance);
    virtual BeSQLite::DbResult  _Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId) override;

protected:
                                RelationshipInstanceLinkTableInserter (ECDbMapCR ecDbMap, ECN::ECRelationshipClassCR relClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence);
    virtual                     ~RelationshipInstanceLinkTableInserter() {}
};

END_BENTLEY_SQLITE_EC_NAMESPACE

