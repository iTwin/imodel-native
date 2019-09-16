/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/LinearlyReferencedLocation.h>

HANDLER_DEFINE_MEMBERS(LinearlyReferencedAtLocationHandler)
HANDLER_DEFINE_MEMBERS(LinearlyReferencedFromToLocationHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocation::LinearlyReferencedLocation()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LinearlyReferencedLocation::SetDistanceExpressionValue(DistanceExpressionR expression, Utf8CP ecPropertyName, ECValueCR value)
    {
    if (0 == strcmp("DistanceAlongFromStart", ecPropertyName) && !value.IsNull())
        {
        expression.SetDistanceAlongFromStart(value.GetDouble());
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp("DistanceAlongFromReferent", ecPropertyName))
        {
        if (value.IsNull())
            expression.SetDistanceAlongFromReferent(NullableDouble());
        else
            expression.SetDistanceAlongFromReferent(value.GetDouble());
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp("LateralOffsetFromILinearElement", ecPropertyName))
        {
        if (value.IsNull())
            expression.SetLateralOffsetFromILinearElement(NullableDouble());
        else
            expression.SetLateralOffsetFromILinearElement(value.GetDouble());
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp("VerticalOffsetFromILinearElement", ecPropertyName))
        {
        if (value.IsNull())
            expression.SetVerticalOffsetFromILinearElement(NullableDouble());
        else
            expression.SetVerticalOffsetFromILinearElement(value.GetDouble());
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LinearlyReferencedLocation::GetECValue(ECN::ECValueR value, DistanceExpressionCR expression, Utf8CP ecPropertyName)
    {
    if (0 == strcmp("DistanceAlongFromStart", ecPropertyName) && !value.IsNull())
        {
        value.SetDouble(expression.GetDistanceAlongFromStart());
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp("DistanceAlongFromReferent", ecPropertyName))
        {
        if (expression.GetDistanceAlongFromReferent().IsNull())
            value.SetIsNull(true);
        else
            value.SetDouble(expression.GetDistanceAlongFromReferent().Value());
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp("LateralOffsetFromILinearElement", ecPropertyName))
        {
        if (expression.GetLateralOffsetFromILinearElement().IsNull())
            value.SetIsNull(true);
        else
            value.SetDouble(expression.GetLateralOffsetFromILinearElement().Value());
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp("VerticalOffsetFromILinearElement", ecPropertyName))
        {
        if (expression.GetVerticalOffsetFromILinearElement().IsNull())
            value.SetIsNull(true);
        else
            value.SetDouble(expression.GetVerticalOffsetFromILinearElement().Value());
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocation::LinearlyReferencedAtLocation()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocation::LinearlyReferencedAtLocation(DistanceExpressionCR atPosition):
    m_atPosition(atPosition)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationPtr LinearlyReferencedAtLocation::Create(DistanceExpressionCR atPosition)
    {
    return new LinearlyReferencedAtLocation(atPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool LinearlyReferencedAtLocation::_HasChanges() const
    {
    return m_originalAtPosition != m_atPosition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedAtLocation::_UpdateProperties(DgnElementCR el, ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " "
        "SET AtPosition.DistanceAlongFromStart = ?, AtPosition.LateralOffsetFromILinearElement = ?, AtPosition.VerticalOffsetFromILinearElement = ?, "
        "   AtPosition.DistanceAlongFromReferent = ?, FromReferent.Id = ? "
        "WHERE ECInstanceId = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindDouble(1, GetAtPosition().GetDistanceAlongFromStart());

    if (GetAtPosition().GetLateralOffsetFromILinearElement().IsValid())
        stmtPtr->BindDouble(2, GetAtPosition().GetLateralOffsetFromILinearElement().Value());
    else
        stmtPtr->BindNull(2);

    if (GetAtPosition().GetVerticalOffsetFromILinearElement().IsValid())
        stmtPtr->BindDouble(3, GetAtPosition().GetVerticalOffsetFromILinearElement().Value());
    else
        stmtPtr->BindNull(3);

    if (GetAtPosition().GetDistanceAlongFromReferent().IsValid())
        stmtPtr->BindDouble(4, GetAtPosition().GetDistanceAlongFromReferent().Value());
    else
        stmtPtr->BindNull(4);

    if (GetAtPosition().GetFromReferentId().IsValid())
        stmtPtr->BindId(5, GetAtPosition().GetFromReferentId());
    else
        stmtPtr->BindNull(5);

    stmtPtr->BindId(6, GetAspectInstanceId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedAtLocation::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT AtPosition.DistanceAlongFromStart, AtPosition.LateralOffsetFromILinearElement, AtPosition.VerticalOffsetFromILinearElement,"
        "   AtPosition.DistanceAlongFromReferent, FromReferent.Id " 
        "FROM " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " "
        "WHERE ECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetAspectInstanceId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadArg;

    m_atPosition = DistanceExpression(
        stmtPtr->GetValueDouble(0), 
        stmtPtr->IsValueNull(1) ? NullableDouble() : stmtPtr->GetValueDouble(1),
        stmtPtr->IsValueNull(2) ? NullableDouble() : stmtPtr->GetValueDouble(2), 
        stmtPtr->IsValueNull(4) ? DgnElementId() : stmtPtr->GetValueId<DgnElementId>(4),
        stmtPtr->IsValueNull(3) ? NullableDouble() : stmtPtr->GetValueDouble(3));

    m_originalAtPosition = m_atPosition;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedAtLocation::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 != strcmp("AtPosition", propertyName))
        return DgnDbStatus::BadRequest;

    if (BentleyStatus::SUCCESS == GetECValue(value, GetAtPosition(), propertyName))
        return DgnDbStatus::Success;

    return DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedAtLocation::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 != strcmp("AtPosition", propertyName))
        return DgnDbStatus::BadRequest;

    if (BentleyStatus::SUCCESS == SetDistanceExpressionValue(GetAtPositionR(), propertyName, value))
        return DgnDbStatus::Success;

    return DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocation::LinearlyReferencedFromToLocation()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocation::LinearlyReferencedFromToLocation(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition):
    m_fromPosition(fromPosition), m_toPosition(toPosition), m_originalFromPosition(DistanceExpression()), m_originalToPosition(DistanceExpression())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationPtr LinearlyReferencedFromToLocation::Create(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition)
    {
    return new LinearlyReferencedFromToLocation(fromPosition, toPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool LinearlyReferencedFromToLocation::_HasChanges() const
    {
    return (m_originalFromPosition != m_fromPosition || m_originalToPosition != m_toPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedFromToLocation::_UpdateProperties(DgnElementCR el, ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " "
        "SET FromPosition.DistanceAlongFromStart = ?, FromPosition.LateralOffsetFromILinearElement = ?, FromPosition.VerticalOffsetFromILinearElement = ?, "
        "   FromPosition.DistanceAlongFromReferent = ?, FromPositionFromReferent.Id = ?, "
        "   ToPosition.DistanceAlongFromStart = ?, ToPosition.LateralOffsetFromILinearElement = ?, ToPosition.VerticalOffsetFromILinearElement = ?, "
        "   ToPosition.DistanceAlongFromReferent = ?, ToPositionFromReferent.Id = ? "
        "WHERE ECInstanceId = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindDouble(1, GetFromPosition().GetDistanceAlongFromStart());

    if (GetFromPosition().GetLateralOffsetFromILinearElement().IsValid())
        stmtPtr->BindDouble(2, GetFromPosition().GetLateralOffsetFromILinearElement().Value());
    else
        stmtPtr->BindNull(2);

    if (GetFromPosition().GetVerticalOffsetFromILinearElement().IsValid())
        stmtPtr->BindDouble(3, GetFromPosition().GetVerticalOffsetFromILinearElement().Value());
    else
        stmtPtr->BindNull(3);

    if (GetFromPosition().GetDistanceAlongFromReferent().IsValid())
        stmtPtr->BindDouble(4, GetFromPosition().GetDistanceAlongFromReferent().Value());
    else
        stmtPtr->BindNull(4);

    if (GetFromPosition().GetFromReferentId().IsValid())
        stmtPtr->BindId(5, GetFromPosition().GetFromReferentId());
    else
        stmtPtr->BindNull(5);

    stmtPtr->BindDouble(6, GetToPosition().GetDistanceAlongFromStart());

    if (GetToPosition().GetLateralOffsetFromILinearElement().IsValid())
        stmtPtr->BindDouble(7, GetToPosition().GetLateralOffsetFromILinearElement().Value());
    else
        stmtPtr->BindNull(7);

    if (GetToPosition().GetVerticalOffsetFromILinearElement().IsValid())
        stmtPtr->BindDouble(8, GetToPosition().GetVerticalOffsetFromILinearElement().Value());
    else
        stmtPtr->BindNull(8);

    if (GetToPosition().GetDistanceAlongFromReferent().IsValid())
        stmtPtr->BindDouble(9, GetToPosition().GetDistanceAlongFromReferent().Value());
    else
        stmtPtr->BindNull(9);

    if (GetToPosition().GetFromReferentId().IsValid())
        stmtPtr->BindId(10, GetToPosition().GetFromReferentId());
    else
        stmtPtr->BindNull(10);

    stmtPtr->BindId(11, GetAspectInstanceId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedFromToLocation::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT FromPosition.DistanceAlongFromStart, FromPosition.LateralOffsetFromILinearElement, FromPosition.VerticalOffsetFromILinearElement,"
        "   FromPosition.DistanceAlongFromReferent, FromPositionFromReferent.Id, "
        "   ToPosition.DistanceAlongFromStart, ToPosition.LateralOffsetFromILinearElement, ToPosition.VerticalOffsetFromILinearElement,"
        "   ToPosition.DistanceAlongFromReferent, ToPositionFromReferent.Id "
        "FROM " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " "
        "WHERE ECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetAspectInstanceId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadArg;

    m_fromPosition = DistanceExpression(
        stmtPtr->GetValueDouble(0),
        stmtPtr->IsValueNull(1) ? NullableDouble() : stmtPtr->GetValueDouble(1),
        stmtPtr->IsValueNull(2) ? NullableDouble() : stmtPtr->GetValueDouble(2),
        stmtPtr->IsValueNull(4) ? DgnElementId() : stmtPtr->GetValueId<DgnElementId>(4),
        stmtPtr->IsValueNull(3) ? NullableDouble() : stmtPtr->GetValueDouble(3));

    m_toPosition = DistanceExpression(
        stmtPtr->GetValueDouble(5),
        stmtPtr->IsValueNull(6) ? NullableDouble() : stmtPtr->GetValueDouble(6),
        stmtPtr->IsValueNull(7) ? NullableDouble() : stmtPtr->GetValueDouble(7),
        stmtPtr->IsValueNull(9) ? DgnElementId() : stmtPtr->GetValueId<DgnElementId>(9),
        stmtPtr->IsValueNull(8) ? NullableDouble() : stmtPtr->GetValueDouble(8));

    m_originalFromPosition = m_fromPosition;
    m_originalToPosition = m_toPosition;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedFromToLocation::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 == strcmp("FromPosition", propertyName))
        {
        if (BentleyStatus::SUCCESS == GetECValue(value, GetFromPosition(), propertyName))
            return DgnDbStatus::Success;
        }

    if (0 == strcmp("ToPosition", propertyName))
        {
        if (BentleyStatus::SUCCESS == GetECValue(value, GetToPosition(), propertyName))
            return DgnDbStatus::Success;
        }

    return DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus LinearlyReferencedFromToLocation::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 == strcmp("FromPosition", propertyName))
        {
        if (BentleyStatus::SUCCESS == SetDistanceExpressionValue(GetFromPositionR(), propertyName, value))
            return DgnDbStatus::Success;
        }

    if (0 == strcmp("ToPosition", propertyName))
        {
        if (BentleyStatus::SUCCESS == SetDistanceExpressionValue(GetToPositionR(), propertyName, value))
            return DgnDbStatus::Success;
        }

    return DgnDbStatus::BadRequest;
    }