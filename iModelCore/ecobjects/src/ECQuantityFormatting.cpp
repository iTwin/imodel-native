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
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatQuantity(BEU::QuantityCR qty, BEF::FormatUnitGroupCP fug, size_t indx, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat)
    {
    Utf8String str;
    ECQuantityFormattingStatus locStat = ECQuantityFormattingStatus::Success;
    if (nullptr == formatStatus) formatStatus = &locStat;
    if (nullptr == defFormat) defFormat = BEF::NumericFormatSpec::DefaultFormat();
    *formatStatus = ECQuantityFormattingStatus::Success;
    Formatting::FormatUnitSetCP fusP = (nullptr == fug) ? nullptr : fug->GetPresentationFUS(indx);

    if (nullptr == fusP) // FUG does not yeild FUS - will be using default NumericFormatSpec: BEF::NumericFormatSpecCP defFormat
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


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatPersistedValue(double dval, BEF::FormatUnitGroupCP fug, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat)
    {
    Formatting::FormatUnitSetCR persistFUS = fug->GetPersistenceFUS();
    BEU::UnitCP unit = persistFUS.GetUnit();
    BEU::Quantity q = BEU::Quantity(dval, *unit);
    return FormatQuantity(q, fug, indx, status, defFormat);
    }



//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/17
//----------------------------------------------------------------------------------------
//Utf8String ECQuantityFormatting::FormatQuantityMagnitude(double dval, Utf8StringCR UnitName, Formatting::FormatUnitSetCR, ECQuantityFormattingStatus* status)
//    {
//    BEU::Quantity q = BEU::Quantity(dval, persistenceUnitName);
//    return FormatQuantity(q, fug, indx, status, defFormat);
//    }




//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Json::Value ECQuantityFormatting::FormatQuantityJson(BEU::QuantityCR qty, KindOfQuantityCP koq, size_t indx, bool useAlias )
    {
     Formatting::FormatUnitSet fus = (nullptr == koq)? BEF::StdFormatSet::DefaultFUS(qty) : koq->GetPresentationFUS(indx);
     return fus.FormatQuantityJson(qty, useAlias);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
Json::Value ECQuantityFormatting::FormatQuantityJson(BEU::QuantityCR qty, BEF::FormatUnitGroupCP fug, size_t indx, bool useAlias)
    {
    Formatting::FormatUnitSet fus = (nullptr == fug) ? BEF::StdFormatSet::DefaultFUS(qty) : fug->GetPresentationFUS(indx);
    return fus.FormatQuantityJson(qty, useAlias);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//   The quantity is created from the text string. it's consistency with the KOQ of the specific
//     context must be checked by the caller. When multiple Units are being used the Quantity Unit
//   will be the "biggest", but in the first implementaiton the biggest is assumed to be the leftmost
//----------------------------------------------------------------------------------------
BEU::Quantity ECQuantityFormatting::CreateQuantity(Utf8CP input, size_t start, Utf8CP unitName)
    {
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, start, unitName);
    BEU::Quantity qty = fps.GetQuantity();
    return qty;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
BEU::Quantity ECQuantityFormatting::CreateQuantity(Utf8CP input, size_t start, double* persist, KindOfQuantityCP koq, size_t indx, Formatting::FormatProblemCode* probCode)
    {
    Formatting::FormatUnitSetCR persistFUS = koq->GetPersistenceUnit();
    BEU::UnitCP persUnit = persistFUS.GetUnit();
    Formatting::FormatUnitSetCP fusP = (nullptr == koq) ? nullptr : koq->GetPresentationFUS(indx);
    BEU::UnitCP unit = (nullptr == fusP) ? persUnit : fusP->GetUnit();
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, start, unit);
    Formatting::FormatProblemCode locCode;
    if (nullptr == probCode) probCode = &locCode;
    *probCode = Formatting::FormatProblemCode::NoProblems;
    BEU::Quantity qty = fps.GetQuantity(probCode);
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
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
BEU::Quantity ECQuantityFormatting::CreateQuantity(Utf8CP input, size_t start, double* persist, BEF::FormatUnitGroupCP fug, size_t indx, Formatting::FormatProblemCode* probCode)
    {
    Formatting::FormatUnitSetCR persistFUS = fug->GetPersistenceFUS();
    BEU::UnitCP persUnit = persistFUS.GetUnit();
    Formatting::FormatUnitSetCP fusP = (nullptr == fug) ? nullptr : fug->GetPresentationFUS(indx);
    BEU::UnitCP unit = (nullptr == fusP) ? persUnit : fusP->GetUnit();
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, start, unit);
    Formatting::FormatProblemCode locCode;
    if (nullptr == probCode) probCode = &locCode;
    *probCode = Formatting::FormatProblemCode::NoProblems;
    BEU::Quantity qty = fps.GetQuantity(probCode);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  10/2017
//---------------------------------------------------------------------------------------
BEF::FormatUnitGroup ECQuantityFormatting::CreateFUGfromKOQ(KindOfQuantityCR koq)
    {
    Formatting::FormatUnitSetCR persistenceFus = koq.GetPersistenceUnit();
    Utf8String fusListString = persistenceFus.ToText(true);
    fusListString.append(" ");
    bool first = true;

    for (Formatting::FormatUnitSetCR fus : koq.GetPresentationUnitList())
        {
        if (fus.HasProblem())
            {
            LOG.errorv(" KindOfQuantity '%s' has problem: '%s'", koq.GetName().c_str(), fus.GetProblemDescription().c_str());
            continue;
            }
        if (!first)
            fusListString.append (",");
        first = false;
        fusListString += fus.ToText(true);
        }

    return BEF::FormatUnitGroup(koq.GetFullName().c_str(), fusListString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
Utf8String ECQuantityFormatting::FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat)
    {
    Utf8String str;
    ECQuantityFormattingStatus locStat = ECQuantityFormattingStatus::Success;
    if (nullptr == formatStatus) formatStatus = &locStat;
    if (nullptr == defFormat) defFormat = BEF::NumericFormatSpec::DefaultFormat();
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
BEU::Quantity ECQuantityFormatting::CreateQuantity(Utf8CP input, size_t start, double* persist, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, Formatting::FormatProblemCode* probCode)
    {
    Formatting::FormatUnitSetCR persistFUS = koq->GetPersistenceUnit();
    BEU::UnitCP persUnit = persistFUS.GetUnit();
    Formatting::FormatParsingSet fps = Formatting::FormatParsingSet(input, start, &presentationUnit);
    Formatting::FormatProblemCode locCode;
    if (nullptr == probCode) probCode = &locCode;
    *probCode = Formatting::FormatProblemCode::NoProblems;
    BEU::Quantity qty = fps.GetQuantity(probCode);
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



END_BENTLEY_ECOBJECT_NAMESPACE