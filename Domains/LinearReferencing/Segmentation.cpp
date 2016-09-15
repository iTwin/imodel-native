/*--------------------------------------------------------------------------------------+
|
|     $Source: Segmentation.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearSegment> ISegmentableLinearElement::_QuerySegments(NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong) const
    {
    Utf8String ecSql(
        "SELECT LinearlyLocated.ECInstanceId, AtLocation.AtPosition.DistanceAlongFromStart, "
            "FromToLocation.FromPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart FROM "
        "(SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " WHERE ILinearElement = ?) LinearlyLocated, "
        BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " AtLocation, "
        BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " FromToLocation "
        "WHERE (LinearlyLocated.ECInstanceId = AtLocation.ElementId ");

    bvector<NullableDouble> bindVals;
    if (fromDistanceAlong.IsValid() && toDistanceAlong.IsValid())
        {
        ecSql.append(
            "AND AtLocation.AtPosition.DistanceAlongFromStart >= ? AND AtLocation.AtPosition.DistanceAlongFromStart <= ? ) "
            "OR (LinearlyLocated.ElementId = FromToLocation AND "
            "((FromToLocation.FromPosition.DistanceAlongFromStart >= ? AND FromToLocation.FromPosition.DistanceAlongFromStart <= ?) "
            " OR (FromToLocation.ToPosition.DistanceAlongFromStart >= ? AND FromToLocation.ToPosition.DistanceAlongFromStart <= ?))) ");

        bindVals.push_back(fromDistanceAlong); bindVals.push_back(toDistanceAlong);
        bindVals.push_back(fromDistanceAlong); bindVals.push_back(toDistanceAlong);
        bindVals.push_back(fromDistanceAlong); bindVals.push_back(toDistanceAlong);
        }
    else if (fromDistanceAlong.IsNull() && toDistanceAlong.IsNull())
        {
        ecSql.append(") OR (LinearlyLocated.ECInstanceId = FromToLocation.ElementId) ");
        }
    else if (fromDistanceAlong.IsValid())
        {
        ecSql.append(
            "AND AtLocation.AtPosition.DistanceAlongFromStart >= ?) "
            "OR (LinearlyLocated.ECInstanceId = FromToLocation.ElementId AND "
                "(FromToLocation.FromPosition.DistanceAlongFromStart >= ? OR FromToLocation.ToPosition.DistanceAlongFromStart >= ?)) ");

        bindVals.push_back(fromDistanceAlong);
        bindVals.push_back(fromDistanceAlong);
        bindVals.push_back(fromDistanceAlong);
        }
    else if (toDistanceAlong.IsValid())
        {
        ecSql.append(
            "AND AtLocation.AtPosition.DistanceAlongFromStart <= ? ) "
            "OR (LinearlyLocated.ECInstanceId = FromToLocation.ElementId AND "
                "(FromToLocation.FromPosition.DistanceAlongFromStart <= ? OR FromToLocation.ToPosition.DistanceAlongFromStart <= ?)) ");

        bindVals.push_back(toDistanceAlong);
        bindVals.push_back(toDistanceAlong);
        bindVals.push_back(toDistanceAlong);
        }

    //ecSql.append("ORDER BY coalesce(AtLocation.AtPosition.DistanceAlongFromStart, FromToLocation.FromPosition.DistanceAlongFromStart);");

    auto& dgnDbR = ToElement().GetDgnDb();
    auto stmtPtr = dgnDbR.GetPreparedECSqlStatement(ecSql.c_str());
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, ToElement().GetElementId());

    int idx = 2;
    for (auto val : bindVals)
        {
        stmtPtr->BindDouble(idx++, val.Value());
        }

    bvector<LinearSegment> retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        {
        auto dgnElementCPtr = dgnDbR.Elements().GetElement(stmtPtr->GetValueId<DgnElementId>(0));
        auto linearlyLocatedCP = dynamic_cast<ILinearlyLocatedCP>(dgnElementCPtr.get());
        BeAssert(linearlyLocatedCP != nullptr);

        double start, stop;
        if (stmtPtr->IsValueNull(1))
            {
            start = stmtPtr->GetValueDouble(2);
            stop = stmtPtr->GetValueDouble(3);
            }
        else
            {
            start = stmtPtr->GetValueDouble(1);
            stop = start;
            }

        retVal.push_back(LinearSegment(*linearlyLocatedCP, start, stop));
        }

    return retVal;
    }