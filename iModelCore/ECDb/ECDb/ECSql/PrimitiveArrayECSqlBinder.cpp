/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveArrayECSqlBinder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
PrimitiveArrayECSqlBinder::PrimitiveArrayECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo)
: ECSqlBinder(ecsqlStatement, typeInfo, 1, true, true), m_initialCapacity(0), m_currentArrayIndex(-1), 
m_arrayElementBinder(*ecsqlStatement.GetECDb(), GetTypeInfo(), ARRAY_PROPERTY_INDEX), m_sqliteIndex(-1)
    {
    m_arrayStorageClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(*ecsqlStatement.GetECDb(), typeInfo.GetPrimitiveType());
    BeAssert(m_arrayStorageClass != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void PrimitiveArrayECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex)
    {
    m_sqliteIndex = (int) sqliteParameterIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveArrayECSqlBinder::_AddArrayElement()
    {
    BeAssert(m_arrayInstance != nullptr);

    const ECSqlStatus stat = ArrayConstraintValidator::ValidateMaximum(GetECDb(), GetTypeInfo(), GetCurrentArrayLength() + 1);
    if (!stat.IsSuccess())
        return NoopECSqlBinder::Get();

    m_currentArrayIndex++;
    uint32_t currentArrayIndex = (uint32_t) m_currentArrayIndex;

    if (currentArrayIndex >= m_initialCapacity)
        m_arrayInstance->AddArrayElements(ARRAY_PROPERTY_INDEX, 1);

    m_arrayElementBinder.Initialize(currentArrayIndex, *m_arrayInstance);
    return m_arrayElementBinder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::_BindNull()
    {
    _OnClearBindings();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& PrimitiveArrayECSqlBinder::_BindPrimitive()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind primitive value to array parameter.");
    return NoopECSqlBinder::Get().BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& PrimitiveArrayECSqlBinder::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind struct value to array parameter.");
    return NoopECSqlBinder::Get().BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& PrimitiveArrayECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    _OnClearBindings();
    m_initialCapacity = initialCapacity;

    if (m_arrayInstance == nullptr)
        m_arrayInstance = m_arrayStorageClass->GetDefaultStandaloneEnabler()->CreateInstance();

    if (m_initialCapacity > 0)
        m_arrayInstance->AddArrayElements(ARRAY_PROPERTY_INDEX, m_initialCapacity);

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::_OnBeforeStep()
    {
    const ECSqlStatus stat = ArrayConstraintValidator::Validate(GetECDb(), GetTypeInfo(), GetCurrentArrayLength());
    if (!stat.IsSuccess())
        return stat;

    PrimitiveECSqlBinder blobBinder(GetECSqlStatementR (), ECSqlTypeInfo(PRIMITIVETYPE_Binary));
    blobBinder.SetSqliteIndex(m_sqliteIndex);

    if (m_currentArrayIndex < 0)
        return blobBinder.BindNull();

    BeAssert(m_arrayInstance != nullptr);
    return blobBinder.BindBlob(m_arrayInstance->GetData(), m_arrayInstance->GetBytesUsed(), IECSqlBinder::MakeCopy::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void PrimitiveArrayECSqlBinder::_OnClearBindings()
    {
    if (m_arrayInstance != nullptr)
        m_arrayInstance->ClearValues();

    m_currentArrayIndex = -1;
    m_initialCapacity = 0;
    }


//====================================PrimitiveArrayToColumnECSqlBinder::ArrayElementBinder===================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
PrimitiveArrayECSqlBinder::ArrayElementBinder::ArrayElementBinder(ECDbCR ecdb, ECSqlTypeInfo const& arrayTypeInfo, uint32_t arrayPropertyIndex)
: IECSqlBinder(), IECSqlPrimitiveBinder(), m_ecdb(ecdb), m_arrayTypeInfo(arrayTypeInfo), m_instance(nullptr), m_arrayPropertyIndex(arrayPropertyIndex)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
void PrimitiveArrayECSqlBinder::ArrayElementBinder::Initialize(uint32_t arrayElementIndex, IECInstanceR instance)
    {
    m_arrayElementIndex = arrayElementIndex;
    m_instance = &instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle       03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindNull()
    {
    ECValue v;
    v.SetToNull();
    return SetValue(v);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindBoolean(bool value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Boolean)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Boolean);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Binary && m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_IGeometry)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Binary);

    ECValue v;
    if (m_arrayTypeInfo.GetPrimitiveType() == PRIMITIVETYPE_IGeometry)
        v.SetIGeometry(static_cast<Byte const*>(value), binarySize, makeCopy == IECSqlBinder::MakeCopy::Yes);
    else
        v.SetBinary(static_cast<Byte const*>(value), binarySize, makeCopy == IECSqlBinder::MakeCopy::Yes);

    return SetValue(v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindZeroBlob(int blobSize)
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Zeroblob value as element of a primitive array parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindDateTime(double julianDay, DateTime::Info const& metadata)
    {
    const uint64_t jdMsec = DateTime::RationalDayToMsec(julianDay);
    return _BindDateTime(jdMsec, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_DateTime)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_DateTime);

    if (metadata.IsValid() && metadata.GetKind() == DateTime::Kind::Local)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECDb does not support to bind local date times.");
        return ECSqlStatus::Error;
        }

    if (!m_arrayTypeInfo.DateTimeInfoMatches(metadata))
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Metadata of DateTime value to bind doesn't match the metadata on the corresponding ECProperty.");
        return ECSqlStatus::Error;
        }

    const int64_t ceTicks = DateTime::JulianDayToCommonEraTicks(julianDayMsec);
    ECValue v;
    v.SetDateTimeTicks(ceTicks, metadata);
    return SetValue(v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindDouble(double value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Double)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Double);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindInt(int value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Integer)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Integer);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindInt64(int64_t value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Long)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Long);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindPoint2d (DPoint2dCR value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Point2d)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Point2d);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindPoint3d (DPoint3dCR value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Point3d)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Point3d);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) 
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_String)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_String);

    return SetValue(ECValue(value, makeCopy == IECSqlBinder::MakeCopy::Yes));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::FailAndLogTypeMismatchError(PrimitiveType typeProvidedByCaller) const
    {
    Utf8CP expectedTypeName = ExpHelper::ToString(m_arrayTypeInfo.GetPrimitiveType());
    Utf8CP providedTypeName = ExpHelper::ToString(typeProvidedByCaller);

    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Primitive array element value type is incorrect. Expecting '%s' and caller provided '%s' which does not match.", expectedTypeName, providedTypeName);
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ArrayElementBinder::SetValue(ECValueCR value)
    {
    if (m_instance->SetValue(m_arrayPropertyIndex, value, m_arrayElementIndex) == ECObjectsStatus::Success)
        return ECSqlStatus::Success;

    BeAssert(false && "Failed to set value of array element");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindStruct()
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind struct value to primitive array element");
    return NoopECSqlBinder::Get().BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& PrimitiveArrayECSqlBinder::ArrayElementBinder::_BindArray(uint32_t initialCapacity)
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array value to array element.");
    return NoopECSqlBinder::Get().BindArray(initialCapacity);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
