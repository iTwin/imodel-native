/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECQuantityFormatting.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ECQuantityFormatting.h>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
// static
Utf8String ECQuantityFormatting::FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat)
    {
    Utf8String str;
    ECQuantityFormattingStatus locStat = ECQuantityFormattingStatus::Success;
    if (nullptr == formatStatus) 
        formatStatus = &locStat;

    *formatStatus = ECQuantityFormattingStatus::Success;

    // check compatibility of Quantity and KOQ
    if (ECUnit::AreCompatible(qty.GetUnit(), &presentationUnit))
        str = BEF::NumericFormatSpec::StdFormatQuantity(formatSpec, qty.ConvertTo(&presentationUnit));
    else
        *formatStatus = ECQuantityFormattingStatus::IncompatibleKOQ;

    return str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
// static
Utf8String ECQuantityFormatting::FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat)
    {
    if (nullptr == defFormat) defFormat = &BEF::NumericFormatSpec::DefaultFormat();
    *formatStatus = ECQuantityFormattingStatus::Success;

    if (nullptr == koq)
        return defFormat->FormatDouble(qty.GetMagnitude());
    
    Formatting::FormatUnitSet fus = koq->GetDefaultPresentationUnit();
    if (ECUnit::AreCompatible(qty.GetUnit(), fus.GetUnit()))
        return fus.FormatQuantity(qty, nullptr);

    *formatStatus = ECQuantityFormattingStatus::IncompatibleKOQ;
    return defFormat->FormatDouble(qty.GetMagnitude());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, KindOfQuantityCP koq, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    return FormatQuantity(BEU::Quantity(dval, *koq->GetPersistenceUnit().GetUnit()), koq, status, defFormat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    return FormatQuantity(BEU::Quantity(dval, *koq->GetPersistenceUnit().GetUnit()), koq, presentationUnit, formatSpec, status, defFormat);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
