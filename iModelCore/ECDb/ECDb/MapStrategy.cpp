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
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbMapStrategy& ECDbMapStrategy::operator=(ECDbMapStrategy const& rhs)
    {
    if (this != &rhs)
        {
        m_strategy = rhs.m_strategy;
        m_options = rhs.m_options;
        m_isPolymorphic = rhs.m_isPolymorphic;
        }

    return *this;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbMapStrategy& ECDbMapStrategy::operator=(ECDbMapStrategy&& rhs)
    {
    if (this != &rhs)
        {
        m_strategy = std::move(rhs.m_strategy);
        m_options = std::move(rhs.m_options);
        m_isPolymorphic = std::move(rhs.m_isPolymorphic);
        }

    return *this;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Assign(MapStrategy strategy, MapStrategyOptions options, bool isPolymorphic)
    {
    m_strategy = strategy;
    m_options = (int) options;
    m_isPolymorphic = isPolymorphic;
    return IsValid() ? SUCCESS:ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMapStrategy::IsValid(bool isResolved) const
    {
    const bool hasOptions = HasOptions();

    switch (m_strategy)
        {
            case MapStrategy::None:
                {
                // in resolved state the strategy should never exist anymore
                if (isResolved)
                    return false;

                return !hasOptions || m_options == (int) MapStrategyOptions::DisableSharedColumns;
                }

            case MapStrategy::SharedTable:
                {
                if (!hasOptions)
                    return true;

                if (!m_isPolymorphic)
                    return m_options == (int) MapStrategyOptions::SharedColumns;


                if (!isResolved)
                    return (m_options == (int) MapStrategyOptions::SharedColumns) || (m_options == (int) MapStrategyOptions::SharedColumnsForSubclasses);
                else
                    return (m_options == (int) MapStrategyOptions::SharedColumns) || 
                           (m_options == (int) MapStrategyOptions::SharedColumnsForSubclasses) ||
                           (m_options == (int) MapStrategyOptions::DisableSharedColumns);
                }

            case MapStrategy::ExistingTable:
                return !hasOptions || m_options == (int) MapStrategyOptions::Readonly;

            case MapStrategy::OwnTable:
            case MapStrategy::NotMapped:
                //these strategies must not have any options
                return !hasOptions;

            //all other strategies are internal ones. So they must not show up until the strategy is resolved.
            //one resolved they are valid if they don't have options
            default:
                {
                if (!isResolved)
                    {
                    BeAssert(false && "should never end up here");
                    return false;
                    }

                return !hasOptions;
                }
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
            case MapStrategy::OwnTable:
                str.append("OwnTable");
                break;

            case MapStrategy::SharedTable:
                str.append("SharedTable");
                break;

            case MapStrategy::ExistingTable:
                str.append("ExistingTable");
                break;

            default:
                BeAssert(false);
                break;
        }

    if (m_isPolymorphic)
        str.append(" (polymorphic)");

    if (HasOptions())
        {
        str.append(" Options: ");
        bool isFirst = true;
        if (HasOption(MapStrategyOptions::SharedColumns))
            {
            if (!isFirst)
                str.append(", ");

            str.append("SharedColumns");
            isFirst = false;
            }

        if (HasOption(MapStrategyOptions::SharedColumnsForSubclasses))
            {
            if (!isFirst)
                str.append(", ");

            str.append("SharedColumnsForSubclasses");
            isFirst = false;
            }


        if (HasOption(MapStrategyOptions::DisableSharedColumns))
            {
            if (!isFirst)
                str.append(", ");

            str.append("DisableSharedColumns");
            isFirst = false;
            }

        if (HasOption(MapStrategyOptions::Readonly))
            {
            if (!isFirst)
                str.append(", ");

            str.append("Readonly");
            isFirst = false;
            }
        }

    return std::move(str);
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbMapStrategy::TryParse(ECDbMapStrategy& mapStrategy, ECN::ECDbClassMap::MapStrategy const& mapStrategyCustomAttribute)
    {
    MapStrategy strategy = MapStrategy::None;
    if (SUCCESS != TryParse(strategy, mapStrategyCustomAttribute.GetStrategy()))
        return ERROR;
 
    int options = (int) MapStrategyOptions::None;
    if (SUCCESS != TryParse(options, mapStrategyCustomAttribute.GetOptions()))
        return ERROR;

    mapStrategy = ECDbMapStrategy(strategy, options, mapStrategyCustomAttribute.IsPolymorphic());
    return mapStrategy.IsValid (false)? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbMapStrategy::TryParse(MapStrategy& mapStrategy, Utf8CP mapStrategyStr)
    {
    if (Utf8String::IsNullOrEmpty(mapStrategyStr))
        mapStrategy = MapStrategy::None;
    else if (BeStringUtilities::Stricmp(mapStrategyStr, "NotMapped") == 0)
        mapStrategy = MapStrategy::NotMapped;
    else if (BeStringUtilities::Stricmp(mapStrategyStr, "OwnTable") == 0)
        mapStrategy = MapStrategy::OwnTable;
    else if (BeStringUtilities::Stricmp(mapStrategyStr, "SharedTable") == 0)
        mapStrategy = MapStrategy::SharedTable;
    else if (BeStringUtilities::Stricmp(mapStrategyStr, "ExistingTable") == 0)
        mapStrategy = MapStrategy::ExistingTable;
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
BentleyStatus ECDbMapStrategy::TryParse(int& mapStrategyOptions, Utf8CP mapStrategyOptionsStr)
    {
    mapStrategyOptions = (int) MapStrategyOptions::None;
    if (Utf8String::IsNullOrEmpty(mapStrategyOptionsStr))
        return SUCCESS;

    bvector<Utf8String> optionTokens;
    BeStringUtilities::Split(mapStrategyOptionsStr, ",;|", optionTokens);

    for (Utf8StringCR optionToken : optionTokens)
        {
        if (optionToken.empty())
            continue;

        if (optionToken.EqualsI("Readonly"))
            mapStrategyOptions |= (int) MapStrategyOptions::Readonly;
        else if (optionToken.EqualsI("SharedColumns"))
            mapStrategyOptions |= (int) MapStrategyOptions::SharedColumns;
        else if (optionToken.EqualsI("SharedColumnsForSubclasses"))
            mapStrategyOptions |= (int) MapStrategyOptions::SharedColumnsForSubclasses;
        else if (optionToken.EqualsI("DisableSharedColumns"))
            mapStrategyOptions |= (int) MapStrategyOptions::DisableSharedColumns;
        else
            {
            LOG.errorv("'%s' is not a valid MapStrategyOptions value.", optionToken.c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


