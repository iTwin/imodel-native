//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAToken.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPAToken
//---------------------------------------------------------------------------
// This class define one kind of grammar object : the token.  A token is a
// terminal step in the grammar definition tree (a leave) : it defines the
// type of a group of characters that form a parsing unit, making a source
// stream to change from a "suite of characters" to a "suite of tokens" which
// organisation will be analyzed to find the rules that were used.
//
// The tokens are used by the "tokenizer", which is the lexical analyzer that
// will make the stream to be viewed as a token suite, and by the grammar
// definition to be used by the parser.  Each token is a gramar object known
// to both agents.
//---------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HPAGrammarObject.h>

class HPAToken : public HPAGrammarObject
    {
public:

    _HDLLu                     HPAToken();
    _HDLLu virtual             ~HPAToken();

    virtual bool       IsTerminal() const {
        return true;
        }

protected:

private:

    // Disabled methods

    HPAToken(const HPAToken&);
    HPAToken& operator=(const HPAToken&);

    };

//---------------------------------------------------------------------------
// The node that is allocated when a token is obtained from a stream,
// creating a leave in the parse tree.
//---------------------------------------------------------------------------

class HPATokenNode : public HPANode
    {
public:

    _HDLLu                     HPATokenNode(HPAGrammarObject* pi_pToken,
                                            const WString& pi_rText,
                                            const HPASourcePos& pi_rLeftPos,
                                            const HPASourcePos& pi_rRightPos,
                                            HPASession* pi_pSession);
    _HDLLu virtual             ~HPATokenNode();

    void                SetText(const WString& pi_rText);
    const WString&      GetText() const;

protected:

private:

    // Disabled methods

    HPATokenNode(const HPATokenNode&);
    HPATokenNode& operator=(const HPATokenNode&);

    // Data members

    WString             m_Text;

    };

//---------------------------------------------------------------------------
// The node that is allocated when a numeric token is obtained from a stream,
// creating a leave in the parse tree.
//---------------------------------------------------------------------------

class HPANumberTokenNode : public HPATokenNode
    {
public:

    _HDLLu                     HPANumberTokenNode(HPAGrammarObject* pi_pToken,
                                                  const WString& pi_rText,
                                                  const HPASourcePos& pi_rLeftPos,
                                                  const HPASourcePos& pi_rRightPos,
                                                  HPASession* pi_pSession,
                                                  double pi_Value);
    _HDLLu virtual             ~HPANumberTokenNode();

    void                SetValue(double pi_Value);
    double             GetValue() const;

protected:

private:

    // Disabled methods

    HPANumberTokenNode(const HPANumberTokenNode&);
    HPANumberTokenNode& operator=(const HPANumberTokenNode&);

    // Data members

    double             m_Value;

    };

#include "HPAToken.hpp"


