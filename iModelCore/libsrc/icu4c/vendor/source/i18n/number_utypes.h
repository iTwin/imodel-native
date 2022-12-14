// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING
#ifndef __SOURCE_NUMBER_UTYPES_H__
#define __SOURCE_NUMBER_UTYPES_H__

#include "unicode/numberformatter.h"
#include "number_types.h"
#include "number_decimalquantity.h"
#include "number_stringbuilder.h"
#include "formattedval_impl.h"

U_NAMESPACE_BEGIN namespace number {
namespace impl {


/** Helper function used in upluralrules.cpp */
const DecimalQuantity* validateUFormattedNumberToDecimalQuantity(
    const UFormattedNumber* uresult, UErrorCode& status);


/**
 * Struct for data used by FormattedNumber.
 *
 * This struct is held internally by the C++ version FormattedNumber since the member types are not
 * declared in the public header file.
 *
 * The DecimalQuantity is not currently being used by FormattedNumber, but at some point it could be used
 * to add a toDecNumber() or similar method.
 */
class UFormattedNumberData : public FormattedValueNumberStringBuilderImpl {
public:
    UFormattedNumberData() : FormattedValueNumberStringBuilderImpl(0) {}
    virtual ~UFormattedNumberData();

    DecimalQuantity quantity;
};


} // namespace impl
} // namespace number
U_NAMESPACE_END

#endif //__SOURCE_NUMBER_UTYPES_H__
#endif /* #if !UCONFIG_NO_FORMATTING */
