/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveArrayToColumnECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ExpHelper.h"
#include "PrimitiveArrayToColumnECSqlBinder.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
PrimitiveArrayToColumnECSqlBinder::PrimitiveArrayToColumnECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo)
: ECSqlBinder(ecsqlStatement, typeInfo, 1, true, true), m_initialCapacity(0), m_currentArrayIndex(-1), 
m_arrayElementBinder(*ecsqlStatement.GetECDb(), GetTypeInfo(), ARRAY_PROPERTY_INDEX), m_sqliteIndex(-1)
    {
    m_arrayStorageClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(*ecsqlStatement.GetECDb(), typeInfo.GetPrimitiveType());
    BeAssert(m_arrayStorageClass != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void PrimitiveArrayToColumnECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex)
    {
    m_sqliteIndex = (int) sqliteParameterIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveArrayToColumnECSqlBinder::_AddArrayElement()
    {
    const ECSqlStatus stat = ArrayConstraintValidator::ValidateMaximum(GetECDb(), GetTypeInfo(), GetCurrentArrayLength() + 1);
    if (!stat.IsSuccess())
        return NoopECSqlBinder::Get();

    m_currentArrayIndex++;
    uint32_t currentArrayIndex = (uint32_t) m_currentArrayIndex;

    auto arrayInstance = GetInstance(true);
    if (currentArrayIndex >= m_initialCapacity)
        arrayInstance->AddArrayElements(ARRAY_PROPERTY_INDEX, 1);

    m_arrayElementBinder.Initialize(currentArrayIndex, *arrayInstance);
    return m_arrayElementBinder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::_BindNull()
    {
    _OnClearBindings();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& PrimitiveArrayToColumnECSqlBinder::_BindPrimitive()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind primitive value to array parameter.");
    return NoopECSqlBinder::Get().BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& PrimitiveArrayToColumnECSqlBinder::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind struct value to array parameter.");
    return NoopECSqlBinder::Get().BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& PrimitiveArrayToColumnECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    _OnClearBindings();
    m_initialCapacity = initialCapacity;
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::_OnBeforeStep()
    {
    const auto stat = ArrayConstraintValidator::Validate(GetECDb(), GetTypeInfo(), GetCurrentArrayLength());
    if (!stat.IsSuccess())
        return stat;

    PrimitiveToSingleColumnECSqlBinder blobBinder(GetECSqlStatementR (), ECSqlTypeInfo(PRIMITIVETYPE_Binary));
    blobBinder.SetSqliteIndex(m_sqliteIndex);

    if (m_instance == nullptr)
        return blobBinder.BindNull();
    else
        return blobBinder.BindBinary(m_instance->GetData(), m_instance->GetBytesUsed(), IECSqlBinder::MakeCopy::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void PrimitiveArrayToColumnECSqlBinder::_OnClearBindings()
    {
    m_instance = nullptr;
    m_currentArrayIndex = -1;
    m_initialCapacity = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
StandaloneECInstanceP PrimitiveArrayToColumnECSqlBinder::GetInstance(bool create) const
    {
    if (create && m_instance.IsNull())
        {
        m_instance = m_arrayStorageClass->GetDefaultStandaloneEnabler()->CreateInstance();
        if (m_initialCapacity > 0)
            m_instance->AddArrayElements(ARRAY_PROPERTY_INDEX, m_initialCapacity);
        }

    return m_instance.get();
    }



//====================================PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder===================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::ArrayElementBinder(ECDbCR ecdb, ECSqlTypeInfo const& arrayTypeInfo, uint32_t arrayPropertyIndex)
: IECSqlBinder(), IECSqlPrimitiveBinder(), m_ecdb(ecdb), m_arrayTypeInfo(arrayTypeInfo), m_instance(nullptr), m_arrayPropertyIndex(arrayPropertyIndex)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
void PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::Initialize(uint32_t arrayElementIndex, IECInstanceR instance)
    {
    m_arrayElementIndex = arrayElementIndex;
    m_instance = &instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle       03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindNull()
    {
    ECValue v;
    v.SetToNull();
    return SetValue(v);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindBoolean(bool value)
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_Boolean);
    if (!status.IsSuccess())
        return status;

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) 
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_Binary);
    if (!status.IsSuccess())
        return status;

    ECValue v;
    v.SetBinary(static_cast<Byte const*>(value), binarySize, makeCopy == IECSqlBinder::MakeCopy::Yes);
    return SetValue(v);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_IGeometry);
    if (!status.IsSuccess())
        return status;

    ECValue v;
    v.SetIGeometry(static_cast<Byte const*>(value), blobSize, makeCopy == IECSqlBinder::MakeCopy::Yes);
    return SetValue(v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindDateTime(double julianDay, DateTime::Info const* metadata)
    {
    const uint64_t jdHns = DateTime::RationalDayToHns(julianDay);
    return _BindDateTime(jdHns, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindDateTime(uint64_t julianDayHns, DateTime::Info const* metadata)
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_DateTime);
    if (!status.IsSuccess())
        return status;

    if (metadata != nullptr && metadata->GetKind() == DateTime::Kind::Local)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECDb does not support to bind local date times.");
        return ECSqlStatus::Error;
        }

    if (!m_arrayTypeInfo.DateTimeInfoMatches(metadata))
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Metadata of DateTime value to bind doesn't match the metadata on the corresponding ECProperty.");
        return ECSqlStatus::Error;
        }

    const int64_t ceTicks = DateTime::JulianDayToCommonEraTicks(julianDayHns);
    ECValue v;
    if (metadata == nullptr)
        v.SetDateTimeTicks(ceTicks);
    else
        v.SetDateTimeTicks(ceTicks, *metadata);

    return SetValue(v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindDouble(double value)
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_Double);
    if (!status.IsSuccess())
        return status;

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindInt(int value)
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_Integer);
    if (!status.IsSuccess())
        return status;

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindInt64(int64_t value)
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_Long);
    if (!status.IsSuccess())
        return status;

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindPoint2d (DPoint2dCR value)
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_Point2d);
    if (!status.IsSuccess())
        return status;

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindPoint3d (DPoint3dCR value)
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_Point3d);
    if (!status.IsSuccess())
        return status;

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) 
    {
    auto status = VerifyType(PrimitiveType::PRIMITIVETYPE_String);
    if (!status.IsSuccess())
        return status;

    return SetValue(ECValue(value, makeCopy == IECSqlBinder::MakeCopy::Yes));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::VerifyType(PrimitiveType type) const
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != type)
        {
        Utf8CP expectedTypeName = ExpHelper::ToString(m_arrayTypeInfo.GetPrimitiveType());
        Utf8CP providedTypeName = ExpHelper::ToString(type);

        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Primitive array element value type is incorrect. Expecting '%s' and user provided '%s' which does not match.", expectedTypeName, providedTypeName);
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::SetValue(ECValueCR value)
    {
    if (m_instance->SetValue(m_arrayPropertyIndex, value, m_arrayElementIndex) == ECObjectsStatus::Success)
        return ECSqlStatus::Success;

    BeAssert(false && "Failed to set value of array element");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindPrimitive()
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindStruct()
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind struct value to primitive array element");
    return NoopECSqlBinder::Get().BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder::_BindArray(uint32_t initialCapacity)
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array value to array element.");
    return NoopECSqlBinder::Get().BindArray(initialCapacity);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
