/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ConfigurationManager_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

#if !defined(BENTLEYCONFIG_OS_ANDROID) && !defined(BENTLEYCONFIG_OS_APPLE_IOS) && !defined(BENTLEYCONFIG_OS_WINRT)

#include <DgnPlatform/Tools/stringop.h>
#include <DgnPlatform/DesktopTools/MacroConfigurationAdmin.h>
#include <DgnPlatform/DesktopTools/MacroFileProcessor.h>
#include <Bentley/BeTextFile.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ConfigurationVariableLevel GetLevelFromString (WCharCP levelString)
    {
    if (0 == wcscmp (levelString, L"system"))
        return ConfigurationVariableLevel::System;

    if (0 == wcscmp (levelString, L"application"))
        return ConfigurationVariableLevel::Application;

    if (0 == wcscmp (levelString, L"organization"))
        return ConfigurationVariableLevel::Organization;

    if (0 == wcscmp (levelString, L"workspace"))
        return ConfigurationVariableLevel::WorkSpace;

    if (0 == wcscmp (levelString, L"workset"))
        return ConfigurationVariableLevel::WorkSet;

    if (0 == wcscmp (levelString, L"role"))
        return ConfigurationVariableLevel::Role;

    if (0 == wcscmp (levelString, L"user"))
        return ConfigurationVariableLevel::User;

    if (0 == wcscmp (levelString, L"predefined"))
        return ConfigurationVariableLevel::Predefined;

    if (0 == wcscmp (levelString, L"system env"))
        return ConfigurationVariableLevel::SysEnv;

    return (ConfigurationVariableLevel)-5;
    }

#if defined (BENTLEY_WIN32) && defined (UNUSED)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            OpenMsDebugFile (BeTextFilePtr& msDebugFile)
    {
    WCharP  directory = _wgetenv (L"TEMP");
    WString msDebugFileName;
    BeFileName::BuildName (msDebugFileName, NULL, directory, L"msdebug", L"txt");

    BeFileStatus    openStatus;
    msDebugFile = BeTextFile::Open (openStatus, msDebugFileName.c_str(), TextFileOpenType::Read, TextFileOptions::None, TextFileEncoding::CurrentLocale);

    ASSERT_TRUE (BeFileStatus::Success == openStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            CheckVersusMsDebugFile (MacroConfigurationAdmin& macroCfgAdmin)
    {
    int     missingErrors       = 0;
    int     expandErrors        = 0;
    int     expansionMismatches = 0;
    int     lockedMismatches    = 0;
    int     levelErrors         = 0;
    int     levelMismatches     = 0;

    WCharCP skipConfigVars[] = { L"MS_LIBRARY_PATH", L"_USTN_MSINFOPATH" };

    BeTextFilePtr   msDebugFile; 
    OpenMsDebugFile (msDebugFile);

    // read the msdebug file, paying no attention until we get to the "Summary"
    WString     textLine;
    while (TextFileReadStatus::Success == msDebugFile->GetLine (textLine))
        {
        if (WString::npos != textLine.find (L"Configuration Variable Summary"))
            break;
        }

    while (TextFileReadStatus::Success == msDebugFile->GetLine (textLine))
        {
        bool    expectedLocked = false;

        // lines we are looking for have a : and an =.
        size_t colonPos;
        size_t equalsPos;
        if ( (WString::npos == (colonPos = textLine.find (':'))) || (WString::npos == (equalsPos = textLine.find ('=', colonPos))) )
            continue;
        
        size_t lockedPos;
        if (WString::npos != (lockedPos = textLine.find (L"<Locked>", equalsPos)))
            expectedLocked = true;

        // extract and strip space before and after the three components we want.
        WString configVarName   = textLine.substr (0, colonPos);
        WString expectedLevel     = textLine.substr (colonPos + 1, (equalsPos - (colonPos+1)));
        WString expectedExpansion = textLine.substr (equalsPos + 1, (WString::npos == lockedPos) ? lockedPos : (lockedPos - (equalsPos+1)));
        configVarName.Trim();
        expectedLevel.Trim();
        expectedExpansion.Trim();
        if (0 == expectedExpansion.compare (L"(null)"))
            expectedExpansion.clear();

        ConfigurationVariableLevel  expectedVarLevel;
        if ((ConfigurationVariableLevel)-5 == (expectedVarLevel = GetLevelFromString (expectedLevel.c_str())))
            wprintf (L"Invalid level %ls\n", expectedLevel.c_str());

        // look up the configuration variable to make sure we have it.
        WString         tmpStr;
        MacroConfigurationAdmin::ExpandOptions   standardOpts;
        WCharCP         translation;
        if (NULL == (translation = macroCfgAdmin.GetMacroTranslation (configVarName.c_str(), tmpStr, ConfigurationVariableLevel::User)))
            {
            wprintf (L"Can't find definition for %ls\n", configVarName.c_str());
            missingErrors++;
            continue;
            }

        // check expansion.
        WString expansion;
        if (BSISUCCESS != macroCfgAdmin.ExpandMacro (expansion, translation, standardOpts))
            {
            wprintf (L"Can't expand translation %ls for macro %ls\n", translation, configVarName.c_str());
            expandErrors++;
            continue;
            }

        bool    skipThisOne = false;
        for (int iSkip=0; iSkip < _countof (skipConfigVars); iSkip++)
            {
            if (0 == configVarName.compare (skipConfigVars[iSkip]))
                {
                skipThisOne = true;
                break;
                }
            }

        if (!skipThisOne && (0 != expansion.compare (expectedExpansion.c_str())))
            {
            wprintf (L"Expand for macro %ls differs. Found [%ls], expected [%ls]\n", configVarName.c_str(), expansion.c_str(), expectedExpansion.c_str());
            expansionMismatches++;
            }

        // check locked status.
        bool    foundLocked;
        if (expectedLocked != (foundLocked = macroCfgAdmin.IsMacroLocked (configVarName.c_str())))
            {
            wprintf (L"Lock state for macro %ls differs. Expected %ls, got %ls\n", configVarName.c_str(), expectedLocked ? L"true" : L"false", foundLocked ? L"true" : L"false");
            lockedMismatches++;
            }

        // check level.
        ConfigurationVariableLevel  foundLevel;
        if (BSISUCCESS != macroCfgAdmin.GetMacroLevel (foundLevel, configVarName.c_str()))
            {
            wprintf (L"Cant get macro level for macro %ls\n", configVarName.c_str());
            levelErrors++;
            continue;
            }

        if (foundLevel != expectedVarLevel)
            {
            wprintf (L"Config Variable Level for macro %ls differs. Expected %ls, got %d\n", configVarName.c_str(), expectedLevel.c_str(), (int)foundLevel);
            levelMismatches++;
            }
        }

    ASSERT_TRUE (0 == missingErrors);
    ASSERT_TRUE (0 == expandErrors);
    ASSERT_TRUE (0 == expansionMismatches);
    ASSERT_TRUE (0 == lockedMismatches);
    ASSERT_TRUE (0 == levelErrors);
    ASSERT_TRUE (0 == levelMismatches);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    GetPredefinedMacrosFromMsDebug (MacroConfigurationAdmin& macroCfgAdmin)
    {
    BeTextFilePtr   msDebugFile; 
    OpenMsDebugFile (msDebugFile);

    // read the msdebug file, paying no attention until we get to the "Summary"
    WString     textLine;
    while (TextFileReadStatus::Success == msDebugFile->GetLine (textLine))
        {
        if (WString::npos != textLine.find (L"Configuration Variable Summary"))
            break;
        }

    while (TextFileReadStatus::Success == msDebugFile->GetLine (textLine))
        {
        // lines we are looking for have a : and an =.
        size_t colonPos;
        size_t equalsPos;
        if ( (WString::npos == (colonPos = textLine.find (':'))) || (WString::npos == (equalsPos = textLine.find ('=', colonPos))) )
            continue;
        
        size_t lockedPos = textLine.find (L"<Locked>", equalsPos);

        // extract and strip space before and after the three components we want.
        WString configVarName   = textLine.substr (0, colonPos);
        WString level     = textLine.substr (colonPos + 1, (equalsPos - (colonPos+1)));
        WString expansion = textLine.substr (equalsPos + 1, (WString::npos == lockedPos) ? lockedPos : (lockedPos - (equalsPos+1)));
        configVarName.Trim();
        level.Trim();
        expansion.Trim();
        if (0 == expansion.compare (L"(null)"))
            expansion.clear();

        ConfigurationVariableLevel  varLevel;
        if (ConfigurationVariableLevel::Predefined != (varLevel = GetLevelFromString (level.c_str())))
            continue;

        macroCfgAdmin.DefineBuiltinMacro (configVarName.c_str(), expansion.c_str());
        }
    }

#endif

#if defined (UNUSED)

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Barry.Bentley                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct MacroDebugOutput : IMacroDebugOutput
{
    BeTextFile*     m_debugFile;

MacroDebugOutput (BeTextFile* debugFile)
    {
    m_debugFile      = debugFile;
    }

virtual void        ShowDebugMessage (int indent,WCharCP format, ...) override
    {
    WString     message;
    va_list     ap;

    va_start (ap, format);
    message.VSprintf (format, ap);
    va_end (ap);

    wprintf (message.c_str());

    if (NULL != m_debugFile)
        m_debugFile->PrintfTo (false, message.c_str());
    }
};

#endif
/*=================================================================================**//**
* @bsiclass                                                     Umar.Hayat      07/16
+===============+===============+===============+===============+===============+======*/
struct ConfigurationManagerTest : public ::testing::Test
{
private:
    Dgn::ScopedDgnHost m_testHost;
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, VariableLevel)
    {
    ConfigurationManager::DefineVariable (L"ConfigurationManagerTest", L"abc", ConfigurationVariableLevel::User);
    WString value;
    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (value, L"ConfigurationManagerTest", ConfigurationVariableLevel::User) );
    EXPECT_TRUE( value == WString(L"abc") );
    EXPECT_TRUE(SUCCESS != ConfigurationManager::GetVariable(value, L"ConfigurationManagerTest", ConfigurationVariableLevel::System));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, Test1)
    {
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"One",                   L"TestValue1") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"Two",                   L"TestValue2") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"SameAsOne",             L"$(One)") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"Three",                 L"$(Two);$(One)") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"Four",                  L"$(Three)") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"SameAsTwo",             L"$(first($(three)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"HasQuotes",             L"Test\"$(One)\"Value") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"StartCycle",            L"$(HasCycle)") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"MidCycle",              L"$(StartCycle)") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"HasCycle",              L"$(MidCycle)") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"HasArgCycle",           L"$(first($(HasCycle)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"MissingCloseParen",     L"$(One") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"MissingOpenParen",      L"$(first $(One)") );

    WString returnVal;
    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"One") );
    EXPECT_STREQ(L"TestValue1" , returnVal.c_str() );

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"Two") );
    EXPECT_STREQ(L"TestValue2", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"SameAsOne") );
    EXPECT_STREQ(L"TestValue1", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"Three") );
    EXPECT_STREQ(L"TestValue2;TestValue1", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"Four") );
    EXPECT_STREQ(L"TestValue2;TestValue1", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"SameAsTwo") );
    EXPECT_STREQ(L"TestValue2", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"HasQuotes") );
    EXPECT_STREQ(L"Test\"TestValue1\"Value", returnVal.c_str());

    EXPECT_TRUE(SUCCESS != ConfigurationManager::GetVariable(returnVal, L"HasCycle"));
    EXPECT_TRUE(SUCCESS != ConfigurationManager::GetVariable(returnVal, L"MissingCloseParen"));
    EXPECT_TRUE(SUCCESS != ConfigurationManager::GetVariable(returnVal, L"MissingOpenParen"));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConfigurationManagerTest, DeferredExpansion)
    {
#if defined (THIS_IS_HANDLED_AT_CONFIGFILE_READ)
    WString returnVal;
    // test immediate vs. deferred expansion
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"Base",                  L"BaseValueAtBeginning") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"BaseImmediate",         L"${Base}") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"BaseDeferred",          L"$(Base)") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"Base",                  L"BaseValueAfterChange") );

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"Base") );
    EXPECT_STREQ(L"BaseValueAfterChange", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"BaseImmediate") );
    EXPECT_STREQ(L"BaseValueAtBeginning", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"BaseDeferred") );
    EXPECT_STREQ(L"BaseValueAfterChange", returnVal.c_str());
#endif
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, CheckOperators)
    {
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"FullFileSpec",          L"c:\\RootDirectory\\SubDirectory\\NestedDirectory\\TestFile.ext") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"BaseName",              L"$(basename($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"Device",                L"$(dev($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"DeviceDirectory",       L"$(devdir($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"Directory",             L"$(dir($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"Extension",             L"$(ext($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"FileName",              L"$(filename($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"FirstDirectory",        L"$(firstdirpiece($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"LastDirectory",         L"$(lastdirpiece($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"NoExtension",           L"$(noext($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"ParentDirectory",       L"$(parentdir($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"ParentDeviceDirectory", L"$(parentdevdir($(FullFileSpec)))") );
    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"BuildFileName",         L"$(build($(Device), $(Directory), $(BaseName), $(Extension)))") );

    WString returnVal;
    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"FullFileSpec"         ) );
    EXPECT_STREQ(L"c:\\RootDirectory\\SubDirectory\\NestedDirectory\\TestFile.ext", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"BaseName"             ) );
    EXPECT_STREQ(L"TestFile", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"Device"               ) );
    EXPECT_STREQ(L"c:", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"DeviceDirectory"      ) );
    EXPECT_STREQ(L"c:\\RootDirectory\\SubDirectory\\NestedDirectory\\", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"Directory"            ) );
    EXPECT_STREQ(L"\\RootDirectory\\SubDirectory\\NestedDirectory\\", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"Extension"            ) );
    EXPECT_STREQ(L".ext", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"FileName"             ) );
    EXPECT_STREQ(L"TestFile.ext", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"FirstDirectory"       ) );
    EXPECT_STREQ(L"RootDirectory", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"LastDirectory"        ) );
    EXPECT_STREQ(L"NestedDirectory", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"NoExtension"          ) );
    EXPECT_STREQ(L"c:\\RootDirectory\\SubDirectory\\NestedDirectory\\TestFile", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"ParentDirectory"      ) );
    EXPECT_STREQ(L"\\RootDirectory\\SubDirectory\\", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"ParentDeviceDirectory" ) );
    EXPECT_STREQ(L"c:\\RootDirectory\\SubDirectory\\", returnVal.c_str());

    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (returnVal, L"BuildFileName" ) );
    EXPECT_STREQ(L"c:\\RootDirectory\\SubDirectory\\NestedDirectory\\TestFile.ext", returnVal.c_str());
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, TestMonitoredVariables)
    {
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedBoolean", L"true") );
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedInteger", L"134") );
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedString",  L"PreDefined String") );

    static  bool                s_testBooleanPreDefined;
    static  IVariableMonitorP   s_tbpdMonitor;
    ConfigurationManager::MonitorBoolean (s_tbpdMonitor, s_testBooleanPreDefined, L"TestPreDefinedBoolean", false);
    ASSERT_TRUE (true == s_testBooleanPreDefined);
    ASSERT_TRUE (nullptr != s_tbpdMonitor);

    static  bool                s_testBooleanNotDefined;
    static  IVariableMonitorP   s_tbndMonitor;
    ConfigurationManager::MonitorBoolean (s_tbndMonitor, s_testBooleanNotDefined, L"TestNotDefinedBoolean", false);
    ASSERT_TRUE (false == s_testBooleanNotDefined);
    ASSERT_TRUE (nullptr != s_tbndMonitor);

    static  int                 s_testIntegerPreDefined;
    static  IVariableMonitorP   s_tipdMonitor;
    ConfigurationManager::MonitorInteger (s_tipdMonitor, s_testIntegerPreDefined, L"TestPreDefinedInteger", 222, 0, 10000);
    ASSERT_TRUE (134 == s_testIntegerPreDefined);
    ASSERT_TRUE (nullptr != s_tipdMonitor);

    static  int                 s_testIntegerNotDefined;
    static  IVariableMonitorP   s_tindMonitor;
    ConfigurationManager::MonitorInteger (s_tindMonitor, s_testIntegerNotDefined, L"TestNotDefinedInteger", 855, 0, 10000);
    ASSERT_TRUE (855 == s_testIntegerNotDefined);
    ASSERT_TRUE (nullptr != s_tindMonitor);

    static  WString             s_testStringPreDefined;
    static  IVariableMonitorP   s_tspdMonitor;
    ConfigurationManager::MonitorString (s_tspdMonitor, s_testStringPreDefined, L"TestPreDefinedString");
    ASSERT_TRUE (0 == s_testStringPreDefined.CompareTo (L"PreDefined String") );
    ASSERT_TRUE (nullptr != s_tspdMonitor);

    static  WString             s_testStringNotDefined;
    static  IVariableMonitorP   s_tsndMonitor;
    ConfigurationManager::MonitorString (s_tsndMonitor, s_testStringNotDefined, L"TestNotDefinedString");
    ASSERT_TRUE (s_testStringNotDefined.empty());
    ASSERT_TRUE (nullptr != s_tsndMonitor);

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedBoolean", L"0") );
    EXPECT_TRUE (false == s_testBooleanPreDefined);

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedInteger", L"256") );
    EXPECT_TRUE (256 == s_testIntegerPreDefined);

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedString",  L"Different String") );
    EXPECT_TRUE (0 == s_testStringPreDefined.CompareTo (L"Different String"));


    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestNotDefinedBoolean", L"true") );
    EXPECT_TRUE (true == s_testBooleanNotDefined);

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestNotDefinedInteger", L"813") );
    EXPECT_TRUE (813 == s_testIntegerNotDefined);

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestNotDefinedString",  L"Now Defined") );
    EXPECT_TRUE (0 == s_testStringNotDefined.CompareTo (L"Now Defined"));

    // stop monitoring the predefined ones and verify that the statics don't change.
    ConfigurationManager::RemoveMonitor (L"TestPreDefinedBoolean", *s_tbpdMonitor);
    ConfigurationManager::RemoveMonitor (L"TestPreDefinedInteger", *s_tipdMonitor);
    ConfigurationManager::RemoveMonitor (L"TestPreDefinedString", *s_tspdMonitor);

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedBoolean", L"on") );
    ASSERT_TRUE (false == s_testBooleanPreDefined);
    WString value;
    ASSERT_TRUE (SUCCESS == ConfigurationManager::GetVariable (value, L"TestPreDefinedBoolean") );
    ASSERT_TRUE (0 == value.CompareTo (L"on"));
    ASSERT_TRUE (ConfigurationManager::IsVariableDefinedAndTrue (L"TestPreDefinedBoolean") );

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedInteger", L"785") );
    ASSERT_TRUE (256 == s_testIntegerPreDefined);
    ASSERT_TRUE (SUCCESS == ConfigurationManager::GetVariable (value, L"TestPreDefinedInteger") );
    ASSERT_TRUE (0 == value.CompareTo (L"785"));

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"TestPreDefinedString",  L"Another String") );
    ASSERT_TRUE (0 == s_testStringPreDefined.CompareTo (L"Different String"));
    ASSERT_TRUE (SUCCESS == ConfigurationManager::GetVariable (value, L"TestPreDefinedString") );
    ASSERT_TRUE (0 == value.CompareTo (L"Another String"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Barry.Bentley                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestMonitor : IVariableMonitor
    {
    int     m_directChange;
    int     m_rootChange;
    int     m_directUndefine;
    int     m_rootUndefined;

    TestMonitor ()
        {
        m_directChange = 0;
        m_rootChange   = 0;
        m_directUndefine = 0;
        m_rootUndefined  = 0;
        }

    virtual void _VariableChanged (WCharCP variableName) override
        {
        m_directChange++;
        }
    virtual void _VariableRootChanged (WCharCP variableName, WCharCP rootVariableName) override
        {
        m_rootChange++;
        }
    virtual void _VariableUndefined (WCharCP variableName) override
        {
        m_directUndefine++;
        }
    virtual void _VariableRootUndefined (WCharCP variableName, WCharCP rootVariableName) override
        {
        m_rootUndefined++;
        }
    virtual void _MonitorStopped (WCharCP variableName) override
        {
        delete this;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, TestMonitoredDependencies)
    {
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test1", L"TestOneValue") );
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test2", L"$(Test1)") );
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test3", L"$(Test2)") );
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test4", L"$(Test3)") );
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test5", L"$(Test1)") );

    TestMonitor    *testMon1, *testMon2, *testMon3, *testMon4, *testMon5;
    ConfigurationManager::MonitorVariable (L"Test1", *(testMon1 = new TestMonitor()));
    ConfigurationManager::MonitorVariable (L"Test2", *(testMon2 = new TestMonitor()));
    ConfigurationManager::MonitorVariable (L"Test3", *(testMon3 = new TestMonitor()));
    ConfigurationManager::MonitorVariable (L"Test4", *(testMon4 = new TestMonitor()));
    ConfigurationManager::MonitorVariable (L"Test5", *(testMon5 = new TestMonitor()));

    ASSERT_TRUE (1 == testMon1->m_directChange);
    ASSERT_TRUE (1 == testMon2->m_directChange);
    ASSERT_TRUE (1 == testMon3->m_directChange);
    ASSERT_TRUE (1 == testMon4->m_directChange);
    ASSERT_TRUE (1 == testMon5->m_directChange);

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test1", L"NewValue"));
    ASSERT_TRUE (2 == testMon1->m_directChange);

    ASSERT_TRUE (1 == testMon2->m_directChange);
    ASSERT_TRUE (1 == testMon2->m_rootChange);

    ASSERT_TRUE (1 == testMon3->m_directChange);
    ASSERT_TRUE (1 == testMon3->m_rootChange);

    ASSERT_TRUE (1 == testMon4->m_directChange);
    ASSERT_TRUE (1 == testMon4->m_rootChange);

    ASSERT_TRUE (1 == testMon5->m_directChange);
    ASSERT_TRUE (1 == testMon5->m_rootChange);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, TestHandlingOfNulls)
    {
    WCharCP nullVar=NULL;
    WCharCP emptyVar=L"";

    ASSERT_TRUE( SUCCESS == ConfigurationManager::DefineVariable (L"ConfigurationManagerTest", L"abc", ConfigurationVariableLevel::User));
    ASSERT_TRUE( ERROR == ConfigurationManager::DefineVariable (nullVar, L"abc", ConfigurationVariableLevel::User));
    ASSERT_TRUE( ERROR == ConfigurationManager::DefineVariable (emptyVar, L"abc", ConfigurationVariableLevel::User));

    WString value;
    ASSERT_TRUE( SUCCESS == ConfigurationManager::GetVariable (value, L"ConfigurationManagerTest", ConfigurationVariableLevel::User) );
    ASSERT_TRUE( value == WString(L"abc") );
    ASSERT_TRUE( SUCCESS != ConfigurationManager::GetVariable (value, nullVar, ConfigurationVariableLevel::System) );
    ASSERT_TRUE( SUCCESS != ConfigurationManager::GetVariable (value, emptyVar, ConfigurationVariableLevel::System) );

    MacroConfigurationAdmin macroCfgAdmin;
    ASSERT_TRUE( SUCCESS == macroCfgAdmin.DefineBuiltinMacro (L"_ROOTDIR",  L"1"));
    ASSERT_TRUE( SUCCESS != macroCfgAdmin.DefineBuiltinMacro (nullVar,      L"1"));
    ASSERT_TRUE( SUCCESS != macroCfgAdmin.DefineBuiltinMacro (emptyVar,     L"1"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, Undefine)
    {
    WCharCP cfgVarName  = L"One";
    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"TestValue1"));
    ASSERT_TRUE(ConfigurationManager::IsVariableDefined(cfgVarName));

    // Undefine
    ASSERT_TRUE(SUCCESS == ConfigurationManager::UndefineVariable(cfgVarName));
    ASSERT_FALSE(ConfigurationManager::IsVariableDefined(cfgVarName));

    // Redefine
    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"TestValue_NewValue"));
    ASSERT_TRUE(ConfigurationManager::IsVariableDefined(cfgVarName));
    WString value;
    ASSERT_TRUE(SUCCESS == ConfigurationManager::GetVariable(value, cfgVarName));
    ASSERT_STREQ(L"TestValue_NewValue" ,value.c_str());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, CheckBooleanStatus)
    {
    WCharCP cfgVarName  = L"TestBoolean";
    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"1"));
    EXPECT_TRUE(ConfigurationManager::IsVariableDefinedAndTrue(cfgVarName));

    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"on"));
    EXPECT_TRUE(ConfigurationManager::IsVariableDefinedAndTrue(cfgVarName));

    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"TRUE"));
    EXPECT_TRUE(ConfigurationManager::IsVariableDefinedAndTrue(cfgVarName));

    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"0"));
    EXPECT_TRUE(ConfigurationManager::IsVariableDefinedAndFalse(cfgVarName));

    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"OFF"));
    EXPECT_TRUE(ConfigurationManager::IsVariableDefinedAndFalse(cfgVarName));

    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"False"));
    EXPECT_TRUE(ConfigurationManager::IsVariableDefinedAndFalse(cfgVarName));

    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Umar.Hayat                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestConfigVarIterator: public IConfigVariableIteratorDelegate
{
    //! Called for each configuration variable.
    virtual void EachConfigVariable(WCharCP name, WCharCP value, ConfigurationVariableLevel level, bool locked) override
    {
        wprintf(L"Name = [%ls] value = [%ls]\n", name, value);
    }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, IterVariable)
    {
    WCharCP cfgVarName  = L"TestBoolean";
    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(cfgVarName, L"1"));
    EXPECT_TRUE(ConfigurationManager::IsVariableDefinedAndTrue(cfgVarName));

    TestConfigVarIterator iter;
    ASSERT_FALSE(SUCCESS == ConfigurationManager::IterateThroughVariables(&iter));
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Umar.Hayat                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SimpleTestMonitor : public SimpleConfigurationVariableMonitor
    {
    int     m_directChange;

    SimpleTestMonitor()
        {
        m_directChange = 0;
        }
    //! Called for each configuration variable.
    virtual void _UpdateState(WCharCP variableName) override
        {
        wprintf(L"Variable changed Name = [%ls]\n", variableName);
        ++m_directChange;
        }
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, SimpleMonitorTest)
    {
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test1", L"TestOneValue") );

    SimpleTestMonitor    *testMon1;
    ConfigurationManager::MonitorVariable (L"Test1", *(testMon1 = new SimpleTestMonitor()));

    ASSERT_EQ (1 , testMon1->m_directChange);

    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test1", L"NewValue"));
    ASSERT_EQ (2 , testMon1->m_directChange);
    
    ASSERT_TRUE(SUCCESS == ConfigurationManager::RemoveMonitor(L"Test1", *testMon1));

    ASSERT_TRUE(SUCCESS == ConfigurationManager::DefineVariable(L"Test1", L"NewValue"));
    ASSERT_EQ(2, testMon1->m_directChange);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ConfigurationManagerTest, StringExpand)
    {
    ASSERT_TRUE (SUCCESS == ConfigurationManager::DefineVariable (L"Test1", L"One") );
    // Method only check string inplace macros starting with $[
    ASSERT_FALSE(ConfigurationManager::StringContainsMacros(L"Test1"));

    WString strHavingMacro(L"$[Test1]:Two");
    ASSERT_TRUE(ConfigurationManager::StringContainsMacros(strHavingMacro.c_str()));

    ConfigurationManager::StringExpandMacros(strHavingMacro);
    ASSERT_STREQ(L"One:Two", strHavingMacro.c_str());

    }

#endif