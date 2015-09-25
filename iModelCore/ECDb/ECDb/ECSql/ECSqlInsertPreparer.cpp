/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlInsertPreparer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <Bentley/BeStringUtilities.h>
#include "ECSqlInsertPreparer.h"
#include "SystemPropertyECSqlBinder.h"
using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::Prepare(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());
    ctx.PushScope(exp);

    auto const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    NativeSqlSnippets insertNativeSqlSnippets;
    ECSqlStatus stat = GenerateNativeSqlSnippets(insertNativeSqlSnippets, ctx, exp, classMap);
    if (!stat.IsSuccess())
        return stat;
    
    if (classMap.IsRelationshipClassMap())
        stat = PrepareInsertIntoRelationship(ctx, insertNativeSqlSnippets, exp , classMap);
    else
        stat = PrepareInsertIntoClass(ctx, insertNativeSqlSnippets, classMap);
    
    if (stat.IsSuccess())
        stat = ECSqlInsertPreparer::PrepareStepTask(ctx, exp);
    
    ctx.PopScope();
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad.zaighum                    8/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
int ECSqlInsertPreparer::GetParamterCount(Exp const& exp, std::set<ParameterExp const*>& namedParameterList)
    {
    int nFoundParameters = 0;
    if (exp.IsParameterExp())
        {
        auto param = static_cast<ParameterExp const*>(&exp);
        if (param->IsNamedParameter())
            {
            if (namedParameterList.find(param) == namedParameterList.end())
                {
                namedParameterList.insert(param);
                }
            }
        else
            nFoundParameters = nFoundParameters + 1;
        }

    for (auto const child : exp.GetChildren())
        {
        if (child->IsParameterExp())
            {
            auto param = static_cast<ParameterExp const*>(child);
            if (param->IsNamedParameter())
                {
                if (namedParameterList.find(param) == namedParameterList.end())
                    {
                    namedParameterList.insert(param);
                    }
                else
                    continue;
                }

            nFoundParameters = nFoundParameters + 1;
            }
        else if (child->GetChildrenCount() > 0)
            {
            nFoundParameters += GetParamterCount(*child, namedParameterList);
            }
        }

    return nFoundParameters;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad.zaighum                    8/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareStepTask(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    auto& ecsqlParameterMap = ctx.GetECSqlStatementR().GetPreparedStatementP()->GetParameterMapR();
    auto noneSelectPreparedStmt = ctx.GetECSqlStatementR().GetPreparedStatementP <ECSqlNonSelectPreparedStatement>();
    BeAssert(noneSelectPreparedStmt != nullptr && "Expecting ECSqlNoneSelectPreparedStatement");
    int ecsqlParameterIndex = 1;
    auto& propertyList = exp.GetPropertyNameListExp()->GetChildren();
    auto& valueList = exp.GetValuesExp()->GetChildren();
    ECSqlBinder* binder = nullptr;
    std::set<ParameterExp const*> namedParameterList;
    for (size_t i = 0; i < exp.GetPropertyNameListExp()->GetChildrenCount(); i++)
        {
        auto propNameExp = static_cast<PropertyNameExp const*> (propertyList[i]);
        auto valueExp = static_cast<ValueExp const*> (valueList[i]);
        auto nParameterInValueExp = GetParamterCount(*valueExp,namedParameterList);
        auto const& typeInfo = propNameExp->GetTypeInfo();

        if (nParameterInValueExp == 0)
            continue;

        if (!ecsqlParameterMap.TryGetBinder(binder, ecsqlParameterIndex).IsSuccess())
            {
            BeAssert(false && "Failed to find binder for given value expression");
            return ECSqlStatus::Error;
            }

        if (typeInfo.GetKind() == ECSqlTypeInfo::Kind::Struct)
            {
            ECSqlStatus stat = SetupBindStructParameter(binder, propNameExp->GetPropertyMap(), noneSelectPreparedStmt, ctx, exp);
            if (!stat.IsSuccess())
                return stat;
            }

        else if (typeInfo.GetKind() == ECSqlTypeInfo::Kind::StructArray)
            {                            
            if (auto structArrayBinder = dynamic_cast<StructArrayToSecondaryTableECSqlBinder*> (binder))
                {
                ECSqlStatus stat = SetupBindStructArrayParameter(structArrayBinder, propNameExp->GetPropertyMap(), noneSelectPreparedStmt, ctx, exp);
                if (!stat.IsSuccess())
                    return stat;
                }
            else
                {
                BeAssert(false && "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                return ECSqlStatus::Error;
                }
            }

        ecsqlParameterIndex += nParameterInValueExp;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad.zaighum                    8/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::SetupBindStructParameter(ECSqlBinder* binder, PropertyMapCR propertyMap, ECSqlNonSelectPreparedStatement* noneSelectPreparedStmt, ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    for (auto childPropertyMap : propertyMap.GetChildren())
        {
        auto& propertyBinder = static_cast<ECSqlBinder&>(binder->BindStruct().GetMember(childPropertyMap->GetProperty().GetName().c_str()));
        if (childPropertyMap->GetProperty().GetIsStruct())
            {
            if (SetupBindStructParameter(&propertyBinder, *childPropertyMap, noneSelectPreparedStmt, ctx, exp) != ECSqlStatus::Success)
                {
                BeAssert(false && "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                return ECSqlStatus::Error;
                }
            }

        else if (childPropertyMap->GetAsPropertyMapToTable() != nullptr)
            {
            auto structArrayBinder = dynamic_cast<StructArrayToSecondaryTableECSqlBinder*> (&propertyBinder);
            if (structArrayBinder == nullptr)
                {
                BeAssert(false && "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                return ECSqlStatus::Error;
                }

            if (SetupBindStructArrayParameter(structArrayBinder, *childPropertyMap, noneSelectPreparedStmt, ctx, exp) != ECSqlStatus::Success)
                {
                BeAssert(false && "Expecting a StructArrayToSecondaryTableECSqlBinder for parameter");
                return ECSqlStatus::Error;
                }
            }
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad.zaighum                    8/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::SetupBindStructArrayParameter(StructArrayToSecondaryTableECSqlBinder* structArrayBinder, PropertyMapCR propertyMap, ECSqlNonSelectPreparedStatement* noneSelectPreparedStmt, ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    auto const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    std::unique_ptr<ECSqlStepTask> stepTask;
    auto propertyName = propertyMap.GetPropertyAccessString();
    auto status = ECSqlStepTaskFactory::CreatePropertyStepTask(stepTask, StepTaskType::Insert, ctx, classMap.GetECDbMap().GetECDbR(), classMap, propertyName);
    if (status != ECSqlStepTaskCreateStatus::NothingToDo)
        {
        if (status != ECSqlStepTaskCreateStatus::Success)
            {
            BeAssert(false && "Failed to create insert step tasks for struct array properties");
            return ECSqlStatus::Error;
            }
        }

    auto& parameterValue = structArrayBinder->GetParameterValue();
    auto structArrayStepTask = static_cast<ParametericStepTask*>(stepTask.get());
    structArrayStepTask->SetParameterSource(parameterValue);
    BeAssert(stepTask != nullptr && "Failed to create step task for struct array");
    noneSelectPreparedStmt->GetStepTasks().Add(move(stepTask));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoClass 
(
ECSqlPrepareContext& ctx, 
NativeSqlSnippets& nativeSqlSnippets, 
IClassMap const& classMap
)
    {
    PreparePrimaryKey(ctx, nativeSqlSnippets, classMap);
    //build SQLite SQL
    BuildNativeSqlInsertStatement(ctx.GetSqlBuilderR (), nativeSqlSnippets);

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoRelationship(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, IClassMap const& classMap)
    {
    BeAssert(classMap.IsRelationshipClassMap());
    BeAssert(ctx.GetClassMapViewMode() == IClassMap::View::DomainClass && "Relationship classes that are also used as structs are not supported.");

    auto const& relationshipClassMap = static_cast<RelationshipClassMapCR> (classMap);

    auto const& specialTokenMap = exp.GetPropertyNameListExp()->GetSpecialTokenExpIndexMap();
    if (specialTokenMap.IsUnset(ECSqlSystemProperty::SourceECInstanceId) || specialTokenMap.IsUnset(ECSqlSystemProperty::TargetECInstanceId))
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "In an ECSQL INSERT statement against an ECRelationship class SourceECInstanceId and TargetECInstanceId must always be specified.");
        return ECSqlStatus::InvalidECSql;
        }

    //Validate and if need be determine SourceECClassId and TargetECClassId
    ECClassId sourceECClassId = ECClass::UNSET_ECCLASSID; //remains unset if is parametrized
    ECSqlStatus stat = ValidateConstraintClassId(sourceECClassId, ctx, exp, relationshipClassMap, ECRelationshipEnd_Source);
    if (!stat.IsSuccess())
        return stat;

    ECClassId targetECClassId = ECClass::UNSET_ECCLASSID;  //remains unset if is parametrized
    stat = ValidateConstraintClassId(targetECClassId, ctx, exp, relationshipClassMap, ECRelationshipEnd_Target);
    if (!stat.IsSuccess())
        return stat;

    if (relationshipClassMap.GetClassMapType() == ClassMap::Type::RelationshipLinkTable)
        return PrepareInsertIntoLinkTableRelationship(ctx, nativeSqlSnippets, relationshipClassMap, sourceECClassId, targetECClassId);
    else
        return PrepareInsertIntoEndTableRelationship(ctx, nativeSqlSnippets, exp, relationshipClassMap, sourceECClassId, targetECClassId);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoLinkTableRelationship(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, RelationshipClassMapCR relationshipClassMap, ECClassId sourceECClassId, ECClassId targetECClassId)
    {
    PreparePrimaryKey(ctx, nativeSqlSnippets, relationshipClassMap);

    if (sourceECClassId >= ECClass::UNSET_ECCLASSID)
        {
        ECSqlStatus stat = PrepareConstraintClassId(nativeSqlSnippets, ctx, *relationshipClassMap.GetSourceECClassIdPropMap(), sourceECClassId);
        if (!stat.IsSuccess())
            return stat;
        }

    if (targetECClassId >= ECClass::UNSET_ECCLASSID)
        {
        ECSqlStatus stat = PrepareConstraintClassId(nativeSqlSnippets, ctx, *relationshipClassMap.GetTargetECClassIdPropMap(), targetECClassId);
        if (!stat.IsSuccess())
            return stat;
        }

    BuildNativeSqlInsertStatement(ctx.GetSqlBuilderR (), nativeSqlSnippets);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoEndTableRelationship(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, RelationshipClassMapCR relationshipClassMap, ECClassId sourceECClassId, ECClassId targetECClassId)
    {
    BeAssert(dynamic_cast<RelationshipClassEndTableMapCP> (&relationshipClassMap) != nullptr);
    auto const& relationshipEndTableMap = static_cast<RelationshipClassEndTableMapCR> (relationshipClassMap);

    const auto thisEnd = relationshipEndTableMap.GetThisEnd();

    auto propNameListExp = exp.GetPropertyNameListExp();
    auto const& specialTokenExpIndexMap = propNameListExp->GetSpecialTokenExpIndexMap();

    std::vector<size_t> expIndexSkipList;
    //if ECInstanceId was specified, put it in skip list as it will always be ignored for end table mappings
    int ecinstanceIdExpIndex = specialTokenExpIndexMap.GetIndex(ECSqlSystemProperty::ECInstanceId);
    if (ecinstanceIdExpIndex >= 0)
        expIndexSkipList.push_back((size_t) ecinstanceIdExpIndex);

    //This end's ecinstanceid is ecinstanceid of relationship instance (by nature of end table mapping)
    int thisEndECInstanceIdIndex = specialTokenExpIndexMap.GetIndex(thisEnd == ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId);
    int thisEndECClassIdIndex = specialTokenExpIndexMap.GetIndex(thisEnd == ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId);

    auto preparedStatement = ctx.GetECSqlStatementR().GetPreparedStatementP<ECSqlInsertPreparedStatement>();

    if (thisEndECInstanceIdIndex >= 0)
        {
        const size_t thisEndECInstanceIdIndexUnsigned = (size_t)thisEndECInstanceIdIndex;
        auto const& ecinstanceIdPropNameSnippets = nativeSqlSnippets.m_propertyNamesNativeSqlSnippets[thisEndECInstanceIdIndexUnsigned];
        nativeSqlSnippets.m_pkColumnNamesNativeSqlSnippets.insert(nativeSqlSnippets.m_pkColumnNamesNativeSqlSnippets.end(), ecinstanceIdPropNameSnippets.begin(), ecinstanceIdPropNameSnippets.end());

        auto const& ecinstanceIdValueSnippets = nativeSqlSnippets.m_valuesNativeSqlSnippets[thisEndECInstanceIdIndexUnsigned];
        if (ecinstanceIdValueSnippets.size() != 1)
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Multi-value ECInstanceIds not supported.");
            return ECSqlStatus::InvalidECSql;
            }

        nativeSqlSnippets.m_pkValuesNativeSqlSnippets.insert(nativeSqlSnippets.m_pkValuesNativeSqlSnippets.end(), ecinstanceIdValueSnippets.begin(), ecinstanceIdValueSnippets.end());
        expIndexSkipList.push_back(thisEndECInstanceIdIndexUnsigned);

        //usually ECInstanceId is auto-generated by ECSqlStatement. For end-table mappings this must not be done, as we have
        //the ECInstanceId already (as it is the same as this end's ECInstanceId).
        auto classId = relationshipClassMap.GetClass().GetId();
        auto parameterExp = exp.GetValuesExp()->TryGetAsParameterExpP (thisEndECInstanceIdIndexUnsigned);
        if (parameterExp != nullptr)
            {
            ECSqlBinder* thisEndECInstanceIdBinder = nullptr;
            ECSqlStatus stat = preparedStatement->GetParameterMapR ().TryGetBinder(thisEndECInstanceIdBinder, parameterExp->GetParameterIndex());
            if (!stat.IsSuccess())
                {
                BeAssert(false && "Could not find this end constraint ECInstanceId parameter binder.");
                return stat;
                }

            BeAssert(thisEndECInstanceIdBinder != nullptr);
            thisEndECInstanceIdBinder->SetOnBindRepositoryBasedIdEventHandler([preparedStatement, classId] (ECInstanceId const& bindValue)
                {
                BeAssert(preparedStatement != nullptr);
                preparedStatement->SetECInstanceKeyInfo(ECSqlInsertPreparedStatement::ECInstanceKeyInfo(classId, ECInstanceId(bindValue.GetValue())));
                });
            }
        else
            {
            auto const& ecinstanceIdValueSnippet = ecinstanceIdValueSnippets[0];
            Utf8CP ecinstanceidStr = ecinstanceIdValueSnippet.ToString();
            ECInstanceId id;
            if (!ECInstanceIdHelper::FromString(id, ecinstanceidStr))
                {
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "'%s' is an invalid ECInstanceId value.", ecinstanceidStr);
                return ECSqlStatus::InvalidECSql;
                }

            BeAssert(id.IsValid());
            preparedStatement->SetECInstanceKeyInfo(ECSqlInsertPreparedStatement::ECInstanceKeyInfo(classId, id));
            }

        }

    if (thisEndECClassIdIndex >= 0)
        {
        expIndexSkipList.push_back((size_t)thisEndECClassIdIndex);

        //if this end was parametrized we must turn the respective binder into a no-op binder
        //so that clients binding values to it will not try to access a SQLite parameter which does not exist
        auto parameterExp = exp.GetValuesExp()->TryGetAsParameterExpP(thisEndECClassIdIndex);
        if (parameterExp != nullptr)
            {
            ECSqlBinder* binder = nullptr;
            preparedStatement->GetParameterMapR().TryGetBinder(binder, parameterExp->GetParameterIndex());
            BeAssert(dynamic_cast<SystemPropertyECSqlBinder*> (binder) != nullptr);
            auto systemPropBinder = static_cast<SystemPropertyECSqlBinder*> (binder);
            systemPropBinder->SetIsNoop();
            }
        }

    std::sort(expIndexSkipList.begin(), expIndexSkipList.end());

    auto otherEndClassId = thisEnd == ECRelationshipEnd_Source ? targetECClassId : sourceECClassId;
    if (otherEndClassId >= ECClass::UNSET_ECCLASSID)
        {
        ECSqlStatus stat = PrepareConstraintClassId(nativeSqlSnippets, ctx, *relationshipEndTableMap.GetOtherEndECClassIdPropMap(), otherEndClassId);
        if (!stat.IsSuccess())
            return stat;
        }

    //now build SQLite SQL
    //Inserting into a relationship with end table mapping translates to an UPDATE statement in SQLite
    BuildNativeSqlUpdateStatement(ctx.GetSqlBuilderR(), nativeSqlSnippets, expIndexSkipList, relationshipEndTableMap);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::GenerateNativeSqlSnippets
(
NativeSqlSnippets& insertSqlSnippets,
ECSqlPrepareContext& ctx,
InsertStatementExp const& exp,
IClassMap const& classMap
)
    {
    ECSqlStatus status = ECSqlExpPreparer::PrepareClassRefExp(insertSqlSnippets.m_classNameNativeSqlSnippet, ctx, *exp.GetClassNameExp());
    if (!status.IsSuccess())
        return status;

    auto propNameListExp = exp.GetPropertyNameListExp();
    status = ECSqlExpPreparer::PreparePropertyNameListExp(insertSqlSnippets.m_propertyNamesNativeSqlSnippets, ctx, propNameListExp);
    if (!status.IsSuccess())
        return status;

    status = ECSqlExpPreparer::PrepareValueExpListExp(insertSqlSnippets.m_valuesNativeSqlSnippets, ctx, exp.GetValuesExp(), propNameListExp, insertSqlSnippets.m_propertyNamesNativeSqlSnippets);
    if (!status.IsSuccess())
        return status;

    if (insertSqlSnippets.m_propertyNamesNativeSqlSnippets.size() != insertSqlSnippets.m_valuesNativeSqlSnippets.size())
        {
        BeAssert(false && "Error preparing insert statement. Number of property name items does not match number of value items. This should have been caught by parser already.");
        return ECSqlStatus::Error;
        }

    insertSqlSnippets.m_ecinstanceIdMode = ValidateUserProvidedECInstanceId(insertSqlSnippets.m_ecinstanceIdExpIndex, ctx, exp, classMap);
    if (insertSqlSnippets.m_ecinstanceIdMode == ECInstanceIdMode::Invalid)
        return ECSqlStatus::InvalidECSql
        ;

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlInsertPreparer::PreparePrimaryKey(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, IClassMap const& classMap)
    {
    //for embedded class view, the ECSQL must contain the primary key props (ParentECInstanceId, ECPropertyId, ECArrayIndex). None
    //of them is auto-generated by ECDb. That case doesn't have a class id either. So therefore just return in that case.
    if (ctx.GetClassMapViewMode() == IClassMap::View::EmbeddedType)
        return;

    const auto ecinstanceIdMode = nativeSqlSnippets.m_ecinstanceIdMode;
    if (ecinstanceIdMode == ECInstanceIdMode::NotUserProvided || ecinstanceIdMode == ECInstanceIdMode::UserProvidedNull) // auto-generate
        {
        //If ECInstanceId is to be auto-generated we need to add the ECInstanceId to the SQLite statement
        //plus an internal binder here.

        int ecinstanceidIndex = -1;
        if (ecinstanceIdMode == ECInstanceIdMode::NotUserProvided)
            {
            //if not user provided ecinstanceid snippet will be appended to column names
            ecinstanceidIndex = (int) nativeSqlSnippets.m_propertyNamesNativeSqlSnippets.size();
            PropertyMapCP ecInstanceIdPropMap = nullptr;
            if (!classMap.GetPropertyMaps().TryGetPropertyMap(ecInstanceIdPropMap, PropertyMapECInstanceId::PROPERTYACCESSSTRING))
                {
                BeAssert(false && "ECInstanceId property map is always expected to exist for domain classes.");
                return;
                }

            nativeSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(move(ecInstanceIdPropMap->ToNativeSql(nullptr, ECSqlType::Insert, false)));
            nativeSqlSnippets.m_valuesNativeSqlSnippets.push_back(NativeSqlBuilder::List {NativeSqlBuilder()});
            }
        else if (ecinstanceIdMode == ECInstanceIdMode::UserProvidedNull)
            {
            BeAssert(nativeSqlSnippets.m_ecinstanceIdExpIndex >= 0 &&
                      (int)nativeSqlSnippets.m_valuesNativeSqlSnippets.size() > nativeSqlSnippets.m_ecinstanceIdExpIndex &&
                      nativeSqlSnippets.m_valuesNativeSqlSnippets[nativeSqlSnippets.m_ecinstanceIdExpIndex].size() == 1);

            ecinstanceidIndex = nativeSqlSnippets.m_ecinstanceIdExpIndex;
            nativeSqlSnippets.m_valuesNativeSqlSnippets[ecinstanceidIndex][0] = NativeSqlBuilder();
            }

        //add binder for the ecinstanceid parameter
        auto& ecsqlStmt = ctx.GetECSqlStatementR ();
        auto preparedECSqlStatement = ctx.GetECSqlStatementR ().GetPreparedStatementP<ECSqlInsertPreparedStatement> ();
        size_t ecinstanceidBinderIndex = 0;
        auto ecinstanceidBinder = preparedECSqlStatement->GetParameterMapR ().AddInternalBinder(ecinstanceidBinderIndex, ecsqlStmt, ECSqlTypeInfo(PRIMITIVETYPE_Long));
        if (ecinstanceidBinder == nullptr)
            {
            BeAssert(false && "Failed to create internal ECInstanceId parameter binder.");
            return;
            }

        preparedECSqlStatement->SetECInstanceKeyInfo(ECSqlInsertPreparedStatement::ECInstanceKeyInfo(classMap.GetClass().GetId(), *ecinstanceidBinder));
        //add SQLite parameter for the ecinstanceid (internal parameters's ECSqlParameterIndex is made negative to distinguish them from real ECSQL parameter)
        nativeSqlSnippets.m_valuesNativeSqlSnippets[ecinstanceidIndex][0].AppendParameter(nullptr, (-1) * (int) ecinstanceidBinderIndex, 1);
        }

    //if table has a class id column, handle this here
    auto classIdColumn = classMap.GetTable().FindColumnCP(ECDB_COL_ECClassId);
    if (classIdColumn != nullptr)
        {
        NativeSqlBuilder::List classIdNameSqliteSnippets {NativeSqlBuilder(classIdColumn->GetName().c_str())};
        nativeSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(move(classIdNameSqliteSnippets));

        NativeSqlBuilder::List classIdSqliteSnippets {NativeSqlBuilder()};
        classIdSqliteSnippets[0].Append(classMap.GetClass().GetId());
        nativeSqlSnippets.m_valuesNativeSqlSnippets.push_back(move(classIdSqliteSnippets));
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::ValidateConstraintClassId
(
ECClassId& retrievedConstraintClassId, 
ECSqlPrepareContext& ctx, 
InsertStatementExp const& exp, 
RelationshipClassMapCR relationshipClassMap,
ECN::ECRelationshipEnd constraintEnd
)
    {
    //This is how the class id handling looks like
    //Case 1: User specified XXClassId in ECSQL
    //      Case 1a: User specified class id value via parameter: Validate at binding time
    //      Case 1b: User specified class id value at prepare time.
    //               Check class id against ids of the classes in the respective constraint of the target relationship.
    //               If specified class id was not found in constraint -> InvalidECSql as user specified mismatching class id
    //               If specified class id was found, return it:
    //Case 2: User did not specify XXClassId in ECSQL
    //        Check constraint that it is not the AnyClass constraint and that it consists of only one class (counting
    //        subclasses, too, in case of a polymorphic constraint).
    //        If check failed -> InvalidECSql as user ommitted XXClassId in case where ECSqlStatement cannot derive it
    //        If check succeeded, return class id
    auto const& constraintMap = relationshipClassMap.GetConstraintMap(constraintEnd);
    auto constraintClassIdPropMap = constraintMap.GetECClassIdPropMap();
    Utf8String constraintClassIdPropName(constraintClassIdPropMap->GetPropertyAccessString());
    int constraintClassIdExpIndex = GetConstraintClassIdExpIndex(exp, constraintEnd);
    if (constraintClassIdExpIndex >= 0)
        //user specified constraint class id in ECSQL
        {
        bool isParameter = false;
        auto stat = GetConstraintClassIdExpValue(isParameter, retrievedConstraintClassId, ctx, *exp.GetValuesExp(), (size_t) constraintClassIdExpIndex, constraintClassIdPropName.c_str());
        if (!stat.IsSuccess())
            return stat;

        //retrievedConstraintClassId < 0 means user specified parameter for it
        if (!isParameter && !constraintMap.ClassIdMatchesConstraint(retrievedConstraintClassId))
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid value %lld for property %s. None of the respective constraint's ECClasses match that ECClassId.",
                                                     retrievedConstraintClassId, constraintClassIdPropName.c_str());
            return ECSqlStatus::InvalidECSql;
            }

        return ECSqlStatus::Success;
        }
    //Sometime SourceECClassId/TargetECClassId  propertyMap is mapped to another table where ECClassId exist.
    //In this case if user did not specify it is not a error..
    if (!constraintClassIdPropMap->IsMappedToPrimaryTable() || constraintClassIdPropMap->GetFirstColumn()->GetKnownColumnId() == ECDbKnownColumns::ECClassId)
        {
        return ECSqlStatus::Success;
        }

    //user did not specify constraint class id in ECSQL -> try to find it which checks whether user should have specified one (because of ambiguity)
    if (!constraintMap.TryGetSingleClassIdFromConstraint(retrievedConstraintClassId))
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "%s can only be omitted from an ECSQL INSERT statement if the constraint consists of only one ECClass (counting subclasses, too, in case of polymorphic constraints) and that ECClass is not 'AnyClass'.",
                                                 constraintClassIdPropName.c_str());
        return ECSqlStatus::InvalidECSql;
        }
 
    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
int ECSqlInsertPreparer::GetConstraintClassIdExpIndex(InsertStatementExp const& exp, ECN::ECRelationshipEnd constraintEnd)
    {
    ECSqlSystemProperty constraintClassIdPropertyKind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId;
    auto propNameListExp = exp.GetPropertyNameListExp();
    return propNameListExp->GetSpecialTokenExpIndexMap().GetIndex(constraintClassIdPropertyKind);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::GetConstraintClassIdExpValue(bool& isParameter, ECClassId& constraintClassId, ECSqlPrepareContext& ctx, ValueExpListExp const& valueListExp, size_t valueExpIndex, Utf8CP constraintClassIdPropertyName)
    {
    auto constraintECClassIdValueExp = valueListExp.GetChildren().Get<Exp> (valueExpIndex);
    const auto expType = constraintECClassIdValueExp->GetType();

    if (expType == Exp::Type::LiteralValue)
        {
        auto constraintECClassIdConstantValueExp = static_cast<LiteralValueExp const*> (constraintECClassIdValueExp);
        auto const& typeInfo = constraintECClassIdConstantValueExp->GetTypeInfo();
        if (!typeInfo.IsExactNumeric())
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Value of %s must be an integral number (which is not NULL).", constraintClassIdPropertyName);
            return ECSqlStatus::InvalidECSql;
            }

        constraintClassId = constraintECClassIdConstantValueExp->GetValueAsInt64();
        isParameter = false;
        return ECSqlStatus::Success;
        }
    else if (expType == Exp::Type::Parameter)
        {
        constraintClassId = ECClass::UNSET_ECCLASSID;
        isParameter = true;
        return ECSqlStatus::Success;
        }


    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "In an ECSQL INSERT statement only literal expressions or parameters are allowed for the %s property.", constraintClassIdPropertyName);
    return ECSqlStatus::InvalidECSql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareConstraintClassId(NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyMapCR constraintClassIdPropMap, ECClassId constraintClassId)
    {
    BeAssert(constraintClassId >= ECClass::UNSET_ECCLASSID);
    //if constraint class id maps to virtual column then ignore it as the column does not exist in the table.
    if (constraintClassIdPropMap.IsVirtual())
        return ECSqlStatus::Success;

    if (!constraintClassIdPropMap.IsMappedToPrimaryTable() || constraintClassIdPropMap.GetFirstColumn()->GetKnownColumnId() == ECDbKnownColumns::ECClassId)
        {
        return ECSqlStatus::Success;
        }

    auto classIdColSqlSnippet = constraintClassIdPropMap.ToNativeSql(nullptr, ECSqlType::Insert, false);
    if (!classIdColSqlSnippet.empty())
        {
        insertNativeSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(move(classIdColSqlSnippet));
        NativeSqlBuilder classIdSnippet;
        classIdSnippet.Append(constraintClassId);
        insertNativeSqlSnippets.m_valuesNativeSqlSnippets.push_back(NativeSqlBuilder::List{ move(classIdSnippet) });
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlInsertPreparer::BuildNativeSqlInsertStatement 
(
NativeSqlBuilder& insertBuilder, 
NativeSqlSnippets const& insertSqlSnippets
)
    {
    //For each expression in the property name / value list, a NativeSqlBuilder::List is created. For simple primitive
    //properties, the list will only contain one snippet, but for multi-dimensional properties (points, structs)
    //the list will contain more than one snippet. Consequently the the list of ECSQL expressions is translated
    //into a list of list of native sql snippets. At this point we don't need that jaggedness anymore and flatten it out
    //before building the final SQLite sql string.
    const std::vector<size_t> emptyIndexSkipList;
    auto propertyNamesNativeSqlSnippets = NativeSqlBuilder::FlattenJaggedList(insertSqlSnippets.m_propertyNamesNativeSqlSnippets, emptyIndexSkipList);
    auto valuesNativeSqlSnippets = NativeSqlBuilder::FlattenJaggedList(insertSqlSnippets.m_valuesNativeSqlSnippets, emptyIndexSkipList);

    insertBuilder.Append("INSERT INTO ").Append(insertSqlSnippets.m_classNameNativeSqlSnippet);

    insertBuilder.AppendSpace().AppendParenLeft().Append(propertyNamesNativeSqlSnippets);
    if (!insertSqlSnippets.m_pkColumnNamesNativeSqlSnippets.empty())
        {
        if (!propertyNamesNativeSqlSnippets.empty())
            insertBuilder.AppendComma();

        insertBuilder.Append(insertSqlSnippets.m_pkColumnNamesNativeSqlSnippets);
        }

    insertBuilder.AppendParenRight();
    insertBuilder.Append(" VALUES ").AppendParenLeft().Append(valuesNativeSqlSnippets);
    if (!insertSqlSnippets.m_pkValuesNativeSqlSnippets.empty())
        {
        if (!valuesNativeSqlSnippets.empty())
            insertBuilder.AppendComma();

        insertBuilder.Append(insertSqlSnippets.m_pkValuesNativeSqlSnippets);
        }

    insertBuilder.AppendParenRight();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlInsertPreparer::BuildNativeSqlUpdateStatement
(
NativeSqlBuilder& updateBuilder,
NativeSqlSnippets const& insertSqlSnippets,
std::vector<size_t> const& expIndexSkipList,
RelationshipClassEndTableMapCR classMap
)
    {
    //For each expression in the property name / value list, a NativeSqlBuilder::List is created. For simple primitive
    //properties, the list will only contain one snippet, but for multi-dimensional properties (points, structs)
    //the list will contain more than one snippet. Consequently the list of ECSQL expressions is translated
    //into a list of list of native sql snippets. At this point we don't need that jaggedness anymore and flatten it out
    //before building the final SQLite sql string.
    auto propertyNamesNativeSqlSnippets = NativeSqlBuilder::FlattenJaggedList(insertSqlSnippets.m_propertyNamesNativeSqlSnippets, expIndexSkipList);
    auto valuesNativeSqlSnippets = NativeSqlBuilder::FlattenJaggedList(insertSqlSnippets.m_valuesNativeSqlSnippets, expIndexSkipList);

    updateBuilder.Append("UPDATE ").Append(insertSqlSnippets.m_classNameNativeSqlSnippet).Append(" SET ");
    updateBuilder.Append(propertyNamesNativeSqlSnippets, " = ", valuesNativeSqlSnippets);

    //add WHERE clause so that the right row in the end table is updated
    updateBuilder.Append(" WHERE ").Append(insertSqlSnippets.m_pkColumnNamesNativeSqlSnippets, " = ", insertSqlSnippets.m_pkValuesNativeSqlSnippets, " AND ");
    //add expression to WHERE clause that only updates the row if the other end id is NULL. If it wasn't NULL, it would mean
    //a cardinality constraint violation, as by definition the other end's cardinality in an end table mapping is 0 or 1.
    auto otherEndECInstanceIdColumnSqlSnippets = classMap.GetOtherEndECInstanceIdPropMap()->ToNativeSql(nullptr, ECSqlType::Update, false);
    for (auto const& otherEndECInstanceIdColSnippet : otherEndECInstanceIdColumnSqlSnippets)
        {
        updateBuilder.Append(" AND ").Append(otherEndECInstanceIdColSnippet).Append(" IS NULL");
        }
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlInsertPreparer::ECInstanceIdMode ECSqlInsertPreparer::ValidateUserProvidedECInstanceId(int& ecinstanceIdExpIndex, ECSqlPrepareContext& ctx, InsertStatementExp const& exp, IClassMap const& classMap)
    {
    ecinstanceIdExpIndex = -1;

    if (ctx.IsEmbeddedStatement())
        return ECInstanceIdMode::NotUserProvided; 

    ECSqlInsertPreparedStatement* preparedStatement = ctx.GetECSqlStatementR().GetPreparedStatementP<ECSqlInsertPreparedStatement>();
    BeAssert(preparedStatement != nullptr);

    if (classMap.IsECInstanceIdAutogenerationDisabled())
        preparedStatement->SetIsECInstanceIdAutogenerationDisabled();

    //Validate whether ECInstanceId is specified and value is set to NULL -> auto-generate ECInstanceId
    auto propNameListExp = exp.GetPropertyNameListExp();
    ecinstanceIdExpIndex = propNameListExp->GetSpecialTokenExpIndexMap().GetIndex(ECSqlSystemProperty::ECInstanceId);
    if (ecinstanceIdExpIndex < 0)
        return ECInstanceIdMode::NotUserProvided; //-> auto-generate

    auto valueExp = exp.GetValuesExp()->GetChildren().Get<Exp> (ecinstanceIdExpIndex);
    BeAssert(valueExp != nullptr);
    if (ECSqlExpPreparer::IsNullExp(*valueExp))
        return ECInstanceIdMode::UserProvidedNull; //-> auto-generate
        
    //now we know that user provided a not-null ECInstanceId

    //for end table relationships we ignore the user provided ECInstanceId
    //as end table relationships don't hve their own ECInstanceId
    const bool isEndTableRelationship = classMap.GetClassMapType() == IClassMap::Type::RelationshipEndTable;

    ECClassId classId = classMap.GetClass().GetId();
    const Exp::Type expType = valueExp->GetType();
    if (expType == Exp::Type::LiteralValue)
        {
        if (!isEndTableRelationship)
            {
            LiteralValueExp const* constValueExp = static_cast<LiteralValueExp const*> (valueExp);
            ECInstanceId instanceId(constValueExp->GetValueAsInt64());
            preparedStatement->SetECInstanceKeyInfo(ECSqlInsertPreparedStatement::ECInstanceKeyInfo(classId, instanceId));
            }
        }
    else if (expType == Exp::Type::Parameter)
        {
        ParameterExp const* paramExp = static_cast<ParameterExp const*> (valueExp);

        ECSqlBinder* ecinstanceidBinder = nullptr;
        auto stat = preparedStatement->GetParameterMapR ().TryGetBinder(ecinstanceidBinder, paramExp->GetParameterIndex());
        if (!stat.IsSuccess())
            {
            BeAssert(false && "Could not find ECInstanceId parameter binder.");
            return ECInstanceIdMode::Invalid;
            }

        BeAssert(ecinstanceidBinder != nullptr);

        if (isEndTableRelationship)
            {
            //for end table relationships we ignore the user provided ECInstanceId
            //as end table relationships don't hve their own ECInstanceId
            BeAssert(dynamic_cast<SystemPropertyECSqlBinder*> (ecinstanceidBinder) != nullptr);
            auto systemPropBinder = static_cast<SystemPropertyECSqlBinder*> (ecinstanceidBinder);
            systemPropBinder->SetIsNoop();
            }
        else
            {
            //capture the bound ecinstanceid in the prepared statement so that it can be returned from Step
            ecinstanceidBinder->SetOnBindRepositoryBasedIdEventHandler([preparedStatement] (ECInstanceId const& bindValue)
                {
                preparedStatement->GetECInstanceKeyInfo().SetBoundECInstanceId(ECInstanceId(bindValue.GetValue()));
                });

            preparedStatement->SetECInstanceKeyInfo(ECSqlInsertPreparedStatement::ECInstanceKeyInfo(classId, *ecinstanceidBinder));
            }
        }
    else
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECInstanceId in an ECSQL INSERT statement can only be NULL, a literal or a parameter.");
        return ECInstanceIdMode::Invalid;
        }

    return ECInstanceIdMode::UserProvidedNotNull;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
