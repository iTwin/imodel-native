/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbClassDependencyAnalyzer.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbClassDependencyAnalyzer : NonCopyableClass
    {
    struct ICallback
        {
        virtual bool CanAnalyze (ECClassId classId) const = 0;
        };
private:
    BeSQLite::Db& m_db;
    std::set<ECClassId> m_classVisited;
    ICallback* m_callback;
private:
    BeSQLite::DbResult ComputeClassDependency (std::vector<ECClassId>& dependencies, ECClassId ecClassId, ECClassId parentId);
    BeSQLite::DbResult ProcessDependencyStatement (std::vector<ECClassId>& dependencies, BeSQLite::CachedStatement* stmt, ECClassId parentId);
    BeSQLite::DbResult ComputeRelationshipsDependency (std::vector<ECClassId>& dependencies, ECClassId ecClassId);
    BeSQLite::DbResult ComputeBaseClassDependency (std::vector<ECClassId>& dependencies, ECClassId ecClassId);
    BeSQLite::DbResult ComputePropertyTypeDependency( std::vector<ECClassId>& dependencies, ECClassId ecClassId);
    BeSQLite::DbResult ComputeCustomAttributeDependency (std::vector<ECClassId>& dependencies, ECContainerId containerId, ECContainerType containerType, ECClassId classId);
    void MarkVisited (ECClassId classId);
    bool HasVisited(std::vector<ECClassId>& dependencies, ECClassId classId, ECClassId parentId);
   
public:
    void DebugPrint(ECClassId classId);
    ECDbClassDependencyAnalyzer(BeSQLite::Db& db) 
        : m_db(db), m_callback(nullptr)
        {
        }
    ~ECDbClassDependencyAnalyzer()
        {
        }
    BeSQLite::DbResult Compute(std::vector<ECClassId>& dependencies, ECClassId classId, ICallback* callback = nullptr);

    };
END_BENTLEY_SQLITE_EC_NAMESPACE
