/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ExpressionHandler.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

#include <ECObjects/ECExpressionNode.h>

/*  TODO list
Want to optimize the code handling PrimaryListNode.  It should be able to detect when the pertinent 
data changes to avoid looking up symbols and properties.

Need much better handling and reporting of errors; distinguish between parsing and 
evaluation errors.

Assignment needs to cast to type being assigned.  Should this be built into SetValue?

Adjust handling of primitives to include point members.  

Add libraries for math, string, and date/time processing.

Handling of units � reporting errors, automatic conversions.

Limiting syntax in "formula" mode. formulas should not use many of the operators that make it difficult to determine 
type.  Shifting, OR�ing, don't allow string if trying to get numeric type

Support for spaces in identifiers; involves eliminating reserved word operators.  This also is probably only for "formula" mode.

Reduce the amount of copying of ECValues

Decide on proper type for returning result; is ECValue sufficient?

ECValue assignment operator causes all kinds of problems

Automatic conversions of types.  Can we build that into ECValue?

Assignment operators: should array assignment allocate array entries if necessary? Only if array is not fixed?  What about arrays of structs? 
Do we always start by creating a stand-alone instance?  Do we want syntax extensions for initializing structs, arrays, and points?

Can we include relationships? What syntax (-> and <-)? What happens if the relationship is not found?  Is it an error or a null result? 
How can support accessing the properties of the related object?  How can we use the properties of the related object for criteria? 
for generating a value of the expression?  Can this modify the related instances? If so, how can we decide when to stop processing 
the related instances?

Verify propagation of null results; compare to errors; it an array index too large an error or a null result?  Current strategy is error forces
ECValue to unitialized; null for IsNull

Correction mode: allows poorly specified units, incorrectly cased property names.

Automatic units conversion and error detection

Tools for helping with method implementation; checking of types, conversion of types.

Threading issues -- can optimization modify nodes?  Do we generate nodes every time?  Allow for DeepCopy so we can keep a per-thread copy?

Instance methods -- what instances are allowed?  It seems that we cannot allow an instance that is an embedded 
*/

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString         Lexer::GetString(ExpressionToken tokenName)
    {
    wchar_t const* retval = L"";

    switch(tokenName)
        {
        case TOKEN_True:
            retval = L"True";
            break;
        case TOKEN_False:
            retval = L"False";
            break;
        case TOKEN_Like:
            retval = L"Like";
            break;
        case TOKEN_Is:
            retval = L"Is";
            break;
        case TOKEN_Star:
            retval = L"*";
            break;
        case TOKEN_Plus:
            retval = L"+";
            break;
        case TOKEN_Minus:
            retval = L"-";
            break;
        case TOKEN_Slash:
            retval = L"/";
            break;
        case TOKEN_Comma:
            retval = L",";
            break;
        case TOKEN_IntegerDivide:
            retval = L"\\";
            break;
        case TOKEN_LParen:
            retval = L"(";
            break;
        case TOKEN_RParen:
            retval = L")";
            break;
        case TOKEN_Exponentiation:
            retval = L"^";
            break;
        case TOKEN_And:
            retval = L"And";
            break;
        case TOKEN_AndAlso:
            retval = L"AndAlso";
            break;
        case TOKEN_Or:
            retval = L"Or";
            break;
        case TOKEN_OrElse:
            retval = L"OrElse";
            break;
        case TOKEN_Concatenate:
            retval = L"&";
            break;
        case TOKEN_Mod:
            retval = L"Mod";
            break;
        case TOKEN_ShiftLeft:
            retval = L"<<";
            break;
        case TOKEN_ShiftRight:
            retval = L">>";
            break;
        case TOKEN_Colon:
            retval = L":";
            break;
        case TOKEN_LessEqual:
            retval = L"<=";
            break;
        case TOKEN_GreaterEqual:
            retval = L">=";
            break;
        case TOKEN_Less:
            retval = L"<";
            break;
        case TOKEN_Greater:
            retval = L">";
            break;
        case TOKEN_Equal:
            retval = L"=";
            break;
        case TOKEN_NotEqual:
            retval = L"<>";
            break;
        case TOKEN_Not:
            retval = L"Not";
            break;
        case TOKEN_Xor:
            retval = L"Xor";
            break;
        case TOKEN_UnsignedShiftRight:
            retval = L">>>";
            break;
        case TOKEN_LeftBracket:
            retval = L"[";
            break;
        case TOKEN_RightBracket:
            retval = L"]";
            break;
        case TOKEN_Dot:
            retval = L".";
            break;
        case TOKEN_IIf:
            retval = L"IIf";
            break;
        }

    return retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const*  Lexer::getTokenStringCP () 
    {
    m_tokenBuilder[m_outputIndex] = 0;
    return m_tokenBuilder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t         Lexer::GetCurrentChar ()
    {
    return m_tokenBuilder [m_outputIndex - 1];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t         Lexer::GetNextChar  ()
    {
    wchar_t         ch;

    if (m_inputIndex >= m_maxInputIndex || m_outputIndex >= m_maxOutputIndex)
        return '\0';

    ch = m_inputCP [m_inputIndex++];
    m_tokenBuilder [m_outputIndex++] = ch;

    return ch;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            Lexer::PushBack ()
    {
    //  Unlike our other tokenizers, this does not allow the caller to specify
    //  the character to push back.  Instead, it just backs up.
    BeAssert (m_inputIndex >= 0);
    BeAssert (m_outputIndex >= 0);

    m_inputIndex--;
    m_outputIndex--;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t         Lexer::PeekChar  ()
    {
    if (m_inputIndex >= m_maxInputIndex)
        return '\0';

    return m_inputCP [m_inputIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t         Lexer::GetDigits
(
ExpressionToken&  tokenType,
bool            isOctal
)
    {
    wchar_t currentChar;

    while  (0 != iswalnum (currentChar = GetNextChar() ) )
        {
        if ((isOctal) && (currentChar > '7'))
            tokenType = TOKEN_BadOctalNumber;
        }

    return currentChar;  //  Returns the first character that is not a digit
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Lexer::IsHexDigit (wchar_t ch)
    {
    return 0 != iswxdigit(ch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t         Lexer::GetHexConstant
(
ExpressionToken&  tokenType
)
    {
    wchar_t    current;

    /*  Move the 'x' to token       */
    GetNextChar();

    while  (0 != iswalnum (current = GetNextChar()))
        {
#if defined (NOTNOW)
        //  Don't understand why this was here
        if (current == 'L' || current == 'l')
            break;
#endif

        if  (!IsHexDigit (current))
            tokenType = TOKEN_BadHexNumber;
        }

    return current;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          Lexer::GetTokenStartOffset()
    {
    return m_startPosition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void             Lexer::MarkOffset()
    {
    m_startPosition = m_inputIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionToken Lexer::GetNumericConstant ()
    {
    ExpressionToken   result = TOKEN_IntegerConstant;
    bool            haveOctal = false;
    bool            haveFloat = false;
    //  bool    haveLong = false;
    wchar_t         currentChar;
    wchar_t         nextChar;

    currentChar = GetCurrentChar ();

    if ( (currentChar == '0') &&  ((nextChar = PeekChar ()) == 'X' || nextChar == 'x'))
        {
        currentChar = GetCurrentChar ();   //  consume the x
        currentChar = GetHexConstant (result);
        }
    else if (currentChar == '.')
        {
        haveFloat = true;
        result = TOKEN_FloatConst;
        currentChar = GetDigits(result, false);
        if ('e' == currentChar || 'E' == currentChar)
            {
            currentChar = PeekChar();
            if  ((currentChar == '+') ||  (currentChar == '-'))
                GetNextChar();
            if (0 == iswdigit(PeekChar()))
                result = TOKEN_BadFloatingPointNumber;
            else
                currentChar = GetDigits(result, false);
            }
        }
    else
        /*  Does not begin with a decimal, and is not a hex number.  */
        {
        if (currentChar == '0')
            haveOctal = true;

        currentChar = GetDigits (result, haveOctal);
        if  (currentChar == '.')
            {
            result = TOKEN_FloatConst;  //  Overwrite valid value or BadOctalNumber
            haveFloat = true;
            currentChar = GetDigits (result, false);
            }
        if  (currentChar == 'E' || currentChar == 'e')
            {
            haveFloat = true;
            currentChar = PeekChar ();
            if  ((currentChar == '+') ||  (currentChar == '-'))
                GetNextChar();   //  Moves it into the token buffer
            if (0 == iswdigit(PeekChar()))
                result = TOKEN_BadFloatingPointNumber;
            else
                currentChar = GetDigits  (result, false);
            }
        }

    if  (currentChar == 'L' || currentChar == 'l')
        {
        if (haveFloat)
            result = TOKEN_BadFloatingPointNumber;
        else
            {
            /*  Consume the 'L', but do not leave it in the token.  */
            m_outputIndex--;
            //  TODO haveLong = true;
            }

        currentChar = GetNextChar();
        }

    //  currentChar now contains the first character after the token.
    //  We think we are done scanning the number, but there are more contiguous letters or numbers.
    if (0 != iswalnum (currentChar))
        {
        while  (0 != iswalnum (currentChar = GetNextChar()));
        PushBack ();
        result = TOKEN_BadNumber;
        }

    PushBack ();

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionToken Lexer::GetString ()
    {
    bool            exitOnNext = false;
    wchar_t         currentChar;

    //  TODO -- error handling, incomplete string
    while (true)
        {
        currentChar = GetNextChar();
        if (exitOnNext)
            {
            if (currentChar == '"')   //  Treat 2 quotes as a single quote
                {
                m_outputIndex--;
                exitOnNext = false;
                }
            else
                break;
            }
        else
            {
            if (currentChar == 0)
                {
                return TOKEN_UnterminatedString;
                }

            exitOnNext = currentChar == '"';
            }
        }

    //  Strip out the quotes from the string
    PushBack ();  //  Push back the first character after the string.
    //  m_tokenBuilder.Remove (0, 1);
    //  m_tokenBuilder.Remove (m_tokenBuilder.Length - 1, 1);

    return TOKEN_StringConst;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionToken Lexer::GetIdentifier ()
    {
    wchar_t    current;

    //  Get the longest token.  The caller has already called recognized the
    //  first character of identifier.  Since this scans for the subsequent
    //  characters it uses iswalnum.
    while (0 != iswalnum (current = PeekChar()) || current == '_')
        GetNextChar();

    wchar_t const*  identifier = getTokenStringCP ();

    if (wcscmp (L"True", identifier) == 0)
        return TOKEN_True;
    else if (wcscmp (L"False", identifier) == 0)
        return TOKEN_False;
    else if (wcscmp (L"And", identifier) == 0)
        return TOKEN_And;
    else if (wcscmp (L"AndAlso", identifier) == 0)
        return TOKEN_AndAlso;
    else if (wcscmp (L"Or", identifier) == 0)
        return TOKEN_Or;
    else if (wcscmp (L"OrElse", identifier) == 0)
        return TOKEN_OrElse;
    else if (wcscmp (L"Mod", identifier) == 0)
        return TOKEN_Mod;
    else if (wcscmp (L"Xor", identifier) == 0)
        return TOKEN_Xor;
    else if (wcscmp (L"Not", identifier) == 0)
        return TOKEN_Not;
    else if (wcscmp (L"Like", identifier) == 0)
        return TOKEN_Like;
    else if (wcscmp (L"Is", identifier) == 0)
        return TOKEN_Is;
    else if (wcscmp (L"IIf", identifier) == 0)
        return TOKEN_IIf;

    return TOKEN_Ident;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionToken Lexer::ScanToken ()
    {
    wchar_t         current;
    ExpressionToken   t = TOKEN_None;
    m_outputIndex   = 0;
    m_tokenModifier = TOKEN_None;

    while (true)
        {
        m_outputIndex   = 0;

        for (;;)
            {
            MarkOffset();
            current = GetNextChar ();
            if (0 == iswspace (current))
                break;

            m_outputIndex   = 0;
            }

        switch (current)
            {
            case '\0':
                return TOKEN_None;

            case '(':
                return TOKEN_LParen;

            case ')':
                return TOKEN_RParen;

            case '*':
                t = TOKEN_Star;
    modify:         if ((current = GetNextChar()) == '=')
                    {
                    m_tokenModifier = t;
                    t = TOKEN_Equal;
                    }
                else
                    {
                    PushBack ();
                    }
                return t;

            case '^':
                t = TOKEN_Exponentiation;
                goto modify;

            case '[':
                return TOKEN_LeftBracket;

            case ']':
                return TOKEN_RightBracket;

            case '/':
                t = TOKEN_Slash;
                goto modify;

            case ',':
                t = TOKEN_Comma;
                goto modify;

            case '+':
                t = TOKEN_Plus;
                goto modify;

            case '\\':
                t = TOKEN_IntegerDivide;
                goto modify;

            case '-':
                t = TOKEN_Minus;
                goto modify;

            case '&':
                return TOKEN_Concatenate;

            case '<':
                // The token may be <, <<, <=, <>
                if ((current = GetNextChar()) == '<')
                    {
                    t = TOKEN_ShiftLeft;
                    goto modify;
                    }
                else if (current == '=')
                    return TOKEN_LessEqual;
                else if (current == '>')
                    return TOKEN_NotEqual;
                else
                    {
                    PushBack ();
                    return TOKEN_Less;
                    }

            case '>':
                //  The token may be >, >>, >%, >>=, >>>, or >>>=
                if ((current = GetNextChar()) == '>')
                    {
                    if ((current = GetNextChar()) == '>')
                        {
                        //  Token may be >>> or >>>=
                        t = TOKEN_UnsignedShiftRight;
                        goto modify;
                        }

                    PushBack ();
                    //  May be >>, >>=, >>>, or >>>=
                    t = TOKEN_ShiftRight;
                    goto modify;
                    }
                else if (current == '=')
                    return TOKEN_GreaterEqual;
                else
                    {
                    PushBack ();
                    return TOKEN_Greater;
                    }

            case '=':
                return TOKEN_Equal;

            case '.':
                if (0 != iswdigit (PeekChar ()))
                    {
                    return GetNumericConstant();
                    }

                return TOKEN_Dot;

            //  May want to also allow ' as the string marker and let
            //  ' strings contain "
            case '\"':
                return GetString ();

            default:
                if (0 != iswalpha (current) || current == '_')
                    {
                    if ((t = GetIdentifier()) != 0)
                        return t;
                    else
                        return TOKEN_Unrecognized;
                    }
                else if (0 != iswdigit (current))
                    {
                    return GetNumericConstant();
                    }

                return TOKEN_Unrecognized;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                Lexer::Lexer (wchar_t const* inputString)
    {
    m_outputIndex = m_inputIndex = 0;

    m_inputString       = Bentley::WString(inputString);
    m_inputCP           = m_inputString.c_str();
    m_maxOutputIndex    = _countof(m_tokenBuilder) - 1;
    m_maxInputIndex     = wcslen(m_inputCP) + 1;   //  It includes the EOS
    m_tokenModifier = m_currentToken = TOKEN_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LexerPtr        Lexer::Create(wchar_t const* inputString)
    {
    return new Lexer(inputString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            Lexer::Advance ()
    {
    m_currentToken = ScanToken ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionToken Lexer::GetTokenType ()
    {
    return m_currentToken;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionToken Lexer::GetTokenModifier ()
    {
    return m_tokenModifier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const*  Lexer::GetTokenStringCP ()
    {
    return getTokenStringCP ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::GetErrorNode(wchar_t const*  errorMessage, wchar_t const* detail1, wchar_t const* detail2)
    {
    ErrorNodePtr    errorNode = ErrorNode::Create(errorMessage, detail1, detail2);
    return errorNode.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::Must
(
ExpressionToken   s,
NodeR            inputNode
)
    {
    if (m_lexer->GetTokenType() == s)
        {
        m_lexer->Advance ();
        return &inputNode;
        }

    return GetErrorNode(L"Expected/Got Error");
    }

    //  Already consumed (, consumes trailing )
/*---------------------------------------------------------------------------------**//**
* Returns an ArgumentTreeNode or an ErrorNode
*
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr  ECEvaluator::ParseArguments
(
)
    {
    ArgumentTreeNodePtr   argTree = Node::CreateArgumentTree();

    if (m_lexer->GetTokenType() == TOKEN_RParen)
        {
        m_lexer->Advance ();
        return argTree.get();
        }

    while (true)
        {
        NodePtr   currentArg = ParseValueExpression ();
        argTree->PushArgument(*currentArg);

        if (m_lexer->GetTokenType() != TOKEN_Comma)
            break;

        m_lexer->Advance ();
        }

    return Must (TOKEN_RParen, *argTree);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParsePrimary
(
)
        {
        NodePtr  result;

        switch (m_lexer->GetTokenType ())
            {
            case TOKEN_True:
                result = Node::CreateBooleanLiteral (true);
                m_lexer->Advance ();
                break;

            case TOKEN_False:
                result = Node::CreateBooleanLiteral (false);
                m_lexer->Advance ();
                break;

            case TOKEN_IIf:
                {
                // Requires ( expression, expression, expression )
                m_lexer->Advance ();
                if (m_lexer->GetTokenType() != TOKEN_LParen)
                    return GetErrorNode(L"IIfError");

                m_lexer->Advance ();

                NodePtr   conditional = ParseValueExpression ();
                if (m_lexer->GetTokenType() != TOKEN_Comma)
                    return GetErrorNode(L"IIfError");

                m_lexer->Advance ();
                NodePtr   trueClause = ParseValueExpression ();

                if (m_lexer->GetTokenType() != TOKEN_Comma)
                    return GetErrorNode(L"IIfError");

                m_lexer->Advance ();
                NodePtr falseClause = ParseValueExpression ();

                if (m_lexer->GetTokenType() != TOKEN_RParen)
                    return GetErrorNode(L"IIfError");

                m_lexer->Advance ();

                result = Node::CreateIIf (*conditional, *trueClause, *falseClause);
                }
                break;

            case TOKEN_Ident:
                {
                bool                dotted = false;
                PrimaryListNodePtr  primaryList = PrimaryListNode::Create ();
                result = primaryList.get();

                IdentNodePtr             identNode   = IdentNode::Create(m_lexer->GetTokenStringCP ());
                m_lexer->Advance ();

                for (;;)
                    {
                    switch (m_lexer->GetTokenType ())
                        {
                        case TOKEN_LParen:
                            {
                            m_lexer->Advance ();
                            NodePtr temp = ParseArguments (); 
                            ArgumentTreeNodeP args = dynamic_cast<ArgumentTreeNodeP>(temp.get());
                            if (NULL == args)
                                {
                                result = temp.get();   //  this should be returning an error
                                break;  //  assume there has already been an error reported.
                                }

                            wchar_t const*   name;
                            if (identNode.IsValid())
                                name = identNode->GetName();
                            else
                                {
                                //  TODO generate error
                                name = L"";
                                }

                            CallNodePtr callNode = CallNode::Create(*args, name, dotted);
                            primaryList->AppendCallNode(*callNode);
                            identNode = NULL;
                            }
                            break;

                        case TOKEN_LeftBracket:
                            {
                            if (identNode.IsValid())
                                primaryList->AppendNameNode(*identNode);

                            identNode = NULL;
                            m_lexer->Advance ();
                            NodePtr     temp = ParseValueExpression ();

                            //  Decide at evaluation time if this is an array index or property reference.
                            LBracketNodePtr lBracket = LBracketNode::Create (*temp);
                            primaryList->AppendArrayNode(*lBracket);

                            result = Must (TOKEN_RightBracket, *result);
                            }
                            break;

                        case TOKEN_Dot:
                            {
                            m_lexer->Advance ();

                            if (identNode.IsValid())
                                primaryList->AppendNameNode(*identNode);

                            DotNodePtr  dotNode = DotNode::Create(m_lexer->GetTokenStringCP ());
                            identNode = dotNode.get();
                            result = Must (TOKEN_Ident, *result);
                            dotted = true;
                            }
                            break;

                        default:
                            if (identNode.IsValid())
                                primaryList->AppendNameNode(*identNode);

                            return result;
                        }
                    }
                }
                break;

            case TOKEN_StringConst:
                result = Node::CreateStringLiteral (m_lexer->GetTokenStringCP ());
                m_lexer->Advance ();
                break;

            //  May want to have these parse the strings immediately and convert to constants
            case TOKEN_IntegerConstant:
                {
                Int64     value;
                
                BeStringUtilities::Swscanf(m_lexer->GetTokenStringCP (), L"%lld", &value);

                if (value >= INT_MIN && value <= INT_MAX)
                    {
                    int  intValue = (int)value;
                    result = Node::CreateIntegerLiteral (intValue);
                    }
                else
                    result = Node::CreateInt64Literal (value);

                m_lexer->Advance ();
                }
                break;

            case TOKEN_FloatConst:
                {
                double d;
                BeStringUtilities::Swscanf(m_lexer->GetTokenStringCP (), L"%lg", &d);
                result = Node::CreateFloatLiteral (d);
                m_lexer->Advance ();
                }
                break;

            case TOKEN_LParen:
                m_lexer->Advance ();
                result = ParseValueExpression ();               //  We do not allow embedded assignments
                result->SetHasParens(true);
                result = Must (TOKEN_RParen, *result);
                break;

    //        case NULL_LITERAL:
    //        case SLASH:  //  Would pattern matching be better?
                //  This is the start of a regular expression

    //        case LEFTBRACKET:
                //  This is the start of an array initializer

    //        case LCURLY:
                //  This is the start of an object initializer

            case TOKEN_None:
                return GetErrorNode(L"UnexpectedEOI");

            default:
                return GetErrorNode(L"TokenUnexpected", m_lexer->GetTokenStringCP ());
            }


        return result;
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseExponentiation
(
)
    {
    NodePtr result = ParsePrimary ();
    while (m_lexer->GetTokenType() == TOKEN_Exponentiation)
        {
        ExpressionToken  op = m_lexer->GetTokenType ();
        m_lexer->Advance ();

        NodePtr     right = ParsePrimary ();

        result = Node::CreateArithmetic (op, *result, *right);
        }

    return result;
    }

    //  If we wanted to support standard scripting, this is where the unary delete, void,
    //  and typeof would go.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseUnaryArith
(
)
    {
    NodePtr  result = NULL;
    ExpressionToken   op;

    if (m_lexer->GetTokenType() == TOKEN_Plus || m_lexer->GetTokenType() == TOKEN_Minus)
        {
        op = m_lexer->GetTokenType ();
        m_lexer->Advance ();
        result = ParseExponentiation ();
        result = Node::CreateUnaryArithmetic (op, *result);
        }
    else
        {
        //  No other choices. This parser does not support C's casting or sizeof operators,
        //  nor does it support ECMAScript's delete, void, or typeof expressions
        result = ParseExponentiation ();
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseMultiplicative
(
)
    {
    NodePtr     result = ParseUnaryArith ();
    while (m_lexer->GetTokenType() == TOKEN_Star || m_lexer->GetTokenType() == TOKEN_Slash)
        {
        ExpressionToken  op = m_lexer->GetTokenType ();
        m_lexer->Advance ();

        NodePtr right = ParseUnaryArith ();

        result = Node::CreateArithmetic (op, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseIntegerDivision
(
)
    {
    NodePtr     result = ParseMultiplicative ();
    while (m_lexer->GetTokenType() == TOKEN_IntegerDivide)
        {
        ExpressionToken  op = m_lexer->GetTokenType ();
        m_lexer->Advance ();

        NodePtr right = ParseMultiplicative ();

        result = Node::CreateArithmetic (op, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseMod
(
)
    {
    NodePtr result = ParseIntegerDivision ();
    while (m_lexer->GetTokenType() == TOKEN_Mod)
        {
        ExpressionToken  op = m_lexer->GetTokenType ();
        m_lexer->Advance ();

        NodePtr right = ParseIntegerDivision ();

        result = Node::CreateArithmetic (op, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseAdditive
(
)
    {
    ExpressionToken   op;
    NodePtr     result = ParseMod ();
    while (m_lexer->GetTokenType() == TOKEN_Plus || m_lexer->GetTokenType() == TOKEN_Minus)
        {
        op = m_lexer->GetTokenType ();
        m_lexer->Advance ();
        NodePtr     right = ParseMod ();

        result = Node::CreateArithmetic (op, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseConcatenation
(
)
    {
    ExpressionToken   op;
    NodePtr     result = ParseAdditive ();
    while (m_lexer->GetTokenType() == TOKEN_Concatenate)
        {
        op = m_lexer->GetTokenType ();
        m_lexer->Advance ();
        NodePtr  right = ParseAdditive ();

        result = Node::CreateArithmetic (op, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseShift
(
)
    {
    ExpressionToken      op;

    NodePtr result = ParseConcatenation ();
    while (m_lexer->GetTokenType() == TOKEN_ShiftLeft
            || m_lexer->GetTokenType() == TOKEN_ShiftRight
            || m_lexer->GetTokenType() == TOKEN_UnsignedShiftRight)
        {
        op = m_lexer->GetTokenType ();
        m_lexer->Advance ();
        NodePtr right = ParseConcatenation ();
        result = Node::CreateShift (op, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseComparison
(
)
    {
    NodePtr     result = ParseShift ();

    while (m_lexer->GetTokenType() == TOKEN_Equal
                || m_lexer->GetTokenType() == TOKEN_NotEqual
                || m_lexer->GetTokenType() == TOKEN_Less
                || m_lexer->GetTokenType() == TOKEN_LessEqual
                || m_lexer->GetTokenType() == TOKEN_Greater
                || m_lexer->GetTokenType() == TOKEN_GreaterEqual
                || m_lexer->GetTokenType() == TOKEN_Like
//                || m_lexer->GetTokenType() == TOKEN_Is      -- not clear this makes sense in our context
                )
        {
        if (m_lexer->GetTokenModifier() != TOKEN_None)
            break;

        ExpressionToken  op = m_lexer->GetTokenType ();

        m_lexer->Advance ();
        NodePtr         right = ParseShift ();
        result = Node::CreateComparison (op, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseNot
(
)
    {
    NodePtr     result;
    if (m_lexer->GetTokenType() == TOKEN_Not)
        {
        m_lexer->Advance ();
        result = ParseNot ();
        result = Node::CreateUnaryArithmetic (TOKEN_Not, *result);
        }
    else
        result = ParseComparison ();

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseConjunction
(
)
    {
    NodePtr     result = ParseNot ();
    while (m_lexer->GetTokenType() == TOKEN_And || m_lexer->GetTokenType() == TOKEN_AndAlso)
        {
        ExpressionToken   tt = m_lexer->GetTokenType ();
        m_lexer->Advance ();
        NodePtr right = ParseNot ();
        result = Node::CreateLogical (tt, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseValueExpression
(
)
    {
    NodePtr   result = NULL;

    result = ParseConjunction ();
    while (m_lexer->GetTokenType() == TOKEN_Or || m_lexer->GetTokenType() == TOKEN_OrElse || m_lexer->GetTokenType() == TOKEN_Xor)
        {
        ExpressionToken   tt = m_lexer->GetTokenType ();
        m_lexer->Advance ();
        NodePtr right = ParseConjunction ();
        result = Node::CreateLogical (tt, *result, *right);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseAssignment
(
)
    {
    NodePtr     result = ParsePrimary ();  //  Essentially primitives and the modifiers

    if (m_lexer->GetTokenType() == TOKEN_Equal)
        {
        ExpressionToken assignmentSubtype = m_lexer->GetTokenModifier();  //  Look for +=, *=, etc.
        m_lexer->Advance ();

        NodePtr  rightSide = ParseValueExpression ();

        // Raise an error if result is not a LHS expression
        result = Node::CreateAssignment (*result, *rightSide, assignmentSubtype);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseExpression
(
wchar_t const*  expression,
bool            tryAssignment
)
    {
    m_lexer = Lexer::Create (expression);
    m_lexer->Advance ();

    NodePtr theNode = tryAssignment ? ParseAssignment() : ParseValueExpression ();

    if (!CheckComplete ())
        theNode = GetErrorNode(L"Unused input", m_lexer->GetTokenStringCP());

    return theNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECEvaluator::CheckComplete ()
    {
    return m_lexer->GetTokenType()  == TOKEN_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseValueExpressionAndCreateTree
(
wchar_t const* expression
)
    {
    //  Probably should just make this reference counted
    ECEvaluator*    evaluator = new ECEvaluator (NULL);
    NodePtr   theNode = evaluator->ParseExpression (expression, false);

    delete evaluator;
    return theNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr         ECEvaluator::ParseAssignmentExpressionAndCreateTree
(
wchar_t const* expression
)
    {
    //  Probably should just make this reference counted
    ECEvaluator*    evaluator = new ECEvaluator (NULL);
    NodePtr   theNode = evaluator->ParseExpression (expression, true);

    delete evaluator;
    return theNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
                ECEvaluator::ECEvaluator
(
ExpressionContextP thisContext
)
    {
    m_thisContext = thisContext;
    }

END_BENTLEY_EC_NAMESPACE
