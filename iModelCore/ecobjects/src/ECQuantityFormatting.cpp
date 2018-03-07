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
Utf8String ECQuantityFormatting::FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat)
    {
    Utf8String str;
    ECQuantityFormattingStatus locStat = ECQuantityFormattingStatus::Success;
    if (nullptr == formatStatus) formatStatus = &locStat;
    if (nullptr == defFormat) defFormat = BEF::NumericFormatSpec::DefaultFormat();
    *formatStatus = ECQuantityFormattingStatus::Success;

    if (nullptr == koq)
        {
        str = defFormat->FormatDouble(qty.GetMagnitude());
        }
    else
        {
        Formatting::FormatUnitSet fus = koq->GetDefaultPresentationUnit();
        if (Units::Unit::AreCompatible(qty.GetUnit(), fus.GetUnit()))
            {
            str = fus.FormatQuantity(qty, nullptr);
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
Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, KindOfQuantityCP koq, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    Formatting::FormatUnitSetCR persistFUS = koq->GetPersistenceUnit();
    BEU::UnitCP unit = persistFUS.GetUnit();
    BEU::Quantity q = BEU::Quantity(dval, *unit);
    return FormatQuantity(q, koq, status, defFormat);
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
    return Formatting::QuantityFormatting::CreateQuantity(input, start, persist, persistFUS, presentationFUS, probCode);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//   The quantity is created from the text string. it's consistency with the KOQ of the specific
//     context must be checked by the caller. When multiple Units are being used the Quantity Unit
//   will be the "biggest", but in the first implementaiton the biggest is assumed to be the leftmost
//----------------------------------------------------------------------------------------
BEU::Quantity ECQuantityFormatting::CreateQuantity(Utf8CP input, size_t start, Formatting::FormatUnitSetCR fus, Formatting::FormatProblemCode* probCode)
    {
    return Formatting::QuantityFormatting::CreateQuantity(input, start, fus, probCode);
    }

END_BENTLEY_ECOBJECT_NAMESPACE