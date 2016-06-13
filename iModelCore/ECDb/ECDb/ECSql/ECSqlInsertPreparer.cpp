/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlInsertPreparer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <Bentley/BeStringUtilities.h>
#include "ECSqlInsertPreparer.h"
#include "SystemPropertyECSqlBinder.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus ECSqlInsertPreparer::Prepare(ECSqlPrepareContext& ctx, InsertStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());
    ctx.PushScope(exp);

    auto const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    if (auto info = ctx.GetJoinedTableInfo())
        {
        ParentOfJoinedTableECSqlStatement* parentOfJoinedTableStmt = ctx.GetECSqlStatementR().GetPreparedStatementP()->CreateParentOfJoinedTableECSqlStatement(classMap.GetClass().GetId());
        ECSqlStatus status = parentOfJoinedTableStmt->Prepare(ctx.GetECDb(), info->GetParentOfJoinedTableECSql());
        if (status != ECSqlStatus::Success)
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Preparing the ECSQL '%s' failed. Preparing the primary table ECSQL '%s' failed", exp.ToECSql().c_str(), info->GetParentOfJoinedTableECSql());
            return ECSqlStatus::InvalidECSql;
            }

        if (info->GetPrimaryECinstanceIdParameterIndex() > 0)
            parentOfJoinedTableStmt->SetECInstanceIdBinder(static_cast<int>(info->GetPrimaryECinstanceIdParameterIndex()));
        }


    NativeSqlSnippets insertNativeSqlSnippets;
    ECSqlStatus stat = GenerateNativeSqlSnippets(insertNativeSqlSnippets, ctx, exp, classMap);
    if (!stat.IsSuccess())
        return stat;

    if (classMap.IsRelationshipClassMap())
        stat = PrepareInsertIntoRelationship(ctx, insertNativeSqlSnippets, exp , classMap);
    else
        stat = PrepareInsertIntoClass(ctx, insertNativeSqlSnippets, classMap);
    
    ctx.PopScope();
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoClass 
(
ECSqlPrepareContext& ctx, 
NativeSqlSnippets& nativeSqlSnippets, 
ClassMap const& classMap
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
ECSqlStatus ECSqlInsertPreparer::PrepareInsertIntoRelationship(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, ClassMap const& classMap)
    {
    BeAssert(classMap.IsRelationshipClassMap());

    auto const& specialTokenMap = exp.GetPropertyNameListExp()->GetSpecialTokenExpIndexMap();
    if (specialTokenMap.IsUnset(ECSqlSystemProperty::SourceECInstanceId) || specialTokenMap.IsUnset(ECSqlSystemProperty::TargetECInstanceId))
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "In an ECSQL INSERT statement against an ECRelationship class SourceECInstanceId and TargetECInstanceId must always be specified.");
        return ECSqlStatus::InvalidECSql;
        }

    RelationshipClassMapCR relationshipClassMap = static_cast<RelationshipClassMapCR> (classMap);
    //Validate and if need be determine SourceECClassId and TargetECClassId
    ECClassId sourceECClassId; //remains unset if is parametrized
    ECSqlStatus stat = ValidateConstraintClassId(sourceECClassId, ctx, exp, relationshipClassMap, ECRelationshipEnd_Source);
    if (!stat.IsSuccess())
        return stat;

    ECClassId targetECClassId;  //remains unset if is parametrized
    stat = ValidateConstraintClassId(targetECClassId, ctx, exp, relationshipClassMap, ECRelationshipEnd_Target);
    if (!stat.IsSuccess())
        return stat;

    if (relationshipClassMap.GetType() == ClassMap::Type::RelationshipLinkTable)
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

    if (sourceECClassId.IsValid())
        {
        ECSqlStatus stat = PrepareConstraintClassId(nativeSqlSnippets, ctx, *relationshipClassMap.GetSourceECClassIdPropMap(), sourceECClassId);
        if (!stat.IsSuccess())
            return stat;
        }

    if (targetECClassId.IsValid())
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
    BeAssert(dynamic_cast<RelationshipClassEndTableMap const*> (&relationshipClassMap) != nullptr);
    auto const& relationshipEndTableMap = static_cast<RelationshipClassEndTableMap const&> (relationshipClassMap);

    const auto foreignEnd = relationshipEndTableMap.GetForeignEnd();

    auto propNameListExp = exp.GetPropertyNameListExp();
    auto const& specialTokenExpIndexMap = propNameListExp->GetSpecialTokenExpIndexMap();

    std::vector<size_t> expIndexSkipList;
    //if ECInstanceId was specified, put it in skip list as it will always be ignored for end table mappings
    int ecinstanceIdExpIndex = specialTokenExpIndexMap.GetIndex(ECSqlSystemProperty::ECInstanceId);
    if (ecinstanceIdExpIndex >= 0)
        expIndexSkipList.push_back((size_t) ecinstanceIdExpIndex);

    //This end's ecinstanceid is ecinstanceid of relationship instance (by nature of end table mapping)
    int foreignEndECInstanceIdIndex = specialTokenExpIndexMap.GetIndex(foreignEnd == ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId);
    int foreignEndECClassIdIndex = specialTokenExpIndexMap.GetIndex(foreignEnd == ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId);
    int referencedEndECClassIdIndex = specialTokenExpIndexMap.GetIndex(foreignEnd == ECRelationshipEnd_Target ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId);
    auto preparedStatement = ctx.GetECSqlStatementR().GetPreparedStatementP<ECSqlInsertPreparedStatement>();

    if (foreignEndECInstanceIdIndex >= 0)
        {
        const size_t foreignEndECInstanceIdIndexUnsigned = (size_t)foreignEndECInstanceIdIndex;
        auto const& ecinstanceIdPropNameSnippets = nativeSqlSnippets.m_propertyNamesNativeSqlSnippets[foreignEndECInstanceIdIndexUnsigned];
        nativeSqlSnippets.m_pkColumnNamesNativeSqlSnippets.insert(nativeSqlSnippets.m_pkColumnNamesNativeSqlSnippets.end(), ecinstanceIdPropNameSnippets.begin(), ecinstanceIdPropNameSnippets.end());

        auto const& ecinstanceIdValueSnippets = nativeSqlSnippets.m_valuesNativeSqlSnippets[foreignEndECInstanceIdIndexUnsigned];
        if (ecinstanceIdValueSnippets.size() != 1)
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Multi-value ECInstanceIds not supported.");
            return ECSqlStatus::InvalidECSql;
            }

        nativeSqlSnippets.m_pkValuesNativeSqlSnippets.insert(nativeSqlSnippets.m_pkValuesNativeSqlSnippets.end(), ecinstanceIdValueSnippets.begin(), ecinstanceIdValueSnippets.end());
        expIndexSkipList.push_back(foreignEndECInstanceIdIndexUnsigned);

        //usually ECInstanceId is auto-generated by ECSqlStatement. For end-table mappings this must not be done, as we have
        //the ECInstanceId already (as it is the same as this end's ECInstanceId).
        auto classId = relationshipClassMap.GetClass().GetId();
        auto parameterExp = exp.GetValuesExp()->TryGetAsParameterExpP (foreignEndECInstanceIdIndexUnsigned);
        if (parameterExp != nullptr)
            {
            ECSqlBinder* foreignEndECInstanceIdBinder = nullptr;
            ECSqlStatus stat = preparedStatement->GetParameterMapR ().TryGetBinder(foreignEndECInstanceIdBinder, parameterExp->GetParameterIndex());
            if (!stat.IsSuccess())
                {
                BeAssert(false && "Could not find this end constraint ECInstanceId parameter binder.");
                return stat;
                }

            BeAssert(foreignEndECInstanceIdBinder != nullptr);
            foreignEndECInstanceIdBinder->SetOnBindBriefcaseBasedIdEventHandler([preparedStatement, classId] (ECInstanceId const& bindValue)
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
            if (SUCCESS != ECInstanceId::FromString(id, ecinstanceidStr))
                {
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "'%s' is an invalid ECInstanceId value.", ecinstanceidStr);
                return ECSqlStatus::InvalidECSql;
                }

            BeAssert(id.IsValid());
            preparedStatement->SetECInstanceKeyInfo(ECSqlInsertPreparedStatement::ECInstanceKeyInfo(classId, id));
            }

        }

    if (foreignEndECClassIdIndex >= 0)
        {
        expIndexSkipList.push_back((size_t)foreignEndECClassIdIndex);

        //if this end was parametrized we must turn the respective binder into a no-op binder
        //so that clients binding values to it will not try to access a SQLite parameter which does not exist
        auto parameterExp = exp.GetValuesExp()->TryGetAsParameterExpP(foreignEndECClassIdIndex);
        if (parameterExp != nullptr)
            {
            ECSqlBinder* binder = nullptr;
            preparedStatement->GetParameterMapR().TryGetBinder(binder, parameterExp->GetParameterIndex());
            BeAssert(dynamic_cast<SystemPropertyECSqlBinder*> (binder) != nullptr);
            auto systemPropBinder = static_cast<SystemPropertyECSqlBinder*> (binder);
            systemPropBinder->SetIsNoop();
            }
        }

    if (referencedEndECClassIdIndex >= 0)
        {
        expIndexSkipList.push_back((size_t)referencedEndECClassIdIndex);
        //if this end was parametrized we must turn the respective binder into a no-op binder
        //so that clients binding values to it will not try to access a SQLite parameter which does not exist
        auto parameterExp = exp.GetValuesExp()->TryGetAsParameterExpP(referencedEndECClassIdIndex);
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

    auto referencedEndClassId = foreignEnd == ECRelationshipEnd_Source ? targetECClassId : sourceECClassId;
    if (referencedEndClassId.IsValid())
        {
        if (referencedEndECClassIdIndex > 0)
            {
            auto& r = nativeSqlSnippets.m_propertyNamesNativeSqlSnippets[referencedEndECClassIdIndex];
            if (!r.empty())
                {
                Utf8String column = r.front().ToString();
                if (column == "[ECClassId]") //Self case where source/target classid is in same table and user provided value for source/target classid
                    {
                    expIndexSkipList.push_back((size_t)referencedEndECClassIdIndex);
                    }
                }
            }

        ECSqlStatus stat = PrepareConstraintClassId(nativeSqlSnippets, ctx, *relationshipEndTableMap.GetReferencedEndECClassIdPropMap(), referencedEndClassId);
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
ClassMap const& classMap
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
        return ECSqlStatus::InvalidECSql;

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlInsertPreparer::PreparePrimaryKey(ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, ClassMap const& classMap)
    {
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
            if (!classMap.GetPropertyMaps().TryGetPropertyMap(ecInstanceIdPropMap, ECInstanceIdPropertyMap::PROPERTYACCESSSTRING))
                {
                BeAssert(false && "ECInstanceId property map is always expected to exist for domain classes.");
                return;
                }

            nativeSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(ecInstanceIdPropMap->ToNativeSql(nullptr, ECSqlType::Insert, false));
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
    DbColumn const* classIdColumn = classMap.GetJoinedTable().FindColumn(ECDB_COL_ECClassId);
    if (classIdColumn != nullptr)
        {
        NativeSqlBuilder::List classIdNameSqliteSnippets {NativeSqlBuilder(classIdColumn->GetName().c_str())};
        nativeSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(move(classIdNameSqliteSnippets));

        NativeSqlBuilder::List classIdSqliteSnippets {NativeSqlBuilder()};
        Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        if (auto joinedTableStatement = dynamic_cast<ParentOfJoinedTableECSqlStatement const*>(&ctx.GetECSqlStatementR()))
            joinedTableStatement->GetClassId().ToString(classIdStr);
        else
            classMap.GetClass().GetId().ToString(classIdStr);

        classIdSqliteSnippets[0].Append(classIdStr);
        nativeSqlSnippets.m_valuesNativeSqlSnippets.push_back(move(classIdSqliteSnippets));
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlInsertPreparer::ValidateConstraintClassId(ECClassId& retrievedConstraintClassId, ECSqlPrepareContext& ctx, 
                            InsertStatementExp const& exp, RelationshipClassMapCR relationshipClassMap,ECN::ECRelationshipEnd constraintEnd)
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
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid value %s for property %s. None of the respective constraint's ECClasses match that ECClassId.",
                                                     retrievedConstraintClassId.ToString().c_str(), constraintClassIdPropName.c_str());
            return ECSqlStatus::InvalidECSql;
            }

        return ECSqlStatus::Success;
        }
    //Sometime SourceECClassId/TargetECClassId  propertyMap is mapped to another table where ECClassId exist.
    //In this case if user did not specify it is not a error..
    if (!constraintClassIdPropMap->IsMappedToClassMapTables() || Enum::Contains(constraintClassIdPropMap->GetSingleColumn()->GetKind(), DbColumn::Kind::ECClassId))
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

        constraintClassId = ECClassId((uint64_t) constraintECClassIdConstantValueExp->GetValueAsInt64());
        isParameter = false;
        return ECSqlStatus::Success;
        }
    else if (expType == Exp::Type::Parameter)
        {
        constraintClassId.Invalidate();
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
ECSqlStatus ECSqlInsertPreparer::PrepareConstraintClassId(NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext& ctx, ECClassIdRelationshipConstraintPropertyMap const& constraintClassIdPropMap, ECClassId constraintClassId)
    {
    BeAssert(constraintClassId.IsValid());
    //if constraint class id maps to virtual column then ignore it as the column does not exist in the table.
    if (constraintClassIdPropMap.IsVirtual())
        return ECSqlStatus::Success;

    if (!constraintClassIdPropMap.IsMappedToClassMapTables() || Enum::Contains(constraintClassIdPropMap.GetSingleColumn()->GetKind(), DbColumn::Kind::ECClassId))
        return ECSqlStatus::Success;

    auto classIdColSqlSnippet = constraintClassIdPropMap.ToNativeSql(nullptr, ECSqlType::Insert, false);
    if (!classIdColSqlSnippet.empty())
        {
        insertNativeSqlSnippets.m_propertyNamesNativeSqlSnippets.push_back(move(classIdColSqlSnippet));

        Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        constraintClassId.ToString(classIdStr);
        NativeSqlBuilder classIdSnippet(classIdStr);
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
RelationshipClassEndTableMap const& classMap
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
    auto referencedEndECInstanceIdColumnSqlSnippets = classMap.GetReferencedEndECInstanceIdPropMap()->ToNativeSql(nullptr, ECSqlType::Update, false);
    for (auto const& referencedEndECInstanceIdColSnippet : referencedEndECInstanceIdColumnSqlSnippets)
        {
        updateBuilder.Append(" AND ").Append(referencedEndECInstanceIdColSnippet).Append(" IS NULL");
        }
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlInsertPreparer::ECInstanceIdMode ECSqlInsertPreparer::ValidateUserProvidedECInstanceId(int& ecinstanceIdExpIndex, ECSqlPrepareContext& ctx, InsertStatementExp const& exp, ClassMap const& classMap)
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
    const bool isEndTableRelationship = classMap.GetType() == ClassMap::Type::RelationshipEndTable;

    ECClassId classId = classMap.GetClass().GetId();
    //override ECClassId in case of join table with secondary class id 
    if (auto joinedTableStatement = dynamic_cast<ParentOfJoinedTableECSqlStatement const*>(&ctx.GetECSqlStatementR()))
        {
        classId = joinedTableStatement->GetClassId(); 
        }

    const Exp::Type expType = valueExp->GetType();
    if (expType == Exp::Type::LiteralValue)
        {
        if (!isEndTableRelationship)
            {
            LiteralValueExp const* constValueExp = static_cast<LiteralValueExp const*> (valueExp);
            ECInstanceId instanceId((uint64_t) constValueExp->GetValueAsInt64());
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
            ecinstanceidBinder->SetOnBindBriefcaseBasedIdEventHandler([preparedStatement] (ECInstanceId const& bindValue)
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
