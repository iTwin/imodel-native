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

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext
    {
private:
    ECDbSchemaManager::IImportIssueListener const& m_importIssueListener;

    mutable std::map<ECN::ECClassCP, std::unique_ptr<UserECDbMapStrategy>> m_userStrategyCache;
    bmap<ECDbSqlIndex const*, ECN::ECClassId> m_classIdFilteredIndices;

    UserECDbMapStrategy* GetUserStrategyP(ECN::ECClassCR, ECN::ECDbClassMap const*) const;

public:
    explicit SchemaImportContext (ECDbSchemaManager::IImportIssueListener const* importIssueListener);

    //! Gets the user map strategy for the specified ECClass.
    //! @return User map strategy. If the class doesn't have one a default strategy is returned. Only in 
    //! case of error, nullptr is returned
    UserECDbMapStrategy const* GetUserStrategy(ECN::ECClassCR, ECN::ECDbClassMap const* = nullptr) const;
    UserECDbMapStrategy* GetUserStrategyP(ECN::ECClassCR) const;

    void AddClassIdFilteredIndex(ECDbSqlIndex const&, ECN::ECClassId);
    bool TryGetClassIdToIndex(ECN::ECClassId&, ECDbSqlIndex const&) const;

    ECDbSchemaManager::IImportIssueListener const& GetIssueListener () const { return m_importIssueListener; }
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct NoopImportIssueListener : ECDbSchemaManager::IImportIssueListener
    {
private:
    static NoopImportIssueListener s_singleton;

    NoopImportIssueListener ()
        : ECDbSchemaManager::IImportIssueListener ()
        {}

    virtual void _OnIssueReported (Severity severity, Utf8CP message) const override {}
public:
    ~NoopImportIssueListener () {}

    static IImportIssueListener const& Singleton () { return s_singleton; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE