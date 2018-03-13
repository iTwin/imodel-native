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

struct ECQuantityFormattingTests : ECTestFixture {};

static void ShowQuantifiedValue(Utf8CP input, Utf8CP formatName, Utf8CP fusUnit, Utf8CP spacer=nullptr)
    {
    ECUnitCP unit = StandardUnitsHelper::GetUnit(fusUnit);
    BEF::NamedFormatSpecCP format = BEF::StdFormatSet::FindFormatSpec(formatName);

    BEF::FormatUnitSet fus = BEF::FormatUnitSet(format, unit);
    EXPECT_FALSE(fus.HasProblem()) << "FUS-Problem: %s" << fus.GetProblemDescription().c_str();
    if (fus.HasProblem())
        return;

    BEF::NamedFormatSpecCP real4u = BEF::StdFormatSet::FindFormatSpec("real4u");

    Formatting::FormatProblemCode code;
    BEF::FormatUnitSet fus0 = BEF::FormatUnitSet(real4u, unit);
    BEU::Quantity qty = ECQuantityFormatting::CreateQuantity(input, 0, fus, &code);
    Utf8String qtyT = fus.FormatQuantity(qty, spacer);
    Utf8String qtyT0 = fus0.FormatQuantity(qty, spacer);

    LOG.errorv("Input: |%s| Quantity %s  (Equivalent: %s)", input, qtyT.c_str(), qtyT0.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             David.Fox-Rabinovitz                    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECQuantityFormattingTests, Preliminary)
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

END_BENTLEY_ECN_TEST_NAMESPACE
