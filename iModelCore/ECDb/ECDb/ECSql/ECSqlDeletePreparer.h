/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlDeletePreparer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    01/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlDeletePreparer
    {
private:
    struct NativeSqlSnippets
        {
        NativeSqlBuilder m_classNameNativeSqlSnippet;
        NativeSqlBuilder::List m_pkColumnNamesNativeSqlSnippets;
        NativeSqlBuilder::List m_pkValuesNativeSqlSnippets;
        NativeSqlBuilder m_whereClauseNativeSqlSnippet;
        NativeSqlBuilder m_systemWhereClauseNativeSqlSnippet;
        };


    //static class
    ECSqlDeletePreparer ();
    ~ECSqlDeletePreparer ();

    static ECSqlStatus GenerateNativeSqlSnippets (NativeSqlSnippets& deleteNativeSqlSnippets, ECSqlPrepareContext& ctx, 
                        DeleteStatementExp const& exp, ClassNameExp const& classNameExp);

    static ECSqlStatus PrepareForClass (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets);
    static ECSqlStatus PrepareForEndTableRelationship (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, RelationshipClassEndTableMapCR classMap);

    static void BuildNativeSqlDeleteStatement (NativeSqlBuilder& deleteBuilder, NativeSqlSnippets const& deleteNativeSqlSnippets);
    static void BuildNativeSqlUpdateStatement (NativeSqlBuilder& updateBuilder, NativeSqlSnippets const& deleteNativeSqlSnippets,
                                    NativeSqlBuilder::List const& propNamesToUnsetNativeSqlSnippets);

public:
    static ECSqlStatus Prepare (ECSqlPrepareContext& ctx, DeleteStatementExp const& exp);
    };
struct ECSqlDeletePreparer2
    {
    private:

        //static class
        ECSqlDeletePreparer2 ();
        ~ECSqlDeletePreparer2 ();

    public:
        static ECSqlStatus Prepare (ECSqlPrepareContext& ctx, DeleteStatementExp const& exp)
            {
            ctx.SetSqlRenderStrategy (ECSqlPrepareContext::SqlRenderStrategy::V1);
            ctx.PushScope (exp);
            auto classExp = exp.GetClassNameExp ();
            auto& map = classExp->GetInfo ().GetMap ();
            auto viewName = SqlGenerator::BuildViewClassName (map.GetClass ());
            ctx.GetSqlBuilderR ().Append ("DELETE FROM ").Append (viewName.c_str ());
            NativeSqlBuilder whereClause;


            if (auto whereExp = exp.GetOptWhereClauseExp ())
                {
                auto status = ECSqlExpPreparer::PrepareWhereExp (whereClause, ctx, whereExp);
                if (status != ECSqlStatus::Success)
                    return status;
                }
            
            if (!classExp->IsPolymorphic ())
                {
                if (!whereClause.IsEmpty ())
                    whereClause.Append (" AND ");
                else
                    whereClause.Append (" WHERE ");

                whereClause.Append ("ECClassId = ").Append (classExp->GetInfo ().GetMap ().GetClass ().GetId ());
                }

            if (!whereClause.IsEmpty ())
                ctx.GetSqlBuilderR ().Append (whereClause);

            ctx.PopScope ();
            ctx.SetSqlRenderStrategy (ECSqlPrepareContext::SqlRenderStrategy::V0);
            return ECSqlStatus::Success;
            }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE