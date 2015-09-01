//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAParser.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPAParser
//---------------------------------------------------------------------------
// This is the main class of the library.  It defines the interface and the
// implementation of the parser engine.  The programmer will have to create
// a class that derives from this one, having a constructor that creates the
// complete grammar definition of the language used by source file to be
// parsed with the engine, and that connects the parser with a specific
// tokenizer.
//---------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HPATokenizer.h>
#include <Imagepp/all/h/HPARule.h>
#include <Imagepp/all/h/HPAToken.h>
#include <Imagepp/all/h/HPAProduction.h>
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HPASession.h>

BEGIN_IMAGEPP_NAMESPACE
class HPANode;

class HPAParser
    {
public:

    IMAGEPP_EXPORT                     HPAParser(HPATokenizer* pi_pTokenizer = 0,
                                         HPAGrammarObject* pi_pStartRule = 0);
    IMAGEPP_EXPORT virtual             ~HPAParser();

    HPATokenizer*       GetTokenizer() const;
    HPAGrammarObject*   GetStartRule() const;

    IMAGEPP_EXPORT virtual HPANode*    Parse(const HFCPtr<HPASession>& pi_pSession);

protected:

    void                SetStartRule(HPAGrammarObject* pi_pTopRule);
    void                SetTokenizer(HPATokenizer* pi_pTokenizer);

private:

    // Disabled methods

    HPAParser(const HPAParser&);
    HPAParser& operator=(const HPAParser&);

    // Internal functions scoped to gain friendship given to HPAParser

    static bool CheckFirstObject(HPARule*, HPAGrammarObject*);
    static bool CheckSecondObject(HPARule*, HPAGrammarObject*);
    static bool CheckIndirectCompatibility(const HPAProduction&,
                                            const HPAProduction&);

    // Data members

    HPAGrammarObject*   m_pStartRule;    // top of tree
    HPATokenizer*       m_pTokenizer;    // bottom of tree, entry points


    };

END_IMAGEPP_NAMESPACE
#include "HPAParser.hpp"


