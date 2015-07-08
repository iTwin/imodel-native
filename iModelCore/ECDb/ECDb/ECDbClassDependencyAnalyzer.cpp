/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbClassDependencyAnalyzer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbClassDependencyAnalyzer::DebugPrint(ECClassId classId)
    {
    //BeSQLite::CachedStatementPtr stmt;
    //m_db.GetCachedStatement (stmt, "SELECT S.Name, C.Name FROM ec_Class C INNER JOIN ec_Schema S ON S.Id = C.SchemaId WHERE C.Id = ?");
    //stmt->BindInt64 (1, classId);
    //if (stmt->Step() == BE_SQLITE_ROW)
    //    {
    //    printf("=> %s %s [%lld]\r\n", stmt->GetValueText(0), stmt->GetValueText(1), classId); //ERROR on iOS
    //    }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbClassDependencyAnalyzer::ComputeClassDependency (std::vector<ECClassId>& dependencies, ECClassId ecClassId, ECClassId parentId)
    {
    if (HasVisited(dependencies, ecClassId, parentId))
        return BE_SQLITE_DONE;

    DbResult r;
    BeSQLite::CachedStatementPtr stmt;
    m_db.GetCachedStatement (stmt, "SELECT SchemaId, IsRelationship FROM ec_Class WHERE Id = ?");
    stmt->BindInt64 (1, ecClassId);
    if ((r = stmt->Step()) != BE_SQLITE_ROW)
        return r;

    auto itor = parentId == 0 ? dependencies.begin() : std::find(dependencies.begin(), dependencies.end(), parentId);
    dependencies.insert(itor, ecClassId);
    MarkVisited(ecClassId);

    auto ecSchemaId = static_cast<ECSchemaId>(stmt->GetValueInt(0));
    auto isRelationship = (stmt->GetValueInt(1) != 0);

    r = ComputeCustomAttributeDependency(dependencies, ecSchemaId, ECContainerType::Schema, ecClassId);
    r = ComputeCustomAttributeDependency(dependencies, ecClassId, ECContainerType::Class, ecClassId);
    r = ComputeBaseClassDependency(dependencies, ecClassId);
    r = ComputePropertyTypeDependency(dependencies, ecClassId);
    if (isRelationship)
        r = ComputeRelationshipsDependency(dependencies, ecClassId);

    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbClassDependencyAnalyzer::ProcessDependencyStatement (std::vector<ECClassId>& dependencies, BeSQLite::CachedStatement* stmt, ECClassId classId)
    {
    DbResult r;
    std::vector<ECClassId> classesToLoad;
    while((r = stmt->Step()) == BE_SQLITE_ROW)
        {
        auto classToLoad = static_cast<ECClassId>(stmt->GetValueInt64(0));
        if (HasVisited(dependencies, classToLoad, classId))
            continue;

        classesToLoad.push_back(classToLoad);
        }

    if (r == BE_SQLITE_DONE)
        {
        for(auto classToLoad : classesToLoad)
            {
            r = ComputeClassDependency (dependencies, classToLoad, classId);
            if (r != BE_SQLITE_DONE)
                return r;
            }
        }

    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbClassDependencyAnalyzer::ComputeRelationshipsDependency (std::vector<ECClassId>& dependencies, ECClassId ecClassId)
    {
    DbResult r;
    BeSQLite::CachedStatementPtr stmt;
    m_db.GetCachedStatement (stmt, "SELECT DISTINCT RelationshipClassId FROM ec_RelationshipConstraintClass WHERE ClassId = ?");
    stmt->BindInt64 (1, ecClassId);

    r = ComputeCustomAttributeDependency (dependencies, ecClassId, ECContainerType::RelationshipConstraintSource, ecClassId);
    if (r != BE_SQLITE_DONE)
        return r;

    r = ComputeCustomAttributeDependency (dependencies, ecClassId, ECContainerType::RelationshipConstraintTarget, ecClassId);
    if (r != BE_SQLITE_DONE)
        return r;

    return ProcessDependencyStatement (dependencies, stmt.get(), ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbClassDependencyAnalyzer::ComputeBaseClassDependency (std::vector<ECClassId>& dependencies, ECClassId ecClassId)
    {
    BeSQLite::CachedStatementPtr stmt;
    m_db.GetCachedStatement (stmt, "SELECT BaseClassId FROM ec_BaseClass WHERE ClassId = ?");
    stmt->BindInt64 (1, ecClassId);
    return ProcessDependencyStatement (dependencies, stmt.get(), ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbClassDependencyAnalyzer::ComputePropertyTypeDependency (std::vector<ECClassId>& dependencies, ECClassId ecClassId)
    {
    DbResult r;
    BeSQLite::CachedStatementPtr stmt;
    m_db.GetCachedStatement (stmt, "SELECT Id, StructType FROM ec_Property WHERE ClassId = ?");
    stmt->BindInt64 (1, ecClassId);

    std::vector<ECClassId> classesToLoad;
    std::vector<ECPropertyId> properties;

    while((r = stmt->Step()) == BE_SQLITE_ROW)
        {   
        properties.push_back (static_cast<ECPropertyId>(stmt->GetValueInt64(0)));
        if (!stmt->IsColumnNull(1))
            {
            auto classToLoad = static_cast<ECClassId>(stmt->GetValueInt64(1));
            if (HasVisited(dependencies, classToLoad, ecClassId))
                continue;

            classesToLoad.push_back(classToLoad);
            }
        }

    if (r != BE_SQLITE_DONE)
        return r;

    for(auto property : properties)
        {
        r = ComputeCustomAttributeDependency (dependencies, property, ECContainerType::Property, ecClassId);
        if (r != BE_SQLITE_DONE)
            return r;
        }

    for(auto classToLoad : classesToLoad)
        {
        r = ComputeClassDependency (dependencies, classToLoad, ecClassId);
        if (r != BE_SQLITE_DONE)
            return r;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbClassDependencyAnalyzer::ComputeCustomAttributeDependency (std::vector<ECClassId>& dependencies, ECContainerId containerId, ECContainerType containerType, ECClassId classId)
    {
    BeSQLite::CachedStatementPtr stmt;
    m_db.GetCachedStatement (stmt, "SELECT ClassId FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ?");
    stmt->BindInt64 (1, containerId);
    stmt->BindInt (2, (int) containerType);
    return ProcessDependencyStatement (dependencies, stmt.get(), classId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbClassDependencyAnalyzer::MarkVisited (ECClassId classId)
    {
    m_classVisited.insert(classId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbClassDependencyAnalyzer::HasVisited(std::vector<ECClassId>& dependencies, ECClassId classId, ECClassId parentId)
    {    
    if (m_classVisited.find (classId) != m_classVisited.end())
        {
        if (parentId != 0 )
            {
            auto itor = std::find(dependencies.begin(), dependencies.end(),  classId);
            if (itor != dependencies.end())
                {
                auto itor2 = std::find(dependencies.begin(), dependencies.end(),  parentId);
                if (itor > itor2) 
                    {
                    dependencies.erase (itor);
                    dependencies.insert(itor2, classId);
                    }
                }
            }
        return true;
        }

    if (m_callback && m_callback->CanAnalyze (classId))
        {
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ECDbClassDependencyAnalyzer::Compute(std::vector<ECClassId>& dependencies, ECClassId classId, ICallback* callback)
    {
    m_callback = callback;
    m_classVisited.clear();
    auto r = ComputeClassDependency(dependencies, classId, 0);

    if (dependencies.empty())
        dependencies.push_back(classId);
    //for(auto classId : dependencies)
    //    {
    //    DebugPrint(classId);
    //    }

    return r;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
