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
#include <ECObjects/CalculatedProperty.h>
#include <ECObjects/SchemaLocalizedStrings.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/BeFileName.h>
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
    ECOBJECTS_EXPORT static Utf8String FormatPersistedValue(double dval, KindOfQuantityCP koq, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
    ECOBJECTS_EXPORT static Utf8String FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat=nullptr);
    ECOBJECTS_EXPORT static Json::Value FormatQuantityJson(BEU::QuantityCR qty, KindOfQuantityCP koq, size_t indx, bool useAlias=true);
    ECOBJECTS_EXPORT ECValue static GetQuantityValue(BEU::QuantityCR qty, UnitCP useUnit = nullptr);

    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, size_t start, Utf8CP unitName = nullptr);
    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, size_t start, Formatting::FormatUnitSetCR fus);
    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, size_t start, double* persist = nullptr, KindOfQuantityCP koq = nullptr, size_t indx = 0, Formatting::FormatProblemCode* prob = nullptr);

    //! Create FormatUnitGroup from a KindOfQuantity. Use FormatUnitGroup::HasProblem to test returned FormatUnitGroup.
    ECOBJECTS_EXPORT static BEF::FormatUnitGroup CreateFUGfromKOQ(KindOfQuantityCR koq);

    // functions below are using FormatUnitGroup as a "surrogate" of KOQ. creating KOQ's on the fly is problematic since each KOQ must belong to some schema\
    //   FormatUnitGroup's can be easily created for various purposes when only the essential Unit/Format parts of the KOQ are
    //    actually needed
    ECOBJECTS_EXPORT static Utf8String FormatPersistedValue(double dval, BEF::FormatUnitGroupCP fug, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
    ECOBJECTS_EXPORT static Utf8String FormatQuantity(BEU::QuantityCR qty, BEF::FormatUnitGroupCP fug, size_t indx, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
    ECOBJECTS_EXPORT static Json::Value FormatQuantityJson(BEU::QuantityCR qty, BEF::FormatUnitGroupCP fug, size_t indx, bool useAlias = true);
    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, size_t start, double* persist = nullptr, BEF::FormatUnitGroupCP fug = nullptr, size_t indx = 0, Formatting::FormatProblemCode* prob = nullptr);

    ECOBJECTS_EXPORT static BEU::Quantity CreateQuantity(Utf8CP input, size_t start, double* persist, KindOfQuantityCP koq, Formatting::FormatUnitSetR presentationFUS, Formatting::FormatProblemCode* probCode);
    ECOBJECTS_EXPORT static Utf8String FormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* formatStatus, BEF::NumericFormatSpecCP defFormat = nullptr);
    ECOBJECTS_EXPORT static Utf8String FormatPersistedValue(double dval, KindOfQuantityCP koq, BEU::UnitCR presentationUnit, BEF::NamedFormatSpecCR formatSpec, ECQuantityFormattingStatus* status, BEF::NumericFormatSpecCP defFormat = nullptr);
    };

END_BENTLEY_ECOBJECT_NAMESPACE