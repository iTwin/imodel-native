/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/TestFixture/FormattingTestFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#if defined (BENTLEY_WIN32)
#include <windows.h>
#include <iostream>
#endif

#include <locale>

#include "FormattingTestFixture.h"

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE
//#define USE_TEST_FILE

static void* testFile = nullptr;

BEU::UnitRegistry* FormattingTestFixture::s_unitsContext = nullptr;

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::SetUp()
    {
    BeFileName sqlangFile;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(sqlangFile);
    sqlangFile.AppendToPath(L"sqlang");
    sqlangFile.AppendToPath(L"Units_en.sqlang.db3");

    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);

    BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory, BeSQLite::BeSQLiteLib::LogErrors::Yes);
    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));

    if (nullptr == s_unitsContext)
        s_unitsContext = new BEU::UnitRegistry();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TearDown()
    {
    BeSQLite::L10N::Shutdown();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void FormattingTestUtils::SignaturePattrenCollapsing(Utf8CP txt, int tstN)
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

    LOG.infov("   Collapsed%02d >%s< (len %d)", tstN, cols.c_str(), cols.length());
    LOG.infov("   Collapsed Signature%02d >%s< (src %d  sig %d) pattern: [%s]", tstN, sig, strlen(txt), strlen(sig), pat);
    LOG.info("=========");
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------  
void FormattingTestUtils::ShowSignature(Utf8CP txt, int tstN)
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
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
int FormattingTestUtils::FindLastDividerPos(Utf8CP txt, Utf8Char div)
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
size_t FormattingTestUtils::FindDividerPos(Utf8CP txt, bvector<int>* pos, Utf8Char div)
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
void FormattingTestUtils::ShowSplitByDividers(Utf8CP txt, Utf8CP divDef)
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
bool FormattingTestUtils::OpenTestData()
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
void FormattingTestUtils::CloseTestData()
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
bool FormattingTestUtils::IsDataAvailalbe()
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
bool FormattingTestUtils::GetNextLine(Utf8P buf, int bufLen)
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
void FormattingTestUtils::DecomposeString(Utf8CP str, bool revers)
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
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
Utf8String FormattingTestUtils::ExtractTokenValue(wchar_t* line, wchar_t* token, wchar_t* delim)
    {
#define ETV_BUFLEN 256
    wchar_t* tokPtr = wcsstr(line, token);
    Utf8String tokVal8 = "";
    if (nullptr == tokPtr)
        return tokVal8;
    size_t tokLen = wcslen(token);
    wchar_t* endPtr = wcsstr (tokPtr + tokLen, delim);
    if (nullptr == endPtr)
        return tokVal8;

    size_t wordLen = endPtr - tokPtr - tokLen;
    wchar_t tokVal[ETV_BUFLEN + 2];
    memset(tokVal, 0, sizeof(tokVal));
    memcpy(tokVal, tokPtr + tokLen, wordLen * sizeof(wchar_t));
    BeStringUtilities::WCharToUtf8(tokVal8, tokVal, -1);

    return tokVal8;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormattingTestUtils::TestScanPointVector(Utf8CP str)
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
void FormattingTestUtils::TestScanTriplets(Utf8CP str)
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
bool FormattingTestUtils::GetNextInstruction(Utf8P buf, int bufLen, Utf8P com, int comLen)
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
size_t FormattingTestUtils::CopyTextSecure(Utf8P dest, size_t destSize, Utf8CP src)
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
size_t FormattingTestUtils::ExtractArgs(Utf8CP desc, Utf8P buf, size_t bufL, bvector<Utf8CP>* parts, Utf8Char div)
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
size_t FormattingTestUtils::GetNextArguments(Utf8P buf, int bufLen, bvector<Utf8CP>* parts, Utf8Char div)
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
bvector<TraitJsonKeyMap> TraitJsonKeyMap::TraitJsonKeySet()
    {
    static bvector<TraitJsonKeyMap> vec;
    if (vec.size() == 0)
        {
        vec.push_back(TraitJsonKeyMap(FormatTraits::TrailingZeroes, json_trailZeroes()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::LeadingZeroes, json_leadZeroes()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::KeepDecimalPoint, json_keepDecimalPrecision()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::KeepSingleZero, json_keepSingleZero()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ZeroEmpty, json_zeroEmpty()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::Use1000Separator, json_use1000Separator()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ApplyRounding, json_applyRounding()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ShowUnitLabel, json_showUnitLabel()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::FractionDash, json_fractionDash()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::PrependUnitLabel, json_prependUnitLabel()));
        }
    return vec;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestUtils::NumericFormatSpecJsonTest(NumericFormatSpecCR nfs)
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
void FormattingTestUtils::ShowPhenomenon(BEU::PhenomenonCP phenP, bvector<BEU::PhenomenonCP>& undefPhenomena)
    {
    if (nullptr == phenP)
        return;
    if (phenP->HasUnits())
        {
        bvector<BEU::UnitCP> unitsV = phenP->GetUnits();
        LOG.infov("Phenomenon %s (UOM list of %d)", phenP->GetName().c_str(), unitsV.size());
        for (const BEU::UnitCP* up = unitsV.begin(); up != unitsV.end(); up++)
            LOG.infov("  %s", (*up)->GetName().c_str());
        }
    else
        {
        LOG.infov("Phenomenon: %s (no UOM defined)", phenP->GetName().c_str());
        undefPhenomena.push_back(phenP);
        }
    return;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
Utf8String FormattingTestUtils::SetLocale(Utf8CP name)
    {
    Utf8String locName;
    //const std::locale& loc = *(locale::global); // (name);
    std::locale currLoc("");

    std::locale loc(name);
    const std::numpunct<char>& myfacet = std::use_facet < numpunct<char> >(loc);
    Utf8Char buf[3];
    buf[0] = myfacet.decimal_point();
    buf[1] = myfacet.thousands_sep();
    buf[2] = 0;
    locName.assign(buf);
    LOG.infov("Current system locale decpnt= %s name %s   switched from %s", locName.c_str(), loc.name().c_str(), currLoc.name().c_str());

    return locName;
    }

END_BENTLEY_FORMATTEST_NAMESPACE
