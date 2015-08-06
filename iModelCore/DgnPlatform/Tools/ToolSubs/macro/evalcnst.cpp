/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/evalcnst.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include    <ctype.h>
#include    "macro.h"
#include    <DgnPlatform/Tools/stringop.h>
#include    <DgnPlatform/DesktopTools/MacroFileProcessor.h>

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     IsIntegerConstant (WCharCP stringValue, int *pValue)
    {
    if (NULL == stringValue)
        return  false;

    if (0 == *stringValue)
        return  false;

    WCharCP pC = stringValue;
    if ('-' == *pC)
        pC++;

    while ( (*pC != 0) && isdigit (*pC) ) 
        pC++;

    if (0 == *pC)
        {
        *pValue = BeStringUtilities::Wtoi (stringValue);
        return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         04/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     StripOutInnerQuote (WCharP pTarget, WCharCP pSource)
    {
    WCharCP pStart = pSource;
    WCharCP pTemp;

    *pTarget = 0;
    if (NULL == (pTemp = ::wcschr (pSource, '"')))
        {
        wcscpy (pTarget, pStart);
        return;
        }

    while (0 != *pSource)
        {
        if (*pSource != '"')
            *pTarget++ = *pSource;
        else if (pSource > pStart && *(pSource-1) == '\\')
            *pTarget++ = *pSource;

        pSource++;
        }

    *pTarget = 0;
    }


enum NodeType
    {
    NODETYPE_Integer    =  1,
    NODETYPE_String     =  2,
    NODETYPE_Char       =  3,
    NODETYPE_Boolean    =  4,
    };

typedef struct Node*     NodeP;
typedef struct Node&     NodeR;

/*=================================================================================**//**
* Node within a ConstantEvaluator
* @bsiclass                                                     Barry.Bentley   01/2012
+===============+===============+===============+===============+===============+======*/
struct Node
{
NodeType            m_type;
MacroFileProcessor& m_macroFileProc;
union
    {
    int         i;              /* int */
    WStringP    string;         /* string */
    WChar       c;              /* character constant */
    } m_value;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
Node (bool value, MacroFileProcessor& mfp) : m_macroFileProc (mfp)
    {
    m_type      = NODETYPE_Boolean;
    m_value.i   = value ? 1 : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
Node (int value, MacroFileProcessor& mfp) : m_macroFileProc (mfp)
    {
    m_type      = NODETYPE_Integer;
    m_value.i   = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
Node (WCharCP stringVal, MacroFileProcessor& mfp) : m_macroFileProc (mfp)
    {
    m_type          = NODETYPE_String;
    m_value.string  = new WString (stringVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
Node (char character, MacroFileProcessor& mfp) : m_macroFileProc (mfp)
    {
    m_type      = NODETYPE_Char;
    m_value.c   = character;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsConstant ()
    {
    int value;

    switch  (m_type)
        {
        case NODETYPE_Integer:
        case NODETYPE_Boolean:
            return  true;

        case NODETYPE_String:
            return IsIntegerConstant (m_value.string->c_str(), &value);

        default:
            return  false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    ConvertToInteger ()
    {
    // converts internal value to string.
    int         value = 0;
    if ( (NODETYPE_Integer == m_type) || (NODETYPE_Boolean == m_type) || (NODETYPE_Char == m_type) )
        return;

    if (NODETYPE_String == m_type)
        {
        if (IsIntegerConstant (m_value.string->c_str(), &value))
            {
            m_type = NODETYPE_Integer;
            m_value.i = value;
            return;
            }
        }
    m_macroFileProc.FatalError (L"expect a constant");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetToBoolean (bool value)
    {
    FreeString ();
    m_value.i = value ? 1 : 0;
    m_type = NODETYPE_Boolean;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetToInteger (int value)
    {
    FreeString ();
    m_value.i = value;
    m_type = NODETYPE_Integer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetToString (WCharCP stringVal)
    {
    FreeString ();
    m_value.string = new WString (stringVal);
    m_type = NODETYPE_String;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GetBooleanValue()
    {
    BeAssert ( (NODETYPE_Integer == m_type) || (NODETYPE_Boolean == m_type) );
    return (0 != m_value.i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
int     GetIntegerValue()
    {
    BeAssert ( (NODETYPE_Integer == m_type) || (NODETYPE_Boolean == m_type) );
    return (m_value.i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsArithType ()
    {
    return ( (m_type == NODETYPE_Integer) || (m_type == NODETYPE_Boolean) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void    RequireArithType ()
    {
    if (!IsArithType())
        m_macroFileProc.FatalError (L"Operands type mismatch");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    FreeString ()
    {
    if ( (NODETYPE_String == m_type) && (NULL != m_value.string) )
        {
        delete m_value.string;
        m_value.string = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
~Node ()
    {
    FreeString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool    Equals (NodeR right)
    {
    if ( (NODETYPE_Integer == m_type) || (NODETYPE_Boolean == m_type) || (NODETYPE_Char == m_type))
        {
        right.ConvertToInteger ();
        return GetIntegerValue() == right.GetIntegerValue();
        }
    else if (NODETYPE_String == m_type)
        {
        if (right.m_type != NODETYPE_String)
            {
            // left is string, but right is not. Convert left to integer and then compare.
            if (IsConstant ())
                {
                ConvertToInteger ();
                return Equals (right);
                }
            else
                m_macroFileProc.FatalError (L"Operands type mismatch");
            }
        else
            {
            // both strings.
            WCharP    leftString = static_cast<WCharP>  (_alloca (sizeof (WChar) * (m_value.string->length()+1)) );
            WCharP    rightString = static_cast<WCharP> (_alloca (sizeof (WChar) * (right.m_value.string->length()+1)) );

            StripOutInnerQuote (leftString, m_value.string->c_str());
            StripOutInnerQuote (rightString, right.m_value.string->c_str());

            return (0 == wcscmp (leftString, rightString));
            }
        }
    // should never get here.
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    /*Node::*/Concatenate (NodeR right)
    {
    if ( (NODETYPE_String != m_type) && (NODETYPE_Char != m_type) )
        {
        BeAssert (false);
        m_macroFileProc.FatalError (L"Operands type mismatch");
        }


    if ( (NODETYPE_String != right.m_type) && (NODETYPE_Char != right.m_type) )
        {
        m_macroFileProc.FatalError (L"Operands type mismatch");
        }

    WStringP     concatenated = new WString();
    if (NODETYPE_String == m_type)
        concatenated->assign (*m_value.string);
    else
        concatenated->assign (1, m_value.c);

    if (NODETYPE_String == right.m_type)
        concatenated->append (*m_value.string);
    else
        concatenated->append (1, m_value.c);

    FreeString();
    m_type = NODETYPE_String;
    m_value.string = concatenated;
    }
};


/*=================================================================================**//**
* Constant Evaluator class
* @bsiclass                                                     Barry.Bentley   01/2012
+===============+===============+===============+===============+===============+======*/
struct ConstantEvaluator
{
enum TokenType
    {
    TOKEN_NotStarted            = 0,
    TOKEN_EndOfString           = -1,
    TOKEN_BadToken              = -2,
    TOKEN_LeftParen             = 1,
    TOKEN_RightParen            = 2,
    TOKEN_LogicalOr             = 3,
    TOKEN_LogicalAnd            = 4,
    TOKEN_LogicalNegate         = 5,
    TOKEN_PreProcDefined        = 6,
    TOKEN_PreProcExists         = 7,
    TOKEN_Symbol                = 8,
    TOKEN_BinaryOr              = 9,       /* InclusiveOrExpression | ExclusiveOrExpression */
    TOKEN_BinaryXor             = 10,      /* ExclusiveOrExpression ^ AndExpression */
    TOKEN_BinaryAnd             = 11,      /* ConditionalAndExpression && InclusiveOrExpression */
    TOKEN_Equals                = 12,      /* EqualityExpression == RelationalExpression */
    TOKEN_NotEquals             = 13,      /* EqualityExpression != RelationalExpression */
    TOKEN_GreaterThan           = 14,      /* RelationalExpression > ShiftExpression */
    TOKEN_LessThan              = 15,      /* RelationalExpression < ShiftExpression */
    TOKEN_GreaterOrEqual        = 16,      /* RelationalExpression >= ShiftExpression */
    TOKEN_LessOrEqual           = 17,      /* RelationalExpression <= ShiftExpression */
    TOKEN_ShiftLeft             = 18,      /* ShiftExpression << AdditiveExpression */
    TOKEN_ShiftRight            = 19,      /* ShiftExpression >> AdditiveExpression */
    TOKEN_Add                   = 20,      /* AdditiveExpression + MultiplicativeExpression */
    TOKEN_Subtract              = 21,      /* AdditiveExpression - MultiplicativeExpression */
    TOKEN_Multiply              = 22,      /* MultiplicativeExpression * UnaryExpression */
    TOKEN_Divide                = 23,      /* MultiplicativeExpression / UnaryExpression */
    TOKEN_Modulus               = 24,      /* MultiplicativeExpression % UnaryExpression */
    TOKEN_StringConstant        = 25,
    TOKEN_Hex                   = 26,
    TOKEN_Octal                 = 27,
    TOKEN_Char                  = 28,
    TOKEN_PreProcessorCommand   = 29,
    };

TokenType                   m_token;
WCharP                      m_expression;
ConfigurationVariableLevel  m_level;
MacroConfigurationAdmin&    m_macroCfgAdmin;
MacroFileProcessor&         m_macroFileProc;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
ConstantEvaluator (MacroConfigurationAdmin& macroCfgAdmin, WCharCP expression, ConfigurationVariableLevel level, MacroFileProcessor& macroFileProc) : m_macroCfgAdmin (macroCfgAdmin), m_macroFileProc (macroFileProc)
    {
    m_expression  = const_cast<WCharP>(expression);
    m_level       = level;
    m_token       = TOKEN_NotStarted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
void    Advance ()
    {
    // This method steps through the input and updates m_expression and m_tokenType.
    int     thisChar;

    for (;;)
        {
        /* skip leading space */
        if (!isspace (thisChar = *(m_expression++)))
            break;
        }

    m_token = TOKEN_BadToken;
    switch (thisChar)
        {
        case '0':
            // Interpretation of integer constant is handled in IsSymbolDefined.
            if (*m_expression == 'x' || *m_expression == 'X') /* hex */
                m_token = TOKEN_Hex;
            else            /* octal */
                m_token = TOKEN_Octal;
            m_expression--;
            break;

        case '"':
            m_token = TOKEN_StringConstant;
            m_expression--;
            break;

        case '\'':
            m_token = TOKEN_Char;
            m_expression--;
            break;

        case '\0':  
            m_token = TOKEN_EndOfString;
            break;

        case '(':   
            m_token = TOKEN_LeftParen;
            break;

        case ')':   
            m_token = TOKEN_RightParen;
            break;

        case '!':
            if ('=' == *m_expression)
                {
                m_expression++;
                m_token = TOKEN_NotEquals;
                }
            else
                m_token = TOKEN_LogicalNegate;

            break;

        case '^':   m_token = TOKEN_BinaryXor;            break;
        case '=':
            if (*m_expression == '=')
                {
                m_expression++;
                m_token = TOKEN_Equals;
                }
            break;

        case '>':
            if (*m_expression == '=')
                {
                m_expression++;
                m_token = TOKEN_GreaterOrEqual;
                }
            else if (*m_expression == '>')
                {
                m_expression++;
                m_token = TOKEN_ShiftRight;
                }
            else
                m_token = TOKEN_GreaterThan;
            break;

        case '<':
            if (*m_expression == '=')
                {
                m_expression++;
                m_token = TOKEN_LessOrEqual;
                }
            else if (*m_expression == '<')
                {
                m_expression++;
                m_token = TOKEN_ShiftLeft;
                }
            else
                m_token = TOKEN_LessThan;
            break;

        case '+':   m_token = TOKEN_Add;            
            break;

        case '-':   m_token = TOKEN_Subtract;            
            break;

        case '*':   m_token = TOKEN_Multiply;            
            break;

        case '/':   m_token = TOKEN_Divide;            
            break;

        case '%':   m_token = TOKEN_Modulus;
            {
            WCharP  pEndOfWord = m_expression;
            WChar   saveChar;

            while (!isspace(*pEndOfWord)) 
                pEndOfWord++;

            if (pEndOfWord == m_expression)
                break;

            saveChar = *pEndOfWord;
            *pEndOfWord = 0;
            if (PREPROCESSOR_NotKeyword != m_macroFileProc.GetPreprocessorCommand (m_expression))
                m_token = TOKEN_PreProcessorCommand;

            *pEndOfWord = saveChar;
            break;
            }

        case '|':
            if ('|' == *m_expression)
                {
                m_expression++;
                m_token = TOKEN_LogicalOr;
                }
            else
                m_token = TOKEN_BinaryOr;
            break;

        case '&':
            if ('&' == *m_expression)
                {
                m_expression++;
                m_token = TOKEN_LogicalAnd;
                }
            else
                m_token = TOKEN_BinaryAnd;
            break;

        case 'd':
            if (0 == wcsncmp (m_expression, L"efined", 6))
                {
                m_expression += 6;
                m_token = TOKEN_PreProcDefined;
                break;
                }
            m_token = TOKEN_Symbol;
            m_expression--;
            break;

        case 'e':
            if (0 == wcsncmp (m_expression, L"xists", 5))
                {
                m_expression += 5;
                m_token = TOKEN_PreProcExists;
                break;
                }
            m_token = TOKEN_Symbol;
            m_expression--;
            break;

        default:
            m_token = TOKEN_Symbol;
            m_expression--;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
void    MustBeTokenType (TokenType checkType)
    {
    if (m_token == TOKEN_EndOfString)
        m_macroFileProc.FatalError (L"unexpected end of line");
    else if (m_token != checkType)
        {
        switch (checkType)
            {
            case TOKEN_LeftParen:
                m_macroFileProc.FatalError (L"syntax error, expected \"(\"");
            case TOKEN_RightParen:
                m_macroFileProc.FatalError (L"syntax error, expected \")\"");
            default:
                m_macroFileProc.FatalError (L"syntax error");
            }
        }
    // token is ok.
    Advance ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   CalculateBinaryNode (int type, Node& left, Node& right)
    {
    // This method Deletes the right node, and returns the value in the left node..
    switch (type)
        {
        case    TOKEN_LogicalOr:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (left.GetBooleanValue() || right.GetBooleanValue()); 
            break;

        case    TOKEN_LogicalAnd:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (left.GetBooleanValue() && right.GetBooleanValue());
            break;

        case    TOKEN_BinaryOr:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (0 != (left.GetIntegerValue() | right.GetIntegerValue())); 
            break;

        case    TOKEN_BinaryXor:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (0 != (left.GetIntegerValue() ^ right.GetIntegerValue())); 
            break;

        case    TOKEN_BinaryAnd:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (0 != (left.GetIntegerValue() & right.GetIntegerValue())); 
            break;

        case    TOKEN_Equals:
            left.SetToBoolean (left.Equals (right));
            break;

        case    TOKEN_NotEquals:
            left.SetToBoolean (!left.Equals (right));
            break;

        case    TOKEN_GreaterThan:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (left.GetIntegerValue() > right.GetIntegerValue());
            break;

        case    TOKEN_LessThan:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (left.GetIntegerValue() < right.GetIntegerValue());
            break;

        case    TOKEN_GreaterOrEqual:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (left.GetIntegerValue() >= right.GetIntegerValue());
            break;

        case    TOKEN_LessOrEqual:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToBoolean (left.GetIntegerValue() <= right.GetIntegerValue());
            break;

        case    TOKEN_ShiftLeft:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToInteger (left.GetIntegerValue() << right.GetIntegerValue());
            break;

        case    TOKEN_ShiftRight:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToInteger (left.GetIntegerValue() >> right.GetIntegerValue());
            break;

        case    TOKEN_Add:
            if (left.m_type == NODETYPE_Integer || left.m_type == NODETYPE_Boolean)
                {
                right.RequireArithType ();
                left.SetToInteger (left.GetIntegerValue() + right.GetIntegerValue());
                }
            else 
                {
                left.Concatenate (right);
                }
            break;

        case    TOKEN_Subtract:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToInteger (left.GetIntegerValue() - right.GetIntegerValue());
            break;

        case    TOKEN_Multiply:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToInteger (left.GetIntegerValue() * right.GetIntegerValue());
            break;

        case    TOKEN_Divide:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToInteger (left.GetIntegerValue() / right.GetIntegerValue());
            break;

        case    TOKEN_Modulus:
            left.ConvertToInteger ();
            right.ConvertToInteger ();
            left.SetToInteger (left.GetIntegerValue() % right.GetIntegerValue());
            break;

        }

    // done with right node.
    delete (&right);

    // return the result node.
    return  &left;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsSymbolChar (int thisChar)
    {
    return  (extended_isAlnum (thisChar) || (thisChar=='_') || (thisChar=='"'));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool    AllNumeric (WCharCP thisChar)
    {
    for ( ; 0 != *thisChar; thisChar++)
        {
        if (!isdigit(*thisChar))
            return false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsStringConstant (WCharCP thisChar)
    {
    WCharCP lastChar = thisChar + (wcslen(thisChar)-1);

    if (*thisChar == '"' && *lastChar == '"')
        return true;
    else if (*thisChar == '\'' && *(thisChar+2) == '\'' && (thisChar + 2) == lastChar)
        return true;

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void    EliminateQuotesFromString (WStringR string)
    {
    if ('"' == string[0])
        {
        size_t closeQuotePosition;
        if (WString::npos != (closeQuotePosition = string.find_first_of ('"', 1)))
            {
            WString  unquoted = string.substr (1, closeQuotePosition - 1);
            string.assign (unquoted.c_str());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsSymbolDefined (NodeP *ppNode)
    {
    m_expression = wskipSpace (m_expression);

    WChar    symbol[500], *outP, *endP;
    for (outP=symbol, endP = outP + (_countof (symbol)-1); (outP < endP) && IsSymbolChar(*m_expression); outP++, m_expression++) 
        *outP = *m_expression;

    if (outP >= endP)
        m_macroFileProc.FatalError (L"string constant too long");
    
    // null terminate the string.
    *outP = 0;

    wstripSpace (symbol);

    bool    retval = true;
    if (0 == symbol[0])
        {
        // this is the case where the first character in the symbol is not a symbol character.
        if (*m_expression == '$')
            {
            for (outP=symbol; *m_expression == '$' || *m_expression == '(' || *m_expression == ')' || IsSymbolChar (*m_expression); outP++, m_expression++)
                {
                *outP = *m_expression;
                if (*outP == ')')
                    {
                    m_expression++;
                    break;
                    }
                }
            // null terminate the string.
            *outP = 0;

            wstripSpace (symbol);
                        
            // in this case, we got $(MACRONAME). It must exist and expand to something.
            MacroExpandOptions  options (m_level);
            WString             expansion;
            if ( (BSISUCCESS != m_macroCfgAdmin.ExpandMacro (expansion, symbol, options)) || expansion.empty() )
                {
                WChar msg[1024];
                BeStringUtilities::Snwprintf (msg, L"Symbol '%ls' is undefined", symbol);
                m_macroFileProc.FatalError (msg);
                }
            else
                {
                if (NULL != ppNode)
                    {
                    if (NULL == *ppNode)
                        *ppNode = new Node (expansion.c_str(), m_macroFileProc);
                    else
                        (*ppNode)->SetToString (expansion.c_str());

                    }
                retval = true;
                }
            }
        return retval;
        }

    if (AllNumeric (symbol))
        {
        // any numeric symbol (apparently even 0) is defined.
        if (NULL == ppNode)
            return true;

        int intValue = 0;
        BE_STRING_UTILITIES_SWSCANF (symbol, L"%d", &intValue);

        if (NULL == *ppNode)
            *ppNode = new Node (intValue, m_macroFileProc);
        else
            (*ppNode)->SetToInteger (intValue);

        return true;
        }

    if (IsStringConstant (symbol))
        {
        if (NULL == ppNode)
            return true;

        if (NULL == *ppNode)
            *ppNode = new Node (symbol, m_macroFileProc);
        else
            (*ppNode)->SetToString (symbol);

        return true;
        }

    WString     tmpString;
    WCharCP     translation = m_macroCfgAdmin.GetMacroTranslation (symbol, tmpString, ConfigurationVariableLevel::User);

    if (translation == NULL)
        return false;
    else if (NULL == ppNode)
        return true;

    int intValue;
    if (IsIntegerConstant (translation, &intValue))
        {
        if (NULL == *ppNode)
            *ppNode = new Node (intValue, m_macroFileProc);
        else
            (*ppNode)->SetToInteger (intValue);
        }
    else if (IsStringConstant (translation))
        {
        if (translation != tmpString.c_str())
            tmpString.assign (translation);

        EliminateQuotesFromString (tmpString);

        if (NULL == *ppNode)
            *ppNode = new Node (tmpString.c_str(), m_macroFileProc);
        else
            (*ppNode)->SetToString (tmpString.c_str());
        }
    else
        {
        if (NULL == *ppNode)
            *ppNode = new Node (true, m_macroFileProc);
        else
            (*ppNode)->SetToBoolean (true);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsFilePresent ()
    {
    WChar   symbol[MAXFILELENGTH], *p;
    int     inParen = 0;

    m_expression = wskipSpace (m_expression);

    memset (symbol, 0, _countof (symbol));
    for (p=symbol; *m_expression && (inParen || (!isspace(*m_expression) && *m_expression!= ')')); p++, m_expression++)
        {
        *p = *m_expression;

        if (*p == '(')
            inParen++;
        else if (*p == ')')
            inParen--;
        }

    wstripSpace (symbol);
    return  (m_macroFileProc.FileExists (symbol, m_level));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadStringNode ()
    {
    WChar    symbol[500], *outP, *endP;

    m_expression++; /* skip over the leading quote */
    for (outP=symbol, endP = outP + (_countof (symbol)-1); (outP < endP) && (*m_expression != '"') && (*m_expression != '\n'); outP++, m_expression++) 
        *outP = *m_expression;
    
    // null terminate the string.
    *outP = 0;

    if (outP >= endP)
        {
        m_macroFileProc.FatalError (L"string constant too long");
        }

    if (*m_expression != '"')
        {
        m_macroFileProc.FatalError (L"expect a \" at end the string");
        return  NULL;
        }

    // skip past the last quote */
    m_expression++; 

    return new Node (symbol, m_macroFileProc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadCharNode ()
    {
    WChar    thisChar;

    m_expression++; /* skip over the leading quote */

    thisChar = *m_expression++;
    if  (*m_expression != '\'')
        {
        m_macroFileProc.FatalError (L"expected ' at end of constant char");
        return  NULL;
        }

    m_expression++; /* skip over the last quote */

    return new Node ((int)thisChar, m_macroFileProc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   PreProcessCommand()
    {
    WCharP  pEndOfWord = m_expression;
    WChar   savechar;
    NodeP   node=NULL;

    while (!isspace(*pEndOfWord)) 
        pEndOfWord++;

    if (pEndOfWord == m_expression)
        m_macroFileProc.FatalError (L"expected a preprocessor command");

    savechar = *pEndOfWord;
    *pEndOfWord = 0;
    PreProcessorCommand preProcessorCmd = m_macroFileProc.GetPreprocessorCommand (m_expression);
    *pEndOfWord = savechar;

    m_expression = pEndOfWord; /* skip the keyword */
    Advance();
    switch (preProcessorCmd)
        {
        case    PREPROCESSOR_IfDefined:
            node = new Node (IsSymbolDefined (NULL), m_macroFileProc);
            break;

        case    PREPROCESSOR_IfNotDefined:
            node = new Node (!IsSymbolDefined (NULL), m_macroFileProc);
            break;

        case    PREPROCESSOR_If:
            node = ReadExpressionNode ();
            break;

#if defined (COMMENT_OUT)
        // these could easily be added, but they were not supported in the V8i preprocessor as it was used for cfg files.
        case   PREPROCESSOR_IfFile:
            node = new Node (IsFilePresent(), m_macroFileProc);
            break;

        case   PREPROCESS_IfNoFile:
            node = new Node (!IsFilePresent(), m_macroFileProc);
            break;
#endif
        default:
            {
            WChar msg[1024];
            BeStringUtilities::Snwprintf (msg, L"bad preprocessor command '%ls' used in expression", m_expression);
            m_macroFileProc.FatalError (msg);
            }
        }

    Advance();
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadUnaryNode ()
    {
    NodeP node = NULL;

    if (m_token == TOKEN_LogicalNegate)
        {
        Advance ();
        node = ReadUnaryNode();
        node->RequireArithType();
        node->SetToBoolean (!node->GetBooleanValue());
        }
    else if (m_token == TOKEN_PreProcDefined)
        {
        Advance ();
        MustBeTokenType (TOKEN_LeftParen);
        node = new Node (IsSymbolDefined (NULL), m_macroFileProc);
        Advance ();
        MustBeTokenType (TOKEN_RightParen);
        }
    else if (m_token == TOKEN_PreProcExists)
        {
        Advance ();
        MustBeTokenType (TOKEN_LeftParen);
        node = new Node (IsFilePresent(), m_macroFileProc);
        Advance ();
        MustBeTokenType (TOKEN_RightParen);
        }
    else if (m_token == TOKEN_StringConstant)
        {
        Advance ();
        node = ReadStringNode ();
        Advance ();
        }
    else if (m_token == TOKEN_Char)
        {
        Advance ();
        node = ReadCharNode ();
        Advance ();
        }
    else if (m_token == TOKEN_Hex || m_token == TOKEN_Octal)
        {
        WCharP  pEnd = NULL;
        node = new Node ((int)BeStringUtilities::Wcstol(m_expression, &pEnd, 0), m_macroFileProc);
        m_expression = pEnd;
        Advance ();
        }
    else if (m_token == TOKEN_Symbol)
        {
        Advance ();
        IsSymbolDefined (&node);
        Advance ();
        }
    else if (m_token == TOKEN_LeftParen)
        {
        Advance ();
        node = ReadExpressionNode ();
        MustBeTokenType (TOKEN_RightParen);
        }
    else if (m_token == TOKEN_Add)
        {
        Advance ();
        node = ReadUnaryNode ();
        }
    else if (m_token == TOKEN_Subtract)
        {
        Advance ();
        node = ReadUnaryNode ();
        node->RequireArithType ();
        node->SetToInteger (node->GetIntegerValue());
        }
    else if (m_token == TOKEN_PreProcessorCommand)
        {
        node = PreProcessCommand ();
        }
    else
        m_macroFileProc.FatalError (L"invalid expression");

    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadMultiplication ()
    {
    NodeP       node;
    TokenType   op;

    node = ReadUnaryNode ();
    op = m_token;
    while (m_token == TOKEN_Multiply || m_token == TOKEN_Divide || m_token == TOKEN_Modulus)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadUnaryNode());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadAddition ()
    {
    NodeP       node;
    TokenType   op;

    node = ReadMultiplication ();
    op = m_token;
    while (m_token == TOKEN_Add || m_token == TOKEN_Subtract)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadMultiplication ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadShift ()
    {
    NodeP       node;
    TokenType   op;

    node = ReadAddition ();
    op = m_token;
    while (m_token == TOKEN_ShiftLeft || m_token == TOKEN_ShiftRight)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadAddition ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadRelational()
    {
    NodeP       node;
    TokenType   op;

    node = ReadShift ();
    op = m_token;
    while (m_token == TOKEN_GreaterThan || m_token == TOKEN_LessThan || m_token == TOKEN_GreaterOrEqual || m_token == TOKEN_LessOrEqual)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadShift ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadEquality()
    {
    NodeP       node;
    TokenType   op;

    node = ReadRelational ();
    op = m_token;
    while (op == TOKEN_Equals || op == TOKEN_NotEquals)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadRelational ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadInclusiveAnd()
    {
    NodeP       node;
    TokenType   op;

    node = ReadEquality ();
    op = m_token;
    while (op == TOKEN_BinaryAnd)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadEquality ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadExclusiveOr ()
    {
    NodeP       node;
    TokenType   op;

    node = ReadInclusiveAnd ();
    op = m_token;
    while (op == TOKEN_BinaryXor)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadInclusiveAnd ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadInclusiveOr ()
    {
    NodeP       node;
    TokenType   op;

    node = ReadExclusiveOr ();
    op = m_token;
    while (op == TOKEN_BinaryOr)
        {
        Advance ();
        node = CalculateBinaryNode  (op, *node, *ReadExclusiveOr ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadLogicalAnd ()
    {
    NodeP       node;
    TokenType   op;

    node   = ReadInclusiveOr ();
    op  = m_token;
    while (op == TOKEN_LogicalAnd)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadInclusiveOr ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadLogicalOr ()
    {
    NodeP       node;
    TokenType   op;

    node = ReadLogicalAnd();
    op   = m_token;
    while (op == TOKEN_LogicalOr)
        {
        Advance ();
        node = CalculateBinaryNode (op, *node, *ReadLogicalAnd ());
        op = m_token;
        }
    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Gray.Yu         03/01
+---------------+---------------+---------------+---------------+---------------+------*/
NodeP   ReadExpressionNode ()
    {
    NodeP   node = ReadLogicalOr ();

    if  (m_token == TOKEN_BadToken)
        {
        m_macroFileProc.FatalError (L"illegal operator");
        }

    return  node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
bool    EvaluateAsBoolean ()
    {
    Advance ();
    NodeP   node = ReadExpressionNode ();

    if ((NULL == node) || !node->IsArithType())
        m_macroFileProc.FatalError (L"expect boolean expression");

    bool    result = node->GetBooleanValue();

    delete  node;

    return  result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
int     EvaluateAsInt ()
    {
    Advance ();
    NodeP   node = ReadExpressionNode ();

    if ((NULL == node) || !node->IsArithType())
        m_macroFileProc.FatalError (L"expect integer expression");

    int     result = node->GetIntegerValue();

    delete  node;

    return  result;
    }

}; // ConstantEvaluator

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    evaluateSymbolAsBoolean
(
WCharCP                     expressionString,
ConfigurationVariableLevel  level,
MacroConfigurationAdmin&    macros,
MacroFileProcessor&         mfp
)
    {
    ConstantEvaluator ce (macros, expressionString, level, mfp);
    return ce.EvaluateAsBoolean ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int     evaluateSymbolAsInt
(
WCharCP                     expressionString,
ConfigurationVariableLevel  level,
MacroConfigurationAdmin&    macros,
MacroFileProcessor&         mfp
)
    {
    ConstantEvaluator ce (macros, expressionString, level, mfp);
    return ce.EvaluateAsInt ();
    }
