//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAGrammarObject.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPAGrammarObject
//---------------------------------------------------------------------------
// This class defines "grammar objects", that kind of object that are
// "resolution steps" in a grammar definition that use a BNF-like form.
// There are two kinds of grammar objects : tokens (or "terminals") and
// rules (or "non-terminals") and these objects are used to construct
// grammar definitions.  Each of these objects describes some state of
// analysis that may occur when parsing a source file, and they are managed
// as a common kind of situation that is handlable with this common interface.
//
// During parsing, grammar objects are "resolved" : this mean that the
// corresponding situation have been identified in the source stream, causing
// the creation of a node in the parse tree being built up.
//---------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HPAProduction.h>
#include <Imagepp/all/h/HPANode.h>

BEGIN_IMAGEPP_NAMESPACE

typedef list<HPAProduction*> HPAReferingProdList;

class HPASession;

class HPAGrammarObject
    {
public:

    virtual             ~HPAGrammarObject();

    HPANode*            Resolve(const HPANodeList& pi_rList,
                                HPASession* pi_pSession);
    void                AddReferingProduction(HPAProduction* pi_pProd);
    const HPAReferingProdList&
    GetReferingProdList() const;
    virtual bool       IsTerminal() const {
        return false;
        }
    void                SetName(const WString& pi_rName) {
        m_Name = pi_rName;
        }
    virtual WString     GetName() const {
        return m_Name;
        }

protected:

    HPAGrammarObject(HPANodeCreator* pi_pCreator);
    void                SetNodeCreator(HPANodeCreator* pi_pCreator);

private:


    // Disabled methods

    HPAGrammarObject(const HPAGrammarObject&);
    HPAGrammarObject& operator=(const HPAGrammarObject&);

    // Data members

    HPANodeCreator*     m_pNodeCreator;
    HPAReferingProdList m_ReferingProductions;
    bool               m_IsLeftRecursive;
    WString             m_Name;

    };


END_IMAGEPP_NAMESPACE

#include "HPAGrammarObject.hpp"

