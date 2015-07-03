/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlDeletePreparer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::Prepare (ECSqlPrepareContext& ctx, DeleteStatementExp const& exp)
    {
    BeAssert (exp.IsComplete ());
    ctx.PushScope (exp);

    auto classNameExp = exp.GetClassNameExp ();
    auto const& classMap = classNameExp->GetInfo ().GetMap ();
 
    NativeSqlSnippets deleteNativeSqlSnippets;
    auto stat = GenerateNativeSqlSnippets (deleteNativeSqlSnippets, ctx, exp, *classNameExp);
    if (stat != ECSqlStatus::Success)
        return stat;

    if (classMap.GetClassMapType () == ClassMap::Type::RelationshipEndTable)
        stat = PrepareForEndTableRelationship (ctx, deleteNativeSqlSnippets, static_cast<RelationshipClassEndTableMapCR> (classMap));
    else
        stat = PrepareForClass (ctx, deleteNativeSqlSnippets);

    //Create child delete step task for delete
    auto noneSelectPreparedStmt = ctx.GetECSqlStatementR ().GetPreparedStatementP <ECSqlNonSelectPreparedStatement> ();

    BeAssert (noneSelectPreparedStmt != nullptr && "Expecting ECSqlNonSelectPreparedStatement");

    auto status = ECSqlStepTaskFactory::CreateClassStepTask (
        noneSelectPreparedStmt->GetStepTasks (),
        StepTaskType::Delete,
        ctx,
        classMap.GetECDbMap ().GetECDbR (),
        classMap,
        classNameExp->IsPolymorphic ());
    if (status != ECSqlStepTaskCreateStatus::NothingToDo)
        {
        if (status != ECSqlStepTaskCreateStatus::Success)
            return ctx.SetError (ECSqlStatus::InvalidECSql, "Failed to create delete step tasks for struct array properties");
        }

    //if (noneSelectPreparedStmt->GetStepTasks ().Size () > 0)
    //    {
        auto& ecsqlParameterMap = ctx.GetECSqlStatementR ().GetPreparedStatementP ()->GetParameterMapR ();
        auto selectorQuery = ECSqlPrepareContext::CreateECInstanceIdSelectionQuery (ctx, *exp.GetClassNameExp (), exp.GetOptWhereClauseExp ());
        auto selectorStmt = noneSelectPreparedStmt->GetStepTasks ().GetSelector ( true );
        selectorStmt->Initialize (ctx, ctx.GetParentArrayProperty (), nullptr);
        stat = selectorStmt->Prepare (classMap.GetECDbMap ().GetECDbR (), selectorQuery.c_str ());
        if (stat != ECSqlStatus::Success)
            {
            BeAssert (false && "Fail to prepared statement for ECInstanceIdSelect. Possible case of struct array containing struct array");
            return stat;
            }

        int parameterIndex = ECSqlPrepareContext::FindLastParameterIndexBeforeWhereClause (exp, exp.GetOptWhereClauseExp ());
        auto nParamterToBind = static_cast<int>(ecsqlParameterMap.Count ()) - parameterIndex;
        for (auto j = 1; j <= nParamterToBind; j++)
            {
            auto& sink = selectorStmt->GetBinder (j);
            ECSqlBinder* source = nullptr;
            auto status = ecsqlParameterMap.TryGetBinder (source, j + parameterIndex);
            if (status == ECSqlStatus::Success)
                source->SetOnBindEventHandler (sink);
            else
                return status;
            }
        //}
    ctx.PopScope ();
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::PrepareForClass 
(
ECSqlPrepareContext& ctx, 
NativeSqlSnippets& nativeSqlSnippets
)
    {
    BuildNativeSqlDeleteStatement (ctx.GetSqlBuilderR (), nativeSqlSnippets); 
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::PrepareForEndTableRelationship 
(
ECSqlPrepareContext& ctx, 
NativeSqlSnippets& nativeSqlSnippets, 
RelationshipClassEndTableMapCR classMap
)
    {
    auto otherEndECInstanceIdPropMap = classMap.GetOtherEndECInstanceIdPropMap ();
    auto otherEndECClassIdPropMap = classMap.GetOtherEndECInstanceIdPropMap ();

    NativeSqlBuilder::List propertyNamesToUnsetSqlSnippets;
    classMap.GetPropertyMaps ().Traverse (
        [&propertyNamesToUnsetSqlSnippets, &otherEndECInstanceIdPropMap, &otherEndECClassIdPropMap] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        //virtual prop maps map to non-existing columns. So they don't need to be considered in the list of columns to be nulled out
        if (!propMap->IsVirtual () && (!propMap->IsSystemPropertyMap () || propMap == otherEndECInstanceIdPropMap || propMap == otherEndECClassIdPropMap))
            {
            auto sqlSnippets = propMap->ToNativeSql (nullptr, ECSqlType::Delete, false);
            propertyNamesToUnsetSqlSnippets.insert (propertyNamesToUnsetSqlSnippets.end (), sqlSnippets.begin (), sqlSnippets.end ());
            }
        }, 
        false); //don't recurse, i.e. only top-level prop maps are interesting here


    BuildNativeSqlUpdateStatement (ctx.GetSqlBuilderR (), nativeSqlSnippets, propertyNamesToUnsetSqlSnippets);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::GenerateNativeSqlSnippets 
(
NativeSqlSnippets& deleteSqlSnippets, 
ECSqlPrepareContext& ctx, 
DeleteStatementExp const& exp,
ClassNameExp const& classNameExp
)
    {
    auto status = ECSqlExpPreparer::PrepareClassRefExp (deleteSqlSnippets.m_classNameNativeSqlSnippet, ctx, &classNameExp);
    if (status != ECSqlStatus::Success)
        return status;

    if (auto whereClauseExp = exp.GetOptWhereClauseExp ())
        {
        status = ECSqlExpPreparer::PrepareWhereExp (deleteSqlSnippets.m_whereClauseNativeSqlSnippet, ctx, whereClauseExp);
        if (status != ECSqlStatus::Success)
            return status;
        }

    if (ctx.GetParentContext() == nullptr)
        {
        IClassMap const& classMap = classNameExp.GetInfo().GetMap();

        NativeSqlBuilder systemWhereClause;
        status = SystemColumnPreparer::GetFor(classMap).GetWhereClause(ctx, systemWhereClause, classMap, ECSqlType::Delete,
                                                                       classNameExp.IsPolymorphic(), nullptr); //no table aliases allowed in SQLite DELETE statement
        if (status != ECSqlStatus::Success)
            return status;

        deleteSqlSnippets.m_systemWhereClauseNativeSqlSnippet = std::move(systemWhereClause);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlDeletePreparer::BuildNativeSqlDeleteStatement
(
NativeSqlBuilder& deleteBuilder,
NativeSqlSnippets const& deleteNativeSqlSnippets
)
    {
    deleteBuilder.Append ("DELETE FROM ").Append (deleteNativeSqlSnippets.m_classNameNativeSqlSnippet);

    bool whereAlreadyAppended = false;
    if (!deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet.IsEmpty ())
        {
        deleteBuilder.AppendSpace ().Append (deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet);
        whereAlreadyAppended = true;
        }
    
    if (!deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet.IsEmpty ())
        {
        if (whereAlreadyAppended)
            deleteBuilder.Append (" AND ");
        else
            deleteBuilder.Append (" WHERE ");

        deleteBuilder.Append (deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlDeletePreparer::BuildNativeSqlUpdateStatement
(
NativeSqlBuilder& updateBuilder,
NativeSqlSnippets const& deleteNativeSqlSnippets,
NativeSqlBuilder::List const& propNamesToUnsetNativeSqlSnippets
)
    {
    updateBuilder.Append ("UPDATE ").Append (deleteNativeSqlSnippets.m_classNameNativeSqlSnippet);

    //Columns of properties of the relationship need to be nulled out when "deleting" the relationship
    BeAssert (!propNamesToUnsetNativeSqlSnippets.empty ());
    updateBuilder.Append (" SET ");
    
    bool isFirstItem = true;
    for (auto const& sqlSnippet : propNamesToUnsetNativeSqlSnippets)
        {
        if (!isFirstItem)
            updateBuilder.AppendComma ();

        updateBuilder.Append (sqlSnippet).Append (" = NULL");
        isFirstItem = false;
        }


    bool whereAlreadyAppended = false;
    if (!deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet.IsEmpty ())
        {
        updateBuilder.AppendSpace ().Append (deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet);
        whereAlreadyAppended = true;
        }

    if (!deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet.IsEmpty ())
        {
        if (whereAlreadyAppended)
            updateBuilder.Append (" AND ");
        else
            updateBuilder.Append (" WHERE ");

        updateBuilder.Append (deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet);
        }
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
