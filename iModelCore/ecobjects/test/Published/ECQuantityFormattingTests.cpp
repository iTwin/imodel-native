/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include "ECObjects/ECQuantityFormatting.h"
#include <Formatting/FormattingApi.h>
#include <BeSQLite/L10N.h>

namespace BEU = BentleyApi::Units;
namespace BEF = BentleyApi::Formatting;

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECQuantityFormattingTest : ECTestFixture {};

static void ShowQuantifiedValue(Utf8CP input, Utf8CP formatName, Utf8CP fusUnit, Utf8CP spacer=nullptr)
    {
    ECUnitCP unit = ECTestFixture::GetUnitsSchema()->GetUnitCP(fusUnit);
    ECFormatCP format = ECTestFixture::GetFormatsSchema()->GetFormatCP(formatName);

    ECFormatCP real4u = ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU");

    NamedFormatP namedFormat = nullptr;
    NamedFormatP real4uFormat = nullptr;
    if (nullptr != unit)
        {
        namedFormat = new NamedFormat(format->GetName() + "[" + unit->GetName().c_str() + "]", format);
        if (!namedFormat->HasComposite())
            {
            Formatting::CompositeValueSpec compositeSpec = Formatting::CompositeValueSpec(*unit);
            EXPECT_FALSE(compositeSpec.IsProblem()) << "Composite spec of " << formatName << " has problem " << compositeSpec.GetProblemDescription().c_str();
            if (compositeSpec.IsProblem())
                return;

            namedFormat->SetCompositeSpec(compositeSpec);
            EXPECT_FALSE(compositeSpec.IsProblem()) << "NamedFormat " << namedFormat->GetName().c_str() << " has problem " << compositeSpec.GetProblemDescription().c_str();
            if (compositeSpec.IsProblem())
                return;
            }

        real4uFormat = namedFormat = new NamedFormat(real4u->GetName() + "[" + unit->GetName().c_str() + "]", real4u);
        real4uFormat->GetNumericSpecP()->SetPrecision(Formatting::DecimalPrecision::Precision4);
        if (!real4uFormat->HasComposite())
            {
            Formatting::CompositeValueSpec compositeSpec(*unit);
            EXPECT_FALSE(compositeSpec.IsProblem()) << "Composite spec of " << formatName << " has problem " << compositeSpec.GetProblemDescription().c_str();
            if (compositeSpec.IsProblem())
                return;

            real4uFormat->SetCompositeSpec(compositeSpec);
            EXPECT_FALSE(compositeSpec.IsProblem()) << "NamedFormat " << real4uFormat->GetName().c_str() << " has problem " << compositeSpec.GetProblemDescription().c_str();
            if (compositeSpec.IsProblem())
                return;
            }
        }
    else
        {
        namedFormat = const_cast<ECFormatP>(format);
        real4uFormat = const_cast<ECFormatP>(real4u);
        }

    Formatting::FormatProblemCode code;
    BEU::Quantity qty = ECQuantityFormatting::CreateQuantity(input, *namedFormat, &code);
    Utf8String qtyT = namedFormat->FormatQuantity(qty, spacer);
    Utf8String qtyT0 = real4uFormat->FormatQuantity(qty, spacer);

    EXPECT_STREQ(qtyT.c_str(), qtyT0.c_str()) << "Input: |" << input << "| Quantity: " << qtyT.c_str() << "  Equivalent: " << qtyT0.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             David.Fox-Rabinovitz                    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECQuantityFormattingTest, Preliminary)
    {
    ShowQuantifiedValue("3ft 4in", "DefaultReal", "IN");
    ShowQuantifiedValue("3ft 4in", "DefaultRealU", "IN", "_");
    ShowQuantifiedValue("3ft4in", "DefaultRealU", "IN", "_");
    ShowQuantifiedValue("3ft4 1/8in", "DefaultRealU", "IN", "_");

    // TODO These are all FormatStrings that override Fractional and FractionalU
    // ShowQuantifiedValue("3ft4 1/8in", "fract16u", "IN", "_");
    // ShowQuantifiedValue("3ft4 1/8in", "fract16", "IN");
    // ShowQuantifiedValue("3.5 FT", "fract16", "IN");

    ShowQuantifiedValue("3.6 FT", "DefaultReal", "IN");
    ShowQuantifiedValue("3.75 FT", "DefaultRealU", "IN", "-");
    ShowQuantifiedValue("3 1/3ft", "DefaultRealU", "IN");
    ShowQuantifiedValue("3 1/3ft", "DefaultRealU", "M");
    ShowQuantifiedValue("3 1/3ft", "DefaultRealU", "MM");
    ShowQuantifiedValue("5:6", "AmerFI", "MM");
    ShowQuantifiedValue("5:6", "AmerFI", "IN");
    ShowQuantifiedValue("5:", "AmerFI", "MM");
    ShowQuantifiedValue("5:", "AmerFI", "IN");
    ShowQuantifiedValue(":6", "AmerFI", "MM");
    ShowQuantifiedValue(":6", "AmerFI", "IN");

    // TODO another FormatString override, f:AngleDMS(8)
    // ShowQuantifiedValue("135:23:11", "dms8", "ARC_DEG");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(ECQuantityFormattingTest, TestWithOnlyMajorUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());

    ECUnitCP ft = ECTestFixture::GetUnitsSchema()->GetUnitCP("FT");
    ECFormatCP fi = ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFi");

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "Test");
    koq->SetPersistenceUnit(*ft);
    koq->SetDefaultPresentationFormat(*fi);

    Formatting::FormatProblemCode problem;
    BEU::Quantity newQuantity = ECQuantityFormatting::CreateQuantity("5", *fi, &problem);
    EXPECT_TRUE(ECUnit::AreEqual(ft, newQuantity.GetUnit()));
    EXPECT_EQ(5, newQuantity.GetMagnitude());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(ECQuantityFormattingTest, FormatPersistedValueUsingKoQPersistenceUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());

    ECUnitCP meter = ECTestFixture::GetUnitsSchema()->GetUnitCP("M");

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKindOfQuantity");
    koq->SetPersistenceUnit(*meter);

    ECQuantityFormattingStatus status;
    // TODO where to handle case where we have no presentation units but we have a persistence unit that we'd like to use as an input unit 
    // for defaultrealU
    BEF::NumericFormatSpec format = BEF::NumericFormatSpec::DefaultFormat();
    Utf8String formatString = ECQuantityFormatting::FormatPersistedValue(5, koq, &status);
    Utf8String expectedFormat = Utf8PrintfString("5%c0 m", format.GetDecimalSeparator());
    EXPECT_STREQ(expectedFormat.c_str(), formatString.c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
