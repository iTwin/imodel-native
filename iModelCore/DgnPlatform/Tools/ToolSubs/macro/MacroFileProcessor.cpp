/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/MacroFileProcessor.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <locale.h>
#include <DgnPlatform/Tools/stringop.h>
#include <DgnPlatform/DesktopTools/fileutil.h>
#include <DgnPlatform/DesktopTools/envvutil.h>
#include <DgnPlatform/DesktopTools/MacroFileProcessor.h>
#include <Bentley/BeFileListIterator.h>
#include "macro.h"

BEGIN_BENTLEY_DGN_NAMESPACE

enum
    {
    ERROR_EXIT                 = 2,
    MAX_LINE_LEN               = (4*1024),
    };

enum
    {
    TAB         = 0x09,
    LF          = 0x0A,
    CR          = 0x0D,
    CNTRL_Z     = 0x1a,
    };

struct MacroFileEntry
{
BeTextFilePtr           m_textFile;
WString                 m_fileName;
long                    m_lineNumber;
int                     m_endOfLineChar;
MacroFileEntry*         m_parent;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroFileEntry (WCharCP fileName, MacroFileEntry* parent) : m_fileName (fileName)
    {
    m_parent        = parent;
    m_lineNumber    = 0;
    m_endOfLineChar = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
~MacroFileEntry ()
    {
    // NULL the file, which closes it if necessary.
    m_textFile = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus    OpenFile ()
    {
    BeFileName  tmpName (m_fileName.c_str());
    WString     extension;
    tmpName.ParseName (nullptr, nullptr, nullptr, &extension);

    BeFileStatus    openStatus;

    // if it doesn't have an extension, try to open it with extension ".cfg" first.
    if (extension.empty())
        {
        tmpName.AppendExtension (L".cfg");

        m_textFile = BeTextFile::Open (openStatus, tmpName.GetName(), TextFileOpenType::Read, TextFileOptions::None);
        if (m_textFile.IsValid() && (BeFileStatus::Success == openStatus))
            {
            m_fileName.assign (tmpName.GetName());
            return BeFileStatus::Success;
            }
        }

    // either has extension, or wouldn't open with ".cfg" extension.
    m_textFile = BeTextFile::Open (openStatus, m_fileName.c_str(), TextFileOpenType::Read, TextFileOptions::None);

    return openStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
wint_t       GetOneChar ()
    {
    wint_t charRead = m_textFile->GetChar ();
    if ( (LF == charRead) || (CR == charRead) )
        {
        // do we know the endOfLineChar for this file yet?
        if (0 == m_endOfLineChar)
            m_endOfLineChar = charRead;

        if (charRead == m_endOfLineChar)
            m_lineNumber++;

        // always return LF for end of a line.
        charRead = LF;
        }

    return  charRead;
    }
};

END_BENTLEY_DGN_NAMESPACE


USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroFileProcessor::MacroFileProcessor (MacroConfigurationAdmin& macroCfgAdmin, MacroReadMonitor* readMonitor, T_LevelWarningList* levelWarningsP) : m_macroCfgAdmin (macroCfgAdmin)
    {
    m_readMonitor               = readMonitor;
    m_currentFileStartReported  = false;
    m_currentFile               = nullptr;
    m_eofHit                    = false;
    m_lastCharRead              = 0;
    m_pushedChar                = 0;
    m_processingLevel           = ConfigurationVariableLevel::Application;
    m_lastNonWhiteSpaceChar     = LF;
    m_debugLevel                = 0;
    m_debugOutput               = nullptr;
    m_featureAspectFunc         = nullptr;
    m_levelWarnings             = levelWarningsP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    MacroFileProcessor::SetDebugOutput (IMacroDebugOutput* debugOutput, int debugLevel)
    {
    m_debugOutput       = debugOutput;
    m_debugLevel        = debugLevel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroFileProcessor::~MacroFileProcessor ()
    {
    }

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   01/10
+===============+===============+===============+===============+===============+======*/
class MacroReadError
    {
    WString m_msg;

public:
    MacroReadError (WCharCP msg) : m_msg(msg){}
    WCharCP GetMessage() {return m_msg.c_str();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::FatalError (WCharCP errmsg)
    {
    // skip already processed files and closed files.
    while ( (nullptr != m_currentFile) && (m_currentFile->m_textFile.IsNull()) )
        PopFile (nullptr);

    WChar   fullErrorMessage[MAX_LINE_LEN];
    if (m_currentFile)
        {
        BeStringUtilities::Snwprintf (fullErrorMessage, L"%ls, [%ls], line %d\n", errmsg, m_currentFile->m_fileName.c_str(), m_currentFile->m_lineNumber);
        errmsg = fullErrorMessage;
        }

    // close files that haven't yet been closed.
    while ( (nullptr != m_currentFile) && (m_currentFile->m_textFile.IsValid()) )
        {
        // close the file.
        m_currentFile->m_textFile = nullptr;
        PopFile(nullptr);
        }

    throw MacroReadError (errmsg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::_GetIncludeFileList (T_WStringVectorR fileList, WStringCR includeFileSpec)
    {
    GetFileList (fileList, includeFileSpec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ConfigurationVariableLevel  MacroFileProcessor::_ReinterpretConfigurationVariableLevel (ConfigurationVariableLevel  inputLevel)
    {
    return inputLevel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    GetConfigurationVariableLevelFromString (ConfigurationVariableLevel& level, WCharCP levelValue)
    {
    if (nullptr == levelValue)
        return BSIERROR;

    size_t  length = wcslen (levelValue);
    if (length < 2)
        return BSIERROR;

    WString levelString (levelValue);
    levelString.Trim();
    levelString.ToLower();
    static WCharCP s_levelNames[] =
        {
        L"system",
        L"application",
        L"organization",
        L"workspace",
        L"workset",
        L"role",
        L"user"
        };
    for (int iLevel=0; iLevel < _countof (s_levelNames); iLevel++)
        {
        if (0 == wcsncmp (s_levelNames[iLevel], levelString.c_str(), length))
            {
            level = static_cast<ConfigurationVariableLevel>(iLevel);
            return BSISUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
MacroLevelWarning::MacroLevelWarning (WCharCP fileName, ConfigurationVariableLevel newLevel, int lineNumber)
    {
    m_fileName.assign (fileName);
    m_newLevel      = newLevel;
    m_lineNumber    = lineNumber;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::DoInclude (WStringR includeFile)
    {
    // get rid of leading and trailing spaces.
    includeFile.Trim();

    // Check for an optional level specification
    WString includeFileCopy (includeFile);
    includeFileCopy.ToUpper();

    ConfigurationVariableLevel newProcessLevel = m_processingLevel;
    size_t                     levelPos;
    if (WString::npos != (levelPos = includeFileCopy.rfind (L" LEVEL")))
        {
        includeFileCopy.erase (0, levelPos + 6);
        ConfigurationVariableLevel  possibleLevel = ConfigurationVariableLevel::Predefined;     // Predefined is an illegal value.
        if (BSISUCCESS == GetConfigurationVariableLevelFromString (possibleLevel, includeFileCopy.c_str()))
            {
            // Terminate the file name only if a valid level is found. Otherwise, assume " level" is part of the filename.
            includeFile.erase (levelPos);
            newProcessLevel = possibleLevel;
            }
        else
            {
            // See if the configuration level is specifed as a number.
            ConfigurationVariableLevel readLevel = (ConfigurationVariableLevel) evaluateSymbolAsInt (includeFileCopy.c_str(), ConfigurationVariableLevel::User, m_macroCfgAdmin, *this);
            // if we read it from as an Integer, allow the reinterpretation logic to change it, and warn.
            possibleLevel = _ReinterpretConfigurationVariableLevel (readLevel);
            
            if (m_macroCfgAdmin.IsValidCfgVarLevelForNewDefinition (possibleLevel))
                {
                if (nullptr != m_levelWarnings)
                    {
                    MacroLevelWarning thisWarning (m_currentFile->m_fileName.c_str(), readLevel, m_currentFile->m_lineNumber);
                    m_levelWarnings->push_back (thisWarning);
                    }

                // Terminate the file name only if a valid level is found. Otherwise, assume " level" is part of the filename.
                includeFile.erase (levelPos);
                newProcessLevel = possibleLevel;
                }
            }
        }

    WString includeName;
    // expand the macro at the existing processing level, then switch to processing level specified in the %include statement (if any).
    m_macroCfgAdmin.ExpandMacro (includeName, includeFile.c_str(), MacroExpandOptions (ConfigurationVariableLevel::User));
    m_processingLevel = newProcessLevel;

    T_WStringVector fileList;
    _GetIncludeFileList (fileList, includeName);

    MacroFileEntry* parentFile = m_currentFile;
    for (T_WStringVector::reverse_iterator iterator = fileList.rbegin(); iterator != fileList.rend(); )
        {
        WStringCR fileName = *iterator;

        ++iterator;

        // make the last one the current file.
        PushMacroFile (fileName.c_str(), parentFile, iterator == fileList.rend());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::DoEcho (WStringR stringToEcho)
    {
    WString output;
    // expand the macro at the existing processing level, then switch to processing level specified in the %include statement (if any).
    m_macroCfgAdmin.ExpandMacro (output, stringToEcho.c_str(), MacroExpandOptions (ConfigurationVariableLevel::User));
    wprintf (L"%s\n", output.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
wint_t       MacroFileProcessor::GetOneChar (WStringP endFileMsg)
    {
    // do we need to open the current file?
    if (!m_currentFile->m_textFile.IsValid())
        {
        if (BeFileStatus::Success != m_currentFile->OpenFile())
            {
            // if we don't stop, return WEOF which will go to next file.
            if (m_macroCfgAdmin._IsConfigVariableDefined(L"_USTN_MACRO_OPENFILE_SOFTFAILURE", ConfigurationVariableLevel::User))
                return WEOF;

            WChar msg[2*MAXFILELENGTH];
            BeStringUtilities::Snwprintf (msg, L"can't open macro file [%ls]", m_currentFile->m_fileName.c_str());

            FatalError (msg);
            }

        // report progress.
        if (nullptr != m_debugOutput)
            {
            // if we have a pending end message, that has to be displayed before showing the open.
            ShowEndFileMessage (endFileMsg, false);
            m_debugOutput->ShowDebugMessage (0, L"\n"); // leave blank line before.
            m_debugOutput->ShowDebugMessage (GetCurrentDepth(), L"Processing macro file [%ls]\n", m_currentFile->m_fileName.c_str());
            }
        }

    return m_currentFile->GetOneChar ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
wint_t      MacroFileProcessor::ReadCharFromCurrentFile (bool doPreprocessor, bool processEof, WStringP endFileMsg)
    {
    // NOTE: When it runs out of data in the current file, it goes back to the file that %included that file,
    //       or the next file that was %included by the include file, if that specifed a "path" config variable.
    wint_t   charRead;
    while (0 != (charRead = GetOneChar(endFileMsg)))
        {
        // we processEof when preProcessing or reading the rest of a comment line.
        if (processEof)
            {
            if (WEOF == charRead)
                {
                PopFile(endFileMsg);

                if (nullptr == m_currentFile)
                    break;

                // continue with the previous file.
                charRead = LF;
                break;
                }
            }

        // we don't preprocess (and don't process eof) if we are looking for the end of a conditional.
        if (!doPreprocessor)
            break;

        // is it a PreProcessor command?
        if ( ('%' == charRead) && (LF == m_lastNonWhiteSpaceChar) )
            {
            PreProcess (endFileMsg);
            m_lastNonWhiteSpaceChar = charRead = LF;
            }
        else
            {
            break;
            }
        }

    if (!isSpaceOrTab (charRead))
        m_lastNonWhiteSpaceChar = charRead;

    return charRead;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             10/89
+---------------+---------------+---------------+---------------+---------------+------*/
wint_t      MacroFileProcessor::GetInputChar (bool doPreprocessor, WStringP endFileMsg)
    {
    wint_t       previousNonWhiteSpaceChar;
    wint_t       charRead;
    for (charRead = 0; ;m_lastCharRead = charRead)
        {
        previousNonWhiteSpaceChar = m_lastNonWhiteSpaceChar;
        if (0 != (charRead = m_pushedChar))
            m_pushedChar = 0;
        else
            charRead = ReadCharFromCurrentFile (doPreprocessor, true, endFileMsg);

        switch (charRead)
            {
            case CNTRL_Z:
                charRead = LF;
                goto done;

            case WEOF:
                charRead = LF;
                m_eofHit = true;
                goto done;

            case TAB:
                charRead = ' ';  /* fall through */

            case ' ':
                if (LF == m_lastCharRead)
                    {
                    charRead = LF;
                    break;
                    }
                goto done;

            case '/':
                charRead = '\\';
                goto done;

            case '\\':
                {
                wint_t   nextChar;
                switch (nextChar = ReadCharFromCurrentFile(doPreprocessor, true, endFileMsg))
                    {
                    case '/':
                        charRead = '/';
                        goto done;

                    case    LF:
                        goto done;

                    default:
                        m_pushedChar = nextChar;
                        goto done;
                    }
                break;
                }

            // treat .ini section header as a comment.
            case '[':
                if (LF == previousNonWhiteSpaceChar)
                    {
                    // [ is comment til ] or end-of-line character.
                    while ( (LF != (charRead = ReadCharFromCurrentFile (doPreprocessor, true, endFileMsg))) && (WEOF != charRead) && ('[' != charRead) )
                        ;
                    charRead = LF;
                    }
                goto done;

            case '#':
                // # is comment til end-of-line character
                if (m_lastCharRead == '$')
                    goto done;

                // read until we hit end of line or end of file.
                while ( (LF != (charRead = ReadCharFromCurrentFile (false, true, endFileMsg))) && (WEOF != charRead) )
                    ;

                // check to see if WEOF hit immediately after comment */
                if (WEOF == charRead)
                    {
                    charRead = LF;
                    m_eofHit = true;
                    }

                goto done;

            case CR:
                charRead = LF;
                goto done;

            case LF:
                goto done;

            default:
                goto done;
            }
        }
done:
    m_lastCharRead = charRead;
    return  charRead;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroFileProcessor::ShowEndFileMessage (WStringP endFileMsg, bool extraIndent)
    {
    if ( (nullptr == endFileMsg) || endFileMsg->empty() )
        return;

    if (nullptr != m_debugOutput)
        {
        m_debugOutput->ShowDebugMessage (GetCurrentDepth() + (extraIndent ? 1 : 0), endFileMsg->c_str());
        if (extraIndent)
            m_debugOutput->ShowDebugMessage (0, L"\n");
        }

    endFileMsg->clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef _MSC_VER
#pragma warning(disable:4702)  // unreachable code warning triggered by final return statement in Release build only.
#endif
MacroOperation  MacroFileProcessor::GetMacroNameAndOperation (WStringR macroName, WStringP endFileMsg)
    {
    macroName.clear();

    // outer loop skips blank lines.
    bool    noNonBlank = true;
    wint_t  charRead = 0;
    while ( noNonBlank && !m_eofHit)
        {
        // build up the macro name up to the end of the line or the operator.
        while (!m_eofHit && ('=' != (charRead = GetInputChar (true, endFileMsg)))   // new rule.
                         && ('+' != charRead)                       // add macro
                         && ('>' != charRead)                       // append path to macro
                         && ('<' != charRead)                       // prepend path to macro
                         && (':' != charRead)                       // new macro (only if macro done)
                         && (CR  != charRead)
                         && (LF  != charRead))
            {
            noNonBlank = false;
            macroName.append (1, (WChar)charRead);
            }
        }

    // if we hit the end of a file while looking for a macro, put it out now.
    ShowEndFileMessage (endFileMsg, true);

    // terminate the macroName and remove white space.
    macroName.Trim ();

    if (m_eofHit)
        return  MacroOperation::EndOfFile;

    switch (charRead)
        {
        case    '=':
            return  MacroOperation::AddMacro;

        case    ':':
            return  MacroOperation::NewMacro;

        case    '+':
            return  MacroOperation::AppendMacro;

        case    '>':
            return  MacroOperation::AppendPath;

        case    '<':
            return  MacroOperation::PrependPath;
        }

    FatalError (L"macro syntax error");

    // Without this : warning C4715: ...  : not all control paths return a value
    return MacroOperation::EndOfFile;
    }
#ifdef _MSC_VER
#pragma warning(default:4702)
#endif
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             10/89
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::GetRestOfLine (WStringR restOfLine, WStringP endFileMsg)
    {
    restOfLine.clear();

    /* read in line */
    wint_t  charRead;
    while ( (LF != (charRead = GetInputChar(true, endFileMsg))) && (CR != charRead) )
        restOfLine.append (1, (WChar)charRead);

    restOfLine.Trim();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::TraceConditionalLevel (WCharCP explanation, int conditionalLevel)
    {
    if ( (m_debugLevel > 2) && (nullptr != m_debugOutput) )
        m_debugOutput->ShowDebugMessage (1+GetCurrentDepth(), L"%ls, [%ls], line %ld, depth = %d.\n", explanation, m_currentFile->m_fileName.c_str(), m_currentFile->m_lineNumber, conditionalLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::SkipToEndOfConditional (bool allowElse, WCharCP fileName, int startLine, WStringP endFileMsg)
    {
    int     conditionalLevel = 0;
    WString line;

    // if there's an End File message cued up, show before the TraceConditionalLevel.
    ShowEndFileMessage (endFileMsg, true);

    TraceConditionalLevel (L"Starting skip to endif", 0);
    for (;;)
        {
        line.clear();

        wint_t  charRead;
        while ( (LF != (charRead = GetInputChar (false, endFileMsg))) && (CR != charRead) )
            {
            if ( (WEOF == charRead) || (CNTRL_Z == charRead) )
                {
                WString errorMsg;
                errorMsg.Sprintf (L"unmatched %%endif for conditional at line %d of file %ls", startLine, fileName);
                FatalError (errorMsg.c_str());
                }

            line.append (1, (WChar)charRead);
            }

        // if we hit the of all files while in GetInputChar, fatal error.
        if (nullptr == m_currentFile)
            {
            WString errorMsg;
            errorMsg.Sprintf (L"unmatched %%endif for conditional at line %d of file %ls", startLine, fileName);
            FatalError (errorMsg.c_str());
            }

        // trim whitespace
        line.Trim();

        // if it isn't a PreProcessor comamnd, it's not the end, so just continue.
        if ('%' != line[0])
            continue;

        // skip past space between % and preprocessor keyword.
        size_t  firstNWAfterPercent;
        if (WString::npos == (firstNWAfterPercent = line.find_first_not_of (L" \t", 1)))
            continue;

        if (firstNWAfterPercent == line.find (L"if", firstNWAfterPercent, 2))
            TraceConditionalLevel (L"if", ++conditionalLevel);

        else if (firstNWAfterPercent == line.find (L"endif", firstNWAfterPercent, 5))
            {
            TraceConditionalLevel (L"endif", conditionalLevel--);
            if (conditionalLevel < 0)
                return;
            }

        // If we allow else, process it.
        else if (allowElse && (conditionalLevel == 0))
            {
            if (firstNWAfterPercent == line.find (L"else", firstNWAfterPercent, 4))
                return;
            else if (firstNWAfterPercent == line.find (L"elif", firstNWAfterPercent, 4))
                {
                line.erase (0, firstNWAfterPercent+4);
                if (evaluateSymbolAsBoolean (line.c_str(), ConfigurationVariableLevel::User, m_macroCfgAdmin, *this))
                    {
                    // if else condition is true, return, else keep looking for endif.
                    TraceConditionalLevel (L"elif", 0);
                    return;
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/00
+---------------+---------------+---------------+---------------+---------------+------*/
PreProcessorCommand     MacroFileProcessor::GetPreprocessorCommand (WCharCP word)
    {
    static WCharCP s_preProcessorCmds[] =
        {
        L"include",
        L"ifdef",
        L"ifndef",
        L"else",
        L"elif",
        L"endif",
        L"if",
        L"level",
        L"error",
        L"undef",
        L"lock",
        L"iffeature",
        L"echo",
        };

    for (int iCmd = 0; iCmd < _countof (s_preProcessorCmds); iCmd++)
        {
        if (0 == BeStringUtilities::Wcsicmp (word, s_preProcessorCmds[iCmd]))
            return (PreProcessorCommand) (iCmd+1);
        }

    return  PreProcessorCommand::NotKeyword;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     MacroFileProcessor::GetCurrentFileName ()
    {
    if (nullptr == m_currentFile)
        return nullptr;

    return m_currentFile->m_fileName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     MacroFileProcessor::GetParentFileName (MacroFileEntry* mfe)
    {
    if (nullptr == mfe->m_parent)
        return nullptr;

    return mfe->m_parent->m_fileName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::PreProcess (WStringP endFileMsg)
    {
    // start with a string of 80 characters (will resize if necessary).
    WString     line;
    line.reserve (80);

begin:
    // read the rest of the line (on entry to this method, the % PreProcessor leadin character has already been read).
    wint_t   previousChar = 0;
    for (wint_t  charRead = 0; LF != (charRead = ReadCharFromCurrentFile (false, false, endFileMsg)); )
        {
        if (WEOF == charRead)
            break;

        line.append (1, charRead);
        previousChar = charRead;
        }

    // check for continuation character, if we find one, read the next line, too.
    if ('\\' == previousChar)
        goto begin;

    // kill remainder of line starting with comment character.
    size_t  commentStart;
    if (WString::npos != (commentStart = line.find ('#')))
        line.erase (commentStart);

    // strip off leading and trailing spaces.
    line.Trim();

    // the first space or tab stops separates the preprocessor command from its argument (if there is one).
    WString     preprocArgument;
    size_t      preprocArgStart;
    if (WString::npos != (preprocArgStart = line.find_first_of (L" \t")))
        {
        preprocArgument = line.substr (preprocArgStart+1);
        line.erase (preprocArgStart);
        }

    // save current file name in case we need to report it in case of error.
    WString conditionalStartFileName (m_currentFile->m_fileName);
    long    conditionalStartLine = m_currentFile->m_lineNumber;

    // find the preprocessor command, if there is one.
    PreProcessorCommand preProcCmd = GetPreprocessorCommand (line.c_str());
    switch (preProcCmd)
        {
        case    PreProcessorCommand::Include:
            DoInclude (preprocArgument);
            break;

        case    PreProcessorCommand::IfDefined:
            if (!m_macroCfgAdmin.IsMacroDefinedFromLevel (preprocArgument.c_str(), ConfigurationVariableLevel::User))
                SkipToEndOfConditional (true, conditionalStartFileName.c_str(), conditionalStartLine, endFileMsg);
            break;

        case    PreProcessorCommand::IfNotDefined:
            if (m_macroCfgAdmin.IsMacroDefinedFromLevel (preprocArgument.c_str(), ConfigurationVariableLevel::User))
                SkipToEndOfConditional (true, conditionalStartFileName.c_str(), conditionalStartLine, endFileMsg);
            break;

        case    PreProcessorCommand::Else:
        case    PreProcessorCommand::ElseIf:
            SkipToEndOfConditional (false, conditionalStartFileName.c_str(), conditionalStartLine, endFileMsg);
            break;

        case    PreProcessorCommand::EndIf:
            break;

        case    PreProcessorCommand::If:
            if (!evaluateSymbolAsBoolean (preprocArgument.c_str(), ConfigurationVariableLevel::User, m_macroCfgAdmin, *this))
                SkipToEndOfConditional (true, conditionalStartFileName.c_str(), conditionalStartLine, endFileMsg);
            break;

        case    PreProcessorCommand::Level:
            {
            ConfigurationVariableLevel newLevel;
            if (BSISUCCESS != GetConfigurationVariableLevelFromString (newLevel, preprocArgument.c_str()))
                {
                ConfigurationVariableLevel  readLevel = (ConfigurationVariableLevel) evaluateSymbolAsInt (preprocArgument.c_str(), ConfigurationVariableLevel::User, m_macroCfgAdmin, *this);
                newLevel = _ReinterpretConfigurationVariableLevel (readLevel);
                if (nullptr != m_levelWarnings)
                    {
                    MacroLevelWarning thisWarning (m_currentFile->m_fileName.c_str(), readLevel, m_currentFile->m_lineNumber);
                    m_levelWarnings->push_back (thisWarning);
                    }
                }

            if (newLevel > ConfigurationVariableLevel::User)
                newLevel = ConfigurationVariableLevel::User;
            else if (newLevel < ConfigurationVariableLevel::System)
                newLevel = ConfigurationVariableLevel::System;
            m_processingLevel = newLevel;
            if (nullptr != m_readMonitor)
                m_readMonitor->NewProcessingLevel (newLevel, GetCurrentFileName());

            break;
            }

        case    PreProcessorCommand::Error:
            {
            WString     errorMsg;
            m_macroCfgAdmin.ExpandMacro (errorMsg, preprocArgument.c_str(), MacroExpandOptions (ConfigurationVariableLevel::User));
            FatalError (errorMsg.c_str());
            break;
            }

        case    PreProcessorCommand::Undef:
            if (nullptr != m_readMonitor)
                m_readMonitor->MacroUndefine (preprocArgument.c_str(), m_processingLevel, GetCurrentFileName());
            m_macroCfgAdmin._UndefineConfigVariable (preprocArgument.c_str());
            break;

        case    PreProcessorCommand::Lock:
            if (nullptr != m_readMonitor)
                m_readMonitor->MacroLock (preprocArgument.c_str(), m_processingLevel, GetCurrentFileName());
            m_macroCfgAdmin.LockMacro (preprocArgument.c_str());
            break;

        case    PreProcessorCommand::IfFeature:
            {
            if ( (nullptr == m_featureAspectFunc) ||(!m_featureAspectFunc (preprocArgument.c_str())) )
                SkipToEndOfConditional (true, conditionalStartFileName.c_str(), conditionalStartLine, endFileMsg);
            break;
            }

        case    PreProcessorCommand::Echo:
            DoEcho (preprocArgument);
            break;

        default:
            {
            WChar msg[MAX_LINE_LEN];

            BeStringUtilities::Snwprintf (msg, L"bad preprocessor command '%ls'", line.c_str());
            FatalError (msg);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             10/89
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::AddMacro (WCharCP macroName, MacroOperation macroOperation, WStringP endFileMsg)
    {
    WString     restOfLine;
    restOfLine.reserve (80);
    GetRestOfLine (restOfLine, endFileMsg);

    // we are setting immediate to false, but that is overridden by ${} expansions. We need to do those at the User (highest) level.
    MacroExpandOptions options (ConfigurationVariableLevel::User);
    options.SetImmediate (false);
    WString     expanded;
    m_macroCfgAdmin.ExpandMacro (expanded, restOfLine.c_str(), options);

    if (nullptr != m_readMonitor)
        m_readMonitor->MacroDefine (macroOperation, macroName, expanded.c_str(), m_processingLevel, GetCurrentFileName());

    m_macroCfgAdmin.DefineMacroWithDebugging (macroName, expanded.c_str(), m_processingLevel, 1+GetCurrentDepth(), m_debugOutput, m_debugLevel);
    }

/*---------------------------------------------------------------------------------**//**
* only add macro if it doesn't already exist
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::NewMacro (WCharCP macroName, WStringP endFileMsg)
    {
    WString     tmpString;

    // assigns new value to macroName only if that macro does not already exist (at any level).
    if (nullptr == m_macroCfgAdmin.GetMacroTranslation (macroName, tmpString, ConfigurationVariableLevel::User))
        {
        AddMacro (macroName, MacroOperation::NewMacro, endFileMsg);
        }
    else
        {
        // just get the rest of the line and throw it away.
        WString restOfLine;
        GetRestOfLine (restOfLine, endFileMsg);

        // make sure the macro defined (if it came from system env table)
        m_macroCfgAdmin.EnsureSysEnvDefinition (macroName, m_processingLevel, m_debugOutput, m_debugLevel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::AppendMacro (WCharCP macroName, WCharCP sepString, bool doAppend, MacroOperation macroOperation, WStringP endFileMsg)
    {
    WString     tmpString;
    WCharCP     currentMacroValue;
    if (nullptr == (currentMacroValue = m_macroCfgAdmin.GetMacroTranslation (macroName, tmpString, m_processingLevel)))
        {
        AddMacro (macroName, macroOperation, endFileMsg);
        return;
        }

    // get the value we want to append/peprend from the input stream.
    WString     appendValue;

    GetRestOfLine (appendValue, endFileMsg);

    MacroExpandOptions options (m_processingLevel);
    options.SetImmediate (false);

    WString     expandedAppendValue;
    m_macroCfgAdmin.ExpandMacro (expandedAppendValue, appendValue.c_str(), options);
    if (expandedAppendValue.empty())
        return;

    // if there is a current definition for macroName, combine the old and new according to the "doAppend" argument
    WString     newValue;
    if (0 != *currentMacroValue)
        {
        if (doAppend)
            {
            newValue.assign (currentMacroValue);
            newValue.append (sepString);
            newValue.append (expandedAppendValue);
            }
        else
            {
            // preprend
            newValue.assign (expandedAppendValue);
            newValue.append (sepString);
            newValue.append (currentMacroValue);
            }
        }
    else
        {
        newValue.assign (expandedAppendValue);
        }

    // macroCfgAdmin takes ownership of allocated newValue...
    if (nullptr != m_readMonitor)
        m_readMonitor->MacroDefine (macroOperation, macroName, appendValue.c_str(), m_processingLevel, GetCurrentFileName());

    m_macroCfgAdmin.DefineMacroWithDebugging (macroName, newValue.c_str(), m_processingLevel, 1+GetCurrentDepth(), m_debugOutput, m_debugLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   12/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::ProcessMacroFile (ConfigurationVariableLevel startingProcessLevel)
    {
    m_eofHit          = false;
    m_processingLevel = startingProcessLevel;
    WString             endFileMsg;
    WStringP            endFileMsgP = (nullptr == m_debugOutput) ? nullptr : &endFileMsg;

    for (;;)
        {
        WString             macroName;
        MacroOperation macroOp = GetMacroNameAndOperation (macroName, endFileMsgP);

        // if we're done, exit the loop.
        if (MacroOperation::EndOfFile == macroOp)
            break;

        switch (macroOp)
            {
            case    MacroOperation::AddMacro:
                AddMacro (macroName.c_str(), macroOp, endFileMsgP);
                break;

            case    MacroOperation::NewMacro:
                NewMacro (macroName.c_str(), endFileMsgP);
                break;

            case    MacroOperation::AppendMacro:
                AppendMacro (macroName.c_str(), L" ", true, macroOp, endFileMsgP);
                break;

            case    MacroOperation::AppendPath:
                AppendMacro (macroName.c_str(), WCSPATH_SEPARATOR, true, macroOp, endFileMsgP);
                break;

            case    MacroOperation::PrependPath:
                AppendMacro (macroName.c_str(), WCSPATH_SEPARATOR, false, macroOp, endFileMsgP);
                break;
            }

        // we defer this until here, otherwise the end file messages come before we report the result of the last line in the file.
        ShowEndFileMessage (endFileMsgP, true);
        }

    /* After processing file return level to highest (i.e. full translation) */
    m_processingLevel = ConfigurationVariableLevel::User;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroFileProcessor::ProcessTopLevelFile (WCharCP fileName, ConfigurationVariableLevel level, FeatureAspectAllowedFunc featureAspectFunc)
    {
    BeFileStatus    openStatus;
    BeTextFilePtr   tempOpen = BeTextFile::Open (openStatus, fileName, TextFileOpenType::Read, TextFileOptions::None, TextFileEncoding::CurrentLocale);

    // NULL out the filePtr, which has the effect of closing the temporarily opened file.
    tempOpen = nullptr;

    if (BeFileStatus::Success != openStatus)
        return (BentleyStatus) openStatus;

    if (_GetSuspendDependencies())
        m_macroCfgAdmin.SuspendDependencies();

    PushMacroFile (fileName, nullptr, true);
    BentleyStatus status = SUCCESS;

    // set the function that tests feature aspects for the %ifFeature conditional
    m_featureAspectFunc = featureAspectFunc;

    try
        {
        ProcessMacroFile (level);
        }
    catch (MacroReadError& err)
        {
        _OnProcessError(err.GetMessage());
        status = ERROR;
        }

    if (_GetSuspendDependencies())
        m_macroCfgAdmin.ResumeDependencies();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::PushMacroFile (WCharCP fileName, MacroFileEntry* parent, bool report)
    {
    MacroFileEntry*   mfe = new MacroFileEntry (fileName, parent);

    // put the current file onto the end of the list, and make it current.
    if (nullptr != m_currentFile)
        m_pendingFiles.push_back (T_FileEntry (m_currentFile, m_currentFileStartReported));

    m_currentFile = mfe;
    m_currentFileStartReported = report;

    if (report && (nullptr != m_readMonitor))
        m_readMonitor->StartFile (GetCurrentFileName(), m_processingLevel, GetParentFileName(mfe));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int         MacroFileProcessor::GetCurrentDepth ()
    {
    int depth = -1;
    for (MacroFileEntry* mfe = m_currentFile; nullptr != mfe; mfe = mfe->m_parent)
        depth++;

    return depth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::PopFile (WStringP endFileMessage)
    {
    // get the next file to go to.
    T_FileEntry mfe;
    if (!m_pendingFiles.empty())
        {
        mfe = *(m_pendingFiles.rbegin ());
        m_pendingFiles.pop_back ();
        }

    if (nullptr != m_readMonitor)
        m_readMonitor->EndFile (GetCurrentFileName(), m_processingLevel, GetParentFileName(m_currentFile));

    if (nullptr != endFileMessage)
        {
        if (endFileMessage->empty())
            endFileMessage->Sprintf (L"End of macro file [%ls]\n", m_currentFile->m_fileName.c_str());
        else
            {
            WString tmp;
            tmp.Sprintf (L"End of macro file [%ls]\n", m_currentFile->m_fileName.c_str());
            endFileMessage->append (tmp.c_str());
            }
        }

    if (nullptr != m_currentFile)
        delete m_currentFile;

    m_currentFile = mfe.first;
    m_currentFileStartReported = true; /* It will be reported by the next call */

    if ((nullptr != m_readMonitor) && (nullptr != mfe.first) && !mfe.second)
        m_readMonitor->StartFile (GetCurrentFileName(), m_processingLevel, GetParentFileName(m_currentFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroFileProcessor::PrintSummary ()
    {
    if (nullptr == m_debugOutput)
        return;

    m_debugOutput->ShowDebugMessage (0, L"\n\n#########################################\n");
    m_debugOutput->ShowDebugMessage (0, L"# Configuration Variable Summary        \n#");
    m_debugOutput->ShowDebugMessage (0, L"#########################################\n");
    m_debugOutput->ShowDebugMessage (0, L"");

    m_macroCfgAdmin.PrintAllMacros (*m_debugOutput, m_debugLevel);

    m_debugOutput->ShowDebugMessage (0, L"");
    m_debugOutput->ShowDebugMessage (0, L"#########################################\n");
    m_debugOutput->ShowDebugMessage (0, L"# End of Configuration Variable Summary #\n");
    m_debugOutput->ShowDebugMessage (0, L"#########################################\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool        fileCompareDescending (WString const& file1, WString const& file2)
    {
    return  file2.CompareToI (file1) > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Laimis.Vaitkus  10/92
+---------------+---------------+---------------+---------------+---------------+------*/
static WString makeAbsolutePath (BeFileNameCR fileName, WStringCR currentFilePath)
    {
    if (fileName.IsAbsolutePath ())
        {
        return fileName;
        }

    return BeFileName::GetDirectoryName (currentFilePath.c_str()).append (fileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nancy.McCall    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     MacroFileProcessor::GetFileList (T_WStringVectorR fileList, WStringCR fileSpec)
    {
    if (fileSpec.empty())
        {
        return;
        }

    // terminate string at path separator.
    size_t separatorPos = fileSpec.find (WCSPATH_SEPARATOR_CHAR);
    WString specFrontPart (fileSpec, 0, separatorPos);

    if (!specFrontPart.empty())
        {
        if (WString::npos != specFrontPart.find (L'*'))
            {
            // iterate files, but not in subdirectories.
            BeFileListIterator iterator (specFrontPart, false);
            T_WStringVector    subList;
            BeFileName         foundName;
            while (iterator.GetNextFileName (foundName) == SUCCESS)
                {
                subList.push_back (makeAbsolutePath (foundName, m_currentFile->m_fileName));
                }

            // sort the sublist in descending order, so when we push them using FOR_EACH they're in ascending order.
            std::sort (subList.begin(), subList.end(), fileCompareDescending);

            // add the sorted file names to the overall list.
            for (const auto& subListFileName : subList)
                {
                fileList.push_back (subListFileName);
                }
            }
        else
            {
            BeFileName fileName (specFrontPart.c_str());
            fileList.push_back (makeAbsolutePath (fileName, m_currentFile->m_fileName));
            }
        }

    if (WString::npos != separatorPos && separatorPos + 1 < fileSpec.length())
        {
        // if this was a path separated list, put the next batch in.
        GetFileList (fileList, fileSpec.substr (separatorPos + 1));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MacroFileProcessor::_FileExists (WStringCR testFileSpec)
    {
    T_WStringVector fileList;
    GetFileList (fileList, testFileSpec);

    for (const auto& fileName : fileList)
        {
        if (BeFileName::DoesPathExist (fileName.c_str()))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Laimis.Vaitkus  10/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MacroFileProcessor::FileExists (WStringCR testFileSpec)
    {
    return _FileExists (testFileSpec);
    }

