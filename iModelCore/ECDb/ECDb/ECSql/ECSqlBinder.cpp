/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** ECSqlBinder **************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlBinder::ECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& nameGen, int mappedSqlParameterCount, bool hasToCallOnBeforeStep, bool hasToCallOnClearBindings)
    : m_preparedStatement(ctx.GetPreparedStatement()), m_typeInfo(typeInfo), m_hasToCallOnBeforeStep(hasToCallOnBeforeStep), m_hasToCallOnClearBindings(hasToCallOnClearBindings)
    {
    for (int i = 0; i < mappedSqlParameterCount; i++)
        {
        m_mappedSqlParameterNames.push_back(nameGen.GetNextName());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Statement& ECSqlBinder::GetSqliteStatement() const { return m_preparedStatement.GetSqliteStatement(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECDbCR ECSqlBinder::GetECDb() const { return m_preparedStatement.GetECDb(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlBinder::LogSqliteError(DbResult sqliteStat, Utf8CP errorMessageHeader) const
    {
    ECDbLogger::LogSqliteError(GetECDb(), sqliteStat, errorMessageHeader);
    return ECSqlStatus(sqliteStat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
Statement::MakeCopy ECSqlBinder::ToBeSQliteBindMakeCopy(IECSqlBinder::MakeCopy makeCopy)
    {
    switch (makeCopy)
        {
            case IECSqlBinder::MakeCopy::No:
                return Statement::MakeCopy::No;

            case IECSqlBinder::MakeCopy::Yes:
            default:
                return Statement::MakeCopy::Yes;
        }
    }


//****************** ECSqlBinderFactory **************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlPrepareContext& ctx, ParameterExp const& parameterExp)
    {
    ECSqlBinder::SqlParamNameGenerator paramNameGen(ctx, parameterExp.GetParameterName().c_str());

    ComputedExp const* targetExp = parameterExp.GetTargetExp();
    if (targetExp != nullptr && targetExp->GetType() == Exp::Type::PropertyName)
        {
        PropertyNameExp const& propNameExp = targetExp->GetAs<PropertyNameExp>();
        ECSqlSystemPropertyInfo const& sysPropInfo = propNameExp.GetSystemPropertyInfo();
        if (sysPropInfo.IsId() || propNameExp.GetTypeInfo().IsId())
            return CreateIdBinder(ctx, propNameExp.GetPropertyMap(), sysPropInfo, paramNameGen);
        }

    if (const Exp* exp = parameterExp.FindParent(Exp::Type::FunctionCall))
        {
        if (FunctionCallExp const* parentExp = exp->GetAsCP<FunctionCallExp>()) {
            if (parentExp->GetFunctionName().EqualsI("InVirtualSet") && parentExp->GetChildren()[0] == &parameterExp)
                return CreateVirtualSetBinder(ctx, parameterExp.GetTypeInfo(), paramNameGen);
            }
        }

    return CreateBinder(ctx, parameterExp.GetTypeInfo(), paramNameGen);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, ECSqlBinder::SqlParamNameGenerator& nameGen)
    {
    ECSqlTypeInfo::Kind typeKind = typeInfo.GetKind();
    BeAssert(typeKind != ECSqlTypeInfo::Kind::Unset);

    switch (typeKind)
        {
            case ECSqlTypeInfo::Kind::Primitive:
            {
            switch (typeInfo.GetPrimitiveType())
                {
                    case ECN::PRIMITIVETYPE_Binary:
                    case ECN::PRIMITIVETYPE_Boolean:
                    case ECN::PRIMITIVETYPE_DateTime:
                    case ECN::PRIMITIVETYPE_Double:
                    case ECN::PRIMITIVETYPE_IGeometry:
                    case ECN::PRIMITIVETYPE_Integer:
                    case ECN::PRIMITIVETYPE_Long:
                    case ECN::PRIMITIVETYPE_String:
                        return std::unique_ptr<ECSqlBinder>(new PrimitiveECSqlBinder(ctx, typeInfo, nameGen));

                    case ECN::PRIMITIVETYPE_Point2d:
                        return std::unique_ptr<ECSqlBinder>(new PointECSqlBinder(ctx, typeInfo, false, nameGen));

                    case ECN::PRIMITIVETYPE_Point3d:
                        return std::unique_ptr<ECSqlBinder>(new PointECSqlBinder(ctx, typeInfo, true, nameGen));

                    default:
                        BeAssert(false && "Could not create parameter mapping for the given parameter exp.");
                        return nullptr;
                }
            break;
            }
            //the rare case of expressions like this: NULL IS ?
            case ECSqlTypeInfo::Kind::Null:
                return std::unique_ptr<ECSqlBinder>(new PrimitiveECSqlBinder(ctx, typeInfo, nameGen));

            case ECSqlTypeInfo::Kind::Struct:
                return std::unique_ptr<ECSqlBinder>(new StructECSqlBinder(ctx, typeInfo, nameGen));

            case ECSqlTypeInfo::Kind::PrimitiveArray:
            case ECSqlTypeInfo::Kind::StructArray:
                return std::unique_ptr<ECSqlBinder>(new ArrayECSqlBinder(ctx, typeInfo, nameGen));
            case ECSqlTypeInfo::Kind::Navigation:
                return std::unique_ptr<ECSqlBinder>(new NavigationPropertyECSqlBinder(ctx, typeInfo, nameGen));

            default:
                BeAssert(false && "ECSqlBinderFactory::CreateBinder> Unhandled ECSqlTypeInfo::Kind value.");
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::unique_ptr<IdECSqlBinder> ECSqlBinderFactory::CreateIdBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlSystemPropertyInfo const& sysPropertyInfo, ECSqlBinder::SqlParamNameGenerator& paramNameGen)
    {
    BeAssert(sysPropertyInfo.IsId() || ECSqlTypeInfo(propMap).IsId());
    const bool isNoopBinder = RequiresNoopBinder(ctx, propMap, sysPropertyInfo);
    return std::unique_ptr<IdECSqlBinder>(new IdECSqlBinder(ctx, ECSqlTypeInfo(propMap), isNoopBinder, paramNameGen));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::unique_ptr<VirtualSetBinder> ECSqlBinderFactory::CreateVirtualSetBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, ECSqlBinder::SqlParamNameGenerator& paramNameGen)
    {
    ECSqlTypeInfo::Kind typeKind = typeInfo.GetKind();
    BeAssert(typeKind != ECSqlTypeInfo::Kind::Unset);
    return std::unique_ptr<VirtualSetBinder>(new VirtualSetBinder(ctx, typeInfo, paramNameGen));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlBinderFactory::RequiresNoopBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlSystemPropertyInfo const& sysPropertyInfo)
    {
    const ECSqlType ecsqlType = ctx.GetCurrentScope().GetECSqlType();
    if (ecsqlType == ECSqlType::Select || ecsqlType == ECSqlType::Delete ||
        (ecsqlType == ECSqlType::Update && ctx.GetCurrentScope().GetExp().GetType() != Exp::Type::AssignmentList))
        return false;


    //only INSERT and UPDATE SET clauses require no-op binders because they directly translate to columns. All other expressions
    //can use constant values for virtual columns and therefore don't need no-op binders.

    BeAssert((sysPropertyInfo.GetType() != ECSqlSystemPropertyInfo::Type::Class || sysPropertyInfo.GetClass() != ECSqlSystemPropertyInfo::Class::ECClassId) && "Inserting/updating ECClassId is not supported and should have been caught before");
    BeAssert(propMap.GetType() != PropertyMap::Type::ECClassId && "Inserting/updating ECClassId is not supported and should have been caught before");
    BeAssert(propMap.GetClassMap().GetType() != ClassMap::Type::RelationshipEndTable);

    ClassMap const& classMap = propMap.GetClassMap();
    switch (propMap.GetType())
        {
            case PropertyMap::Type::ConstraintECClassId:
            {
            ConstraintECClassIdPropertyMap const& constraintClassIdPropMap = propMap.GetAs<ConstraintECClassIdPropertyMap>();
            if (nullptr != ConstraintECClassIdJoinInfo::RequiresJoinTo(constraintClassIdPropMap, true /*ignoreVirtualColumnCheck*/))
                return true;

            BeAssert(classMap.GetTables().size() == 1 && constraintClassIdPropMap.GetTables().size() == 1);
            DbTable const& contextTable = classMap.GetJoinedOrPrimaryTable();
            if (contextTable.GetType() == DbTable::Type::Virtual)
                return true; //if table is virtual this is a noop.

            ConstraintECClassIdPropertyMap::PerTableIdPropertyMap const* contextConstraintClassIdPropMap = constraintClassIdPropMap.FindDataPropertyMap(contextTable);
            BeAssert(contextConstraintClassIdPropMap != nullptr);
            return contextConstraintClassIdPropMap->GetColumn().GetPersistenceType() == PersistenceType::Virtual;
            }
            case PropertyMap::Type::NavigationRelECClassId:
            {
            DbColumn const& col = propMap.GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>().GetColumn();
            if (col.GetTable().GetType() == DbTable::Type::Virtual)
                return true;

            return col.GetPersistenceType() == PersistenceType::Virtual;
            }
            default:
                return false;
        }
    }



//****************** ECSqlParameterMap **************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlParameterMap::Contains(int& ecsqlParameterIndex, Utf8StringCR ecsqlParameterName) const
    {
    ecsqlParameterIndex = GetIndexForName(ecsqlParameterName);
    return ecsqlParameterIndex > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlParameterMap::TryGetBinder(ECSqlBinder*& binder, Utf8StringCR ecsqlParameterName) const
    {
    int ecsqlParameterIndex = -1;
    if (!Contains(ecsqlParameterIndex, ecsqlParameterName))
        return false;

    BeAssert(ecsqlParameterIndex > 0);
    return TryGetBinder(binder, ecsqlParameterIndex) == ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::TryGetBinder(ECSqlBinder*& binder, int ecsqlParameterIndex) const
    {
    if (ecsqlParameterIndex <= 0 || ecsqlParameterIndex > (int) (m_binders.size()))
        return ECSqlStatus::Error;

    //parameter indices are 1-based, but stored in a 0-based vector.
    binder = m_binders[(size_t) (ecsqlParameterIndex - 1)];
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECSqlParameterMap::GetIndexForName(Utf8StringCR ecsqlParameterName) const
    {
    auto it = m_nameToIndexMapping.find(ecsqlParameterName);
    if (it != m_nameToIndexMapping.end())
        return it->second;
    else
        return -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlBinder* ECSqlParameterMap::AddBinder(ECSqlPrepareContext& ctx, ParameterExp const& parameterExp)
    {
    int ecsqlParameterIndex = 0;
    //unnamed parameters don't have an identity, therefore always add a new binder in that case
    if (parameterExp.IsNamedParameter() && Contains(ecsqlParameterIndex, parameterExp.GetParameterName()))
        {
        BeAssert(false && "ECSqlParameterMap::AddBinder: mapping already exists");
        return nullptr;
        }

    std::unique_ptr<ECSqlBinder> binder = ECSqlBinderFactory::CreateBinder(ctx, parameterExp);
    if (binder == nullptr)
        return nullptr;

    ECSqlBinder* binderP = binder.get(); //cache raw pointer as return value as the unique_ptr will be moved into the list
    m_ownedBinders.push_back(std::move(binder));
    m_binders.push_back(binderP);

    BeAssert(((int) m_binders.size()) == parameterExp.GetParameterIndex()); //Parameter indices are 1-based

    if (binderP->HasToCallOnBeforeStep())
        m_bindersToCallOnStep.push_back(binderP);

    if (binderP->HasToCallOnClearBindings())
        m_bindersToCallOnClearBindings.push_back(binderP);

    //insert name to index mapping. 
    if (parameterExp.IsNamedParameter())
        m_nameToIndexMapping[parameterExp.GetParameterName()] = parameterExp.GetParameterIndex();

    return binderP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::OnBeforeStep()
    {
    for (ECSqlBinder* binder : m_bindersToCallOnStep)
        {
        ECSqlStatus stat = binder->OnBeforeStep();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlParameterMap::OnClearBindings()
    {
    for (ECSqlBinder* binder : m_bindersToCallOnClearBindings)
        {
        binder->OnClearBindings();
        }
    }

//****************** ArrayConstraintValidator **************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ArrayConstraintValidator::Validate(ECDbCR ecdb, ECSqlTypeInfo const& expected, uint32_t actualArrayLength)
    {
    if (expected.GetArrayMinOccurs() != nullptr && actualArrayLength < expected.GetArrayMinOccurs().Value())
        {
        if (expected.GetPropertyMap() == nullptr)
            LOG.errorv("Array to be bound to the array parameter must at least have %" PRIu32 " element(s) as defined in the respective ECProperty.", expected.GetArrayMinOccurs().Value());
        else
            {
            ECN::ECPropertyCR prop = expected.GetPropertyMap()->GetProperty();
            LOG.errorv("Array to be bound to the array parameter must at least have %" PRIu32 " element(s) as defined in ECProperty '%s.%s'.",
                       expected.GetArrayMinOccurs().Value(), prop.GetClass().GetFullName(), prop.GetName().c_str());
            }

        return ECSqlStatus::Error;
        }

    return ValidateMaximum(ecdb, expected, actualArrayLength);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ArrayConstraintValidator::ValidateMaximum(ECDbCR ecdb, ECSqlTypeInfo const& expected, uint32_t actualArrayLength)
    {
    if (expected.GetArrayMaxOccurs() != nullptr && actualArrayLength > expected.GetArrayMaxOccurs().Value())
        {
        if (expected.GetPropertyMap() == nullptr)
            LOG.errorv("Array to be bound to the array parameter must at most have %" PRIu32 " element(s) as defined in the respective ECProperty.", expected.GetArrayMaxOccurs().Value());
        else
            {
            ECN::ECPropertyCR prop = expected.GetPropertyMap()->GetProperty();
            LOG.errorv("Array to be bound to the array parameter must at most have %" PRIu32 " element(s) as defined in ECProperty '%s.%s'.",
                       expected.GetArrayMaxOccurs().Value(), prop.GetClass().GetFullName(), prop.GetName().c_str());
            }

        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
