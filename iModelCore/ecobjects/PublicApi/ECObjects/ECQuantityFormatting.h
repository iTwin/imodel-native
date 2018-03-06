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



enum class ECQuantityFormattingStatus
    {
    Success = 0,
    IncompatibleKOQ = 1,
    InvalidKOQ = 2
    };

struct ECQuantityFormatting
    {
private:
    ECOBJECTS_EXPORT static Utf8String FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
public:
    ECOBJECTS_EXPORT static Utf8String FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat = nullptr);
    ECOBJECTS_EXPORT static Utf8String FormatPersistedValue(double dval, KindOfQuantityCP koq, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
    ECOBJECTS_EXPORT static Utf8String FormatPersistedValue(double dval, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
    
    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, size_t start, double* persist, KindOfQuantityCP koq, Formatting::FormatUnitSetR presentationFUS, Formatting::FormatProblemCode* probCode);
    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, size_t start, Formatting::FormatUnitSetCR fus);
    };

END_BENTLEY_ECOBJECT_NAMESPACE