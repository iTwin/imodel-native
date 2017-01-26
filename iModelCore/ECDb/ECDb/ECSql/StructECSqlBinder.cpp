/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructECSqlBinder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
StructECSqlBinder::StructECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo)
    : ECSqlBinder(ecsqlStatement, ecsqlTypeInfo, 0, true, true)
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
BentleyStatus StructECSqlBinder::Initialize(ECSqlPrepareContext& ctx)
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
void StructECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteIndex)
    {
    BeAssert(ecsqlParameterComponentIndex >= 0 && (size_t) ecsqlParameterComponentIndex < m_ecsqlComponentIndexToMemberBinderMapping.size());
    auto const& mapping = m_ecsqlComponentIndexToMemberBinderMapping[(size_t) ecsqlParameterComponentIndex];
    int compIndexOfMember = ecsqlParameterComponentIndex - mapping.GetECSqlComponentIndexOffset();

    mapping.GetMemberBinder().SetSqliteIndex(compIndexOfMember, sqliteIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2015
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_OnBeforeStep()
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
void StructECSqlBinder::_OnClearBindings()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        kvPair.second->OnClearBindings();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindNull()
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
IECSqlBinder& StructECSqlBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    auto memberProp = GetTypeInfo().GetStructType().GetPropertyP(structMemberPropertyName, true);
    if (memberProp == nullptr)
        {
        LOG.errorv("Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        BeAssert(false);
        return NoopECSqlBinder::Get();
        }

    return _BindStructMember(memberProp->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructECSqlBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
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

        LOG.errorv("Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        return NoopECSqlBinder::Get();
        }

    return *it->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind boolean value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2015
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind double value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind integer value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindInt64(int64_t value)
    {
    LOG.error("Type mismatch. Cannot bind int64_t value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    LOG.error("Type mismatch. Cannot bind string value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point2d value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point3d value to struct parameter.");
    return ECSqlStatus::Error;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to struct parameter.");
    return NoopECSqlBinder::Get();
    }
END_BENTLEY_SQLITE_EC_NAMESPACE