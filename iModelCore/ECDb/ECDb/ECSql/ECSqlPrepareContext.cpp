/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** ECSqlPrepareContext ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ECSqlPrepareContext(IECSqlPreparedStatement& stmt, Db const& dataSourceECDb, ScopedIssueReporter const& issues) : m_ecdb(stmt.GetECDb()), m_dataSourceECDb(dataSourceECDb), m_issues(issues)
    {
    if (!stmt.IsCompoundStatement())
        {
        BeAssert(dynamic_cast<SingleECSqlPreparedStatement*> (&stmt) != nullptr);
        m_singlePreparedStatement = static_cast<SingleECSqlPreparedStatement*> (&stmt);
        }
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2017
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::Reset(SingleECSqlPreparedStatement& preparedStmt)
    {
    m_singlePreparedStatement = &preparedStmt;

    m_nativeSqlBuilder.Clear();
    m_scopes.Clear();
    m_selectionOptions.Clear();
    }


//****************************** ECSqlPrepareContext::ExpScope ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPrepareContext::ExpScope::ExpScope(ExpCR exp, ExpScope const* parent, OptionsExp const* options)
    : m_exp(exp), m_parent(parent), m_options(options), m_nativeSqlSelectClauseColumnCount(0), m_extendedOptions(ExtendedOptions::None)
    {
    m_ecsqlType = DetermineECSqlType(exp);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlType ECSqlPrepareContext::ExpScope::DetermineECSqlType(ExpCR exp) const
    {
    switch (exp.GetType())
        {
            case Exp::Type::SingleSelect:
            case Exp::Type::Select:
                return ECSqlType::Select;
            case Exp::Type::Insert:
                return ECSqlType::Insert;
            case Exp::Type::Update:
                return ECSqlType::Update;
            case Exp::Type::Delete:
                return ECSqlType::Delete;
            default:
            {
            BeAssert(m_parent != nullptr && "DetermineECSqlType");
            return m_parent->GetECSqlType();
            }
        }
    }

//****************************** ECSqlPrepareContext::ExpScopeStack ********************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::ExpScopeStack::Push(ExpCR exp, OptionsExp const* options)
    {
    ExpScope const* parent = nullptr;
    if (Depth() > 0)
        parent = &Current();

    m_scopes.push_back(ExpScope(exp, parent, options));
    }

//************************ ECSqlPrepareContext::SelectClauseInfo **************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       11/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPrepareContext::SelectClauseInfo::AddProperty(PropertyMap const& propertyMap)
    {
    SearchPropertyMapVisitor typeVisitor(PropertyMap::Type::System | PropertyMap::Type::SingleColumnData);
    propertyMap.AcceptVisitor(typeVisitor);
    for (PropertyMap const* propMap : typeVisitor.Results())
        {
        Utf8String path;
        for (Utf8StringCR subPath : Split(propMap->GetAccessString(), '.'))
            {
            if (path.empty())
                path.assign(subPath);
            else
                path.append(".").append(subPath);

            m_selectClause.insert(path);
            }
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       11/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlPrepareContext::SelectClauseInfo::IsSelected(Utf8StringCR accessString) const
    {
    if (m_selectClause.find(accessString) != m_selectClause.end())
        return true;

    //these system properties are always selected (? is this true??)
    return accessString.EqualsIAscii(ECDBSYS_PROP_ECInstanceId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_ECClassId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_SourceECClassId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId) ||
        accessString.EqualsIAscii(ECDBSYS_PROP_TargetECClassId);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       11/2016
//+---------------+---------------+---------------+---------------+---------------+------
bvector<Utf8String> ECSqlPrepareContext::SelectClauseInfo::Split(Utf8StringCR accessString, Utf8Char separator)
    {
    bvector<Utf8String> output;
    Utf8String::size_type prev_pos = 0, pos = 0;
    while ((pos = accessString.find(separator, pos)) != Utf8String::npos)
        {
        output.push_back(accessString.substr(prev_pos, pos - prev_pos));
        prev_pos = ++pos;
        }

    output.push_back(accessString.substr(prev_pos, pos - prev_pos)); // Last word
    return output;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE


