//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPAToken
//---------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HPAToken.h>

#if 0
static struct HPATokenNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            HPASession* pi_pSession)
        {
        return new HPATokenNode(pi_pObj);
        }
    } s_TokenNodeCreator;
#endif

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPAToken::HPAToken()
    : HPAGrammarObject(0)
    {
    // Nothing else to do
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPAToken::~HPAToken()
    {
    // Nothing to do
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPATokenNode::~HPATokenNode()
    {
    // Nothing to do
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPANumberTokenNode::~HPANumberTokenNode()
    {
    // Nothing to do
    }
