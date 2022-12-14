// © 2019 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/numberformatter.h"
#include "number_utypes.h"
#include "util.h"
#include "number_decimalquantity.h"

U_NAMESPACE_BEGIN
namespace number {


UPRV_FORMATTED_VALUE_SUBCLASS_AUTO_IMPL(FormattedNumber)

#define UPRV_NOARG

UBool FormattedNumber::nextFieldPosition(FieldPosition& fieldPosition, UErrorCode& status) const {
    UPRV_FORMATTED_VALUE_METHOD_GUARD(FALSE)
    // NOTE: MSVC sometimes complains when implicitly converting between bool and UBool
    return fData->getStringRef().nextFieldPosition(fieldPosition, status) ? TRUE : FALSE;
}

void FormattedNumber::getAllFieldPositions(FieldPositionIterator& iterator, UErrorCode& status) const {
    FieldPositionIteratorHandler fpih(&iterator, status);
    getAllFieldPositionsImpl(fpih, status);
}

void FormattedNumber::getAllFieldPositionsImpl(FieldPositionIteratorHandler& fpih,
                                               UErrorCode& status) const {
    UPRV_FORMATTED_VALUE_METHOD_GUARD(UPRV_NOARG)
    fData->getStringRef().getAllFieldPositions(fpih, status);
}

void FormattedNumber::getDecimalQuantity(impl::DecimalQuantity& output, UErrorCode& status) const {
    UPRV_FORMATTED_VALUE_METHOD_GUARD(UPRV_NOARG)
    output = fData->quantity;
}


impl::UFormattedNumberData::~UFormattedNumberData() = default;


} // namespace number
U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_FORMATTING */
