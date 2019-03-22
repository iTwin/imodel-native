//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAProduction.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HPAProduction
//---------------------------------------------------------------------------
// This class defines a kind of object used when defining grammar rules :
// the production.  Each rule is composed of one or mule production, each one
// being the definition of the combination and the order of rules and/or
// tokens to be resolved to make the rule to be resolved.
//---------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HPARule;
class HPAGrammarObject;

class HPAProduction
    {
public:

    HPAProduction();
    HPAProduction(HPAGrammarObject& pi_rFirst);
    HPAProduction(const HPAProduction& pi_rProduction);
    HPAProduction&      operator=(const HPAProduction& pi_rProduction);

    void                SetRule(HPARule* pi_pRule);
    HPARule*            GetRule() const;
    void                AddGrammarObject(HPAGrammarObject* pi_pObj);

    bool               operator==(const HPAProduction& pi_rRight);
    bool               operator!=(const HPAProduction& pi_rRight);
    bool               operator>(const HPAProduction& pi_rRight);
    bool               operator<(const HPAProduction& pi_rRight);
    bool               operator<=(const HPAProduction& pi_rRight);

protected:

private:

    friend class HPAParser;

    // Types and data members

    typedef vector<HPAGrammarObject*> Syntax;

    HPARule*            m_pRule;
    Syntax              m_Syntax;

    };

// The global type for production lists that define rules

typedef list<HPAProduction> HPAProductionList;

// These operators are defined to help defining grammars using C++.

inline HPAProduction            operator+(HPAGrammarObject& pi_rLeft,
                                          HPAGrammarObject& pi_rRight);

inline const HPAProduction&     operator+(const HPAProduction& pi_rLeft,
                                          HPAGrammarObject& pi_rRight);

inline HPAProductionList        operator||(const HPAProduction& pi_rLeft,
                                           const HPAProduction& pi_rRight);

inline HPAProductionList        operator||(const HPAProduction& pi_rLeft,
                                           HPAGrammarObject& pi_rRight);

inline const HPAProductionList& operator||(const HPAProductionList& pi_rLeft,
                                           const HPAProduction& pi_rRight);

inline const HPAProductionList& operator||(const HPAProductionList& pi_rLeft,
                                           HPAGrammarObject& pi_rRight);

END_IMAGEPP_NAMESPACE

#include "HPAProduction.hpp"

