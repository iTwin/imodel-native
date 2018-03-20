/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECQuantityFormatting.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <Formatting/FormattingApi.h>

namespace BEU = BentleyApi::Units;
namespace BEF = BentleyApi::Formatting;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
//! Status code used for Quantity formatting
//=======================================================================================
enum class ECQuantityFormattingStatus
    {
    Success = 0,
    IncompatibleKOQ = 1,
    InvalidKOQ = 2
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct ECQuantityFormatting
{
private:
    static Utf8String FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
public:
    ECOBJECTS_EXPORT static Utf8String FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat = nullptr);
    ECOBJECTS_EXPORT static Utf8String FormatPersistedValue(double dval, KindOfQuantityCP koq, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
    ECOBJECTS_EXPORT static Utf8String FormatPersistedValue(double dval, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);

    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, double* persist, KindOfQuantityCP koq, Formatting::FormatUnitSetCR presentationFUS, Formatting::FormatProblemCode* probCode)
        {return Formatting::QuantityFormatting::CreateQuantity(input, persist, koq->GetPersistenceUnit(), presentationFUS, probCode);}

    //! The quantity is created from the text string. It's consistency with the KOQ of the specific context must 
    //! be checked by the caller. When multiple Units are being used the Quantity Unit will be the "biggest",
    //! but in the first implementaiton the biggest is assumed to be the leftmost
    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, Formatting::FormatUnitSetCR fus, Formatting::FormatProblemCode* probCode)
        {return Formatting::QuantityFormatting::CreateQuantity(input, fus, probCode);}
};

END_BENTLEY_ECOBJECT_NAMESPACE