/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlUpdatePreparer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlUpdatePreparer.h"
#include "ECSqlPropertyNameExpPreparer.h"
#include "StructArrayToSecondaryTableECSqlBinder.h"
using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::Prepare (ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    BeAssert (exp.IsComplete ());
    ctx.PushScope (exp);

    ClassNameExp const* classNameExp = exp.GetClassNameExp ();

    Exp::SystemPropertyExpIndexMap const& specialTokenExpIndexMap = exp.GetAssignmentListExp()->GetSpecialTokenExpIndexMap();
    if (!specialTokenExpIndexMap.IsUnset (ECSqlSystemProperty::ECInstanceId))
        return ctx.SetError (ECSqlStatus::InvalidECSql, "ECInstanceId is not allowed in SET clause of ECSQL UPDATE statement. ECDb does not support to modify auto-generated ECInstanceIds.");

    IClassMap const& classMap = classNameExp->GetInfo ().GetMap ();
    if (classMap.IsRelationshipClassMap ())
        {
        if (!specialTokenExpIndexMap.IsUnset (ECSqlSystemProperty::SourceECInstanceId) ||
            !specialTokenExpIndexMap.IsUnset (ECSqlSystemProperty::SourceECClassId) ||
            !specialTokenExpIndexMap.IsUnset (ECSqlSystemProperty::TargetECInstanceId) ||
            !specialTokenExpIndexMap.IsUnset (ECSqlSystemProperty::TargetECClassId))
            return ctx.SetError (ECSqlStatus::InvalidECSql, "SourceECInstanceId, TargetECInstanceId, SourceECClassId, or TargetECClassId are not allowed in the SET clause of ECSQL UPDATE statement. ECDb does not support to modify those as they are keys of the relationship. Instead delete the relationship and insert the desired new one.");
        }

    NativeSqlBuilder& nativeSqlBuilder = ctx.GetSqlBuilderR ();
    
    // UPDATE clause
    nativeSqlBuilder.Append ("UPDATE ");
    auto status = ECSqlExpPreparer::PrepareClassRefExp (nativeSqlBuilder, ctx, classNameExp);
    if (status != ECSqlStatus::Success)
        return status;

    //PropertyValueMap& propertyValueMap;
    // SET clause
    NativeSqlBuilder::ListOfLists assignmentListSnippetLists;
    status = PrepareAssignmentListExp (assignmentListSnippetLists, ctx, exp.GetAssignmentListExp ());
    if (status != ECSqlStatus::Success)
        return status;

    const std::vector<size_t> emptyIndexSkipList;
    auto assignmentListSnippets = NativeSqlBuilder::FlattenJaggedList (assignmentListSnippetLists, emptyIndexSkipList);
    nativeSqlBuilder.Append (" SET ").Append (assignmentListSnippets);
    if (assignmentListSnippets.size() == 0)
        ctx.SetNativeNothingToUpdate(true);

    bool hasWhereClause = false;
    if (auto whereClauseExp = exp.GetOptWhereClauseExp ())
        {
        nativeSqlBuilder.AppendSpace ();
        status = ECSqlExpPreparer::PrepareWhereExp(nativeSqlBuilder, ctx, whereClauseExp);
        if (status != ECSqlStatus::Success)
            return status;

        hasWhereClause = true;
        }

    // WHERE clause
    NativeSqlBuilder systemWhereClause;
    //status = SystemColumnPreparer::GetFor(classMap).GetWhereClause(ctx, systemWhereClause, classMap, ECSqlType::Update,
    //                classNameExp->IsPolymorphic (), nullptr); //SQLite UPDATE does not allow table aliases

    //if (status != ECSqlStatus::Success)
    //    return status;

    auto& storageDesc = classMap.GetStorageDescription ();
    if (storageDesc.GetNonVirtualHorizontalPartitionIndices ().empty () || !exp.GetClassNameExp ()->IsPolymorphic ())
        {
        if (auto classIdColumn = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId))
            {
            if (classIdColumn->GetPersistenceType () == PersistenceType::Persisted)
                {
                systemWhereClause.AppendEscaped (classIdColumn->GetName ().c_str ()).Append (" = ").Append (classMap.GetClass ().GetId ());
                }
            }
        }
    else if (storageDesc.GetNonVirtualHorizontalPartitionIndices ().size () == 1 && exp.GetClassNameExp ()->IsPolymorphic ())
        {
        if (auto classIdColumn = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId))
            {
            if (classIdColumn->GetPersistenceType () == PersistenceType::Persisted)
                {
                auto& partition = storageDesc.GetHorizontalPartitions ().at (storageDesc.GetNonVirtualHorizontalPartitionIndices ().at (0));               
                if (partition.NeedsClassIdFilter()) 
                    {
                    systemWhereClause.AppendEscaped(classIdColumn->GetName().c_str());
                    partition.AppendECClassIdFilterSql(systemWhereClause);
                    }
                }
            }
        }

    if (!systemWhereClause.IsEmpty ())
        {
        if (!hasWhereClause)
            nativeSqlBuilder.Append (" WHERE ");
        else
            nativeSqlBuilder.Append (" AND ");

        nativeSqlBuilder.Append (systemWhereClause);
        }

    status = PrepareStepTask (ctx, exp);

    ctx.PopScope ();
    return status;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    04/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::PrepareStepTask (ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    auto& ecsqlParameterMap = ctx.GetECSqlStatementR().GetPreparedStatementP()->GetParameterMapR();
    auto const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    auto noneSelectPreparedStmt = ctx.GetECSqlStatementR().GetPreparedStatementP <ECSqlNonSelectPreparedStatement>();
    BeAssert(noneSelectPreparedStmt != nullptr && "Expecting ECSqlNoneSelectPreparedStatement");
    int ecsqlParameterIndex = 1;
    ECSqlBinder* binder = nullptr;
    for (auto childExp : exp.GetAssignmentListExp()->GetChildren())
        {
        auto assignementExp = static_cast<AssignmentExp const*> (childExp);
        auto propNameExp = assignementExp->GetPropertyNameExp();
        auto& typeInfo = propNameExp->GetTypeInfo();
        if (ecsqlParameterMap.TryGetBinder(binder, ecsqlParameterIndex) != ECSqlStatus::Success)
            {
            continue;
            }

        ECSqlStatus stat;
        if (typeInfo.GetKind() == ECSqlTypeInfo::Kind::Struct)
            {
            stat = StructPrepareStepTask(assignementExp, classMap, propNameExp->GetPropertyMap(), binder, noneSelectPreparedStmt, ctx, exp);
            if (stat != ECSqlStatus::Success)
                {
                BeAssert(false && "PrepareStepTask Failed for Struct");
                return ctx.SetError(ECSqlStatus::ProgrammerError, "PrepareStepTask Failed for Struct ");
                }

            }

        else if (typeInfo.GetKind() == ECSqlTypeInfo::Kind::StructArray)
            {
            stat = StructArrayPrepareStepTask(assignementExp, classMap, propNameExp->GetPropertyMap(), binder, noneSelectPreparedStmt, ctx, exp);
            if (stat != ECSqlStatus::Success)
                {
                BeAssert(false && "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                return ctx.SetError(ECSqlStatus::ProgrammerError, "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                }

            }

        ecsqlParameterIndex++;
        }

    auto selectorQuery = ECSqlPrepareContext::CreateECInstanceIdSelectionQuery(ctx, *exp.GetClassNameExp(), exp.GetOptWhereClauseExp());
    auto selectorStmt = noneSelectPreparedStmt->GetStepTasks().GetSelector(true);
    selectorStmt->Initialize(ctx, ctx.GetParentArrayProperty(), nullptr);
    auto stat = selectorStmt->Prepare(classMap.GetECDbMap().GetECDbR(), selectorQuery.c_str());
    if (stat != ECSqlStatus::Success)
        {
        BeAssert(false && "Fail to prepared statement for ECInstanceIdSelect. Possible case of struct array containing struct array");
        return stat;
        }

    int parameterIndex = ECSqlPrepareContext::FindLastParameterIndexBeforeWhereClause(exp, exp.GetOptWhereClauseExp());
    auto nParamterToBind = static_cast<int>(ecsqlParameterMap.Count()) - parameterIndex;
    for (auto j = 1; j <= nParamterToBind; j++)
        {
        auto& sink = selectorStmt->GetBinder(j);
        ECSqlBinder* source = nullptr;
        auto status = ecsqlParameterMap.TryGetBinder(source, j + parameterIndex);
        if (status == ECSqlStatus::Success)
            source->SetOnBindEventHandler(sink);
        else
            return status;
        }

    return ECSqlStatus::Success;
    }


ECSqlStatus ECSqlUpdatePreparer::StructArrayPrepareStepTask(const AssignmentExp* assignementExp, const IClassMap &classMap, PropertyMapCR propNameExp, ECSqlBinder* binder, ECSqlNonSelectPreparedStatement* noneSelectPreparedStmt, ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    auto stepTaskType = ECSqlExpPreparer::IsNullExp(*assignementExp->GetValueExp())
        ? StepTaskType::Delete : StepTaskType::Update;

    std::unique_ptr<ECSqlStepTask> stepTask;

    auto status = ECSqlStepTaskFactory::CreatePropertyStepTask(stepTask, stepTaskType, ctx, classMap.GetECDbMap().GetECDbR(), classMap, propNameExp.GetPropertyAccessString());
    if (status != ECSqlStepTaskCreateStatus::NothingToDo)
        {
        if (status != ECSqlStepTaskCreateStatus::Success)
            return ctx.SetError(ECSqlStatus::InvalidECSql, "Failed to create insert step tasks for struct array properties");
        }

    if (stepTaskType == StepTaskType::Update)
        {
        auto structArrayBinder = dynamic_cast<StructArrayToSecondaryTableECSqlBinder*> (binder);
        if (structArrayBinder == nullptr)
            {
            BeAssert(false && "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
            return ctx.SetError(ECSqlStatus::ProgrammerError, "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
            }

        auto& parameterValue = structArrayBinder->GetParameterValue();
        auto structArrayStepTask = static_cast<ParametericStepTask*>(stepTask.get());
        structArrayStepTask->SetParameterSource(parameterValue);
        }

    BeAssert(stepTask != nullptr && "Failed to create step task for struct array");
    noneSelectPreparedStmt->GetStepTasks().Add(move(stepTask));
   
    return ECSqlStatus::Success;

    }

ECSqlStatus ECSqlUpdatePreparer::StructPrepareStepTask(const AssignmentExp* assignementExp, const IClassMap &classMap, PropertyMapCR propertyMap, ECSqlBinder* binder, ECSqlNonSelectPreparedStatement* noneSelectPreparedStmt, ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    for (auto childPropertyMap : propertyMap.GetChildren())
        {
        auto& propertyBinder = static_cast<ECSqlBinder&>(binder->BindStruct().GetMember(childPropertyMap->GetProperty().GetName().c_str()));
        if (childPropertyMap->GetProperty().GetIsStruct())
            {
            if ((StructPrepareStepTask(assignementExp, classMap, *childPropertyMap, &propertyBinder, noneSelectPreparedStmt, ctx, exp)) != ECSqlStatus::Success)
                {
                BeAssert(false && "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                return ctx.SetError(ECSqlStatus::ProgrammerError, "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                }
            }

        else if (childPropertyMap->GetAsPropertyMapToTable() != nullptr)
            {
            if (StructArrayPrepareStepTask(assignementExp, classMap, *childPropertyMap, &propertyBinder, noneSelectPreparedStmt, ctx, exp) != ECSqlStatus::Success)
                {
                BeAssert(false && "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                return ctx.SetError(ECSqlStatus::ProgrammerError, "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                }
            }
        }

    return ECSqlStatus::Success;
        }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::PrepareAssignmentListExp (NativeSqlBuilder::ListOfLists& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, AssignmentListExp const* assignmentListExp)
    {
    ctx.PushScope (*assignmentListExp);
    BeAssert (nativeSqlSnippetLists.empty ());
    for (auto childExp : assignmentListExp->GetChildren ())
        {
        BeAssert (childExp != nullptr);

        auto assignmentExp = static_cast<AssignmentExp const*> (childExp);
        NativeSqlBuilder::List nativeSqlSnippets;
        auto stat = ECSqlPropertyNameExpPreparer::Prepare (nativeSqlSnippets, ctx, assignmentExp->GetPropertyNameExp ());
        if (stat != ECSqlStatus::Success)
            {
            ctx.PopScope ();
            return stat;
            }

        const auto sqlSnippetCount = nativeSqlSnippets.size ();
        //If target expression does not have any SQL snippets, it means the expression is not necessary in SQLite SQL (e.g. for source/target class id props)
        //In that case the respective value exp does not need to be prepared either.
        if (sqlSnippetCount > 0)
            {
            NativeSqlBuilder::List rhsNativeSqlSnippets;
            auto valueExp = assignmentExp->GetValueExp ();
            //if value is null exp, we need to pass target operand snippets
            if (ECSqlExpPreparer::IsNullExp (*valueExp))
                {
                BeAssert (dynamic_cast<ConstantValueExp const*> (valueExp) != nullptr);
                stat = ECSqlExpPreparer::PrepareNullConstantValueExp (rhsNativeSqlSnippets, ctx, static_cast<ConstantValueExp const*> (valueExp), sqlSnippetCount);
                }
            else
                stat = ECSqlExpPreparer::PrepareValueExp (rhsNativeSqlSnippets, ctx, valueExp);

            if (stat != ECSqlStatus::Success)
                {
                ctx.PopScope ();
                return stat;
                }

            if (sqlSnippetCount != rhsNativeSqlSnippets.size ())
                return ctx.SetError (ECSqlStatus::ProgrammerError, "LHS and RHS SQLite SQL snippet count differs for the ECSQL UPDATE assignment '%s'", assignmentExp->ToECSql ().c_str ());

            for (size_t i = 0; i < sqlSnippetCount; i++)
                {
                nativeSqlSnippets[i].Append (" = ").Append (rhsNativeSqlSnippets[i]);
                }
            }
        else
            {
            if (assignmentExp->GetPropertyNameExp ()->GetTypeInfo ().GetKind () == ECSqlTypeInfo::Kind::StructArray)
                {
                auto valueExp = assignmentExp->GetValueExp ();
                if (!ECSqlExpPreparer::IsNullExp (*valueExp))
                    {
                    NativeSqlBuilder::List rhsNativeSqlSnippets;
                    auto valueExp = assignmentExp->GetValueExp ();
                    stat = ECSqlExpPreparer::PrepareValueExp (rhsNativeSqlSnippets, ctx, valueExp);
                    }
                }

            }

        nativeSqlSnippetLists.push_back (move (nativeSqlSnippets));
        }

    ctx.PopScope ();
    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
