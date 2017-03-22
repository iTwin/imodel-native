/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbExpressionSymbolProvider.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbExpressionSymbolProvider.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                    Grigas.Petraitis                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECDbExpressionSymbolContext::ECDbExpressionSymbolContext(ECDbCR ecdb)
    {
    m_provider = new ECDbExpressionSymbolProvider(ecdb);
    InternalECSymbolProviderManager::GetManager().RegisterSymbolProvider(*m_provider);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Grigas.Petraitis                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
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
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionContext final : ECN::SymbolExpressionContext
{
private:
    ECDbCR m_db;

    ECDbExpressionContext(ECDbCR db) : SymbolExpressionContext(nullptr), m_db(db)
        {
        AddSymbol(*PropertySymbol::Create<ECDbExpressionContext, Utf8CP>("Path", *this, &ECDbExpressionContext::GetPath));
        AddSymbol(*PropertySymbol::Create<ECDbExpressionContext, ECValue>("Name", *this, &ECDbExpressionContext::GetName));
        }

    Utf8CP GetPath() const {return m_db.GetDbFileName();}
    ECN::ECValue GetName() const {return ECN::ECValue(BeFileName(m_db.GetDbFileName()).GetFileNameWithoutExtension().c_str());}

public:
    static RefCountedPtr<ECDbExpressionContext> Create(ECDbCR db) {return new ECDbExpressionContext(db);}
};

//--------------------------------------------------------------------------------------
// @bsimethod                                    Grigas.Petraitis                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbExpressionSymbolProvider::_PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const
    {
    context.AddSymbol(*ContextSymbol::CreateContextSymbol("ECDb", *ECDbExpressionContext::Create(m_db)));
    context.AddSymbol(*MethodSymbol::Create("GetECClassId", &GetClassId, nullptr, const_cast<ECDb*>(&m_db)));
    context.AddSymbol(*MethodSymbol::Create("GetRelatedInstance", nullptr, &GetRelatedInstance, const_cast<ECDb*>(&m_db)));
    context.AddSymbol(*MethodSymbol::Create("HasRelatedInstance", nullptr, &HasRelatedInstance, const_cast<ECDb*>(&m_db)));
    context.AddSymbol(*MethodSymbol::Create("GetRelatedValue", nullptr, &GetRelatedValue, const_cast<ECDb*>(&m_db)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetClassId(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
    {
    if (2 != args.size())
        return ExpressionStatus::WrongNumberOfArguments;

    if (!args[0].IsECValue() || !args[0].GetECValue()->IsString()
        || !args[1].IsECValue() || !args[1].GetECValue()->IsString())
        return ExpressionStatus::WrongType;

    Utf8CP className = args[0].GetECValue()->GetUtf8CP();
    Utf8CP schemaName = args[1].GetECValue()->GetUtf8CP();
    ECDbCR db = *reinterpret_cast<ECDb const*>(context);
    ECClassId classId = db.Schemas().GetClassId(schemaName, className);
    evalResult.InitECValue().SetLong(classId.GetValueUnchecked());
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatOld(Utf8StringR query, ECEntityClassCP& relatedClass, ECDbCR db, ECInstanceListCR instanceData, EvaluationResult const& arg)
    {
    if (!arg.IsECValue() || !arg.GetECValue()->IsUtf8())
        return ExpressionStatus::WrongType;

    if (instanceData.empty())
        return ExpressionStatus::WrongType;
    
    bvector<Utf8String> argTokens;
    BeStringUtilities::Split(arg.GetECValue()->GetUtf8CP(), ":", nullptr, argTokens);
    if (argTokens.size() != 3 || argTokens[1].length() != 1)
        return ExpressionStatus::UnknownError;

    Utf8CP thisInstanceIdColumnName,thisClassIdColumnName,
        relatedInstanceIdColumnName, relatedClassIdColumnName;
    ECRelatedInstanceDirection direction;
    switch (argTokens[1][0])
        {
        case '0':
            direction = ECRelatedInstanceDirection::Forward; 
            thisInstanceIdColumnName = ECDBSYS_PROP_SourceECInstanceId;
            thisClassIdColumnName = ECDBSYS_PROP_SourceECClassId;
            relatedInstanceIdColumnName = ECDBSYS_PROP_TargetECInstanceId;
            relatedClassIdColumnName = ECDBSYS_PROP_TargetECClassId;
            break;
        case '1': 
            direction = ECRelatedInstanceDirection::Backward;
            thisInstanceIdColumnName = ECDBSYS_PROP_TargetECInstanceId;
            thisClassIdColumnName = ECDBSYS_PROP_TargetECClassId;
            relatedInstanceIdColumnName = ECDBSYS_PROP_SourceECInstanceId;
            relatedClassIdColumnName = ECDBSYS_PROP_SourceECClassId;
            break;
        default:  
            return ExpressionStatus::UnknownError;
        }

    Utf8CP relationshipName = argTokens[0].c_str();
    Utf8CP relatedName      = argTokens[2].c_str();

    ECRelationshipClassCP relationshipClass = nullptr;
    relatedClass = nullptr;
    if (SUCCESS != FindRelationshipAndClassInfo(db, relationshipClass, relationshipName, relatedClass, relatedName))
        return ExpressionStatus::UnknownError;

    if (nullptr == relationshipClass || nullptr == relatedClass)
        {
        BeAssert(false);
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
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedInstanceQueryFormatNew(Utf8StringR query, ECDbCR db, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (instanceData.empty())
        return ExpressionStatus::WrongType;

    if (args.size() < 3)
        return ExpressionStatus::WrongNumberOfArguments;

    for (size_t i = 0; i < 3; ++i)
        {
        if (!args[i].IsECValue() || !args[i].GetECValue()->IsString())
            return ExpressionStatus::WrongType;
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
        return ExpressionStatus::UnknownError;
        }

    Utf8String relationshipSchemaName, relationshipClassName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(relationshipSchemaName, relationshipClassName, args[0].GetECValue()->GetUtf8CP()))
        {
        BeAssert(false);
        return ExpressionStatus::UnknownError;
        }

    Utf8String relatedClassSchemaName, relatedClassName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(relatedClassSchemaName, relatedClassName, args[2].GetECValue()->GetUtf8CP()))
        {
        BeAssert(false);
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
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::HasRelatedInstance(EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (3 != args.size())
        return ExpressionStatus::WrongNumberOfArguments;

    ECDbCR db = *reinterpret_cast<ECDb const*>(context);

    Utf8String queryFormat;
    ExpressionStatus stat = GetRelatedInstanceQueryFormatNew(queryFormat, db, instanceData, args);
    if (ExpressionStatus::Success != stat)
        return stat;

    for (IECInstancePtr const& instance : instanceData)
        {
        Utf8PrintfString query(queryFormat.c_str(), ECDBSYS_PROP_ECInstanceId, instance->GetClass().GetECSqlName().c_str());
        
        ECSqlStatement stmt;
        ECSqlStatus status = stmt.Prepare(db, query.c_str());
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        status = stmt.BindText(1, instance->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        if (DbResult::BE_SQLITE_ROW == stmt.Step())
            {
            evalResult.InitECValue().SetBoolean(true);
            return ExpressionStatus::Success;
            }
        }
    
    evalResult.InitECValue().SetBoolean(false);
    return ExpressionStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedInstance(EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (1 != args.size())
        return ExpressionStatus::WrongNumberOfArguments;

    ECDbCR db = *reinterpret_cast<ECDb const*>(context);

    Utf8String queryFormat;
    ECEntityClassCP relatedClass;
    ExpressionStatus stat = GetRelatedInstanceQueryFormatOld(queryFormat, relatedClass, db, instanceData, args[0]);
    if (ExpressionStatus::Success != stat)
        return stat;    

    IECInstancePtr relatedInstance;
    for (IECInstancePtr const& instance : instanceData)
        {
        Utf8PrintfString query(queryFormat.c_str(), "*", instance->GetClass().GetECSqlName().c_str());

        ECSqlStatement stmt;
        ECSqlStatus status = stmt.Prepare(db, query.c_str());
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        status = stmt.BindText(1, instance->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        ECInstanceECSqlSelectAdapter adapter(stmt);
        if (DbResult::BE_SQLITE_ROW == stmt.Step())
            {
            relatedInstance = adapter.GetInstance();
            break;
            }
        }
    
    if (relatedInstance.IsValid())
        evalResult.SetInstance (*relatedInstance);
    else
        evalResult.InitECValue().SetToNull();

    return ExpressionStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
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
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedValue(EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    if (4 != args.size())
        return ExpressionStatus::WrongNumberOfArguments;

    ECDbCR db = *reinterpret_cast<ECDb const*>(context);

    Utf8String queryFormat;
    ExpressionStatus stat = GetRelatedInstanceQueryFormatNew(queryFormat, db, instanceData, args);
    if (ExpressionStatus::Success != stat)
        return stat;    

    EvaluationResult const& propertyNameArg = args[3];
    if (!propertyNameArg.IsECValue() || !propertyNameArg.GetECValue()->IsUtf8())
        return ExpressionStatus::WrongType;
    
    IECInstancePtr relatedInstance;
    for (IECInstancePtr const& instance : instanceData)
        {
        Utf8PrintfString query(queryFormat.c_str(), propertyNameArg.GetECValue()->GetUtf8CP(), instance->GetClass().GetECSqlName().c_str());

        ECSqlStatement stmt;
        ECSqlStatus status = stmt.Prepare(db, query.c_str());
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        status = stmt.BindText(1, instance->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
        if (!status.IsSuccess())
            {
            BeAssert(false);
            continue;
            }

        if (DbResult::BE_SQLITE_ROW == stmt.Step())
            {
            evalResult.InitECValue() = GetECValueFromSqlValue(stmt.GetValue(0));
            return ExpressionStatus::Success;
            }
        }
    
    evalResult.InitECValue().SetToNull();
    return ExpressionStatus::Success;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Grigas.Petraitis                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
static ECEntityClassCP GetEntityClassFromSameSchema(ECClassCR other, Utf8CP name)
    {
    ECClassCP classCP = other.GetSchema().GetClassCP(name);
    return (nullptr != classCP) ? classCP->GetEntityClassCP() : nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Grigas.Petraitis                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
static ECRelationshipClassCP GetRelationshipClassFromSameSchema(ECClassCR other, Utf8CP name)
    {
    ECClassCP classCP = other.GetSchema().GetClassCP(name);
    return (nullptr != classCP) ? classCP->GetRelationshipClassCP() : nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Grigas.Petraitis                02/2016
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
    
    Utf8String sql("SELECT Id FROM ec_Class WHERE Name=?");
    ECDb* ecdb = const_cast<ECDb*>(&db);
    CachedStatementPtr statement = ecdb->GetCachedStatement(sql.c_str());
    BeAssert(statement.IsValid());
    if (nullptr == relationship)
        {
        statement->BindText(1, relationshipName, BeSQLite::Statement::MakeCopy::No);

        if (DbResult::BE_SQLITE_ROW != statement->Step())
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
        if (DbResult::BE_SQLITE_ROW != statement->Step())
            return ERROR;

        ECClassCP candidateEntityClass = db.Schemas().GetClass(statement->GetValueId<ECClassId>(0));
        if (nullptr == candidateEntityClass)
            return ERROR;
        entityClass = candidateEntityClass->GetEntityClassCP();
        }
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
