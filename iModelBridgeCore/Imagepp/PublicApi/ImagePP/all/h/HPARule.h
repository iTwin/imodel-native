//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPARule.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPARule
//---------------------------------------------------------------------------
// This class define one kind of grammar object : the rule.  A rule is a
// non-terminal step in the grammar definition tree (a node that is not a
// leave): it defines a "state of resolution" that may be obtained when a
// specific combination of tokens and/or "sub-rules" have been found; that
// combination is labeled "production".  A rule may be described by more
// than one production, these being OR'ed (resolving one of them make
// the rule to be resolved).
//---------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HPAGrammarObject.h>
#include <Imagepp/all/h/HPAProduction.h>

BEGIN_IMAGEPP_NAMESPACE
class HPARule : public HPAGrammarObject
    {
public:


    IMAGEPP_EXPORT                     HPARule();
    IMAGEPP_EXPORT                     HPARule(const HPAProduction& pi_rProduction);
    IMAGEPP_EXPORT                     HPARule(const HPAProductionList& pi_rProdList);
    IMAGEPP_EXPORT virtual             ~HPARule();
    IMAGEPP_EXPORT HPARule&            operator=(const HPAProduction& pi_rProduction);
    IMAGEPP_EXPORT HPARule&            operator=(const HPAProductionList& pi_rProdList);

    HPARule&            operator()(HPANodeCreator* pi_pCreator);

    bool               IsLeftRecursive() const;
    void                SetLeftRecursive();

protected:

    void                LinkProductions();

private:

    friend class HPAParser;

    // Disabled methods

    HPARule(const HPARule&);
    HPARule& operator=(const HPARule&);

    // Productions and production lists are duplicated
    // Production cross-reference is built here and not in production's code

    HPAProductionList   m_Productions;
    bool               m_IsLeftRecursive;

    };

END_IMAGEPP_NAMESPACE
#include "HPARule.hpp"

