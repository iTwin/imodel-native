//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPATokenizer.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPATokenizer
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPATokenizer.h>
#include <Imagepp/all/h/HPAToken.h>
#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/HFCBinStream.h>

#if 0
//----------------------------------------------------------------------------
// Here's a little consumer-transformer following the STL design philosophy.
// Notice how, since UTF-8 is bound to specific bit-patterns, our types are
// only generic in what the input and output containers are.
//
// For more on the UTF-8 layout, see
//
//   http://en.wikipedia.org/wiki/Utf8
//
// Specifically,
//   0xxxxxxx                            --> 00000000 00000000 xxxxxxxx
//   110yyyyy 10xxxxxx                   --> 00000000 00000yyy yyxxxxxx
//   1110zzzz 10yyyyyy 10xxxxxx          --> 00000000 zzzzyyyy yyxxxxxx
//   11110www 10zzzzzz 10yyyyyy 10xxxxxx --> 000wwwzz zzzzyyyy yyxxxxxx
//
// Notice how the first form is identical to ASCII.
//
// This algorithm does NOT consider whether or not your wchar_t is large
// enough to hold a 21-bit character. (UTF-8 is specified over U+0000 to
// U+10FFFF. Most modern C++ compilers use a 32-bit wchar_t, particularly
// on Linux, but some older ones still have a 16-bit wchar_t, truncating
// the range to U+0000 to U+FFFF.)
//
template <typename InputIterator,typename OutputIterator>
WString& utf8_to_wchar_t(InputIterator begin, InputIterator end, WString& result)
    {
    for (; begin != end; ++begin, ++result)
        {
        int      count = 0;       // the number of bytes in the UTF-8 sequence
        unsigned c     = (unsigned char)*begin;
        unsigned i     = 0x80;

        // Skip the stupid UTF-8 BOM that Windows programs add
        //
        // (And yes, we have to do it here like this due to problems
        // that iostream iterators have with multiple data accesses.)
        //
        // Note that 0xEF is an illegal UTF-8 code, so it is safe to have
        // this check in the loop.
        //
        if (c == 0xEF)
            c = (unsigned char)* ++ ++ ++begin;

        // Resynchronize after errors (which shouldn't happen)
        while ((c & 0xC0) == 0x80)
            c = (unsigned char)*++begin;

        // Now we count the number of bytes in the sequence...
        for (; c & i; i >>= 1) ++count;
        // ...and strip the high-code-bits from the character value
        c &= i - 1;

        // Now we build the resulting wchar_t by
        // appending all the character bits together
        for (; count > 1; --count)
            {
            c <<= 6;
            c |=  (*++begin) & 0x3F;
            }

        // And we store the result in the output container
        result += c;
        }

    // The usual generic stuff
    return result;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
class ImagePP::HPAStreamReaderUTF8 : public HFCShareableObject<HPAStreamReaderUTF8>
{
public:
    HPAStreamReaderUTF8(HFCBinStream* pStream)
        :m_pStream(pStream)    
        {
        m_bufferIndex = 0;
        m_bufferDataSize = 0;
        m_decodedLineIndex = 0;
        }

    ~HPAStreamReaderUTF8()
        {
        // The memory life cycle of the HFCBinStream used by HPA* is impossible to follow.
        // I assumed it was OK and did not change the logic so we have nothing to do here
        // since we are not the owner of m_pStream.
        }

    bool GetNextChar(WChar& next)
        {
        if(!HaveCharInLine())
            {
            if(!GetLine(m_decodedLine))
                return false;

            m_decodedLineIndex = 0;
            }

        next = m_decodedLine[m_decodedLineIndex++];
        return true;
        }

    HFCBinStream* GetStream() {return m_pStream;}

    void FreeStream() {delete m_pStream;}

private:
    bool HaveCharInLine() const {return m_decodedLineIndex < m_decodedLine.size();}

    bool HaveCharInBuffer() const {return m_bufferIndex < m_bufferDataSize;}

     bool ReadNextChar(char& next)
         {
         if(!HaveCharInBuffer())
             {
             if(!ReadToBuffer())
                 return false;            
             }

         next = m_buffer[m_bufferIndex++];
         return true;
         }

     bool ReadToBuffer()
         {
         m_bufferIndex = 0;
         m_bufferDataSize = (uint32_t)m_pStream->Read(m_buffer, sizeof(m_buffer));
         return 0 != m_bufferDataSize;             
         }

     bool GetLine(WString& line)
         {
         Utf8String lineUTF8;

         while(1)
             {
             char currentChar;
             if(!ReadNextChar(currentChar))
                 {
                 break; // no more char.
                 }             
             else if('\n' == currentChar)
                 {
                 lineUTF8 += currentChar;
                 break;
                 }
             else
                 {
                 lineUTF8 += currentChar;
                 }
             }

         line.AssignUtf8(lineUTF8.c_str());     // Convert UTF8 to Unicode.

         return !line.empty();
         }

    HFCBinStream*   m_pStream;

    char            m_buffer[256];
    uint32_t        m_bufferDataSize;
    uint32_t        m_bufferIndex;

    WString         m_decodedLine;
    uint32_t        m_decodedLineIndex;
    };
// Global token descriptors

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPATokenizer::HPATokenizer()
    {
    // Nothing to do!
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPATokenizer::~HPATokenizer()
    {
    // Nothing to do!
    }

//---------------------------------------------------------------------------
//--START FLOAT-ENABLED TOKENIZER----------------------------------------------
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPAFloatEnabledDefaultTokenizer::HPAFloatEnabledDefaultTokenizer(bool pi_IsCaseSensitive)
    : HPADefaultTokenizer(pi_IsCaseSensitive)
    {
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPAFloatEnabledDefaultTokenizer::~HPAFloatEnabledDefaultTokenizer()
    {
    // Nothing to do!
    }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPANode* HPAFloatEnabledDefaultTokenizer::GetToken()
    {
    if (!m_StreamRefStack.size())
        return 0;

    HPAStreamReaderUTF8* pStream = m_StreamRefStack.front().GetPtr();

    // Checks first char : if numeric, we have a number, if alpha, we have
    // an ident of a symbol, other : we have a symbol.

    HPANode* pNode = 0;
    WString ReadToken;
    WString UppercaseToken;
    HPAToken* pFoundToken = 0;
    WChar LastChar;
    bool CharAvailable = GetChar(pStream, &LastChar);
    if (CharAvailable)
        {
        HPASourcePos LeftPos = m_StreamInfoStack.front();
        LeftPos.m_Column++;

        // Skipping whitespace, counting lines

        while (CharAvailable && (iswspace(LastChar) || iswcntrl(LastChar)))
            {
            if (LastChar == L'\n')
                {
                LeftPos.m_Line++;
                LeftPos.m_Column = 1;
                }
            CharAvailable = GetChar(pStream, &LastChar);
            LeftPos.m_Column++;
            }

        if (!CharAvailable)
            return 0;

        HPASourcePos RightPos = LeftPos;

        // Extracting a number

        if (iswdigit(LastChar))
            {
            while (CharAvailable && (iswdigit(LastChar) ||
                                     (LastChar == L'.') ||
                                     (LastChar == L'-') ||
                                     (LastChar == L'+') ||
                                     (LastChar == L'E') ||
                                     (LastChar == L'e')))
                {
                ReadToken.append(1, LastChar);
                CharAvailable = GetChar(pStream, &LastChar);
                RightPos.m_Column++;
                }
#if (0)
            while (CharAvailable && (iswdigit(LastChar) || (LastChar == L'.')))
                {
                ReadToken.append(1, LastChar);
                CharAvailable = GetChar(pStream, &LastChar);
                RightPos.m_Column++;
                }
#endif
            if (CharAvailable)
                {
                PushChar(LastChar);
                RightPos.m_Column--;
                }
            pFoundToken = m_pNumberToken;
            }

        // Extracting an alphanumeric symbol

        else if (iswalpha(LastChar) || (LastChar == L'_'))
            {
            while (CharAvailable && (iswalnum(LastChar) || (LastChar == L'_')))
                {
                ReadToken.append(1, LastChar);
                UppercaseToken.append(1, towupper(LastChar));
                CharAvailable = GetChar(pStream, &LastChar);
                RightPos.m_Column++;
                }
            if (CharAvailable)
                {
                PushChar(LastChar);
                RightPos.m_Column--;
                }

            SymbolTable::iterator itr;
            if (m_IsCaseSensitive)
                itr = m_SymbolTable.find(ReadToken);
            else
                itr = m_SymbolTable.find(UppercaseToken);
            if (itr != m_SymbolTable.end())
                pFoundToken = (*itr).second;
            else
                pFoundToken = m_pIdentifierToken;
            }

        // Extracting a string

        else if (LastChar == L'"')
            {
            do
                {
                CharAvailable = GetChar(pStream, &LastChar);
                if (LastChar == L'"')
                    break;
                else
                    ReadToken.append(1, LastChar);
                RightPos.m_Column++;
                }
            while (CharAvailable);
            pFoundToken = m_pStringToken;
            }

        // Skipping a comment and getting next token

        else if (LastChar == m_CommentMarker)
            {
            while (CharAvailable && (LastChar != L'\n'))
                CharAvailable = GetChar(pStream, &LastChar);
            LeftPos.m_Line++;
            LeftPos.m_Column = 1;
            m_StreamInfoStack.front() = LeftPos;
            return GetToken();
            // no need to handle RightPos, we have returned.
            }

        // Otherwise : extracting a non-alphanumeric symbol

        else {
            ReadToken.append(1, LastChar);
            SymbolTable::iterator itr;
            itr = m_SymbolTable.find(ReadToken);
            if (itr != m_SymbolTable.end())
                pFoundToken = (*itr).second;
            else
                {
                if (m_pErrorToken)
                    pFoundToken = m_pErrorToken;
                else
                    {
                    HFCPtr<HPANode> pExceptionNode(new HPANode(0, LeftPos, RightPos, m_pSession));
                    throw HPANoTokenException(pExceptionNode);
                    }
                }
            }

        // Updating position info

        m_StreamInfoStack.front() = RightPos;

        // Making the node with found token.

        if (pFoundToken)
            {
            pNode = MakeNode(pFoundToken, ReadToken, LeftPos, RightPos, m_pSession);
            }
        }

    // If EOF occurred, look at the stream-stack :  maybe we were scanning
    // an included file, so we should get back to previously scanned file.

    else {
        if (m_StreamRefStack.size() > 1)
            {
            m_StreamRefStack.front()->FreeStream();
            m_StreamRefStack.pop_front();
            m_StreamInfoStack.pop_front();
            pNode = GetToken();
            }
        }
    return pNode;
    }


//---------------------------------------------------------------------------
//--END FLOAT-ENABLED TOKENIZER----------------------------------------------
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPADefaultTokenizer::HPADefaultTokenizer(bool pi_IsCaseSensitive)
    : m_IsCaseSensitive(pi_IsCaseSensitive)
    {
    m_CommentMarker = 0;
    m_CurrentLine = 1;
    m_pNumberToken = 0;
    m_pStringToken = 0;
    m_pIdentifierToken = 0;
    m_pErrorToken = 0;
    m_CurrentLine = 0;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPADefaultTokenizer::~HPADefaultTokenizer()
    {
    // Nothing to do!
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline bool HPADefaultTokenizer::GetChar(HPAStreamReaderUTF8* pi_pStream,
                                          WChar* po_pChar)
    {
    if (m_PushedChars.size())
        {
        *po_pChar = m_PushedChars.front();
        m_PushedChars.pop_front();
        return true;
        }
    else
        {
        return pi_pStream->GetNextChar(*po_pChar);
        }
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::PushChar(WChar pi_Char)
    {
    m_PushedChars.push_front(pi_Char);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPANode* HPADefaultTokenizer::GetToken()
    {
    if (!m_StreamRefStack.size())
        return 0;

    HPAStreamReaderUTF8* pStream = m_StreamRefStack.front().GetPtr();

    // Checks first char : if numeric, we have a number, if alpha, we have
    // an ident of a symbol, other : we have a symbol.

    HPANode* pNode = 0;
    WString ReadToken;
    WString UppercaseToken;
    HPAToken* pFoundToken = 0;
    WChar LastChar;
    bool CharAvailable = GetChar(pStream, &LastChar);
    if (CharAvailable)
        {
        HPASourcePos LeftPos = m_StreamInfoStack.front();
        LeftPos.m_Column++;

        // Skipping whitespace, counting lines

        while (CharAvailable && (iswspace(LastChar) || iswcntrl(LastChar)))
            {
            if (LastChar == L'\n')
                {
                LeftPos.m_Line++;
                LeftPos.m_Column = 1;
                }
            CharAvailable = GetChar(pStream, &LastChar);
            LeftPos.m_Column++;
            }

        if (!CharAvailable)
            return 0;

        HPASourcePos RightPos = LeftPos;

        // Extracting a number

        if (iswdigit(LastChar))
            {
            while (CharAvailable && (iswdigit(LastChar) || (LastChar == L'.')))
                {
                ReadToken.append(1, LastChar);
                CharAvailable = GetChar(pStream, &LastChar);
                RightPos.m_Column++;
                }
            if (CharAvailable)
                {
                PushChar(LastChar);
                RightPos.m_Column--;
                }
            pFoundToken = m_pNumberToken;
            }

        // Extracting an alphanumeric symbol

        else if (iswalpha(LastChar) || (LastChar == L'_'))
            {
            while (CharAvailable && (iswalnum(LastChar) || (LastChar == L'_')))
                {
                ReadToken.append(1, LastChar);
                UppercaseToken.append(1, towupper(LastChar));
                CharAvailable = GetChar(pStream, &LastChar);
                RightPos.m_Column++;
                }
            if (CharAvailable)
                {
                PushChar(LastChar);
                RightPos.m_Column--;
                }

            SymbolTable::iterator itr;
            if (m_IsCaseSensitive)
                itr = m_SymbolTable.find(ReadToken);
            else
                itr = m_SymbolTable.find(UppercaseToken);
            if (itr != m_SymbolTable.end())
                pFoundToken = (*itr).second;
            else
                pFoundToken = m_pIdentifierToken;
            }

        // Extracting a string

        else if (LastChar == L'"')
            {
            do
                {
                CharAvailable = GetChar(pStream, &LastChar);
                if (LastChar == L'"')
                    break;
                else
                    ReadToken.append(1, LastChar);
                RightPos.m_Column++;
                }
            while (CharAvailable);
            pFoundToken = m_pStringToken;
            }

        // Skipping a comment and getting next token

        else if (LastChar == m_CommentMarker)
            {
            while (CharAvailable && (LastChar != L'\n'))
                CharAvailable = GetChar(pStream, &LastChar);
            LeftPos.m_Line++;
            LeftPos.m_Column = 1;
            m_StreamInfoStack.front() = LeftPos;
            return GetToken();
            // no need to handle RightPos, we have returned.
            }

        // Otherwise : extracting a non-alphanumeric symbol

        else {
            ReadToken.append(1, LastChar);
            SymbolTable::iterator itr;
            itr = m_SymbolTable.find(ReadToken);
            if (itr != m_SymbolTable.end())
                pFoundToken = (*itr).second;
            else
                {
                if (m_pErrorToken)
                    pFoundToken = m_pErrorToken;
                else
                    {
                    HFCPtr<HPANode> pExceptionNode(new HPANode(0, LeftPos, RightPos, m_pSession));
                    throw HPANoTokenException(pExceptionNode);
                    }
                }
            }

        // Updating position info

        m_StreamInfoStack.front() = RightPos;

        // Making the node with found token.

        if (pFoundToken)
            {
            pNode = MakeNode(pFoundToken, ReadToken, LeftPos, RightPos, m_pSession);
            }
        }

    // If EOF occurred, look at the stream-stack :  maybe we were scanning
    // an included file, so we should get back to previously scanned file.

    else {
        if (m_StreamRefStack.size() > 1)
            {
            m_StreamRefStack.front()->FreeStream();
            m_StreamRefStack.pop_front();
            m_StreamInfoStack.pop_front();
            pNode = GetToken();
            }
        }
    return pNode;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPANode* HPADefaultTokenizer::MakeNode(HPAToken* pi_pToken, const WString& pi_rText,
                                       const HPASourcePos& pi_rLeft,
                                       const HPASourcePos& pi_rRight,
                                       HPASession* pi_pSession)
    {
    HPANode* pNode;
    if (pi_pToken == m_pNumberToken)
        {
        WChar* pDummy;
        pNode = new HPANumberTokenNode(pi_pToken, pi_rText,
                                       pi_rLeft, pi_rRight,
                                       pi_pSession,
                                       wcstod(pi_rText.c_str(), &pDummy));
        }
    else
        pNode = new HPATokenNode(pi_pToken, pi_rText,
                                 pi_rLeft, pi_rRight,
                                 pi_pSession);
    return pNode;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HPADefaultTokenizer::BeginSession(HFCBinStream* pi_pStream,
                                       HPASession* pi_pSession)
    {
    m_pSession = pi_pSession;
    m_BackupTable = m_SymbolTable;
    Include(pi_pStream);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HPADefaultTokenizer::EndSession()
    {
    m_SymbolTable = m_BackupTable;
    m_StreamRefStack.clear();
    m_PushedChars.clear();
    m_CurrentLine = 1;
    m_pSession = 0;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HPADefaultTokenizer::Include(HFCBinStream* pi_pStream)
    {
    HPRECONDITION(pi_pStream != 0);
    StreamRefStack::iterator itr(m_StreamRefStack.begin());
    while (itr != m_StreamRefStack.end())
        {
        if ((*itr)->GetStream()->GetURL()->GetURL() == pi_pStream->GetURL()->GetURL())
            return false;
        ++itr;
        }
    m_StreamRefStack.push_front(new HPAStreamReaderUTF8(pi_pStream));
    HPASourcePos NewPos;
    NewPos.m_pURL = pi_pStream->GetURL();
    NewPos.m_Line = 1;
    NewPos.m_Column = 1;
    m_StreamInfoStack.push_front(NewPos);
    return true;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HPADefaultTokenizer::AddSymbol(const WString& pi_rString, HPAToken& pi_rToken)
    {
    WString SymbolText = pi_rString;
    if (m_IsCaseSensitive)
        CaseInsensitiveStringTools().ToUpper(SymbolText);

    m_SymbolTable.insert(SymbolTable::value_type(SymbolText, &pi_rToken));
    pi_rToken.SetName(L"Token : " + pi_rString);
    }



