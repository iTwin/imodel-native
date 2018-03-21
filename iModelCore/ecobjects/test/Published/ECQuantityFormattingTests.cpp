/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECQuantityFormattingTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

static Formatting::StdFormatSet const stdFmtSet;
static void ShowQuantifiedValue(Utf8CP input, Utf8CP formatName, Utf8CP fusUnit, Utf8CP spacer=nullptr)
    {
    ECUnitCP unit = StandardUnitsHelper::GetUnit(fusUnit);
    BEF::NamedFormatSpecCP format = stdFmtSet.FindNamedFormatSpec(formatName);

    BEF::FormatUnitSet fus = BEF::FormatUnitSet(format, unit);
    EXPECT_FALSE(fus.HasProblem()) << "FUS-Problem: %s" << fus.GetProblemDescription().c_str();
    if (fus.HasProblem())
        return;

    BEF::NamedFormatSpecCP real4u = stdFmtSet.FindNamedFormatSpec("real4u");

    Formatting::FormatProblemCode code;
    BEF::FormatUnitSet fus0 = BEF::FormatUnitSet(real4u, unit);
    BEU::Quantity qty = ECQuantityFormatting::CreateQuantity(input, fus, &code);
    Utf8String qtyT = fus.FormatQuantity(qty, spacer);
    Utf8String qtyT0 = fus0.FormatQuantity(qty, spacer);

    EXPECT_STREQ(qtyT.c_str(), qtyT0.c_str()) << "Input: |" << input << "| Quantity: " << qtyT.c_str() << "  Equivalent: " << qtyT0.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             David.Fox-Rabinovitz                    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECQuantityFormattingTest, Preliminary)
    {
    ShowQuantifiedValue("3ft 4in", "real", "IN");
    ShowQuantifiedValue("3ft 4in", "realu", "IN", "_");
    ShowQuantifiedValue("3ft4in", "realu", "IN", "_");
    ShowQuantifiedValue("3ft4 1/8in", "realu", "IN", "_");
    ShowQuantifiedValue("3ft4 1/8in", "fract16u", "IN", "_");
    ShowQuantifiedValue("3ft4 1/8in", "fract16", "IN");

    ShowQuantifiedValue("3.5 FT", "fract16", "IN");
    ShowQuantifiedValue("3.6 FT", "real", "IN");
    ShowQuantifiedValue("3.75 FT", "realu", "IN", "-");
    ShowQuantifiedValue("3 1/3ft", "realu", "IN");
    ShowQuantifiedValue("3 1/3ft", "realu", "M");
    ShowQuantifiedValue("3 1/3ft", "realu", "MM");
    ShowQuantifiedValue("5:6", "fi8", "MM");
    ShowQuantifiedValue("5:6", "fi8", "IN");
    ShowQuantifiedValue("5:", "fi8", "MM");
    ShowQuantifiedValue("5:", "fi8", "IN");
    ShowQuantifiedValue(":6", "fi8", "MM");
    ShowQuantifiedValue(":6", "fi8", "IN");
    ShowQuantifiedValue("135:23:11", "dms8", "ARC_DEG");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(ECQuantityFormattingTest, TestWithOnlyInputUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->AddReferencedSchema(*StandardUnitsHelper::GetSchema());

    ECUnitCP meter = StandardUnitsHelper::GetUnit("M");
    Formatting::NamedFormatSpecCP fi = stdFmtSet.FindNamedFormatSpec("fi8");

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "Test");
    koq->SetPersistenceUnit(*meter, fi);

    Formatting::FormatProblemCode problem;
    BEU::Quantity newQuantity = ECQuantityFormatting::CreateQuantity("5", koq->GetDefaultPresentationUnit(), &problem);
    EXPECT_EQ(meter, newQuantity.GetUnit());
    EXPECT_EQ(5, newQuantity.GetMagnitude());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(ECQuantityFormattingTest, FormatPersistedValueUsingKoQPersistenceUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->AddReferencedSchema(*StandardUnitsHelper::GetSchema());

    ECUnitCP meter = StandardUnitsHelper::GetUnit("M");

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKindOfQuantity");
    koq->SetPersistenceUnit(*meter);

    ECQuantityFormattingStatus status;
    Utf8String formatString = ECQuantityFormatting::FormatPersistedValue(5, koq, &status);
    EXPECT_STREQ("5.0 m", formatString.c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
