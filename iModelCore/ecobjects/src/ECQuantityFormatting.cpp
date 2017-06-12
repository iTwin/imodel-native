/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECQuantityFormatting.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
//#include <Formatting/FormattingApi.h>
//#include <ECObjects/ECSchema.h>
#include <ECObjects/ECQuantityFormatting.h>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

Utf8String ECQuantityFormatting::FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, size_t indx, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat)
    {
    Utf8String str;
    ECQuantityFormattingStatus locStat = ECQuantityFormattingStatus::Success;
    if (nullptr == formatStatus) formatStatus = &locStat;
    if (nullptr == defFormat) defFormat = BEF::NumericFormatSpec::DefaultFormat();
    *formatStatus = ECQuantityFormattingStatus::Success;
    Formatting::FormatUnitSetCP fusP = (nullptr == koq) ? nullptr : koq->GetPresentationFUS(indx);

    if (nullptr == fusP) // KOQ does not yeild FUS - will be using default NumericFormatSpec: BEF::NumericFormatSpecCP defFormat
        {
        str = defFormat->FormatDouble(qty.GetMagnitude());
        }
    else
        {
        // check compatibility of Quantity and KOQ
        if (Formatting::Utils::AreUnitsComparable(qty.GetUnit(), fusP->GetUnit()))
            {
            str = fusP->FormatQuantity(qty, nullptr);
            }
        else
            {
            str = defFormat->FormatDouble(qty.GetMagnitude());
            *formatStatus = ECQuantityFormattingStatus::IncompatibleKOQ;
            }
        }
    return str;
    }

Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, KindOfQuantityCP koq, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    Formatting::FormatUnitSetCR persistFUS = koq->GetPersistenceUnit();
    BEU::UnitCP unit = persistFUS.GetUnit();
    BEU::Quantity q = BEU::Quantity(dval, *unit);
    return FormatQuantity(q, koq, indx, status, defFormat);
    }

Json::Value ECQuantityFormatting::FormatQuantityJson(BEU::QuantityCR qty, KindOfQuantityCP koq, size_t indx, bool useAlias )
    {
     Formatting::FormatUnitSet fus = (nullptr == koq)? BEF::StdFormatSet::DefaultFUS(qty) : koq->GetPresentationFUS(indx);
     return fus.FormatQuantityJson(qty, useAlias);
    }

END_BENTLEY_ECOBJECT_NAMESPACE