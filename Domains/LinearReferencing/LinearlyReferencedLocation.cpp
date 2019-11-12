/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/LinearlyReferencedLocation.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocation::LinearlyReferencedLocation()
    {
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
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpression LinearlyReferencedAtLocation::ToAtDistanceExpression(ECN::IECInstanceCR instance)
    {
    DistanceExpression retVal;

    ECValue ecVal;
    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "AtPosition.DistanceAlongFromStart"))
        retVal.SetDistanceAlongFromStart(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "AtPosition.LateralOffsetFromILinearElement") && !ecVal.IsNull())
        retVal.SetLateralOffsetFromILinearElement(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "AtPosition.VerticalOffsetFromILinearElement") && !ecVal.IsNull())
        retVal.SetVerticalOffsetFromILinearElement(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "AtPosition.DistanceAlongFromReferent") && !ecVal.IsNull())
        retVal.SetDistanceAlongFromReferent(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "FromReferent.Id") && !ecVal.IsNull())
        retVal.m_fromReferentId = DgnElementId(static_cast<uint64_t>(ecVal.GetLong()));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr LinearlyReferencedAtLocation::_ToECInstance(DgnDbR dgnDb) const
    {
    if (m_instancePtr.IsValid())
        return m_instancePtr;

    auto ecInstancePtr = dgnDb.Schemas().GetClass(QueryClassId(dgnDb))->GetDefaultStandaloneEnabler()->CreateInstance();
    PopulateInstance(*ecInstancePtr);
    return ecInstancePtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void LinearlyReferencedAtLocation::PopulateInstance(IECInstanceR instance) const
    {
    instance.SetValue("AtPosition.DistanceAlongFromStart", ECValue(GetAtPosition().GetDistanceAlongFromStart()));

    if (GetAtPosition().GetLateralOffsetFromILinearElement().IsValid())
        instance.SetValue("AtPosition.LateralOffsetFromILinearElement", ECValue(GetAtPosition().GetLateralOffsetFromILinearElement().Value()));

    if (GetAtPosition().GetVerticalOffsetFromILinearElement().IsValid())
        instance.SetValue("AtPosition.VerticalOffsetFromILinearElement", ECValue(GetAtPosition().GetVerticalOffsetFromILinearElement().Value()));

    if (GetAtPosition().GetDistanceAlongFromReferent().IsValid())
        instance.SetValue("AtPosition.DistanceAlongFromReferent", ECValue(GetAtPosition().GetDistanceAlongFromReferent().Value()));

    if (GetAtPosition().GetFromReferentId().IsValid())
        instance.SetValue("FromReferent.Id", ECValue(GetAtPosition().GetFromReferentId().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void LinearlyReferencedAtLocation::SetAtPosition(DistanceExpressionCR atPosition)
    {
    m_atPosition = atPosition;

    if (m_instancePtr.IsValid())
        {
        PopulateInstance(*m_instancePtr);
        }
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
    m_fromPosition(fromPosition), m_toPosition(toPosition)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void LinearlyReferencedFromToLocation::ToFromToDistanceExpression(IECInstanceCR instance, DistanceExpressionR from, DistanceExpressionR to)
    {
    DistanceExpression retVal;

    ECValue ecVal;
    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "FromPosition.DistanceAlongFromStart"))
        from.SetDistanceAlongFromStart(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "FromPosition.LateralOffsetFromILinearElement") && !ecVal.IsNull())
        from.SetLateralOffsetFromILinearElement(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "FromPosition.VerticalOffsetFromILinearElement") && !ecVal.IsNull())
        from.SetVerticalOffsetFromILinearElement(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "FromPosition.DistanceAlongFromReferent") && !ecVal.IsNull())
        from.SetDistanceAlongFromReferent(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "FromPositionFromReferent.Id") && !ecVal.IsNull())
        from.m_fromReferentId = DgnElementId(static_cast<uint64_t>(ecVal.GetLong()));

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "ToPosition.DistanceAlongFromStart"))
        to.SetDistanceAlongFromStart(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "ToPosition.LateralOffsetFromILinearElement") && !ecVal.IsNull())
        to.SetLateralOffsetFromILinearElement(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "ToPosition.VerticalOffsetFromILinearElement") && !ecVal.IsNull())
        to.SetVerticalOffsetFromILinearElement(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "ToPosition.DistanceAlongFromReferent") && !ecVal.IsNull())
        to.SetDistanceAlongFromReferent(ecVal.GetDouble());

    if (ECObjectsStatus::Success == instance.GetValue(ecVal, "ToPositionFromReferent.Id") && !ecVal.IsNull())
        to.m_fromReferentId = DgnElementId(static_cast<uint64_t>(ecVal.GetLong()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void LinearlyReferencedFromToLocation::PopulateInstance(IECInstanceR instance) const
    {
    instance.SetValue("FromPosition.DistanceAlongFromStart", ECValue(GetFromPosition().GetDistanceAlongFromStart()));

    if (GetFromPosition().GetLateralOffsetFromILinearElement().IsValid())
        instance.SetValue("FromPosition.LateralOffsetFromILinearElement", ECValue(GetFromPosition().GetLateralOffsetFromILinearElement().Value()));

    if (GetFromPosition().GetVerticalOffsetFromILinearElement().IsValid())
        instance.SetValue("FromPosition.VerticalOffsetFromILinearElement", ECValue(GetFromPosition().GetVerticalOffsetFromILinearElement().Value()));

    if (GetFromPosition().GetDistanceAlongFromReferent().IsValid())
        instance.SetValue("FromPosition.DistanceAlongFromReferent", ECValue(GetFromPosition().GetDistanceAlongFromReferent().Value()));

    if (GetFromPosition().GetFromReferentId().IsValid())
        instance.SetValue("FromPositionFromReferent.Id", ECValue(GetFromPosition().GetFromReferentId().GetValue()));

    instance.SetValue("ToPosition.DistanceAlongFromStart", ECValue(GetToPosition().GetDistanceAlongFromStart()));

    if (GetToPosition().GetLateralOffsetFromILinearElement().IsValid())
        instance.SetValue("ToPosition.LateralOffsetFromILinearElement", ECValue(GetToPosition().GetLateralOffsetFromILinearElement().Value()));

    if (GetToPosition().GetVerticalOffsetFromILinearElement().IsValid())
        instance.SetValue("ToPosition.VerticalOffsetFromILinearElement", ECValue(GetToPosition().GetVerticalOffsetFromILinearElement().Value()));

    if (GetToPosition().GetDistanceAlongFromReferent().IsValid())
        instance.SetValue("ToPosition.DistanceAlongFromReferent", ECValue(GetToPosition().GetDistanceAlongFromReferent().Value()));

    if (GetToPosition().GetFromReferentId().IsValid())
        instance.SetValue("ToPositionFromReferent.Id", ECValue(GetToPosition().GetFromReferentId().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr LinearlyReferencedFromToLocation::_ToECInstance(DgnDbR dgnDb) const
    {
    if (m_instancePtr.IsValid())
        return m_instancePtr;

    auto ecInstancePtr = dgnDb.Schemas().GetClass(QueryClassId(dgnDb))->GetDefaultStandaloneEnabler()->CreateInstance();
    PopulateInstance(*ecInstancePtr);
    return ecInstancePtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationPtr LinearlyReferencedFromToLocation::Create(DistanceExpressionCR fromPosition, DistanceExpressionCR toPosition)
    {
    return new LinearlyReferencedFromToLocation(fromPosition, toPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void LinearlyReferencedFromToLocation::SetFromPosition(DistanceExpressionCR position)
    {
    m_fromPosition = position;

    if (m_instancePtr.IsValid())
        {
        PopulateInstance(*m_instancePtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void LinearlyReferencedFromToLocation::SetToPosition(DistanceExpressionCR position)
    {
    m_toPosition = position;

    if (m_instancePtr.IsValid())
        {
        PopulateInstance(*m_instancePtr);
        }
    }