/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "CodesHelpers.h"
#include "IntegrationTestsSettings.h"
#include "TestsProgressCallback.h"
#include "DgnPlatformHelpers.h"
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeStringUtilities.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_HTTP

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectCodesEqual(DgnCodeStateCR exp, DgnCodeStateCR act)
    {
    if (exp.IsAvailable())
        {
        EXPECT_TRUE(act.IsAvailable());
        }
    else if (exp.IsReserved())
        {
        EXPECT_TRUE(act.IsReserved());
        EXPECT_EQ(exp.GetReservedBy().GetValue(), act.GetReservedBy().GetValue());
        }
    else
        {
        EXPECT_EQ(exp.IsDiscarded(), act.IsDiscarded());
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectCodesEqual(DgnCodeInfoCR exp, DgnCodeInfoCR act)
    {
    EXPECT_EQ(exp.GetCode(), act.GetCode());
    ExpectCodesEqual(static_cast<DgnCodeState>(exp), static_cast<DgnCodeState>(act));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectCodesEqual(DgnCodeInfoSet const& expected, DgnCodeInfoSet const& actual)
    {
    EXPECT_EQ(expected.size(), actual.size());
    for (auto expIter = expected.begin(), actIter = actual.begin(); expIter != expected.end() && actIter != actual.end(); ++expIter, ++actIter)
        ExpectCodesEqual(*expIter, *actIter);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectCodesState(DgnCodeInfoSet const& expect, IRepositoryManagerP imodelManager)
    {
    DgnCodeInfoSet actual;
    DgnCodeSet codes;
    for (auto const& info : expect)
        codes.insert(info.GetCode());

    EXPECT_STATUS(Success, imodelManager->QueryCodeStates(actual, codes));
    ExpectCodesEqual(expect, actual);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectCodeState(DgnCodeInfoCR expect, IRepositoryManagerP imodelManager)
    {
    DgnCodeInfoSet expectInfos;
    expectInfos.insert(expect);
    ExpectCodesState(expectInfos, imodelManager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectNoCodesWithState(DgnCodeInfoSet const& expect, IRepositoryManagerP imodelManager)
    {
    DgnCodeInfoSet actual;
    DgnCodeSet codes;
    for (auto const& info : expect)
        codes.insert(info.GetCode());

    EXPECT_STATUS(Success, imodelManager->QueryCodeStates(actual, codes));
    EXPECT_EQ(0, actual.size());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectNoCodeWithState(DgnCodeInfoCR expect, IRepositoryManagerP imodelManager)
    {
    DgnCodeInfoSet expectInfos;
    expectInfos.insert(expect);
    ExpectNoCodesWithState(expectInfos, imodelManager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCodeInfo CreateCodeAvailable(DgnCodeCR code)
    {
    return DgnCodeInfo(code);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCodeInfo CreateCodeDiscarded(DgnCodeCR code, Utf8StringCR changeSetId)
    {
    DgnCodeInfo info(code);
    info.SetDiscarded(changeSetId);
    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCodeInfo CreateCodeReserved(DgnCodeCR code, DgnDbR db)
    {
    DgnCodeInfo info(code);
    info.SetReserved(db.GetBriefcaseId());
    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCodeInfo CreateCodeUsed(DgnCodeCR code, Utf8StringCR changeSetId)
    {
    DgnCodeInfo info(code);
    info.SetUsed(changeSetId);
    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCode CreateElementCode(DgnDbR db, Utf8StringCR name, Utf8CP nameSpace)
    {
    return nullptr != nameSpace ? PhysicalMaterial::CreateCode(db.GetDictionaryModel(), name) : SpatialCategory::CreateCode(db.GetDictionaryModel(), name);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCode MakeModelCode(Utf8CP name, DgnDbR db)
    {
    return PhysicalPartition::CreateCode(*db.Elements().GetRootSubject(), name);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
int GetCodesCount(DgnDbR db)
    {
    // LRP InitializeFileJob initialize imodel and create codes in iModel Hub Services. If this place fails, check if LRP are running fine in the server
    int count = 0;

    auto iterator = DgnCode::MakeIterator(db);
    for (auto it = iterator.begin(); it != iterator.end(); ++it)
        count++;

    return count;
    }

Utf8String GetScopeString(CodeSpecPtr codeSpec, DgnElementCR scopeElement)
    {
    return codeSpec->GetScopeElementId(scopeElement).ToString(BeInt64Id::UseHex::Yes);
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
