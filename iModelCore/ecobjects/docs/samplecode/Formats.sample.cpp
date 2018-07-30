/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/Formats.sample.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Overview_Formats_Include.sampleCode
#include <ECObjects/ECObjectsAPI.h>
#include <ECObjects/ECQuantityFormatting.h>
//__PUBLISH_EXTRACT_END__

USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 07/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CreateFormat()
	{
	//__PUBLISH_EXTRACT_START__ Overview_Formats_CreateFormat.sampleCode
    ECSchemaPtr schema;
    ECFormatP format;
    ECSchema::CreateSchema(schema, "FormatSchema", "format", 5, 0, 6);
    Formatting::NumericFormatSpec spec = Formatting::NumericFormatSpec();
    schema->CreateFormat(format, "TestFormat", "TestDisplayLabel", "TestDescription", &spec);
    //__PUBLISH_EXTRACT_END__
    using namespace Units;
    using namespace Formatting;
    //__PUBLISH_EXTRACT_START__ Overview_Formats_TraitsEnum.sampleCode
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::TrailingZeroes);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::KeepDecimalPoint);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::KeepSingleZero);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::ExponenentOnlyNegative);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::ZeroEmpty);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::Use1000Separator);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::ApplyRounding);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::FractionDash);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::ShowUnitLabel);
    format->GetNumericSpecP()->SetFormatTraits(FormatTraits::PrependUnitLabel);
    //__PUBLISH_EXTRACT_END__

    //__PUBLISH_EXTRACT_START__ Overview_Formats_TraitsApi.sampleCode
    format->GetNumericSpecP()->SetKeepTrailingZeroes(true);
    format->GetNumericSpecP()->SetKeepDecimalPoint(true);
    format->GetNumericSpecP()->SetKeepSingleZero(true);
    format->GetNumericSpecP()->SetExponentOnlyNegative(true);
    format->GetNumericSpecP()->SetZeroEmpty(true);
    format->GetNumericSpecP()->SetUse1000Separator(true);
    format->GetNumericSpecP()->SetApplyRounding(true);
    format->GetNumericSpecP()->SetFractionDash(true);
    format->GetNumericSpecP()->SetShowUnitLabel(true);
    format->GetNumericSpecP()->SetPrependUnitLabel(true);
    //__PUBLISH_EXTRACT_END__

    return BentleyStatus::SUCCESS;
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 07/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus FormatQuantity()
    {
    ECSchemaPtr schema;
    ECFormatP format;
    Units::UnitCP unit = nullptr;
    KindOfQuantityCP koq = nullptr;
    ECSchema::CreateSchema(schema, "FormatSchema", "format", 5, 0, 6);
    Formatting::NumericFormatSpec spec = Formatting::NumericFormatSpec();
    schema->CreateFormat(format, "TestFormat", "TestDisplayLabel", "TestDescription", &spec);
    //__PUBLISH_EXTRACT_START__ Overview_Formats_CreateQuantity.sampleCode
    Formatting::FormatProblemCode code;  
    // From a string
    BEU::Quantity qty = ECN::ECQuantityFormatting::CreateQuantity("1000 M", *format, &code);
    // From a unit
    qty = BEU::Quantity(1000.0, *unit);
    // Format the quantity
    auto formatted = ECN::ECQuantityFormatting::FormatQuantity(qty, koq, *unit, *format, nullptr);
    //__PUBLISH_EXTRACT_END__
    return BentleyStatus::SUCCESS;
    }