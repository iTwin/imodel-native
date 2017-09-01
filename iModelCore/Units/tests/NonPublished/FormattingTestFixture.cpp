/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/FormattingTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#if defined (BENTLEY_WIN32)
#include <windows.h>
#include <iostream>
#endif
#include "UnitsTests.h"
#include <Formatting/FormattingApi.h>
#include <Units/UnitRegistry.h>
#include <Units/UnitTypes.h>
#include <Units/Quantity.h>
#include <Units/Units.h>
#include "FormattingTestFixture.h"

using namespace BentleyApi::Formatting;
BEGIN_BENTLEY_FORMATTEST_NAMESPACE
//#define USE_TEST_FILE

static void* testFile = nullptr;
/*=================================================================================**//**
* @bsiclass                                     		David Fox-Rabinovitz 06/2017
+===============+===============+===============+===============+===============+======*/
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
void FormattingTestFixture::ShowHexDump(Utf8CP str, int len)
    {
    Utf8String hd = Utils::HexDump(str, 30);
    LOG.infov(u8"COL: %s", hd.c_str());
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
    LOG.infov("JSON: >%s<   (aliased) >%s<", strN.c_str(), strA.c_str());
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
    FormatUnitGroup fug1 = FormatUnitGroup::FormatUnitGroup(jval);
    EXPECT_TRUE(fug.IsIdentical(fug1));
    //LOG.infov("restored FUS Group: %s identical: %s", fug1.ToText(true).c_str(), FormatConstant::BoolText(fug.IsIdentical(fug1)));
    }
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
    LOG.infov("===ShowQuantity: %f of %s = %s", dval, uom, fmtQ.c_str());
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
    LOG.infov("CSP: |%s|", csp.ToText().c_str());

    while (!csp.IsEndOfLine() && n > 0)
        {
        csp.Iterate(str, revers);
        LOG.infov("CSP: |%s|", csp.ToText().c_str());
        --n;
        }
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TestScanPointVector(Utf8CP str)
    {

    FormatParseVector forw(str, false);
    bvector<CursorScanPoint> fvect = forw.GetArray();
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
void FormattingTestFixture::TestSegments(Utf8CP input, size_t start, Utf8CP unitName)
    {
    LOG.infov("=========== TestSegments |%s| from %d", input, start);
    FormatParsingSet fps = FormatParsingSet(input, start, unitName);
    bvector<FormatParsingSegment> segs = fps.GetSegments();
    int n = 0;
    for (FormatParsingSegmentP s = segs.begin(); s != segs.end(); s++)
        {
        LOG.info(s->ToText(n++).c_str());
        }
    LOG.infov("Signature: %s reduced %s", fps.GetSignature(true).c_str(), fps.GetSignature(false).c_str());
    LOG.info("=========== TestSegments End =============");
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::ParseToQuantity(Utf8CP input, size_t start, Utf8CP unitName)
    {
    LOG.infov("=========== Parsing To Quantity |%s| from %d", input, start);
    FormatParsingSet fps = FormatParsingSet(input, start, unitName);
    BEU::Quantity qty = fps.GetQuantity();
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
        Json::Value val = namF->ToJson(false);
        LOG.infov("Format %s = %s ", stdName, val.ToString().c_str());
        JsonValueCR spc = val[json_NumericFormat()];
        LOG.infov("NumSpec %s", spc.ToString().c_str());
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
    FormatTraits traits = FormatTraits::DefaultZeroes;
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
         traits = FormatTraits::DefaultZeroes;
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
void FormattingTestFixture::NamedFormatJsonTest(Utf8CP stdName, bool verbose, Utf8CP expected)
    {
    NamedFormatSpecCP  nfsP = StdFormatSet::FindFormatSpec(stdName);
    Json::Value jval = nfsP->ToJson(verbose);
    LOG.infov("Format %s json: %s", stdName, jval.ToString().c_str());
    bool equ; 

    NamedFormatSpec nfs1 = NamedFormatSpec(jval);

    equ = nfsP->IsIdentical(nfs1);
   // EXPECT_TRUE(equ);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::NumericFormatSpecJsonTest(NumericFormatSpecCR nfs)
    {
    Json::Value jval = nfs.ToJson(true);
    //Utf8String str = jval.ToString();
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



END_BENTLEY_FORMATTEST_NAMESPACE

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



