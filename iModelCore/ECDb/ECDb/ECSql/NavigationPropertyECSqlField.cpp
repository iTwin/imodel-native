/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlField::SetMembers(std::unique_ptr<ECSqlField> idField, std::unique_ptr<ECSqlField> relClassIdField)
    {
    BeAssert(idField != nullptr && !idField->RequiresOnAfterReset() && !idField->RequiresOnAfterStep());
    BeAssert(relClassIdField != nullptr && !relClassIdField->RequiresOnAfterReset() && !relClassIdField->RequiresOnAfterStep());
    m_idField = std::move(idField);
    m_relClassIdField = std::move(relClassIdField);
    BeAssert(m_idField != nullptr && m_relClassIdField != nullptr);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
IECSqlValue const& NavigationPropertyECSqlField::_GetStructMemberValue(Utf8CP memberName) const
    {
    if (BeStringUtilities::StricmpAscii(memberName, ECDBSYS_PROP_NavPropId) == 0)
        return *m_idField;

    if (BeStringUtilities::StricmpAscii(memberName, ECDBSYS_PROP_NavPropRelECClassId) == 0)
        return *m_relClassIdField;

    LOG.errorv("Member name '%s' passed to navigation property IECSqlValue[Utf8CP] does not exist.", memberName);
    return NoopECSqlValue::GetSingleton()[memberName];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlField::_OnAfterStep()
    {
    ECSqlStatus stat = m_idField->OnAfterStep();
    if (!stat.IsSuccess())
        return stat;

    return m_relClassIdField->OnAfterStep();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlField::_OnAfterReset()
    {
    ECSqlStatus stat = m_idField->OnAfterReset();
    if (!stat.IsSuccess())
        return stat;

    return m_relClassIdField->OnAfterReset();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void const* NavigationPropertyECSqlField::_GetBlob(int* blobSize) const
    {
    LOG.error("GetBlob cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool NavigationPropertyECSqlField::_GetBoolean() const
    {
    LOG.error("GetBoolean cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t NavigationPropertyECSqlField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    {
    LOG.error("GetDateTime cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double NavigationPropertyECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    LOG.error("GetDateTime cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double NavigationPropertyECSqlField::_GetDouble() const
    {
    LOG.error("GetDouble cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int NavigationPropertyECSqlField::_GetInt() const
    {
    LOG.error("GetInt cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t NavigationPropertyECSqlField::_GetInt64() const
    {
    LOG.error("GetInt64 cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetInt64();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP NavigationPropertyECSqlField::_GetText() const
    {
    LOG.error("GetText cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetText();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2014
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr NavigationPropertyECSqlField::_GetGeometry() const
    {
    LOG.error("GetGeometry cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d NavigationPropertyECSqlField::_GetPoint2d() const
    {
    LOG.error("GetPoint2d cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d NavigationPropertyECSqlField::_GetPoint3d() const
    {
    LOG.error("GetPoint3d cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
int NavigationPropertyECSqlField::_GetArrayLength() const
    {
    LOG.error("GetArrayLength cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& NavigationPropertyECSqlField::_GetArrayIterable() const
    {
    LOG.error("GetArrayIterable cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
    }

//***************************** NavigationPropertyECSqlField::IteratorState *****************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
void NavigationPropertyECSqlField::IteratorState::_MoveToNext(bool onInitializingIterator) const
    {
    uint8_t current = (uint8_t) m_state;
    current++;
    m_state = (State) current;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& NavigationPropertyECSqlField::IteratorState::_GetCurrent() const
    {
    switch (m_state)
        {
            case State::Id:
                return *m_field.m_idField;

            case State::RelECClassId:
                return *m_field.m_relClassIdField;

            default:
                BeAssert(false);
                return NoopECSqlValue::GetSingleton();
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE