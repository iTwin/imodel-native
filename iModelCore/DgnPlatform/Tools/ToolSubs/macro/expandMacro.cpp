/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/expandMacro.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include <DgnPlatform/Tools/stringop.h>
#include <DgnPlatform/DesktopTools/envvutil.h>
#include <DgnPlatform/DesktopTools/fileutil.h>
#include <RmgrTools/Tools/heapsig.h>
#include <RmgrTools/Tools/memutil.h>
#include <RmgrTools/Tools/UglyStrings.h>
#include "macro.h"

USING_NAMESPACE_BENTLEY_DGN

#define DQUOTE                  (0x22)
#define SQUOTE                  (0x27)
#define SPACE                   (0x20)
#define TABCHAR                 (0x9)
#define CFGVAR_DEFINED_NULL         ((void *)-1) /* defined but no translation */

#define ISSEPARATOR(c)  ((DIR_SEPARATOR_CHAR == (c)) || (ALT_DIR_SEPARATOR_CHAR == (c)))
#define ISWHITESPACE(c) ((SPACE == (c)) || (TABCHAR == (c)))


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isDirSeparator (WChar c)
    {
    return (c == '/' || c == '\\' || c == ':');
    }

enum    ExpandStatus
    {
    EXPAND_Success          = 0,
    EXPAND_NullExpression,
    EXPAND_Circular,
    EXPAND_DollarNotFollowedByParentheses,
    EXPAND_NoCloseParentheses,
    EXPAND_SyntaxErrorAfterDollarSign,
    EXPAND_SyntaxErrorExpansion,
    EXPAND_SyntaxErrorUnmatchedQuote,
    EXPAND_SyntaxErrorUnmatchedParen,
    EXPAND_SyntaxErrorUnmatchedBrace,
    EXPAND_SyntaxErrorInOperand,
    EXPAND_SyntaxErrorBadOperator,
    };

struct  MacroExpander;

/*=================================================================================**//**
* @bsiclass                                     Barry.Bentley                   01/2012
+===============+===============+===============+===============+===============+======*/
struct      ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) = 0;
};

struct      PossibleMacroOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      MustBeMacroOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      BaseNameOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      BuildOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      ConcatenateOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      DeviceDirectoryOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      DeviceOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      DirectoryOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      ExtensionOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      FileNameOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      FirstOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      FirstDirectoryPartOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      LastDirectoryPartOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      NoExtensionOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      ParentDirectoryAndDeviceOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      ParentDirectoryOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};

struct      RegistryReadOperator : ExpandOperator
{
virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) override;
};


/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      02/2012
+===============+===============+===============+===============+===============+======*/
struct      MacroExpander
{
enum
    {
    MAX_EXPAND_RECURSIONS   = 128,
    };

struct  OperatorInfo
    {
    WCharCP         m_name;
    ExpandOperator* m_operator;
    };

MacroConfigurationAdmin&    m_macroCfgAdmin;
int                         m_recursionDepth; // this is the recursion level of macro expansions (i.e., macros that reference other macros).
bool                        m_expandOnlyIfFullyDefined;
bool                        m_expandAllImmediately;
bool                        m_formatExpansion;
ConfigurationVariableLevel  m_cfgLevel;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MacroExpander (MacroConfigurationAdmin& macroCfgAdmin, MacroConfigurationAdmin::ExpandOptions const& options) : m_macroCfgAdmin (macroCfgAdmin)
    {
    m_recursionDepth                = 0;
    m_expandOnlyIfFullyDefined      = options.m_expandOnlyIfFullyDefined;
    m_expandAllImmediately          = options.m_immediate;
    m_cfgLevel                      = options.m_level;
    m_formatExpansion               = options.m_formatExpansion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Trefz   06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool CharIsLeftParen (WChar thisChar, bool immediate) 
    {
    return ( (thisChar == '{') || (immediate && (thisChar == '(')) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool         ContainsExpression (WCharCP textExpression, bool immediate)
    {
    WCharCP     dollarSign;
    if (NULL == (dollarSign = ::wcschr (textExpression, '$')))
        return false;

    for (WCharCP openParen = dollarSign; 0 != *openParen; openParen++)
        {
        // allow whitespace between.
        if ( ISWHITESPACE (*openParen) )
            continue;
        if ( CharIsLeftParen (*openParen, immediate) )
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          SkipWhiteSpace (WString inputString, size_t position)
    {
    size_t  length = inputString.length();
    for ( ; position < length; position++)
        {
        WChar   thisChar = inputString[position];
        if (!ISWHITESPACE (thisChar))
            return position;
        }
    return WString::npos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandOperator*     FindOperator (WCharCP operatorString)
    {
    static OperatorInfo     s_operators[] =
        {
        {L"",               new MustBeMacroOperator()               },
        {L"$",              new PossibleMacroOperator()             },
        {L"basename",       new BaseNameOperator()                  },
        {L"build",          new BuildOperator()                     },
        {L"concat",         new ConcatenateOperator()               },
        {L"devdir",         new DeviceDirectoryOperator()           },
        {L"dev",            new DeviceOperator()                    },
        {L"dir",            new DirectoryOperator()                 },
        {L"ext",            new ExtensionOperator()                 },
        {L"filename",       new FileNameOperator()                  },
        {L"first",          new FirstOperator()                     },
        {L"firstdirpiece",  new FirstDirectoryPartOperator()        },
        {L"lastdirpiece",   new LastDirectoryPartOperator()         },
        {L"noext",          new NoExtensionOperator()               },
        {L"parentdevdir",   new ParentDirectoryAndDeviceOperator()  },
        {L"parentdir",      new ParentDirectoryOperator()           },
        {L"registryread",   new RegistryReadOperator()              },
        };

    // NULL operatorString means use the evaluate operator.
    if (NULL == operatorString)
        return s_operators[0].m_operator;

    for (int iOperator=1; iOperator < _countof(s_operators); iOperator++)
        {
        OperatorInfo*   opInfo = &s_operators[iOperator];
        if (0 == wcscmp (operatorString, opInfo->m_name))
            return opInfo->m_operator;
        }

    // didn't find an operator that matches!
    return NULL;
    }

enum    ArgParseState
    {
    ARG_Start,
    ARG_Copying,
    ARG_HaveOpenParen,
    ARG_HaveOpenBrace,
    ARG_HaveOpenDoubleQuote,
    ARG_HaveOpenSingleQuote,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus    SplitArguments (size_t& errorPosition, T_WStringVectorR arguments, WStringR operandString)
    {
    // This is called by the two operators (concat and build) that take more than one argument.
    // On entry, operandString contains the contents of all arguments to the operators,
    // e.g. for $(concat ($(MS_DIR),$(filename ($first (MS_RFDIR))))), it would be $(MS_DIR),$(filename ($first (MS_RFDIR)))

    // make sure argument array is empty.
    arguments.clear();

    WString         argument;
    int             parenthesesDepth    = 0;
    int             braceDepth          = 0;
    size_t          stringLength        = operandString.length();
    size_t          operandStringPos      = 0;

    ArgParseState   previousParseState  = ARG_Start;
    for (ArgParseState parseState = ARG_Start; ; operandStringPos++)
        {
        // at end of string, evaluate whether it was successful or not.
        if (operandStringPos > stringLength)
            {
            if (ARG_Copying == parseState)
                {
                if (!argument.empty())
                    arguments.push_back (argument);
                return EXPAND_Success;
                }

            else if ( (ARG_HaveOpenDoubleQuote == parseState) || (ARG_HaveOpenSingleQuote == parseState) )
                return EXPAND_SyntaxErrorUnmatchedQuote;

            else if (ARG_HaveOpenParen == parseState) 
                return EXPAND_SyntaxErrorUnmatchedParen;

            else if (ARG_HaveOpenBrace == parseState) 
                return EXPAND_SyntaxErrorUnmatchedParen;
            }

        errorPosition = operandStringPos;

        // get the next character and figure out what it means.
        WChar   thisChar = operandString[operandStringPos];

        switch (parseState)
            {
            case ARG_Start:
            case ARG_Copying:
                {
                // we are transferring data from the input to the output until we come across something interesting.
                if (ISWHITESPACE (thisChar))
                    {
                    // leading white space is skipped, but kept if we're already copying.
                    if (ARG_Start == parseState)
                        break;
                    }
                else if ('"' == thisChar)
                    {
                    // ignore anything inside DoubleQuotes or SingleQutoes.
                    previousParseState = parseState;
                    parseState = ARG_HaveOpenDoubleQuote;
                    }
                else if ('\'' == thisChar)
                    {
                    previousParseState = parseState;
                    parseState = ARG_HaveOpenSingleQuote;
                    }
                else if (',' == thisChar)
                    {
                    arguments.push_back (argument);
                    argument.clear();
                    parseState = ARG_Start;
                    break;
                    }
                else if ('(' == thisChar)
                    {
                    parseState = ARG_HaveOpenParen;
                    parenthesesDepth = 1;
                    }
                else if ('{' == thisChar)
                    {
                    parseState = ARG_HaveOpenBrace;
                    braceDepth = 1;
                    }
                argument.append (1, thisChar);
                break;
                }
            
            case ARG_HaveOpenDoubleQuote:
                {
                // we simply copy to the copyDestination until we get the closing double quote.
                argument.append (1, thisChar);
                if ('"' == thisChar)
                    parseState = previousParseState;
                break;
                }

            case ARG_HaveOpenSingleQuote:
                {
                // we simply copy to the copyDestination until we get the closing single quote.
                argument.append (1, thisChar);
                if ('\'' == thisChar)
                    parseState = previousParseState;
                break;
                }

            case ARG_HaveOpenBrace:
                {
                argument.append (1, thisChar);
                if ('"' == thisChar)
                    {
                    // ignore anything inside DoubleQuotes or SingleQutoes.
                    previousParseState = parseState;
                    parseState = ARG_HaveOpenDoubleQuote;
                    }
                else if ('\'' == thisChar)
                    {
                    previousParseState = parseState;
                    parseState = ARG_HaveOpenSingleQuote;
                    }
                else if ('}' == thisChar)
                    {
                    if (0 == --braceDepth)
                        parseState = ARG_Copying;
                    }
                else if ('{' == thisChar)
                    {
                    braceDepth++;
                    }
                break;
                }

            case ARG_HaveOpenParen:
                {
                argument.append (1, thisChar);
                if ('"' == thisChar)
                    {
                    // ignore anything inside DoubleQuotes or SingleQutoes.
                    previousParseState = parseState;
                    parseState = ARG_HaveOpenDoubleQuote;
                    }
                else if ('\'' == thisChar)
                    {
                    previousParseState = parseState;
                    parseState = ARG_HaveOpenSingleQuote;
                    }
                else if (')' == thisChar)
                    {
                    if (0 == --parenthesesDepth)
                        {
                        parseState = ARG_Copying;
                        }
                    }
                else if ('(' == thisChar)
                    {
                    parenthesesDepth++;
                    }
                break;
                }
            default:
                {
                BeAssert (false);
                }
            }
        }

    // Unreachable code
    //return EXPAND_Success;
    }

enum    ExpressionParseState
    {
    EXP_Start,
    EXP_Copying,
    EXP_HaveDollarSign,
    EXP_HaveStartExpansionOrOperator,
    EXP_HaveOpenDoubleQuote,
    EXP_HaveOpenSingleQuote,
    EXP_InParenEnclosedOperand,
    EXP_GetClosingOperandParentheses,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus    SplitExpression (size_t& errorPosition, WStringR expansion, ExpandOperator*& thisOperator, bool& immediateExpansion, WStringR operandString, WStringR macroString)
    {
    // The overall idea is that the input is macroString. If there are no expansions or operators, we simply copy it to the expansion and we're done.
    // If there is a ${, that indicates the start of an immediate expansion.
    // If there is a $(, that indicates the start of an expansion or operator. However, unless immediateOperation is set, we simply copy to the output.
    // The way this is implemented, either ) or } interchangeable close a $( or ${. However, the opening { or ( determines whether the expression is immediate or not.
    operandString.clear();
    thisOperator = NULL;

    // This method parses macroString into an operator and an operandString. 
    // Everything preceding the start of an operator is appended to expansion as a literal string.
    // At the end, macroString is set to the portion of it that follows the operator.
    errorPosition = 0;

    // we start out with zero depths, copying to the expansion, and an empty operatorName.
    int                     parenthesesDepth    = 0;
    WStringP                copyDestination     = &expansion;
    WString                 operatorName;
    WString                 doubleQuotedString;
    WString                 singleQuotedString;

    size_t                  stringLength        = macroString.length();
    size_t                  macroStringPos      = 0;
    ExpressionParseState    previousParseState  = EXP_Start;

    for (ExpressionParseState parseState = EXP_Start; ; macroStringPos++)
        {
        // at end of string, evaluate whether it was successful or not.
        if (macroStringPos >= stringLength)
            {
            if (EXP_Copying == parseState)
                return EXPAND_Success;

            if (EXP_HaveDollarSign == parseState)
                return EXPAND_SyntaxErrorAfterDollarSign;

            else if (EXP_HaveStartExpansionOrOperator == parseState)
                return EXPAND_SyntaxErrorExpansion;

            else if ( (EXP_HaveOpenDoubleQuote == parseState) || (EXP_HaveOpenSingleQuote == parseState) )
                return EXPAND_SyntaxErrorUnmatchedQuote;

            else if (EXP_InParenEnclosedOperand == parseState)
                return EXPAND_SyntaxErrorInOperand;

            else if (EXP_GetClosingOperandParentheses == parseState)
                return EXPAND_SyntaxErrorInOperand;

            else
                {
                BeAssert (false);
                return EXPAND_Success;
                }
            }

        errorPosition = macroStringPos;

        // get the next character and figure out what it means.
        WChar   thisChar = macroString[macroStringPos];

        switch (parseState)
            {
            case EXP_Start:
            case EXP_Copying:
                {
                // we are transferring data from the input to the output until we come across something interesting.
                if (ISWHITESPACE (thisChar))
                    {
                    // leading white space is skipped, but kept if we're already copying.
                    if (EXP_Start == parseState)
                        break;
                    }
                else if ('$' == thisChar)
                    {
                    // should find an open paren or brace next. Don't copy to output yet.
                    parseState = EXP_HaveDollarSign;
                    break;
                    }
                else if ('"' == thisChar)
                    {
                    // ignore anything inside DoubleQuotes or SingleQutoes.
                    previousParseState = parseState;
                    copyDestination->append (1, thisChar);
                    doubleQuotedString.clear();
                    parseState = EXP_HaveOpenDoubleQuote;
                    break;
                    }
                else if ('\'' == thisChar)
                    {
                    previousParseState = parseState;
                    copyDestination->append (1, thisChar);
                    singleQuotedString.clear();
                    parseState = EXP_HaveOpenSingleQuote;
                    break;
                    }

                copyDestination->append (1, thisChar);
                parseState = EXP_Copying;
                break;
                }
            
            case EXP_HaveOpenDoubleQuote:
                {
                // copy to the doubleQuotedString until we get the closing double quote, then expand that string and save in copyDestination.
                if ('"' == thisChar)
                    {
                    // try to expand the stuff inside the double quoted string.
                    WString doubleQuotedExpansion;
                    ExpandMacroExpression (doubleQuotedExpansion, doubleQuotedString.c_str(), false, immediateExpansion);
                    copyDestination->append (doubleQuotedExpansion);
                    copyDestination->append (1, thisChar);
                    parseState = previousParseState;
                    }
                else
                    doubleQuotedString.append (1, thisChar);
                break;
                }

            case EXP_HaveOpenSingleQuote:
                {
                // copy to the singleQuotedString until we get the closing single quote, then expand that string and save in copyDestination.
                if ('\'' == thisChar)
                    {
                    // try to expand the stuff inside the double quoted string.
                    WString singleQuotedExpansion;
                    ExpandMacroExpression (singleQuotedExpansion, singleQuotedString.c_str(), false, immediateExpansion);
                    copyDestination->append (singleQuotedExpansion);
                    copyDestination->append (1, thisChar);
                    parseState = previousParseState;
                    }
                else
                    singleQuotedString.append (1, thisChar);
                break;
                }

            case EXP_HaveDollarSign:
                {
                // the only valid characters following a $ are whitespace, (, or {.
                if (ISWHITESPACE (thisChar))
                    {
                    // leading white space between $ and parentheses or brace is skipped.
                    }
                else if ('{' == thisChar)
                    {
                    // all nested expansions are immediate.
                    immediateExpansion = true;
                    parseState      = EXP_HaveStartExpansionOrOperator;
                    }
                else if ('(' == thisChar)
                    {
                    // If we are in an immediate expansion, this is the start of an expansion or operator.
                    // Otherwise, we just want to copy to the output.
                    if (immediateExpansion)
                        {
                        parseState      = EXP_HaveStartExpansionOrOperator;
                        }
                    else
                        {
                        // put the $ back into the copyDestination.
                        copyDestination->append (1, '$');
                        copyDestination->append (1, '(');
                        parseState = EXP_Copying;
                        }
                    }
                else
                    {
                    return EXPAND_SyntaxErrorAfterDollarSign;
                    }
                break;
                }

            case EXP_HaveStartExpansionOrOperator:
                {
                // This state is only possible when we are in immediate expandsion mode.
                // we have to collect the input until we get either another (, which indicates that what was between ('s should have been an operator, (e.g. $(dir(...))
                // or a closing ), which indicates that we got a macro name that needs to be expanded, like $(MSDIR).
                if ( ('(' == thisChar) || ('{' == thisChar) )
                    {
                    operatorName.Trim();

                    // if we got here, operatorName is either empty or should contain the name of the operator.
                    // The contents of the parentheses is frequently a macro, even though it does not have a $() surrounding it.
                    // For example, you often see stuff like $(devdir (MSDIR)) rather than $(devdir ($(MSDIR)))
                    if (NULL == (thisOperator = FindOperator (operatorName.empty() ? L"$" : operatorName.c_str())))
                        return EXPAND_SyntaxErrorBadOperator;

                    parseState = EXP_InParenEnclosedOperand;
                    // we have to pull off both of the last two closing parentheses in an expression like $(first ($(MS_DIR)))
                    copyDestination = &operandString;
                    }
                else if ('"' == thisChar)
                    {
                    // ignore anything inside DoubleQuotes or SingleQutoes.
                    previousParseState = parseState;
                    parseState = EXP_HaveOpenDoubleQuote;
                    copyDestination->append (1, thisChar);
                    break;
                    }
                else if ('\'' == thisChar)
                    {
                    previousParseState = parseState;
                    parseState = EXP_HaveOpenSingleQuote;
                    copyDestination->append (1, thisChar);
                    break;
                    }
                else if ( (')' == thisChar) || ('}' == thisChar) )
                    {
                    // We got something like $(ABC), so what we thought might be an operator name turned out to be a macroName.
                    operandString.assign (operatorName);
                    // get the "Evaluate operation".
                    thisOperator = FindOperator (NULL);
                    // erase the portion of the macroString we have processed.
                    macroString.erase (0, macroStringPos+1);
                    return EXPAND_Success;
                    }
                else
                    {
                    operatorName.append (1, thisChar);
                    }
                break;
                }

            case EXP_InParenEnclosedOperand:
                {
                // This state is only possible when we are in immediate expandsion mode.
                if ( (')' == thisChar) || ('}' == thisChar) )
                    {
                    if (0 == parenthesesDepth--)
                        {
                        // we have the operand, but we need the closing parentheses. We simply throw away everything until we get it.
                        parseState = EXP_GetClosingOperandParentheses;
                        break;
                        }
                    }
                else if ( ('(' == thisChar) || ('{' == thisChar) )
                    {
                    parenthesesDepth++;
                    }
                else if ('"' == thisChar)
                    {
                    // ignore anything inside DoubleQuotes or SingleQutoes.
                    previousParseState = parseState;
                    parseState = EXP_HaveOpenDoubleQuote;
                    }
                else if ('\'' == thisChar)
                    {
                    previousParseState = parseState;
                    parseState = EXP_HaveOpenSingleQuote;
                    }

                copyDestination->append (1, thisChar);
                break;
                }

            case EXP_GetClosingOperandParentheses:
                {
                if ( (')' == thisChar) || ('}' == thisChar) )
                    {
                    macroString.erase (0, macroStringPos+1);
                    return EXPAND_Success;
                    }
                else if (ISWHITESPACE (thisChar))
                    {
                    // continue looking
                    break;
                    }
                else
                    {
                    // should not get anything else.
                    return EXPAND_SyntaxErrorInOperand;
                    }
                }

            default:
                {
                BeAssert (false);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus    ExpandMacroExpression (WStringR expansion, WCharCP macroExpression, bool considerToBeMacro, bool immediateExpansion)
    {
    expansion.clear();

    if (NULL == macroExpression)
        return EXPAND_NullExpression;

    if (0 == *macroExpression)
        return EXPAND_Success;

    // see if it could possible if it's less than three characters, there can be no expressions.
    bool    containsExpression = ContainsExpression (macroExpression, immediateExpansion);
    if ( (wcslen (macroExpression) < 3) || (!considerToBeMacro && !containsExpression))
        {
        expansion.assign (macroExpression);
        return EXPAND_Success;
        }

    if (considerToBeMacro && !containsExpression)
        {
        WString     tmpStorage;
        WCharCP     translation = m_macroCfgAdmin.GetMacroTranslation (macroExpression, tmpStorage, m_cfgLevel);
        if (NULL != translation)
            {
            expansion.assign (translation);
            return EXPAND_Success;
            }
        }

    // if our recursion is too deep, we probably have a circular definition.
    if (++m_recursionDepth > MAX_EXPAND_RECURSIONS)
        return EXPAND_Circular;

    // get a local copy of the input.
    WString     macroString (macroExpression);

    // find the first operand and argument list.
    WString             operandString;
    ExpandOperator*     thisOperator;
    for ( ; !macroString.empty(); )
        {
        ExpandStatus    status;
        bool            innerImmediate = immediateExpansion;
        size_t          errorPosition  = 0;

        if (EXPAND_Success != (status = SplitExpression (errorPosition, expansion, thisOperator, innerImmediate, operandString, macroString)))
            return status;

        // if no operand, we're done.
        if (operandString.empty())
            return EXPAND_Success;

        WString     operandExpansion;
        if (EXPAND_Success != (status = thisOperator->Execute (errorPosition, *this, operandExpansion, operandString, innerImmediate)))
            return status;

        // the operandExpansion might itself be need to be expanded.
        WString     subExpansion;
        if (EXPAND_Success != (status = ExpandMacroExpression (subExpansion, operandExpansion.c_str(), false, innerImmediate)))
            return status;

        expansion.append (subExpansion);
        }

    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/93
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       FormatExpansion (WStringR expandedString)
    {
    // make sure there is something to format.
    if (expandedString.empty())
        return SUCCESS;

    // allocate memory to temporarily hold formatted expansion - result will be <= to expString */
    WCharP       outputP, outputHeadP;
    outputP  = outputHeadP = (WCharP) _alloca (sizeof (WChar) * (expandedString.length()+1));
    *outputP = 0;

    WCharCP inputP = expandedString.c_str();

    // skip over leading path separators.
    while (inputP[0] == PATH_SEPARATOR[0])
        inputP++;

    bool        checkWin32Mount = true;
    bool        checkForURL     = true;
    bool        isURL           = false;
    /* loop through rest of string */
    for (; 0 != *inputP; inputP++)
        {
        if (checkWin32Mount)
            {
            if (0 == wcsncmp (inputP, L"\\\\", 2))
                {
                /*  ----------------------------------------------------------------
                    WIN32 describes a mount point with 2 leading backslashes so we
                        don't want to remove them.

                    Example: "\\mount\dir\"
                    ---------------------------------------------------------------- */
                *outputP++ = *inputP++;
                }
            checkWin32Mount = false;
            }

        if (checkForURL)
            {
            /* ----------------------------------------------------------------------
                The macro system has already converted forward slashes ("/") into the
                platform-specific separator.  However, URLs always use forward slashes
                no matter what the platform is.  We're making the assumption that a
                pattern of semicolon-dirSeparator-dirSeparator indicates that a URL
                was desired and that we should convert back to forward slashes.
            ------------------------------------------------------------------------ */
            if  ((':' == inputP[0]) && isDirSeparator (inputP[1]) && isDirSeparator (inputP[2]))
                {
                checkForURL = false;
                isURL = true;

                *outputP = ':';
                outputP++;
                inputP++;

                while (isDirSeparator(inputP[0]))
                    {
                    *outputP = '/';
                    outputP++;
                    inputP++;
                    }
                }

            /* if we have stepped to the end of the string, we are done. */
            if (0 == *inputP)
                break;
            }

        if ((DIR_SEPARATOR_CHAR == *inputP) || (ALT_DIR_SEPARATOR_CHAR == *inputP))
            {
            WChar tempChar = *inputP;

            /* -------------------------------------------------------------
                Having 2 of the same dir separator next to each other is
                  invalid, so remove duplicates.

                Example:  don't want \ustation\\config\
               ------------------------------------------------------------- */
            while (tempChar == inputP[1])
                inputP++;

            if (isURL)
                *outputP++ = '/';
            else
                *outputP++ = *inputP;
            }
        else if (*inputP == PATH_SEPARATOR[0])
            {
            /* -------------------------------------------------------------
                Having 2 path separators next to each other is
                  undesirable, so remove duplicates.

                Example:  don't want \ustation\;;\ustation\config\
               ------------------------------------------------------------- */
            while (inputP[1] == PATH_SEPARATOR[0])
                inputP++;

            // after a path separator, need to recheck for a Win32 mount and URL
            checkWin32Mount = true;
            checkForURL     = true;
            isURL           = false;

            *outputP++ = *inputP;
            }
        else
            {
            *outputP++ = *inputP;
            }
        }
    *outputP = 0;

    if (0 == outputHeadP[0])
        {
        expandedString.clear();
        return SUCCESS;
        }

    // remove trailing separators
    if (*(outputP-1) == PATH_SEPARATOR[0])
        *(outputP-1) = 0;

    expandedString.assign (outputHeadP);

    return SUCCESS;
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Expand (WStringR expansion, WCharCP macroExpression)
    {
    StatusInt status = ExpandMacroExpression (expansion, macroExpression, false, m_expandAllImmediately);

    if ( (BSISUCCESS == status) && m_formatExpansion)
        return FormatExpansion (expansion);

    return status;
    }


}; // MacroExpander

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus    MustBeMacroOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    // if it must be a macro, then if there is no translation, return an empty string.
    WString     tmpStorage;
    WCharCP     translation = macroExpander.m_macroCfgAdmin.GetMacroTranslation (operandString.c_str(), tmpStorage, macroExpander.m_cfgLevel);
    if (NULL == translation)
        return EXPAND_Success;

    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, translation, false, immediateExpansion)))
        return status;

    result.assign (expansion);
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus    PossibleMacroOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    result.assign (expansion);
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus    BaseNameOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    // treat the operandString like a file name and get the base name of it.
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    result = BeFileName::GetFileNameWithoutExtension (expansion.c_str());
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus BuildOperator::Execute (size_t& errorPosition,  MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    T_WStringVector args;

    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.SplitArguments (errorPosition, args, operandString)))
        return status;

    if (0 == args.size())
        return EXPAND_Success;

    // start with empty components.
    WString     device;
    WString     directory;
    WString     fileName;
    WString     extension;
    WChar       dirSeparators[] = { DIR_SEPARATOR_CHAR, ALT_DIR_SEPARATOR_CHAR, ':', 0 };

    // We are trying to do the same thing as what the 08.11.09 version did, so we work from the last argument.
    for (size_t iArg=args.size(); iArg > 0; --iArg)
        {
        WString argExpansion;
        macroExpander.ExpandMacroExpression (argExpansion, args[iArg-1].c_str(), true, immediateExpansion);

        if (argExpansion.empty())
            continue;

        if (extension.empty())
            {
            if ('.' == argExpansion[0])
                {
                extension.assign (1 + argExpansion.c_str());
                continue;
                }
            }

        if (fileName.empty())
            {
            // if no separators, use it, otherwise don't.
            if (WString::npos == argExpansion.find_first_of (dirSeparators))
                {
                WString     tmpExt;
                BeFileName::ParseName (NULL, NULL, &fileName, &tmpExt, argExpansion.c_str());
                if (!tmpExt.empty())
                    extension.assign (tmpExt);
                continue;
                }
            }

        if (device.empty())
            {
            size_t  firstSep = argExpansion.find_first_of (dirSeparators);
            if ( (WString::npos != firstSep) && (firstSep < argExpansion.length()) )
                {
                WString tmp (argExpansion.c_str());
                if (!directory.empty() )
                    tmp.append (directory);
                else if (!fileName.empty() && (DIR_SEPARATOR_CHAR != tmp[tmp.length()-1]) )
                    tmp.append (1, DIR_SEPARATOR_CHAR);

                BeFileName::ParseName (&device, &directory, fileName.empty() ? &fileName : NULL, extension.empty() ? &extension : NULL, tmp.c_str());
                continue;
                }
            }

        if (directory.empty())
            {
            size_t  firstSep = argExpansion.find_first_of (dirSeparators);
            if ( (WString::npos != firstSep) && (firstSep < argExpansion.length()) )
                {
                WString tmp (argExpansion.c_str());
                tmp.append (1, DIR_SEPARATOR_CHAR);
                BeFileName::ParseName (&device, &directory, fileName.empty() ? &fileName : NULL, extension.empty() ? &extension : NULL, tmp.c_str());
                continue;
                }
            }
        }
    BeFileName::BuildName (result, device.c_str(), directory.c_str(), fileName.c_str(), extension.c_str());
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus ConcatenateOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    T_WStringVector args;
    macroExpander.SplitArguments (errorPosition, args, operandString);
    for (size_t iArg=0; iArg < args.size(); iArg++)
        {
        WString argExpansion;
        macroExpander.ExpandMacroExpression (argExpansion, args[iArg].c_str(), true, immediateExpansion);
        result.append (argExpansion);
        }
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus DeviceDirectoryOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    result = BeFileName::GetDirectoryName (expansion.c_str());
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus DeviceOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    result = BeFileName::GetDevice (expansion.c_str());
    if (!result.empty())
        result.append (1, ':');

    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus DirectoryOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    result = BeFileName::GetDirectoryWithoutDevice (expansion.c_str());
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus ExtensionOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    result = BeFileName::GetExtension (expansion.c_str());
    //extension is returned without leading dot, so add it
    if (! result.empty())
        result.insert (0, L".");
    
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus FileNameOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    result = BeFileName::GetFileNameAndExtension (expansion.c_str());
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus FirstOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    // take everything before first PATH_SEPARATOR_CHAR
    size_t  pathSepIndex;
    if (WString::npos != (pathSepIndex = expansion.find (PATH_SEPARATOR_CHAR)))
        expansion.erase (pathSepIndex);

    result.assign (expansion);

    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus FirstDirectoryPartOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    // take everything between the first and second DIR_SEPARATOR_CHAR's
    size_t  firstSepIndex;
    if (WString::npos == (firstSepIndex = expansion.find (DIR_SEPARATOR_CHAR)))
        return EXPAND_Success;

    size_t  secondSepIndex;
    if (WString::npos != (secondSepIndex = expansion.find (DIR_SEPARATOR_CHAR, firstSepIndex+1)))
        result.assign (expansion, firstSepIndex+1, secondSepIndex - (firstSepIndex+1));
    else
        result.assign (expansion, firstSepIndex+1, expansion.length() - (firstSepIndex+1));

    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus LastDirectoryPartOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    // take everything between the last and second-to-last DIR_SEPARATOR_CHAR's
    size_t  lastSepIndex;
    if (WString::npos == (lastSepIndex = expansion.rfind (DIR_SEPARATOR_CHAR)))
        return EXPAND_Success;

    size_t  secondToLastSepIndex;
    if (WString::npos == (secondToLastSepIndex = expansion.rfind (DIR_SEPARATOR_CHAR, lastSepIndex-1)))
        return EXPAND_Success;

    result.assign (expansion, secondToLastSepIndex+1, lastSepIndex - (secondToLastSepIndex+1));

    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus NoExtensionOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    WString extension = BeFileName::GetExtension (expansion.c_str());
    if (!extension.empty())
        {
		//extension is returned without leading dot, so add it
        extension.insert (0, L".");

        size_t  extensionStart;
        if (WString::npos != (extensionStart = expansion.rfind (extension)))
            expansion.erase (extensionStart);
        }
    result.assign (expansion);
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus ParentDirectoryAndDeviceOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    expansion = BeFileName::GetDirectoryName (expansion.c_str());

    // take everything up to and including the second-to-last DIR_SEPARATOR_CHAR's
    size_t  lastSepIndex;
    if (WString::npos == (lastSepIndex = expansion.rfind (DIR_SEPARATOR_CHAR)))
        return EXPAND_Success;

    size_t  secondToLastSepIndex;
    if (WString::npos == (secondToLastSepIndex = expansion.rfind (DIR_SEPARATOR_CHAR, lastSepIndex-1)))
        return EXPAND_Success;

    result.assign (expansion, 0, secondToLastSepIndex+1);
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus ParentDirectoryOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), true, immediateExpansion)))
        return status;

    expansion = BeFileName::GetDirectoryWithoutDevice (expansion.c_str());

    // take everything up to and including the second-to-last DIR_SEPARATOR_CHAR's
    size_t  lastSepIndex;
    if (WString::npos == (lastSepIndex = expansion.rfind (DIR_SEPARATOR_CHAR)))
        return EXPAND_Success;

    size_t  secondToLastSepIndex;
    if (WString::npos == (secondToLastSepIndex = expansion.rfind (DIR_SEPARATOR_CHAR, lastSepIndex-1)))
        return EXPAND_Success;

    result.assign (expansion, 0, secondToLastSepIndex+1);
    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
ExpandStatus RegistryReadOperator::Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion)
    {
    WString         expansion;
    ExpandStatus    status;
    if (EXPAND_Success != (status = macroExpander.ExpandMacroExpression (expansion, operandString.c_str(), false, immediateExpansion)))
        return status;

    expansion.DropQuotes();
    if (BSISUCCESS == util_readRegistry (result, expansion.c_str()))
        return EXPAND_Success;
    else
        result.assign (expansion);

    return EXPAND_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   MacroConfigurationAdmin::ExpandMacro (WStringR expansion, WCharCP macroExpression, ExpandOptions const& options)
    {
    MacroExpander mx (*this, options);
    return mx.Expand (expansion, macroExpression);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MacroConfigurationAdmin::ContainsExpression (WCharCP macroExpression)
    {
    return MacroExpander::ContainsExpression (macroExpression, true);
    }


