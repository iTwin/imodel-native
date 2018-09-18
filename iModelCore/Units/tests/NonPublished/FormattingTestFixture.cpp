/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/FormattingTestFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#if defined (BENTLEY_WIN32)
#include <windows.h>

#endif
#include <iostream>
#include <time.h>
#include <ctime>
#include <locale>
#include <chrono>
#include <iomanip>

#include "UnitsTests.h"
#include <Formatting/FormattingApi.h>
#include <Units/UnitRegistry.h>
#include <Units/UnitTypes.h>
#include <Units/Quantity.h>
#include <Units/Units.h>
//#include <ECObjects/ECQuantityFormatting.h>
#include "FormattingTestFixture.h"

using namespace BentleyApi::Formatting;
BEGIN_BENTLEY_FORMATTEST_NAMESPACE
//#define USE_TEST_FILE

static void* testFile = nullptr;

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::SetUpL10N()
    {
    BeFileName sqlangFile;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(sqlangFile);
    sqlangFile.AppendToPath(L"sqlang");
    sqlangFile.AppendToPath(L"Units_en.sqlang.db3");

    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);

    BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory, BeSQLite::BeSQLiteLib::LogErrors::Yes);
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TearDownL10N()
    {
    BeSQLite::L10N::Shutdown();
    }


/*=================================================================================**//**
* @bsiclass                                     		David Fox-Rabinovitz 06/2017
+===============+===============+===============+===============+===============+======*/

void FormattingTestFixture::LogMessage(Utf8CP format, va_list argptr)
    {
    /*WChar buf[512];
    WString::VSprintf(WCharCP format, va_list argptr)
    WCharP wp = BeStringUtilities::Utf8ToWChar(buf, txt, sizeof(buf));
    LOG.infov("Signature Test%02d  >%s<================", tstN, txt);*/
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::StdFormattingTest(Utf8CP formatName, double dval, Utf8CP expectedValue)
    {
    EXPECT_STREQ (expectedValue, NumericFormatSpec::StdFormatDouble(formatName, dval).c_str());
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::SignaturePattrenCollapsing(Utf8CP txt, int tstN, bool hexDump)
    {
    LOG.infov("Signature Test%02d  >%s<================", tstN, txt);
    FormattingScannerCursor curs1 = FormattingScannerCursor(txt, -1);
    Utf8CP sig = curs1.GetSignature(true, true);
    LOG.infov("Original Signature Test%02d  >%s< Signature >%s< ", tstN, txt, sig);
    sig = curs1.GetReversedSignature(true, true);
    Utf8String rpat = curs1.ReversedPattern();
    LOG.infov("Reversed Signature Test%02d  >%s< Signature >%s< ", tstN, txt, sig);
    LOG.infov("Restored Signature Test%02d  >%s< Signature >%s< Pattern >%s< ", tstN, txt, curs1.ReversedSignature().c_str(), rpat.c_str());

    Utf8String cols = curs1.CollapseSpaces(true);
    sig = curs1.GetSignature(true, true);
    Utf8CP pat = curs1.GetPattern(false, true);

    if (hexDump)
        {
        Utf8String hd = Utils::HexDump(cols.c_str(), 30);
        LOG.infov(u8"CollapsedHEX: %s", hd.c_str());
        }
    LOG.infov("   Collapsed%02d >%s< (len %d)", tstN, cols.c_str(), cols.length());
    LOG.infov("   Collapsed Signature%02d >%s< (src %d  sig %d) pattern: [%s]", tstN, sig, strlen(txt), strlen(sig), pat);
    LOG.info("=========");
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------  
void FormattingTestFixture::ShowSignature(Utf8CP txt, int tstN)
    {
    FormattingScannerCursor curs = FormattingScannerCursor(txt, -1);
    Utf8CP sig = curs.GetSignature(true, true);
    Utf8CP pat = curs.GetPattern(false, false);
    LOG.infov("Signature Test%02d  >%s< Signature >%s< Pattern >%s<", tstN, txt, sig, pat);
    sig = curs.GetReversedSignature(true, true);
    pat = curs.GetPattern(false, false);
    LOG.infov("Reversed Signature Test%02d  >%s< Signature >%s< Pattern >%s<", tstN, txt, sig, pat);
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ShowHexDump(Utf8String str, int len)
    {
    Utf8String hd = Utils::HexDump(str.c_str(), 30);
    LOG.infov(u8"COL: %s", hd.c_str());
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ShowHexDump(Utf8CP str, int len, Utf8CP message)
    {
    Utf8String hd = Utils::HexDump(str, 30);
    if(nullptr == message)
        LOG.infov(u8"COL: %s", hd.c_str());
    else
        LOG.infov(u8"%s => %s", message, hd.c_str());
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ShowFUS(Utf8CP koq)
    {
    FormatUnitSet fus = FormatUnitSet(koq);
    if (fus.HasProblem())
        LOG.infov("Invalid KOQ: >%s<", koq);
    else
        LOG.infov("KOQ: >%s<  Normilized: >%s< WithAlias: >%s< ", koq, fus.ToText(false).c_str(), fus.ToText(true).c_str());
    Utf8String strA = fus.ToJsonString(true);
    Utf8String strN = fus.ToJsonString(false);
    LOG.infov("FUS JSON: >%s<   (aliased) >%s<", strN.c_str(), strA.c_str());
    }


void FormattingTestFixture::RegisterFUS(Utf8CP descr, Utf8CP name)
{
	FormatUnitSetCP fusP = StdFormatSet::AddFUS(descr, name);
	if (StdFormatSet::FusRegistrationHasProblem())
		LOG.infov("Invalid FUS definition: >%s<", descr);
	else
		LOG.infov("Registring FUS (%s): >%s<  Normilized: >%s< WithAlias: >%s< ", name, descr, fusP->ToText(false).c_str(), fusP->ToText(true).c_str());
	Utf8String strA = fusP->ToJsonString(true);
	Utf8String strN = fusP->ToJsonString(false);
	LOG.infov("Registered FUS: %s JSON: >%s<   (aliased) >%s<", fusP->GetFusName(), strN.c_str(), strA.c_str());
}


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::CrossValidateFUS(Utf8CP descr1, Utf8CP descr2)
    {
    FormatUnitSet fus1 = FormatUnitSet(descr1);
    if (fus1.HasProblem())
        LOG.infov("Invalid descr1: >%s<", descr1);
    FormatUnitSet fus2 = FormatUnitSet(descr2);
    if (fus2.HasProblem())
        LOG.infov("Invalid descr1: >%s<", descr1);
    EXPECT_TRUE (fus1.IsIdentical(fus2));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ShowFUG(Utf8CP name, Utf8CP fugText)
    {
    FormatUnitGroup fug = FormatUnitGroup(name, fugText);
    Json::Value jval = fug.ToJson(true);
    LOG.infov("FUS Group: %s JSON: >%s<", name, jval.ToString().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TestFUS(Utf8CP fusText, Utf8CP norm, Utf8CP aliased)
    {
    FormatUnitSet fus = FormatUnitSet(fusText);
    EXPECT_STREQ (norm, fus.ToText(false).c_str());
    EXPECT_STREQ (aliased, fus.ToText(true).c_str());
    Json::Value jval = fus.ToJson(true);
    FormatUnitSet fus1;
    fus1.LoadJsonData(jval);
    EXPECT_TRUE(fus.IsIdentical(fus1));
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TestFUG(Utf8CP name, Utf8CP fusText, Utf8CP norm, Utf8CP aliased)
    {
    FormatUnitGroup fug = FormatUnitGroup(name, fusText);
    Json::Value jval = fug.ToJson(true);
    //LOG.infov("FUS Group: %s JSON: >%s<", name, jval.ToString().c_str());
    EXPECT_STREQ (norm, fug.ToText(false).c_str());
    EXPECT_STREQ (aliased, fug.ToText(true).c_str());
    FormatUnitGroup fug1 = FormatUnitGroup(jval);
    EXPECT_TRUE(fug.IsIdentical(fug1));
    //LOG.infov("restored FUS Group: %s identical: %s", fug1.ToText(true).c_str(), FormatConstant::BoolText(fug.IsIdentical(fug1)));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/17
//----------------------------------------------------------------------------------------
//static void TestFUGFormat(double dval, Utf8CP uom, Utf8CP name, Utf8CP fusText)
//    {
//    FormatUnitGroup fug = FormatUnitGroup(name, fusText);
//    BEU::UnitCP unit = BEU::UnitRegistry::Instance().LookupUnitCI(uom);
//    if (nullptr == unit)
//        {
//        LOG.infov("Invalid UOM: >%s<", uom);
//        return;
//        }
//    BEU::Quantity qty = BEU::Quantity(dval, *unit);
//    ECN::ECQuantityFormattingStatus status;
//    Utf8String str = ECN::ECQuantityFormatting::FormatQuantity(qty, &fug, 0, &status);
//
//    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ShowQuantity(double dval, Utf8CP uom, Utf8CP fusUnit, Utf8CP fusFormat, Utf8CP space)
    {
    BEU::UnitCP unit = BEU::UnitRegistry::Instance().LookupUnit(uom);
    if (nullptr == unit)
        {
        LOG.infov("Invalid UOM: >%s<", uom);
        return;
        }
    BEU::Quantity const q = BEU::Quantity(dval, *unit);
    FormatUnitSet fus = FormatUnitSet(fusFormat, fusUnit);
    if (fus.HasProblem())
        {
        LOG.infov("Invalid Formatting Set: >%s< or unit: >%s<", fus.GetProblemDescription().c_str());
        return;
        }

    Utf8String fmtQ = fus.FormatQuantity(q, space);
    LOG.infov("\n===ShowQuantity: %f of %s = %s", dval, uom, fmtQ.c_str());
    Json::Value jval = fus.FormatQuantityJson(q, true);
    Utf8String jsonQ = jval.ToString();
    LOG.infov("JSON: %s", jsonQ.c_str());
    FormatUnitSet deFUS = StdFormatSet::DefaultFUS(q);
    LOG.infov("Default FUS JSON: %s", deFUS.ToJsonString(true).c_str());
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ShowQuantityS(Utf8CP descr)
    {
    size_t bufL = 256;
    Utf8P buf = (Utf8P)alloca(bufL);
    bvector<Utf8CP> parts;

    size_t partN = ExtractArgs(descr, buf, bufL, &parts, ';');
    if (partN < 5)
        LOG.infov("Invalid input string for ShowQunatity (%d args)", partN);
    else
        {
        double dval = atof(parts[0]);
        Utf8CP uom = parts[1];
        Utf8CP fusUnit = parts[2];
        Utf8CP fusFormat = parts[3];
        Utf8CP space = parts[4];
        ShowQuantity(dval, uom, fusUnit, fusFormat, space);
        }
    }

void FormattingTestFixture::CustomFormatAnalyzer(double dval, Utf8CP uom, Utf8CP jsonCustomFormat)
{
    BEU::UnitCP unit = BEU::UnitRegistry::Instance().LookupUnit(uom);
    if (nullptr == unit)
        {
        LOG.infov("Invalid UOM: >%s<", uom);
        return;
        }
    BEU::Quantity const qty = BEU::Quantity(dval, *unit);
     
    // now we need to add a new format definition from the json string

    FormatProblemDetail problem;
    NamedFormatSpecCP nfs = StdFormatSet::AppendCustomFormat(jsonCustomFormat, problem);
    if (problem.IsProblem())
        {
        LOG.infov("Invalid JSON definition. error code %d", problem.GetProblemCode());
        return;
        }
    LOG.infov("Created custom format spec %s (%s)", nfs->GetName(), nfs->GetAlias());
    Utf8String str = NumericFormatSpec::StdFormatQuantity(*nfs, qty);
    LOG.infov("===CustomFormatAnalyzer: %f of %s = %s", dval, uom, str.c_str());
}
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
NumericAccumulator* FormattingTestFixture::NumericAccState(NumericAccumulator* nacc, Utf8CP txt)
    {
    if (nullptr == nacc)
    return nacc;
    while (nacc->CanTakeNext(txt))
    {
    LOG.infov("Added[%d] %c  state %s", nacc->GetByteCount(), *txt, Utils::AccumulatorStateName(nacc->AddSymbol((size_t)*txt)).c_str());
    ++txt;
    }
    nacc->SetComplete();
    return nacc;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TestFUSQuantity(double dval, Utf8CP uom, Utf8CP fusDesc, Utf8CP space)
    {
    BEU::UnitCP unit = BEU::UnitRegistry::Instance().LookupUnit(uom);
    BEU::Quantity q = BEU::Quantity(dval, *unit);
    FormatUnitSet fus = FormatUnitSet(fusDesc);
    LOG.infov("Testing FUS->Q  %s", fus.FormatQuantity(q, space).c_str());
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
int FormattingTestFixture::FindLastDividerPos(Utf8CP txt, Utf8Char div)
    {
    int pos = -1;
    int i = 0;
    if (nullptr == txt)
        return pos;
    while ('\0' != *txt)
        {
        if (*txt == div)
            pos = i;
        i++;
        txt++;
        }
    return pos;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
size_t FormattingTestFixture::FindDividerPos(Utf8CP txt, bvector<int>* pos, Utf8Char div)
    {
    if (nullptr == pos || nullptr == txt || *txt == '\0')
        return 0;
    int i = 0;
    pos->clear();
    while ('\0' != *txt)
        {
        if (*txt == div)
            pos->push_back(i);
        i++;
        txt++;
        }
    return pos->size();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ShowSplitByDividers(Utf8CP txt, Utf8CP divDef)
    {
    FormattingDividers div = FormattingDividers(divDef);
    LOG.infov("Examining:|%s|", txt);
    Utf8Char buf[64];
    memset(buf, '\0', sizeof(buf));
    size_t len;
    Utf8CP s = txt, e = txt;
    for (Utf8CP p = txt, s = txt; *p != '\0'; p++)
        {
        if (div.IsDivider(*p))
            {
            len = p - s;
            memcpy(buf, s, len);
            buf[len] = '\0';
            LOG.infov("segment:|%s| at %d len %d", buf, (p - txt), len);
            s = p + 1;
            }
        e = p;
        }
    len = e - s + 1;
    if(len > 0)
        LOG.infov("last segment:|%s| at %d len %d", e, (e - txt), len);

    return;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
bool FormattingTestFixture::OpenTestData()
    {
#if defined (BENTLEY_WIN32)
    char buf[MAX_PATH + 128];
    memset(buf, 0, sizeof(buf));
    GetModuleFileName(NULL, buf, MAX_PATH);
    if (strlen(buf) > 1)
        {
        int pos = FindLastDividerPos(buf, '\\');
        LOG.infov("Work Directory: |%s| last %d", buf, FindLastDividerPos(buf, '\\'));
        if (strlen(buf) > 0)
            {
            strcpy(&buf[pos + 1], "UnitTestSource.txt");
            LOG.infov("TestFileName: |%s|", buf);
            FILE* in = fopen(buf, "r");
            testFile = in;
            if (nullptr != in)
                return true;
            }
        }
#endif
    return false;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::CloseTestData()
    {
    if (nullptr != testFile)
        {
        FILE* in = (FILE*)testFile;
        fclose(in);
        testFile = nullptr;
        }
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
bool FormattingTestFixture::IsDataAvailalbe()
    {
    if (nullptr != testFile)
        {
        FILE* in = (FILE*)testFile;
        return (feof(in)) ? false : true;
        }
    return false;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
bool FormattingTestFixture::GetNextLine(Utf8P buf, int bufLen)
    {
#if defined (BENTLEY_WIN32)
    if (nullptr != testFile)
        {
        FILE* in = (FILE*)testFile;
        if (nullptr != buf && bufLen > 0)
            *buf = '\0';
        if (feof(in))
            return false;
        fgets(buf, bufLen, in);
        for (size_t i = strlen(buf); i >= 0; --i)
            {
            if (buf[i] == '\0' || buf[i] == '\n' || buf[i] == '\r' || buf[i] == '\f')
                buf[i] = '\0';
            else
                break;
            }
        return true;
        }
#endif
    return false;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::DecomposeString(Utf8CP str, bool revers)
    {
    size_t n = strlen(str);
    CursorScanPoint csp(str, revers? n:0, revers);

	LOG.infov("\n Decomposed String: %s =============", str);
    LOG.infov("CSP: |%s|", csp.ToText().c_str());

    while (!csp.IsEndOfLine() && n > 0)
        {
        csp.Iterate(str, revers);
        LOG.infov("CSP: |%s|", csp.ToText().c_str());
        --n;
        }
	LOG.infov("=================================\n", str);
    }

size_t FormattingTestFixture::AddSignatureSymbol(Utf8CP outBuf, size_t bufLen, size_t* bufIndex)
{
	return 0;
}

Utf8String FormattingTestFixture::GetStringSignature(Utf8CP str)
{
#define GSS_BUFSIZE 128
	Utf8String sig;
	size_t n = strlen(str);
	//Utf8Char buf[GSS_BUFSIZE + 2];

	CursorScanPoint csp(str, n, false);

	while (!csp.IsEndOfLine() && n > 0)
	{
		csp.Iterate(str,  false);

		LOG.infov("CSP: |%s|", csp.ToText().c_str());
		--n;
	}

	return sig;
}



Utf8String FormattingTestFixture::ExtractTokenValue(WCharCP line, WCharCP token, WCharCP delim)
    {
#define ETV_BUFLEN 256
    WCharCP tokPtr = wcsstr(line, token);
    Utf8String tokVal8 = "";
    if (nullptr == tokPtr)
        return tokVal8;
    size_t tokLen = wcslen(token);
    WCharCP endPtr = wcsstr (tokPtr + tokLen, delim);
    if (nullptr == endPtr)
        return tokVal8;

    size_t wordLen = endPtr - tokPtr - tokLen;
    wchar_t tokVal[ETV_BUFLEN + 2];
    memset(tokVal, 0, sizeof(tokVal));
    memcpy(tokVal, tokPtr + tokLen, wordLen * sizeof(wchar_t));
    BeStringUtilities::WCharToUtf8(tokVal8, tokVal, -1);

    return tokVal8;
    }

#ifdef _WIN32
bool FormattingTestFixture::ValidateSchemaUnitNames(char* schemaPath, Utf8CP token, char* reportPath)
{
#define VSN_BUFLEN 1024
    if (Utils::IsNameNullOrEmpty(schemaPath))
        return false;
    wchar_t fnw[256];
    wchar_t tokW[256];
    BeStringUtilities::Utf8ToWChar(fnw, schemaPath, 254);
    FILE* in = _wfopen(fnw, L"rtS, ccs=UTF-8"); // fopen(schemaPath, "r");
    if (nullptr == in)
        return false;
    FILE* out = nullptr;
    char fullTok[VSN_BUFLEN + 2];
    sprintf(fullTok, "<%s>", token);
    BeStringUtilities::Utf8ToWChar(tokW, fullTok, 254);

    wchar_t locW[VSN_BUFLEN + 2];// , unitName[VSN_BUFLEN + 2], className[VSN_BUFLEN + 2];
    memset(locW, 0, sizeof(locW));
    WCharCP tokSymb = L"<";
    Utf8String tokVal8;
    BEU::UnitCP unitP, oldP;
    int count = 0;
    int old = 0;
    int miss = 0;
    int tot = 0;
    int cls = 0;
    int prop = 0;
    int active = 0;
    bvector<Utf8String> clasV, propV;
    while (!feof(in))
        {
        fgetws(locW, VSN_BUFLEN, in);
        count++;
        tokVal8 = ExtractTokenValue(locW, tokW, tokSymb);
        if (!tokVal8.empty())
            {
            unitP = BEU::UnitRegistry::Instance().LookupUnit(tokVal8.c_str());
            oldP = nullptr;
            if (nullptr == unitP)
                oldP = BEU::UnitRegistry::Instance().LookupUnitUsingOldName(tokVal8.c_str());
            if (nullptr != reportPath)
                {
                if (nullptr == out)
                    out = fopen(reportPath, "w");
                if (nullptr != out)
                    {
                    if (nullptr == unitP)
                        {
                        if (nullptr == oldP)
                            {
                            fprintf (out, "     ***%s\n", tokVal8.c_str());
                            miss++;
                            }
                        else
                            {
                            fprintf (out, "     ###%s\n", tokVal8.c_str());
                            old++;
                            }
                        }
                    else
                        {
                        fprintf (out, "     %s\n", tokVal8.c_str());
                        active++;
                        }
                    tot++;
                    }
                }
            }// !tokVal8.empty()
        else
            {
            tokVal8 = ExtractTokenValue(locW, L"<ECClass typeName=\"", L"\"");
            if (!tokVal8.empty())
                {
                if (nullptr != reportPath)
                    {
                    if (nullptr == out)
                        out = fopen(reportPath, "w");
                    if (nullptr != out)
                        {
                        fprintf (out, "== Class: %s\n", tokVal8.c_str());
                        cls++;
                        clasV.push_back(tokVal8);
                        }
                    }
                }
            else
                {
                tokVal8 = ExtractTokenValue(locW, L"<ECProperty propertyName=\"", L"\"");
                if (!tokVal8.empty())
                    {
                    if (nullptr != reportPath)
                        {
                        if (nullptr == out)
                            out = fopen(reportPath, "w");
                        if (nullptr != out)
                            {
                            fprintf (out, "    === Property: %s\n", tokVal8.c_str());
                            propV.push_back(tokVal8);
                            prop++;
                            }
                        }
                    }
                }
            }
        } //<ECProperty propertyName="
   
    if (nullptr != out)
        {
        fprintf (out, "Total %d obsolete %d missing %d active %d classes %d props %d\n ", tot, old, miss, active, cls, prop);
        if(clasV.size() > 0)
        {
            fprintf (out, "=====list of %d classes\n", (int)clasV.size());
            for (int i = 0; i < clasV.size(); i++)
                {
                fprintf (out, "%s\n", clasV[i].c_str());
                }
        }
        
        if (propV.size() > 0)
            {
            fprintf (out, "=====list of %d props\n", (int)propV.size());
            for (int i = 0; i < propV.size(); i++)
                {
                fprintf (out, "%s\n", propV[i].c_str());
                }
            }        fclose(out);
        }
    count++;
    return true;
}
#endif
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TestScanPointVector(Utf8CP str)
    {

    FormatParseVector forw(str, false);
    bvector<CursorScanPoint> fvect = forw.GetArray();
	LOG.infov("========ScanPointVector for %s=================", str);
    LOG.info("============= Forward Vector scan =================");
    for (CursorScanPointP csp = fvect.begin(); csp != fvect.end(); csp++)
        {
        LOG.infov("->CSP: |%s|", csp->ToText().c_str());
        }
    LOG.infov("->Pattern: |%s|", forw.GetPattern().c_str());
    LOG.info("============= Reversed Vector scan =================");
    FormatParseVector cont(str, true);
    bvector<CursorScanPoint> vect = cont.GetArray();

    for(CursorScanPointP csp = vect.begin(); csp != vect.end(); csp++)
        { 
        LOG.infov("<-CSP: |%s|", csp->ToText().c_str());
        }
    LOG.infov("<-Pattern: |%s|", cont.GetPattern().c_str());
    LOG.info("============= Vector Scan complete =================\n");
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TestScanTriplets(Utf8CP str)
    {
    LOG.info("============= Triplet Scan =================\n");
    LOG.infov("Input: |%s|", str);
     FormatParseVector forw(str, false);
     LOG.infov("->Signature: |%s|", forw.GetSignature().c_str());
     do {
         LOG.infov("->Triplet at %d |%s|", forw.GetTripletIndex(), forw.GetTriplet().c_str());
         } while(forw.MoveFrame());

    FormatParseVector revs(str, true);
    LOG.infov("<-Signature: |%s|", revs.GetSignature().c_str());
    do {
        LOG.infov("<-Triplet at %d |%s|", revs.GetTripletIndex(), revs.GetTriplet().c_str());
        } while (revs.MoveFrame());
    LOG.info("============= Triplet Scan complete =================\n");
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Utf8CP FormattingTestFixture::TestGrabber(Utf8CP input, size_t start)
    {
    LOG.infov("=========== Numeric Grabber test |%s| from %d", input, start);
    NumberGrabber ng = NumberGrabber(input, start);
    int repet = 100000;
    FormatStopWatch wat = FormatStopWatch();
    size_t len = ng.Grab();
    for (int i = 0; i < repet; i++)
        {
         len = ng.Grab();
        }
    LOG.info(wat.LastIntervalMetrics(repet).c_str());
    size_t ti = ng.GetNextIndex();
    if (ng.GetType() == ParsingSegmentType::Real)
        {
        LOG.infov("Real %.6f Integer %d  Tail |%s| nextInd %d", ng.GetReal(), ng.GetInteger(), ng.GetTail(), ti);
        }
    else if (ng.GetType() == ParsingSegmentType::Integer)
        {
        LOG.infov("Integer %d Real %.6f Tail |%s| nextInd %d", ng.GetInteger(), ng.GetReal(), ng.GetTail(), ng.GetNextIndex());
        }
    else
        LOG.infov("Cannot interpret input. Tail |%s| nextInd %d", ng.GetTail(), ng.GetNextIndex());
    input = ng.GetTail();
    return input;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TestSegments(Utf8CP input, size_t start, Utf8CP unitName, Utf8CP expectReduced)
    {
    FormatParsingSet fps = FormatParsingSet(input, start, unitName);
    if (nullptr == expectReduced)
        {
        LOG.infov("=========== TestSegments |%s| from %d", input, start);

        bvector<FormatParsingSegment> segs = fps.GetSegments();
        int n = 0;
        for (FormatParsingSegmentP s = segs.begin(); s != segs.end(); s++)
            {
            LOG.info(s->ToText(n++).c_str());
            }
        LOG.infov("Signature: %s reduced %s", fps.GetSignature(true).c_str(), fps.GetSignature(false).c_str());
        LOG.info("=========== TestSegments End =============");
        }
    else
        {
        EXPECT_STREQ (expectReduced, fps.GetSignature(false).c_str());
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ParseToQuantity(Utf8CP input, size_t start, Utf8CP unitName, Utf8CP formatName)
    {
    LOG.infov("=========== Parsing To Quantity |%s| from %d", input, start);
    FormatUnitSet fus = FormatUnitSet(formatName, unitName);
    FormatProblemCode probCode;
    FormatParsingSet fps = FormatParsingSet(input, start, unitName);
    BEU::Quantity qty = fps.GetQuantity(&probCode, &fus);
    if(qty.IsNullQuantity())
        LOG.info("Parsing failed");
    else
        {
        LOG.infov("Unit: %s Magnitude %.6f", qty.GetUnitName(), qty.GetMagnitude());
        BEU::UnitCP un1 = BEU::UnitRegistry::Instance().LookupUnit(unitName);
        BEU::Quantity q1 = qty.ConvertTo(un1);
        if (q1.IsNullQuantity())
            LOG.infov("Invalid alternative Unit: %s", unitName);
        else
            LOG.infov("Unit: %s Magnitude %.6f", q1.GetUnitName(), q1.GetMagnitude());
        }
    LOG.info("=========== Parsing To Quantity End =============");
    }

void FormattingTestFixture::ShowQuantifiedValue(Utf8CP input, Utf8CP formatName, Utf8CP fusUnit, Utf8CP spacer)
    {
    FormatUnitSet fus = FormatUnitSet(formatName, fusUnit);
    FormatParsingSet fps = FormatParsingSet(input, 0, fusUnit);
    if (fus.HasProblem())
        {
        LOG.errorv("FUS-problem: %s", fus.GetProblemDescription().c_str());
        return;
        }
    FormatProblemCode probCode;
    BEU::Quantity qty = fps.GetQuantity(&probCode, &fus);
    Utf8String qtyT = fus.FormatQuantity(qty, spacer);
    LOG.errorv("Input:%s Quantity %s", input, qtyT.c_str());
    return;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
bool FormattingTestFixture::GetNextInstruction(Utf8P buf, int bufLen, Utf8P com, int comLen)
    {
#if defined (BENTLEY_WIN32) && defined (USE_TEST_FILE)
    if (nullptr != testFile)
        {
        FILE* in = (FILE*)testFile;
        if (nullptr != buf && bufLen > 0)
            *buf = '\0';
        if (nullptr != com && comLen > 0)
            *com = '\0';
        if (feof(in))
            {
            CloseTestData();
            return false;
            }
        int locL = 256;
        Utf8P loc = (Utf8P)alloca(locL);
        fgets(loc, locL, in);
        for (size_t i = strlen(loc); i >= 0; --i)
            {
            if (loc[i] == '\0' || loc[i] == '\n' || loc[i] == '\r' || loc[i] == '\f')
                loc[i] = '\0';
            else
                break;
            }
        int pos = FindLastDividerPos(loc, ':');

        if (pos > 0)  // there is a command
            {
            size_t tl = strlen(loc + pos + 1);
            if (tl > bufLen - 1)
                tl = bufLen - 1;
            memcpy(buf, loc + pos + 1, tl);
            buf[tl] = '\0';
            if (comLen - 1 < pos)
                pos = comLen - 1;
            memcpy(com, loc, pos);
            com[pos] = '\0';
            }
        else
            {
            size_t tl = strlen(loc);
            if (tl > bufLen - 1)
                tl = bufLen - 1;
            memcpy(buf, loc, tl);
            buf[tl] = '\0';
            }
        return true;
        }
#endif
    return false;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
size_t FormattingTestFixture::CopyTextSecure(Utf8P dest, size_t destSize, Utf8CP src)
    {
    if (nullptr == dest || destSize == 0)
        return 0;
    *dest = '\0';
    if (nullptr == src || *src == '\0')
        return 0;
    size_t len = strlen(src);
    if (len > destSize - 1)
        len = destSize - 1;
    memcpy(dest, src, len);
    dest[len] = '\0';
    return len;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
size_t FormattingTestFixture::ExtractArgs(Utf8CP desc, Utf8P buf, size_t bufL, bvector<Utf8CP>* parts, Utf8Char div)
    {
    size_t descL = (nullptr == desc) ? 0 : strlen(desc) + 1;
    if (descL > bufL - 1)
        descL = bufL - 1;
    if (nullptr == buf || bufL < 2)
        return 0;
    CopyTextSecure(buf, bufL, desc);

    for (size_t i = descL; i >= 0; --i)
        {
        if (buf[i] == '\0' || buf[i] == '\n' || buf[i] == '\r' || buf[i] == '\f')
            buf[i] = '\0';
        else
            break;
        }
    bvector<int> posV;
    parts->push_back(buf);  // first part or the whole string
    Utf8P ptr = buf;
    size_t partN = FindDividerPos(buf, &posV, div);
    int indx;
    for (int i = 0; i < partN; ++i)
        {
        indx = posV[i];
        buf[indx] = '\0';
        ptr = buf + indx + 1;
        parts->push_back(ptr);
        }
    return parts->size();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
PUSH_MSVC_IGNORE(6385 6386)
size_t FormattingTestFixture::GetNextArguments(Utf8P buf, int bufLen, bvector<Utf8CP>* parts, Utf8Char div)
    {
#if defined (BENTLEY_WIN32)
    if (nullptr == parts)
        return 0;
    parts->clear();
    if (nullptr == buf || bufLen < 2 || nullptr == testFile)
        return 0;

    FILE* in = (FILE*)testFile;
    if (feof(in))
        return 0;

    bvector<int> posV;
    int locL = 256;
    wchar_t* locW = (wchar_t*)alloca(locL);
    *locW = 0;
    fgetws(locW, locL, in);
    Utf8String str8;
    BeStringUtilities::WCharToUtf8(str8, locW, -1);
    Utf8P loc = (Utf8P)alloca(locL);
    *loc = 0;
    //fgets(loc, locL, in);
    strcpy(loc, str8.c_str());

    for (size_t i = strlen(loc); i >= 0; --i)
        {
        if (loc[i] == '\0' || loc[i] == '\n' || loc[i] == '\r' || loc[i] == '\f')
            loc[i] = '\0';
        else
            break;
        }
    CopyTextSecure(buf, bufLen, loc);

    parts->push_back(buf);  // first part or the whole string
    Utf8P ptr = buf;
    size_t partN = FindDividerPos(loc, &posV, div);
    int indx;
    for (int i = 0; i < partN; ++i)
        {
        indx = posV[i];
        buf[indx] = '\0';
        ptr = buf + indx + 1;
        parts->push_back(ptr);
        }
    return parts->size();
#else
    return 0;
#endif
    }
POP_MSVC_IGNORE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void  FormattingTestFixture::NamedSpecToJson(Utf8CP stdName)
    {
    if (nullptr == stdName)
        {
        NumericFormatSpec defS = NumericFormatSpec();
        NamedFormatSpec def("default", defS, "def");
        Json::Value val = defS.ToJson(false);
        LOG.infov("Format %s = %s ", stdName, val.ToString().c_str());
        JsonValueCR spc = val[json_NumericFormat()];
        LOG.infov("NumSpec %s", spc.ToString().c_str());
        }
    else
        {
        NamedFormatSpecCP namF = StdFormatSet::FindFormatSpec(stdName);
        if (nullptr == namF)
            {
            LOG.infov("Format %s is not defined", stdName);
            }
        else
            {
            Json::Value val = namF->ToJson(false);
            LOG.infov("Format %s = %s ", stdName, val.ToString().c_str());
            JsonValueCR spc = val[json_NumericFormat()];
            LOG.infov("NumSpec %s", spc.ToString().c_str());
            }
        }
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
bvector<TraitJsonKeyMap> TraitJsonKeyMap::TraitJsonKeySet()
    {
    static bvector<TraitJsonKeyMap> vec;
    if (vec.size() == 0)
        {
        vec.push_back(TraitJsonKeyMap(FormatTraits::TrailingZeroes, json_TrailZeroes()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::LeadingZeroes, json_LeadZeroes()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::KeepDecimalPoint, json_KeepDecPnt()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::KeepSingleZero, json_KeepSingleZero()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ExponentZero, json_ExponentZero()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ZeroEmpty, json_ZeroEmpty()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::Use1000Separator, json_Use1000Separator()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ApplyRounding, json_ApplyRounding()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::AppendUnitName, json_AppendUnitName()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::UseFractSymbol, json_UseFractSymbol()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::FractionDash, json_FractionDash()));
        }
    return vec;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::FormattingTraitsTest()
    {
    bvector<TraitJsonKeyMap> vec = TraitJsonKeyMap::TraitJsonKeySet();
    bvector<string> pos;
    bvector<string> neg;

    pos.push_back("{\"TrailZeroes\":\"true\"}");
    neg.push_back("{\"TrailZeroes\":\"false\"}");
    pos.push_back("{\"LeadZeroes\":\"true\"}");
    neg.push_back("{\"LeadZeroes\":\"false\"}");
    pos.push_back("{\"KeepDecPnt\":\"true\"}");
    neg.push_back("{\"KeepDecPnt\":\"false\"}");
    pos.push_back("{\"KeepSingleZero\":\"true\"}");
    neg.push_back("{\"KeepSingleZero\":\"false\"}");
    pos.push_back("{\"ExponentZero\":\"true\"}");
    neg.push_back("{\"ExponentZero\":\"false\"}");
    pos.push_back("{\"ZeroEmpty\":\"true\"}");
    neg.push_back("{\"ZeroEmpty\":\"false\"}");
    pos.push_back("{\"Use1000Separator\":\"true\"}");
    neg.push_back("{\"Use1000Separator\":\"false\"}");
    pos.push_back("{\"ApplyRounding\":\"true\"}");
    neg.push_back("{\"ApplyRounding\":\"false\"}");
    pos.push_back("{\"AppendUnitName\":\"true\"}");
    neg.push_back("{\"AppendUnitName\":\"false\"}");
    pos.push_back("{\"UseFractSymbol\":\"true\"}");
    neg.push_back("{\"UseFractSymbol\":\"false\"}");
    pos.push_back("{\"FractionDash\":\"true\"}");
    neg.push_back("{\"FractionDash\":\"false\"}");

    TraitJsonKeyMap* map;
    Json::Value val;
    FormatTraits traits = FormatTraits::DefaultTraits;
    for(int i =0; i < vec.size(); i++)
        {
         map = &vec[i];
         traits = NumericFormatSpec::SetTraitsBit(map->GetTrait(), traits, true);
         NumericFormatSpec::TraitsBitToJsonKey(val, map->GetKey(), map->GetTrait(), traits);
         EXPECT_STREQ (pos[i].c_str(), val.ToString().c_str());
         //LOG.infov("Bit %s set: %s", map->GetKey(), val.ToString().c_str());
         traits = NumericFormatSpec::SetTraitsBit(map->GetTrait(), traits, false);
         NumericFormatSpec::TraitsBitToJsonKey(val, map->GetKey(), map->GetTrait(), traits);
         EXPECT_STREQ (neg[i].c_str(), val.ToString().c_str());
         //LOG.infov("Bit %s drop: %s", map->GetKey(), val.ToString().c_str());      
         val.clear();
         traits = FormatTraits::DefaultTraits;
        }
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::FormattingSpecTraitsTest(Utf8CP testName, NumericFormatSpecCR spec, bool verbose)
    {
     Json::Value val = spec.JsonFormatTraits(verbose);
     LOG.infov("Test %s json: %s", testName, val.ToString().c_str());
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::NamedFormatJsonTest(int testNum, Utf8CP stdName, bool verbose, Utf8CP expected)
    {
    NamedFormatSpecCP  nfsP = StdFormatSet::FindFormatSpec(stdName);
    Json::Value jval = nfsP->ToJson(verbose);
    LOG.infov("[%03d] Format %s json: %s", testNum, stdName, jval.ToString().c_str());
    bool equ; 

    NamedFormatSpec nfs1 = NamedFormatSpec(jval);

    equ = nfsP->IsIdentical(nfs1);
    if (equ)
        EXPECT_TRUE(equ);
    else
        LOG.infov("Format %s conversion failed", stdName);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::NumericFormatSpecJsonTest(NumericFormatSpecCR nfs)
    {
    Json::Value jval = nfs.ToJson(true);
    Utf8String str = jval.ToString();
    NumericFormatSpec nfs1 = NumericFormatSpec(jval);
    EXPECT_TRUE(nfs.IsIdentical(nfs1));
    jval = nfs.ToJson(false);
    nfs1 = NumericFormatSpec(jval);
    EXPECT_TRUE(nfs.IsIdentical(nfs1));
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::UnitProxyJsonTest(Utf8CP unitName, Utf8CP labelName)
    {
    UnitProxyCR up1 = UnitProxy(unitName, labelName);
    Json::Value jval = up1.ToJson();
    UnitProxy up2 = UnitProxy(jval);
    EXPECT_TRUE(up1.IsIdentical(up2));
    }

void FormattingTestFixture::UnitSynonymMapTest(Utf8CP unitName, Utf8CP synonym)
    {
    BEU::UnitSynonymMap map = BEU::UnitSynonymMap(unitName, synonym);
    Json::Value jval = map.ToJson();
    BEU::UnitSynonymMap other = BEU::UnitSynonymMap(jval);
    bool ident = map.IsIdentical(other);
    EXPECT_TRUE(map.IsIdentical(other));
    if (Utf8String::IsNullOrEmpty(synonym))
        synonym = "*";
    if (Utf8String::IsNullOrEmpty(unitName))
        unitName = "*";
    WString wName(unitName, true);
    WString wSyn(synonym, true);
    WString wJson(jval.ToString().c_str(), true);
    WString wBool(FormatConstant::BoolText(ident), true);
    //WChar outStr[258];
    //memset(outStr, 0, sizeof(outStr));
    //BeStringUtilities::Snwprintf(outStr, 256, L"UnitSynonymMap(%ls, %ls)", wName.c_str(), wSyn.c_str(), wJson.c_str(), wBool.c_str());
    //LOG.infov(L"FormattedMapString: %ls", outStr);
    LOG.infov(L"UnitSynonymMap(%ls, %ls) => json: %ls (%ls)", wName.c_str(), wSyn.c_str(), wJson.c_str(), wBool.c_str());

    //LOG.infov(L"UnitSynonymMap(%ls, %ls) => json: %ls (%ls)", unitName, synonym, jval.ToString().c_str(), FormatConstant::BoolText(ident));
    }

void FormattingTestFixture::StandaloneNamedFormatTest(Utf8CP jsonFormat, bool doPrint)
    {
    NamedFormatSpec nfs = NamedFormatSpec(jsonFormat);
    Json::Value nfsJ = nfs.ToJson(false);
    int diff = BeStringUtilities::StricmpAscii(jsonFormat, nfsJ.ToString().c_str());

    NamedFormatSpec nfsE = NamedFormatSpec();
    nfsE.LoadJson(jsonFormat);
    Json::Value nfsEJ = nfsE.ToJson(false);
    int diffE = BeStringUtilities::StricmpAscii(jsonFormat, nfsEJ.ToString().c_str());

    EXPECT_TRUE(diff == 0);
    EXPECT_TRUE(diffE == 0);
    if (doPrint)
        {
        LOG.infov("\n=================StandaloneFormatTest================");
        LOG.infov("Restored Json: %s   (diff %d)", nfsJ.ToString().c_str(), diff);
        LOG.infov("Json-loaded: %s   (diff %d)", nfsEJ.ToString().c_str(), diffE);
        if(diff != 0 || diffE != 0)
            LOG.infov("Different from original: %s   (diff %d)", jsonFormat);
        LOG.infov("=======================================================\n");
        }
    }

void FormattingTestFixture::StandaloneFUSTest(double dval, Utf8CP unitName, Utf8CP fusUnitName, Utf8CP formatName, Utf8CP result)
    {
    LOG.info("\n=========== StandaloneFUSTest=============");
    BEU::UnitCP uom = BEU::UnitRegistry::Instance().LookupUnit(unitName);
    if (nullptr == uom)
        {
        LOG.infov("Invalid Unit Name %s", unitName);
        return;
        }
    if (Utils::IsNameNullOrEmpty(fusUnitName))
        {
        LOG.infov("Missing FUS Unit Name");
        return;
        }
    BEU::UnitCP fusUOM = BEU::UnitRegistry::Instance().LookupUnit(fusUnitName);
    if (nullptr == uom)
        {
        LOG.infov("Invalid FUS Unit Name %s", fusUnitName);
        return;
        }
    fusUOM = nullptr;
    Utf8Char buf[256];
    if(Utils::IsNameNullOrEmpty(formatName))
        sprintf(buf, "%s", fusUnitName);
    else
        sprintf(buf, "%s(%s)", fusUnitName, formatName);
    FormatUnitSet fus = FormatUnitSet(buf);
    if (Utils::IsNameNullOrEmpty(formatName))
        sprintf(buf, "{\"unitName\":\"%s\"}", fusUnitName);
    else
        sprintf(buf, "{\"unitName\":\"%s\",\"formatName\":\"%s\"}", fusUnitName, formatName);
    LOG.infov("JSON %s", buf);
    FormatUnitSet fusS = FormatUnitSet(buf);

    BEU::Quantity qty = BEU::Quantity(dval, *uom);
    Utf8String qtyT = fus.FormatQuantity(qty, "");
    Utf8String qtyS = fusS.FormatQuantity(qty, "");

    LOG.infov("qty value: %s (expected %s)", qtyT.c_str(), result);
    LOG.infov("JSON-qty value: %s (expected %s)", qtyS.c_str(), result);
    EXPECT_STREQ (result, qtyT.c_str());
    EXPECT_STREQ (result, qtyS.c_str());

    Utf8String fusJ = fus.ToJsonString(false, true);
    LOG.infov("\nfusJ  %s\n", fusJ.c_str());
    FormatUnitSet fusFromJ = FormatUnitSet(fusJ.c_str());
    qtyT = fusFromJ.FormatQuantity(qty, "");
    EXPECT_STREQ (result, qtyT.c_str());
    LOG.infov("restored qty value: %s (expected %s)", qtyT.c_str(), result);
    LOG.info("=========== StandaloneFUSTest=============\n");
    }

void FormattingTestFixture::FormatDoubleTest(double dval, Utf8CP fmtName, int prec, double round, Utf8CP expect)
    {
    Utf8String txt = NumericFormatSpec::StdFormatDouble(fmtName, dval, prec, round);
    if(Utils::IsNameNullOrEmpty(expect))
       LOG.infov("%f formatted: %s (%d)", dval, txt.c_str(), txt.size());
    else
        EXPECT_STREQ (expect, txt.c_str());

    }

void FormattingTestFixture::VerifyQuantity(Utf8CP input, Utf8CP unitName, Utf8CP formatName, double magnitude, Utf8CP qtyUnitName)
    {
    FormatUnitSet fus = FormatUnitSet(formatName, unitName);
    FormatProblemCode probCode;
    FormatParsingSet fps = FormatParsingSet(input, 0, unitName);
    BEU::Quantity qty = fps.GetQuantity(&probCode, &fus);
    if (FormatProblemCode::NoProblems != probCode)
        {
        LOG.infov("Parsing problem: Input |%s| - ", Utils::SubstituteEmptyOrNull(input, "<empty>"), fps.GetProblemDescription().c_str());
        }
    else
        {
        BEU::PhenomenonCP pp = fus.GetPhenomenon();
        BEU::UnitCP unit = (nullptr == pp) ? BEU::UnitRegistry::Instance().LookupUnit(qtyUnitName) : pp->LookupUnit(qtyUnitName);
        BEU::Quantity temp = BEU::Quantity(magnitude, *unit);
        bool eq = qty.IsClose(temp, 0.0001);
        EXPECT_TRUE(eq);
        if (!eq)
            {
            Utf8PrintfString txt("Quantity (%s} not equal {%s}", qty.ToDebugText().c_str(), temp.ToDebugText().c_str());
            LOG.infov("Verification problem: Input |%s| - %s", Utils::SubstituteEmptyOrNull(input, "<empty>"), txt.c_str());
            }
        }
    }

void FormattingTestFixture::ShowPhenomenon(BEU::PhenomenonCP phenP, bvector<BEU::PhenomenonCP>& undefPhenomena)
    {
    if (nullptr == phenP)
        return;
    if (phenP->HasUnits())
        {
        bvector<BEU::UnitCP> unitsV = phenP->GetUnits();
        LOG.infov("Phenomenon %s (UOM list of %d)", phenP->GetName(), unitsV.size());
        for (const BEU::UnitCP* up = unitsV.begin(); up != unitsV.end(); up++)
            {
            LOG.infov("  %s", (*up)->GetName());
            }
        if (phenP->HasSynonyms())
            {
            BEU::T_UnitSynonymVector* synV = phenP->GetSynonymVector();
            LOG.infov("  List of %d synonyms:", synV->size());
            for (const BEU::UnitSynonymMap* up = synV->begin(); up != synV->end(); up++)
                {
                LOG.infov(u8"  %s => %s", up->GetSynonym(), up->GetUnitName());
                }
            LOG.info("=======================================");
            }
        else
            LOG.info("==== No synonyms are defined===========");
        }
    else
        {
        LOG.infov("Phenomenon: %s (no UOM defined)", phenP->GetName());
        undefPhenomena.push_back(phenP);
        }
    return;
    }

void FormattingTestFixture::ShowKnownPhenomena()
    {
    bvector<BEU::PhenomenonCP> allPhenomena;
    bvector<BEU::PhenomenonCP> undefPhenomena;
    BEU::UnitRegistry::Instance().AllPhenomena(allPhenomena);
    for (const BEU::PhenomenonCP* ph = allPhenomena.begin(); ph != allPhenomena.end(); ph++)
        {
        ShowPhenomenon(*ph, undefPhenomena);
        }
    if (undefPhenomena.size() > 0)
        {
        LOG.infov("\nList of %d Phenomena without UOM: ", undefPhenomena.size());
        for (const BEU::PhenomenonCP* ph = undefPhenomena.begin(); ph != undefPhenomena.end(); ph++)
            {
            LOG.infov("  %s", (*ph)->GetName());
            }
        }
    }

void FormattingTestFixture::ShowSynonyms()
    {
    bvector<BEU::UnitCP> allUnits;
    bvector<Utf8CP> synonyms;
    WString wsyn;
    BEU::UnitRegistry::Instance().AllUnits(allUnits);
    for (BEU::UnitCP* un = allUnits.begin(); un != allUnits.end();un++)
        {
        if ((*un)->GetSynonymList(synonyms) > 0)
            {
            LOG.infov("Unit %s synonyms:", (*un)->GetName());
            for (size_t i = 0; i < synonyms.size(); i++)
                {
                wsyn = WString(synonyms[i], true);
                LOG.infov(L"%02d %ls", i + 1, wsyn.c_str());
                }
            }
        }
    }

void FormattingTestFixture::TestFusLabel(Utf8CP fusFormat, Utf8CP fusUnit, Utf8CP fusName)
{
	FormatUnitSet fus = FormatUnitSet(fusFormat, fusUnit);


}

Utf8String FormattingTestFixture::SetLocale(Utf8CP name)
{
	Utf8String locName;
	//const std::locale& loc = *(locale::global); // (name);
	std::locale currLoc("");
	std::locale loc(name);
	const std::numpunct<char>& myfacet = std::use_facet < numpunct<char> >(loc);

	//const std::time_get<char>&timfacet = std::use_facet < time_get<char> >(loc);

	std::time_get<char>::dateorder ord;
	ord = std::use_facet<std::time_get<char> >(loc).date_order();

	Utf8Char buf[3];
	buf[0] = myfacet.decimal_point();
	buf[1] = myfacet.thousands_sep();
	buf[2] = 0;
	int ordN = (int)ord;
	locName.assign(buf);
	LOG.infov("Current system locale decpnt= %s name %s  ord %d  switched from %s", locName.c_str(), loc.name().c_str(), ordN, currLoc.name().c_str());

	//locName.assign(setlocale(LC_ALL, ""));
	//setlocale(LC_ALL, name);
	/*LOG.infov("Locale changed from %s to %s >>> DecimalPoint %s 100Separator %s Grouping %s", locName.c_str(), Utils::SubstituteNull(name, "<empty>"),
		                              Utils::GetCurrentDecimalSeparator().c_str(), Utils::GetCurrentThousandSeparator().c_str(),
	                                  Utils::GetCurrentGrouping().c_str());*/
	return locName;
}
void FormattingTestFixture::TestTimeFormat(int year, int month, int day, int hour, int min, int sec)
{
	struct tm  myT;
	memset(&myT, 0, sizeof(myT));

	myT.tm_sec = sec;
	myT.tm_min = min;
	myT.tm_hour = hour;
	myT.tm_mday = day;
	myT.tm_mon = month-1;
	myT.tm_year = year-1900;
	//	int tm_wday;    /* days since Sunday - [0,6] */
	//	int tm_yday;    /* days since January 1 - [0,365] */
	myT.tm_isdst = 1;
	//};
	time_t tim = mktime(&myT);
time_t time = (time_t)(BeTimeUtilities::GetCurrentTimeAsUnixMillis() / 1000.0); time;  // Convert in second   
Utf8String ts = ctime(&time);
Utf8String ts1 = ctime(&tim);
ts.Trim();
LOG.infov("Current time %s", ts.c_str());
LOG.infov("induced time %d/%d/%d %d:%d:%d  =>  %s", month, day, year, hour, min, sec, ts1.c_str());
Utf8Char buf[128];
strftime(buf, 126, "%c   %Ec", &myT);
LOG.infov("Locale Induced C-time %s", buf);
strftime(buf, 126, "%x   %Ex", &myT);
LOG.infov("Locale Induced X-time %s", buf);
strftime(buf, 126, "%D   %F", &myT);
LOG.infov("Locale Induced D,F-time %s", buf);

}


//void FormattingTestFixture::TestTime(Utf8CP localeName, Utf8CP label, Utf8CP pattern)
//{
//	std::locale loc(localeName);
//	std::wstringstream strm;
//	
//	strm.imbue(loc);
//	strm.clear();
//	if (Utils::IsNameNullOrEmpty(pattern))
//		pattern = "%c";
//
//	if (Utils::IsNameNullOrEmpty(label))
//		label = "time";
//	WString wPatt(pattern, true);
//
//	using std::chrono::system_clock;
//	std::time_t tt = system_clock::to_time_t(system_clock::now());
//
//	struct std::tm * ptm = std::localtime(&tt);
//
//	strm << std::put_time(ptm, wPatt.c_str()) << '\n';
//	Utf8String utf8Buffer(strm.str().c_str());
//
//	LOG.infov(u8"Locale %s %s: %s", std::locale(localeName).name().c_str(), label, utf8Buffer.c_str());
//
//	//Utf8String dump = Utils::HexDump((Utf8CP)((void*)(strm.str().c_str())), (int)utf8Buffer.size() * 2);
//	//LOG.infov("Dump[%d]: %s", dump.size(), dump.c_str());
//
//}

//WString wName(unitName, true);
//WString wSyn(synonym, true);
//WString wJson(jval.ToString().c_str(), true);
//WString wBool(FormatConstant::BoolText(ident), true);
////WChar outStr[258];
////memset(outStr, 0, sizeof(outStr));
////BeStringUtilities::Snwprintf(outStr, 256, L"UnitSynonymMap(%ls, %ls)", wName.c_str(), wSyn.c_str(), wJson.c_str(), wBool.c_str());
////LOG.infov(L"FormattedMapString: %ls", outStr);
//LOG.infov(L"UnitSynonymMap(%ls, %ls) => json: %ls (%ls)", wName.c_str(), wSyn.c_str(), wJson.c_str(), wBool.c_str());



END_BENTLEY_FORMATTEST_NAMESPACE

//WString wJson(jval.ToString().c_str(), true);
//WString wBool(FormatConstant::BoolText(ident), true);
////WChar outStr[258];
////memset(outStr, 0, sizeof(outStr));
////BeStringUtilities::Snwprintf(outStr, 256, L"UnitSynonymMap(%ls, %ls)", wName.c_str(), wSyn.c_str(), wJson.c_str(), wBool.c_str());
////LOG.infov(L"FormattedMapString: %ls", outStr);
//LOG.infov(L"UnitSynonymMap(%ls, %ls) => json: %ls (%ls)", wName.c_str(), wSyn.c_str(), wJson.c_str(), wBool.c_str());


//FormattingTestFixture::
//EXPECT_STREQ ("{\"TrailZeroes\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"TrailZeroes\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"LeadZeroes\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"LeadZeroes\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"KeepDecPnt\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"KeepDecPnt\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"KeepSingleZero\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"KeepSingleZero\":\"false\"}", val.ToString().c_str());
//
//EXPECT_STREQ ("{\"ExponentZero\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"ExponentZero\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"ZeroEmpty\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"ZeroEmpty\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"Use1000Separator\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"Use1000Separator\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"ApplyRounding\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"ApplyRounding\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"AppendUnitName\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"AppendUnitName\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"UseFractSymbol\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"UseFractSymbol\":\"false\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"FractionDash\":\"true\"}", val.ToString().c_str());
//EXPECT_STREQ ("{\"FractionDash\":\"false\"}", val.ToString().c_str());



