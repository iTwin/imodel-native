/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbSchemaManager.h>
#include "ECDbSql.h"
#include "ClassMap.h"
#include "ClassMapInfo.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//struct ClassMapInfo;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext
    {
private:
    mutable std::map<ECN::ECClassCP, std::unique_ptr<UserECDbMapStrategy>> m_userStrategyCache;

    mutable std::vector<std::pair<ClassMap const*, std::unique_ptr<ClassMapInfo>>> m_classMapInfoCache;
    bmap<ECDbSqlIndex const*, ECN::ECClassId> m_classIdFilteredIndices;

    UserECDbMapStrategy* GetUserStrategyP(ECN::ECClassCR, ECN::ECDbClassMap const*) const;

public:
    SchemaImportContext () {}

    //! Gets the user map strategy for the specified ECClass.
    //! @return User map strategy. If the class doesn't have one a default strategy is returned. Only in 
    //! case of error, nullptr is returned
    UserECDbMapStrategy const* GetUserStrategy(ECN::ECClassCR, ECN::ECDbClassMap const* = nullptr) const;
    UserECDbMapStrategy* GetUserStrategyP(ECN::ECClassCR) const;

    void CacheClassMapInfo(ClassMap const&, std::unique_ptr<ClassMapInfo>&) const;
    std::vector<std::pair<ClassMap const*, std::unique_ptr<ClassMapInfo>>> const& GetClassMapInfoCache() const { return m_classMapInfoCache; }
 /*   void AddClassIdFilteredIndex(ECDbSqlIndex const&, ECN::ECClassId);
    bool TryGetClassIdToIndex(ECN::ECClassId&, ECDbSqlIndex const&) const;
    */
    };

END_BENTLEY_SQLITE_EC_NAMESPACE