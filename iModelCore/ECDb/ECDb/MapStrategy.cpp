/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapStrategy.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UserECDbMapStrategy::Assign(Strategy strategy, Options options, bool isPolymorphic)
    {
    m_strategy = strategy;
    m_options = options;
    m_appliesToSubclasses = isPolymorphic;

    return IsValid() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool UserECDbMapStrategy::IsValid() const
    {
    switch (m_strategy)
        {
            case Strategy::None:
                return !m_appliesToSubclasses && (m_options == Options::None || m_options == Options::DisableSharedColumns);

            case Strategy::SharedTable:
                {
                if (m_options == Options::None)
                    return true;

                if (!m_appliesToSubclasses)
                    return m_options == Options::SharedColumns;

                return m_options == Options::SharedColumns || m_options == Options::SharedColumnsForSubclasses;
                }

            case Strategy::ExistingTable:
                return !m_appliesToSubclasses && m_options == Options::None;

            default:
                //these strategies must not have any options
                return m_options == Options::None;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2015
//+---------------+---------------+---------------+---------------+---------------+------
UserECDbMapStrategy const& UserECDbMapStrategy::AssignRoot(UserECDbMapStrategy const& parent)
    {
    m_root = parent.m_root != nullptr ? parent.m_root : &parent;
    return *m_root;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus UserECDbMapStrategy::TryParse(UserECDbMapStrategy& mapStrategy, ECN::ECDbClassMap::MapStrategy const& mapStrategyCustomAttribute)
    {
    Strategy strategy = Strategy::None;
    if (SUCCESS != TryParse(strategy, mapStrategyCustomAttribute.GetStrategy()))
        return ERROR;

    Options option = Options::None;
    if (SUCCESS != TryParse(option, mapStrategyCustomAttribute.GetOptions()))
        return ERROR;

    return mapStrategy.Assign(strategy, option, mapStrategyCustomAttribute.AppliesToSubclasses());
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus UserECDbMapStrategy::TryParse(Strategy& mapStrategy, Utf8CP mapStrategyStr)
    {
    if (Utf8String::IsNullOrEmpty(mapStrategyStr))
        mapStrategy = Strategy::None;
    else if (BeStringUtilities::Stricmp(mapStrategyStr, "NotMapped") == 0)
        mapStrategy = Strategy::NotMapped;
    else if (BeStringUtilities::Stricmp(mapStrategyStr, "OwnTable") == 0)
        mapStrategy = Strategy::OwnTable;
    else if (BeStringUtilities::Stricmp(mapStrategyStr, "SharedTable") == 0)
        mapStrategy = Strategy::SharedTable;
    else if (BeStringUtilities::Stricmp(mapStrategyStr, "ExistingTable") == 0)
        mapStrategy = Strategy::ExistingTable;
    else
        {
        LOG.errorv("'%s' is not a valid MapStrategy value.", mapStrategyStr);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus UserECDbMapStrategy::TryParse(Options& mapStrategyOptions, Utf8CP mapStrategyOptionsStr)
    {
    mapStrategyOptions = Options::None;
    if (Utf8String::IsNullOrEmpty(mapStrategyOptionsStr))
        return SUCCESS;

    bvector<Utf8String> optionTokens;
    BeStringUtilities::Split(mapStrategyOptionsStr, ",;|", optionTokens);

    for (Utf8StringCR optionToken : optionTokens)
        {
        if (optionToken.empty())
            continue;

        if (optionToken.EqualsI("SharedColumns"))
            mapStrategyOptions = Enum::Or(mapStrategyOptions, Options::SharedColumns);
        else if (optionToken.EqualsI("SharedColumnsForSubclasses"))
            mapStrategyOptions = Enum::Or(mapStrategyOptions, Options::SharedColumnsForSubclasses);
        else if (optionToken.EqualsI("DisableSharedColumns"))
            mapStrategyOptions = Enum::Or(mapStrategyOptions, Options::DisableSharedColumns);
        else if (optionToken.EqualsI("JoinedTableForSubclasses"))
            mapStrategyOptions = Enum::Or(mapStrategyOptions, Options::JoinedTableForSubclasses);
        else
            {
            LOG.errorv("'%s' is not a valid MapStrategy option value.", optionToken.c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              06/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UserECDbMapStrategy::ToString() const
    {
    Utf8String str;
    switch (m_strategy)
        {
            case Strategy::None:
                str.append("None");
                break;

            case Strategy::NotMapped:
                str.append("NotMapped");
                break;

            case Strategy::OwnTable:
                str.append("OwnTable");
                break;

            case Strategy::SharedTable:
                str.append("SharedTable");
                break;

            case Strategy::ExistingTable:
                str.append("ExistingTable");
                break;

            default:
                BeAssert(false);
                break;
        }

    if (m_appliesToSubclasses)
        str.append(" (applies to subclasses)");

    switch (m_options)
        {
            case Options::None:
                break;

            case Options::SharedColumns:
                str.append("Option: SharedColumns");
                break;

            case Options::SharedColumnsForSubclasses:
                str.append("Option: SharedColumnsForSubclasses");
                break;

            case Options::DisableSharedColumns:
                str.append("Option: DisableSharedColumns");
                break;

            case Options::JoinedTableForSubclasses:
                str.append("Option: JoinedTableForSubclasses");
                break;

            default:
                BeAssert(false);
                break;
        }

    return std::move(str);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Assign(UserECDbMapStrategy const& userStrategy)
    {
    m_appliesToSubclasses = userStrategy.AppliesToSubclasses();

    switch (userStrategy.GetStrategy())
        {
            case UserECDbMapStrategy::Strategy::ExistingTable:
                m_strategy = Strategy::ExistingTable;
                break;
            case UserECDbMapStrategy::Strategy::NotMapped:
                m_strategy = Strategy::NotMapped;
                break;
            case UserECDbMapStrategy::Strategy::None: //default
            case UserECDbMapStrategy::Strategy::OwnTable:
                m_strategy = Strategy::OwnTable;
                break;
            case UserECDbMapStrategy::Strategy::SharedTable:
                m_strategy = Strategy::SharedTable;
                break;

            default:
                BeAssert(false);
                break;
        };

    const UserECDbMapStrategy::Options userOptions = userStrategy.GetOptions();
    
    m_options = Options::None;
    if (Enum::Contains(userOptions, UserECDbMapStrategy::Options::SharedColumns))
        m_options = Enum::Or(m_options, Options::SharedColumns);
    
    if (Enum::Contains(userOptions, UserECDbMapStrategy::Options::JoinedTableForSubclasses))
        m_options = Enum::Or(m_options, Options::ParentOfJoinedTable);

    if (!IsValid())
        {
        LOG.errorv("Invalid MapStrategy: %s", ToString().c_str());
        return ERROR;
        }

    m_isResolved = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Assign(Strategy strategy, Options option, bool appliesToSubclasses)
    {
    m_strategy = strategy;
    m_options = option;
    m_appliesToSubclasses = appliesToSubclasses;
    
    if (!IsValid())
        {
        LOG.errorv("Invalid MapStrategy: %s", ToString().c_str());
        return ERROR;
        }
    
    m_isResolved = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMapStrategy::IsValid() const
    {
    switch (m_strategy)
        {
            case Strategy::SharedTable:
            {
            if (!m_appliesToSubclasses)
                return m_options == Options::None || m_options == Options::SharedColumns;

            Options validOptions1 = Enum::Or(Options::SharedColumns, Options::JoinedTable);
            Options validOptions2 = Enum::Or(Options::SharedColumns, Options::ParentOfJoinedTable);
            return m_options == Options::None || Enum::Contains(validOptions1, m_options) || Enum::Contains(validOptions2, m_options);
            }

            default:
                return m_options == Options::None;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              06/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECDbMapStrategy::ToString() const
    {
    Utf8String str;
    switch (m_strategy)
        {
            case Strategy::NotMapped:
                str.append("NotMapped");
                break;

            case Strategy::OwnTable:
                str.append("OwnTable");
                break;

            case Strategy::SharedTable:
                str.append("SharedTable");
                break;

            case Strategy::ExistingTable:
                str.append("ExistingTable");
                break;

            case Strategy::ForeignKeyRelationshipInSourceTable:
                str.append("ForeignKeyRelationshipInSourceTable");
                break;

            case Strategy::ForeignKeyRelationshipInTargetTable:
                str.append("ForeignKeyRelationshipInTargetTable");
                break;

            default:
                BeAssert(false);
                break;
        }

    if (m_appliesToSubclasses)
        str.append(" (applies to subclasses)");

    switch (m_options)
        {
            case Options::None:
                break;

            case Options::SharedColumns:
                str.append("Option: SharedColumns");
                break;

            default:
                BeAssert(false);
                break;
        }

    return std::move(str);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE


