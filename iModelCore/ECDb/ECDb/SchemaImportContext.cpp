/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SchemaImportContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************************************************************
// SchemaImportContext
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
SchemaImportContext::SchemaImportContext (ECDbSchemaManager::IImportIssueListener const* importIssueListener)
: m_importIssueListener (importIssueListener != nullptr ? *importIssueListener : NoopImportIssueListener::Singleton ())
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy const* SchemaImportContext::GetUserStrategy(ECClassCR ecclass, ECDbClassMap const* classMapCA) const
    {
    return GetUserStrategyP(ecclass, classMapCA);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy* SchemaImportContext::GetUserStrategyP(ECClassCR ecclass) const
    {
    return GetUserStrategyP(ecclass, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
UserECDbMapStrategy* SchemaImportContext::GetUserStrategyP(ECClassCR ecclass, ECDbClassMap const* classMapCA) const
    {
    auto it = m_userStrategyCache.find(&ecclass);
    if (it != m_userStrategyCache.end())
        return it->second.get();

    bool hasClassMapCA = true;
    ECDbClassMap classMap;
    if (classMapCA == nullptr)
        {
        hasClassMapCA = ECDbMapCustomAttributeHelper::TryGetClassMap(classMap, ecclass);
        classMapCA = &classMap;
        }

    std::unique_ptr<UserECDbMapStrategy> userStrategy = std::unique_ptr<UserECDbMapStrategy>(new UserECDbMapStrategy());

    if (hasClassMapCA)
        {
        ECDbClassMap::MapStrategy strategy;
        if (ECOBJECTS_STATUS_Success != classMapCA->TryGetMapStrategy(strategy))
            return nullptr; // error

        if (SUCCESS != UserECDbMapStrategy::TryParse(*userStrategy, strategy) || !userStrategy->IsValid())
            return nullptr; // error
        }

    UserECDbMapStrategy* userStrategyP = userStrategy.get();
    m_userStrategyCache[&ecclass] = std::move(userStrategy);
    return userStrategyP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   05/2015
//---------------------------------------------------------------------------------------
void SchemaImportContext::AddClassIdFilteredIndex(ECDbSqlIndex const& index, ECClassId classId)
    {
    BeAssert(m_classIdFilteredIndices.find(&index) == m_classIdFilteredIndices.end());
    m_classIdFilteredIndices[&index] = classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   05/2015
//---------------------------------------------------------------------------------------
bool SchemaImportContext::TryGetClassIdToIndex(ECClassId& classId, ECDbSqlIndex const& index) const
    {
    auto it = m_classIdFilteredIndices.find(&index);
    if (it == m_classIdFilteredIndices.end())
        return false;

    classId = it->second;
    return true;
    }

//*************************************************************************************
// NoopImportIssueListener
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    04/2014
//---------------------------------------------------------------------------------------
//static member initialization
NoopImportIssueListener NoopImportIssueListener::s_singleton;

END_BENTLEY_SQLITE_EC_NAMESPACE
