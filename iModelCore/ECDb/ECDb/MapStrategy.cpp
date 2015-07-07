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
BentleyStatus UserECDbMapStrategy::Assign(Strategy strategy, Option option, bool isPolymorphic)
    {
    m_strategy = strategy;
    m_option = option;
    m_isPolymorphic = isPolymorphic;

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
                return !m_isPolymorphic && (m_option == Option::None || m_option == Option::DisableSharedColumns);

            case Strategy::SharedTable:
                {
                if (m_option == Option::None)
                    return true;

                if (!m_isPolymorphic)
                    return m_option == Option::SharedColumns;

                return m_option == Option::SharedColumns || m_option == Option::SharedColumnsForSubclasses;
                }

            case Strategy::ExistingTable:
                return !m_isPolymorphic && (m_option == Option::None || m_option == Option::Readonly);

            default:
                //these strategies must not have any options
                return m_option == Option::None;
        }
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

    Option option = Option::None;
    if (SUCCESS != TryParse(option, mapStrategyCustomAttribute.GetOptions()))
        return ERROR;

    return mapStrategy.Assign(strategy, option, mapStrategyCustomAttribute.IsPolymorphic());
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
BentleyStatus UserECDbMapStrategy::TryParse(Option& mapStrategyOption, Utf8CP mapStrategyOptionsStr)
    {
    mapStrategyOption = Option::None;
    if (Utf8String::IsNullOrEmpty(mapStrategyOptionsStr))
        return SUCCESS;

    bvector<Utf8String> optionTokens;
    BeStringUtilities::Split(mapStrategyOptionsStr, ",;|", optionTokens);

    if (optionTokens.size() > 1)
        {
        LOG.errorv("Multiple MapStrategy options '%s' are not valid in ECDb. In ECDb multiple options are never necessary.",
                   mapStrategyOptionsStr);
        return ERROR;
        }

    Utf8StringCR optionToken = optionTokens[0];
    if (optionToken.empty())
        return SUCCESS;

    if (optionToken.EqualsI("Readonly"))
        mapStrategyOption = Option::Readonly;
    else if (optionToken.EqualsI("SharedColumns"))
        mapStrategyOption = Option::SharedColumns;
    else if (optionToken.EqualsI("SharedColumnsForSubclasses"))
        mapStrategyOption = Option::SharedColumnsForSubclasses;
    else if (optionToken.EqualsI("DisableSharedColumns"))
        mapStrategyOption = Option::DisableSharedColumns;
    else
        {
        LOG.errorv("'%s' is not a valid MapStrategy option value.", optionToken.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Assign(UserECDbMapStrategy const& userStrategy)
    {
    m_isPolymorphic = userStrategy.IsPolymorphic();

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
        }

    switch (userStrategy.GetOption())
        {
            case UserECDbMapStrategy::Option::None:
            case UserECDbMapStrategy::Option::SharedColumnsForSubclasses:
            case UserECDbMapStrategy::Option::DisableSharedColumns:
                m_option = Option::None;
                break;
            case UserECDbMapStrategy::Option::Readonly:
                m_option = Option::Readonly;
                break;
            case UserECDbMapStrategy::Option::SharedColumns:
                m_option = Option::SharedColumns;
                break;
            default:
                BeAssert(false);
                return ERROR;
        }

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
BentleyStatus ECDbMapStrategy::Assign(Strategy strategy, Option option, bool isPolymorphic)
    {
    m_strategy = strategy;
    m_option = option;
    m_isPolymorphic = isPolymorphic;
    
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
                return m_option == Option::None || m_option == Option::SharedColumns;

            case Strategy::ExistingTable:
                return m_option == Option::None || m_option == Option::Readonly;

            default:
                return m_option == Option::None;
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

    if (m_isPolymorphic)
        str.append(" (polymorphic)");

    switch (m_option)
        {
            case Option::None:
                break;

            case Option::Readonly:
                str.append("Option: Readonly");
                break;

            case Option::SharedColumns:
                str.append("Option: SharedColumns");
                break;

            default:
                BeAssert(false);
                break;
        }

    return std::move(str);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE


