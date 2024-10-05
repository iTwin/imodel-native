/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StructECSqlBinder::StructECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& ecsqlTypeInfo, SqlParamNameGenerator& paramNameGen)
    : ECSqlBinder(ctx, ecsqlTypeInfo, paramNameGen, true, true), m_binderInfo( BinderInfo::BinderType::Struct)
    {
    Initialize(ctx, paramNameGen);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus StructECSqlBinder::Initialize(ECSqlPrepareContext& ctx, SqlParamNameGenerator& paramNameGen)
    {
    ECSqlTypeInfo const& typeInfo = GetTypeInfo();
    BeAssert(typeInfo.GetPropertyMap() != nullptr && typeInfo.GetPropertyMap()->GetType() == PropertyMap::Type::Struct && "Struct parameters are expected to always have a PropertyNameExp as target expression");
    StructPropertyMap const& structPropMap = typeInfo.GetPropertyMap()->GetAs<StructPropertyMap>();

    for (PropertyMap const* memberPropMap : structPropMap) //GetChildren ensures the correct and always same order
        {
        std::unique_ptr<ECSqlBinder> binder = ECSqlBinderFactory::CreateBinder(ctx, *memberPropMap, paramNameGen);
        if (binder == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        AddChildMemberMappedSqlParameterIndices(*binder);
        m_memberBinders[memberPropMap->GetProperty().GetId()] = std::move(binder);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_OnBeforeFirstStep()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        auto stat = kvPair.second->OnBeforeFirstStep();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void StructECSqlBinder::_OnClearBindings()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        kvPair.second->OnClearBindings();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind boolean value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind double value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind integer value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindInt64(int64_t value)
    {
    LOG.error("Type mismatch. Cannot bind int64_t value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    LOG.error("Type mismatch. Cannot bind string value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point2d value to struct parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point3d value to struct parameter.");
    return ECSqlStatus::Error;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& StructECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to struct parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BinderInfo const& StructECSqlBinder::_GetBinderInfo()
    {
    return m_binderInfo;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE