/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/MacroFileProcessor.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include    <DgnPlatform/DesktopTools/MacroConfigurationAdmin.h>
#include    <Bentley/BeTextFile.h>

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* Utility class that can read a MicroStation .cfg or pcf file.
* @bsiclass                                                     Keith.Bentley   01/10
+===============+===============+===============+===============+===============+======*/
struct          MacroFileEntry;
typedef         bvector<MacroFileEntry*> T_FileStack;

enum PreProcessorCommand
    {
    PREPROCESSOR_NotKeyword     = 0,
    PREPROCESSOR_Include        = 1,
    PREPROCESSOR_IfDefined,
    PREPROCESSOR_IfNotDefined,
    PREPROCESSOR_Else,
    PREPROCESSOR_ElseIf,
    PREPROCESSOR_EndIf,
    PREPROCESSOR_If,
    PREPROCESSOR_Level,
    PREPROCESSOR_Error,
    PREPROCESSOR_Undef,
    PREPROCESSOR_Lock,
    };

enum MacroOperation
    {
    OPERATION_AddMacro         = 1,
    OPERATION_NewMacro         = 2,
    OPERATION_AppendMacro      = 3,
    OPERATION_AppendPath       = 4,
    OPERATION_PrependPath      = 5,
    OPERATION_EOF
    };

class           MacroFileProcessor
{
private:
    MacroConfigurationAdmin&        m_macroCfgAdmin;

    MacroFileEntry*                 m_currentFile;
    T_FileStack                     m_pendingFiles;

    bool                            m_eofHit;

    WChar                           m_lastCharRead;
    WChar                           m_pushedChar;
    WChar                           m_lastNonWhiteSpaceChar;

    ConfigurationVariableLevel      m_processingLevel;


    // methods used internally.
    void                            DoInclude (WStringR includeFile);
    WChar                           GetOneChar ();
    WChar                           ReadCharFromCurrentFile (bool doPreprocess);
    WChar                           GetInputChar (bool doPreprocess);
    MacroOperation                  GetMacroNameAndOperation (WStringR macroName);
    void                            GetRestOfLine (WStringR restOfLine);
    void                            TraceConditionalLevel (WCharCP explanation, int conditionalLevel);
    void                            SkipToEndOfConditional (bool allowElse);
    void                            PreProcess ();
    void                            AddMacro (WCharCP macroName);
    void                            NewMacro (WCharCP macroName);
    void                            AppendMacro (WCharCP macroName, WCharCP sepString, bool doAppend);
    void                            ProcessMacroFile (ConfigurationVariableLevel startingProcessLevel);
    void                            PushMacroFile (WCharCP fileName);
    void                            PopFile ();
    void                            GetFileList (T_WStringVector& fileList, WCharCP fileName);

protected:
    BeTextFilePtr                   m_debugFile;
    int                             m_debugLevel;
                                    // overridden by MicroStation to control exit logic.
    virtual void                    _OnProcessError (WCharCP msg) {}

public:
    DESKTOP_TOOLS_EXPORT                          MacroFileProcessor (MacroConfigurationAdmin& macroCfgAdmin);
    DESKTOP_TOOLS_EXPORT  virtual                 ~MacroFileProcessor();

    DESKTOP_TOOLS_EXPORT  BentleyStatus           ProcessTopLevelFile (WCharCP fileName, ConfigurationVariableLevel level);
    DESKTOP_TOOLS_EXPORT  void                    PrintSummary ();
    DESKTOP_TOOLS_EXPORT  void                    SetDebugFile (BeTextFileP debugFile, int debugLevel=1);

                    void                    FatalError (WCharCP errmsg);
                    PreProcessorCommand     GetPreprocessorCommand (WCharCP word);
                    bool                    FileExists (WCharCP testFileSpec, ConfigurationVariableLevel processingLevel);

};


END_BENTLEY_DGN_NAMESPACE

