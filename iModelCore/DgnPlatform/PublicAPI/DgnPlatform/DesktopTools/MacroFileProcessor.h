/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/MacroFileProcessor.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
typedef         bpair<MacroFileEntry*, bool>    T_FileEntry;
typedef         bvector<T_FileEntry>            T_FileStack;

enum class PreProcessorCommand
    {
    NotKeyword     = 0,
    Include        = 1,
    IfDefined,
    IfNotDefined,
    Else,
    ElseIf,
    EndIf,
    If,
    Level,
    Error,
    Undef,
    Lock,
    IfFeature,
    Echo,
    };

enum class MacroOperation
    {
    AddMacro         = 1,
    NewMacro,
    AppendMacro,
    AppendPath,
    PrependPath,
    EndOfFile,
    };

struct          MacroReadMonitor
    {
    virtual void    MacroDefine (MacroOperation defineType, WCharCP macroName, WCharCP operandValue, ConfigurationVariableLevel level, WCharCP sourceFileName) = 0;
    virtual void    MacroLock (WCharCP macroName, ConfigurationVariableLevel level, WCharCP sourceFileName) = 0;
    virtual void    MacroUndefine (WCharCP macroName, ConfigurationVariableLevel level, WCharCP sourceFileName) = 0;
    virtual void    NewProcessingLevel (ConfigurationVariableLevel level, WCharCP sourceFileName) = 0;
    virtual void    StartFile (WCharCP includedFileName, ConfigurationVariableLevel level, WCharCP sourceFileName) = 0;
    virtual void    EndFile (WCharCP includedFileName, ConfigurationVariableLevel level, WCharCP sourceFileName) = 0;
    };

struct          MacroLevelWarning
    {
    WString                     m_fileName;
    ConfigurationVariableLevel  m_newLevel;
    int                         m_lineNumber;
    MacroLevelWarning (WCharCP fileName, ConfigurationVariableLevel newLevel, int lineNumber);
    };

typedef         bvector<MacroLevelWarning> T_LevelWarningList;

typedef bool    (*FeatureAspectAllowedFunc)(WCharCP featureAspectName);

struct           MacroFileProcessor
{
private:
    bool                            m_currentFileStartReported;
    MacroFileEntry*                 m_currentFile;
    T_FileStack                     m_pendingFiles;
    T_LevelWarningList*             m_levelWarnings;

    bool                            m_eofHit;

    wint_t                          m_lastCharRead;
    wint_t                          m_pushedChar;
    wint_t                          m_lastNonWhiteSpaceChar;

    ConfigurationVariableLevel      m_processingLevel;
    MacroReadMonitor*               m_readMonitor;

    // methods used internally.
    void                            DoInclude (WStringR includeFile);
    void                            DoEcho (WStringR stringToEcho);
    wint_t                          GetOneChar (WStringP endFileMsg);
    wint_t                          ReadCharFromCurrentFile (bool doPreprocess, bool processEof, WStringP endFileMsg); // may return WEOF
    wint_t                          GetInputChar (bool doPreprocess, WStringP endFileMsg);
    MacroOperation                  GetMacroNameAndOperation (WStringR macroName, WStringP endFileMsg);
    void                            GetRestOfLine (WStringR restOfLine, WStringP endFileMsg);
    void                            TraceConditionalLevel (WCharCP explanation, int conditionalLevel);
    void                            SkipToEndOfConditional (bool allowElse, WCharCP fileName, int startLine, WStringP endFileMsg);
    void                            PreProcess (WStringP endFileMsg);
    void                            AddMacro (WCharCP macroName, MacroOperation op, WStringP endFileMsg);
    void                            NewMacro (WCharCP macroName, WStringP endFileMsg);
    void                            AppendMacro (WCharCP macroName, WCharCP sepString, bool doAppend, MacroOperation op, WStringP endFileMsg);
    void                            ProcessMacroFile (ConfigurationVariableLevel startingProcessLevel);
    void                            PushMacroFile (WCharCP fileName, MacroFileEntry* parent, bool report);
    void                            PopFile (WStringP endFileMsg);
    void                            ShowEndFileMessage (WStringP endFileMsg, bool extraIndent);
    void                            GetFileList (T_WStringVectorR fileList, WStringCR fileSpec);
    WCharCP                         GetCurrentFileName();
    WCharCP                         GetParentFileName(MacroFileEntry* mfe);
    int                             GetCurrentDepth();

protected:
    MacroConfigurationAdmin&        m_macroCfgAdmin;
    IMacroDebugOutput*              m_debugOutput;
    int                             m_debugLevel;
    FeatureAspectAllowedFunc        m_featureAspectFunc;

                        // overridden by MicroStation to control exit logic.
                        virtual void        _OnProcessError (WCharCP msg) {}
                        virtual bool        _GetSuspendDependencies () {return false;}

    //! Method called during the %include operation to provide a list of the file paths to be included for the
    //! given file specification.
    //! @param[out] fileList        Array of the absolute file paths to be included for the given file
    //!                             specification. The files will be proceesed in the same order as they appear
    //!                             in this array.
    //! @param[in]  includeFileSpec Expanded specification for the files to be included, coming from the
    //!                             configuration file currently being processed (as part of %include command).
    //!                             Example: "c:/MyFolder/*.cfg;d:/custom.cfg"
    //! @note This method can be overridden to add the support for the URIs on the remote file systems.
    //!       For instance, a custom implementation could download remote resources and fill the [fileList] vector
    //!       with file names of the local copies to be included.
    DGNPLATFORM_EXPORT  virtual void        _GetIncludeFileList (T_WStringVectorR fileList, WStringCR includeFileSpec);

    //! Checks for the existance of the given file specification (e.g., calling with "c:/MyFolder/*.cfg" will return
    //! true if any .cfg file exists in c:/MyFolder/).
    //! @param[in]  testFileSpec    Expanded specification for the files to be checked for existance, coming from the
    //!                             configuration file currently being processed (as part of %if exists () command).
    //!                             Example: "c:/MyFolder/*.cfg;d:/custom.cfg"
    //! @note This method can be overridden to add the support for the URIs on the remote file systems.
    DGNPLATFORM_EXPORT  virtual bool        _FileExists (WStringCR testFileSpec);

    //! Allows reinterpretation of ConfigurationVariableLevel for reading legacy configuration files.
    //! @param[in]  inputLevel  The level as read from the configuration file.
    //! @returns    The level as it should be stored in the macro table. 
    //! @note This method can be overridden to add the support for the URIs on the remote file systems.
    DGNPLATFORM_EXPORT  virtual ConfigurationVariableLevel  _ReinterpretConfigurationVariableLevel (ConfigurationVariableLevel inputLevel);

public:
    DGNPLATFORM_EXPORT                      MacroFileProcessor (MacroConfigurationAdmin& macroCfgAdmin, MacroReadMonitor* monitor, T_LevelWarningList* levelWarningsP = nullptr);
    DGNPLATFORM_EXPORT  virtual             ~MacroFileProcessor();

    DGNPLATFORM_EXPORT  BentleyStatus       ProcessTopLevelFile (WCharCP fileName, ConfigurationVariableLevel level, FeatureAspectAllowedFunc);
    DGNPLATFORM_EXPORT  void                PrintSummary ();
    DGNPLATFORM_EXPORT  void                SetDebugOutput (IMacroDebugOutput* debugOutput, int debugLevel);

                        void                FatalError (WCharCP errmsg);
                        PreProcessorCommand GetPreprocessorCommand (WCharCP word);

                        bool                FileExists (WStringCR testFileSpec);
};


END_BENTLEY_DGN_NAMESPACE

