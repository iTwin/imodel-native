/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapStrategy.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UserECDbMapStrategy::Assign(Strategy strategy, Options options, int minimumSharedColumnCount, bool appliesToSubclasses)
    {
    m_strategy = strategy;
    m_options = options;
    m_minimumSharedColumnCount = minimumSharedColumnCount;
    m_appliesToSubclasses = appliesToSubclasses;
    return IsValid() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool UserECDbMapStrategy::IsValid() const
    {
    Options invalidOptions1 = Options::SharedColumns | Options::SharedColumnsForSubclasses;
    Options invalidOptions2 = Options::SharedColumns | Options::DisableSharedColumns;
    Options invalidOptions3 = Options::SharedColumnsForSubclasses | Options::DisableSharedColumns;

    switch (m_strategy)
        {
            case Strategy::None:
            {
            bool isValid = !m_appliesToSubclasses && !Enum::Contains(m_options, invalidOptions1) && !Enum::Contains(m_options, invalidOptions2) && !Enum::Contains(m_options, invalidOptions3);
            if (!isValid)
                return false;

            return m_minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT || Enum::Intersects(m_options, Options::SharedColumns | Options::SharedColumnsForSubclasses);
            }
            case Strategy::SharedTable:
            {
            if (!m_appliesToSubclasses)
                return m_options == Options::None || m_options == Options::SharedColumns;

            if (Enum::Contains(m_options, invalidOptions1) || Enum::Contains(m_options, invalidOptions2) || Enum::Contains(m_options, invalidOptions3))
                return false;

            return m_minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT || Enum::Intersects(m_options, Options::SharedColumns | Options::SharedColumnsForSubclasses);
            }

            case Strategy::ExistingTable:
                return !m_appliesToSubclasses && m_options == Options::None && m_minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT;

            default:
                //these strategies must not have any options
                return m_options == Options::None && m_minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT;
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

    Options option = Options::None;
    if (SUCCESS != TryParse(option, mapStrategyCustomAttribute.GetOptions()))
        return ERROR;

    return mapStrategy.Assign(strategy, option, mapStrategyCustomAttribute.GetMinimumSharedColumnCount(), mapStrategyCustomAttribute.AppliesToSubclasses());
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus UserECDbMapStrategy::TryParse(Strategy& mapStrategy, Utf8CP mapStrategyStr)
    {
    if (Utf8String::IsNullOrEmpty(mapStrategyStr))
        mapStrategy = Strategy::None;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "NotMapped") == 0)
        mapStrategy = Strategy::NotMapped;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "OwnTable") == 0)
        mapStrategy = Strategy::OwnTable;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "SharedTable") == 0)
        mapStrategy = Strategy::SharedTable;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "ExistingTable") == 0)
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

        if (optionToken.EqualsI(USERMAPSTRATEGY_OPTIONS_SHAREDCOLUMNS))
            mapStrategyOptions = Enum::Or(mapStrategyOptions, Options::SharedColumns);
        else if (optionToken.EqualsI(USERMAPSTRATEGY_OPTIONS_SHAREDCOLUMNSFORSUBCLASSES))
            mapStrategyOptions = Enum::Or(mapStrategyOptions, Options::SharedColumnsForSubclasses);
        else if (optionToken.EqualsI(USERMAPSTRATEGY_OPTIONS_DISABLESHAREDCOLUMNS))
            mapStrategyOptions = Enum::Or(mapStrategyOptions, Options::DisableSharedColumns);
        else if (optionToken.EqualsI(USERMAPSTRATEGY_OPTIONS_JOINEDTABLEPERDIRECTSUBCLASS))
            mapStrategyOptions = Enum::Or(mapStrategyOptions, Options::JoinedTablePerDirectSubclass);
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

    if (m_options != Options::None)
        str.append(" Options: ").append(ToString(m_options));

    if (m_minimumSharedColumnCount != ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT)
        {
        Utf8String snip;
        snip.Sprintf(" Minimum shared column count: %d", m_minimumSharedColumnCount);
        str.append(std::move(snip));
        }

    return str;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              11/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UserECDbMapStrategy::ToString(Options options)
    {
    Utf8String str;
    if (options == UserECDbMapStrategy::Options::None)
        return str;

    bvector<Utf8CP> tokens;
    if (Enum::Contains(options, Options::SharedColumns))
        tokens.push_back(USERMAPSTRATEGY_OPTIONS_SHAREDCOLUMNS);

    if (Enum::Contains(options, Options::SharedColumnsForSubclasses))
        tokens.push_back(USERMAPSTRATEGY_OPTIONS_SHAREDCOLUMNSFORSUBCLASSES);

    if (Enum::Contains(options, Options::DisableSharedColumns))
        tokens.push_back(USERMAPSTRATEGY_OPTIONS_DISABLESHAREDCOLUMNS);

    if (Enum::Contains(options, Options::JoinedTablePerDirectSubclass))
        tokens.push_back(USERMAPSTRATEGY_OPTIONS_JOINEDTABLEPERDIRECTSUBCLASS);

    bool isFirstItem = true;
    for (Utf8CP token : tokens)
        {
        if (!isFirstItem)
            str.append(",");

        str.append(token);
        isFirstItem = false;
        }

    return str;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Assign(UserECDbMapStrategy const& userStrategy)
    {
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
                m_strategy = userStrategy.AppliesToSubclasses() ? Strategy::TablePerHierarchy : Strategy::SharedTable;
                break;

            default:
                BeAssert(false);
                break;
        };

    const UserECDbMapStrategy::Options userOptions = userStrategy.GetOptions();
    
    m_options = Options::None;
    if (Enum::Contains(userOptions, UserECDbMapStrategy::Options::SharedColumns))
        {
        m_options = Enum::Or(m_options, Options::SharedColumns);
        //minimum shared column count only valid for this strategy if shared columns is on for this strategy.
        //if SharedColumnsForSubclasses is set in the user strategy, no minimum shared columns are used
        //for this strategy
        m_minimumSharedColumnCount = userStrategy.GetMinimumSharedColumnCount();
        }

    if (Enum::Contains(userOptions, UserECDbMapStrategy::Options::JoinedTablePerDirectSubclass))
        m_options = Enum::Or(m_options, Options::ParentOfJoinedTable);

    if (!IsValid())
        return ERROR;

    m_isResolved = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Assign(Strategy strategy, Options option, int minimumSharedColumnCount)
    {
    m_strategy = strategy;
    m_options = option;
    m_minimumSharedColumnCount = minimumSharedColumnCount;
    
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
            case Strategy::TablePerHierarchy:
            {
            const Options validOptions1 = Enum::Or(Options::SharedColumns, Options::JoinedTable);
            const Options validOptions2 = Enum::Or(Options::SharedColumns, Options::ParentOfJoinedTable);
            const bool isValid = m_options == Options::None || Enum::Contains(validOptions1, m_options) || Enum::Contains(validOptions2, m_options);
            if (!isValid)
                return false;
            
            return Enum::Contains(m_options, Options::SharedColumns) || m_minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT;
            }

            case Strategy::SharedTable:
                return m_options == Options::None || m_options == Options::SharedColumns;

            default:
                return m_options == Options::None && m_minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              06/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECDbMapStrategy::ToString() const
    {
    Utf8String str(ToString(m_strategy));

    if (m_options != Options::None)
        {
        str.append(" Option: ");

        bool needsComma = false;
        if (Enum::Contains(m_options, Options::SharedColumns))
            {
            str.append(" SharedColumns");
            needsComma = true;
            }

        if (Enum::Contains(m_options, Options::JoinedTable))
            {
            if (needsComma)
                str.append(",");

            str.append(" JoinedTable");
            needsComma = true;
            }

        if (Enum::Contains(m_options, Options::ParentOfJoinedTable))
            {
            if (needsComma)
                str.append(",");

            str.append(" ParentOfJoinedTable");
            needsComma = true;
            }

        }

    if (m_minimumSharedColumnCount != ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT)
        {
        Utf8String snip;
        snip.Sprintf(" Minimum shared column count: %d", m_minimumSharedColumnCount);
        str.append(snip);
        }

    return str;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP ECDbMapStrategy::ToString(Strategy strategy)
    {
    switch (strategy)
        {
        case Strategy::ExistingTable:
            return "ExistingTable";
        case Strategy::ForeignKeyRelationshipInSourceTable:
            return "ForeignKeyRelationshipInSourceTable";
        case Strategy::ForeignKeyRelationshipInTargetTable:
            return "ForeignKeyRelationshipInTargetTable";
        case Strategy::NotMapped:
            return "NotMapped";
        case Strategy::OwnTable:
            return "OwnTable";
        case Strategy::SharedTable:
            return "SharedTable";
        case Strategy::TablePerHierarchy:
            return "TablePerHierarchy";
        default:
            BeAssert(false && "Unhandled value for ECDbMapStrategy::Strategy in ToString");
            return nullptr;
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE


