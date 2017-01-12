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
: ECSqlBinder(ecsqlStatement, typeInfo, 1, true, true), m_currentElementBinder(*ecsqlStatement.GetECDb(), GetTypeInfo()), m_sqliteIndex(-1)
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::_OnBeforeStep()
    {
    const ECSqlStatus stat = ArrayConstraintValidator::Validate(GetECDb(), GetTypeInfo(), m_currentElementBinder.GetArrayLength());
    if (!stat.IsSuccess())
        return stat;

    PrimitiveECSqlBinder blobBinder(GetECSqlStatementR(), ECSqlTypeInfo(PRIMITIVETYPE_Binary));
    blobBinder.SetSqliteIndex(m_sqliteIndex);

    if (m_currentElementBinder.GetArrayLength() == 0)
        return blobBinder.BindNull();

    StandaloneECInstance& arrayInstance = m_currentElementBinder.GetArrayInstance();
    return blobBinder.BindBlob(arrayInstance.GetData(), arrayInstance.GetBytesUsed(), IECSqlBinder::MakeCopy::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveArrayECSqlBinder::_AddArrayElement()
    {
    if (ECSqlStatus::Success != m_currentElementBinder.MoveNext())
        return NoopECSqlBinder::Get();

    return m_currentElementBinder;
    }


//*****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/201´7
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::MoveNext()
    {
    const ECSqlStatus stat = ArrayConstraintValidator::ValidateMaximum(m_ecdb, m_arrayTypeInfo, GetArrayLength() + 1);
    if (!stat.IsSuccess())
        return stat;

    if (ECObjectsStatus::Success != GetArrayInstance().AddArrayElements(ARRAY_PROPERTY_INDEX, 1))
        return ECSqlStatus::Error;

    m_currentArrayIndex++;
    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/201´7
//---------------------------------------------------------------------------------------
void PrimitiveArrayECSqlBinder::ElementBinder::Clear()
    {
    if (m_arrayInstance != nullptr)
        m_arrayInstance->ClearValues();

    m_currentArrayIndex = -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindBoolean(bool value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Boolean)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Boolean);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
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
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value as element of a primitive array parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindDateTime(double julianDay, DateTime::Info const& metadata)
    {
    const uint64_t jdMsec = DateTime::RationalDayToMsec(julianDay);
    return _BindDateTime(jdMsec, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_DateTime)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_DateTime);

    if (metadata.IsValid() && metadata.GetKind() == DateTime::Kind::Local)
        {
        LOG.error("ECDb does not support to bind local date times.");
        return ECSqlStatus::Error;
        }

    if (!m_arrayTypeInfo.DateTimeInfoMatches(metadata))
        {
        LOG.error("Metadata of DateTime value to bind doesn't match the metadata on the corresponding ECProperty.");
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
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindDouble(double value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Double)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Double);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindInt(int value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Integer)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Integer);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindInt64(int64_t value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Long)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Long);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindPoint2d(DPoint2dCR value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Point2d)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Point2d);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindPoint3d(DPoint3dCR value)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Point3d)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_Point3d);

    return SetValue(ECValue(value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    if (m_arrayTypeInfo.GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_String)
        return FailAndLogTypeMismatchError(PrimitiveType::PRIMITIVETYPE_String);

    return SetValue(ECValue(value, makeCopy == IECSqlBinder::MakeCopy::Yes));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveArrayECSqlBinder::ElementBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to primitive array parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveArrayECSqlBinder::ElementBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to primitive array parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveArrayECSqlBinder::ElementBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to element of a primitive array parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::SetValue(ECValueCR value)
    {
    if (GetArrayInstance().SetValue(ARRAY_PROPERTY_INDEX, value, m_currentArrayIndex) == ECObjectsStatus::Success)
        return ECSqlStatus::Success;

    BeAssert(false && "Failed to set value of array element");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/201´7
//---------------------------------------------------------------------------------------
StandaloneECInstance& PrimitiveArrayECSqlBinder::ElementBinder::GetArrayInstance()
    {
    if (m_arrayInstance == nullptr)
        {
        ECClassCP arrayStorageClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(m_ecdb, m_arrayTypeInfo.GetPrimitiveType());
        BeAssert(arrayStorageClass != nullptr);
        m_arrayInstance = arrayStorageClass->GetDefaultStandaloneEnabler()->CreateInstance();
        }

    BeAssert(m_arrayInstance != nullptr);
    return *m_arrayInstance;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayECSqlBinder::ElementBinder::FailAndLogTypeMismatchError(PrimitiveType typeProvidedByCaller) const
    {
    Utf8CP expectedTypeName = ExpHelper::ToString(m_arrayTypeInfo.GetPrimitiveType());
    Utf8CP providedTypeName = ExpHelper::ToString(typeProvidedByCaller);

    LOG.errorv("Primitive array element value type is incorrect. Expecting '%s' and caller provided '%s' which does not match.", expectedTypeName, providedTypeName);
    return ECSqlStatus::Error;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
