/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ECQuantityFormatting.h>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
// static
Utf8String ECQuantityFormatting::FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::FormatCR formatSpec, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat)
    {
    Utf8String str;
    ECQuantityFormattingStatus locStat = ECQuantityFormattingStatus::Success;
    if (nullptr == formatStatus) 
        formatStatus = &locStat;

    *formatStatus = ECQuantityFormattingStatus::Success;

    // check compatibility of Quantity and KOQ
    if (ECUnit::AreCompatible(qty.GetUnit(), &presentationUnit))
        str = formatSpec.FormatQuantity(qty.ConvertTo(&presentationUnit));
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
        return defFormat->Format(qty.GetMagnitude());
    
    NamedFormatCP format = koq->GetDefaultPresentationFormat();
    if (nullptr != format && (!format->HasCompositeMajorUnit() || ECUnit::AreCompatible(qty.GetUnit(), format->GetCompositeSpec()->GetMajorUnit())))
        return format->FormatQuantity(qty, nullptr);

    *formatStatus = ECQuantityFormattingStatus::IncompatibleKOQ;
    return defFormat->Format(qty.GetMagnitude());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, KindOfQuantityCP koq, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    return FormatQuantity(BEU::Quantity(dval, *koq->GetPersistenceUnit()), koq, status, defFormat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::FormatCR formatSpec, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    return FormatQuantity(BEU::Quantity(dval, *koq->GetPersistenceUnit()), koq, presentationUnit, formatSpec, status, defFormat);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
