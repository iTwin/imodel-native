/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECObjects/ECExpressionNode.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbExpressionSymbolContext::ECDbExpressionSymbolContext(ECDbCR ecdb, ECSqlStatementCache const* statementCache)
    {
    if (nullptr != statementCache)
        {
        m_statementCache = statementCache;
        m_ownsStatementCache = false;
        }
    else
        {
        m_statementCache = new ECSqlStatementCache(4, "ECDbExpressionSymbolContext");
        m_ownsStatementCache = true;
        }
    m_provider = new ECDbExpressionSymbolProvider(ecdb, *m_statementCache);
    InternalECSymbolProviderManager::GetManager().RegisterSymbolProvider(*m_provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbExpressionSymbolContext::~ECDbExpressionSymbolContext()
    {
    LeaveContext();
    if (m_ownsStatementCache)
        DELETE_AND_CLEAR(m_statementCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbExpressionSymbolContext::LeaveContext()
    {
    if (nullptr == m_provider)
        return;

    InternalECSymbolProviderManager::GetManager().UnregisterSymbolProvider(*m_provider);

    if (nullptr != m_provider)
        {
        delete m_provider;
        m_provider = nullptr;
        }
    }

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionContext final : ECN::SymbolExpressionContext
{
private:
    ECDbCR m_db;

    explicit ECDbExpressionContext(ECDbCR db) : SymbolExpressionContext(nullptr), m_db(db)
        {
        AddSymbol(*PropertySymbol::Create<ECDbExpressionContext, Utf8CP>("Path", *this, &ECDbExpressionContext::GetPath));
        AddSymbol(*PropertySymbol::Create<ECDbExpressionContext, ECValue>("Name", *this, &ECDbExpressionContext::GetName));
        }

    Utf8CP GetPath() const {return m_db.GetDbFileName();}
    ECN::ECValue GetName() const {return ECN::ECValue(BeFileName(m_db.GetDbFileName()).GetFileNameWithoutExtension().c_str());}

public:
    static RefCountedPtr<ECDbExpressionContext> Create(ECDbCR db) {return new ECDbExpressionContext(db);}
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolProvider::ECDbExpressionEvaluationContext
{
private:
    ECDbCR m_db;
    ECSqlStatementCache const& m_statementCache;
public:
    ECDbExpressionEvaluationContext(ECDbCR db, ECSqlStatementCache const& statementCache)
        : m_db(db), m_statementCache(statementCache)
        {}
    ECDbCR GetECDb() const { return m_db; }
    ECSqlStatementCache const& GetStatementCache() const { return m_statementCache; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbExpressionSymbolProvider::ECDbExpressionSymbolProvider(ECDbCR db, ECSqlStatementCache const& statementCache)
    : ECN::IECSymbolProvider(), m_context(new ECDbExpressionEvaluationContext(db, statementCache))
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbExpressionSymbolProvider::~ECDbExpressionSymbolProvider()
    {
    DELETE_AND_CLEAR(m_context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbExpressionSymbolProvider::_PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const
    {
    context.AddSymbol(*ContextSymbol::CreateContextSymbol("ECDb", *ECDbExpressionContext::Create(m_context->GetECDb())));
    context.AddSymbol(*MethodSymbol::Create("GetECClassId", &GetClassId, nullptr, m_context));
    context.AddSymbol(*MethodSymbol::Create("GetRelatedInstancesCount", nullptr, &GetRelatedInstancesCount, m_context));
    context.AddSymbol(*MethodSymbol::Create("GetRelatedInstance", nullptr, &GetRelatedInstance, m_context));
    context.AddSymbol(*MethodSymbol::Create("HasRelatedInstance", nullptr, &HasRelatedInstance, m_context));
    context.AddSymbol(*MethodSymbol::Create("GetRelatedValue", nullptr, &GetRelatedValue, m_context));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetClassId(EvaluationResult& evalResult, void* contextP, EvaluationResultVector& args)
    {
    if (2 != args.size())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::GetClassId: WrongNumberOfArguments. Expected 2, actually %" PRIu64, (uint64_t)args.size()).c_str());
        return ExpressionStatus::WrongNumberOfArguments;
        }

    if (!args[0].IsECValue() || !args[0].GetECValue()->IsString()
        || !args[1].IsECValue() || !args[1].GetECValue()->IsString())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::GetClassId: WrongType. Invalid arguments");
        return ExpressionStatus::WrongType;
        }

    ECDbExpressionEvaluationContext const& context = *reinterpret_cast<ECDbExpressionEvaluationContext const*>(contextP);

    Utf8CP className = args[0].GetECValue()->GetUtf8CP();
    Utf8CP schemaName = args[1].GetECValue()->GetUtf8CP();
    ECClassId classId = context.GetECDb().Schemas().GetClassId(schemaName, className);
    evalResult.InitECValue().SetLong(classId.GetValueUnchecked());
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ECDbExpressionSymbolProvider::GetClassId: Result: %s", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatOld(Utf8StringR query, ECEntityClassCP& relatedClass, ECDbCR db, ECInstanceListCR instanceData, EvaluationResult const& arg)
    {
    if (!arg.IsECValue() || !arg.GetECValue()->IsUtf8())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatOld: WrongType. Argument is not ECValue or argument value is not a string");
        return ExpressionStatus::WrongType;
        }

    if (instanceData.empty())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatOld: WrongType. ECInstanceList is empty");
        return ExpressionStatus::WrongType;
        }

    bvector<Utf8String> argTokens;
    BeStringUtilities::Split(arg.GetECValue()->GetUtf8CP(), ":", nullptr, argTokens);
    if (argTokens.size() != 3 || argTokens[1].length() != 1)
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatOld: UnknownError. Invalid argument format");
        return ExpressionStatus::UnknownError;
        }

    Utf8CP thisInstanceIdColumnName,thisClassIdColumnName,
        relatedInstanceIdColumnName, relatedClassIdColumnName;
    switch (argTokens[1][0])
        {
        case '0':
            thisInstanceIdColumnName = ECDBSYS_PROP_SourceECInstanceId;
            thisClassIdColumnName = ECDBSYS_PROP_SourceECClassId;
            relatedInstanceIdColumnName = ECDBSYS_PROP_TargetECInstanceId;
            relatedClassIdColumnName = ECDBSYS_PROP_TargetECClassId;
            break;
        case '1':
            thisInstanceIdColumnName = ECDBSYS_PROP_TargetECInstanceId;
            thisClassIdColumnName = ECDBSYS_PROP_TargetECClassId;
            relatedInstanceIdColumnName = ECDBSYS_PROP_SourceECInstanceId;
            relatedClassIdColumnName = ECDBSYS_PROP_SourceECClassId;
            break;
        default:
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatOld: UnknownError. Invalid relationship direction");
            return ExpressionStatus::UnknownError;
        }

    Utf8CP relationshipName = argTokens[0].c_str();
    Utf8CP relatedName      = argTokens[2].c_str();

    ECRelationshipClassCP relationshipClass = nullptr;
    relatedClass = nullptr;
    if (SUCCESS != FindRelationshipAndClassInfo(db, relationshipClass, relationshipName, relatedClass, relatedName))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatOld: UnknownError. Could not find relationship and class info (%s, %s)", relationshipName, relatedName).c_str());
        return ExpressionStatus::UnknownError;
        }

    if (nullptr == relationshipClass || nullptr == relatedClass)
        {
        BeAssert(false);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatOld: UnknownError. Could not find relationship or class");
        return ExpressionStatus::UnknownError;
        }

    static Utf8CP selectQueryFormat = "SELECT related.%%s FROM %%s this, %s relationship, %s related "
        "WHERE this." ECDBSYS_PROP_ECInstanceId "=? AND "
        "      this." ECDBSYS_PROP_ECInstanceId "=relationship.[%s] AND this." ECDBSYS_PROP_ECClassId "=relationship.[%s] AND "
        "      related." ECDBSYS_PROP_ECInstanceId "=relationship.[%s] AND related." ECDBSYS_PROP_ECClassId "=relationship.[%s]";

    query = Utf8PrintfString(selectQueryFormat,
                relationshipClass->GetECSqlName().c_str(), relatedClass->GetECSqlName().c_str(),
                thisInstanceIdColumnName, thisClassIdColumnName, relatedInstanceIdColumnName, relatedClassIdColumnName);
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus CreateRelatedInstanceQueryFrom3Args(Utf8StringR query, EvaluationResultVector const& args)
    {
    for (size_t i = 0; i < 3; ++i)
        {
        if (!args[i].IsECValue() || !args[i].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::CreateRelatedInstanceQueryFrom3Args: WrongType. Invalid argument type (expecting string)");
            return ExpressionStatus::WrongType;
            }
        }

    Utf8CP direction = args[1].GetECValue()->GetUtf8CP();
    Utf8CP thisInstanceIdColumnName, thisClassIdColumnName,
        relatedInstanceIdColumnName, relatedClassIdColumnName;
    if (0 == BeStringUtilities::Stricmp("Forward", direction))
        {
        thisInstanceIdColumnName = ECDBSYS_PROP_SourceECInstanceId;
        thisClassIdColumnName = ECDBSYS_PROP_SourceECClassId;
        relatedInstanceIdColumnName = ECDBSYS_PROP_TargetECInstanceId;
        relatedClassIdColumnName = ECDBSYS_PROP_TargetECClassId;
        }
    else if (0 == BeStringUtilities::Stricmp("Backward", direction))
        {
        thisInstanceIdColumnName = ECDBSYS_PROP_TargetECInstanceId;
        thisClassIdColumnName = ECDBSYS_PROP_TargetECClassId;
        relatedInstanceIdColumnName = ECDBSYS_PROP_SourceECInstanceId;
        relatedClassIdColumnName = ECDBSYS_PROP_SourceECClassId;
        }
    else
        {
        BeAssert(false);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::CreateRelatedInstanceQueryFrom3Args: UnknownError. Invalid direction (%s)", direction).c_str());
        return ExpressionStatus::UnknownError;
        }

    Utf8String relationshipSchemaName, relationshipClassName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(relationshipSchemaName, relationshipClassName, args[0].GetECValue()->GetUtf8CP()))
        {
        BeAssert(false);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::CreateRelatedInstanceQueryFrom3Args: UnknownError. Could not parse relationship name: %s", args[0].GetECValue()->GetUtf8CP()).c_str());
        return ExpressionStatus::UnknownError;
        }

    Utf8String relatedClassSchemaName, relatedClassName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(relatedClassSchemaName, relatedClassName, args[2].GetECValue()->GetUtf8CP()))
        {
        BeAssert(false);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::CreateRelatedInstanceQueryFrom3Args: UnknownError. Could not parse class name: %s", args[2].GetECValue()->GetUtf8CP()).c_str());
        return ExpressionStatus::UnknownError;
        }

    query = Utf8PrintfString("SELECT related.%%s FROM %%s this, [%s].[%s] relationship, [%s].[%s] related "
        "WHERE this." ECDBSYS_PROP_ECInstanceId "=? AND "
        "      this." ECDBSYS_PROP_ECInstanceId "=relationship.[%s] AND this." ECDBSYS_PROP_ECClassId "=relationship.[%s] AND "
        "      related." ECDBSYS_PROP_ECInstanceId "=relationship.[%s] AND related." ECDBSYS_PROP_ECClassId "=relationship.[%s]",
        relationshipSchemaName.c_str(), relationshipClassName.c_str(),
        relatedClassSchemaName.c_str(), relatedClassName.c_str(),
        thisInstanceIdColumnName, thisClassIdColumnName, relatedInstanceIdColumnName, relatedClassIdColumnName);
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus CreateRelatedInstanceQueryFrom2Args(Utf8StringR query, EvaluationResultVector const& args)
    {
    if (!args[0].IsECValue() || !args[0].GetECValue()->IsString())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::CreateRelatedInstanceQueryFrom2Args: WrongType. Invalid argument 0 type (expecting string)");
        return ExpressionStatus::WrongType;
        }

    if (!args[1].IsLambda())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::CreateRelatedInstanceQueryFrom2Args: WrongType. Invalid argument 1 type (expecting lambda)");
        return ExpressionStatus::WrongType;
        }

    Utf8String relatedClassSchemaName, relatedClassName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(relatedClassSchemaName, relatedClassName, args[0].GetECValue()->GetUtf8CP()))
        {
        BeAssert(false);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::CreateRelatedInstanceQueryFrom2Args: UnknownError. Could not parse class name: %s", args[0].GetECValue()->GetUtf8CP()).c_str());
        return ExpressionStatus::UnknownError;
        }

    LambdaNodeCR lambda = args[1].GetLambda()->GetNode();
    query.append("SELECT [").append(lambda.GetSymbolName()).append("].[%s] ");
    query.append("FROM %s AS this ");
    query.append("JOIN [").append(relatedClassSchemaName).append("].[").append(relatedClassName).append("] AS [").append(lambda.GetSymbolName()).append("] ");
    query.append("ON ").append(lambda.GetExpression().ToExpressionString()).append(" ");
    query.append("WHERE this." ECDBSYS_PROP_ECInstanceId " = ?");

    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionStatus GetRelatedInstanceQueryFormatNew(Utf8StringR query, EvaluationResultVector const& args, uint32_t additionalArgsCount)
    {
    if (3 == (args.size() - additionalArgsCount))
        return CreateRelatedInstanceQueryFrom3Args(query, args);

    if (2 == (args.size() - additionalArgsCount))
        return CreateRelatedInstanceQueryFrom2Args(query, args);

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatNew: WrongNumberOfArguments. Expected 2 or 3, actually %" PRIu64, (uint64_t)args.size()).c_str());
    return ExpressionStatus::WrongNumberOfArguments;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::HasRelatedInstance(EvaluationResult& evalResult, void* contextP, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (3 != args.size() && 2 != args.size())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::HasRelatedInstance: WrongNumberOfArguments. Expected 2 or 3, actually %" PRIu64, (uint64_t)args.size()).c_str());
        return ExpressionStatus::WrongNumberOfArguments;
        }

    ECDbExpressionEvaluationContext const& context = *reinterpret_cast<ECDbExpressionEvaluationContext const*>(contextP);

    Utf8String queryFormat;
    ExpressionStatus stat = GetRelatedInstanceQueryFormatNew(queryFormat, args, 0);
    if (ExpressionStatus::Success != stat)
        return stat;

    for (IECInstancePtr const& instance : instanceData)
        {
        Utf8PrintfString query(queryFormat.c_str(), ECDBSYS_PROP_ECInstanceId, instance->GetClass().GetECSqlName().c_str());

        CachedECSqlStatementPtr stmt = context.GetStatementCache().GetPreparedStatement(context.GetECDb(), query.c_str());
        if (stmt.IsNull())
            {
            BeAssert(false);
            continue;
            }

        ECInstanceId id;
        ECInstanceId::FromString(id, instance->GetInstanceId().c_str());
        ECSqlStatus status = stmt->BindId(1, id);
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        if (DbResult::BE_SQLITE_ROW == stmt->Step())
            {
            evalResult.InitECValue().SetBoolean(true);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ECDbExpressionSymbolProvider::HasRelatedInstance: Result: %s", evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }
        }

    evalResult.InitECValue().SetBoolean(false);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ECDbExpressionSymbolProvider::HasRelatedInstance: Result: %s", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedInstance(EvaluationResult& evalResult, void* contextP, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (1 != args.size())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedInstance: WrongNumberOfArguments. Expected 1, actually %" PRIu64, (uint64_t)args.size()).c_str());
        return ExpressionStatus::WrongNumberOfArguments;
        }

    ECDbExpressionEvaluationContext const& context = *reinterpret_cast<ECDbExpressionEvaluationContext const*>(contextP);

    Utf8String queryFormat;
    ECEntityClassCP relatedClass;
    ExpressionStatus stat = GetRelatedInstanceQueryFormatOld(queryFormat, relatedClass, context.GetECDb(), instanceData, args[0]);
    if (ExpressionStatus::Success != stat)
        return stat;

    IECInstancePtr relatedInstance;
    for (IECInstancePtr const& instance : instanceData)
        {
        Utf8PrintfString query(queryFormat.c_str(), "*", instance->GetClass().GetECSqlName().c_str());

        CachedECSqlStatementPtr stmt = context.GetStatementCache().GetPreparedStatement(context.GetECDb(), query.c_str());
        if (stmt.IsNull())
            {
            BeAssert(false);
            continue;
            }

        ECInstanceId id;
        ECInstanceId::FromString(id, instance->GetInstanceId().c_str());
        ECSqlStatus status = stmt->BindId(1, id);
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        ECInstanceECSqlSelectAdapter adapter(*stmt);
        if (DbResult::BE_SQLITE_ROW == stmt->Step())
            {
            relatedInstance = adapter.GetInstance();
            break;
            }
        }

    if (relatedInstance.IsValid())
        evalResult.SetInstance (*relatedInstance);
    else
        evalResult.InitECValue().SetToNull();

    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedInstance: Result: %s", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedInstancesCount(EvaluationResult& evalResult, void* contextP, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (3 != args.size() && 2 != args.size())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedInstancesCount: WrongNumberOfArguments. Expected 2 or 3, actually %" PRIu64, (uint64_t)args.size()).c_str());
        return ExpressionStatus::WrongNumberOfArguments;
        }

    ECDbExpressionEvaluationContext const& context = *reinterpret_cast<ECDbExpressionEvaluationContext const*>(contextP);

    Utf8String queryFormat;
    ExpressionStatus stat = GetRelatedInstanceQueryFormatNew(queryFormat, args, 0);
    if (ExpressionStatus::Success != stat)
        return stat;

    uint64_t count = 0;
    for (IECInstancePtr const& instance : instanceData)
        {
        Utf8PrintfString nestedQuery(queryFormat.c_str(), ECDBSYS_PROP_ECInstanceId, instance->GetClass().GetECSqlName().c_str());
        Utf8PrintfString query("SELECT COUNT(1) FROM (%s)", nestedQuery.c_str());

        CachedECSqlStatementPtr stmt = context.GetStatementCache().GetPreparedStatement(context.GetECDb(), query.c_str());
        if (stmt.IsNull())
            {
            BeAssert(false);
            continue;
            }

        ECInstanceId id;
        ECInstanceId::FromString(id, instance->GetInstanceId().c_str());
        ECSqlStatus status = stmt->BindId(1, id);
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        if (DbResult::BE_SQLITE_ROW == stmt->Step())
            count += stmt->GetValueUInt64(0);
        }

    evalResult.InitECValue().SetLong(count);
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedInstancesCount: Result: %s", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECValue GetECValueFromSqlValue(IECSqlValue const& sqlValue)
    {
    if (!sqlValue.GetColumnInfo().GetDataType().IsPrimitive())
        {
        BeAssert(false);
        return ECValue();
        }

    switch (sqlValue.GetColumnInfo().GetDataType().GetPrimitiveType())
        {
        case PrimitiveType::PRIMITIVETYPE_Boolean: return ECValue(sqlValue.GetBoolean());
        case PrimitiveType::PRIMITIVETYPE_DateTime: return ECValue(sqlValue.GetDateTime());
        case PrimitiveType::PRIMITIVETYPE_Double: return ECValue(sqlValue.GetDouble());
        case PrimitiveType::PRIMITIVETYPE_Integer: return ECValue(sqlValue.GetInt());
        case PrimitiveType::PRIMITIVETYPE_Long: return ECValue(sqlValue.GetInt64());
        case PrimitiveType::PRIMITIVETYPE_Point2d: return ECValue(sqlValue.GetPoint2d());
        case PrimitiveType::PRIMITIVETYPE_Point3d: return ECValue(sqlValue.GetPoint3d());
        case PrimitiveType::PRIMITIVETYPE_String: return ECValue(sqlValue.GetText());
        case PrimitiveType::PRIMITIVETYPE_Binary: return ECValue();
        case PrimitiveType::PRIMITIVETYPE_IGeometry: return ECValue();
        }

    BeAssert(false);
    return ECValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedValue(EvaluationResult& evalResult, void* contextP, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (4 != args.size() && 3 != args.size())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedValue: WrongNumberOfArguments. Expected 3 or 4, actually %" PRIu64, (uint64_t)args.size()).c_str());
        return ExpressionStatus::WrongNumberOfArguments;
        }

    ECDbExpressionEvaluationContext const& context = *reinterpret_cast<ECDbExpressionEvaluationContext const*>(contextP);

    Utf8String queryFormat;
    ExpressionStatus stat = GetRelatedInstanceQueryFormatNew(queryFormat, args, 1);
    if (ExpressionStatus::Success != stat)
        return stat;

    EvaluationResult const& propertyNameArg = args[args.size() - 1];
    if (!propertyNameArg.IsECValue() || !propertyNameArg.GetECValue()->IsUtf8())
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECDbExpressionSymbolProvider::GetRelatedValue: WrongType. Property name is invalid");
        return ExpressionStatus::WrongType;
        }

    IECInstancePtr relatedInstance;
    for (IECInstancePtr const& instance : instanceData)
        {
        Utf8PrintfString query(queryFormat.c_str(), propertyNameArg.GetECValue()->GetUtf8CP(), instance->GetClass().GetECSqlName().c_str());

        CachedECSqlStatementPtr stmt = context.GetStatementCache().GetPreparedStatement(context.GetECDb(), query.c_str());
        if (stmt.IsNull())
            {
            BeAssert(false);
            continue;
            }

        ECInstanceId id;
        ECInstanceId::FromString(id, instance->GetInstanceId().c_str());
        ECSqlStatus status = stmt->BindId(1, id);
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        if (DbResult::BE_SQLITE_ROW == stmt->Step())
            {
            evalResult.InitECValue() = GetECValueFromSqlValue(stmt->GetValue(0));
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedValue: Result: %s", evalResult.ToString().c_str()).c_str());
            return ExpressionStatus::Success;
            }
        }

    evalResult.InitECValue().SetToNull();
    ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ECDbExpressionSymbolProvider::GetRelatedValue: Result: %s", evalResult.ToString().c_str()).c_str());
    return ExpressionStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static ECEntityClassCP GetEntityClassFromSameSchema(ECClassCR other, Utf8CP name)
    {
    ECClassCP classCP = other.GetSchema().GetClassCP(name);
    return (nullptr != classCP) ? classCP->GetEntityClassCP() : nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static ECRelationshipClassCP GetRelationshipClassFromSameSchema(ECClassCR other, Utf8CP name)
    {
    ECClassCP classCP = other.GetSchema().GetClassCP(name);
    return (nullptr != classCP) ? classCP->GetRelationshipClassCP() : nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbExpressionSymbolProvider::FindRelationshipAndClassInfo(ECDbCR db, ECRelationshipClassCP& relationship, Utf8CP relationshipName, ECEntityClassCP& entityClass, Utf8CP className)
    {
    // already have both - immediate return
    if (nullptr != relationship && nullptr != entityClass)
        return SUCCESS;

    // high chances to find related class in relationship's schema
    if (nullptr != relationship && nullptr != (entityClass = GetEntityClassFromSameSchema(*relationship, className)))
        return SUCCESS;

    // high chances to find relationship class in related class' schema
    if (nullptr != entityClass && nullptr != (relationship = GetRelationshipClassFromSameSchema(*entityClass, relationshipName)))
        return SUCCESS;

    Utf8String sql("SELECT Id FROM main.ec_Class WHERE Name=?");
    CachedStatementPtr statement = db.GetImpl().GetCachedSqliteStatement(sql.c_str());
    BeAssert(statement.IsValid());
    if (nullptr == relationship)
        {
        statement->BindText(1, relationshipName, BeSQLite::Statement::MakeCopy::No);

        if (BE_SQLITE_ROW != statement->Step())
            return ERROR;

        ECClassCP candidateRelationshipClass = db.Schemas().GetClass(statement->GetValueId<ECClassId>(0));
        if (nullptr == candidateRelationshipClass)
            return ERROR;
        relationship = candidateRelationshipClass->GetRelationshipClassCP();
        }

    if (nullptr == entityClass)
        {
        statement->ClearBindings();
        statement->Reset();
        statement->BindText(1, className, BeSQLite::Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != statement->Step())
            return ERROR;

        ECClassCP candidateEntityClass = db.Schemas().GetClass(statement->GetValueId<ECClassId>(0));
        if (nullptr == candidateEntityClass)
            return ERROR;
        entityClass = candidateEntityClass->GetEntityClassCP();
        }
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
