/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbExpressionSymbolProvider.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
struct ECDbExpressionContext : ECN::SymbolExpressionContext
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
    context.AddSymbol(*MethodSymbol::Create("GetRelatedInstance", nullptr, &GetRelatedInstance, const_cast<ECDbP>(&m_db)));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Grigas.Petraitis                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
ExpressionStatus ECDbExpressionSymbolProvider::GetRelatedInstance(EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
    {
    // This method takes a single string argument of the format:
    //  "RelationshipClassName:Direction:RelatedClassName[, PropertyName:FailureValue]"
    // where dir: 0=forward, 1=backward
    // It doesn't specify schema names for either class, which makes our job more difficult and more time-consuming
    // If we fail to find any related instance, and PropertyName:FailureValue is specified, then we create a fake IECInstance
    // with a single property "PropertyName" with the value of FailureValue

    if (1 != args.size())
        return ExpressionStatus::WrongNumberOfArguments;

    if (!args[0].IsECValue() || !args[0].GetECValue()->IsUtf8())
        return ExpressionStatus::WrongType;

    if (instanceData.empty())
        return ExpressionStatus::WrongType;

    if (nullptr == context)
        return ExpressionStatus::UnknownError;
    
    ECDbCR db = *reinterpret_cast<ECDbCP>(context);

    bvector<Utf8String> argTokens;
    BeStringUtilities::Split(args[0].GetECValue()->GetUtf8CP(), ",", nullptr, argTokens);
    if (1 > argTokens.size() || 2 < argTokens.size())
        return ExpressionStatus::UnknownError;

    Utf8String arg = argTokens[0];                                        // "RelationshipClass:Direction:RelatedClass"
    Utf8String failureSpec = (argTokens.size() > 1) ? argTokens[1] : "";  // "PropertyName:FailureValue"
    argTokens.clear();

    BeStringUtilities::Split(arg.c_str(), ":", nullptr, argTokens);
    if (argTokens.size() != 3 || argTokens[1].length() != 1)
        return ExpressionStatus::UnknownError;

    Utf8CP thisInstanceIdColumnName,thisClassIdColumnName,
        relatedInstanceIdColumnName, relatedClassIdColumnName;
    ECRelatedInstanceDirection direction;
    switch (argTokens[1][0])
        {
        case '0':
            direction = ECRelatedInstanceDirection::Forward; 
            thisInstanceIdColumnName = "SourceECInstanceId";
            thisClassIdColumnName = "SourceECClassId";
            relatedInstanceIdColumnName = "TargetECInstanceId";
            relatedClassIdColumnName = "TargetECClassId";
            break;
        case '1': 
            direction = ECRelatedInstanceDirection::Backward;
            thisInstanceIdColumnName = "TargetECInstanceId";
            thisClassIdColumnName = "TargetECClassId"; 
            relatedInstanceIdColumnName = "SourceECInstanceId";
            relatedClassIdColumnName = "SourceECClassId";
            break;
        default:  
            return ExpressionStatus::UnknownError;
        }

    Utf8CP relationshipName = argTokens[0].c_str();
    Utf8CP relatedName      = argTokens[2].c_str();

    static Utf8CP selectQueryFormat = ""
        "SELECT related.* "
        "  FROM %s.%s this, %s.%s relationship, %s.%s related "
        " WHERE     this.ECInstanceId = ? "
        "       AND this.ECInstanceId = relationship.%s AND this.GetECClassId() = relationship.%s "
        "       AND related.ECInstanceId = relationship.%s AND related.GetECClassId() = relationship.%s";

    ECRelationshipClassCP relationshipClass = nullptr;
    ECEntityClassCP relatedEntityClass = nullptr;

    IECInstancePtr relatedInstance;
    for (IECInstancePtr const& instance : instanceData)
        {
        if ((nullptr == relationshipClass || nullptr == relatedEntityClass) 
            && SUCCESS != FindRelationshipAndClassInfo(db, relationshipClass, relationshipName, relatedEntityClass, relatedName))
            continue;

        BeAssert(nullptr != relationshipClass && nullptr != relatedEntityClass);

        Utf8PrintfString queryStr(selectQueryFormat, 
            instance->GetClass().GetSchema().GetName().c_str(), instance->GetClass().GetName().c_str(),
            relationshipClass->GetSchema().GetName().c_str(), relationshipClass->GetName().c_str(),
            relatedEntityClass->GetSchema().GetName().c_str(), relatedEntityClass->GetName().c_str(),
            thisInstanceIdColumnName, thisClassIdColumnName, relatedInstanceIdColumnName, relatedClassIdColumnName);

        ECSqlStatement stmt;
        ECSqlStatus status = stmt.Prepare(db, queryStr.c_str());
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

    /*if (relatedInstance.IsNull() && !failureSpec.empty())
        relatedInstance = CreatePseudoRelatedInstance (failureSpec.c_str());*/
    
    if (relatedInstance.IsValid())
        evalResult.SetInstance (*relatedInstance);
    else
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
    
    // search in all schemas
    bvector<ECN::ECSchemaCP> schemas = db.Schemas().GetECSchemas();
    if (schemas.empty())
        return ERROR;

    for (ECSchemaCP schema : schemas)
        {
        if (nullptr == relationship)
            {
            ECClassCP candidateRelationshipClass = schema->GetClassCP(relationshipName);
            if (nullptr != candidateRelationshipClass)
                relationship = candidateRelationshipClass->GetRelationshipClassCP();
            }

        if (nullptr == entityClass)
            {
            ECClassCP candidateEntityClass = schema->GetClassCP(className);
            if (nullptr != candidateEntityClass)
                entityClass = candidateEntityClass->GetEntityClassCP();
            }

        if (nullptr != relationship && nullptr != entityClass)
            return SUCCESS;
        }

    return ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
