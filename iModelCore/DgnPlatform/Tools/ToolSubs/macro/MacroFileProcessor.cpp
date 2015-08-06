/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/MacroFileProcessor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdarg.h>
#include    <locale.h>
#include    <DgnPlatform/Tools/stringop.h>
#include    <DgnPlatform/DesktopTools/fileutil.h>
#include    <DgnPlatform/DesktopTools/envvutil.h>
#include    <DgnPlatform/DesktopTools/MacroFileProcessor.h>
#include    <Bentley/BeFileListIterator.h>
#include    "macro.h"
#undef DGN_PLATFORM_MT
#include    <RmgrTools/Tools/ToolSubs.h>

BEGIN_BENTLEY_DGN_NAMESPACE

enum
    {
    ERROR_EXIT                 = 2,
    MAX_LINE_LEN               = (4*1024),
    };

struct MacroFileEntry
{
BeTextFilePtr           m_textFile;
WString                 m_fileName;
long                    m_lineNumber;
int                     m_endOfLineChar;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroFileEntry (WCharCP fileName) : m_fileName (fileName)
    {
    m_lineNumber    = 0;
    m_endOfLineChar = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
~MacroFileEntry ()
    {
    // NULL the file, which closes it if necessary.
    m_textFile = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus    OpenFile ()
    {
    BeFileName  tmpName (m_fileName.c_str());
    WString     extension;
    tmpName.ParseName (NULL, NULL, NULL, &extension);

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
WChar       GetOneChar ()
    {
    WChar charRead = m_textFile->GetChar ();
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



BEGIN_EXTERN_C
extern ToolsFuncs       toolsFunctions;
END_EXTERN_C

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroFileProcessor::MacroFileProcessor (MacroConfigurationAdmin& macroCfgAdmin) : m_macroCfgAdmin (macroCfgAdmin)
    {
    m_debugLevel            = 0;
    m_currentFile           = NULL;
    m_eofHit                = false;
    m_lastCharRead          = 0;
    m_pushedChar            = 0;
    m_processingLevel       = ConfigurationVariableLevel::Appl;
    m_lastNonWhiteSpaceChar = LF;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    MacroFileProcessor::SetDebugFile (BeTextFileP debugFile, int debugLevel)
  {
  m_debugFile           = debugFile; 
  m_debugLevel          = debugLevel;
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
    while ( (NULL != m_currentFile) && (m_currentFile->m_textFile.IsNull()) )
        PopFile();

    WChar   fullErrorMessage[MAX_LINE_LEN];
    if (m_currentFile)
        {
        BeStringUtilities::Snwprintf (fullErrorMessage, L"%ls, [%ls], line %d\n", errmsg, m_currentFile->m_fileName.c_str(), m_currentFile->m_lineNumber);
        errmsg = fullErrorMessage;
        }

    // close files that haven't yet been closed.
    while ( (NULL != m_currentFile) && (m_currentFile->m_textFile.IsValid()) )
        {
        // close the file.
        m_currentFile->m_textFile = NULL;
        PopFile();
        }

    throw MacroReadError (errmsg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::DoInclude (WStringR includeFile)
    {
    // get rid of leading and trailing spaces.
    includeFile.Trim();

    WString     includeFileCopy (includeFile);

    includeFileCopy.ToUpper();

    // Check for an optional level specification
    ConfigurationVariableLevel newProcessLevel = m_processingLevel;
    size_t                     levelPos;
    if (WString::npos != (levelPos = includeFileCopy.rfind (L" LEVEL")))
        {
        includeFileCopy.erase (0, levelPos + 6);

        // terminate the file name only if a valid level is found.  Otherwise, assume " level" is part of the filename
        if ( (1 == BE_STRING_UTILITIES_SWSCANF (includeFileCopy.c_str(), L"%d", &newProcessLevel)) && m_macroCfgAdmin.IsValidCfgVarLevelForNewDefinition (newProcessLevel) )
            includeFile.erase (levelPos);
        }

    if (!m_macroCfgAdmin.IsValidCfgVarLevelForNewDefinition (newProcessLevel))
        newProcessLevel = m_processingLevel;

    WString includeName;
    m_macroCfgAdmin.ExpandMacro (includeName, includeFile.c_str(), MacroExpandOptions (m_processingLevel));
    m_processingLevel = newProcessLevel;

    T_WStringVector     fileList;
    GetFileList (fileList, includeName.c_str());
    T_WStringVector::reverse_iterator iterator;
    for (iterator = fileList.rbegin(); iterator != fileList.rend(); ++iterator)
        {
        WString     fileName = *iterator;
        WChar       currentName[MAXFILELENGTH*2];
        BeFileName  fileSpec (fileName.c_str());
        if (fileSpec.IsAbsolutePath ())
            {
            wcscpy (currentName, fileName.c_str());
            }
        else
            {
            WString     path = BeFileName::GetDirectoryName (m_currentFile->m_fileName.c_str());
            path.append (fileName);
            wcscpy (currentName, path.c_str());
            }

        PushMacroFile (currentName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
WChar       MacroFileProcessor::GetOneChar ()
    {
    // do we need to open the current file?
    if (!m_currentFile->m_textFile.IsValid())
        {
        if (BeFileStatus::Success != m_currentFile->OpenFile())
            {
            // if we don't stop, return WEOF which will go to next file.
            if (ConfigurationManager::IsVariableDefined (L"_USTN_MACRO_OPENFILE_SOFTFAILURE"))
                return WEOF;

            WChar msg[2*MAXFILELENGTH];
            BeStringUtilities::Snwprintf (msg, L"can't open macro file [%ls]", m_currentFile->m_fileName.c_str());

            FatalError (msg);
            }

        // report progress.
        if (m_debugFile.IsValid())
            m_debugFile->PrintfTo (true, L"\nProcessing macro file [%ls]\n", m_currentFile->m_fileName.c_str());
        }

    return m_currentFile->GetOneChar ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WChar       MacroFileProcessor::ReadCharFromCurrentFile (bool doPreprocessor)
    {
    // NOTE: When it runs out of data in the current file, it goes back to the file that %included that file,
    //       or the next file that was %included by the include file, if that specifed a "path" config variable.
    WChar   charRead;
    while (0 != (charRead = GetOneChar()))
        {
        // don't skip to next file or do PreProcess if we are skipping to end of conditional
        if (!doPreprocessor)
            break;

        if (WEOF == charRead)
            {
            PopFile ();

            if (NULL == m_currentFile)
                break;

            // continue with the previous file.
            charRead = LF;
            break;
            }

        // is it a PreProcessor command?
        if ( ('%' == charRead) && (LF == m_lastNonWhiteSpaceChar) )
            {
            PreProcess ();
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
WChar       MacroFileProcessor::GetInputChar (bool doPreprocessor)
    {
    WChar       previousNonWhiteSpaceChar;
    WChar       charRead;
    for (charRead = 0; ;m_lastCharRead = charRead)
        {
        previousNonWhiteSpaceChar = m_lastNonWhiteSpaceChar;
        if (0 != (charRead = m_pushedChar))
            m_pushedChar = 0;
        else
            charRead = ReadCharFromCurrentFile (doPreprocessor);

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
                charRead =' ';  /* fall through */

            case ' ':
                if (LF == m_lastCharRead)
                    {
                    charRead = LF;
                    break;
                    }
                if (' ' == m_lastCharRead)
                    break;
                goto done;

            case '/':
                charRead = '\\';
                goto done;

            case '\\':
                {
                WChar   nextChar;
                switch (nextChar = ReadCharFromCurrentFile(doPreprocessor))
                    {
                    case '\\':
                        goto done;

                    case '/':
                        charRead = '/';
                        goto done;

                    case    LF:
                        break;

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
                    while ( (LF != (charRead = ReadCharFromCurrentFile (doPreprocessor))) && (WEOF != charRead) && ('[' != charRead) )
                        ;       
                    charRead = LF;
                    }
                goto done;

            case '#':
                // # is comment til end-of-line character
                if (m_lastCharRead == '$')
                    goto done;

                // read until we hit end of line or end of file.
                while ( (LF != (charRead = ReadCharFromCurrentFile (false))) && (WEOF != charRead) )
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
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
#pragma warning(disable:4702)  // unreachable code warning triggered by final return statement in Release build only.
MacroOperation  MacroFileProcessor::GetMacroNameAndOperation (WStringR macroName)
    {
    macroName.clear();

    // outer loop skips blank lines.
    bool    noNonBlank = true;
    WChar   charRead = 0;
    while ( noNonBlank && !m_eofHit)
        {
        // build up the macro name up to the end of the line or the operator.
        while (!m_eofHit && ('=' != (charRead = GetInputChar (true)))   // new rule.
                         && ('+' != charRead)                       // add macro
                         && ('>' != charRead)                       // append path to macro
                         && ('<' != charRead)                       // prepend path to macro
                         && (':' != charRead)                       // new macro (only if macro done)
                         && (CR  != charRead)
                         && (LF  != charRead))
            {
            noNonBlank = false;
            macroName.append (1, charRead);
            }
        }

    // terminate the macroName and remove white space.
    macroName.Trim ();

    if (m_eofHit)
        return  OPERATION_EOF;

    switch (charRead)
        {
        case    '=':    
            return  OPERATION_AddMacro;

        case    ':':
            return  OPERATION_NewMacro;

        case    '+':    
            return  OPERATION_AppendMacro;

        case    '>':    
            return  OPERATION_AppendPath;

        case    '<':    
            return  OPERATION_PrependPath;
        }

    FatalError (L"macro syntax error");

    // Without this : warning C4715: ...  : not all control paths return a value    
    return OPERATION_EOF;
    }
#pragma warning(default:4702)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             10/89
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::GetRestOfLine (WStringR restOfLine)
    {
    restOfLine.clear();

    /* read in line */
    WChar   charRead;
    while ( (LF != (charRead = GetInputChar(true))) && (CR != charRead) )
        restOfLine.append (1, charRead);

    restOfLine.Trim();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::TraceConditionalLevel (WCharCP explanation, int conditionalLevel)
    {
    if ( (m_debugLevel > 2) && m_debugFile.IsValid() )
        m_debugFile->PrintfTo (true, L"%ls, [%ls], line %ld, depth = %d.\n", explanation, m_currentFile->m_fileName.c_str(), m_currentFile->m_lineNumber, conditionalLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::SkipToEndOfConditional (bool allowElse)
    {
    int     conditionalLevel = 0;
    WString line;

    TraceConditionalLevel (L"Starting skip to end", 0);
    for (;;)
        {
        line.clear();

        WChar  charRead;
        while ( (LF != (charRead = GetInputChar (false))) && (CR != charRead) )
            {
            if ( (WEOF == charRead) || (CNTRL_Z == charRead) )
                FatalError (L"unexpected end of file");

            line.append (1, charRead);
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
                if (evaluateSymbolAsBoolean (line.c_str(), m_processingLevel, m_macroCfgAdmin, *this))
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
        };

    for (int iCmd = 0; iCmd < _countof (s_preProcessorCmds); iCmd++)
        {
        if (0 == BeStringUtilities::Wcsicmp (word, s_preProcessorCmds[iCmd]))
            return (PreProcessorCommand) (iCmd+1);
        }

    return  PREPROCESSOR_NotKeyword;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::PreProcess ()
    {
    // start with a string of 80 characters (will resize if necessary).
    WString     line;
    line.reserve (80);

begin:
    // read the rest of the line (on entry to this method, the % PreProcessor leadin character has already been read).
    WChar   previousChar = 0;
    for (WChar  charRead = 0; LF != (charRead = ReadCharFromCurrentFile (false)); )
        {
        if (WEOF == charRead)
            FatalError (L"unexpected end of macro file");

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

    // find the preprocessor command, if there is one.
    PreProcessorCommand preProcCmd = GetPreprocessorCommand (line.c_str());
    switch (preProcCmd)
        {
        case    PREPROCESSOR_Include:
            DoInclude (preprocArgument);
            break;

        case    PREPROCESSOR_IfDefined:
            if (!m_macroCfgAdmin.IsMacroDefinedFromLevel (preprocArgument.c_str(), m_processingLevel))
                SkipToEndOfConditional (true);
            break;

        case    PREPROCESSOR_IfNotDefined:
            if (m_macroCfgAdmin.IsMacroDefinedFromLevel (preprocArgument.c_str(), m_processingLevel))
                SkipToEndOfConditional (true);
            break;

        case    PREPROCESSOR_Else:
        case    PREPROCESSOR_ElseIf:
            SkipToEndOfConditional (false);
            break;

        case    PREPROCESSOR_EndIf:
            break;

        case    PREPROCESSOR_If:
            if (!evaluateSymbolAsBoolean (preprocArgument.c_str(), m_processingLevel, m_macroCfgAdmin, *this))
                SkipToEndOfConditional (true);
            break;

        case    PREPROCESSOR_Level:
            {
            ConfigurationVariableLevel level = (ConfigurationVariableLevel) evaluateSymbolAsInt (preprocArgument.c_str(), m_processingLevel, m_macroCfgAdmin, *this);
            if (level > ConfigurationVariableLevel::User)
                level = ConfigurationVariableLevel::User;
            else if (level < ConfigurationVariableLevel::System)
                level = ConfigurationVariableLevel::System;
            m_processingLevel = level;
            break;
            }

        case    PREPROCESSOR_Error:
            {
            WString     errorMsg;
            m_macroCfgAdmin.ExpandMacro (errorMsg, preprocArgument.c_str(), MacroExpandOptions (m_processingLevel));
            FatalError (errorMsg.c_str());
            break;
            }

        case    PREPROCESSOR_Undef:
            ConfigurationManager::UndefineVariable (preprocArgument.c_str());
            break;

        case    PREPROCESSOR_Lock:
            m_macroCfgAdmin.LockMacro (preprocArgument.c_str());
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
void        MacroFileProcessor::AddMacro (WCharCP macroName)
    {
    WString     restOfLine;
    restOfLine.reserve (80);
    GetRestOfLine (restOfLine);

    MacroExpandOptions options (m_processingLevel);
    options.SetImmediate (false);
    WString     expanded;
    m_macroCfgAdmin.ExpandMacro (expanded, restOfLine.c_str(), options);
    m_macroCfgAdmin.DefineMacroWithDebugging (macroName, expanded.c_str(), m_processingLevel, m_debugFile.get(), m_debugLevel);
    }

/*---------------------------------------------------------------------------------**//**
* only add macro if it doesn't already exist
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::NewMacro (WCharCP macroName)
    {
    WString     tmpString;

    // assigns new value to macroName only if that macro does not already exist.
    if (NULL == m_macroCfgAdmin.GetMacroTranslation (macroName, tmpString, m_processingLevel))
        {
        AddMacro (macroName);
        }
    else
        {
        // just get the rest of the line and throw it away.
        WString restOfLine;
        GetRestOfLine (restOfLine);

        // make sure the macro defined (if it came from system env table)
        m_macroCfgAdmin.EnsureSysEnvDefinition (macroName, m_processingLevel, m_debugFile.get(), m_debugLevel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::AppendMacro (WCharCP macroName, WCharCP sepString, bool doAppend)
    {
    WString     tmpString;
    WCharCP     currentMacroValue;
    if (NULL == (currentMacroValue = m_macroCfgAdmin.GetMacroTranslation (macroName, tmpString, m_processingLevel)))
        {
        AddMacro (macroName);
        return;
        }

    // get the value we want to append/peprend from the input stream.
    WString     appendValue;

    GetRestOfLine (appendValue);

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
    m_macroCfgAdmin.DefineMacroWithDebugging (macroName, newValue.c_str(), m_processingLevel, m_debugFile.get(), m_debugLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   12/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::ProcessMacroFile (ConfigurationVariableLevel startingProcessLevel)
    {
    m_eofHit          = false;
    m_processingLevel = startingProcessLevel;
    for (;;)
        {
        WString             macroName;
        MacroOperation macroOp = GetMacroNameAndOperation (macroName);

        // if we're done, exit the loop.
        if (OPERATION_EOF == macroOp)
            break;

        switch (macroOp)
            {
            case    OPERATION_AddMacro:          
                AddMacro (macroName.c_str());                           
                break;

            case    OPERATION_NewMacro:       
                NewMacro (macroName.c_str());                                     
                break;

            case    OPERATION_AppendMacro:    
                AppendMacro (macroName.c_str(), L" ",    true);          
                break;

            case    OPERATION_AppendPath:     
                AppendMacro (macroName.c_str(), WCSPATH_SEPARATOR, true);  
                break;

            case    OPERATION_PrependPath:    
                AppendMacro (macroName.c_str(), WCSPATH_SEPARATOR, false); 
                break;
            }
        }

    /* After processing file return level to highest (i.e. full translation) */
    m_processingLevel = ConfigurationVariableLevel::User;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroFileProcessor::ProcessTopLevelFile (WCharCP fileName, ConfigurationVariableLevel level)
    {
    BeFileStatus    openStatus;
    BeTextFilePtr   tempOpen = BeTextFile::Open (openStatus, fileName, TextFileOpenType::Read, TextFileOptions::None, TextFileEncoding::CurrentLocale);

    // NULL out the filePtr, which has the effect of closing the temporarily opened file.
    tempOpen = NULL;

    if (BeFileStatus::Success != openStatus)
        return (BentleyStatus) openStatus;

    PushMacroFile (fileName);
    BentleyStatus status = SUCCESS;

    try 
        {
        ProcessMacroFile (level);
        }
    catch (MacroReadError& err)
        {
        _OnProcessError(err.GetMessage());
        status = ERROR;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::PushMacroFile (WCharCP fileName)
    {
    MacroFileEntry*   mfe = new MacroFileEntry (fileName);

    // put the current file onto the end of the list, and make it current.
    if (NULL != m_currentFile)
        m_pendingFiles.push_back (m_currentFile);

    m_currentFile = mfe;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroFileProcessor::PopFile ()
    {
    // get the next file to go to.
    MacroFileEntry* mfe = NULL;
    if (!m_pendingFiles.empty())
        {
        mfe = *(m_pendingFiles.rbegin());
        m_pendingFiles.pop_back ();
        }

    if (NULL != m_currentFile)
        delete m_currentFile;

    m_currentFile = mfe;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroFileProcessor::PrintSummary ()
    {
    if (!m_debugFile.IsValid())
        return;

    m_debugFile->PrintfTo (true, L"\n\n#########################################\n");
    m_debugFile->PrintfTo (true, L"# Configuration Variable Summary        #\n");
    m_debugFile->PrintfTo (true, L"#########################################\n\n");

    m_macroCfgAdmin.PrintAllMacros (*m_debugFile.get(), m_debugLevel);

    m_debugFile->PrintfTo (true, L"\n#########################################\n");
    m_debugFile->PrintfTo (true, L"# End of Configuration Variable Summary #\n");
    m_debugFile->PrintfTo (true, L"#########################################\n");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool        fileCompareDescending (WString const& file1, WString const& file2)
    {
    return  file2.compare (file1) > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void     appendToFileList (T_WStringVector& fileList, WCharCP fileName)
    {
    // terminate string at path separator.
    WCharP  pNextFileName = NULL;
    WCharP  pPathSep;
    if (NULL != (pPathSep = (WCharP) ::wcschr (fileName, WCSPATH_SEPARATOR_CHAR)))
        {
        *pPathSep = 0;
        pPathSep++;
        pNextFileName = (0 != *pPathSep) ? pPathSep : NULL;
        }

    if (NULL != ::wcschr (fileName, '*'))
        {
        // iterate files, but not in subdirectories.
        BeFileListIterator  iterator (fileName, false);
        BeFileName          foundName;
        T_WStringVector     subList;
        while (iterator.GetNextFileName (foundName) == SUCCESS)
            subList.push_back (WString (foundName.GetName()));

        // sort the sublist in descending order, so when we push them using FOR_EACH they're in ascending order..
        std::sort (subList.begin(), subList.end(), fileCompareDescending);

        // add the sorted file names to the overall list.
        FOR_EACH (WString subListFileName, subList)
            fileList.push_back (subListFileName);
        }
    else
        {
        fileList.push_back (WString (fileName));
        }

    // if this was a path separated list, put the next batch in.
    if (NULL != pNextFileName)
        appendToFileList (fileList, pNextFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nancy.McCall    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     MacroFileProcessor::GetFileList (T_WStringVector& fileList, WCharCP fileName)
    {
    if ( (NULL == fileName) || (0 == *fileName) )
        return;

    return  appendToFileList (fileList, fileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MacroFileProcessor::FileExists (WCharCP testFileSpec, ConfigurationVariableLevel processingLevel)
    {
    WChar    tmpName[MAX_LINE_LEN];

    wcscpy (tmpName, testFileSpec);
    wstripSpace (tmpName);
    WString expandedName;
    m_macroCfgAdmin.ExpandMacro (expandedName, tmpName, MacroExpandOptions (processingLevel));

    T_WStringVector   fileList;
    GetFileList (fileList, expandedName.c_str());

    FOR_EACH (WString fileName, fileList)
        {
        if (BeFileName::DoesPathExist (fileName.c_str()))
            return true;
        }

    return  false;
    }

