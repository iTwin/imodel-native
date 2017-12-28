/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECQuantityFormattingTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include <Formatting/FormattingApi.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECQuantityFormatting.h>
#include <BeSQLite/L10N.h>

namespace BEU = BentleyApi::Units;
namespace BEF = BentleyApi::Formatting;
BEGIN_BENTLEY_ECOBJECT_NAMESPACE


static void SetUpL10N() 
    {
    BeFileName sqlangFile;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(sqlangFile);
    sqlangFile.AppendToPath(L"sqlang");
    sqlangFile.AppendToPath(L"Units_en.sqlang.db3");

    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);

    BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory, BeSQLite::BeSQLiteLib::LogErrors::Yes);
    BeSQLite::L10N::Shutdown();
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));
    }

static void TearDownL10N()
    {
    BeSQLite::L10N::Shutdown();
    }

static void ShowQuantifiedValue(Utf8CP input, Utf8CP formatName, Utf8CP fusUnit, Utf8CP spacer=nullptr)
    {
    BEF::FormatUnitSet fus = BEF::FormatUnitSet(formatName, fusUnit);
    if (fus.HasProblem())
        {
        LOG.errorv("FUS-problem: %s", fus.GetProblemDescription().c_str());
        return;
        }
    BEF::FormatUnitSet fus0 = BEF::FormatUnitSet("real4u", fusUnit);
    BEU::Quantity qty = ECQuantityFormatting::CreateQuantity(input, 0, fus);
    Utf8String qtyT = fus.FormatQuantity(qty, spacer);
    Utf8String qtyT0 = fus0.FormatQuantity(qty, spacer);

    LOG.errorv("Input: |%s| Quantity %s  (Equivalent: %s)", input, qtyT.c_str(), qtyT0.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             David.Fox-Rabinovitz                    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECQuantityFormattingTests, Preliminary)
    {
    SetUpL10N();
    LOG.error("================  Quantity Formatting Log ===========================");

    BEF::FormatUnitSet fus = BEF::FormatUnitSet("realu", "IN");

    ShowQuantifiedValue("3' 4\"", "real", "IN");
    ShowQuantifiedValue("3' 4\"", "realu", "IN", "_");
    ShowQuantifiedValue("3'4\"", "realu", "IN", "_");
    ShowQuantifiedValue("3'4 1/8\"", "realu", "IN", "_");
    ShowQuantifiedValue("3'4 1/8\"", "fract16u", "IN", "_");
    ShowQuantifiedValue("3'4 1/8\"", "fract16", "IN");
    ShowQuantifiedValue("3.5 FT", "fract16", "IN");
    ShowQuantifiedValue("3.6 FT", "real", "IN");
    ShowQuantifiedValue("3.75 FT", "realu", "IN", "-");
    ShowQuantifiedValue("3 1/3'", "realu", "IN");
    ShowQuantifiedValue("3 1/3'", "realu", "M");
    ShowQuantifiedValue("3 1/3'", "realu", "MM");
    ShowQuantifiedValue("5:6", "fi8", "MM");
    ShowQuantifiedValue("5:6", "fi8", "IN");
    ShowQuantifiedValue("5:", "fi8", "MM");
    ShowQuantifiedValue("5:", "fi8", "IN");
    ShowQuantifiedValue(":6", "fi8", "MM");
    ShowQuantifiedValue(":6", "fi8", "IN");
    ShowQuantifiedValue("135:23:11", "dms8", "ARC_DEG");
    LOG.error("================  End of Quantity Formatting Log  ===========================");
    TearDownL10N();
    }

END_BENTLEY_ECOBJECT_NAMESPACE