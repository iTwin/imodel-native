/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/InstanceDeleter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ECPersistence.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE



struct RelationshipInstanceDeleter;

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan      04/2012
+===============+===============+===============+===============+===============+======*/
struct InstanceDeleter : RefCountedBase
    {
    typedef bvector<InstanceDeleterPtr> PropertyDeleters;
private:
    ECN::ECPropertyCP               m_ecProperty;
    int64_t                         m_ecPropertyId;
    PropertyDeleters                m_propertyDeleters;
    BeSQLite::CachedStatementPtr    m_deleteStatement;
    bool                            m_deleteDependentInstances;
    std::unique_ptr<ECInstanceFinder> m_instanceFinder;
//    ECN::ECPropertyId               m_ecPropertyIdForPersistence; -- detected as unused by clang

    static InstanceDeleterPtr CreateChildDeleter (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, ECN::ECPropertyId propertyIdForPersistence);
    InstanceDeleter (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, ECN::ECPropertyCP ecProperty, ECN::ECPropertyId propertyIdForPersistence);

    MapStatus Initialize ();
    
    bool IsTopLevelDeleter();
    
    ECInstanceFinder* GetInstanceFinder();
    BentleyStatus FindOrphanedInstances 
        (
        ECInstanceKeyMultiMap& orphanedRelationshipInstances,
        ECInstanceKeyMultiMap& orphanedInstances, 
        ECN::ECClassId deletedClassId,
        ECInstanceId deletedInstanceId, 
        ECDbR ecDb
        );

    static BentleyStatus DeleteInstances (int& numDeleted, const ECInstanceKeyMultiMap& instanceMap, ECDbR ecDb, ECDbDeleteHandlerP deleteHandler);
    BentleyStatus DeleteDependentInstances (int& numDeleted, const ECInstanceIdSet& deletedInstanceIds, ECDbDeleteHandlerP deleteHandler);
    
    void GetDeleteSql (Utf8StringR deleteSql);
    DbResult PrepareDeleteStatement();
    DbResult GetDeleteStatement (CachedStatementPtr& deleteStatement);

    void GetSelectSql (Utf8StringR selectSql, Utf8CP whereCriteria);
protected:
    ECDbMapCR                       m_ecDbMap;
    ECN::ECClassCR                  m_ecClass;
    ECN::ECPropertyId               m_propertyIdForPersistence;
    InstanceDeleter (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, bool deleteDependentInstances);
    ~InstanceDeleter();

    virtual MapStatus _Initialize();
    void InitializeStructArrayDeleters (IClassMap const& classMap, DbTableCR table);

    virtual BentleyStatus _Delete (int& numDeleted, const ECInstanceIdSet& ecInstanceIds, ECDbDeleteHandlerP deleteHandler);
    virtual void _ClearCache();

    BentleyStatus FindInstances (ECInstanceIdSet& instanceIds, Utf8CP whereCriteria);
    BentleyStatus DeleteStructArrays (const ECInstanceIdSet& instanceIds);
    void OnBeforeDelete (ECDbDeleteHandlerP deleteHandler, const ECInstanceIdSet& instanceIds);
    
public:
    static InstanceDeleterPtr Create (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, bool deleteDependentInstances);
    BentleyStatus Delete (ECInstanceId ecInstanceId, ECDbDeleteHandlerP deleteHandler);

    // TO BE DEPRECATED
    BentleyStatus Delete (int* pNumDeleted, Utf8CP whereCriteria, ECDbDeleteHandlerP deleteHandler);

    BentleyStatus Delete (int* pNumDeleted, const ECInstanceIdSet& ecInstanceIds, ECDbDeleteHandlerP deleteHandler);

    void ClearCache() {_ClearCache();}
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      03/2013
+===============+===============+===============+===============+===============+======*/
struct RelationshipInstanceDeleter : InstanceDeleter
    {
friend struct InstanceDeleter;
private:
    BeSQLite::CachedStatementPtr m_updateStatement;
    BentleyStatus DeleteFromRelationshipEndTable (int& numDeleted, const ECInstanceIdSet& ecInstanceIds, ECDbDeleteHandlerP deleteHandler);

    void GetUpdateSql (Utf8StringR updateSql);
    DbResult PrepareUpdateStatement();
    DbResult GetUpdateStatement (CachedStatementPtr& updateStatement);

    virtual MapStatus _Initialize() override;
    virtual void _ClearCache() override;
    virtual BentleyStatus _Delete (int& numDeleted, const ECInstanceIdSet& ecInstanceIds, ECDbDeleteHandlerP deleteHandler) override;

protected:
    RelationshipInstanceDeleter (ECDbMapCR ecDbMap, ECN::ECRelationshipClassCR ecRelClass);
    };
    
END_BENTLEY_SQLITE_EC_NAMESPACE
