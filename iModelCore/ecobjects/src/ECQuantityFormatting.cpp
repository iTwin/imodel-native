/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECQuantityFormatting.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
//#include <Formatting/FormattingApi.h>
//#include <ECObjects/ECSchema.h>
#include <ECObjects/ECQuantityFormatting.h>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat)
    {
    Utf8String str;
    ECQuantityFormattingStatus locStat = ECQuantityFormattingStatus::Success;
    if (nullptr == formatStatus) formatStatus = &locStat;
    *formatStatus = ECQuantityFormattingStatus::Success;

    // check compatibility of Quantity and KOQ
    if (Formatting::Utils::AreUnitsComparable(qty.GetUnit(), &presentationUnit))
        {
        str = BEF::NumericFormatSpec::StdFormatQuantity(formatSpec, qty.ConvertTo(&presentationUnit));
        }
    else
        {
        *formatStatus = ECQuantityFormattingStatus::IncompatibleKOQ;
        }

    return str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, KindOfQuantityCP koq, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    Formatting::FormatUnitSetCR persistFUS = koq->GetPersistenceUnit();
    BEU::UnitCP unit = persistFUS.GetUnit();
    BEU::Quantity q = BEU::Quantity(dval, *unit);
    return FormatQuantity(q, koq, indx, status, defFormat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    Formatting::FormatUnitSetCR persistFUS = koq->GetPersistenceUnit();
    BEU::UnitCP persistUnit = persistFUS.GetUnit();
    BEU::Quantity q = BEU::Quantity(dval, *persistUnit);
    return FormatQuantity(q, koq, presentationUnit, formatSpec, status, defFormat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
BEU::Quantity ECQuantityFormatting::CreateQuantity(Utf8CP input, size_t start, double* persist, KindOfQuantityCP koq, Formatting::FormatUnitSetR presentationFUS, Formatting::FormatProblemCode* probCode)
    {
    Formatting::FormatUnitSetCR persistFUS = koq->GetPersistenceUnit();
    BEU::UnitCP persUnit = persistFUS.GetUnit();
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, start, presentationFUS.GetUnit());
    Formatting::FormatProblemCode locCode;
    if (nullptr == probCode) probCode = &locCode;
    *probCode = Formatting::FormatProblemCode::NoProblems;
    BEU::Quantity qty = fps.GetQuantity(probCode, &presentationFUS);
    if (*probCode == Formatting::FormatProblemCode::NoProblems)
        {
        if (nullptr != persist)
            {
            BEU::Quantity persQty = qty.ConvertTo(persUnit);
            *persist = persQty.GetMagnitude();
            }
        }
    else if (nullptr != persist)
        *persist = 0.0;

    return qty;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//   The quantity is created from the text string. it's consistency with the KOQ of the specific
//     context must be checked by the caller. When multiple Units are being used the Quantity Unit
//   will be the "biggest", but in the first implementaiton the biggest is assumed to be the leftmost
//----------------------------------------------------------------------------------------
BEU::Quantity ECQuantityFormatting::CreateQuantity(Utf8CP input, size_t start, Formatting::FormatUnitSetCR fus)
    {
    Utf8String unitName = fus.GetUnitName();
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, start, unitName.c_str());
    Formatting::FormatProblemCode prob;
    BEU::Quantity qty = fps.GetQuantity(&prob, &fus);
    return qty;
    }

END_BENTLEY_ECOBJECT_NAMESPACE