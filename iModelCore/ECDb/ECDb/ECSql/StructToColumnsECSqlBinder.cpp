/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructToColumnsECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "StructToColumnsECSqlBinder.h"
#include "ECSqlTypeInfo.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
StructToColumnsECSqlBinder::StructToColumnsECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo)
    : ECSqlBinder(ecsqlStatement, ecsqlTypeInfo, 0, true, true), IECSqlStructBinder()
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
BentleyStatus StructToColumnsECSqlBinder::Initialize(ECSqlPrepareContext& ctx)
    {
    ECSqlTypeInfo const& typeInfo = GetTypeInfo();
    BeAssert(typeInfo.GetPropertyMap() != nullptr && typeInfo.GetPropertyMap()->GetType() == PropertyMap::Type::Struct && "Struct parameters are expected to always have a PropertyNameExp as target expression");
    StructPropertyMap const* structPropMap = typeInfo.GetPropertyMap()->GetAs<StructPropertyMap>();

    int totalMappedSqliteParameterCount = 0;
    for (PropertyMap const* memberPropMap : *structPropMap) //GetChildren ensures the correct and always same order
        {
        std::unique_ptr<ECSqlBinder> binder = ECSqlBinderFactory::CreateBinder(ctx, *memberPropMap);
        if (binder == nullptr)
            return ERROR;

        int mappedSqliteParameterCount = binder->GetMappedSqlParameterCount();

        auto binderP = binder.get(); //cache raw pointer as it is needed after the unique_ptr has been moved into the collection
        m_memberBinders[memberPropMap->GetProperty().GetId()] = std::move(binder);

        //for SetSqliteIndex we need a mapping from a given ECSqlComponent index to the respective member binder
        //and also the relative component index within that member binder.
        m_ecsqlComponentIndexToMemberBinderMapping.reserve(mappedSqliteParameterCount);
        m_ecsqlComponentIndexToMemberBinderMapping.insert(m_ecsqlComponentIndexToMemberBinderMapping.end(), mappedSqliteParameterCount,
                                                          MemberBinderInfo(*binderP, totalMappedSqliteParameterCount));

        totalMappedSqliteParameterCount += mappedSqliteParameterCount;
        }

    SetMappedSqlParameterCount(totalMappedSqliteParameterCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void StructToColumnsECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteIndex)
    {
    BeAssert(ecsqlParameterComponentIndex >= 0 && (size_t) ecsqlParameterComponentIndex < m_ecsqlComponentIndexToMemberBinderMapping.size());
    auto const& mapping = m_ecsqlComponentIndexToMemberBinderMapping[(size_t) ecsqlParameterComponentIndex];
    int compIndexOfMember = ecsqlParameterComponentIndex - mapping.GetECSqlComponentIndexOffset();

    mapping.GetMemberBinder().SetSqliteIndex(compIndexOfMember, sqliteIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2015
//---------------------------------------------------------------------------------------
ECSqlStatus StructToColumnsECSqlBinder::_OnBeforeStep()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        auto stat = kvPair.second->OnBeforeStep();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2015
//---------------------------------------------------------------------------------------
void StructToColumnsECSqlBinder::_OnClearBindings()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        kvPair.second->OnClearBindings();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructToColumnsECSqlBinder::_BindNull()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        auto stat = kvPair.second->BindNull();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& StructToColumnsECSqlBinder::_BindPrimitive()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind primitive value to ECStruct parameter.");
    return NoopECSqlBinder::Get().BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& StructToColumnsECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array to ECStruct parameter.");
    return NoopECSqlBinder::Get().BindArray(initialCapacity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructToColumnsECSqlBinder::_GetMember(Utf8CP structMemberPropertyName)
    {
    auto memberProp = GetTypeInfo().GetStructType().GetPropertyP(structMemberPropertyName, true);
    if (memberProp == nullptr)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        BeAssert(false);
        return NoopECSqlBinder::Get();
        }

    return _GetMember(memberProp->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructToColumnsECSqlBinder::_GetMember(ECN::ECPropertyId structMemberPropertyId)
    {
    auto it = m_memberBinders.find(structMemberPropertyId);
    if (it == m_memberBinders.end())
        {
        Utf8CP structMemberPropertyName = nullptr;
        for (auto prop : GetTypeInfo().GetStructType().GetProperties(true))
            {
            if (prop->GetId() == structMemberPropertyId)
                {
                structMemberPropertyName = prop->GetName().c_str();
                break;
                }
            }

        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        return NoopECSqlBinder::Get();
        }

    return *it->second;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE