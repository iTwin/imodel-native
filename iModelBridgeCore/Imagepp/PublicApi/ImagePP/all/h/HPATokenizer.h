//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPATokenizer.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPATokenizer
//---------------------------------------------------------------------------
// The tokenizer is the engine that is used to transform a character stream
// into a token stream. It knows the rules to be used to extracts numbers,
// operators, statements and identifiers fromt he stream, using a symbol
// table if necessary.
//---------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HPANode.h>
class HPATokenNode;
class HPAToken;
class HFCBinStream;
class HPASession;

class HPATokenizer
    {
public:

    _HDLLu                         HPATokenizer();
    _HDLLu virtual                 ~HPATokenizer();

    virtual bool           Include(HFCBinStream* pi_pStream) = 0;

protected:

    friend class HPAParser;

    virtual HPANode*        MakeNode(HPAToken* pi_pToken, const WString& pi_rText,
                                     const HPASourcePos& pi_rLeft,
                                     const HPASourcePos& pi_rRight,
                                     HPASession* pi_pSession) = 0;
    virtual void            BeginSession(HFCBinStream* pi_pStream,
                                         HPASession* pi_pSession) = 0;
    virtual HPANode*        GetToken() = 0;
    virtual void            EndSession() = 0;

private:

    // Disabled methods

    HPATokenizer(const HPATokenizer&);
    HPATokenizer& operator=(const HPATokenizer&);
    };

//---------------------------------------------------------------------------
// The default tokenizer, ready-to-use.  Alphanumeric characters are grouped
// into identifiers, symbols or numbers, blank characters are
// ignored, other characters are read one by one until a corresponding entry
// in symbol table is found.
//
// Supported global tokens : g_NumberToken, g_IdentifierToken,
//                           g_StringToken, g_ErrorToken
// (strings have to be enclosed in double-quotes)
//
// Number extraction is simple, only contiguous digits are found.  Support
// for decimal point and sign must be done through grammar definition.
//---------------------------------------------------------------------------

class HPADefaultTokenizer : public HPATokenizer
    {
public:

    _HDLLu                     HPADefaultTokenizer(bool pi_IsCaseSensitive = false);
    _HDLLu virtual             ~HPADefaultTokenizer();

    _HDLLu virtual bool       Include(HFCBinStream* pi_pStream);

    void                SetCommentMarker(WChar pi_Marker);
    _HDLLu void                AddSymbol(const WString& pi_rString, HPAToken& pi_rToken);
    void                SetNumberToken(HPAToken& pi_rToken);
    void                SetStringToken(HPAToken& pi_rToken);
    void                SetIdentifierToken(HPAToken& pi_rToken);
    void                SetErrorToken(HPAToken& pi_rToken);

protected:

    _HDLLu virtual HPANode*        MakeNode(HPAToken* pi_pToken, const WString& pi_rText,
                                            const HPASourcePos& pi_rLeft,
                                            const HPASourcePos& pi_rRight,
                                            HPASession* pi_pSession);
    _HDLLu virtual void            BeginSession(HFCBinStream* pi_pStream,
                                                HPASession* pi_pSession);
    _HDLLu virtual HPANode*        GetToken();
    _HDLLu virtual void            EndSession();

    bool                   GetChar(HFCBinStream* pi_pStream, WChar* po_pChar);
    void                    PushChar(WChar pi_Char);

#if (0)
// HChk AR To override default token, all members require protected access
// It would be preferable to provide protected access methods
private:
#else
protected:
#endif

    typedef map<WString, HPAToken*, less<WString> > SymbolTable;

    typedef deque<HFCBinStream*> StreamRefStack;

    typedef deque<HPASourcePos> StreamInfoStack;

    typedef deque<WChar> PushedChars;

    HPASession*         m_pSession;
    SymbolTable         m_SymbolTable;
    bool               m_IsCaseSensitive;
    WChar              m_CommentMarker;
    HPAToken*           m_pNumberToken;
    HPAToken*           m_pStringToken;
    HPAToken*           m_pIdentifierToken;
    HPAToken*           m_pErrorToken;
    uint32_t            m_CurrentLine;
    PushedChars         m_PushedChars;

    SymbolTable         m_BackupTable;
    StreamRefStack      m_StreamRefStack;
    StreamInfoStack     m_StreamInfoStack;
    };


//---------------------------------------------------------------------------
// The default float-enabled tokenizer, ready-to-use.  Alphanumeric characters are grouped
// into identifiers, symbols or numbers, blank characters are
// ignored, other characters are read one by one until a corresponding entry
// in symbol table is found.
//
// Supported global tokens : g_NumberToken, g_IdentifierToken,
//                           g_StringToken, g_ErrorToken
// (strings have to be enclosed in double-quotes)
//
// Number extraction allows floating points without the use of grammar.
//---------------------------------------------------------------------------

class HPAFloatEnabledDefaultTokenizer : public HPADefaultTokenizer
    {
public:

    HPAFloatEnabledDefaultTokenizer(bool pi_IsCaseSensitive = false);
    virtual             ~HPAFloatEnabledDefaultTokenizer();

protected:
    virtual HPANode*        GetToken();


    };


#include "HPATokenizer.hpp"

