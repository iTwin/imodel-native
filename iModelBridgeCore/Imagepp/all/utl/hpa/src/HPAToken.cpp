//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPAToken.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPAToken
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPAToken.h>

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
