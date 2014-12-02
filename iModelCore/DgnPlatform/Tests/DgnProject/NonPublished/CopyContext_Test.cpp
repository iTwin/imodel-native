/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/CopyContext_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

#if defined (DGNPLATFORM_HAVE_DGN_IMPORTER)
#include <DgnPlatform/ForeignFormat/CopyContext.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_FOREIGNFORMAT

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  10/10
+===============+===============+===============+===============+===============+======*/
struct          CopyContextTestFixtureBase : ::testing::Test
{
private:

ScopedDgnHost   m_host;
DgnDbTestDgnManager* m_srcTDM;
DgnDbTestDgnManager* m_dstTDM;
bool            m_isSetup;

protected:

DgnModelP       m_srcModel;
DgnModelP       m_dstModel;
ElementAgenda   m_agenda;

virtual void    _PrepareSource () {} // Setup source color table, levels, etc.
virtual void    _PrepareDestination () {} // Setup destination color table, levels, etc.
virtual void    _InitCopyOptions (CopyContextR copyContext) {} // Change default copy options...
virtual void    _CreateSourceElements () = 0; // Create elements to be added to source file and copied...
virtual void    _ValidateElement (ElementHandleCR eh) = 0; // Inspect copy for expected result...

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
CopyContextTestFixtureBase (bool srcIs3d, bool dstIs3d, bool sameFile)
    {
    // Different source/destintation models...could be in same file or different files...
    m_srcTDM = new DgnDbTestDgnManager (L"3dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE);
    m_dstTDM = sameFile ? m_srcTDM : new DgnDbTestDgnManager (L"2dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE);

    EXPECT_FALSE (NULL == m_srcTDM->GetLoadedDgnPtr()) << "Failed to create source file.\n";
    EXPECT_FALSE (NULL == m_dstTDM->GetLoadedDgnPtr()) << "Failed to create destination file.\n";

    m_srcModel = m_srcTDM->GetLoadedDgnPtr ()->CreateNewModelFromSeed (NULL, "ModelS", DgnModelId());
    m_dstModel = m_dstTDM->GetLoadedDgnPtr ()->CreateNewModelFromSeed (NULL, "ModelD", DgnModelId());

    EXPECT_TRUE (NULL != m_srcModel) << "Failed to create source model.\n";
    EXPECT_TRUE (NULL != m_dstModel) << "Failed to create destination model.\n";

    m_isSetup = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
CopyContextTestFixtureBase (bool modelIs3d)
    {
    // Same source/destintation model...
    m_srcTDM = new DgnDbTestDgnManager (modelIs3d ? L"3dMetricGeneral.idgndb" : L"2dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE);
    m_dstTDM = m_srcTDM;

    EXPECT_FALSE (NULL == m_srcTDM->GetLoadedDgnPtr()) << "Failed to create source file.\n";

    //m_srcModel = m_srcTDM->GetLoadedDgnPtr ()->CreateNewModel (NULL, L"ModelA", DgnModelType::Physical, modelIs3d);
    m_srcModel = m_srcTDM->GetLoadedDgnPtr ()->CreateNewModelFromSeed (NULL, "ModelA", DgnModelId());
    m_dstModel = m_srcModel;

    EXPECT_TRUE (NULL != m_srcModel) << "Failed to create source model.\n";

    m_isSetup = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
~CopyContextTestFixtureBase ()
    {
#if defined (NOT_NOW_DEBUG)
    m_srcTDM->SaveDgn ();
    m_srcTDM->CopyFileTo (L"src.dgn");

    m_dstTDM->SaveDgn ();
    m_dstTDM->CopyFileTo (L"dst.dgn");
#endif

    m_agenda.clear();

    if (m_dstTDM && (m_dstTDM != m_srcTDM))
        delete (m_dstTDM);

    if (m_srcTDM)
        delete (m_srcTDM);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            CreateDefaultTestElements ()
    {
    EditElementHandle eeh;

    // Create simple open element...
    ASSERT_TRUE (SUCCESS == LineHandler::CreateLineElement (eeh, NULL, DSegment3d::From (0, 0, 0, 0, 0, 0), m_srcModel->Is3d (), *m_srcModel));
    m_agenda.Insert (eeh);

    // Create simple closed element...
    ASSERT_TRUE (SUCCESS == EllipseHandler::CreateEllipseElement (eeh, NULL, DEllipse3d::From (0, 0, 0, 1, 0, 0, 0, 1, 0, 0, msGeomConst_2pi), m_srcModel->Is3d (), *m_srcModel));
    m_agenda.Insert (eeh);

    // Create complex element with public children...
    ElementAgenda   groupAgenda;

    ASSERT_TRUE (SUCCESS == LineHandler::CreateLineElement (eeh, NULL, DSegment3d::From (0, 0, 0, 0, 0, 0), m_srcModel->Is3d (), *m_srcModel));
    groupAgenda.Insert (eeh);

    ASSERT_TRUE (SUCCESS == EllipseHandler::CreateEllipseElement (eeh, NULL, DEllipse3d::From (0, 0, 0, 1, 0, 0, 0, 1, 0, 0, msGeomConst_2pi), m_srcModel->Is3d (), *m_srcModel));
    groupAgenda.Insert (eeh);

    ASSERT_TRUE (SUCCESS == NormalCellHeaderHandler::CreateGroupCellElement (eeh, groupAgenda));
    m_agenda.Insert (eeh);

    // Create complex element with private children...
    ElementAgenda       holeAgenda;
    EditElementHandle   solidEeh;

    ASSERT_TRUE (SUCCESS == EllipseHandler::CreateEllipseElement (solidEeh, NULL, DEllipse3d::From (0, 0, 0, 2, 0, 0, 0, 2, 0, 0, msGeomConst_2pi), m_srcModel->Is3d (), *m_srcModel));
    ASSERT_TRUE (SUCCESS == EllipseHandler::CreateEllipseElement (eeh, NULL, DEllipse3d::From (0, 0, 0, 1, 0, 0, 0, 1, 0, 0, msGeomConst_2pi), m_srcModel->Is3d (), *m_srcModel));
    holeAgenda.Insert (eeh);

    ASSERT_TRUE (SUCCESS == GroupedHoleHandler::CreateGroupedHoleElement (eeh, solidEeh, holeAgenda));
    m_agenda.Insert (eeh);

    // Create text element...
    TextBlockPtr textBlock = TextBlock::Create (*m_srcModel);

    textBlock->AppendText (L"Hello!");
    ASSERT_TRUE (SUCCESS == TextHandlerBase::CreateElement (eeh, NULL, *textBlock));
    m_agenda.Insert (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            WriteSourceElements (IEditProperties* remapper)
    {
    EditElementHandleP first = m_agenda.GetFirstP ();
    EditElementHandleP end  = first + m_agenda.GetCount ();

    for (EditElementHandleP curr = first; curr < end ; curr++)
        {
        if (!curr->IsValid ())
            continue;

        if (remapper)
            PropertyContext::EditElementProperties (*curr, remapper);

        curr->AddToModel ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DoCopyTest ()
    {
    // Only want to call these once...in case of repeated tests...
    if (!m_isSetup)
        {
        _PrepareSource (); // Make sure source/destination are setup...
        _PrepareDestination ();

        m_isSetup = true;
        }

    m_agenda.Empty (); // Empty in case of multiple copy test using same source and destination...

    _CreateSourceElements ();

    EXPECT_FALSE (m_agenda.IsEmpty ()) << "Failed to create source element(s).\n";

    IllegalDependencyRemapper noDependenciesExpected;
    CopyContext copier (m_dstModel, noDependenciesExpected);

    _InitCopyOptions (copier);

    EditElementHandleP first = m_agenda.GetFirstP ();
    EditElementHandleP end  = first + m_agenda.GetCount ();

    for (EditElementHandleP curr = first; curr < end ; curr++)
        {
        if (!curr->IsValid ())
            continue;

        EXPECT_EQ (SUCCESS, copier.DoCopy (*curr)) << "Failed to copy element.\n";
        }

    for (EditElementHandleP curr = first; curr < end ; curr++)
        {
        if (!curr->IsValid ())
            continue;

        EXPECT_EQ (m_dstModel, curr->GetDgnModel ()) << "Failed copied element validation.\n";

        _ValidateElement (*curr);
        }
    }

}; // CopyContextTestFixtureBase

/*=================================================================================**//**
* Test color remap when elements are copied between files with different color tables.
* @bsiclass                                                     Brien.Bastings  10/10
+===============+===============+===============+===============+===============+======*/
struct          CopyContextTestColorRemap : CopyContextTestFixtureBase, IQueryProperties
{
public:

enum TestType
    {
    TABLE_INDEX_Same,           //! RGB of color table index is the same in both source/destination.
    TABLE_INDEX_Different,      //! RGB of color table index is different in source/destination.
    TABLE_INDEX_Background,     //! Test that background color index is never changed.

    EXTENDED_INDEX_Same,        //! RGB for extended color exists in source/destination, closest 0-255 match required remap.
    EXTENDED_INDEX_Different,   //! RGB for extended color exists with different extended index in source/destination.
    EXTENDED_INDEX_New,         //! RGB for extended color only exists in source.

    EXTENDED_BOOK_Same,         //! RGB for book color exists in source/destination at same extended index.
    EXTENDED_BOOK_Different,    //! RGB for book color exists with different book name in source/destination.
    EXTENDED_BOOK_New,          //! RGB for book color only exists in source.
    };

enum TestColorId
    {
    COLORID_Black,
    COLORID_White,
    COLORID_Red,
    COLORID_Green,
    COLORID_Blue,
    COLORID_Cyan,
    COLORID_Magenta,
    COLORID_Yellow, 
    COLORID_DarkGrey,  
    COLORID_MediumGrey,  
    COLORID_Violet,
    };

private:

bool            m_preserveTableIndex;
TestType        m_colorTest;

protected:

CopyContextTestColorRemap () : CopyContextTestFixtureBase (true, true, false)
    {
    m_colorTest = TABLE_INDEX_Same;
    m_preserveTableIndex = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          GetTestColor (TestColorId id)
    {
    static UInt32 s_rgbColors[] =
        {
        0x000000,   // black    = 0
        0xFFFFFF,   // white    = 1
        0x0000FF,   // red      = 2
        0x00FF00,   // green    = 3
        0xFF0000,   // blue     = 4
        0xFFFF00,   // cyan     = 5
        0xFF00FF,   // magenta  = 6
        0x00FFFF,   // yellow   = 7
        0x555555,   // dgrey    = 8
        0x888888,   // mgrey    = 9
        0x800080,   // violet   = 10
        };

    return s_rgbColors[id];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PrepareSource () override
    {
    DgnColorMapP colorMap = (DgnColorMapP) DgnColorMap::GetForFile (m_srcModel->GetDgnProject ());

    // Setup some known color indices with rgb mis-match...
    colorMap->GetTbgrColorsP ()[0]   = GetTestColor (COLORID_Red);
    colorMap->GetTbgrColorsP ()[1]   = GetTestColor (COLORID_Green);
    colorMap->GetTbgrColorsP ()[2]   = GetTestColor (COLORID_Blue);

    // Setup a couple common entries for preserving index during closest rgb remap...
    colorMap->GetTbgrColorsP ()[3]   = GetTestColor (COLORID_Violet);
    colorMap->GetTbgrColorsP ()[4]   = GetTestColor (COLORID_Violet);

    // Create some known rgb values so we can test for exact match with extended color's "closest" match index...
    colorMap->GetTbgrColorsP ()[5]   = GetTestColor (COLORID_Cyan);
    colorMap->GetTbgrColorsP ()[6]   = GetTestColor (COLORID_Yellow);
    colorMap->GetTbgrColorsP ()[7]   = GetTestColor (COLORID_Magenta);

    // setup different background colors...
    colorMap->GetTbgrColorsP ()[255] = GetTestColor (COLORID_MediumGrey);

    // Create some common rgb colors ahead of time to control extended color index...
    DgnColors& colors = m_srcModel->Colors();
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Red)),    NULL, NULL);
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Green)),  NULL, NULL);
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Blue)),   NULL, NULL);
 
    // Create some named rgb colors...
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Cyan)),   "BOOK1", "CYAN");
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Yellow)), "BOOK1", "YELLOW");

    // Create rgb/book colors that only exist in source...
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Violet)), NULL, NULL);
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Magenta)), "BOOK1", "MAGENTA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PrepareDestination () override
    {
    DgnColorMapP colorMap = (DgnColorMapP) DgnColorMap::GetForFile (m_dstModel->GetDgnProject ());

    // Setup some known color indices with rgb mis-match...
    colorMap->GetTbgrColorsP ()[0]   = GetTestColor (COLORID_Green);
    colorMap->GetTbgrColorsP ()[1]   = GetTestColor (COLORID_Blue);
    colorMap->GetTbgrColorsP ()[2]   = GetTestColor (COLORID_Red);

    // Setup a couple common entries for preserving index during closest rgb remap...
    colorMap->GetTbgrColorsP ()[3]   = GetTestColor (COLORID_Violet);
    colorMap->GetTbgrColorsP ()[4]   = GetTestColor (COLORID_Violet);

    // Create some known rgb values so we can test for exact match with extended color's "closest" match index...
    colorMap->GetTbgrColorsP ()[5]   = GetTestColor (COLORID_Cyan);
    colorMap->GetTbgrColorsP ()[6]   = GetTestColor (COLORID_Yellow);
    colorMap->GetTbgrColorsP ()[7]   = GetTestColor (COLORID_Magenta);

    // setup different background colors...
    colorMap->GetTbgrColorsP ()[255] = GetTestColor (COLORID_DarkGrey);

    // Create some common rgb colors ahead of time to control extended color index...
    DgnColors& colors = m_dstModel->Colors();
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Red)),   NULL, NULL);
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Blue)),  NULL, NULL);
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Green)), NULL, NULL);

    // Create some named rgb colors...
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Cyan)),   "BOOK1", "CYAN");
    colors.CreateElementColor (IntColorDef (GetTestColor (COLORID_Yellow)), "BOOK2", "YELLOW");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _InitCopyOptions (CopyContextR copyContext) override
    {
    copyContext.SetKeepRefColorIndexOnCopy (m_preserveTableIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _CreateSourceElements () override
    {
    ElementPropertiesSetterPtr remapper = ElementPropertiesSetter::Create ();

    remapper->SetChangeEntireElement (true);
    remapper->SetColor (GetSourceElementColor ());

    CreateDefaultTestElements ();
    WriteSourceElements (remapper.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          GetSourceElementColor ()
    {
    switch (m_colorTest)
        {
        default:
        case TABLE_INDEX_Same:
            return 4;

        case TABLE_INDEX_Different:
            return 0;

        case TABLE_INDEX_Background:
            return 255;

        case EXTENDED_INDEX_Same:
            return m_srcModel->Colors().CreateElementColor (IntColorDef (GetTestColor (COLORID_Red)),     NULL, NULL);

        case EXTENDED_INDEX_Different:
            return m_srcModel->Colors().CreateElementColor (IntColorDef (GetTestColor (COLORID_Green)),   NULL, NULL);

        case EXTENDED_INDEX_New:
            return m_srcModel->Colors().CreateElementColor (IntColorDef (GetTestColor (COLORID_Violet)),  NULL, NULL);

        case EXTENDED_BOOK_Same:
            return m_srcModel->Colors().CreateElementColor (IntColorDef (GetTestColor (COLORID_Cyan)),    NULL, NULL);

        case EXTENDED_BOOK_Different:
            return m_srcModel->Colors().CreateElementColor (IntColorDef (GetTestColor (COLORID_Yellow)),  NULL, NULL);

        case EXTENDED_BOOK_New:
            return m_srcModel->Colors().CreateElementColor (IntColorDef (GetTestColor (COLORID_Magenta)), NULL, NULL);
        }
    }

#ifdef DGNV10FORMAT_CHANGES_WIP

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    EachColorCallback (EachColorArg& arg) override
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    DgnColorMapP srcColorMap = DgnColorMap::GetForFile (m_srcModel->GetDgnProject ());
    DgnColorMapP dstColorMap = DgnColorMap::GetForFile (m_dstModel->GetDgnProject ());

    IntColorDef  srcColorDef, dstColorDef;
    UInt32       srcColorIndex, dstColorIndex;
    bool         srcIsTrueColor, dstIsTrueColor;
    WString     srcBookName, dstBookName;
    WString     srcEntryName, dstEntryName;

    EXPECT_EQ (SUCCESS, srcColorMap->ExtractElementColorInfo (&srcColorDef, &srcColorIndex, &srcIsTrueColor, &srcBookName, &srcEntryName, GetSourceElementColor (), *m_srcModel->GetDgnProject ()));
    EXPECT_EQ (SUCCESS, dstColorMap->ExtractElementColorInfo (&dstColorDef, &dstColorIndex, &dstIsTrueColor, &dstBookName, &dstEntryName, arg.GetStoredValue (), *m_dstModel->GetDgnProject ()));

    EXPECT_EQ (srcIsTrueColor, dstIsTrueColor); // Expect color to remain either color table index or extended index...

    switch (m_colorTest)
        {
        case TABLE_INDEX_Same:
            {
            EXPECT_EQ (dstIsTrueColor, false);
            EXPECT_EQ (srcColorIndex, dstColorIndex); // Expect index to be preserved...
            EXPECT_EQ (srcColorDef.m_int, dstColorDef.m_int); // Expect rgb to be the same...

            break;
            }

        case TABLE_INDEX_Different:
            {
            EXPECT_EQ (dstIsTrueColor, false);

            if (m_preserveTableIndex)
                {
                EXPECT_EQ (srcColorIndex, dstColorIndex); // Expect index to be preserved...
                EXPECT_NE (srcColorDef.m_int, dstColorDef.m_int); // Expect rgb to be different...
                }
            else
                {
                EXPECT_NE (srcColorIndex, dstColorIndex); // Expect index to be different...
                EXPECT_EQ (srcColorDef.m_int, dstColorDef.m_int); // Expect rgb to be the same...
                }

            break;
            }

        case TABLE_INDEX_Background:
            {
            EXPECT_EQ (dstIsTrueColor, false);
            EXPECT_EQ (srcColorIndex, dstColorIndex); // Expect index to be preserved...
            EXPECT_NE (srcColorDef.m_int, dstColorDef.m_int); // Expect rgb to be different...
            break;
            }

        case EXTENDED_BOOK_Same:
        case EXTENDED_BOOK_Different:
        case EXTENDED_BOOK_New:
            {
            EXPECT_STREQ (srcBookName.c_str (), dstBookName.c_str ());
            EXPECT_STREQ (srcEntryName.c_str (), dstEntryName.c_str ());

            // Fall through...
            }

        case EXTENDED_INDEX_Same:
        case EXTENDED_INDEX_Different:
        case EXTENDED_INDEX_New:
            {
            EXPECT_EQ (dstIsTrueColor, true);
            EXPECT_EQ (srcColorDef.m_int, dstColorDef.m_int); // Expect rgb to be the same always...

            IntColorDef  closeColorDef;

            EXPECT_EQ (SUCCESS, dstColorMap->ExtractElementColorInfo (&closeColorDef, NULL, NULL, NULL, NULL, dstColorIndex, *m_dstModel->GetDgnProject ()));
            EXPECT_EQ (closeColorDef.m_int, dstColorDef.m_int); // Expect rgb of "closest" match from fixed color table to be the same (exact match)...
            break;
            }
        }
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ElementProperties GetQueryPropertiesMask () override
    {
    return ELEMENT_PROPERTY_Color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _ValidateElement (ElementHandleCR eh) override
    {
    PropertyContext::QueryElementProperties (eh, this);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DoColorTest (TestType testType, bool preserveIndex = false)
    {
    m_preserveTableIndex = preserveIndex;
    m_colorTest = testType;
    DoCopyTest ();
    }

}; // CopyContextTestColorRemap

/*---------------------------------------------------------------------------------**//**
* Tests color remapping when copying between files.
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CopyContextTestColorRemap, CopyContextRemapColor)
    {
    // Test with table color index remapping disallowed...
    DoColorTest (TABLE_INDEX_Same, true);
    DoColorTest (TABLE_INDEX_Different, true);
    DoColorTest (TABLE_INDEX_Background, true);

    // Test with table color index remapping allowed...
    DoColorTest (TABLE_INDEX_Same);
    DoColorTest (TABLE_INDEX_Different);
    DoColorTest (TABLE_INDEX_Background);

    // Test un-named extended colors...
    DoColorTest (EXTENDED_INDEX_Same);
    DoColorTest (EXTENDED_INDEX_Different);
    DoColorTest (EXTENDED_INDEX_New);

    // Test named extended colors...
    DoColorTest (EXTENDED_BOOK_Same);
    DoColorTest (EXTENDED_BOOK_Different);
    DoColorTest (EXTENDED_BOOK_New);
    }

/*=================================================================================**//**
* Test level remap when elements are copied between files with different level tables.
* @bsiclass                                                     Brien.Bastings  10/10
+===============+===============+===============+===============+===============+======*/
struct          CopyContextTestLevelRemap : CopyContextTestFixtureBase, IQueryProperties
{
private:

LevelId         m_firstCreatedLevelId;
WChar         m_levelName[MAX_LINKAGE_STRING_LENGTH];

protected:

CopyContextTestLevelRemap () : CopyContextTestFixtureBase (true, true, false)
    {
    m_firstCreatedLevelId = 0;
    m_levelName[0] = '\0';
    }

void CreateLevel (LevelId* levelId, DgnLevels& levelTable, WCharCP levelName)
    {
    DgnLevels::Level level (levelTable.QueryHighestId()+1, "CCTEST_Level1", DgnLevels::Level::Flags());
    levelTable.InsertLevel (level);
    ASSERT_TRUE (levelTable.QueryLevelById (level.GetLevelId()).IsValid()) << "Failed to create source level.\n";
    if (levelId)
        *levelId = level.GetLevelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PrepareSource () override
    {
    DgnLevels levelCache = DGN_TABLE_LEVEL_FOR_MODEL(m_srcModel);
    CreateLevel (&m_firstCreatedLevelId, levelCache, L"CCTEST_Level1");
    CreateLevel (NULL, levelCache, L"CCTEST_Level2");
    CreateLevel (NULL, levelCache, L"CCTEST_Level3");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PrepareDestination () override
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _InitCopyOptions (CopyContextR copyContext) override
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _CreateSourceElements () override
    {
    CreateDefaultTestElements ();

    DgnLevels::Level level = DGN_TABLE_LEVEL_FOR_MODEL(m_srcModel).QueryLevelByName (Utf8String(m_levelName).c_str()); // string conversion
    ASSERT_TRUE (level.IsValid ()) << "Failed to find source level.";

    ElementPropertiesSetterPtr remapper = ElementPropertiesSetter::Create ();

    remapper->SetChangeEntireElement (true);
    remapper->SetLevel (level.GetLevelId ());

    WriteSourceElements (remapper.get ());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    EachLevelCallback (EachLevelArg& arg) override
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    ASSERT_TRUE (LEVEL_DEFAULT_LEVEL_ID != arg.GetStoredValue ());

    DgnLevels::Level dstLevel = DGN_TABLE_LEVEL_FOR_MODEL(m_dstModel).QueryLevelById (arg.GetStoredValue ());

    ASSERT_TRUE (dstLevel.IsValid ()) << "Failed to find destination level.";
    EXPECT_STREQ (WString (dstLevel.GetName(), true).c_str(), m_levelName) << "Failed source/destination level name match.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ElementProperties GetQueryPropertiesMask () override
    {
    return ELEMENT_PROPERTY_Level;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _ValidateElement (ElementHandleCR eh) override
    {
    PropertyContext::QueryElementProperties (eh, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DoLevelTest (WCharCP levelName)
    {
    wcscpy (m_levelName, levelName);
    DoCopyTest ();
    }

}; // CopyContextTestLevelRemap

/*---------------------------------------------------------------------------------**//**
* Tests level remapping when copying between files.
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CopyContextTestLevelRemap, CopyContextRemapLevel)
    {
    DoLevelTest (L"CCTEST_Level1");
    DoLevelTest (L"CCTEST_Level2");
    DoLevelTest (L"CCTEST_Level3");
    }

#if defined FOREIGN_FORMAT_REORG
/*=================================================================================**//**
* Test tag remap when elements are copied between files.
* @bsiclass                                                     Brien.Bastings  10/10
+===============+===============+===============+===============+===============+======*/
struct          CopyContextTestTagRemap : CopyContextTestFixtureBase
{
private:

WChar         m_tagSetName[TAG_SET_NAME_MAX];

protected:

CopyContextTestTagRemap () : CopyContextTestFixtureBase (true, true, false) {m_tagSetName[0] = '\0';}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PrepareSource () override
    {
    DgnTagDefinition    tagDefs[3];
    DgnTagValue         value;

    value.type = MS_TAGTYPE_LINT;
    value.val.longVal = 1;

    memset (tagDefs, 0, sizeof (tagDefs));

    wcscpy (tagDefs[0].name, L"TAG1");
    wcscpy (tagDefs[1].name, L"TAG2");
    wcscpy (tagDefs[2].name, L"TAG3");

    tagDefs[0].value = value;
    tagDefs[1].value = value;
    tagDefs[2].value = value;

    EditElementHandle   eeh;

    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 3, L"TAGSET1", NULL, true, *m_srcModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();

    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 3, L"TAGSET2", NULL, true, *m_srcModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();

    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 3, L"TAGSET3", NULL, true, *m_srcModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();

    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 3, L"TAGSET4", NULL, true, *m_srcModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();

    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 3, L"TAGSET5", NULL, true, *m_srcModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PrepareDestination () override
    {
    DgnTagDefinition    tagDefs[4];
    DgnTagValue         value;

    value.type = MS_TAGTYPE_LINT;
    value.val.longVal = 2;

    memset (tagDefs, 0, sizeof (tagDefs));

    wcscpy (tagDefs[0].name, L"TAG1");
    wcscpy (tagDefs[1].name, L"TAG2");
    wcscpy (tagDefs[2].name, L"TAG3");
    wcscpy (tagDefs[3].name, L"TAG4");

    tagDefs[0].value = value;
    tagDefs[1].value = value;
    tagDefs[2].value = value;
    tagDefs[3].value = value;

    EditElementHandle   eeh;

    // Exact match with source...
    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 3, L"TAGSET1", NULL, true, *m_dstModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();

    // More entries than source...
    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 4, L"TAGSET2", NULL, true, *m_dstModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();

    // Less entries than source...
    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 2, L"TAGSET3", NULL, true, *m_dstModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();

    // Additional entries and different attribute ids than source...
    tagDefs[0].id = 1;
    tagDefs[1].id = 3;
    tagDefs[2].id = 2;
    tagDefs[3].id = 4;

    EXPECT_EQ (SUCCESS, TagSetHandler::Create (eeh, tagDefs, 4, L"TAGSET4", NULL, false, *m_dstModel->GetDgnProject ())) << "Failed to create tag set.\n";
    eeh.AddToModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _CreateSourceElements () override
    {
    DgnTextStylePtr     textStyle = DgnTextStyle::Create (L"TextStyle", *m_srcModel->GetDgnProject ());
    ITagCreateDataPtr   tagInfo;
    WChar             tagName[TAG_NAME_MAX];
    EditElementHandle   eeh;

    // All source tag sets have 3 sequential tags...
    for (int iTag = 0; iTag < 3; iTag++)
        {
        BeStringUtilities::Snwprintf (tagName, L"TAG%d", iTag+1);
        tagInfo = ITagCreateData::Create (tagName, m_tagSetName, *textStyle.get (), *m_srcModel->GetDgnProject ());
        ASSERT_FALSE (tagInfo.IsNull ()) << "Failed to create tag info.\n";

        // Create 2 tags for every entry to test that subsequent copies also work...
        EXPECT_EQ (SUCCESS, TagElementHandler::Create (eeh, NULL, *tagInfo.get (), *m_srcModel, m_srcModel->Is3d (), DPoint3d::From (0, 0, 0), RotMatrix::FromIdentity (), NULL));
        m_agenda.Insert (eeh);
        EXPECT_EQ (SUCCESS, TagElementHandler::Create (eeh, NULL, *tagInfo.get (), *m_srcModel, m_srcModel->Is3d (), DPoint3d::From (0, 0, 0), RotMatrix::FromIdentity (), NULL));
        m_agenda.Insert (eeh);
        }

    WriteSourceElements (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _ValidateElement (ElementHandleCR eh) override
    {
    int         setIndex = 0;

    EXPECT_EQ (1, BE_STRING_UTILITIES_SWSCANF (m_tagSetName, L"TAGSET%d", &setIndex));

    DgnTagSpec          dstTagSpec;
    DgnTagDefinition    dstTagDef;

    EXPECT_EQ (SUCCESS, TagElementHandler::Extract (dstTagSpec, eh, *m_dstModel)) << "Failed to extract tag spec.\n";
    EXPECT_EQ (SUCCESS, TagElementHandler::Extract (dstTagDef, eh, *m_dstModel)) << "Failed to extract tag defintion.\n";

    EditElementHandle   srcTagSetEeh;
    DgnTagDefinition    srcTagDef;

    EXPECT_EQ (SUCCESS, TagSetHandler::GetByName (srcTagSetEeh, m_tagSetName, *m_srcModel->GetDgnProject ())) << "Failed to find source tag set.\n";
    EXPECT_EQ (SUCCESS, TagSetHandler::ExtractTagDefByName (srcTagSetEeh, srcTagDef, dstTagSpec.tagName)) << "Failed to find source tag definition by name.\n";

    switch (setIndex)
        {
        case 3:
            {
            // Verify that tag set name conflict has been resolved...
            EXPECT_STREQ (dstTagDef.name, srcTagDef.name) << "Failed source/destination tag name match.";
            EXPECT_STRNE (dstTagSpec.set.setName, m_tagSetName) << "Failed source/destination tag set name remap.";
            EXPECT_STREQ (dstTagSpec.set.setName, L"TAGSET3_1") << "Failed source/destination tag set name remap.";
            EXPECT_EQ (dstTagDef.id, srcTagDef.id) << "Failed source/destination tag id match.";
            break;
            }

        case 4:
            {
            // Verify that tag attribute id has been remapped...
            EXPECT_STREQ (dstTagDef.name, srcTagDef.name) << "Failed source/destination tag name match.";
            EXPECT_STREQ (dstTagSpec.set.setName, m_tagSetName) << "Failed source/destination tag set name match.";

            switch (dstTagDef.id)
                {
                case 1:
                    EXPECT_EQ (1, srcTagDef.id) << "Failed source/destination tag id remap.";
                    break;

                case 2:
                    EXPECT_EQ (3, srcTagDef.id) << "Failed source/destination tag id remap.";
                    break;

                case 3:
                    EXPECT_EQ (2, srcTagDef.id) << "Failed source/destination tag id remap.";
                    break;

                case 4:
                    EXPECT_EQ (4, srcTagDef.id) << "Failed source/destination tag id remap.";
                    break;

                default:
                    assert (false);
                    break;
                }
            break;
            }

        default:
            {
            EXPECT_STREQ (dstTagDef.name, srcTagDef.name) << "Failed source/destination tag name match.";
            EXPECT_STREQ (dstTagSpec.set.setName, m_tagSetName) << "Failed source/destination tag set name match.";
            EXPECT_EQ (dstTagDef.id, srcTagDef.id) << "Failed source/destination tag id match.";
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DoTagTest (WCharCP tagSetName)
    {
    wcscpy (m_tagSetName, tagSetName);
    DoCopyTest ();
    }

}; // CopyContextTestTagRemap

/*---------------------------------------------------------------------------------**//**
* Tests tag set name/tag attrId remapping when copying between files.
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CopyContextTestTagRemap, CopyContextRemapTags)
    {
    DoTagTest (L"TAGSET1");
    DoTagTest (L"TAGSET2");
    DoTagTest (L"TAGSET3");
    DoTagTest (L"TAGSET4");
    DoTagTest (L"TAGSET5");

    // Test that subsequent clone operations give same result...
    DoTagTest (L"TAGSET1");
    DoTagTest (L"TAGSET2");
    DoTagTest (L"TAGSET3");
    DoTagTest (L"TAGSET4");
    DoTagTest (L"TAGSET5");
    }
#endif

/*=================================================================================**//**
* Test shared cell remap when elements are copied between files.
* @bsiclass                                                     Brien.Bastings  10/10
+===============+===============+===============+===============+===============+======*/
struct          CopyContextTestSharedCellRemap : CopyContextTestFixtureBase
{
private:

bool                m_resolveConflicts;
WChar             m_cellName[MAX_CELLNAME_LENGTH];
int                 m_anonymousIndex;
int                 m_expectedDependents;
bvector<ElementId>  m_scDefIds;

protected:

CopyContextTestSharedCellRemap () : CopyContextTestFixtureBase (true, true, false)
    {
    m_expectedDependents = 0;
    m_anonymousIndex = -1;
    m_resolveConflicts = false; 
    m_cellName[0] = '\0';
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/10
+---------------+---------------+---------------+---------------+---------------+------*/
int             GetCurrentSCIndex ()
    {
    int         scIndex = 0;

    EXPECT_EQ (1, BE_STRING_UTILITIES_SWSCANF (m_cellName, L"SC%d", &scIndex));

    return scIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            CreateSharedCellDef (WCharCP name, bool anonymous, DgnModelR model, bool is3d, double size, UInt32 color1, UInt32 color2)
    {
    ElementPropertiesSetterPtr remapper = ElementPropertiesSetter::Create ();

    remapper->SetChangeEntireElement (true);

    EditElementHandle eeh, childEeh;

    SharedCellDefHandler::CreateSharedCellDefElement (eeh, name, is3d, model);
    SharedCellDefHandler::SetAnonymous (eeh, anonymous);

    EXPECT_EQ (SUCCESS, LineHandler::CreateLineElement (childEeh, NULL, DSegment3d::From (-size, 0, 0, size, 0, 0), is3d, model));
    remapper->SetColor (color1);
    EXPECT_TRUE (remapper->Apply (childEeh));
    EXPECT_EQ (SUCCESS, SharedCellDefHandler::AddChildElement (eeh, childEeh));

    EXPECT_EQ (SUCCESS, LineHandler::CreateLineElement (childEeh, NULL, DSegment3d::From (0, -size, 0, 0, size, 0), is3d, model));
    remapper->SetColor (color2);
    EXPECT_TRUE (remapper->Apply (childEeh));
    EXPECT_EQ (SUCCESS, SharedCellDefHandler::AddChildElement (eeh, childEeh));

    EXPECT_EQ (SUCCESS, SharedCellDefHandler::AddChildComplete (eeh));

    eeh.AddToModel ();

    // Save source shared cell defs ids for created anonymous instances...
    if (m_srcModel == &model)
        m_scDefIds.push_back (eeh.GetElementCP ()->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PrepareSource () override
    {
    DgnColorMapP colorMap = (DgnColorMapP) DgnColorMap::GetForFile (m_srcModel->GetDgnProject ());

    // Setup some known color indices...
    colorMap->GetTbgrColorsP ()[1] = 0x0000FF; // Red   - Same as destination
    colorMap->GetTbgrColorsP ()[2] = 0x00FF00; // Green - Different from destination
    colorMap->GetTbgrColorsP ()[3] = 0xFF0000; // Blue  - Different from destination

    // Create shared cell def for exact match test...
    CreateSharedCellDef (L"SC1", false, *m_srcModel, true, 1.0, 1, 1);

    // Create shared cell def for geometry match test w/color remap required...
    CreateSharedCellDef (L"SC2", false, *m_srcModel, true, 1.0, 1, 2);

    // Create anonymous shared cell def for exact geom match test w/color mis-match...
    CreateSharedCellDef (L"SC3", false, *m_srcModel, true, 1.0, 1, 2);

    // Create shared cell def for geometry size mismatch...
    CreateSharedCellDef (L"SC4", false, *m_srcModel, true, 1.0, 1, 1);

    // Create shared cell def that doesn't exist in destination...
    CreateSharedCellDef (L"SC5", false, *m_srcModel, true, 1.0, 1, 1);

    // Create anonymous shared cell def for exact match test...
    CreateSharedCellDef (L"SC6", true, *m_srcModel, true, 1.0, 1, 1);

    // Create anonymous shared cell def for geometry match test w/color remap required...
    CreateSharedCellDef (L"SC7", true, *m_srcModel, true, 1.0, 1, 2);

    // Create anonymous shared cell def for exact geom match test w/color mis-match...
    CreateSharedCellDef (L"SC8", true, *m_srcModel, true, 1.0, 1, 2);

    // Create anonymous shared cell def for geometry size mismatch...
    CreateSharedCellDef (L"SC9", true, *m_srcModel, true, 1.0, 1, 1);

    // Create anonymous shared cell def that doesn't exist in destination...
    CreateSharedCellDef (L"SC10", true, *m_srcModel, true, 1.0, 1, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PrepareDestination () override
    {
    DgnColorMapP colorMap = (DgnColorMapP) DgnColorMap::GetForFile (m_dstModel->GetDgnProject ());

    // Setup some known color indices...
    colorMap->GetTbgrColorsP ()[1] = 0x0000FF; // Red   - Same as source
    colorMap->GetTbgrColorsP ()[2] = 0xFF0000; // Blue  - Different from source
    colorMap->GetTbgrColorsP ()[3] = 0x00FF00; // Green - Different from source

    // Create shared cell def for exact match test...
    CreateSharedCellDef (L"SC1", false, *m_dstModel, true, 1.0, 1, 1);

    // Create shared cell def for geometry match test w/color remap required...
    CreateSharedCellDef (L"SC2", false, *m_dstModel, true, 1.0, 1, 3);

    // Create shared cell def for exact geom match test w/color mis-match...
    CreateSharedCellDef (L"SC3", false, *m_dstModel, true, 1.0, 1, 2);

    // Create shared cell def for geometry size mismatch...
    CreateSharedCellDef (L"SC4", false, *m_dstModel, true, 2.0, 1, 1);

    // Create anonymous shared cell def for exact match test...
    CreateSharedCellDef (L"SC6", true, *m_dstModel, true, 1.0, 1, 1);

    // Create anonymous shared cell def for geometry match test w/color remap required...
    CreateSharedCellDef (L"SC7", true, *m_dstModel, true, 1.0, 1, 3);

    // Create anonymous shared cell def for exact geom match test w/color mis-match...
    CreateSharedCellDef (L"SC8", true, *m_dstModel, true, 1.0, 1, 2);

    // Create anonymous shared cell def for geometry size mismatch...
    CreateSharedCellDef (L"SC9", true, *m_dstModel, true, 2.0, 1, 1);

    m_anonymousIndex = 6;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _InitCopyOptions (CopyContextR copyContext) override
    {
    // NOTE: Default is SharedCellNameConflicts::HasDefId...None isn't important we was always try to match anonymous anyway...
    copyContext.SetSharedCellNameConflicts (m_resolveConflicts ? CopyContext::SharedCellNameConflicts::All : CopyContext::SharedCellNameConflicts::HasDefId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _CreateSourceElements () override
    {
    EditElementHandle eeh;

    SharedCellHandler::CreateSharedCellElement (eeh, NULL, m_cellName, NULL, NULL, NULL, m_srcModel->Is3d (), *m_srcModel);

    int         scIndex = GetCurrentSCIndex ();

    // Must add dependency for anonymous shared cells...
    if (scIndex >= m_anonymousIndex)
        SharedCellHandler::SetDefinitionID (eeh, m_scDefIds[scIndex-1]);

    EXPECT_EQ (SUCCESS, SharedCellHandler::CreateSharedCellComplete (eeh)); 
    m_agenda.Insert (eeh);

    WriteSourceElements (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _ValidateElement (ElementHandleCR eh) override
    {
    ISharedCellQuery* scInstQuery = dynamic_cast <ISharedCellQuery*> (&eh.GetHandler ());

    ASSERT_TRUE (NULL != scInstQuery) << "Unexpected element.";

    EditElementHandle scDefEh (scInstQuery->GetDefinition (eh, *eh.GetDgnProject ()));

    ASSERT_TRUE (scDefEh.IsValid ()) << "Unable to fine shared cell def.";

    ISharedCellQuery* scDefnQuery = dynamic_cast <ISharedCellQuery*> (&scDefEh.GetHandler ());

    ASSERT_TRUE (NULL != scDefnQuery) << "Unexpected element.";

    WChar     sCellName[MAX_CELLNAME_LENGTH], scDefName[MAX_CELLNAME_LENGTH];

    EXPECT_EQ (SUCCESS, scInstQuery->ExtractName (sCellName, MAX_CELLNAME_LENGTH, eh));
    EXPECT_EQ (SUCCESS, scDefnQuery->ExtractName (scDefName, MAX_CELLNAME_LENGTH, scDefEh));

    EXPECT_STREQ (sCellName, scDefName) << "Shared cell instance name doesn't match definition.";

    switch (GetCurrentSCIndex ())
        {
        case 3:
            EXPECT_STREQ (m_resolveConflicts ? L"SC3_1" : m_cellName, scDefName) << "Shared cell instance name remapped incorrectly.";
            break;

        case 4:
            EXPECT_STREQ (m_resolveConflicts ? L"SC4_1" : m_cellName, scDefName) << "Shared cell instance name remapped incorrectly.";
            break;

        default:
            EXPECT_STREQ (m_cellName, scDefName) << "Shared cell instance name doesn't match definition.";
            break;
        }

#if defined (WIP_V10)
    if (scDefnQuery->IsAnonymous (scDefEh))
        EXPECT_EQ (m_expectedDependents, scDefEh.GetElementRef ()->GetDependents (NULL, 0));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DoSharedCellTest (WCharCP cellName, int expectedDependents = 0, bool resolveConflicts = true)
    {
    wcscpy (m_cellName, cellName);
    m_expectedDependents = expectedDependents;
    m_resolveConflicts = resolveConflicts;
    DoCopyTest ();
    }

}; // CopyContextTestSharedCellRemap

/*---------------------------------------------------------------------------------**//**
* Tests shared cell remapping when copying between files.
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CopyContextTestSharedCellRemap, CopyContextRemapSharedCell)
    {
    // Test clone of named shared cells without resolving name conflicts...
    DoSharedCellTest (L"SC1", 0, false);
    DoSharedCellTest (L"SC2", 0, false);
    DoSharedCellTest (L"SC3", 0, false);
    DoSharedCellTest (L"SC4", 0, false);

    // Now test clone with name remapping/geometry compare...
    DoSharedCellTest (L"SC1",  1);
    DoSharedCellTest (L"SC2",  1);
    DoSharedCellTest (L"SC3",  1);
    DoSharedCellTest (L"SC4",  1);
    DoSharedCellTest (L"SC5",  1);
    DoSharedCellTest (L"SC6",  1);
    DoSharedCellTest (L"SC7",  1);
    DoSharedCellTest (L"SC8",  1);
    DoSharedCellTest (L"SC9",  1);
    DoSharedCellTest (L"SC10", 1);

    // Test that subsequent clone operations give same result...
    DoSharedCellTest (L"SC1",  2);
    DoSharedCellTest (L"SC2",  2);
    DoSharedCellTest (L"SC3",  2);
    DoSharedCellTest (L"SC4",  2);
    DoSharedCellTest (L"SC5",  2);
    DoSharedCellTest (L"SC6",  2);
    DoSharedCellTest (L"SC7",  2);
    DoSharedCellTest (L"SC8",  2);
    DoSharedCellTest (L"SC9",  2);
    DoSharedCellTest (L"SC10", 2);
    }
#endif