/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SystemPropertyECSqlBinder.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SystemPropertyECSqlBinder.h"
#include "ECSqlStatementBase.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
void SystemPropertyECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteIndex)
    {
    BeAssert(!IsNoop());
    m_sqliteIndex = (int) sqliteIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_OnBeforeStep()
    {
    if ((IsEnsureConstraints()) && m_bindValueIsNull)
        {
        Utf8CP propName = SystemPropertyToString();
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Constraint violation. No value bound to %s parameter. %s cannot be NULL.", propName, propName);
        }

    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
void SystemPropertyECSqlBinder::_OnClearBindings()
    {
    m_bindValueIsNull = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindNull()
    {
    if (IsEnsureConstraints())
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Constraint violation. Cannot bind NULL to %s parameter.", SystemPropertyToString());

    if (auto eh = GetOnBindEventHandler())
        {
        auto es = eh->BindNull();
        if (es != ECSqlStatus::Success)
            return GetStatusContext().SetError(es, "OnBindEventHandler Failed");
        }

    if (!IsNoop())
        {
        const auto sqliteStat = GetSqliteStatementR ().BindNull(m_sqliteIndex);
        if (sqliteStat != BE_SQLITE_OK)
            return SetError(sqliteStat, "ECSqlStatement::BindNull against system property.");
        }

    m_bindValueIsNull = true;
    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindPoint2D (DPoint2dCR value)
    {
    return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind Point3D value to %s parameter.", SystemPropertyToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindPoint3D (DPoint3dCR value)
    {
    return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind Point3D value to %s parameter.", SystemPropertyToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindBoolean(bool value)
    {
    return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind boolean value to %s parameter.", SystemPropertyToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind Blob value to %s parameter.", SystemPropertyToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindDateTime(uint64_t julianDayTicksHns, DateTime::Info const* metadata)
    {
    return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind DateTime value to %s parameter.", SystemPropertyToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindDouble(double value)
    {
    return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind double value to %s parameter.", SystemPropertyToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind IGeometry value to %s parameter.", SystemPropertyToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindInt(int value)
    {
    return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind 32 bit integer value to %s parameter.", SystemPropertyToString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindInt64(int64_t value)
    {
    auto stat = FailIfConstraintClassIdViolation(static_cast<ECClassId> (value));
    if (stat != ECSqlStatus::Success)
        return stat;

    if (auto eh = GetOnBindEventHandler())
        {
        auto es = eh->BindInt64(value);
        if (es != ECSqlStatus::Success)
            return GetStatusContext().SetError(es, "OnBindEventHandler Failed");
        }

    if (!IsNoop())
        {
        const auto sqliteStat = GetSqliteStatementR ().BindInt64(m_sqliteIndex, value);
        if (sqliteStat != BE_SQLITE_OK)
            return SetError(sqliteStat, "ECSqlStatement::BindInt64");
        }

    auto onBindEventHandler = GetOnBindRepositoryBasedIdEventHandler();
    if (onBindEventHandler != nullptr)
        onBindEventHandler(ECInstanceId(value));

    m_bindValueIsNull = false;
    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    //by ECSQL design ECInstanceIds can be specified as numeric values or as string values
    if (m_systemProperty != ECSqlSystemProperty::ECInstanceId && m_systemProperty != ECSqlSystemProperty::ParentECInstanceId &&
        m_systemProperty != ECSqlSystemProperty::SourceECInstanceId && m_systemProperty != ECSqlSystemProperty::TargetECInstanceId)
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind string value to %s parameter.", SystemPropertyToString());

    if (auto eh = GetOnBindEventHandler())
        {
        auto es = eh->BindText(value, makeCopy, byteCount);
        if (es != ECSqlStatus::Success)
            return GetStatusContext().SetError(es, "OnBindEventHandler Failed");
        }

    if (!IsNoop())
        {
        const auto sqliteStat = GetSqliteStatementR ().BindText(m_sqliteIndex, value, ToBeSQliteBindMakeCopy(makeCopy), byteCount);
        if (sqliteStat != BE_SQLITE_OK)
            return SetError(sqliteStat, "ECSqlStatement::BindText");
        }

    auto onBindEventHandler = GetOnBindRepositoryBasedIdEventHandler();
    if (onBindEventHandler != nullptr)
        {
        ECInstanceId id;
        if (!ECInstanceIdHelper::FromString(id, value))
            return GetStatusContext().SetError(ECSqlStatus::UserError, "Binding string value to %s parameter failed. Value cannot be converted to an ECInstanceId.", SystemPropertyToString());

        onBindEventHandler(id);
        }

    m_bindValueIsNull = false;
    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& SystemPropertyECSqlBinder::_BindPrimitive()
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& SystemPropertyECSqlBinder::_BindStruct()
    {
    const auto stat = GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind ECStruct to %s parameter.", SystemPropertyToString());
    return GetNoopBinder(stat).BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& SystemPropertyECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    const auto stat = GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind array to %s parameter.", SystemPropertyToString());
    return GetNoopBinder(stat).BindArray(initialCapacity);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus SystemPropertyECSqlBinder::FailIfConstraintClassIdViolation(ECN::ECClassId const& constraintClassId) const
    {
    if (!IsEnsureConstraints() || (m_systemProperty != ECSqlSystemProperty::SourceECClassId && m_systemProperty != ECSqlSystemProperty::TargetECClassId))
        return ECSqlStatus::Success;

    const auto relationshipEnd = m_systemProperty == ECSqlSystemProperty::SourceECClassId ? ECN::ECRelationshipEnd_Source : ECN::ECRelationshipEnd_Target;
    if (!m_constraints->GetConstraintMap(relationshipEnd).ClassIdMatchesConstraint(constraintClassId))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Constraint violation. ECClassId %lld not valid as value for %s parameter.", 
        constraintClassId, SystemPropertyToString());

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
Utf8CP SystemPropertyECSqlBinder::SystemPropertyToString() const
    {
    return ECDbSystemSchemaHelper::ToString(m_systemProperty);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE